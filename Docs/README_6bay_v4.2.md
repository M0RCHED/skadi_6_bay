
# ⚡ Skadi 6‑Bay Smart Charger — v4.2

> **Production & Safety Revision**  
> This document describes the final hardware and firmware design of the 6‑bay intelligent charger for **Skadi 2S Li‑ion (7.2 V / 3.3 Ah)** battery packs, integrating embedded **BQ24610** charger ICs and **MAX17263** fuel‑gauges.  
> The design now includes production‑grade safety, reliability, and diagnostic features based on the internal review (Nov 2025).

---

## 🧭 Project Overview

Due to transport regulations (**IATA DGR / UN 3480**), lithium‑ion batteries must be shipped at **≈ 30 % SoC** for thermal safety.  
This charger automates the management of **6 bays** to:

- Safely **charge packs below 30 %** up to 30 % (storage) or 100 % (full mode)  
- **Initialize MAX17263** fuel‑gauges (LUT write + status check)  
- Ensure per‑bay electrical isolation and fault protection  
- Provide visual status feedback via LEDs driven by PCA9634

**Input Power :** 12 V / 12 A DC  
**Battery Interface :** 2×6 pogo pins per bay (12 V, GND, SDA, SCL, DET, reserved)

---

## 🧩 System Architecture

> 🖼️ *A color system diagram will be included here (I²C bus, 12 V bus, ADC monitor, supervisor, watchdog).*  

```
12 V / 12 A PSU
   │
   ├── High‑side P‑MOSFETs ×6 → Bays 1‑6
   │
   └── Buck 12→3.3 V → MCU (STM32L031K6)
                │
                ├── I²C Root Bus (PB6/PB7)
                │      ├─ TCA9548A (MUX)
                │      │     ├─ Ch1 → TCA9803 → Bay 1 (MAX17263 / BQ24610)
                │      │     ├─ Ch2 → TCA9803 → Bay 2
                │      │     ├─ …
                │      │     └─ Ch6 → TCA9803 → Bay 6
                │      └─ PCA9634 → LED Indicators (1‑2 LEDs / bay)
                │
                ├── UART (PA9/PA10) → BLE Module (HC‑08)
                ├── ADC Input → 12 V monitor (divider 100 k / 20 k)
                └── GPIOs PB0‑PB5 → N‑MOS drivers → P‑MOS high‑side
```

---

## ⚙️ Main Components

| Function | Component | Package | Notes |
|-----------|------------|----------|-------|
| MCU | **STM32L031K6** | LQFP‑32 | Low‑power Cortex‑M0+, 3.3 V logic |
| Supervisor | **TPS3809K33** | SOT‑23‑3 | Reset on undervoltage (BOD support) |
| I²C Multiplexer | **TCA9548A** | TSSOP‑24 | 8 channels, 3.3 V logic |
| I²C Buffers (per bay) | **TCA9803 ×6** | SOT‑23‑8 | Level translation + rise‑time acceleration |
| LED Driver | **PCA9634** | TSSOP‑28 | 16‑channel current sink I²C 3.3 V |
| Charger (in pack) | **BQ24610** | — | CC/CV Li‑ion charger |
| Fuel Gauge (in pack) | **MAX17263** | — | ModelGauge m5, SoC / Temp / Volt / Cap |
| Power MOSFETs | **AO4407A (P‑MOS)** + **DMG2302UX (N‑MOS)** | SO‑8 + SOT‑23 | 12 V high‑side switch control |
| Protection | PTC 2–2.5 A, TVS SMBJ10–15A | — | Per‑bay safety |
| PSU | External 12 V / 12 A | — | Shared input bus |

---

## 🛡️ Safety and Reliability Features

| Feature | Description |
|----------|--------------|
| **Dual Watchdog (IWDG + WWDG)** | MCU self‑recovery on firmware freeze |
| **Brown‑Out + Supervisor (TPS3809)** | Reset MCU below 3.0 V or PSU drop |
| **TCA9803 Pull‑downs 10 kΩ (B‑side)** | Prevent I²C floating on battery disconnect |
| **12 V ADC Monitor** | Detect PSU fault / overcurrent |
| **Thermal Vias under MOSFETs** | Better heat dissipation |
| **SWD + UART Pads** | Factory flashing + debug access |
| **Timeouts (2 h / 4 h)** | Auto cutoff per mode (storage/full) |
| **Event Logging in Flash** | Store bay faults / runtime data |

