/*
 *
 */

#ifndef SUPPLY_CURRENT_H
#define SUPPLY_CURRENT_H

#include <stdint.h>

void current_update (uint8_t *param, uint8_t update_id);

void current_value (uint8_t high, uint8_t low);

void current_init ();

#endif
