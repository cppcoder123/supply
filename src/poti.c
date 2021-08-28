/*
 *
 */

#include <avr/io.h>

#include "poti.h"

#define OUT_PORT PORTC
#define CS 0
#define INC 1
#define U_D 2

#define OUT_DDR DDRC

/*
 *
 */
#define KNOB_INITIAL 6
#define KNOB_MAX 100

#define SELECT (OUT_PORT &= ~(1 << CS))
#define DESELECT (OUT_PORT |= (1 << CS))

#define UP (OUT_PORT |= (1 << U_D))
#define DOWN (OUT_PORT &= ~(1 << U_D))

static uint8_t knob = KNOB_INITIAL;

static void cycle (uint8_t number)
{
  for (uint8_t i = 0; i < number; ++i) {
    /* this pin is "negative edge triggered" , so */
    OUT_PORT |= (1 << INC);
    OUT_PORT &= ~(1 << INC);
  }
}

void poti_tweak (uint8_t new_knob)
{
  if (new_knob > KNOB_MAX)
    new_knob = KNOB_MAX;

  if (new_knob == knob)
    return;

  SELECT;
  if (new_knob > knob) {
    UP;
    cycle (new_knob - knob);
  } else {
    DOWN;
    cycle (knob - new_knob);
  }
  DESELECT;

  knob = new_knob;
}

void poti_init ()
{
  /* configure outputs first */
  OUT_DDR |= (1 << CS) | (1 << INC) | (1 << U_D);

  SELECT;
  DOWN;
  cycle (100);
  DESELECT;

  poti_tweak (KNOB_INITIAL);
  /* fixme */
}
