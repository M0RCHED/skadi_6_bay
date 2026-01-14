#include "services_identity.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

/*
 * STM32F1 Unique Device ID base address
 * - 96-bit unique identifier
 * - Provided by ST, guaranteed unique per MCU
 * - Read-only, stored in system memory
 */
#define STM32_UID_BASE  (0x1FFFF7E8UL)

/*
 * Convert a 32-bit value to 8 ASCII hexadecimal characters.
 * Output buffer must be at least 8 bytes.
 *
 * Example:
 *   v = 0x1A2B3C4D  -> "1A2B3C4D"
 */
static void hex_u32(char *dst8, uint32_t v)
{
  static const char h[] = "0123456789ABCDEF";

  /* Generate hex string, MSB first */
  for (int i = 0; i < 8; i++) {
    dst8[7 - i] = h[v & 0x0Fu];
    v >>= 4;
  }
}

/*
 * Build a 24-character ASCII string from the STM32 96-bit UID.
 *
 * Format:
 *   UID = UID[31:0] | UID[63:32] | UID[95:64]
 *   Result: 24 hex characters + null terminator
 *
 * Example output:
 *   "2A01C3F08B1D77AA0019FF10"
 *
 * out25 must be at least 25 bytes.
 */
void ChargerUid_ToString(char out25[25])
{
  /* Pointer to UID registers in system memory */
  const uint32_t *uid = (const uint32_t*)STM32_UID_BASE;

  /* Convert each 32-bit word to ASCII hex */
  hex_u32(&out25[0],  uid[0]);
  hex_u32(&out25[8],  uid[1]);
  hex_u32(&out25[16], uid[2]);

  /* Null-terminate C string */
  out25[24] = '\0';
}
