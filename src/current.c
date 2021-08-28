/*
 *
 */

#include "current.h"
#include "gui.h"
#include "param.h"
#include "update.h"

/*
 * fixme: Implement current monitor later
 */


/* 15 A? */
#define LIMIT_HIGH 150
/* 0.1 A? */
#define LIMIT_LOW 1

enum {
  ERROR_0 = 200,
  ERROR_1,
};

static uint8_t max_current = 0;
static uint8_t current = 0;

void current_update (uint8_t *param, uint8_t update_id)
{
  switch (update_id) {
  case UPDATE_DEC:
    if (*param > LIMIT_LOW)
      --(*param);
    break;
  case UPDATE_INC:
    if (*param < LIMIT_HIGH)
      ++(*param);
    break;
  case UPDATE_SWAP:
    max_current = *param;
    *param = current;
    break;
  default:
    break;
  }
}

void current_value (uint8_t high, uint8_t low)
{
  /*right adjusted*/
  /* check sign, it should be positive */
  if ((high & (1 << 1)) != 0) {
    /* report strange value */
    gui_update (PARAM_CURRENT, ERROR_0);
    return;
  }

  /* fixme: just ignore the last bit in low, tune it later to match voltmeter readings */
  /* 
   * 1. shift low to the right once
   * 2. shift high to the left 7 times
   * 3. or 1. & 2.
   */
  const uint8_t new_current = (high << 7) | (low >> 1);
  if (new_current != current)
    gui_update (PARAM_CURRENT, current = new_current);
}

void current_init ()
{
  max_current = 30;             /* 3 Amps */
  current = 0;

  gui_update (PARAM_CURRENT, current);
}
