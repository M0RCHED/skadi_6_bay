/*
 * drivers_tca9548a.c
 *
 *  Created on: 9 Jan 2026
 *      Author: Morched
 */


#include "drivers_tca9548a.h"
#include "board_io.h"



HAL_StatusTypeDef Mux_Select(uint8_t channel)
{
  if (channel > 7u) return HAL_ERROR;
  uint8_t mask = (uint8_t)(1u << channel);
  return HAL_I2C_Master_Transmit(&hi2c1, TCA9548A_ADDR, &mask, 1, 50);
}



HAL_StatusTypeDef Mux_DisableAll(void)
{
  uint8_t mask = 0u;
  return HAL_I2C_Master_Transmit(&hi2c1, TCA9548A_ADDR, &mask, 1, 50);
}


