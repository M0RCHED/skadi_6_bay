#include "app.h"
#include "board_io.h"
#include "drivers_tca9548a.h"
#include "drivers_max17263.h"
#include "drivers_pca9634.h"
#include "services_led.h"
#include "services_telemetry.h"
#include "services_cmd.h"
#include <string.h>

static uint8_t temp_fault(const bay_t *b) { return (b->temp_c >= TEMP_MAX_C); }
static uint8_t temp_recover(const bay_t *b){ return (b->temp_c <= TEMP_RECOVER_C); }

static void bay_enter(bay_t *b, bay_state_t s)
{
  b->state = s;
  b->entry_ms = HAL_GetTick();
}

static void bay_fault(bay_t *b, bay_error_t e)
{
  b->err = e;
  b->cmd = BAY_CMD_STABLE;
  bay_enter(b, BAY_S_FAULT);
}

static uint8_t i2c_read_one_bay(app_t *a, uint8_t i)
{
  bay_t *b = &a->bay[i];
  b->i2c_ok = 0;

  if (Mux_Select(i) != HAL_OK) return 0;

  for (uint8_t k=0;k<I2C_RETRY_MAX;k++) {
    float soc=0, t=0, v=0; uint16_t st=0;
    if (Gauge_Read(&soc, &t, &v, &st) == HAL_OK) {
      b->soc = soc; b->temp_c = t; b->vcell_v = v; b->status = st;
      b->i2c_ok = 1;
      b->i2c_fail_cnt = 0;
      (void)Mux_DisableAll();
      return 1;
    }
    HAL_Delay(I2C_RETRY_DELAY_MS);
  }

  b->i2c_fail_cnt++;
  (void)Mux_DisableAll();
  return 0;
}

/* Select up to eload_slots bays to discharge (highest SOC first) */
static void eload_select_set(app_t *a, uint8_t allow_dis[BAY_COUNT])
{
  for (uint8_t i=0;i<BAY_COUNT;i++) allow_dis[i] = 0;

  uint8_t slots = a->eload_slots;
  if (slots < 1) slots = 1;
  if (slots > BAY_COUNT) slots = BAY_COUNT;

  for (uint8_t s=0;s<slots;s++) {
    float best_soc = SOC_DISCH_START;
    int best = -1;
    for (uint8_t i=0;i<BAY_COUNT;i++) {
      bay_t *b = &a->bay[i];
      if (allow_dis[i]) continue;
      if (!b->present || !b->i2c_ok) continue;
      if (b->state == BAY_S_FAULT) continue;
      if (temp_fault(b)) continue;
      if (b->soc <= SOC_DISCH_START) continue;
      if (b->soc > best_soc) { best_soc = b->soc; best = (int)i; }
    }
    if (best >= 0) allow_dis[(uint8_t)best] = 1;
  }
}

static bay_cmd_t decide_cmd(app_t *a, uint8_t i, uint8_t allow_dis)
{
  bay_t *b = &a->bay[i];

  if (!b->present) return BAY_CMD_STABLE;
  if (!b->i2c_ok) return BAY_CMD_STABLE;
  if (temp_fault(b)) return BAY_CMD_STABLE;

  if (a->mode == MODE_TARGET_30) {
    if (b->soc < SOC_LOW_START) return BAY_CMD_CHARGE;
    if (b->soc > SOC_DISCH_START) return allow_dis ? BAY_CMD_DISCHARGE : BAY_CMD_STABLE;
    return BAY_CMD_STABLE;
  }

  if (b->soc < 99.0f) return BAY_CMD_CHARGE;
  return BAY_CMD_STABLE;
}

