# ⚡ Skadi 6-Bay Smart Charger — v1.0 

> Version conceptuelle matérielle + logicielle décrivant l’architecture cible, la logique firmware et les contraintes de conception.
> Ce projet open hardware vise à fournir une plateforme de gestion intelligente pour 6 packs Li-ion 2S (Excel 2EXL1505 / Skadi).
> 🔗 **Documentation complète :** [hardware/](./hardware) • [firmware/](./firmware) • [docs/](./docs)

![EOS Logo](./Docs/eos_logo.png)

---

## 🧭 1 — Présentation du projet

Les transporteurs internationaux (UPS, FedEx, DHL, etc.) n’acceptent pas l’expédition de batteries Li-ion **> 30 % SoC** en raison des risques thermiques.  
Le **Skadi 6-Bay Smart Charger** vise à développer un **système à 6 baies** capable de **ramener automatiquement chaque batterie au seuil de ~30 % SoC**, qu’elle soit initialement trop basse (charge) ou trop haute (**décharge via ELOAD**).

Chaque **batterie Skadi** est **assemblée avec sa propre carte électronique**, intégrant :
- un **chargeur BQ24610** (régulation CC/CV, gestion thermique NTC) ;
- un **fuel gauge MAX17263** (mesure **SoC / tension / température** via **I²C**) ;
- un **fusible interne 2 A**, un **PTC** et un **NTC**.

La **carte maître** n’exécute **pas** la régulation CC/CV :  
elle agit comme **contrôleur d’alimentation** et d’interface :

- **Autoriser ou couper le 12 V** vers chaque baie (charge vers ~30 % SoC) ;  
- **Rediriger une baie vers un ELOAD externe** pour la décharge contrôlée lorsque le SoC est trop élevé ;  
- **Superviser** l’état de chaque pack via I²C (SoC, tension, température, alarmes) ;  
- **Afficher** l’état via LEDs (PCA9634) et **remonter les informations via BLE**.

---

### ⚙️ Spécifications prévues

| Élément | Description |
|---|---|
| **Alimentation principale** | 12 V / 12 A DC |
| **Nombre de baies** | 6 baies indépendantes (commutation high-side MOSFET) |
| **Connectique par baie** | 2×6 pogo pins *(12 V, GND, SDA, SCL, DET, réservé)* |
| **Électronique pack** | **BQ24610** (chargeur CC/CV) + **MAX17263** (fuel gauge I²C) |
| **Carte maître** | **STM32L031K6** + **TCA9548A** (MUX I²C) + **PCA9634** (LEDs) + **BLE HC-08** |
| **Alim logique 3.3 V** | Buck synchrone **TPS562201** (12 V → 3.3 V / 2 A) |
| **Ventilation** | Ventilateur 12 V **en fonctionnement continu** (≈ 27 dB), sans contrôle PWM |
| **Stratégie SoC** | **Charge** (12 V ON) ou **décharge** (ELOAD) pour converger vers ~30 % SoC |

---

## ⚙️ 2 — Architecture du système et logique firmware

Le **Skadi 6-Bay Smart Charger** est une plateforme modulaire conçue pour ajuster automatiquement l’état de charge de six batteries **Li-ion 2S Skadi (Excel 2EXL1505)** afin de respecter la limite réglementaire de **30 % SoC** imposée par les normes **IATA DGR / UN 3480**.  
Le système repose sur une **carte maître** qui pilote la distribution du 12 V, supervise l’état de chaque pack, et commande la décharge via un **ELOAD externe** lorsque nécessaire.

---

### 🧩 Architecture matérielle (vue fonctionnelle)

