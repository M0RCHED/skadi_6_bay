# ⚡ Skadi 6‑Bay Smart Charger — v4.5 (Final Engineering Revision)

> **Projet EOS Positioning Systems — Division R&D Skadi**  
> Version consolidée (matérielle + logicielle) intégrant la logique firmware finale, la sécurité complète, et les spécifications de production.  
> Conçu pour les batteries **Excel 2EXL1505 (7.2 V / 3.3 Ah / 23.76 Wh)** à base de cellules **NCR18650B**.  
> Objectif : maintenir automatiquement le SoC à **30 % (stockage)** ou **100 % (plein)**, avec supervision, isolation et télémétrie BLE.

---

## 🧭 1 — Présentation du Projet

Ce chargeur intelligent à **6 baies** permet de gérer automatiquement la charge des batteries Skadi 2S Li‑ion selon deux modes :
- **Mode Stockage (30 %)** : ramener ou maintenir le SoC à 30 % pour conformité IATA DGR / UN 3480 ;
- **Mode Plein (100 %)** : recharge complète pour tests et exploitation ;

Chaque pack intègre son **chargeur CC/CV (BQ24610)** et son **fuel‑gauge (MAX17263)**.  
La carte maître contrôle l’alimentation 12 V, supervise l’état, et communique par BLE.

**Entrée** : 12 V / 12 A DC  **Connectique baie** : 2×6 pogo pins (12 V, GND, SDA, SCL, DET, réservé)

---

## ⚙️ 2 — Architecture du Système

```
        ┌───────────────────────────────┐
        │ PSU 12 V / 12 A (150 W)      │
        └──────────┬────────────────────┘
                   │  (Fuse 15 A + TVS + NTC)
        ┌──────────┴──────────┐
        │ Bus 12 V commun    │
        └┬─┬─┬─┬─┬─┬────────┘
         │ │ │ │ │ │  (6 baies)
         ▼ ▼ ▼ ▼ ▼ ▼
     ┌────────────────────┐
     │ P‑MOS AO4407A ×6  │ ← commande grille via N‑MOS DMG2302UX
     └────────────────────┘
         │   │   │   │   │
         ▼   ▼   ▼   ▼   ▼
   ┌──────────────────────────────┐
   │ Pogo pins (2×6) → Packs 2S  │
   │  (MAX17263 + BQ24610)       │
   └──────────────────────────────┘

             │  (Bus I²C racine 3V3)
      ┌──────┴────────────────────────────┐
      │ STM32L031K6 (MCU)                │
      │  ├─ TCA9548A (MUX I²C 8 ch)      │
      │  │    ├─ TCA9803×6 → Packs       │
      │  │    └─ PCA9634 → LED status    │
      │  ├─ ADC → 12 V Monitor           │
      │  ├─ UART → HC‑08 (BLE)           │
      │  └─ GPIO → MOSFETs / Ventilateur │
      └───────────────────────────────────┘
```

---

## 🔋 3 — Référence Batterie : Excel 2EXL1505

| Paramètre | Valeur | Commentaire |
|------------|--------|--------------|
| Type | Li‑ion 2S (2 × 18650 NCR18650B) | Cellules Panasonic 3.35 Ah |
| Tension nominale | 7.2 V | 3.6 V × 2 |
| Tension max | 8.4 V | Fin CV du BQ24610 |
| Capacité | 3.3 Ah | 23.76 Wh (étiquette) |
| Courant charge | 1.6 A (0.5 C) | Typique CC/CV |
| Protection interne | Fusible 2 A + PTC + NTC 10 k | Double sécurité |
| Température charge | 0 → 45 °C |  |
| Température décharge | –20 → 60 °C |  |

**Remarque :** La LUT du MAX17263 utilise le profil NCR18650B → SoC précis sans recalibration.

---

## 🧮 4 — Dimensionnement & Formules

\[
I_{IN} = \frac{V_{BAT} × I_{BAT}}{η × V_{IN}}
\]

Exemple : 7.4 V × 1.6 A / (0.9 × 12 V) ≈ 1.1 A par baie → 6 baies ≈ 7 A totaux.

**Bus 12 V :** pistes 3 mm (ΔT ≤ 20 °C) ; branches ≥ 1 mm ; norme IPC‑2152.

---

## 🧠 5 — Logique Firmware (Machine à états finis)

```
              ┌────────────┐
              │   IDLE     │←────────────┐
              │ (aucun pack)│            │
              └──────┬─────┘            │
                     │                 │
             Pack détecté              │
                     ▼                 │
               ┌────────────┐          │
               │   CHECK    │          │
               │ Lecture SoC│          │
               └──────┬─────┘          │
         SoC<seuil    │   défaut I²C   │
                     ▼                 │
             ┌──────────────┐          │
             │  CHARGING    │──────────┘
             │ 12 V ON→pack │
             └──────┬───────┘
           SoC≥cible│  T°>45 °C
                     ▼
             ┌──────────────┐
             │   STABLE     │
             │ (30 % ou 100 %)│
             └──────┬───────┘
           défaut → │
                     ▼
             ┌──────────────┐
             │   FAULT      │
             └──────────────┘
```

