/*
 *
 */
#ifndef SUPPLY_ROTOR_H
#define SUPPLY_ROTOR_H

/* #include <stdint.h> */

void rotor_init ();

void rotor_try ();

enum {
  /* knob ids */
  ROTOR_0,
  ROTOR_1,
  ROTOR_2,
  ROTOR_3,
  /* actions */
  ROTOR_CLOCKWISE,
  ROTOR_COUNTER_CLOCKWISE,
};

#endif
