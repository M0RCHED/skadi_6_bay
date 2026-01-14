#include "drivers_max17263.h"
#include "board_io.h"

static HAL_StatusTypeDef i2c_r16(uint8_t reg, uint16_t *out)
{
  uint8_t b[2];
  if (HAL_I2C_Master_Transmit(&hi2c1, MAX17263_ADDR, &reg, 1, 80) != HAL_OK) return HAL_ERROR;
  if (HAL_I2C_Master_Receive(&hi2c1, MAX17263_ADDR, b, 2, 80) != HAL_OK) return HAL_ERROR;
  *out = (uint16_t)(b[0] | (b[1] << 8));
  return HAL_OK;
}
static HAL_StatusTypeDef i2c_w16(uint8_t reg, uint16_t val)
{
  uint8_t b[3] = { reg, (uint8_t)(val & 0xFFu), (uint8_t)(val >> 8) };
  return HAL_I2C_Master_Transmit(&hi2c1, MAX17263_ADDR, b, 3, 80);
}

#define REG_STATUS   0x00u
#define REG_REPSOC   0x06u
#define REG_TEMP     0x08u
#define REG_VCELL    0x09u

HAL_StatusTypeDef Gauge_Read(float *soc, float *temp_c, float *vcell_v, uint16_t *status)
{
  uint16_t v;

  if (i2c_r16(REG_STATUS, &v) != HAL_OK) return HAL_ERROR;
  *status = v;

  if (i2c_r16(REG_REPSOC, &v) != HAL_OK) return HAL_ERROR;
  *soc = (float)v / 256.0f;

  if (i2c_r16(REG_TEMP, &v) != HAL_OK) return HAL_ERROR;
  int16_t ts = (int16_t)v;
  *temp_c = (float)ts / 256.0f;

  if (i2c_r16(REG_VCELL, &v) != HAL_OK) return HAL_ERROR;
  *vcell_v = (float)v * 78.125e-6f;

  return HAL_OK;
}

/* Optional: keep as hook; fill exact EZ sequence when you finalize params */
HAL_StatusTypeDef Gauge_EZ_Init(void)
{
  /* Implement later if needed. Return OK for now. */
  return HAL_OK;
}

