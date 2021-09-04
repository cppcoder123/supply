/*
 * Control fan start current
 */

#ifndef SUPPLY_DAC_H
#define SUPPLY_DAC_H

#include <stdint.h>

enum {
  DAC_MAX,
  DAC_MIN,
  DAC_UNKNOWN,                  /* smth between */
};

void dac_rough_set (uint8_t level);

uint8_t dac_check (uint8_t level/*either min or max*/);

void dac_set (uint8_t high, uint8_t low);

void dac_init ();

#endif
