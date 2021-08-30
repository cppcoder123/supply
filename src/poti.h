/*
 * Variable resistor functionality
 */

#ifndef SUPPLY_POTI_H
#define SUPPLY_POTI_H

#include <stdint.h>

/* fixme: check */
#define POTI_MAX 99
/* fixme: check */
#define POTI_MIN 0
enum {
  POTI_ID_DCDC,
  POTI_ID_POWER,
  POTI_ID_SIZE,                 /* keep it last */
};

void poti_tweak (uint8_t poti_id, uint8_t position);

void poti_init ();

#endif
