#ifndef PINMAP_H
#define PINMAP_H

// === STM32L031K6 Pin Mapping ===
// Bus principal I2C (vers MUX TCA9548A et driver LED TLC59116)
#define PIN_I2C_SCL     PB6
#define PIN_I2C_SDA     PB7

// UART pour module BLE (HC-08)
#define PIN_UART_TX     PA9
#define PIN_UART_RX     PA10

// DIP switch 2 positions
#define PIN_DIP1        PA0
#define PIN_DIP2        PA1

// Contrôle des baies (via N-MOS → P-MOS high-side)
#define PIN_BAY1_CTRL   PB0
#define PIN_BAY2_CTRL   PB1
#define PIN_BAY3_CTRL   PB2
#define PIN_BAY4_CTRL   PB3
#define PIN_BAY5_CTRL   PB4
#define PIN_BAY6_CTRL   PB5

// Optionnels
#define PIN_FAN_PWM     PA8     // Ventilateur PWM
#define PIN_NTC_ADC     PA4     // Sonde de température châssis
#define PIN_LED_STATUS  PC13    // LED statut global

// Interface debug
#define PIN_SWDIO       PA13
#define PIN_SWCLK       PA14

// Boot et Reset
#define PIN_BOOT0       --      // Strap à GND via 100kΩ
#define PIN_NRST        --      // Reset (via SWD, pas de bouton physique)

#endif // PINMAP_H