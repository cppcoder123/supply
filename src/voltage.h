/*
 *
 */

#ifndef SUPPLY_VOLTAGE_H
#define SUPPLY_VOLTAGE_H

#include <stdint.h>

/* void voltage_update (uint8_t *value, uint8_t update_id); */

void voltage_value (uint8_t high, uint8_t low);

void voltage_init ();

#endif
