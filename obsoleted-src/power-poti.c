/*
 *
 *
 *
 */

#include "poti.h"
#include "power.h"

#define POSITION_LIMIT 35
#define POSITION_OFF POTI_MAX
#define POSITION_ON POTI_MIN

static uint8_t level = POWER_OFF;

void power_set (uint8_t new_level)
{
  switch (new_level) {
  case POWER_LIMIT:
    poti_tweak (POTI_ID_POWER, POSITION_LIMIT);
    break;
  case POWER_OFF:
    poti_tweak (POTI_ID_POWER, POSITION_OFF);
    break;
  case POWER_ON:
    poti_tweak (POTI_ID_POWER, POSITION_ON);
    break;
  default:
    return;
    break;
  }

  level = new_level;
}

uint8_t power_get ()
{
  return level;
}

void power_init ()
{
  /* twi_slave (TWI_ID_POWER, TWI_ADRESS); */
  /* poti_tweak (POTI_MAX); */

  level = POWER_OFF;
  power_set (POWER_OFF);
}