| Bloc fonctionnel | Rôle principal |
|------------------|----------------|
| **Alimentation 12 V / 12 A** | Source commune, protégée (fusible, TVS, NTC, condensateurs bulk) |
| **Buck 12 V → 3.3 V (TPS562201)** | Alimentation logique (MCU, MUX, buffers, LEDs, BLE) |
| **MOSFETs high-side ×6 (AO4407A)** | Commutation du 12 V pour chaque baie |
| **Drivers N-MOS (DMG2302UX)** | Commande des grilles des P-MOSFETs high-side |
| **MUX I²C TCA9548A** | Sélection de chaque baie sur le bus I²C |
| **Buffers I²C TCA9803 (×6)** | Isolation et level-shift des lignes I²C vers les packs |
| **MCU STM32L031K6** | Supervision, lecture SoC/T°, décision charge/décharge, télémétrie |
| **PCA9634** | Pilotage des LEDs d’état (1 LED/baie + éventuelle LED système) |
| **ELOAD externe** | Décharge sécurisée et précise (une baie à la fois, mode CC) |
| **Ventilateur 12 V** | Ventilation **en continu** du châssis, sans pilotage MCU |
| **BLE HC-08** | Télémétrie et diagnostic sans fil (SoC, tensions, états, faults) |

Chaque baie est indépendante et équipée d’une interface **2×6 pogo pins** (12 V, GND, SDA, SCL, DET, réservé).  
Les batteries embarquent déjà BQ24610 + MAX17263 + protections internes : la carte maître se contente de **gérer le 12 V et l’ELOAD**, et de **lire les mesures**.

---

### 🧠 Architecture firmware (FSM)

Le firmware est organisé sous forme de **machine à états finis (FSM)**, pilotée par le STM32L031K6, qui traite les baies **séquentiellement**.

#### Séquence de fonctionnement (par baie)

1. **INIT**  
   - Configuration MCU, I²C, MUX TCA9548A, buffers TCA9803.  
   - Initialisation du MAX17263 via **ModelGauge m5 EZ** (modèle Li-ion compatible NCR / EXCELL 2EXL1505).  
   - Détection de présence du pack (DET, tension, communication I²C).

2. **CHECK**  
   - Lecture cyclique (par ex. toutes les **5 s**) du SoC, de la tension pack et de la température.  
   - Décision : CHARGE, DÉCHARGE ou STABLE.

3. **CHARGE (12 V ON)**  
   - Activation du MOSFET high-side pour alimenter le BQ24610 en 12 V.  
   - Maintien jusqu’à ce que le SoC atteigne la zone cible (≈ 30 % + hystérésis) ou qu’un timeout soit atteint.

4. **DÉCHARGE (ELOAD)**  
   - Sélection d’une seule baie à la fois vers l’ELOAD externe (mode courant constant).  
   - Maintien jusqu’à ce que le SoC redescende autour de 30 % ou qu’un timeout soit atteint.

5. **STABLE (~30 % SoC)**  
   - Ni charge ni décharge : la baie reste en simple surveillance.  
   - LEDs et télémétrie indiquent que la batterie est prête pour le stockage ou l’expédition.

6. **FAULT**  
   - En cas de surchauffe, tension hors plage, erreur I²C persistante ou défaut pack.  
   - Coupure du 12 V et de l’ELOAD, mise à jour des LEDs et remontée d’un code d’erreur via BLE.

Les baies sont traitées en **round-robin** (une par une, en boucle), ce qui simplifie la gestion de puissance et évite les pointes de courant.

---

## 🔋 3 — Référence batterie : Excel 2EXL1505 (Skadi 2S Li-ion Pack)

Le chargeur est conçu pour les **batteries Skadi 2S Li-ion (Excel 2EXL1505)**, composées de deux cellules de marque **EXCELL**,  
électriquement **équivalentes aux Panasonic NCR18650B**.  
Chaque pack intègre sa propre carte électronique, comprenant :

- un **chargeur BQ24610** (profil CC/CV, fin de charge à 8.4 V) ;  
- un **fuel-gauge MAX17263** (mesure SoC, tension, température via I²C) ;  
- un **capteur NTC** pour la mesure thermique (valeur typique entre **10 kΩ et 47 kΩ**, à confirmer) ;  
- un **fusible interne 2 A** et un **PTC réarmable**.

---

### 3.1 — Caractéristiques principales

