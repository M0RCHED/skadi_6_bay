# ⚡ Skadi 6-Bay Smart Charger — v4.5 (Final Engineering Revision)

> **Projet EOS Positioning Systems — Division R&D Skadi**  
> Version consolidée (matérielle + logicielle) intégrant la logique firmware finale, la sécurité complète, et les spécifications de production.  
> Conçu pour les batteries **Excel 2EXL1505 (7.2 V / 3.3 Ah / 23.76 Wh)** (2×18650), cellules équivalentes **NCR18650B**.  
> Objectif : maintenir automatiquement le SoC à **30 % (stockage)** ou **100 % (plein)**, avec supervision, isolation et télémétrie BLE.

---

## 🧭 1 — Présentation du Projet

Ce chargeur intelligent à **6 baies** gère automatiquement la charge des batteries Skadi 2S Li‑ion selon deux modes :
- **Mode Stockage (30 %)** : amener/maintenir le SoC à 30 % (conformité IATA DGR / UN 3480) ;  
- **Mode Plein (100 %)** : recharge complète pour tests et exploitation.

Chaque pack possède son **chargeur CC/CV (BQ24610)** et son **fuel‑gauge (MAX17263)**.  
La carte maître contrôle l’alimentation **12 V**, supervise l’état, et communique par **BLE**.

**Entrée** : 12 V / 12 A DC  **Connectique baie** : 2×6 pogo pins (12 V, GND, SDA, SCL, DET, réservé)

---

## ⚙️ 2 — Architecture du Système

```
        ┌───────────────────────────────┐
        │ PSU 12 V / 12 A (150 W)      │
        └──────────┬────────────────────┘
                   │  (Fuse 15 A + TVS + NTC)
        ┌──────────┴──────────┐
        │ Bus 12 V commun    │
        └┬─┬─┬─┬─┬─┬────────┘
         │ │ │ │ │ │  (6 baies)
         ▼ ▼ ▼ ▼ ▼ ▼
     ┌────────────────────┐
     │ P‑MOS AO4407A ×6  │ ← commande grille via N‑MOS DMG2302UX
     └────────────────────┘
         │   │   │   │   │
         ▼   ▼   ▼   ▼   ▼
   ┌──────────────────────────────┐
   │ Pogo pins (2×6) → Packs 2S  │
   │  (MAX17263 + BQ24610)       │
   └──────────────────────────────┘

             │  (Bus I²C racine 3V3)
      ┌──────┴────────────────────────────┐
      │ STM32L031K6 (MCU)                │
      │  ├─ TCA9548A (MUX I²C 8 ch)      │
      │  │    ├─ TCA9803×6 → Packs       │
      │  │    └─ PCA9634 → LED status    │
      │  ├─ ADC → 12 V Monitor           │
      │  ├─ UART → HC‑08 (BLE)           │
      │  └─ GPIO → MOSFETs / Ventilateur │
      └───────────────────────────────────┘
```

---

## 🔋 3 — Référence Batterie : Excel 2EXL1505

| Paramètre | Valeur | Commentaire |
|---|---:|---|
| Type | Li‑ion 2S (2 × 18650) | Cellules équivalentes NCR18650B |
| Tension nominale | 7.2 V | 3.6 V × 2 |
| Tension max | 8.4 V | Fin CV du BQ24610 |
| Capacité | 3.3 Ah | 23.76 Wh (étiquette) |
| Courant charge | 1.6 A (0.5 C) | CC/CV standard |
| Protection interne | Fusible 2 A + PTC + NTC 10 k | Double sécurité intégrée |
| Température charge | 0 → 45 °C |  |
| Température décharge | –20 → 60 °C |  |

**Remarque :** Le MAX17263 peut utiliser un profil NCR18650B pour un SoC précis **sans recalibration** (ModelGauge m5).

---

## 🧮 4 — Dimensionnement & Formules

**Puissance d’entrée** (buck interne BQ24610) :  
\[
I_{IN} = \frac{V_{BAT} \times I_{BAT}}{\eta \times V_{IN}}
\]

