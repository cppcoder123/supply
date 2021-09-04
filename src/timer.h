/*
 *
 */
#ifndef SUPPLY_TIMER_H
#define SUPPLY_TIMER_H

#include <avr/io.h>

#include <stdint.h>

enum {                       /* prescaler values for 0 & 1 */
  TIMER_01_PRESCALER_0 = (0), /* default value, disables the counter */
  TIMER_01_PRESCALER_1 = (1 << CS00), /* no prescaling */
  TIMER_01_PRESCALER_8 = (1 << CS01),
  TIMER_01_PRESCALER_64 = ((1 << CS00) | (1 << CS01)),
  TIMER_01_PRESCALER_256 = (1 << CS02),
  TIMER_01_PRESCALER_1024 = ((1 << CS02) | (1 << CS00)),
  /* ext clock signal on Tn pin*/
  TIMER_01_PRESCALER_EXT_FALL = ((1 << CS02) | (1 << CS01)),
  TIMER_01_PRESCALER_EXT_RISE = ((1 << CS02) | (1 << CS01) | (1 << CS00)),
};

enum {
  TIMER_2_PRESCALER_0 = (0), /* default value, disables the counter */
  TIMER_2_PRESCALER_1 = (1 << CS20), /* no prescaling */
  TIMER_2_PRESCALER_8 = (1 << CS21),
  TIMER_2_PRESCALER_32 = ((1 << CS20) | (1 << CS21)),
  TIMER_2_PRESCALER_64 = (1 << CS22),
  TIMER_2_PRESCALER_128 = ((1 << CS20) | (1 << CS22)),
  TIMER_2_PRESCALER_256 = ((1 << CS21) | (1 << CS22)),
  TIMER_2_PRESCALER_1024 = ((1 << CS20) | (1 << CS21) | (1 << CS22)),
};

/* Note: pwm should be enabled when a register A is set */
enum {
  TIMER_PWM_A = (1 << 0),   /* toggle output A */
  TIMER_PWM_B = (1 << 1),   /* toggle output B */
  TIMER_PWM_INVERT = (1 << 2),  /* inverted PWM mode */
};

/* all our counters */
enum {
    TIMER_0,
    TIMER_1,
    TIMER_2,
};

/* registers we want to update */
enum {
    TIMER_OUTPUT_COMPARE_A,
    TIMER_OUTPUT_COMPARE_B,
    /* output_compare_c is not used */
    TIMER_VALUE,
};

/* interrupt action */
typedef void (*timer_callback) ();

/* init internal structure */
void timer_init ();

void timer_enable (uint8_t timer_id, uint8_t prescaler);
void timer_disable (uint8_t timer_id);

/*
 * Note:
 *    Only ctc compare-a interrupts are used now,
 *    so output-compare-a register should be updated before
 *    calling timer_enable
 */
void timer_interrupt_enable (uint8_t timer_id, timer_callback fun);
void timer_interrupt_disable (uint8_t timer_id);

/*
 * Fast pwm with output-compare-a & output-compare-b
 * these registers should be set and !!! b <= a !!!
 * before timer_enable call
 * if positive > 0 then positive pwm else negative
 */
void timer_pwm_enable (uint8_t timer_id, uint8_t flag);
void timer_pwm_disable (uint8_t timer_id, uint8_t flag);

/* only low is valid for counter 0, 2*/
void timer_register_write (uint8_t timer_id, uint8_t reg_id,
                           uint8_t low, uint8_t high);
void timer_register_read (uint8_t timer_id, uint8_t reg_id,
                          uint8_t *low, uint8_t *high);

/* perform tasks scheduled by interrupts */
void timer_try ();

#endif
