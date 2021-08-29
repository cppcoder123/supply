/*
 * Control fan start current
 */

#ifndef SUPPLY_POWER_H
#define SUPPLY_POWER_H

#include <stdint.h>

enum {
  POWER_LIMIT,
  POWER_OFF,
  POWER_ON,
};

void power_set (uint8_t level);

uint8_t power_get ();

void power_init ();

#endif
