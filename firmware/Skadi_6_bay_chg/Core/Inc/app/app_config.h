#pragma once
#include "stm32f1xx_hal.h"
#include <stdint.h>

#define BAY_COUNT                  6u

#define SOC_LOW_START              28.0f
#define SOC_STABLE_HIGH            30.5f
#define SOC_DISCH_START            31.0f

#define TEMP_MAX_C                 45.0f
#define TEMP_RECOVER_C             40.0f

#define SCAN_PERIOD_MS_PER_BAY     5000u
#define CTRL_PERIOD_MS             500u
#define TELEMETRY_PERIOD_MS        5000u

#define CHG_TIMEOUT_MS             (25u * 60u * 1000u)
#define DIS_TIMEOUT_MS             (20u * 60u * 1000u)

#define I2C_RETRY_MAX              3u
#define I2C_RETRY_DELAY_MS         50u

#define TCA9548A_ADDR              (0x70u << 1)
#define MAX17263_ADDR              (0x36u << 1)
#define PCA9634_ADDR               (0x60u << 1)

/* LED channels mapping  */
#define LED_CH_BAY0                0
#define LED_CH_BAY1                1
#define LED_CH_BAY2                2
#define LED_CH_BAY3                3
#define LED_CH_BAY4                4
#define LED_CH_BAY5                5
#define LED_CH_SYS0                6
#define LED_CH_SYS1                7
