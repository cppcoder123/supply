/*
 *
 */

#include "voltage.h"
#include "gui.h"
#include "param.h"
#include "update.h"

#define VOLTAGE_MIN 12
#define VOLTAGE_MAX 195

static uint8_t voltage = 0;

static void change (uint8_t new_voltage)
{
 /* fixme: access digital potentiometer chip here */
}

void voltage_update (uint8_t *param, uint8_t update_id)
{
  uint8_t tmp = 0;

  switch (update_id) {
  case UPDATE_DEC:
    if (voltage > VOLTAGE_MIN)
      --(*param);
    break;
  case UPDATE_INC:
    if (voltage < VOLTAGE_MAX)
      ++(*param);
    break;
  case UPDATE_SWAP:
    tmp = voltage;
    change (*param);
    *param = tmp;
    break;
  default:
    break;
  }
}

void voltage_value (uint8_t high, uint8_t low)
{
  /* single ended, max is 1024 */
  /* fixme: Move to the left 2 times for now, tune it later */
  /* 1. high to the left 6 times */
  /* 2. low to the right 2 times */
  /* 3. Or 1. & 2. */
  gui_update (PARAM_VOLTAGE, (high << 6) | (low >> 2));
}

void voltage_init ()
{
  voltage = 0;
}
