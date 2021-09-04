/*
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "buf.h"
#include "timer.h"

#define MAX_ID TIMER_2

#define PRESCALER_MASK ((1 << CS00) | (1 << CS01) | (1 << CS02))

/*
 * enable CTC
 * ----------
 *
 * WGM01, WGM21
 */
#define FLAG_8_ENABLE_CTC_COMPARE_A (1 << 1)
/*
 * WGM12, WGM32, WGM42, WGM52
 */
#define FLAG_16_ENABLE_CTC_COMPARE_A (1 << 3)

/*
 * Define Fast PWM with OCRA and switch A or B output
 *
 *
 * 8 bit counters
 * --------------
 *
 *
 * Register A:
 *
 * (1 << 0) - WGMx0
 * (1 << 1) - WGMx1
 *
 * (1 << 4) - COMxA0
 * (1 << 5) - COMxA1
 */
#define PWM_8_REG_A ((1 << WGM00) | (1 << WGM01))

#define PWM_8_REG_A_OUT_A (1 << COM0A1)
#define PWM_8_REG_A_OUT_A_INVERT (1 << COM0A0)

#define PWM_8_REG_A_OUT_B (1 << COM0B1)
#define PWM_8_REG_A_OUT_B_INVERT (1 << COM0B0)

/*
 * Register B:
 *
 * (1 << 3) - WGMx2
 */
#define PWM_8_REG_B (1 << WGM02)

/*
 * 16 bit counters (NOTE: pwm is not used here with 16 bit timer)
 * ---------------
 *
 * (1 << 0) - WGMx0 one of fast pwm flags
 * (1 << 1) - WGMx1
 * (1 << 5) - is non inverting mode,
 * (1 << 4) - together with (1 << 5) inverting mode
 */
#define PWM_16_REG_A ((1 << WGM10) | (1 << WGM11))

#define PWM_16_REG_A_OUT_A (1 << COM1A1)
#define PWM_16_REG_A_OUT_A_INVERT (1 << COM1A0)

#define PWM_16_REG_A_OUT_B (1 << COM1B1)
#define PWM_16_REG_A_OUT_B_INVERT (1 << COM1B0)

/*
 * (1 << 3) - WGMx2
 * (1 << 4) - WGMx3
 */
#define PWM_16_REG_B ((1 << WGM12) | (1 << WGM13))

/*
 * enable interrupt
 * ----------------
 */
#define FLAG_INTERRUPT_COMPARE_A (1 << 1)

enum {
  CONTROL_REGISTER_A,
  CONTROL_REGISTER_B,
  CONTROL_REGISTER_INTERRUPT,
};

typedef void (*handle_flag) (uint8_t timer_id, uint8_t reg_id, uint8_t flag);

static timer_callback callback_array[MAX_ID + 1];

static struct buf_t invoke_queue;

void timer_init ()
{
  for (uint8_t i = 0; i <= MAX_ID; ++i)
    callback_array[i] = 0;

  buf_init (&invoke_queue);
}

static uint8_t eight_bits (uint8_t id)
{
  return ((id == TIMER_0) || (id == TIMER_2)) ? 1 : 0;
}

static volatile uint8_t* control_register_get (uint8_t timer_id,
                                               uint8_t register_id)
{
  volatile uint8_t *reg_array[] =
    {
      &TCCR0A, &TCCR1A, &TCCR2A,
      &TCCR0B, &TCCR1B, &TCCR2B,
      &TIMSK0, &TIMSK1, &TIMSK2, 
    };
  const uint8_t factor = (register_id == CONTROL_REGISTER_B) ? 1
    : (register_id == CONTROL_REGISTER_INTERRUPT) ? 2
    : 0;

  return reg_array[timer_id + (MAX_ID + 1) * factor];
}

static void control_register_set (uint8_t timer_id,
                                  uint8_t register_id, uint8_t value)
{
  volatile uint8_t *reg = control_register_get (timer_id, register_id);
  *reg |= value;
}

static void control_register_clear (uint8_t timer_id,
                                    uint8_t register_id, uint8_t value)
{
  volatile uint8_t *reg = control_register_get (timer_id, register_id);
  *reg &= ~value;
}

void timer_enable (uint8_t id, uint8_t prescaler)
{
  if (id > MAX_ID)
    return;

  control_register_set (id, CONTROL_REGISTER_B, prescaler & PRESCALER_MASK);
}

void timer_disable (uint8_t id)
{
  if (id > MAX_ID)
    return;

  control_register_clear (id, CONTROL_REGISTER_B, PRESCALER_MASK);
}