Exemple : 7.4 V × 1.6 A / (0.9 × 12 V) ≈ **1.1 A** par baie → 6 baies ≈ **6.6–7.3 A** totaux (avec logique).

### 4.1 — Dimensionnement de piste **3 mm** (IPC‑2152, cuivre 1 oz, couche externe)
- **Section** : 3.0 mm × 35 µm = **0.105 mm²**.  
- **Résistance linéique** (ρCu = 1.724×10⁻⁸ Ω·m) sur **10 cm** :  
  \(R = \frac{\rho L}{A} = \frac{1.724\times10^{-8} \times 0.1}{0.105\times10^{-6}} \approx 16.4\,mΩ\).  
- **Chute de tension** à **8 A** (plein débit bus) sur 10 cm : **≈ 0.13 V**.  
- **Perte Joule** : P = I²R ≈ **8² × 0.0164 ≈ 1.05 W** (répartie, acceptable avec convection + plan GND).  
- **Élévation thermique** visée : **ΔT ≤ 20 °C** → **3 mm** sur bus principal + **pours de cuivre** + **via‑stitching**.  
- **Branches** par baie : **≥ 1.0–1.2 mm** (I_baie ≈ 1.1–1.3 A, ΔT < 10 °C).  
- **Bonnes pratiques** : coins 45°, éviter étranglements, pas de « spokes » thermiques sur le bus 12 V.

### 4.2 — Courant d’entrée global
- I_total ≈ 6 × 1.1 A + logique 0.2–0.4 A = **6.8–7.3 A**.  
- PSU **12 V / 12 A** offre une **marge > 50 %** (transitoires, ventilateur).

---

## 🧠 5 — Logique Firmware (Machine à états finis)

```
              ┌────────────┐
              │   IDLE     │←────────────┐
              │ (aucun pack)│            │
              └──────┬─────┘            │
                     │                  │
             Pack détecté               │
                     ▼                  │
               ┌────────────┐           │
               │   CHECK    │           │
               │ Lecture SoC│           │
               └──────┬─────┘           │
         SoC<seuil    │   défaut I²C    │
                     ▼                  │
             ┌──────────────┐           │
             │  CHARGING    │───────────┘
             │ 12 V ON→pack │
             └──────┬───────┘
           SoC≥cible│  T°>45 °C
                     ▼
             ┌──────────────┐
             │   STABLE     │
             │ (30 % ou 100 %)│
             └──────┬───────┘
           défaut → │
                     ▼
             ┌──────────────┐
             │   FAULT      │
             └──────────────┘
```

**Principes clés :**
- Boucle **non bloquante** (Systick 2 s) + **IWDG/WWDG**.  
- **I²C** via TCA9548A + TCA9803 ; **recovery** si bus collé (reset canal + OFF/ON baie 500 ms).  
- **LEDs PCA9634** : PWM 5–10 mA/canal ; mapping ci‑dessous.  
- **Mode 100 %** : **12 V reste appliqué** ; fin de charge gérée par **BQ24610 (CC→CV→cut‑off)**.  
- **Mode 30 %** : coupure 12 V à **30.5 %** (hystérésis 2–3 %).

### 5.1 — Trames BLE (télémétrie)
- **TX** (5 s) : `B<n>;MODE=<0|1>;SOC=<%>;T=<°C>;U=<V>;VPSU=<V>;S=<OK|FLT>`  
- **RX** (debug) : `SET MODE <0|1>`, `SCAN`, `RST BAY <n>`

### 5.2 — Adressage I²C et MUX
| Élément | Adresse I²C | Remarque |
|---|---|---|
| PCA9634 | 0x60–0x67 (config) | 1 puce partagée |
| TCA9548A | 0x70 (par défaut) | 8 canaux, 6 utilisés |
| MAX17263 | 0x36 (par pack) | Vu via canal MUX + TCA9803 |

