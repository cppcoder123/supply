/*
 * Code that controls display
 */

#ifndef SUPPLY_GUI_H
#define SUPPLY_GUI_H

#include <stdint.h>

/* update current parameter values */
void gui_update (uint8_t param, uint8_t value);

void gui_rotor (uint8_t rotor_id, uint8_t rotor_action);

void gui_init ();

#endif
