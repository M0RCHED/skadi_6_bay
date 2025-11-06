
# ⚡ Skadi 6‑Bay Smart Charger — v4.1

> **Firmware‑Ready Hardware Documentation**  
> This document describes the hardware architecture, logic, and electrical design of the **6‑bay intelligent charger** for **Skadi 2S Li‑ion (7.2 V / 3.3 Ah)** battery packs, featuring an embedded BQ24610 charger and MAX17263 fuel‑gauge.

---

## 🧭 Project Overview

Due to transport regulations (**IATA DGR / UN 3480**), lithium‑ion batteries must be shipped at **≈ 30 % SoC** to ensure thermal safety.  
This system allows automatic handling of **6 battery bays** to:
- Safely **charge packs below 30 %** to the 30 % or 100 % target.  
- **Initialize the MAX17263** fuel‑gauge LUT and ensure correct SoC reporting.  
- Optionally, discharge packs > 30 % (future firmware feature).  
- Provide **per‑bay isolation**, **electrical safety**, and **status LEDs**.

**Power Input :** 12 V / 12 A DC  
**Battery Interface :** 2 × 6 pogo‑pin connector per bay (12 V, GND, SDA, SCL, DET, reserved)

---

## 🧩 System Architecture

> 🖼️ *A color system diagram will be included here in the GitHub version.*  
> The following summary reflects the latest design (with TCA9803 buffers + PCA9634 LED driver).

```
12V / 12A PSU
   │
   ├── High‑side P‑MOSFETs × 6  → Bays 1‑6
   │
   └── Buck 12→3.3V → MCU ( STM32L031K6 )
                │
                ├── I²C Root Bus ( PB6/PB7 )
                │      ├─ TCA9548A (MUX)
                │      │     ├─ Ch 1 → TCA9803 → Bay 1 (MAX17263/BQ24610)
                │      │     ├─ Ch 2 → TCA9803 → Bay 2
                │      │     ├─ …
                │      │     └─ Ch 6 → TCA9803 → Bay 6
                │      └─ PCA9634 → LED Indicators (1–2 LEDs per bay)
                │
                ├── UART ( PA9/PA10 ) → BLE Module (HC‑08)
                └── GPIOs PB0–PB5 → N‑MOS drivers → P‑MOS high‑side
```

---

## ⚙️ Main Components

| Function | Component | Package | Notes |
|-----------|------------|----------|-------|
| MCU | **STM32L031K6** | LQFP‑32 | Low‑power Cortex‑M0+, 3.3 V logic |
| I²C Multiplexer | **TCA9548A** | TSSOP‑24 | 8 channels, 3.3 V logic |
| I²C Buffers (per bay) | **TCA9803 × 6** | SOT‑23‑8 | Level translation + rise‑time acceleration |
| LED Driver | **PCA9634** | TSSOP‑28 | 16‑channel current sink, I²C 3.3 V |
| Charger IC (in pack) | **BQ24610** | — | Constant Current/Voltage (CC/CV) Li‑ion charger |
| Fuel Gauge (in pack) | **MAX17263** | — | ModelGauge m5, SoC / Temp / Volt / Cap |
| Power MOSFETs | AO4407A (P‑MOS) + 2N7002 (N‑MOS) | SO‑8 + SOT‑23 | 12 V high‑side switch control |
| Protection | PTC 2–2.5 A, TVS SMBJ10–15A | — | Per‑bay protection |
| PSU | External 12 V / 12 A | — | Shared input bus |

---

## 🔌 Power and Current Calculations

The battery internal BQ24610 controls CC/CV.  
\(
I_{IN} ≈ \frac{V_{BAT}·I_{BAT}}{η·V_{IN}}
\)

Example: \( V_{BAT}=7.4 V \), \( I_{BAT}=1.6 A \), \( η=0.9 \), \( V_{IN}=12 V \)  
→ \( I_{IN} ≈ 1.1 A \) per bay → ≈ 6.6 A for 6 bays + logic.

### Track Width 12 V Bus
- Copper 1 oz (35 µm), ΔT ≤ 20 °C → **3 mm bus**, 1 mm branches min.  
- Apply IPC‑2152 guidelines + via‑stitching.

---

## 🧠 Charge Logic (Firmware Perspective)

| Mode | Condition Start | Condition Stop | Target SoC | Temp Limit |
|-------|----------------|----------------|-------------|------------|
| **Storage Mode** | SoC < 28 % | SoC ≥ 30.5 % | 30 % | < 45 °C |
| **Full Mode** | SoC < 99 % | SoC ≥ 99.5 % | 100 % | < 45 °C |

Safety events → Cut MOSFET (HW) + Reset I²C bus if fault detected.

---

## 🧰 Design Notes

- **Decoupling :** 100 nF per VDD + 4.7 µF bulk near MCU.  
- **BOOT0 :** pulled to GND via 100 kΩ (resistor strap).  
- **NRST :** no button, SWD reset only (+ 100 nF cap).  
- **TCA9803** close to pogo pins (reduce B‑side loop).  
- **ESD arrays** near battery connectors.  
- Keep GND plane continuous / stitch vias around buffers and MOSFETs.

---

## 📦 BOM (Excerpt / Grouped)

| Section | Components | Qty | Notes |
|----------|-------------|-----|-------|
| Input 12 V | PSU 12 V 12 A, Fusible 15 A (T), TVS SMBJ15A, NTC 5D‑11, Bulk 470–1000 µF | 1 | Common supply |
| Power switch per bay | P‑MOS AO4407A, N‑MOS 2N7002, R_g 47–100 Ω, Pull‑up 100 kΩ → 12 V, Zener 12–15 V G‑S, PTC 2–2.5 A, TVS SMBJ10–12A, Cdec 10 µF + 0.1 µF | 6 | High‑side stage |
| Logic & I²C | STM32L031K6, TCA9548A, TCA9803 × 6, PCA9634, R_pull‑ups 2.2–4.7 kΩ, R_series 33–47 Ω, ESD array | 1 set | Core logic |
| Connectors | 2 × 6 pogo pins per bay | 6 | Battery interface |

---

## 🧮 Formulas Reference

- \( t_{CC} ≈ \frac{C_{eff}(SOC_2 − SOC_1)}{I_{BAT}} \)  
- Example : \( C=3.3 Ah \), 0→100 % → ~2.06 h + CV ≈ 2.3–2.6 h per pack (typ.)

---

## 🚀 Next Steps

- Firmware: implement **MAX17263 init LUT**, SoC/Temp monitoring via I²C.  
- Integrate “storage/full” modes with MOSFET control.  
- Add diagnostics (LED patterns via PCA9634) and auto‑bus recovery.

---

© 2025 EOS Positioning Systems — Internal development project  