### 5.3 — Mapping LEDs (par baie)
| État | Couleur | Pattern |
|---|---|---|
| IDLE | OFF | — |
| CHECK | Bleu | blink 1 Hz |
| CHARGING | Jaune | fixe |
| STABLE‑30 | Vert | pulse 0.5 Hz |
| STABLE‑100 | Vert | fixe |
| FAULT | Rouge | blink 2 Hz |

---

## 🔎 6 — Surveillance ADC 12 V

**Pont diviseur** : 100 k / 20 k → ≈ 2.0 V @ 12 V.  
**Lecture ADC** → filtrage médian (5 échantillons). **Seuils & actions** :

| Condition | Seuil | Action firmware |
|---|---:|---|
| **PSU basse** | < 9.5 V (3 s) | Coupure toutes baies + log fault |
| **PSU haute** | > 13.2 V | Alerte BLE + désactivation fan |
| **Nominal** | 11.5–12.5 V | Fonctionnement normal |

---

## 🛡️ 7 — Sécurité et Fiabilité

| Fonction | Description |
|---|---|
| **Double Watchdog (IWDG + WWDG)** | Redémarrage auto en cas de blocage firmware |
| **Supervisor TPS3809K33** | Reset MCU si VCC < 3.0 V ou chute PSU |
| **Protection interne pack** | **Fusible 2 A + PTC interne + NTC 10 k** (Excel 2EXL1505) |
| **Protection externe baie** | **PTC réarmable 2.5 A** + **TVS SMBJ12A** + **zener 12–15 V G‑S** + **Cdec 10 µF** |
| **TCA9803 Pull‑down 10 kΩ (B‑side)** | Évite bus I²C flottant si pack débranché |
| **Surveillance ADC 12 V** | Pont 100 k/20 k → seuils 9.5 / 13.2 V ; actions firmware |
| **Thermal vias MOSFETs** | ΔT < 15 °C @ 1 A ; zones cuivre étendues |
| **ESD** | Réseaux ESD 4 lignes proches pogo ; châssis à GND |
| **Event logging Flash** | Historique SoC/T°/Fault (32 événements FIFO) |

> **Anti-redondance :** le pack Excel intègre déjà **fusible + PTC** ; la carte ajoute une **seconde barrière** (PTC externe + TVS) contre défauts **amont** (bus 12 V, erreurs d’assemblage, hot‑plug).

---

## 📈 8 — Profil de Charge (CC/CV)

```
Courant (A)
││ │  \______              ← Phase CV : courant chute
│         \___
│_____________ Temps →
```

**Phases :**  
- **CC** : courant constant jusqu’à ≈ 4.15 V / cellule ;  
- **CV** : tension constante **8.4 V** (pack 2S), courant → **~0.06 A** ;  
- **Fin charge** : courant < **65 mA** → arrêt automatique du **BQ24610**.

Temps CC approximatif entre SOC₁→SOC₂ :  
\[
t_{CC} \approx \frac{C_{eff} \times (SOC_2 - SOC_1)}{I_{BAT}}
\]  
Exemples : **3.35 Ah** → 0→100 % ≈ **2.3 h** (+15–25 min CV) ; 20→30 % ≈ **~12 min**.

---

## 📦 9 — BOM (Détail Complet)

> **Références MPN indicatives** (équivalents acceptés) — préciser footprints avant routage.

### 9.1 — Entrée 12 V & Distribution
| Réf. | Élément | MPN (exemple) | Boîtier | Notes |
|---|---|---|---|---|
| F1 | Fusible temporisé 15 A | Bel Fuse 3SB 15A | 5×20 | Inrush protégé par NTC |
| RV1 | NTC inrush | 5D‑11 (MF72‑5D‑11) | Disc | Limite courant d’appel |
| D1 | TVS 600 W | SMBJ15A (Littelfuse) | SMBJ | Protection transitoires |
| Cbulk | Capacité entrée | 470–1000 µF / 25 V | Radial/SMT | ESR faible conseillé |
| Bus | Piste 12 V | — | — | **3 mm**, voir §4.1 |

