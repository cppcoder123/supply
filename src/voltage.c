/*
 *
 */

#include "voltage.h"
#include "gui.h"
#include "param.h"
#include "poti.h"
#include "update.h"

#define VOLTAGE_MIN 12
#define VOLTAGE_MAX 200

static uint8_t voltage = 0;

static void change (uint8_t desired_voltage)
{
  /* resistor range is 0-99, so divide desired voltage by 2 */
  poti_tweak (POTI_ID_DCDC, desired_voltage / 2);
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
  const uint8_t new_voltage = (high << 6) | (low >> 2);
  if (new_voltage != voltage)
    gui_update (PARAM_VOLTAGE, voltage = new_voltage);
}

void voltage_init ()
{
  voltage = 0;

  poti_tweak (POTI_ID_DCDC, VOLTAGE_MIN);

  gui_update (PARAM_VOLTAGE, voltage);
}
