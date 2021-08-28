/*
 * Control fan start current
 */

#ifndef SUPPLY_POWER_H
#define SUPPLY_POWER_H

#include <stdint.h>

enum {
  POWER_OFF,
  POWER_LIMIT,
  POWER_ON,
};

void power_set (uint8_t level);

uint8_t power_get ();

#endif
