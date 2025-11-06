# ⚡ Skadi 6-Bay Smart Charger — v1.0 (Final Engineering Revision)


> Version consolidée matérielle + logicielle intégrant la logique firmware finale, la sécurité complète, et les spécifications de production.
> Ce projet open hardware vise à fournir une plateforme de charge intelligente pour 6 packs Li-ion 2S (Excel 2EXL1505).
> 🔗 **Documentation complète :** [hardware/](./hardware) • [firmware/](./firmware) • [docs/](./docs)

![EOS Logo](./docs/eos_logo.png)

---

## 🧭 1 — Présentation du Projet

Ce chargeur intelligent à **6 baies** permet de gérer automatiquement la charge des batteries Skadi 2S Li-ion selon deux modes :

* **Mode Stockage (30 %)** : ramener ou maintenir le SoC à 30 % pour conformité IATA DGR / UN 3480 ;
* **Mode Plein (100 %)** : recharge complète pour tests et exploitation ;

Chaque pack intègre son **chargeur CC/CV (BQ24610)** et son **fuel-gauge (MAX17263)**.
La carte maître contrôle l’alimentation 12 V, supervise l’état, et communique par BLE.

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
     │ P-MOS AO4407A ×6  │ ← commande grille via N-MOS DMG2302UX
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
      │  ├─ UART → HC-08 (BLE)           │
      │  └─ GPIO → MOSFETs / Ventilateur │
      └───────────────────────────────────┘
