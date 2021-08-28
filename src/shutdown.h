/*
 *
 */

#ifndef SUPPLY_SHUTDOWN_H
#define SUPPLY_SHUTDOWN_H

#include <stdint.h>

void shutdown_enable_update (uint8_t *param, uint8_t update_id);
void shutdown_hour_update (uint8_t *param, uint8_t update_id);
void shutdown_minute_update (uint8_t *param, uint8_t update_id);

void shutdown_init ();

#endif
