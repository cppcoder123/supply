/*
 *
 */

#include <avr/io.h>

#include <stdint.h>


#include "cron.h"
/* #include "dac.h" */
#include "drive.h"
#include "fan.h"
#include "feedback.h"
#include "gui.h"
#include "param.h"
#include "timer.h"
#include "update.h"

/* fixme, needed rev/min */
/* #define FEEDBACK_TARGET 30 */
/* fixme, needed parameter accuracy, 10% ? */
#define FEEDBACK_DELTA 2
/* fixme, how responsive is feedback */
/* #define FEEDBACK_DELAY 2 */
/* ignore first measurements */
#define FEEDBACK_IGNORE 3

#define PWM_TIMER TIMER_0
#define PWM_PRESCALER TIMER_01_PRESCALER_1
#define PWM_FREQUENCY 160
#define PWM_MAX PWM_FREQUENCY
#define PWM_MIN 2
/* #define PWM_START ((PWM_MAX + PWM_MIN) / 2)*/
#define PWM_START PWM_MIN
#define PWM_DELTA_FINE 1
#define PWM_DELTA_ROUGH 5

#define METER_TIMER TIMER_1
#define METER_PRESCALER TIMER_01_PRESCALER_EXT_RISE
/* measure frequency, once per 5 sec?*/
#define METER_FREQUENCY 250
/* difference is too small => no correction is needed */
#define METER_DELTA_TINY FEEDBACK_DELTA
/* diff is small, correct carefully*/
#define METER_DELTA_FINE (3 * FEEDBACK_DELTA)

#define FAN_ZERO 0

#define PWM_BIT PORTD6
/* #define PWM_PORT PORTE */
#define PWM_DDR DDRD
/* negative polarity? it needs to be checked */
#define PWM_POLARITY 0

#define ON 1
#define OFF 0

/* limit current during the fan start */
/* fixme: find the right value here */
#define LIMIT_HIGH 0x05
#define LIMIT_LOW 0x55

/* #define FAN_INIT_DURATION 255 */

#define SPEED_MIN 20
#define SPEED_MAX 100


static struct feedback_t feedback;
static uint8_t speed = 0;
static uint8_t pwm_value = PWM_MAX;

void fan_try ()
{
  /* fixme */
  if (speed == 0)
    return;

  feedback_try (&feedback);
}

static void pwm (uint8_t arg)
{
  if (arg == ON) {
    /* configure as output */
    PWM_DDR |= (1 << PWM_BIT);

    pwm_value = PWM_START;
    timer_register_write
      (PWM_TIMER, TIMER_OUTPUT_COMPARE_A, PWM_FREQUENCY, FAN_ZERO);
    timer_register_write
      (PWM_TIMER, TIMER_OUTPUT_COMPARE_B, pwm_value, FAN_ZERO);
    timer_pwm_enable (PWM_TIMER, PWM_POLARITY);
    timer_enable (PWM_TIMER, PWM_PRESCALER);
  } else {                      /* off */
    timer_disable (PWM_TIMER);
    timer_pwm_disable (PWM_TIMER, PWM_POLARITY);
    /* release pwm wire */
    PWM_DDR &= ~(1 << PWM_BIT);
  }
}

static void measure ()
{
  uint8_t low = 0, high = 0;
  timer_register_read (METER_TIMER, TIMER_VALUE, &low, &high);
  timer_register_write (METER_TIMER, TIMER_VALUE, FAN_ZERO, FAN_ZERO);

  if ((low > 0x7F) && (high < 0xFF))
    ++high;

  feedback_data (&feedback, high);

  /* if (high != FAN_ZERO) */
  /* debug_1 (DEBUG_FAN, 222, high); */
}

static void meter (uint8_t arg) /* frequency-meter */
{
  if (arg == ON) {
    timer_register_write (METER_TIMER, TIMER_VALUE, FAN_ZERO, FAN_ZERO);
    timer_enable (METER_TIMER, METER_PRESCALER);
    cron_enable (CRON_ID_FAN, METER_FREQUENCY, &measure);
  } else {                      /* off */
    cron_disable (CRON_ID_FAN);
    timer_disable (METER_TIMER);
  }
}

static uint8_t get_pwm_delta (uint8_t meter_delta)
{
  if (meter_delta <= METER_DELTA_TINY)
    return FAN_ZERO;
  if (meter_delta <= METER_DELTA_FINE)
    return PWM_DELTA_FINE;

  return PWM_DELTA_ROUGH;
}

static void control (uint8_t current)
{
  /* fixme */
  gui_update (PARAM_FAN, current);

  uint8_t need_more = (current < speed) ? 1 : 0;
  uint8_t meter_delta = (need_more > 0) ? speed - current : current - speed;
  uint8_t pwm_delta = get_pwm_delta (meter_delta);

  /* if ((dac_check (DAC_MIN) == 0) */
  /*     && (need_more == 0)) */
  /*   dac_rough_set (DAC_MIN); */
  if ((drive_get () != DRIVE_OPEN)
      && (need_more == 0))
    drive_set (DRIVE_OPEN);

  if (pwm_delta == FAN_ZERO) {
    /* debug_1 (DEBUG_FAN, 111, meter_delta); */
    return;
  }

  pwm_value = (need_more > 0)
    ? ((pwm_value + pwm_delta < PWM_MAX) ? pwm_value + pwm_delta : PWM_MAX)
    : (((pwm_value > pwm_delta) && (pwm_value - pwm_delta > PWM_MIN))
       ? pwm_value - pwm_delta : PWM_MIN);

  /* debug_1 (DEBUG_FAN, 123, pwm); */

  /* fixme */
  /* pwm = 160; */

  timer_register_write (PWM_TIMER,
                          TIMER_OUTPUT_COMPARE_B, pwm_value, FAN_ZERO);
}

void change_speed (uint8_t new_speed)
{
  if (new_speed > SPEED_MAX)
    new_speed = SPEED_MAX;

  if (new_speed == speed)
    return;

  if ((speed == 0) && (new_speed > 0)) {
    /* start */
    feedback_init (&feedback, &speed,
                   FEEDBACK_DELTA, FEEDBACK_IGNORE, &control);
    pwm (ON);
    meter (ON);
    /* dac_set (LIMIT_HIGH, LIMIT_LOW); */
    drive_set (DRIVE_LIMIT);
  } else if ((speed > 0) && (new_speed == 0)) {
    pwm (OFF);
    meter (OFF);
    /* max value disables fan power */
    /* dac_rough_set (DAC_MAX); */
    drive_set (DRIVE_CLOSE);
  }

  speed = new_speed;
}

void fan_update (uint8_t *param, uint8_t update_id)
{
  switch (update_id) {
  case UPDATE_DEC:
    if (*param > SPEED_MIN)
      --(*param);
    else
      *param = 0;
    break;
  case UPDATE_INC:
    if (*param == 0)
      *param = SPEED_MIN;
    else if (*param < SPEED_MAX)
      ++(*param);
    break;
  case UPDATE_SWAP:
    {
      /* we can't change speed instantly => report the old one */
      uint8_t old_speed = speed;
      change_speed (*param);
      *param = old_speed;
    }
    break;
  default:
    break;
  }
}

void fan_init ()
{
  /* fixme */

  /* initial state is off */
  speed = 0;

  gui_update (PARAM_FAN, speed);
}
