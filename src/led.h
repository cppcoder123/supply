/*
 * Max 7219 related code
 *
 * Bit bang to communicate with the chip.
 * CLK and IN (MOSI) belongs to port A,
 * CS belongs to port B
 */

#ifndef SUPPLY_LED_H
#define SUPPLY_LED_H

#include <stdint.h>

#define LED_ROW_SIZE 8
#define LED_DISPLAY_SIZE (LED_ROW_SIZE * 4)

/* data should have at least LED_DISPLAY_SIZE bytes ! */
void led_display (uint8_t *data);

void led_init ();

#endif