**Principes clés :**
- Boucle non bloquante (Systick 2 s).  
- Lecture I²C via TCA9548A + TCA9803 ;
- Contrôle LEDs via PCA9634 ;
- Mode 100 % : le 12 V reste appliqué → le BQ24610 termine la charge (CC→CV→cut‑off) ;
- Mode 30 % : coupure 12 V après atteinte du SoC cible.

---

## 🔎 6 — Surveillance ADC 12 V

Pont diviseur : 100 k / 20 k → ≈ 2 V @ 12 V entrée.

| Condition | Seuil | Action firmware |
|------------|--------|----------------|
| **PSU basse** | < 9.5 V (3 s) | Coupure toutes baies + log fault |
| **PSU haute** | > 13.2 V | Alerte BLE + désactivation fan |
| **Nominal** | 11.5–12.5 V | Fonctionnement normal |

---

## 🛡️ 7 — Sécurité et Fiabilité

| Fonction | Description |
|-----------|-------------|
| **Double Watchdog (IWDG + WWDG)** | Redémarrage auto en cas de blocage firmware |
| **Supervisor TPS3809K33** | Reset MCU si VCC < 3.0 V ou chute PSU |
| **TCA9803 Pull‑down 10 kΩ** | Évite bus I²C flottant si pack débranché |
| **Protection interne pack** | Fusible 2 A + PTC interne + NTC 10 k |
| **Protection externe baie** | PTC 2.5 A + TVS SMBJ12A par baie |
| **Thermal vias MOSFETs** | ΔT < 15 °C @ 1 A avec ventilation |
| **Event logging Flash** | Historique SoC/T°/Fault (32 événements FIFO) |

---

## 📈 8 — Profil de Charge (CC/CV)

```
Courant (A)
│\
│ \
│  \______              ← Phase CV : courant chute
│         \___
│_____________ Temps →
```

**Phases :**  
- CC : courant constant jusqu’à ≈ 4.15 V / cellule ;
- CV : tension constante 8.4 V (pack 2S), courant → 0.06 A ;
- Fin charge : courant < 65 mA → arrêt automatique du BQ24610.

\[
t_{CC} ≈ \frac{C_{eff} × (SOC_2 − SOC_1)}{I_{BAT}}
\]
Exemple : 3.35 Ah, 0 → 100 % → ≈ 2.3 h (+ 15–25 min CV) à 0.5 C.  
20 → 30 % → ≈ 12 min.

---

## 📦 9 — BOM (Détail Complet)

| Section | Composants | Qté | Notes |
|----------|-------------|-----|-------|
| **Entrée 12 V** | PSU 12 V 12 A, Fusible 15 A (T), TVS SMBJ15A, NTC 5D‑11, Condensateurs 470–1000 µF + 0.1 µF | 1 | Alim commune |
| **Commutation par baie** | AO4407A (P‑MOS), DMG2302UX (N‑MOS), R_g 47–100 Ω, Pull‑up 100 kΩ → 12 V, Zener 12–15 V G‑S, PTC 2.5 A, TVS SMBJ12A, Cdec 10 µF + 0.1 µF | 6 | Étages haute tension |
| **Logique & I²C** | STM32L031K6, TPS3809, TCA9548A, TCA9803×6, PCA9634, R_pull‑ups 2.2–4.7 kΩ, R_series 33–47 Ω, R_pulldown 10 kΩ (B‑side), ESD array | 1 | Cœur système |
| **LEDs & indication** | LEDs R/V/B/J, PCA9634, R_EXT courant 10 mA / canal | 1 | Statut par baie |
| **Connectique** | Pogo pins 2×6 or plated | 6 | Interface batterie |
| **Debug** | Pads SWD (3 pins) + UART (3 pins) | 1 | Programmation / test |
| **Ventilation** | Fan 12 V PWM (option), MOSFET AO3400, NTC 10 k châssis | 1 | Gestion thermique |

---

## 🧰 10 — Notes de Conception

- Découplage : 100 nF par VDD + 4.7 µF bulk près du MCU.  
- BOOT0 : strap GND via 100 kΩ ; NRST : C 100 nF.  
- Traces SDA/SCL courtes + R‑série 33–47 Ω.  
- GND plan continu + via‑stitching autour des MOSFETs.  
- Distance I²C ≤ 10 cm entre MUX et TCA9803.  
- Pistes MOSFETs larges ≥ 3 mm, zones cuivre autour PTC.

---

## 🚀 11 — Étapes Suivantes

1. Validation thermique (6× 1.6 A / 8.4 V).  
2. Calibration ADC PSU + supervisor TPS3809.  
3. Tests I²C multi‑baies + reconnexion à chaud.  
4. Intégration firmware BLE / LEDs / logs Flash.  
5. Finalisation PCB production (2 couches ou 4 avec fan optionnel).

---

© 2025 EOS Positioning Systems — R&D Skadi Project