| Paramètre | Valeur typique | Commentaire |
|------------|----------------|--------------|
| **Type de pack** | Li-ion 2S (2 × 18650) | Cellules EXCELL 2EXL1505 (≈ NCR18650B) |
| **Chimie** | NCR (Nickel-Cobalt) | Compatible profil EZ Li-ion standard |
| **Tension nominale** | 7.2 V | (3.6 V × 2 cellules) |
| **Tension maximale** | 8.4 V | Fin de charge CV |
| **Tension minimale** | 5.0 V | Coupure sécurité (≈2.5 V/cellule) |
| **Capacité nominale** | 3.3 Ah | min. 3.25 Ah @ 25 °C |
| **Énergie spécifique** | ≈ 23.8 Wh / pack | — |
| **Courant de charge recommandé** | 1.6 A (0.5 C) | CC/CV – coupure à I_term faible |
| **Température de charge** | 0 → 45 °C | Limite thermique via NTC |
| **Température de décharge** | –20 → 60 °C | Surveillance par fuel-gauge |
| **Protection interne** | Fusible 2 A + PTC + NTC | Double sécurité intégrée |

---

## 🧠 4 — Initialisation du MAX17263 — Mode EZ (ModelGauge m5 EZ)

Le **MAX17263** exploite l’algorithme **ModelGauge m5 EZ**, permettant une initialisation simple, sans LUT spécifique.  
Ce mode charge automatiquement un modèle interne optimisé pour les chimies **Li-ion NCR/NCA**, compatible avec les cellules **EXCELL 2EXL1505 / NCR18650B**.

### 4.1 — Séquence d’initialisation (résumé)

1. **Reset logiciel** (`Command` 0x60 = 0x000F, attente ~200 ms).  
2. **Programmation des paramètres pack** : capacité, énergie, seuils de tension, courant de fin de charge, compensation, etc.  
3. **Configuration thermique** : NTC externe comme source température.  
4. **Activation du modèle EZ** (`ModelCfg` 0xDB = 0x8000, attente 1–2 s).  
5. **Option : sauvegarde NVM** (`Command` 0x60 = 0xE904).

Après cette séquence, le SoC, la tension et la température sont disponibles avec une précision suffisante pour piloter la logique de charge/décharge autour de **30 % SoC**.

---

## ⚙️ 5 — Dimensionnement & Formules

Cette section regroupe les calculs de dimensionnement électrique et thermique afin de justifier les choix d’alimentation, de protection et de routage PCB.

### 5.1 — Alimentation principale

- Source : **12 V / 12 A (150 W max)**  
- Nombre de baies : **6**  
- Charge typique par baie : **1.6 A @ 7.4 V** (via BQ24610)  
- Rendement estimé pack (chargeur interne) : **η ≈ 90 %**

**Courant d’entrée par baie :**  
I_in = (V_bat × I_bat) / (η × V_in)  
≈ (7.4 V × 1.6 A) / (0.9 × 12 V) ≈ **1.1 A**

👉 Chaque baie consomme ≈ **1.1 A** côté 12 V.

### 5.2 — Courant total et marge PSU

I_total = 6 × 1.1 A ≈ **6.6 A**  
En ajoutant pertes + logique + ventilateur : **≈ 8 A**.

Avec une alimentation 12 V / 12 A, la marge est confortable.  
Le traitement séquentiel des baies limite naturellement les pics de courant.

### 5.3 — Puissance

P_out_bay = 7.4 V × 1.6 A ≈ **11.8 W**  
P_in_bay ≈ 11.8 / 0.9 ≈ **13.2 W**  
P_total_6bays ≈ 6 × 13.2 ≈ **79 W**, bien en dessous de 150 W.

### 5.4 — Protection & filtrage d’entrée

- Fusible temporisé 15 A en entrée.  
- NTC MF72-5D-11 pour limiter le courant d’appel.  
- TVS SMBJ15A pour les transitoires.  
- Condensateurs bulk 470–1000 µF / 25 V + céramiques 100 nF.

### 5.5 — Pistes PCB (cuivre 1 oz)

| Type de piste | Courant max | Largeur mini conseillée |
|----------------|-------------|--------------------------|
| Bus 12 V | ≈10.5 A max | ≥ 3.0 mm |
| Ligne baie | 1.1–1.3 A | ≥ 1.0 mm |
| Retour GND | idem | Plan continu, via stitching |
| Signaux I²C/logique | < 5 mA | 0.25 mm |

