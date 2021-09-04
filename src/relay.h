/*
 *
 */

#ifndef SUPPLY_RELAY_H
#define SUPPLY_RELAY_H

#include <stdint.h>

/* it expects either BOOL_ON or BOOL_OFF */
void relay_switch (uint8_t bool);

void relay_init ();

#endif