static void timer_interrupt (uint8_t enable,
                               uint8_t timer_id, timer_callback fun)
{
  if ((timer_id > MAX_ID)
      || ((enable != 0)
          && (fun == 0)))
    return;

  uint8_t flag = (eight_bits (timer_id) != 0)
    ? FLAG_8_ENABLE_CTC_COMPARE_A : FLAG_16_ENABLE_CTC_COMPARE_A;
  uint8_t control_reg = (eight_bits (timer_id) != 0)
    ? CONTROL_REGISTER_A : CONTROL_REGISTER_B;
  handle_flag handle = (enable != 0)
    ? &control_register_set : &control_register_clear;

  if (enable != 0)
    callback_array[timer_id] = fun;

  handle (timer_id, control_reg, flag);
  handle (timer_id, CONTROL_REGISTER_INTERRUPT, FLAG_INTERRUPT_COMPARE_A);
}

void timer_interrupt_enable (uint8_t timer_id, timer_callback fun)
{
  timer_interrupt (1, timer_id, fun);
}

void timer_interrupt_disable (uint8_t timer_id)
{
  timer_interrupt (0, timer_id, 0);
}

static void timer_pwm (uint8_t enable, uint8_t timer_id, uint8_t flag)
{
  handle_flag handle = (enable != 0)
    ? &control_register_set : &control_register_clear;

  const uint8_t eight = eight_bits (timer_id);
  const uint8_t out_a = (flag & TIMER_PWM_A) ? 1 : 0;

  uint8_t reg_a = (eight) ? PWM_8_REG_A : PWM_16_REG_A;
  if (flag | TIMER_PWM_A)
    reg_a |= (eight) ? PWM_8_REG_A_OUT_A : PWM_8_REG_A_OUT_B;
  if (flag | TIMER_PWM_INVERT)
    reg_a |= (eight)
      ? ((out_a) ? PWM_8_REG_A_OUT_A_INVERT : PWM_8_REG_A_OUT_B_INVERT)
      : ((out_a) ? PWM_16_REG_A_OUT_A_INVERT : PWM_16_REG_A_OUT_B_INVERT);

  uint8_t reg_b = (eight) ? PWM_8_REG_B : PWM_16_REG_B;

  handle (timer_id, CONTROL_REGISTER_A, reg_a);
  handle (timer_id, CONTROL_REGISTER_B, reg_b);
}

void timer_pwm_enable (uint8_t timer_id, uint8_t flag)
{
  timer_pwm (1, timer_id, flag);
}

void timer_pwm_disable (uint8_t timer_id, uint8_t flag)
{
  timer_pwm (0, timer_id, flag);
}

static void value_register_get (uint8_t timer_id, uint8_t reg_id,
                                volatile uint8_t **reg_low, volatile uint8_t **reg_high)
{
  volatile uint8_t *reg_array[] =
    {
     &OCR0A, &OCR0A,            /*output compare a*/
     &OCR1AL, &OCR1AH,
     &OCR2A, &OCR2A,
     &OCR0B, &OCR0B,            /*output compare b*/
     &OCR1BL, &OCR1BH,
     &OCR2B, &OCR2B,
     &TCNT0, &TCNT0,            /* counter itself */
     &TCNT1L, &TCNT1H,
     &TCNT2, &TCNT2
  };

  const uint8_t shift = (reg_id == TIMER_OUTPUT_COMPARE_B) ? 12
    : (reg_id == TIMER_VALUE) ? 24 : 0;

  const uint8_t reg_array_id = timer_id * 2 + shift;

  *reg_low = reg_array[reg_array_id];
  if (eight_bits (timer_id) == 0)
    *reg_high = reg_array[reg_array_id + 1];
}

void timer_register_write (uint8_t timer_id,
                             uint8_t reg_id, uint8_t low, uint8_t high)
{
  volatile uint8_t *reg_low = 0;
  volatile uint8_t *reg_high = 0;
  value_register_get (timer_id, reg_id, &reg_low, &reg_high);

  /* ! high byte first */
  if (eight_bits (timer_id) == 0)
    *reg_high = high;
  *reg_low = low;
}

void timer_register_read (uint8_t timer_id,
                            uint8_t reg_id, uint8_t *low, uint8_t *high)
{
  volatile uint8_t *reg_low = 0;
  volatile uint8_t *reg_high = 0;
  value_register_get (timer_id, reg_id, &reg_low, &reg_high);

  /* ! low byte first */
  *low = *reg_low;
  if (eight_bits (timer_id) == 0)
    *high = *reg_high;
}

ISR(TIMER0_COMPA_vect)
{
  buf_byte_fill (&invoke_queue, TIMER_0);
}

ISR(TIMER1_COMPA_vect)
{
  buf_byte_fill (&invoke_queue, TIMER_1);
}

ISR(TIMER2_COMPA_vect)
{
  buf_byte_fill (&invoke_queue, TIMER_2);
}

void timer_try ()
{
  uint8_t id = 0;
  if ((buf_byte_drain (&invoke_queue, &id) == 0)
      || (id > MAX_ID))
    return;

  /* if (fun != 0) --- it doesn't work ---*/
  callback_array[id] ();
}