### 5.6 — Gestion thermique & ventilation

Sources de pertes principales :
- MOSFETs AO4407A : < 0.2 W/baie à 1.1 A.  
- Buck TPS562201 : ≈ 0.5–0.7 W selon charge.  
- Logique : < 0.2 W.

Total pertes ≈ **1.8–2.0 W**.

Un **ventilateur 12 V** fonctionne en **permanence** à faible vitesse (~27 dB), ce qui suffit pour évacuer la chaleur sans nécessiter de contrôle PWM.  
Le firmware surveille principalement la **température pack** (pour couper CHARGE / DÉCHARGE en cas d’anomalie), mais ne pilote pas le ventilateur.

---

## 🧠 6 — Paramètres d’exploitation & Politiques Firmware

Cette section fixe les seuils, temporisations, règles de sécurité, arbitrage ELOAD, retours visuels et télémétrie.

### 6.1 — Seuils & temporisations (valeurs initiales)

| Élément | Valeur | Détail | Objectif |
|---|---:|---|---|
| SoC cible | 30 % | — | Conformité IATA / UN 3480 |
| Hystérésis SoC | 28 % / 30.5 % | bas / haut | Évite le pompage charge/décharge |
| Période de scan | 5 s/baie | lecture I²C + décision | Stabilité / charge CPU |
| T° pack max | 45 °C | via MAX17263 (NTC) | Coupure CHARGE/DÉCHARGE |
| Timeout décharge | 20 min | par session | Protéger en cas d’ELOAD mal réglé |
| Timeout charge | 25 min | par session | Protéger en cas de pack anormal |
| Re-try erreur I²C | 3 essais | 50 ms d’intervalle | Robustesse bus |

### 6.2 — Matrice sécurité (fault → action)

| Condition | Détection | Action immédiate | Récupération |
|---|---|---|---|
| T° pack > 45 °C | MAX17263.Temp | OFF 12 V, OFF ELOAD, log | Attendre < 40 °C → retour CHECK |
| Tension pack < 5.0 V | MAX17263.VCell | OFF ELOAD | Autoriser charge si T° ok |
| Tension pack > 8.6 V | ADC + gauge | OFF 12 V, log | Reprise après 5 s si normalisé |
| I²C bloqué | NACK / timeout | Reset canal MUX, 3 re-try | Isoler baie si persiste |
| Alim 12 V faible | ADC 12 V < 10.8 V | Stop nouvelles charges, log | Reprise > 11.5 V |
| ELOAD indispo | GPIO / handshake | Sauter baie, replanifier | Re-try au cycle suivant |

### 6.3 — Arbitrage ELOAD (une baie à la fois)

1. Sélection : baie la plus au-dessus de 30 % SoC.  
2. Exclusivité : verrou logiciel `ELOAD_LOCK`.  
3. Interlock : interdire 12 V et ELOAD simultanés sur une même baie.  
4. Courant ELOAD typique : 0.5–1.0 A (selon dissipation pack).  
5. Fin session : SoC ≤ 30 % ou timeout atteint.

### 6.4 — Politique d’activation 12 V (charge)

- Autoriser la charge si : SoC < 28 % et T° < 45 °C.  
- Interdire si : alimentation < 10.8 V ou erreur pack.  
- Fin de charge : SoC ≥ 30.5 % ou timeout atteint.

### 6.5 — Indications visuelles (LEDs via PCA9634)

| État baie | Couleur / pattern | Signification |
|---|---|---|
| IDLE (pack absent) | LED éteinte | Aucun pack détecté |
| CHECK | Blanc fixe faible | Lecture SoC / T° / V |
| CHARGE | Vert clignotement lent | 12 V actif |
| DÉCHARGE | Orange clignotement lent | ELOAD actif |
| STABLE (~30 %) | Bleu fixe | Conforme expédition |
| FAULT | Rouge clignotement rapide | Coupure sécurité / défaut |

### 6.6 — Télémétrie BLE (HC-08)

**Périodicité :** 1 trame / 5 s / baie active.  
**Format texte CSV**, simple à parser côté application :