### 9.2 — Commutation par baie (×6)
| Réf. | Élément | MPN (exemple) | Boîtier | Notes |
|---|---|---|---|---|
| QP | P‑MOS high‑side | AO4407A (AOS) | SO‑8 | R_DS(on) ~ 12 mΩ |
| QN | N‑MOS driver | DMG2302UX (Diodes Inc.) | SOT‑23 | Tire G→0 V |
| Rg | Résistance gate | 47–100 Ω | 0603 | Limite dV/dt |
| Rpu | Pull‑up gate→12 V | 100 kΩ | 0603 | OFF par défaut |
| Dz | Zener G‑S | BZT52 12–15 V | SOD‑123 | Clamp transitoires |
| PTC | Polyfuse baie | MF‑R250 (Bourns) | Radial/1206 reset | 2.5 A hold |
| DTVS | TVS locale | SMBJ12A | SMBJ | Proche pogo |
| Cdec | Découplage | 10 µF + 0.1 µF | 1206+0603 | À l’entrée baie |

### 9.3 — Logique, I²C & LEDs
| Réf. | Élément | MPN (exemple) | Boîtier | Notes |
|---|---|---|---|---|
| U1 | MCU | STM32L031K6T6 | LQFP‑32 | 3.3 V, basse conso |
| U2 | Supervisor | TPS3809K33DBVR | SOT‑23‑3 | Reset 3.0 V |
| U3 | MUX I²C | TCA9548APWR | TSSOP‑24 | Addr 0x70 |
| U4–U9 | Buffer I²C (×6) | TCA9803DGKR | VSSOP‑8 | 1/baie |
| U10 | LED driver | PCA9634PW | TSSOP‑28 | 16 canaux PWM |
| Rpull | Pull‑ups I²C | 2.2–4.7 kΩ | 0603 | Racine 3V3 |
| Rser | Séries SDA/SCL | 33–47 Ω | 0603 | Damping |
| Rext | Set courant LEDs | 3.9–6.8 kΩ | 0603 | 5–10 mA/LED |

### 9.4 — Connectique & Debug
| Réf. | Élément | MPN (exemple) | Boîtier | Notes |
|---|---|---|---|---|
| JBay | Pogo pins 2×6 | Mill‑Max 858‑XX‑XXX‑10‑XXX101 | — | Or dur, anti‑tilt |
| JSWD | Pads SWD | — | 3 pads | SWDIO, SWCLK, NRST |
| JUART | Pads UART | — | 3 pads | TX, RX, GND |

### 9.5 — Ventilation (option)
| Réf. | Élément | MPN (exemple) | Boîtier | Notes |
|---|---|---|---|---|
| FAN | Ventilateur 12 V | Sunon 40×40 | — | PWM via MCU |
| Qfan | MOSFET N | AO3400 | SOT‑23 | Low‑side fan |
| NTCch | NTC châssis | 10 k B3950 | 0603 | Mesure T° boîtier |

---

## 🧰 10 — Notes de Conception

- Découplage : 100 nF par VDD + 4.7 µF bulk près du MCU.  
- BOOT0 : strap GND via 100 kΩ ; NRST : C 100 nF.  
- Traces SDA/SCL courtes + R‑série 33–47 Ω.  
- GND plan continu + via‑stitching autour des MOSFETs.  
- Distance I²C ≤ 10 cm entre MUX et TCA9803.  
- Pistes MOSFETs larges ≥ 3 mm, zones cuivre autour PTC.

---

## 🚀 11 — Étapes Suivantes

1. Validation thermique (6× 1.6 A / 8.4 V).  
2. Calibration ADC PSU + supervisor TPS3809.  
3. Tests I²C multi‑baies + reconnexion à chaud.  
4. Intégration firmware BLE / LEDs / logs Flash.  
5. Finalisation PCB production (2 couches ou 4 avec fan optionnel).

---

© 2025 EOS Positioning Systems — R&D Skadi Project
