
# ⚡ Skadi 6‑Bay Charger Firmware

Firmware embarqué pour le **chargeur intelligent multi‑baies (6‑bay)** destiné aux **batteries Skadi 2S Li‑ion (7.2 V / 3.3 Ah)** intégrant un **BQ24610** (charge CC/CV) et un **MAX17263** (fuel‑gauge).  
Le microcontrôleur principal est un **STM32L031K6** communiquant via **I²C** avec un **TCA9548A** (multiplexeur) et un **TLC59116** (driver LED I²C).  
L’objectif du firmware est de gérer automatiquement la charge à **30 % / 100 %**, d’initialiser le fuel‑gauge et d’assurer la supervision thermique et électrique.

---

## 🧭 Fonctionnalités principales

- Lecture du **State of Charge (SoC)** et de la température via MAX17263.  
- **Initialisation / programmation** du fuel‑gauge (écriture LUT / ModelGauge m5).  
- Gestion des modes : **Stockage (30 %)** et **Pleine charge (100 %)** via **DIP switch**.  
- Pilotage de **6 baies indépendantes** : activation des MOSFETs via GPIO.  
- **Indication LED** par bus I²C (TLC59116).  
- Communication optionnelle **BLE (UART)** pour télémétrie.  
- Lecture de la **sonde NTC** (châssis) et gestion éventuelle d’un **ventilateur PWM** (option).

---

## 🧩 Architecture logicielle

```
firmware/
 ├─ boards/
 │   ├─ pinmap.h          // Définition des broches MCU
 │   └─ board_config.h    // Paramètres baies, adresses I²C, options
 ├─ drivers/
 │   ├─ tca9548a.c/.h     // MUX I²C
 │   ├─ tlc59116.c/.h     // Driver LED courant constant
 │   ├─ fuelgauge.c/.h    // MAX17263 : init LUT, lecture SoC/Temp/Volt
 │   ├─ bays.c/.h         // Commande P‑MOS/N‑MOS
 │   ├─ dip.c/.h          // Lecture DIP switch
 │   └─ fan.c/.h          // Ventilation (option)
 ├─ app/
 │   ├─ main.c            // Point d’entrée, boucle principale
 │   ├─ scheduler.c/.h    // Gestion périodique (1 Hz / 100 ms)
 │   └─ ble.c/.h          // Communication Bluetooth Low Energy
 ├─ platform/
 │   └─ stm32l0_hal/      // HAL / CMSIS
 └─ tools/
     └─ lut_max17263.h    // Modèle LUT fuel‑gauge
```

---

## ⚙️ Configuration matérielle

- **MCU** : STM32L031K6 (LQFP‑32)  
- **Bus I²C** : TCA9548A (8 canaux) + TLC59116 (driver LED)  
- **Alim 3.3 V** : MP1584/MP2307 (buck)  
- **MOSFETs high‑side** : AO4407A (P‑MOS) commandés par 2N7002 (N‑MOS)  
- **Connecteurs baies** : 2 × 6 pogo pins  
- **Entrées DIP switch** : 2 positions (30 % / 100 %)  
- **UART BLE** : HC‑08 ou équivalent

---

## 🔧 Outils de développement

- **IDE :** [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) ou [PlatformIO](https://platformio.org/)  
- **Programmateur :** ST‑Link V2 / V3 (SWD)  
- **Toolchain :** GCC ARM Embedded  
- **Scripts :** `build.ps1` et `flash.ps1` (à venir)

---

## 🧰 Commandes Git / PowerShell (exemple)

```powershell
# Initialiser le dépôt et ajouter le pinmap
git init
mkdir firmware, firmware\boards
notepad firmware\boards\pinmap.h
git add .
git commit -m "Initial commit: project structure and pinmap.h"
git branch -M main
git remote add origin https://github.com/<user>/skadi-6bay-fw.git
git push -u origin main
```

---

## 🚀 Procédure de build & flash

1. Ouvrir le dossier dans STM32CubeIDE ou PlatformIO.  
2. Sélectionner la cible **STM32L031K6**.  
3. Compiler (`Ctrl +B`) → `.elf` généré dans `build/`.  
4. Connecter **ST‑Link** → `Run > Debug As > STM32 C/C++ Application`.  
5. Vérifier la sortie série (UART BLE) : messages init, I²C OK, SoC %.

---

## 🧮 Évolution prévue

- Implémentation du **mode décharge → 30 %**.  
- Ajout d’un **logging BLE** (trames JSON SoC / Temp).  
- Calibration NTC châssis + profil ventilation auto.  
- Support multi‑firmware (3‑bay / 6‑bay / 8‑bay).

---

## 📄 Licence

Projet interne EOS / Skadi. Licence à déterminer (MIT ou propriétaire).

---

## 🤝 Contributions

- Respecter la structure du projet et la norme de nommage : `snake_case`.  
- Créer une **branche feature/** avant PR (`feat/i2c_mux`, `feat/fuelgauge_init`, etc.).  
- Documenter les ajouts dans `CHANGELOG.md` et mettre à jour `pinmap.h` si besoin.
