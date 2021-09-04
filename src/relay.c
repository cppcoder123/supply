/*
 *
 */

#include <avr/io.h>

#include "bool.h"
#include "relay.h"

#define RELAY_PORT PORTB
#define RELAY_BIT 2

void relay_switch (uint8_t on)
{
  /* note: on is zero */
  if (on == BOOL_ON)
    RELAY_PORT &= ~(1 << RELAY_BIT);
  else
    RELAY_PORT |= (1 << RELAY_BIT);
}

void relay_init ()
{
  /* configure output & switch on */
  DDRB |= (1 << RELAY_BIT);

  relay_switch (BOOL_ON);
}
