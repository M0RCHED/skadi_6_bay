#pragma once
#include "app_config.h"

typedef enum {
  MODE_TARGET_30 = 0,
  MODE_TARGET_100 = 1
} app_mode_t;

typedef enum {
  BAY_CMD_STABLE = 0,
  BAY_CMD_CHARGE,
  BAY_CMD_DISCHARGE
} bay_cmd_t;

typedef enum {
  BAY_S_INIT = 0,
  BAY_S_ABSENT,
  BAY_S_CHECK,
  BAY_S_CHARGE,
  BAY_S_DISCHARGE,
  BAY_S_STABLE,
  BAY_S_FAULT
} bay_state_t;

typedef enum {
  BAY_ERR_NONE = 0,
  BAY_ERR_I2C,
  BAY_ERR_OVER_TEMP,
  BAY_ERR_TIMEOUT,
  BAY_ERR_INVALID
} bay_error_t;

typedef struct {
  uint8_t     present;
  uint8_t     i2c_ok;
  uint8_t     gauge_inited;

  float       soc;
  float       temp_c;
  float       vcell_v;
  uint16_t    status;

  bay_state_t state;
  bay_cmd_t   cmd;
  bay_error_t err;

  uint32_t    last_scan_ms;
  uint32_t    entry_ms;

  uint8_t     i2c_fail_cnt;
  uint32_t    chg_session_ms;
  uint32_t    dis_session_ms;
} bay_t;

typedef struct {
  app_mode_t  mode;
  bay_t       bay[BAY_COUNT];

  uint32_t    now_ms;
  uint32_t    last_ctrl_ms;
  uint32_t    last_telemetry_ms;
  uint8_t     rr_index;

  uint8_t     eload_slots;     /* 1..BAY_COUNT */
} app_t;
