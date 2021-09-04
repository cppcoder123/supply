/*
 *
 */

#include <avr/io.h>

#include "poti.h"

#define OUT_PORT PORTC

#define OUT_DDR DDRC

#define CS_0 0
#define CS_1 1

#define INC 2
#define U_D 3

#define POSITION_MAX 100
#define POSITION_MIN 0

static uint8_t position[POTI_ID_SIZE];

static void select (uint8_t cs)
{
  OUT_PORT &= ~(1 << cs);
}

static void deselect (uint8_t cs)
{
  OUT_PORT |= (1 << cs);
}

static void up ()
{
  OUT_PORT |= (1 << U_D);
}

static void down ()
{
  OUT_PORT &= ~(1 << U_D);
}

static void cycle (uint8_t delta)
{
  for (uint8_t i = 0; i < delta; ++i) {
    /* this pin is "negative edge triggered" , so */
    OUT_PORT |= (1 << INC);
    OUT_PORT &= ~(1 << INC);
  }
}

void poti_tweak (uint8_t poti_id, uint8_t new_position)
{
  if (new_position > POSITION_MAX)
    new_position = POSITION_MAX;

  if (position[poti_id] == new_position)
    return;

  const uint8_t id = (poti_id == POTI_ID_DCDC) ? CS_0 : CS_1;

  select (id);
  if (position[poti_id] > new_position) {
    up ();
    cycle (position[poti_id] - new_position);
  } else {
    down ();
    cycle (position[poti_id] - new_position);
  }
  deselect (id);

  position[poti_id] = new_position;
}

void poti_init ()
{
  /* configure outputs first */
  OUT_DDR |= (1 << CS_0) | (1 << CS_1) | (1 << INC) | (1 << U_D);

  select (CS_0);
  down ();
  cycle (100);
  deselect (CS_0);

  select (CS_1);
  down ();
  cycle (100);
  deselect (CS_1);

  position[POTI_ID_DCDC] = 0;
  position[POTI_ID_POWER] = 0;
}
