/*
 *
 */

#include <avr/interrupt.h>
#include <avr/io.h>


#include "adc.h"
#include "buf.h"
#include "current.h"
#include "voltage.h"

enum {
  CHANNEL_CURRENT,
  CHANNEL_VOLTAGE,
};

enum {
  COMMAND_TOGGLE,               /* toggle input channel */
  COMMAND_START,
};

static uint8_t channel = 0;

static struct buf_t command_buf;
static struct buf_t current_buf;
static struct buf_t voltage_buf;

static void write (struct buf_t *buf, uint8_t high, uint8_t low)
{
  if (buf_space (buf) < 2)
    return;

  if (buf_byte_fill (buf, high) == 0)
    return;

  if (buf_byte_fill (buf, low) == 0) {
    /* error: try to drain high */
    buf_byte_drain (buf, &high);
    return;
  }
}

static uint8_t read (struct buf_t *buf, uint8_t *high, uint8_t *low)
{
  if ((buf_size (buf) < 2)
      || (buf_byte_drain (buf, high) == 0)
      || (buf_byte_drain (buf, low) == 0))  /* ref-1 */
    return 0;

  /* ref-1 */
  /* fixme: high is ok, but low is bad, how to handle this ? */
  
  return 1;
}

static void channel_toggle ()
{
  /* 
   * 1. change the channel id
   */
  channel = (channel == CHANNEL_CURRENT) ? CHANNEL_VOLTAGE : CHANNEL_CURRENT;

  /* 
   * 2. adjust adc input 
   */

  /* 2.1. reset selector */
  ADMUX &= ~((1 << MUX3) | (1 << MUX1) | (1 << MUX0));

  /* 2.2. select input(s) */
  switch (channel) {
  case CHANNEL_CURRENT:
    /* diff input ADC0 & ADC1, 10x gain */
    ADMUX |= (1 << MUX3) | (1 << MUX0);
    break;
  case CHANNEL_VOLTAGE:
    /* single ended, ADC2 input */
    ADMUX |= (1 << MUX1);
    break;
  default:
    break;
  }

  /*
   * 3. schedule measurement
   */
  buf_byte_fill (&command_buf, COMMAND_START);
}

static void channel_start ()
{
  /* ADCSRA |= (1 << ADSC) | (1 << ADIE); */
  ADCSRA |= (1 << ADSC);
}

/* static void channel_stop () */
/* { */
/*   ADCSRA &= ~((1 << ADSC) | (1 << ADIE)); */
/* } */

void adc_init ()
{
  buf_init (&command_buf);
  buf_init (&current_buf);
  buf_init (&voltage_buf);

  /* 2.56 as a reference voltage */
  ADMUX |= (1 << REFS0) | (1 << REFS1);

  /* disable digital input to save electricity */
  DIDR0 |= (1 << ADC0D) | (1 << ADC1D) | (1 << ADC2D);

  /*enable adc, enable interrupt, */
  /* 128 clock division factor*/
  /* fixme: is the clock div factor correct? check later */
  /* fixme: do we need to enable intr here or after channel switch ? */
  ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE);

  channel = CHANNEL_CURRENT;
  channel_start ();
}

void adc_try ()
{
  uint8_t high = 0, low = 0;

  if (read (&current_buf, &high, &low) != 0)
    current_value (high, low);
  
  if (read (&voltage_buf, &high, &low) != 0)
    voltage_value (high, low);

  if (buf_byte_drain (&command_buf, &high) != 0) {
    if (high == COMMAND_TOGGLE)
      channel_toggle ();
    else if (high == COMMAND_START)
      channel_start ();
  }
}

ISR (ADC_vect)
{
  /* channel_stop (); */
  /* channel_toggle (); */

  const uint8_t low = ADCL;
  const uint8_t high = ADCH;

  if (channel == CHANNEL_CURRENT)
    write (&current_buf, high, low);
  else if (channel == CHANNEL_VOLTAGE)
    write (&voltage_buf, high, low);

  buf_byte_fill (&command_buf, COMMAND_TOGGLE);

  /* switch (channel) { */
  /* case CHANNEL_0: */
  /*   buf_byte_fill (&buf_0, ADCH); */
  /*   break; */
  /* case CHANNEL_1: */
  /*   buf_byte_fill (&buf_1, ADCH); */
  /*   break; */
  /* case CHANNEL_2: */
  /*   buf_byte_fill (&buf_2, ADCH); */
  /*   break; */
  /* default: */
  /*   break; */
  /* } */

  /* channel_start (); */
}