static void bay_step(app_t *a, uint8_t i, uint8_t allow_dis)
{
  bay_t *b = &a->bay[i];
  uint32_t now = a->now_ms;

  b->present = Board_BayPresent(i);

  switch (b->state) {
    case BAY_S_INIT:
      b->cmd = BAY_CMD_STABLE;
      b->err = BAY_ERR_NONE;
      b->gauge_inited = 0;
      if (!b->present) bay_enter(b, BAY_S_ABSENT);
      else bay_enter(b, BAY_S_CHECK);
      break;

    case BAY_S_ABSENT:
      b->cmd = BAY_CMD_STABLE;
      b->i2c_ok = 0;
      b->gauge_inited = 0;
      b->err = BAY_ERR_NONE;
      if (b->present) bay_enter(b, BAY_S_CHECK);
      break;

    case BAY_S_CHECK:
      b->cmd = BAY_CMD_STABLE;
      if (!b->present) { bay_enter(b, BAY_S_ABSENT); break; }
      if (!b->i2c_ok) {
        if (b->i2c_fail_cnt >= I2C_RETRY_MAX) bay_fault(b, BAY_ERR_I2C);
        break;
      }
      if (!b->gauge_inited) {
        if (Mux_Select(i) == HAL_OK) {
          if (Gauge_EZ_Init() == HAL_OK) b->gauge_inited = 1;
          (void)Mux_DisableAll();
        }
      }
      if (temp_fault(b)) { bay_fault(b, BAY_ERR_OVER_TEMP); break; }

      b->cmd = decide_cmd(a, i, allow_dis);
      if (b->cmd == BAY_CMD_CHARGE) { b->chg_session_ms = now; bay_enter(b, BAY_S_CHARGE); }
      else if (b->cmd == BAY_CMD_DISCHARGE) { b->dis_session_ms = now; bay_enter(b, BAY_S_DISCHARGE); }
      else bay_enter(b, BAY_S_STABLE);
      break;

    case BAY_S_CHARGE:
      if (!b->present) { b->cmd = BAY_CMD_STABLE; bay_enter(b, BAY_S_ABSENT); break; }
      if (!b->i2c_ok) { b->cmd = BAY_CMD_STABLE; if (b->i2c_fail_cnt >= I2C_RETRY_MAX) bay_fault(b, BAY_ERR_I2C); break; }
      if (temp_fault(b)) { b->cmd = BAY_CMD_STABLE; bay_fault(b, BAY_ERR_OVER_TEMP); break; }
      if ((now - b->chg_session_ms) > CHG_TIMEOUT_MS) { b->cmd = BAY_CMD_STABLE; bay_fault(b, BAY_ERR_TIMEOUT); break; }

      if (b->soc >= SOC_STABLE_HIGH) { b->cmd = BAY_CMD_STABLE; bay_enter(b, BAY_S_STABLE); }
      else b->cmd = BAY_CMD_CHARGE;
      break;

    case BAY_S_DISCHARGE:
      if (!b->present) { b->cmd = BAY_CMD_STABLE; bay_enter(b, BAY_S_ABSENT); break; }
      if (!b->i2c_ok) { b->cmd = BAY_CMD_STABLE; if (b->i2c_fail_cnt >= I2C_RETRY_MAX) bay_fault(b, BAY_ERR_I2C); break; }
      if (temp_fault(b)) { b->cmd = BAY_CMD_STABLE; bay_fault(b, BAY_ERR_OVER_TEMP); break; }
      if ((now - b->dis_session_ms) > DIS_TIMEOUT_MS) { b->cmd = BAY_CMD_STABLE; bay_fault(b, BAY_ERR_TIMEOUT); break; }

      if (!allow_dis) { b->cmd = BAY_CMD_STABLE; bay_enter(b, BAY_S_CHECK); break; }

      if (b->soc <= SOC_STABLE_HIGH) { b->cmd = BAY_CMD_STABLE; bay_enter(b, BAY_S_STABLE); }
      else b->cmd = BAY_CMD_DISCHARGE;
      break;

    case BAY_S_STABLE:
      b->cmd = BAY_CMD_STABLE;
      if (!b->present) { bay_enter(b, BAY_S_ABSENT); break; }
      if (!b->i2c_ok) { if (b->i2c_fail_cnt >= I2C_RETRY_MAX) bay_fault(b, BAY_ERR_I2C); break; }
      if (temp_fault(b)) { bay_fault(b, BAY_ERR_OVER_TEMP); break; }
      if (b->soc < SOC_LOW_START || b->soc > SOC_DISCH_START) bay_enter(b, BAY_S_CHECK);
      break;

    case BAY_S_FAULT:
      b->cmd = BAY_CMD_STABLE;
      if (!b->present) { bay_enter(b, BAY_S_ABSENT); break; }
      if (b->err == BAY_ERR_OVER_TEMP && b->i2c_ok && temp_recover(b)) { b->err = BAY_ERR_NONE; bay_enter(b, BAY_S_CHECK); }
      if (b->err == BAY_ERR_I2C && b->i2c_ok) { b->err = BAY_ERR_NONE; bay_enter(b, BAY_S_CHECK); }
      break;

    default:
      bay_fault(b, BAY_ERR_INVALID);
      break;
  }

  Board_BayCmd(i, b->cmd);
}

void App_Init(app_t *a)
{
  memset(a, 0, sizeof(*a));
  a->mode = MODE_TARGET_30;
  a->rr_index = 0;
  a->eload_slots = 1;

  for (uint8_t i=0;i<BAY_COUNT;i++) {
    a->bay[i].state = BAY_S_INIT;
    a->bay[i].cmd = BAY_CMD_STABLE;
    a->bay[i].err = BAY_ERR_NONE;
  }

  for (uint8_t i=0;i<BAY_COUNT;i++) Board_BayCmd(i, BAY_CMD_STABLE);

  (void)Mux_DisableAll();
  (void)LedDrv_Init();
}

void App_Tick(app_t *a)
{
  a->now_ms = HAL_GetTick();

  static uint8_t allow_dis[BAY_COUNT];

  bay_t *brr = &a->bay[a->rr_index];
  if ((a->now_ms - brr->last_scan_ms) >= SCAN_PERIOD_MS_PER_BAY) {
    brr->last_scan_ms = a->now_ms;
    brr->present = Board_BayPresent(a->rr_index);
    if (brr->present) (void)i2c_read_one_bay(a, a->rr_index);
    else { brr->i2c_ok = 0; brr->i2c_fail_cnt = 0; }
    a->rr_index = (uint8_t)((a->rr_index + 1u) % BAY_COUNT);
  }

  if ((a->now_ms - a->last_ctrl_ms) >= CTRL_PERIOD_MS) {
    a->last_ctrl_ms = a->now_ms;

    eload_select_set(a, allow_dis);

    for (uint8_t i=0;i<BAY_COUNT;i++) {
      bay_step(a, i, allow_dis[i]);
    }

    Led_Apply(a);
  }

  if ((a->now_ms - a->last_telemetry_ms) >= TELEMETRY_PERIOD_MS) {
    a->last_telemetry_ms = a->now_ms;
    float vin12 = Board_ReadVin12();
    Telemetry_SendAll(a, vin12);
  }
}