```
BAY,<id>,STATE,<CHK|CHG|DIS|OK|FLT>,SOC,<%>,VCELL,<mV>,TEMP,<0.1°C>,ELD,<0|1>,ERR,<code>
```

**Exemple :**

```
BAY,03,STATE,DIS,SOC,47.2,VCELL,7730,TEMP,314,ELD,1,ERR,0
```

---

## 🧰 7 — Notes de conception matérielle & Routage PCB

Cette section résume les recommandations de placement, de routage et de CEM.

### 7.1 — Placement des blocs critiques

- MOSFETs high-side AO4407A : proches des connecteurs de baies.  
- Buffers I²C TCA9803 : un par baie, très proches des pogo pins.  
- MUX TCA9548A : centré, lignes courtes vers MCU.  
- Buck TPS562201 :  
  - près de l’entrée 12 V ;  
  - boucle VIN–SW–inductance–Cout compacte ;  
  - condensateur Cin à 2–3 mm de VIN/GND.  
- STM32L031K6 : dans une zone logique calme, plan GND propre.  
- PCA9634 : proche des LEDs.  
- HC-08 : éloigné de la self du buck et des pistes 12 V.

### 7.2 — Routage I²C

- Bus racine 3.3 V depuis MCU → MUX → buffers.  
- SDA/SCL par baie < 10 cm (pogo + câbles).  
- Résistances série 33–47 Ω sur SDA/SCL côté MCU.  
- Pull-ups 2.2–4.7 kΩ (bus + segments).  
- Plan GND continu sous les lignes I²C.

### 7.3 — Routage puissance

- Bus 12 V ≥ 3 mm, cuivre 1 oz.  
- Vers baies ≥ 1 mm.  
- Via-stitching autour des MOSFETs.  
- Condos 10 µF + 100 nF très proches des MOSFETs et des pogo pins.

### 7.4 — Ventilation

- Ventilateur 12 V, 40×40×10 mm, alimenté **directement** sur 12 V (marche en continu).  
- Positionné pour balayer les MOSFETs et la zone buck.  
- Pas de pilotage PWM ni MOSFET de commande requis.

### 7.5 — CEM / EMI

- TVS SMBJ15A sur l’entrée 12 V.  
- Ferrite d’entrée possible pour filtrer les hautes fréquences.  
- Condensateurs 100 nF à chaque VDD (MCU, MUX, buffers, PCA9634).  
- Éviter les pistes rapides sous l’antenne BLE.  
- Plan GND solide, via stitching, pas de coupures sous les signaux sensibles.

---

## 📦 8 — BOM principale (références clés)

Cette section liste les principaux composants.  
La BOM détaillée (R/C/D, footprints, tolérances) sera maintenue dans `hardware/bom/`.

### 8.1 — Alimentation & distribution 12 V

| Fonction | Référence / MPN | Boîtier | Notes |
|---------|------------------|---------|-------|
| Entrée DC 12 V | Connecteur borne / jack | — | Selon mécanique du châssis |
| Fusible principal | Bel Fuse 3SB 15A ou équiv. | 5×20 mm | Protection en entrée |
| NTC inrush | MF72-5D-11 ou équiv. | Disc | Limitation courant d’appel |
| TVS 12 V | SMBJ15A | SMB | Protection surtensions / transitoires |
| Condensateurs bulk | 470–1000 µF / 25 V | Radial / SMT | Lissage bus 12 V |
| Buck 12 V → 3.3 V | TPS562201 (TI) ou équiv. | SOT-23 / TSOT-6 | Alim logique 3.3 V (2 A max) |
| Condensateurs buck | Selon datasheet TPS562201 | 0805 / 1206 | Cin, Cout, bootstrap, etc. |

### 8.2 — Commutation par baie (×6)

| Fonction | Référence / MPN | Boîtier | Notes |
|---------|------------------|---------|-------|
| MOSFET high-side 12 V | AO4407A (AOS) ou équiv. | SO-8 | R_DS(on) faible |
| Driver N-MOS | DMG2302UX (Diodes Inc.) | SOT-23 | Commande de grille |
| Résistance gate | 47–100 Ω | 0603 | Limite dV/dt |
| Pull-up gate | 100 kΩ | 0603 | OFF par défaut |
| Zener G-S | BZT52 (12–15 V) | SOD-123 | Clamp transitoires |
| Découplage baie | 10 µF + 100 nF | 1206 + 0603 | Local près des pogo pins |

