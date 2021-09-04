/*
 * Clock module, communicates with RTC hw module.
 * It sets 1Hz interrupt and reads data from the module
 */
#ifndef SUPPLY_CLOCK_H
#define SUPPLY_CLOCK_H

#include <stdint.h>

void clock_update (uint8_t *param, uint8_t param_id, uint8_t update_id);

void clock_init ();

#endif