---

## 🔌 Power and Current Calculations

The internal BQ24610 handles CC/CV charging.  
\(
I_{IN} ≈ \frac{V_{BAT} × I_{BAT}}{η × V_{IN}}
\)

Example: \( V_{BAT}=7.4 V \), \( I_{BAT}=1.6 A \), \( η=0.9 \), \( V_{IN}=12 V \)  
→ \( I_{IN}≈1.1 A \) per bay → ≈ 6.6 A for 6 bays + logic.  

**Track Width:** 3 mm bus + 1 mm branches (1 oz Cu, ΔT ≤ 20 °C). Follow IPC‑2152.

---

## 🧠 Firmware Logic (Simplified)

| Mode | Start Condition | Stop Condition | Target SoC | Timeout |
|-------|----------------|----------------|-------------|----------|
| **Storage Mode** | SoC < 28 % | SoC ≥ 30.5 % | 30 % | 2 h |
| **Full Mode** | SoC < 99 % | SoC ≥ 99.5 % | 100 % | 4 h |

**Core behaviors:**  
- I²C transactions via TCA9548A + TCA9803 buffers  
- PCA9634 drives status LEDs (per bay)  
- Non‑blocking loop (Systick) + watchdogs  
- I²C recovery routine (OFF → 500 ms → ON per bay)  
- BLE authentication + message rate‑limit  

---

## 🧰 Design Notes

- **Decoupling:** 100 nF per VDD + 4.7 µF bulk near MCU  
- **BOOT0:** GND via 100 kΩ strap  
- **NRST:** SWD reset only (+ 100 nF cap)  
- **TCA9803:** close to pogo pins (reduce loop)  
- **Pull‑downs 10 kΩ:** on each B‑side of TCA9803  
- **Thermal vias:** under MOSFETs AO4407A  
- **R_EXT (PCA9634):** set for 5‑10 mA/LED  
- **ADC divider:** 100 k / 20 k (≈ 2 V @ 12 V input)  

---

## 📦 BOM (Excerpt)

| Section | Components | Qty | Notes |
|----------|-------------|-----|-------|
| Input 12 V | PSU 12 V 12 A, Fuse 15 A (T), TVS SMBJ15A, NTC 5D‑11, Bulk 470–1000 µF | 1 | Common input |
| Power switch per bay | P‑MOS AO4407A, N‑MOS DMG2302UX, R_g 47–100 Ω, Pull‑up 100 kΩ → 12 V, Zener 12–15 V G‑S, PTC 2–2.5 A, TVS SMBJ10–12A, Cdec 10 µF + 0.1 µF | 6 | High‑side stage |
| Logic & I²C | STM32L031K6, TPS3809, TCA9548A, TCA9803 × 6, PCA9634, R_pull‑ups 2.2–4.7 kΩ, R_series 33–47 Ω, R_pulldown 10 kΩ (B‑side), ESD array | 1 set | Core logic |
| Connectors | 2×6 pogo pins per bay | 6 | Battery interface |
| Debug pads | SWD (3 pins), UART (3 pins) | 1 | Factory flashing + test |

---

## 🧮 Formulas Reference

- \( t_{CC}≈\frac{C_{eff}(SOC_2−SOC_1)}{I_{BAT}} \)  
- Example : \( C=3.3 Ah \), 0→100 % → ~2.06 h + CV ≈ 2.3–2.6 h typ.  

---

## 🚀 Next Steps

- Validate ADC monitor readings + supervisor timings.  
- Tune watchdog window periods and firmware timeouts.  
- Perform thermal tests @ 6× 1.6 A load (steady state).  
- Begin PCB layout and firmware integration.  

---

© 2025 EOS Positioning Systems — Internal development project  
