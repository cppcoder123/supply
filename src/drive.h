/*
 * Drive mosfet gate 
 *  a. open it
 *  b. close
 *  c. set smth between a. & b. to limit the current
 */

#ifndef SUPPLY_DRIVE_H
#define SUPPLY_DRIVE_H

#include <stdint.h>

enum {
  DRIVE_CLOSE,
  DRIVE_LIMIT,
  DRIVE_OPEN,
};

void drive_set (uint8_t level);

uint8_t drive_get ();

void drive_init ();

#endif
