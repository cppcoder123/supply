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

struct led_t
{
  uint8_t cs;
  uint8_t clk;
  uint8_t in;
};

void led_init (struct led_t *chip);

/* data should have at least 8 bytes ! */
void led_display (struct led_t *led, uint8_t *data);

#endif
