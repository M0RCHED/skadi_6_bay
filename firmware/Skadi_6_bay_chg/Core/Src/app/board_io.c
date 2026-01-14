#include "board_io.h"
#include "main.h"

#include "drivers_tca9548a.h"
#include "app_config.h"



/* Map these to your CubeMX pins */
static void bay_gpio(uint8_t bay, uint8_t is_chg, GPIO_PinState st)
{
  switch (bay) {
    case 0:
      if (is_chg) HAL_GPIO_WritePin(BAY1_CHG_EN_GPIO_Port, BAY1_CHG_EN_Pin, st);
      else        HAL_GPIO_WritePin(BAY1_DIS_EN_GPIO_Port, BAY1_DIS_EN_Pin, st);
      break;
    case 1:
      if (is_chg) HAL_GPIO_WritePin(BAY2_CHG_EN_GPIO_Port, BAY2_CHG_EN_Pin, st);
      else        HAL_GPIO_WritePin(BAY2_DIS_EN_GPIO_Port, BAY2_DIS_EN_Pin, st);
      break;
    case 2:
      if (is_chg) HAL_GPIO_WritePin(BAY3_CHG_EN_GPIO_Port, BAY3_CHG_EN_Pin, st);
      else        HAL_GPIO_WritePin(BAY3_DIS_EN_GPIO_Port, BAY3_DIS_EN_Pin, st);
      break;
    case 3:
      if (is_chg) HAL_GPIO_WritePin(BAY4_CHG_EN_GPIO_Port, BAY4_CHG_EN_Pin, st);
      else        HAL_GPIO_WritePin(BAY4_DIS_EN_GPIO_Port, BAY4_DIS_EN_Pin, st);
      break;
    case 4:
      if (is_chg) HAL_GPIO_WritePin(BAY5_CHG_EN_GPIO_Port, BAY5_CHG_EN_Pin, st);
      else        HAL_GPIO_WritePin(BAY5_DIS_EN_GPIO_Port, BAY5_DIS_EN_Pin, st);
      break;
    case 5:
      if (is_chg) HAL_GPIO_WritePin(BAY6_CHG_EN_GPIO_Port, BAY6_CHG_EN_Pin, st);
      else        HAL_GPIO_WritePin(BAY6_DIS_EN_GPIO_Port, BAY6_DIS_EN_Pin, st);
      break;
    default: break;
  }
}

void Board_BayCmd(uint8_t bay_idx, bay_cmd_t cmd)
{
  if (cmd == BAY_CMD_CHARGE) {
    bay_gpio(bay_idx, 0, GPIO_PIN_RESET);
    bay_gpio(bay_idx, 1, GPIO_PIN_SET);
  } else if (cmd == BAY_CMD_DISCHARGE) {
    bay_gpio(bay_idx, 1, GPIO_PIN_RESET);
    bay_gpio(bay_idx, 0, GPIO_PIN_SET);
  } else {
    bay_gpio(bay_idx, 1, GPIO_PIN_RESET);
    bay_gpio(bay_idx, 0, GPIO_PIN_RESET);
  }
}

app_mode_t Board_ReadMode(void)
{
  return MODE_TARGET_30;
}

uint8_t Board_BayPresent(uint8_t bay_idx)
{
  if (bay_idx >= BAY_COUNT) return 0;

  extern I2C_HandleTypeDef hi2c1;

  /* Detect presence by probing the MAX17263 ACK on the selected MUX channel. */
  if (Mux_Select(bay_idx) != HAL_OK) {
    (void)Mux_DisableAll();
    return 0;
  }

  HAL_StatusTypeDef st = HAL_I2C_IsDeviceReady(&hi2c1, MAX17263_ADDR, 1, 20);
  (void)Mux_DisableAll();
  return (st == HAL_OK) ? 1u : 0u;
}

float Board_ReadVin12(void)
{
  extern ADC_HandleTypeDef hadc1;

  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) { HAL_ADC_Stop(&hadc1); return 0.0f; }
  uint32_t raw = HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);

  float vadc = 3.3f * (float)raw / 4095.0f;
  float vin  = vadc * (133.0f / 33.0f); /* 100k/33k */
  return vin;
}

/* Implement in usbd_cdc_if.c (CDC_Transmit_FS wrapper) */
uint8_t USB_CDC_Tx(const uint8_t *buf, uint16_t len)
{
  extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
  return CDC_Transmit_FS((uint8_t*)buf, len);
}
