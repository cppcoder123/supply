/*
 *
 */

#include <avr/io.h>

#include "poti.h"

#define OUT_PORT PORTC
/* #define CS 0 */
/* #define INC 1 */
/* #define U_D 2 */

#define OUT_DDR DDRC

/*
 *
 */
#define KNOB_INITIAL 6
#define KNOB_MAX 100


/* static uint8_t knob = KNOB_INITIAL; */

static void select (struct poti_t *poti)
{
  OUT_PORT &= ~(1 << poti->cs);
}

static void deselect (struct poti_t *poti)
{
  OUT_PORT |= (1 << poti->cs);
}

static void up (struct poti_t *poti)
{
  OUT_PORT |= (1 << poti->ud);
}

static void down (struct poti_t *poti)
{
  OUT_PORT &= ~(1 << poti->ud);
}

static void cycle (struct poti_t* poti, uint8_t delta)
{
  for (uint8_t i = 0; i < delta; ++i) {
    /* this pin is "negative edge triggered" , so */
    OUT_PORT |= (1 << poti->inc);
    OUT_PORT &= ~(1 << poti->inc);
  }
}

void poti_tweak (struct poti_t *poti, uint8_t position)
{
  if (position > KNOB_MAX)
    position = KNOB_MAX;

  if (position == poti->position)
    return;

  select (poti);
  if (position > poti->position) {
    up (poti);
    cycle (poti, position - poti->position);
  } else {
    down (poti);
    cycle (poti, position - poti->position);
  }
  deselect (poti);

  poti->position = position;
}

void poti_init (struct poti_t *poti)
{
  /* configure outputs first */
  OUT_DDR |= (1 << poti->cs) | (1 << poti->inc) | (1 << poti->ud);

  select (poti);
  down (poti);
  cycle (poti, 100);
  deselect (poti);

  poti_tweak (poti, KNOB_INITIAL);
  /* fixme */
}
