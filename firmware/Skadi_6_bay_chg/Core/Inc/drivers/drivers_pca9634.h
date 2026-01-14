#pragma once
#include "app_config.h"

HAL_StatusTypeDef LedDrv_Init(void);
HAL_StatusTypeDef LedDrv_Set(uint8_t ch, uint8_t duty);
