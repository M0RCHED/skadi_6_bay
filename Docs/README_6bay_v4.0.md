
# 6‑Bay Smart Charger — v4.0 (Document de conception technique)

## 🧭 Origine et Objectif du Projet
Le présent document décrit la conception d’un **chargeur intelligent multi‑baies (6‑bay)** destiné aux **batteries Skadi 2S Li‑ion (7.2 V / 3.3 Ah)** équipées d’une carte interne avec **BQ24610** (chargeur CC/CV) et **MAX17263** (fuel‑gauge).  
Contexte réglementaire : les normes **IATA DGR / UN 3480** exigent un **SOC ≈ 30 %** pour l’expédition des batteries Li‑ion. Objectifs : **initialiser** correctement le fuel‑gauge (LUT / Model), **recharger** vers **30 %** ou **100 %**, et (futur) **décharger** vers 30 %, avec **traçabilité SoC** et **sécurité électrique**.  
**Ventilation : optionnelle** (prévue pour usage intensif / environnement chaud).

> **Scope.** 6 baies, alimentation **12 V / 12 A**, connecteurs par baie **2 × 6 pogo pins**.  
> Notre carte alimente/coupe le 12 V de chaque baie, lit le SoC via **I²C** (TCA9548A), gère les modes **30 %** / **100 %**.

---

## 1) Architecture système
- Entrée 12 V → protections (fusible, TVS, NTC inrush) → bus 12 V.  
- Par baie (×6) : **P‑MOS high‑side** + PTC + TVS + Cdec local.  
- Logique : **STM32L031K6 (32 pins)** + **TCA9548A** (MUX I²C) + **TLC59116** (driver LED I²C, courant constant).  
- Firmware : lecture **MAX17263** (SoC, Temp, Volt), initialisation **LUT**, modes 30 % / 100 %.

Schéma bloc (texte) :
```
PSU 12V → F1 → TVS → Bulk → ──┬── QHS1 → Bay1 (2×6 pogo)
                               ├── QHS2 → Bay2
                               ├── ...
                               └── QHS6 → Bay6

12V → Buck 3V3 → STM32L031K6 ── I²C ── TCA9548A ──┬─ Bay1 (MAX17263)
                                                  ├─ Bay2
                                                  └─ ...
                                     └── TLC59116 (LEDs)
```

---

## 2) Alimentation & dimensionnement
Formule entrée/sortie (buck interne BQ) :  
\[
I_{IN} \approx \frac{V_{BAT} \cdot I_{BAT}}{\eta \cdot V_{IN}}
\]  
Ex. (7.4 V, 1.6 A, η=0.9, 12 V) → **~1.1 A** par baie. 6 baies → typiquement **6.2–8.3 A** (avec logique).

---

## 3) Logique de charge (firmware)
**Mode 30 %** : si `SOC < 28 %` et `T_pack < 45 °C` → ON jusqu’à `SOC ≥ 30.5 %`, puis OFF.  
**Mode 100 %** : si `SOC < 99 %` et `T_pack < 45 °C` → ON jusqu’à fin CV (≈99.5 %), puis OFF.  
Sécurités : défaut I²C/MUX (reset), PTC déclenchée, débranchement à chaud, T_pack ≥ 45 °C.

---

## 4) Courbe de charge (CC/CV) & formules
- Profil pack (BQ24610) : pré‑charge → **CC** (I fixé par **R_ISET**) → **CV** (8.4 V) → terminaison.  
- Temps CC approx. entre SOC₁→SOC₂ :  
\[
t_{CC} \approx \frac{C_{eff} \cdot (SOC_2 - SOC_1)}{I_{BAT}}
\]  
Ex. 0→100 % : 3.3/1.6 ≈ 2.06 h + CV (≈15–30 min) → **~2.3–2.6 h**.

---

## 5) Largeur de piste 12 V (3 mm) — justification
Cuivre 1 oz (35 µm), ΔT cible ≈10–20 °C, I_bus ≤ ~10 A.  
Pratique IPC‑2152 (pistes externes + convection) → **3 mm** sur tronc + **pours cuivre** + **via‑stitching**. Branches par baie **≥1.0–1.2 mm**.

---

## 6) MCU : STM32L031K6 (32 pins) — implantation minimale
- **Découplage** : 100 nF **par VDD** + 4.7 µF bulk près du MCU.  
- **BOOT0** : strap **GND** via 100 kΩ. **NRST** : pas de bouton (SWD suffit), C 100 nF conseillé.  
- **SWD** : 2×5 (SWDIO, SWCLK, NRST, 3V3, GND). Horloge : HSI par défaut.

---

