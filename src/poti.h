/*
 * Variable resistor functionality
 */

#ifndef SUPPLY_POTI_H
#define SUPPLY_POTI_H

#include <stdint.h>

struct poti_t {
  uint8_t cs;
  uint8_t ud;                   /* up or down */
  uint8_t inc;                  /* or dec */
  /**/
  uint8_t position;             /* current position */
};

void poti_tweak (struct poti_t *poti, uint8_t knob);

void poti_init (struct poti_t *poti);

#endif