```

🔗 Voir le schéma détaillé dans [`docs/system_architecture.svg`](./docs/system_architecture.svg)

---

## 🔋 3 — Référence Batterie : Excel 2EXL1505

| Paramètre            | Valeur                          | Commentaire                |
| -------------------- | ------------------------------- | -------------------------- |
| Type                 | Li-ion 2S (2 × 18650 NCR18650B) | Cellules Panasonic 3.35 Ah |
| Tension nominale     | 7.2 V                           | 3.6 V × 2                  |
| Tension max          | 8.4 V                           | Fin CV du BQ24610          |
| Capacité             | 3.3 Ah                          | 23.76 Wh (étiquette)       |
| Courant charge       | 1.6 A (0.5 C)                   | Typique CC/CV              |
| Protection interne   | Fusible 2 A + PTC + NTC 10 k    | Double sécurité            |
| Température charge   | 0 → 45 °C                       |                            |
| Température décharge | –20 → 60 °C                     |                            |

**Remarque :** La LUT du MAX17263 utilise le profil NCR18650B → SoC précis sans recalibration.

---

## 🧮 4 — Dimensionnement & Formules

[
I_{IN} = \frac{V_{BAT} × I_{BAT}}{η × V_{IN}}
]

Exemple : 7.4 V × 1.6 A / (0.9 × 12 V) ≈ 1.1 A par baie → 6 baies ≈ 7 A totaux.

**Bus 12 V :** pistes 3 mm (ΔT ≤ 20 °C) ; branches ≥ 1 mm ; norme IPC-2152.

---

## 🧠 5 — Logique Firmware (Machine à états finis)

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
               │ Lecture SoC│          │
               └──────┬─────┘          │
         SoC<seuil    │   défaut I²C   │
                     ▼                 │
             ┌──────────────┐          │
             │  CHARGING    │──────────┘
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

---
# 9 📦 BOM détaillée — Skadi 6-Bay Smart Charger v4.5  

> **Références MPN indicatives** (équivalents acceptés)  /  mais faut préciser les footprints avant le routage.  
>  
> 

---

## 🔗 Sommaire  
- [9.1 — Entrée 12 V & Distribution](#91--entrée-12v--distribution)  
- [9.2 — Commutation par baie (×6)](#92--commutation-par-baie-×6)  
- [9.3 — Logique, I²C & LEDs](#93--logique-i²c--leds)  
- [9.4 — Connectique & Debug](#94--connectique--debug)  
- [9.5 — Ventilation (option)](#95--ventilation-option)  

---

## 9.1 — Entrée 12 V & Distribution  

| Section | Élément | Référence / MPN | Boîtier | Qté | Notes |
|----------|----------|-----------------|----------|-----|-------|
| Entrée 12 V | Fusible temporisé 15 A | Bel Fuse 3SB 15A | 5×20 mm | 1 | Protection principale en entrée, limite le courant d’appel |
| Entrée 12 V | NTC inrush | MF72-5D-11 | Disc | 1 | Limite le courant d’appel au démarrage |
| Entrée 12 V | TVS 600 W | SMBJ15A (Littelfuse) | SMBJ | 1 | Protection contre surtensions transitoires |
| Entrée 12 V | Condensateur électrolytique | 470 µF – 1000 µF / 25 V | Radial/SMT | 2 | Filtrage et stabilité d’alimentation |
| Entrée 12 V | Condensateur céramique | 0.1 µF | 0603 | 2 | Découplage haute fréquence |
| Entrée 12 V | Piste bus 12 V | — | — | — | Largeur 3 mm, cuivre 1 oz, ΔT ≤ 20 °C (voir § 4.1) |

---

## 9.2 — Commutation par baie (×6)  

| Section | Élément | Référence / MPN | Boîtier | Qté | Notes |
|----------|----------|-----------------|----------|-----|-------|
| Baie | P-MOSFET High-Side | AO4407A (AOS) | SO-8 | 6 | R_DS(on) ≈ 12 mΩ, commande du 12 V par baie |
| Baie | N-MOS driver | DMG2302UX (Diodes Inc.) | SOT-23 | 6 | Pilote la grille du P-MOS (pull-down) |
| Baie | Résistance gate | 47 Ω – 100 Ω | 0603 | 6 | Limite le dV/dt sur la grille |
| Baie | Pull-up gate | 100 kΩ | 0603 | 6 | Maintient OFF par défaut |
| Baie | Diode zener G-S | BZT52 (12–15 V) | SOD-123 | 6 | Clamp transitoires grille-source |
| Baie | Polyfuse réarmable | MF-R250 (Bourns) | 1206 / Radial | 6 | Protection 2.5 A hold, 5 A trip |
| Baie | TVS locale | SMBJ12A (Littelfuse) | SMBJ | 6 | Protection ligne 12 V locale |
| Baie | Condensateur découplage | 10 µF + 0.1 µF | 1206 + 0603 | 12 | Filtrage local de baie |

---

## 9.3 — Logique, I²C & LEDs  

| Section | Élément | Référence / MPN | Boîtier | Qté | Notes |
|----------|----------|-----------------|----------|-----|-------|
| Logique | Microcontrôleur | STM32L031K6T6 (STMicroelectronics) | LQFP-32 | 1 | Cœur du système, basse consommation |
| Logique | Superviseur tension | TPS3809 (TI) | SOT-23-3 | 1 | Reset 3.0 V, sécurité brown-out |
| Logique | Multiplexeur I²C | TCA9548A (TI) | TSSOP-24 | 1 | 8 canaux, 6 utilisés |
| Logique | Buffer I²C | TCA9803 (TI) | VSSOP-8 | 6 | Interface basse tension / pack |
| LEDs | Driver LED PWM | PCA9634 (NXP) | TSSOP-28 | 1 | 16 canaux PWM, RGB/états |
| I²C | Résistance pull-up SDA/SCL | 2.2 kΩ – 4.7 kΩ | 0603 | 4 | Ligne principale et secondaire |
| I²C | Résistance série SDA/SCL | 33 Ω – 47 Ω | 0603 | 4 | Protection et amortissement |
| LEDs | Résistance réglage courant | 3.9 kΩ – 6.8 kΩ | 0603 | 1 | Fixe courant de sortie LED (≈ 10 mA) |
| Sécurité | Réseau ESD 4 lignes | PESD5V0S4 (Nexperia) | SOT-23-6 | 2 | Protection bus I²C et connecteurs |

---

## 9.4 — Connectique & Debug  

| Section | Élément | Référence / MPN | Boîtier | Qté | Notes |
|----------|----------|-----------------|----------|-----|-------|
| Connectique | Pogo pins 2×6 | Mill-Max 858-XX-XXX-10-XXX101 | — | 6 | Connecteurs dorés, tolérance faible |
| Debug | Pads SWD | — | — | 1 | Interface de programmation (SWDIO, SWCLK, NRST) |
| Debug | Pads UART | — | — | 1 | Interface série (TX, RX, GND) |
| Connectique | Broches test GND/VCC | — | 2 mm Header | 1 | Points de mesure tension/alim |

---

## 9.5 — Ventilation (option)  

| Section | Élément | Référence / MPN | Boîtier | Qté | Notes |
|----------|----------|-----------------|----------|-----|-------|
| Ventilation | Ventilateur 12 V | Sunon MB40201VX-000U-A99 | 40×40×10 mm | 1 | Commande PWM par MCU |
| Ventilation | MOSFET commande fan | AO3400 (AOS) | SOT-23 | 1 | Commutation low-side ventilateur |
| Capteur thermique | NTC châssis | 10 k B3950 (Vishay) | 0603 | 1 | Surveillance T° interne boîtier |



---

## 🧰 10 — Notes de Conception

* Découplage : 100 nF par VDD + 4.7 µF bulk près du MCU.
* BOOT0 : strap GND via 100 kΩ ; NRST : C 100 nF.
* Traces SDA/SCL courtes + R-série 33–47 Ω.
* GND plan continu + via-stitching autour des MOSFETs.
* Distance I²C ≤ 10 cm entre MUX et TCA9803.
* Pistes MOSFETs larges ≥ 3 mm, zones cuivre autour PTC.

---

## 📂 Structure du Dépôt

```
Skadi-6Bay-Charger/
├── hardware/           # Schémas, PCB KiCad
├── firmware/           # Code STM32 + BLE
├── docs/               # Images, graphiques, datasheets
├── README.md           # Présent document
└── LICENSE             # Licence projet
```

---

## 🚀 11 — Étapes Suivantes

1. Validation 
2. Creation et Finalisation schematic + PCB
3. passer au prototypage

---

© 2025 EOS Positioning Systems — 
