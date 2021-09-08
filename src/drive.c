/*
 *
 */

#include <avr/io.h>

#include "drive.h"

#define HS_PORT PORTB
#define HS_BIT 5
#define HS_DDR DDRB

#define LS_PORT PORTB
#define LS_BIT 4
#define LS_DDR DDRB

#define OPEN_PORT PORTB
#define OPEN_BIT 2
#define OPEN_DDR DDRB

static uint8_t level = DRIVE_CLOSE;

void drive_set (uint8_t new_level)
{
  if (new_level == level)
    return;

  new_level = level;

  switch (level) {
  case DRIVE_CLOSE:
    OPEN_PORT &= ~(1 << OPEN_BIT);
    HS_PORT |= (1 << HS_BIT);
    LS_PORT &= ~(1 << LS_BIT);
    break;
  case DRIVE_LIMIT:
    OPEN_PORT &= ~(1 << OPEN_BIT);
    HS_PORT &= ~(1 << HS_BIT);
    LS_PORT |= (1 << LS_BIT);
    break;
  case DRIVE_OPEN:
    /* close hs before setting open bit */
    HS_PORT |= (1 << HS_BIT);
    LS_PORT &= ~(1 << LS_BIT);
    OPEN_PORT |= (1 << OPEN_BIT);
    break;
  default:
    break;
  }
}

uint8_t drive_get ()
{
  return level;
}

void drive_init ()
{
  HS_DDR |= (1 << HS_BIT);
  LS_DDR |= (1 << LS_BIT);
  OPEN_DDR |= (1 << OPEN_BIT);

  drive_set (DRIVE_CLOSE);
}
