/*
 *
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/atomic.h>


#include "adc.h"
#include "buf.h"
#include "cron.h"
#include "current.h"
#include "voltage.h"

#define READING_SIZE 3

#define CRON_FACTOR 5

enum {
  CHANNEL_CURRENT,
  CHANNEL_VOLTAGE,
};

/* enum { */
/*   COMMAND_TOGGLE,               /\* toggle input channel *\/ */
/*   COMMAND_START, */
/* }; */

enum {                          /* state */
  STATE_ADC_COMPLETED,
  STATE_ADC_STARTED,
  STATE_CHANNEL_SELECTED,
};


volatile static uint8_t channel = 0;
volatile static uint8_t state = STATE_ADC_COMPLETED;

static struct buf_t reading_buf;

static void write (uint8_t channel_id, uint8_t high, uint8_t low)
{
  ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
    if (buf_space (&reading_buf) < READING_SIZE)
      return;

    if (buf_byte_fill (&reading_buf, channel_id) == 0)
      return;

    if (buf_byte_fill (&reading_buf, high) == 0) {
      buf_byte_drain (&reading_buf, &channel_id);
      return;
    }

    if (buf_byte_fill (&reading_buf, low) == 0) {
      buf_byte_drain (&reading_buf, &channel_id);
      buf_byte_drain (&reading_buf, &high);
      return;
    }
  }
}

static uint8_t read (uint8_t *channel_id, uint8_t *high, uint8_t *low)
{
  uint8_t result = 0;

  ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
    if (buf_size (&reading_buf) < READING_SIZE)
      return 0;

    result = ((buf_byte_drain (&reading_buf, channel_id) != 0)
              && (buf_byte_drain (&reading_buf, high) != 0)
              && (buf_byte_drain (&reading_buf, low) != 0))
      ? 1 : 0;
  }

  return result;
}

static void select_channel (uint8_t channel_id)
{
  /* 
   * adjust adc input 
   */

  /* reset selector */
  ADMUX &= ~(1 << MUX0);

  /* select input */
  switch (channel) {
  case CHANNEL_CURRENT:
    /* do nothing, we use 0 channel for current */
    break;
  case CHANNEL_VOLTAGE:
    /* single ended, ADC1 input */
    ADMUX |= (1 << MUX0);
    break;
  default:
    break;
  }
}

static void adc_start ()
{
  /* ADCSRA |= (1 << ADSC) | (1 << ADIE); */
  ADCSRA |= (1 << ADSC);
}

/* static void adc_stop () */
/* { */
/*   ADCSRA &= ~((1 << ADSC) | (1 << ADIE)); */
/* } */

void adc_try ()
{
  uint8_t channel_id = 0, high = 0, low = 0;

  if (read (&channel_id, &high, &low) == 0)
    return;

  if (channel_id == CHANNEL_CURRENT)
    current_value (high, low);
  else if (channel_id == CHANNEL_VOLTAGE)
    voltage_value (high, low);
}

ISR (ADC_vect)
{
  /* channel_stop (); */
  /* channel_toggle (); */

  const uint8_t low = ADCL;
  const uint8_t high = ADCH;

  write (channel, high, low);

  state = STATE_ADC_COMPLETED;
}

static void kick ()
{
  switch (state) {
  case STATE_ADC_COMPLETED:
    /* toggle channel */
    channel = (channel == CHANNEL_CURRENT) ? CHANNEL_VOLTAGE : CHANNEL_CURRENT;
    /* adjust mux */
    select_channel (channel);
    /* change state */
    state = STATE_CHANNEL_SELECTED;
    break;
  case STATE_ADC_STARTED:
    /* do nothing, wait for ADC */
    break;
  case STATE_CHANNEL_SELECTED:
    /* start conversion */
    adc_start ();
    /* change state */
    state = STATE_ADC_STARTED;
    break;
  default:
    break;
  }
}

void adc_init ()
{
  buf_init (&reading_buf);

  /* 1.1V as a reference voltage */
  ADMUX |= (1 << REFS0) | (1 << REFS1);

  /* disable digital input to save electricity */
  DIDR0 |= (1 << ADC0D) | (1 << ADC1D) | (1 << ADC2D);

  /*enable adc, enable interrupt, */
  /* 128 clock division factor*/
  /* fixme: is the clock div factor correct? check later */
  /* fixme: do we need to enable intr here or after channel switch ? */
  ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE);

  channel = CHANNEL_CURRENT;
  select_channel (channel);
  state = STATE_CHANNEL_SELECTED;

  cron_enable (CRON_ID_ADC, CRON_FACTOR, &kick);


}

