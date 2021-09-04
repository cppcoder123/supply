/*
 *
 */
#include <avr/interrupt.h>
#include <avr/io.h>

#include "bool.h"
#include "buf.h"
#include "clock.h"
#include "gui.h"
#include "param.h"
#include "relay.h"
#include "twi.h"
#include "update.h"

#define TWI_ADDRESS 0x68

#define REG_CONTROL 0x0e
#define REG_ENABLE REG_CONTROL

#define REG_STATUS 0x0f
#define REG_ENABLE_32KHZ REG_STATUS

#define REG_SECOND 0
#define REG_MINUTE 1
#define REG_HOUR 2

#define MAX_HOUR 23
#define MAX_MINUTE 59
#define INVALID_HOUR 99

/*
 *     Bit: 7     6     5    4   3   2     1    0
 *          !EOSC BBSQW CONV RS2 RS1 INTCN A2IE A1IE
 * enable:  0     0     0    0   0   0     0    0
 * disable: 0     0     0    0   0   1     0    0
 *
 * RS2 == RS1 == 0 => 1Hz
 * INTCN 0 enables SQW pulse signal
 */
#define REG_VALUE_ENABLE 0
#define REG_VALUE_DISABLE 4

/*
 * It looks 32kHz signal is enabled by default,
 * Do we need to disable it?
 *
 *  Status register:
 *
 *     Bit: 7   6 5 4 3     2    1   0
 *          OSF R R R 32kHz Busy A2F A1F
 * enable:  0   0 0 0 1     0    0   0
 * disable: 0   0 0 0 0     0    0   0
 *
 *   => 0 disables 32kHz
 */
#define REG_VALUE_DISABLE_32KHZ 0

/* set initial time */
#define INITIAL_TIME 1

/* 10 pixels from left */
#define IMAGE_INDENT 10

/*
 * Buffer, only for read
 */
enum {
      BUFFER_SECOND,
      BUFFER_MINUTE,
      BUFFER_HOUR,
      BUFFER_SIZE               /* ! Keep it last */
};

static uint8_t read_buffer[BUFFER_SIZE];

enum {
      RTC_TO,                   /* convert time to ds3231 format */
      RTC_FROM,                 /* from */
      RTC_HOUR,
      RTC_MINUTE,
      RTC_SECOND,
};

/*
 * Current hour and minute values
 */
static uint8_t hour = 0;
static uint8_t minute = 0;

static uint8_t disconnect_hour = INVALID_HOUR;
static uint8_t disconnect_minute = 0;

/* fixme: add disconnect functionality */

/*
 * Convert hour/minute/second to/from ds3231 format.
 *
 * hours:
 *    Assume 24h mode:
 *    Right for bits are hours (less than 10),
 *    bits 5 and 6 represent 10-nth of hours
 *
 * minutes/seconds:
 *    Right 4 bits are minutes (less than 10),
 *    bits 5,6,7 are tens of seconds
 */
static uint8_t rtc (uint8_t src, uint8_t direction, uint8_t unit)
{
  uint8_t result = 0;

  if (direction == RTC_TO) {
    if ((unit == RTC_HOUR)
        && (src > 23))
      src = 23;
    if ((unit != RTC_HOUR)
        && (src > 59))
      src = 59;
    result = ((src / 10) << 4) | (src % 10);
  } else {                      /* from rtc */
    const uint8_t tens_mask = (unit == RTC_HOUR) ? 0x3 : 0x7;
    result = ((src >> 4) & tens_mask) * 10 + (src & 0xf);
  }

  return result;
}

static void render ()
{
  uint8_t tmp = rtc (read_buffer[BUFFER_HOUR], RTC_FROM, RTC_HOUR);
  if (tmp != hour)
    gui_update (PARAM_CLOCK_HOUR, tmp);
  hour = tmp;

  tmp = rtc (read_buffer[BUFFER_MINUTE], RTC_FROM, RTC_MINUTE);
  if (tmp != minute)
    gui_update (PARAM_CLOCK_MINUTE, tmp);
  minute = tmp;

  if ((hour == disconnect_hour)
      && (minute == disconnect_minute))
    relay_switch (BOOL_OFF);
}

static void read_callback (uint8_t tag, uint8_t status, uint8_t len,
                           volatile uint8_t *value)
{
  if ((status != TWI_SUCCESS)
      || (tag >= BUFFER_SIZE)
      || (len < 1))
    return;

  read_buffer[tag] = value[0];

  render ();
}

static void init_write_buffer (uint8_t *write_buffer)
{
  for (uint8_t i = 0; i < BUFFER_SIZE; ++i)
    *(write_buffer + i) = read_buffer[i];
}

static void update_value (uint8_t *param, uint8_t update_id, uint8_t max_value)
{
  switch (update_id) {
  case UPDATE_DEC:
    if (*param > 0)
      --(*param);
    break;
  case UPDATE_INC:
    if (*param < max_value)
      ++(*param);
    break;
  default:
    break;
  }
}

