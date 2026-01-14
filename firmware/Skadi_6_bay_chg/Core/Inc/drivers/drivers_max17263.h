#pragma once
#include "app_config.h"

HAL_StatusTypeDef Gauge_Read(float *soc, float *temp_c, float *vcell_v, uint16_t *status);
HAL_StatusTypeDef Gauge_EZ_Init(void); /* optional hook */
