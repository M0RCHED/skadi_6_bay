#pragma once
#include "app_types.h"


extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

/* USB CDC hooks: implement with your usbd_cdc_if.c */
uint8_t USB_CDC_Tx(const uint8_t *buf, uint16_t len);

/* Board specific */
app_mode_t Board_ReadMode(void);
uint8_t    Board_BayPresent(uint8_t bay_idx);
float      Board_ReadVin12(void);

void       Board_BayCmd(uint8_t bay_idx, bay_cmd_t cmd);