void clock_update (uint8_t *param, uint8_t param_id, uint8_t update_id)
{
  uint8_t max_value
    = ((param_id == PARAM_CLOCK_HOUR)
       || (param_id == PARAM_DISCONNECT_HOUR)) ? MAX_HOUR : MAX_MINUTE;

  switch (update_id) {
  case UPDATE_DEC:
    if ((param_id == PARAM_DISCONNECT_HOUR)
        && (*param == INVALID_HOUR))
      *param = MAX_HOUR;
    else
      update_value (param, update_id, max_value);
    break;
  case UPDATE_INC:
    if ((param_id == PARAM_DISCONNECT_HOUR)
        && (*param == MAX_HOUR))
      *param = INVALID_HOUR;
    else
      update_value (param, update_id, max_value);
    break;
  case UPDATE_SWAP:
    {
      uint8_t write_buffer[BUFFER_SIZE];

      switch (param_id) {
      case PARAM_CLOCK_HOUR:
        init_write_buffer (write_buffer);
        write_buffer[BUFFER_HOUR] = *param;
        twi_write_array (TWI_ID_CLOCK, 0,
                         BUFFER_SIZE, REG_SECOND, write_buffer);
        break;
      case PARAM_CLOCK_MINUTE:
        init_write_buffer (write_buffer);
        write_buffer[BUFFER_MINUTE] = *param;
        twi_write_array (TWI_ID_CLOCK, 0,
                         BUFFER_SIZE, REG_SECOND, write_buffer);
        break;
      case PARAM_DISCONNECT_HOUR:
        disconnect_hour = *param;
        break;
      case PARAM_DISCONNECT_MINUTE:
        disconnect_minute = *param;
        break;
      default:
        break;
      }
    }
    break;
  default:
    break;
  }
}

/* void clock_hour_update (uint8_t *param, uint8_t update_id) */
/* { */
/*   switch (update_id) { */
/*   case UPDATE_DEC: */
/*     if (*param > 0) */
/*       --(*param); */
/*     break; */
/*   case UPDATE_INC: */
/*     if (*param < 24) */
/*       ++(*param); */
/*     break; */
/*   case UPDATE_SWAP: */
/*     read_buffer[BUFFER_HOUR] = *param; */
/*     twi_write_array (TWI_ID_CLOCK, 0, BUFFER_SIZE, REG_SECOND, read_buffer); */
/*     break; */
/*   default: */
/*     return; */
/*     break; */
/*   } */
/*   /\* fixme *\/ */
/* } */

/* void clock_minute_update (uint8_t *param, uint8_t update_id) */
/* { */
/*   switch (update_id) { */
/*   case UPDATE_DEC: */
/*     if (*param > 0) */
/*       --(*param); */
/*     break; */
/*   case UPDATE_INC: */
/*     if (*param < 60) */
/*       ++(*param); */
/*     break; */
/*   case UPDATE_SWAP: */
/*     read_buffer[BUFFER_MINUTE] = *param; */
/*     twi_write_array (TWI_ID_CLOCK, 0, BUFFER_SIZE, REG_SECOND, read_buffer); */
/*     break; */
/*   default: */
/*     return; */
/*     break; */
/*   } */
/*   /\* fixme: ignore seconds *\/ */
/* } */

void clock_init ()
{
  /*
   * Register for 1Hz signal
   *
   * 1 & 1 => rising edge
   */
  EICRA |= ((1 << ISC01) | (1 << ISC00));
  /*
   * Enable INT2 interrupt
   */
  EIMSK |= (1 << INT0);

  for (uint8_t i = 0; i < BUFFER_SIZE; ++i)
    read_buffer[i] = 0;

  hour = INITIAL_TIME;
  minute = INITIAL_TIME;

  twi_slave_r (TWI_ID_CLOCK, TWI_ADDRESS, &read_callback);

  twi_write_byte (TWI_ID_CLOCK, 0, REG_ENABLE_32KHZ, REG_VALUE_DISABLE_32KHZ);

  uint8_t epoch[BUFFER_SIZE];
  epoch[BUFFER_SECOND] = rtc (INITIAL_TIME, RTC_TO, RTC_SECOND);
  epoch[BUFFER_MINUTE] = rtc (INITIAL_TIME, RTC_TO, RTC_MINUTE);
  epoch[BUFFER_HOUR] = rtc (INITIAL_TIME, RTC_TO, RTC_HOUR);

  twi_write_array (TWI_ID_CLOCK, 0, BUFFER_SIZE, REG_SECOND, epoch);

  gui_update (PARAM_CLOCK_HOUR, INITIAL_TIME);
  gui_update (PARAM_CLOCK_MINUTE, INITIAL_TIME);

 /* fixme: */
}

ISR (INT0_vect)
{
  /* ! second should be the last one */
  /*
   */
  /* fixme: we don't need seconds, hour & minute should be enough, remove it later */
  twi_read_byte (TWI_ID_CLOCK, BUFFER_HOUR, REG_HOUR);
  twi_read_byte (TWI_ID_CLOCK, BUFFER_MINUTE, REG_MINUTE);
  twi_read_byte (TWI_ID_CLOCK, BUFFER_SECOND, REG_SECOND);
  /* twi_read_array (TWI_ID_CLOCK, 0, BUFFER_SIZE, REG_SECOND); */
}
