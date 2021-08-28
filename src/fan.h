/*
 *
 */
#ifndef SUPPLY_FAN_H
#define SUPPLY_FAN_H

#include <stdint.h>

/* enum { */
/*   FAN_SPEED_MIN = 0,        /\* stop the fan *\/ */
/*   FAN_SPEED_MAX = 100, */
/* }; */

/* set fan speed */
/* void fan_speed (uint8_t speed); */

void fan_update (uint8_t *param, uint8_t update_id);

void fan_try ();

void fan_init ();

#endif
