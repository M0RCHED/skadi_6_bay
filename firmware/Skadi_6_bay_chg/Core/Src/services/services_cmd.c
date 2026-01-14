#include "services_cmd.h"
#include <string.h>
#include <stdlib.h>

static char rxbuf[96];
static uint16_t rxlen;

static void apply_line(app_t *a, const char *s)
{
  if (strncmp(s, "ELOADS=", 7) == 0) {
    int v = atoi(&s[7]);
    if (v < 1) v = 1;
    if (v > (int)BAY_COUNT) v = (int)BAY_COUNT;
    a->eload_slots = (uint8_t)v;
  }
  if (strncmp(s, "MODE=30", 7) == 0) a->mode = MODE_TARGET_30;
  if (strncmp(s, "MODE=100", 8) == 0) a->mode = MODE_TARGET_100;
}

void Cmd_Ingest(app_t *a, const uint8_t *data, uint16_t len)
{
  for (uint16_t i=0;i<len;i++) {
    char c = (char)data[i];
    if (c == '\r') continue;
    if (c == '\n') {
      rxbuf[rxlen] = 0;
      if (rxlen) apply_line(a, rxbuf);
      rxlen = 0;
      continue;
    }
    if (rxlen < (sizeof(rxbuf)-1)) rxbuf[rxlen++] = c;
  }
}
