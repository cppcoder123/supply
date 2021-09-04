/*
 *
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#include "buf.h"
#include "gui.h"
#include "rotor.h"

/*0,1, & 3,4*/
#define ALL_PINS ((1 << 0) | (1 << 1) | (1 << 3) | (1 << 4))

/* a & b inputs */
#define MASK_A (1 << 0)
#define MASK_B (1 << 1)

/* 
 * one knob takes 2 bits, we are ignoring push, 
 * and one wire beetwen rotor pins,
 * see "ALL_PINS"
*/
#define STEP 3

static struct buf_t event_buf;

void rotor_init ()
{
  buf_init (&event_buf);

  /* configure as inputs*/
  DDRD &= ~ALL_PINS;

  /* enable 3-rd series of pin change interrupts */
  PCICR |= (1 << PCIE2);

  /* set pin change mask to needed bits */
  PCMSK2 |= (1 << PCINT16) | (1 << PCINT17) | (1 << PCINT19) | (1 << PCINT20);

  /* enable internal pull-up resistors */
  PORTD |= ALL_PINS;
}

static uint8_t handle_event (uint8_t modern, uint8_t old, uint8_t oldest)
{
  uint8_t mask_a = MASK_A;
  uint8_t mask_b = MASK_B;
  uint8_t mask_both = (mask_a | mask_b);

  uint8_t status = 0;

  for (uint8_t i = ROTOR_ID_0; i <= ROTOR_ID_MAX; ++i) {
    if ((modern & mask_both) == 0) {
      if ((old & mask_a)
          && (oldest & mask_b)) {
        gui_rotor (i, ROTOR_FORWARD);
        status = 1;
      } else if ((old & mask_b)
                 && (oldest & mask_a)) {
        gui_rotor (i, ROTOR_BACKWARD);
        status = 1;
      }
    }
    mask_a <<= STEP;
    mask_b <<= STEP;
    mask_both = mask_a | mask_b;
  }

  return status;
}

static uint8_t queue_head (struct buf_t *buf,
                           uint8_t *modern, uint8_t *old, uint8_t *oldest)
{
  if (buf_size (buf) < 3)
    return 0;

  return ((buf_byte_get (buf, 0, oldest))
          && (buf_byte_get (buf, 1, old))
          && (buf_byte_get (buf, 2, modern)));
}

static void queue_drain (struct buf_t *buf, uint8_t drain_size)
{
  uint8_t dummy = 0;
  for (uint8_t i = 0; i < drain_size; ++i)
    buf_byte_drain (buf, &dummy);
}

void rotor_try ()
{
  /* current, previous and before previous events*/
  uint8_t modern = 0, old = 0, oldest = 0;
  if (queue_head (&event_buf, &modern, &old, &oldest)) {
      if (handle_event (modern, old, oldest))
        /*
         * Succeded to drain  the queue,
         * and succeded to handle it, drain all 3 events
         */
        queue_drain (&event_buf, 3);
      else
        /*        
         * Queue handling is failed => bouncing ?
         * Discard the oldest event
         */
        queue_drain (&event_buf, 1);
  }
}

ISR (PCINT2_vect)
{
  /* debug_2 (DEBUG_ROTOR, 22, 22, ~(PIND & ALL_PINS)); */
  buf_byte_fill (&event_buf, ~(PIND & ALL_PINS));
}
