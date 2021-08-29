/*
 *
 *
 *
 */

#include "power.h"
#include "twi.h"

#define TWI_ADRESS 0x62

#define OFF_VALUE_HIGH 0xF
#define OFF_VALUE_LOW 0xFF

#define ON_VALUE_HIGH 0x0
#define ON_VALUE_LOW 0x0

/* fixme: select write value to limit the current */
#define LIMIT_VALUE_HIGH 0x05
#define LIMIT_VALUE_LOW 0x55

/*
 * We are using "fast command mode"
 * and "normal mode" shutdown register,
 * so we need to transfer zero to designate both of them
 *
 * DAC register ("1100abcd")
 *   ab = 00 => fast mode
 *   cd = 00 => normal mode for shutdown reg
 */
#define DAC_REG 0xC0
#define MODE_MASK 0xF

/* arbitrary twi tag, we don't use callbacks  */
#define DAC_TAG 111

/* data buffer size*/
#define DATA_SIZE 2


static uint8_t level = POWER_OFF;

static void write (uint8_t high, uint8_t low)
{
  uint8_t data[DATA_SIZE];
  /* clear left nibble for high */
  data[0] = high & MODE_MASK;
  data[1] = low;
  twi_write_array (TWI_ID_POWER, DAC_TAG, DATA_SIZE, DAC_REG, data);
}

void power_set (uint8_t new_level)
{
  switch (new_level) {
  case POWER_LIMIT:
    write (LIMIT_VALUE_HIGH, LIMIT_VALUE_LOW);
    break;
  case POWER_OFF:
    write (OFF_VALUE_HIGH, OFF_VALUE_LOW);
    break;
  case POWER_ON:
    write (ON_VALUE_HIGH, ON_VALUE_LOW);
    break;
  default:
    return;
  }

  level = new_level;
}

uint8_t power_get ()
{
  return level;
}

void power_init ()
{
  twi_slave (TWI_ID_POWER, TWI_ADRESS);

  power_set (POWER_OFF);
}

