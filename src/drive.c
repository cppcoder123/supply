/*
 *
 */

#include <avr/io.h>

#include "drive.h"

#define LIMIT_HS_PORT PORTC
#define LIMIT_HS_BIT 2
#define LIMIT_HS_DDR DDRC

#define LIMIT_LS_PORT PORTC
#define LIMIT_LS_BIT 1
#define LIMIT_LS_DDR DDRC

#define OPEN_LS_PORT PORTB
#define OPEN_LS_BIT 2
#define OPEN_LS_DDR DDRB

#define OPEN_HS_PORT PORTC
#define OPEN_HS_BIT 0
#define OPEN_HS_DDR DDRC

static uint8_t level = DRIVE_CLOSE;

void drive_set (uint8_t new_level)
{
  if (new_level == level)
    return;

  new_level = level;

  switch (level) {
  case DRIVE_CLOSE:
    OPEN_LS_PORT &= ~(1 << OPEN_LS_BIT);
    LIMIT_HS_PORT |= (1 << LIMIT_HS_BIT);
    LIMIT_LS_PORT &= ~(1 << LIMIT_LS_BIT);
    break;
  case DRIVE_LIMIT:
    OPEN_LS_PORT &= ~(1 << OPEN_LS_BIT);
    LIMIT_HS_PORT &= ~(1 << LIMIT_HS_BIT);
    LIMIT_LS_PORT |= (1 << LIMIT_LS_BIT);
    break;
  case DRIVE_OPEN:
    /* close hs before setting open bit */
    LIMIT_HS_PORT |= (1 << LIMIT_HS_BIT);
    LIMIT_LS_PORT &= ~(1 << LIMIT_LS_BIT);
    OPEN_LS_PORT |= (1 << OPEN_LS_BIT);
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
  LIMIT_HS_DDR |= (1 << LIMIT_HS_BIT);
  LIMIT_LS_DDR |= (1 << LIMIT_LS_BIT);
  OPEN_LS_DDR |= (1 << OPEN_LS_BIT);

  drive_set (DRIVE_CLOSE);
}
