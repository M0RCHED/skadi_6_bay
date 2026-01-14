#include "drivers_pca9634.h"
#include "board_io.h"

static HAL_StatusTypeDef w8(uint8_t reg, uint8_t val)
{
  uint8_t b[2] = { reg, val };
  return HAL_I2C_Master_Transmit(&hi2c1, PCA9634_ADDR, b, 2, 80);
}

HAL_StatusTypeDef LedDrv_Init(void)
{
  if (w8(0x00, 0x00) != HAL_OK) return HAL_ERROR; /* MODE1 */
  if (w8(0x01, 0x00) != HAL_OK) return HAL_ERROR; /* MODE2 */

  if (w8(0x14, 0xAA) != HAL_OK) return HAL_ERROR;
  if (w8(0x15, 0xAA) != HAL_OK) return HAL_ERROR;
  if (w8(0x16, 0xAA) != HAL_OK) return HAL_ERROR;
  if (w8(0x17, 0xAA) != HAL_OK) return HAL_ERROR;

  for (uint8_t ch = 0; ch < 16; ch++) {
    uint8_t b[2] = { (uint8_t)(0x02u + ch), 0u };
    if (HAL_I2C_Master_Transmit(&hi2c1, PCA9634_ADDR, b, 2, 80) != HAL_OK) return HAL_ERROR;
  }
  return HAL_OK;
}

HAL_StatusTypeDef LedDrv_Set(uint8_t ch, uint8_t duty)
{
  uint8_t b[2] = { (uint8_t)(0x02u + ch), duty };
  return HAL_I2C_Master_Transmit(&hi2c1, PCA9634_ADDR, b, 2, 80);
}


