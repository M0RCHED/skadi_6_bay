#include "services_led.h"
#include "drivers_pca9634.h"

static uint8_t blink_1hz(uint32_t t) { return ((t/500u)%2u) ? 255u : 0u; }
static uint8_t blink_2hz(uint32_t t) { return ((t/250u)%2u) ? 255u : 0u; }

void Led_Apply(app_t *a)
{
  uint32_t t = a->now_ms;

  uint8_t any_fault = 0;
  for (uint8_t i=0;i<BAY_COUNT;i++) if (a->bay[i].state == BAY_S_FAULT) any_fault = 1;

  (void)LedDrv_Set(LED_CH_SYS0, blink_1hz(t));
  (void)LedDrv_Set(LED_CH_SYS1, any_fault ? 255u : 0u);

  for (uint8_t i=0;i<BAY_COUNT;i++) {
    uint8_t duty = 0;
    switch (a->bay[i].state) {
      case BAY_S_ABSENT:    duty = 0; break;
      case BAY_S_CHECK:     duty = 40; break;
      case BAY_S_CHARGE:    duty = blink_1hz(t); break;
      case BAY_S_DISCHARGE: duty = blink_1hz(t); break;
      case BAY_S_STABLE:    duty = 255; break;
      case BAY_S_FAULT:     duty = blink_2hz(t); break;
      default:              duty = 0; break;
    }
    (void)LedDrv_Set((uint8_t)(LED_CH_BAY0 + i), duty);
  }
}


