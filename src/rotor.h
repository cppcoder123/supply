/*
 *
 */
#ifndef SUPPLY_ROTOR_H
#define SUPPLY_ROTOR_H

enum {
  ROTOR_ID_0,
  ROTOR_ID_1,
  ROTOR_ID_MAX,                 /* keep it last */
};

enum {                          /* actions */
  ROTOR_FORWARD,                /* clockwise */
  ROTOR_BACKWARD,               /* counter clockwise */
};

void rotor_init ();

void rotor_try ();

#endif
