

#include "services_telemetry.h"
#include "board_io.h"
#include "services_identity.h"
#include <stdio.h>

// example
// UID,2A01C3F08B1D77AA0019FF10,TS,123456,BAY,3,ST,DIS,SOC_D10,472,VCELL_MV,7730,TEMP_D10,314,PRES,1,I2C,1,ERR,0,ELOADS,2,VIN_CV,1215



static const char* st_str(bay_state_t s)
{
  switch (s) {
    case BAY_S_INIT:      return "INI";
    case BAY_S_ABSENT:    return "ABS";
    case BAY_S_CHECK:     return "CHK";
    case BAY_S_CHARGE:    return "CHG";
    case BAY_S_DISCHARGE: return "DIS";
    case BAY_S_STABLE:    return "OK";
    case BAY_S_FAULT:     return "FLT";
    default:              return "UNK";
  }
}

/* CRC16-CCITT (0x1021), init 0xFFFF */
static uint16_t crc16_ccitt(const uint8_t *data, uint16_t len)
{
  uint16_t crc = 0xFFFFu;
  for (uint16_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t b = 0; b < 8; b++) {
      crc = (crc & 0x8000u) ? (uint16_t)((crc << 1) ^ 0x1021u) : (uint16_t)(crc << 1);
    }
  }
  return crc;
}

void Telemetry_SendAll(app_t *a, float vin12)
{
  static char uid[25];
  static uint8_t uid_ok = 0;




  if (!uid_ok) {
    ChargerUid_ToString(uid);  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!
    uid_ok = 1;
  }

  // appeler ChargerUid_ToString() une seule fois dans App_Init
  // et stocker la string, au lieu de la générer à la demande.


  uint32_t ts = HAL_GetTick();
  int vin_cV  = (int)(vin12 * 100.0f);   /* normalement c'est en centivolts  */

  char line[220];

  for (uint8_t i = 0; i < BAY_COUNT; i++) {
    const bay_t *b = &a->bay[i];

    int soc_d10   = (int)(b->soc * 10.0f);         /* 0.1 % */
    int temp_d10  = (int)(b->temp_c * 10.0f);      /* 0.1 C */
    int vcell_mV  = (int)(b->vcell_v * 1000.0f);   /* mV */

    /* Build line without CRLF first, append CRC then CRLF */
    int n = snprintf(line, sizeof(line),
      "UID,%s,TS,%lu,BAY,%u,ST,%s,SOC_D10,%d,VCELL_MV,%d,TEMP_D10,%d,PRES,%u,I2C,%u,ERR,%u,ELOADS,%u,VIN_CV,%d",
      uid,
      (unsigned long)ts,
      (unsigned)(i + 1),
      st_str(b->state),
      soc_d10,
      vcell_mV,
      temp_d10,
      (unsigned)b->present,
      (unsigned)b->i2c_ok,
      (unsigned)b->err,
      (unsigned)a->eload_slots,
      vin_cV
    );

    if (n > 0 && n < (int)sizeof(line) - 10) {
      uint16_t crc = crc16_ccitt((const uint8_t*)line, (uint16_t)n);
      int n2 = snprintf(line + n, sizeof(line) - (size_t)n, ",CRC,%04X\r\n", (unsigned)crc);
      if (n2 > 0) {
        uint16_t tot = (uint16_t)(n + n2);
        HAL_UART_Transmit(&huart2, (uint8_t*)line, tot, 50);
        USB_CDC_Tx((uint8_t*)line, tot);
      }
    }
  }



}