### 8.3 — Logique, I²C & LEDs

| Fonction | Référence / MPN | Boîtier | Notes |
|---------|------------------|---------|-------|
| MCU principal | STM32L031K6T6 (ST) | LQFP-32 | Cœur firmware, basse conso |
| Superviseur reset | TPS3809 (TI) | SOT-23-3 | Reset propre 3.3 V |
| MUX I²C | TCA9548A (TI) | TSSOP-24 | 8 canaux, 6 utilisés |
| Buffers I²C (×6) | TCA9803 (TI) | VSSOP-8 | Isolation / level shifting |
| Driver LEDs | PCA9634 (NXP) | TSSOP-28 | 16 canaux PWM |
| Pull-up I²C | 2.2–4.7 kΩ | 0603 | Bus principal + segments |
| Résistances série I²C | 33–47 Ω | 0603 | Amortissement / protection |
| Découplage logique | 100 nF + 4.7 µF | 0603 + 1206 | À chaque VDD (MCU, MUX, buffers, LED driver) |

### 8.4 — Interface & télémétrie

| Fonction | Référence / MPN | Boîtier | Notes |
|---------|------------------|---------|-------|
| Module BLE | HC-08 ou équiv. BLE UART | Module | Télémétrie / debug |
| Interface UART debug | Header 3 pins | 2.54 mm | TX/RX/GND |
| Connecteur SWD | Tag-Connect 6 pins ou pads SWD | — | Programmation / debug STM32 |

### 8.5 — Connectique baies & mécanique

| Fonction | Référence / MPN | Boîtier | Notes |
|---------|------------------|---------|-------|
| Pogo pins baies | Mill-Max 858-xx ou équiv. | — | Matrice 2×6 : 12 V, GND, SDA, SCL, DET, réservé |
| Guides mécaniques | Goupilles / inserts | — | Alignement batterie / pogo |
| Points de test | Pads GND / 12 V / I²C | — | Debug, validation production |

### 8.6 — Ventilation & capteurs thermiques

| Fonction | Référence / MPN | Boîtier | Notes |
|---------|------------------|---------|-------|
| Ventilateur | 12 V, 40×40×10 mm (ex. Sunon) | — | Fonctionnement continu (~27 dB) |
| NTC châssis (option) | 10 kΩ B3950 (à confirmer) | 0603 | Mesure T° interne (monitoring uniquement) |

### 8.7 — Protection & CEM

| Fonction | Référence / MPN | Boîtier | Notes |
|---------|------------------|---------|-------|
| Réseau ESD I²C | PESD5V0S4 (Nexperia) ou équiv. | SOT-23-6 | Protection SDA/SCL + lignes externes |
| Ferrites d’entrée (option) | ≥ 600 Ω @ 100 MHz | 0805 | Filtrage EMI sur 12 V |
| Plan GND | — | — | Masse continue, via stitching autour des zones puissance |

---

## 📂 9 — Structure du dépôt

```text
Skadi-6Bay-Charger/
├── hardware/           # Schémas, PCB KiCad
├── firmware/           # Code STM32 + BLE
├── docs/               # Images, graphiques, datasheets
├── README.md           # Présent document
└── LICENSE             # Licence projet
```

---

## 🚀 10 — Étapes suivantes

1. Finaliser le schéma et la netlist (incluant TPS562201 et fan toujours ON).  
2. Lancer le routage PCB en respectant les contraintes de puissance, I²C et CEM.  
3. Prototyper une première carte et vérifier :  
   - communication MAX17263 / MUX / buffers,  
   - comportement MOSFETs / baies,  
   - régulation 3.3 V (buck),  
   - ventilation et thermique.  
4. Ajuster les seuils firmware (SoC, temps, faults) selon les mesures réelles.  

---

© 2025 EOS Positioning Systems —
