/*
 *
 */

#include "buf.h"
#include "timer.h"
#include "cron.h"

#define CRON_TIMER TIMER_2
#define CRON_PRESCALER TIMER_2_PRESCALER_1024

/*low-78, high-0 => 50 Hz*/
#define FACTOR_LOW 77
/* #define FACTOR_HIGH 0 */

static cron_callback callback_array[CRON_ID_MAX];
static uint8_t factor_array[CRON_ID_MAX];
static uint8_t timer_array[CRON_ID_MAX];

static struct buf_t queue;

void cron_init ()
{
  for (uint8_t i = 0; i < CRON_ID_MAX; ++i) {
    callback_array[i] = 0;
    factor_array[i] = 0;
    timer_array[i] = 0;
  }

  buf_init (&queue);
}

void cron_try ()
{
  uint8_t id = CRON_ID_MAX;
  if ((buf_byte_drain (&queue, &id) == 0)
      || (id >= CRON_ID_MAX)  /* ? */
      || (callback_array[id] == 0))
    return;

  callback_array[id] ();
}

static void interrupt_function ()
{
  for (uint8_t i = 0; i < CRON_ID_MAX; ++i) {
    if (callback_array[i] == 0)
      continue;
    if (++(timer_array[i]) >= factor_array[i]) {
      timer_array[i] = 0;
      buf_byte_fill (&queue, i);
    }
  }
}

static void engage_timer ()
{
  timer_interrupt_enable (CRON_TIMER, interrupt_function);
  timer_register_write (CRON_TIMER,
                          TIMER_OUTPUT_COMPARE_A, FACTOR_LOW, FACTOR_LOW);
  timer_enable (CRON_TIMER, CRON_PRESCALER);
}

static void disengage_timer ()
{
  timer_interrupt_disable (CRON_TIMER);
  timer_disable (CRON_TIMER);
}

static uint8_t is_required ()
{
  uint8_t status = 0;
  for (uint8_t i = 0; i < CRON_ID_MAX; ++i)
    if (callback_array[i] != 0) {
      status = 1;
      break;
    }

  return status;
}

uint8_t cron_enable (uint8_t id, uint8_t factor, cron_callback callback)
{
  if (id >= CRON_ID_MAX)
    return 0;

  callback_array[id] = callback;
  factor_array[id] = factor;
  timer_array[id] = 0;

  if (is_required () != 0)
    engage_timer ();

  return 1;
}

uint8_t cron_disable (uint8_t id)
{
  if (id >= CRON_ID_MAX)
    return 0;

  callback_array[id] = 0;
  factor_array[id] = 0;
  timer_array[id] = 0;

  if (is_required () == 0)
    disengage_timer ();

  return 1;
}