## 7) MUX I²C : TCA9548A — règles
- **VCC 3.3 V**, découp. 100 nF + 1 µF. A0–A2 : adresses (GND/VCC). RESET (actif bas) au MCU.  
- **Pull‑ups I²C** (maître &/ou canaux).  
  - \(R_{p(min)} = \frac{V_{DPUX} - V_{OL(max)}}{I_{OL}}\) ;  
  - \(R_{p(max)} = \frac{t_r}{0.8473 \cdot C_b}\) (Fast‑mode, \(t_r=300\) ns, \(C_b \le 400\) pF).  
- Traces SDA/SCL courtes ; R‑série 33–47 Ω ; réseau ESD près des connecteurs.

---

## 8) **Pin‑planning & choix “driver LED + commande MOSFET”** ✅ *(intégré)*
### A) Pin‑planning (besoins & mappage proposé)
- **I²C1** (TCA9548A + **TLC59116**) → **PB6=SCL, PB7=SDA**  
- **UART BLE** → **PA9=TX, PA10=RX**  
- **DIP 2 pos.** → **PA0, PA1** (entrées + pull‑ups)  
- **Fan PWM** (option) → **PA8** ; **NTC châssis** (option) → **PA4 (ADC)**  
- **Baies 1..6 CTRL** (via N‑MOS → P‑MOS high‑side) → **PB0..PB5**  
- **SWD** → **PA13/PA14** ; **LED statut** → **PC13**  

### B) Driver LED I²C
- **TLC59116** (**courant constant**, TSSOP‑28) sur **bus I²C racine** (zéro GPIO supplémentaire).  
- Alloue 1–2 LEDs par baie + 1 LED globale (≤16 canaux → une puce suffit).

### C) Commande P‑MOS high‑side propre (niveau 12 V)
- Pull‑up grille **au 12 V** (≈100 kΩ), **N‑MOS logique** (**2N7002/AO3400**) pour tirer G→0 V (ON).  
- **R_g** 47–100 Ω, **zener 12–15 V** (G‑S), **pulldown** éventuel sur N‑MOS gate.

### D) PCB : 2 couches vs 4 couches
- **2 couches** (1 oz, top‑side assembly JLCPCB) **suffisent** avec pours cuivre + via‑stitching.  
- **4 couches** si BLE embarqué RF/EMC exigeant ou densité très élevée.

### E) BOM — ajouts liés
- **TLC59116** + résistances LED si besoin de repères,  
- **2N7002/AO3400 ×6**, **R_g**, **pull‑up 100 kΩ**, **zener 12–15 V**.  
- **DIP 2 pos**, **Rpull‑ups I²C 2.2–4.7 kΩ**, **R‑série 33–47 Ω**, **ESD array**.

---

## 9) Sécurités & thermique
Par baie : **PTC 2–2.5 A**, TVS locale, découplage 10 µF + 100 nF.  
Entrée : fusible temporisé 15 A + TVS SMBJ15A + NTC inrush (option).  
Arrêt charge si **T_pack ≥ 45 °C**. **Ventilateur : optionnel.**

---

## 10) BOM (extrait ré‑espacé)
- **Entrée 12 V** : PSU 12 V/12 A • Bornier 2P 15 A • Fusible 15 A (T) • TVS SMBJ15A • NTC 5D‑11 • Bulk 470–1000 µF + 0.1 µF  
- **Baies (×6)** : P‑MOS SO‑8 (R_DS(on) < 15 mΩ) • **N‑MOS 2N7002/AO3400** • R_g 47–100 Ω • Pull‑up G→12 V 100 kΩ • Zener 12–15 V • **PTC 2–2.5 A** • TVS SMBJ10–12A • Cdec 10 µF + 0.1 µF • **JBay = 2 × 6 pogo pins**  
- **Logique** : Buck 12→3.3 V (MP1584/MP2307) • L1 10–22 µH • CIN/COUT 22–47 µF • **STM32L031K6** + 100 nF/VDD + 4.7 µF bulk • **TCA9548A** (100 nF + 1 µF) • **TLC59116** • Pull‑ups I²C 2.2–4.7 kΩ • R‑série SDA/SCL 33–47 Ω • ESD array 4 lignes • DIP 2 pos • LED + R • SWD 2×5  
- **Refroidissement (option)** : Fan 12 V • MOSFET N AO3400 • Cfan 100 µF • NTC châssis 10 k

---

## 11) Glossaire
TVS • PTC • NTC • R_DS(on) • SWD • PWM • ESR • (Fast‑mode) I²C • LUT (fuel‑gauge)

---

## 12) Prochaines étapes (suggestion)
- Firmware : **init LUT MAX17263**, télémétrie BLE (SoC/Temp).  
- Schéma PCB : intégration **TLC59116** + **2N7002** (x6) + protections.  
- Bring‑up : I²C (root) → MUX → MAX17263 → TLC59116 → contrôle baies.
