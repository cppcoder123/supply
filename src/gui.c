/*
 *
 *
 *
 */
 
#include "bool.h"
#include "clock.h"
#include "current.h"
#include "debug.h"
#include "fan.h"
#include "gui.h"
#include "led.h"
#include "param.h"
#include "rotor.h"
#include "row.h"
#include "update.h"
#include "voltage.h"

/*
 * Define symbols we want to display
 */
#define SEG_POINT (1 << 7)
#define SEG_A (1 << 6)
#define SEG_B (1 << 5)
#define SEG_C (1 << 4)
#define SEG_D (1 << 3)
#define SEG_E (1 << 2)
#define SEG_F (1 << 1)
#define SEG_G (1)

#define CHAR_DASH SEG_G
#define CHAR_POINT SEG_POINT
#define CHAR_0 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define CHAR_1 (SEG_B | SEG_C)
#define CHAR_2 (SEG_A | SEG_B | SEG_G | SEG_E | SEG_D)
#define CHAR_3 (SEG_A | SEG_B | SEG_G | SEG_C | SEG_D)
#define CHAR_4 (SEG_F | SEG_G | SEG_B | SEG_C)
#define CHAR_5 (SEG_A | SEG_F | SEG_G | SEG_C | SEG_D)
#define CHAR_6 (SEG_A | SEG_F | SEG_E | SEG_D | SEG_C | SEG_G)
#define CHAR_7 (SEG_A | SEG_B | SEG_C)
#define CHAR_8 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define CHAR_9 (SEG_A | SEG_B | SEG_G | SEG_F | SEG_C | SEG_D)

#define CHAR_A (SEG_A | SEG_B | SEG_C | SEG_F | SEG_E | SEG_G)
#define CHAR_b (SEG_F | SEG_E | SEG_G | SEG_D | SEG_C)
#define CHAR_C (SEG_A | SEG_F | SEG_E | SEG_D)
#define CHAR_d (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)
#define CHAR_E (SEG_A | SEG_F | SEG_E | SEG_D | SEG_G)
#define CHAR_F (SEG_A | SEG_F | SEG_E | SEG_G)
#define CHAR_H (SEG_F | SEG_E | SEG_B | SEG_C | SEG_G)
#define CHAR_I CHAR_1
#define CHAR_L (SEG_F | SEG_E | SEG_D)
#define CHAR_n (SEG_E | SEG_G | SEG_C)
#define CHAR_O CHAR_0
#define CHAR_P (SEG_A | SEG_F | SEG_E | SEG_G | SEG_B)
#define CHAR_r (SEG_E | SEG_G)
#define CHAR_S CHAR_5
#define CHAR_t (SEG_F | SEG_E | SEG_G | SEG_D)
#define CHAR_U (SEG_F | SEG_E | SEG_D | SEG_C | SEG_B)

/* bytes per one led chip */
#define INFO_SIZE 8

/* rotor handling functionality related constants */
#define KNOB_POINTER ROTOR_ID_0
#define KNOB_VALUE ROTOR_ID_1

/* display pointer */
#define POINTER_MIN 1
#define POINTER_MAX (POINTER_MIN << 7)

/*
 * Our display capabilities
 */
enum {
  ROW_0,
  ROW_1,
  ROW_2,
  ROW_3,
  ROW_SIZE,
};

static struct row_t *display[ROW_SIZE];

static void display_init ()
{
  static struct row_t row_0;
  static struct row_t row_1;
  static struct row_t row_2;
  static struct row_t row_3;

  row_init (&row_0, PARAM_VOLTAGE);
  row_init (&row_1, PARAM_CURRENT);
  row_init (&row_2, PARAM_CLOCK);
  row_init (&row_3, PARAM_DISCONNECT);

  display[ROW_0] = &row_0;
  display[ROW_1] = &row_1;
  display[ROW_2] = &row_2;
  display[ROW_3] = &row_3;
}

/* param value list */
static uint8_t param_list[PARAM_SIZE];


/* static uint8_t content[ROW_SIZE]; */

/*
 * Left bit means value selection, right bit means param selection
 *
 * 0-led either 00000001 or 00000010
 * 1-led either 00000100 or 00001000
 * 2-led either 00010000 or 00100000
 * 3-led either 01000000 or 10000000
 */
static uint8_t display_pointer;

/*
 *
 *
 *
 */

static uint8_t led_current ()
{
  uint8_t mask = 0x3;
  for (uint8_t i = 0; i < ROW_SIZE; ++i) {
    if (display_pointer & mask)
      return i;
    mask <<= 2;
  }

  /* we should not reach this point */
  return 0;
}

static uint8_t param_current ()
{
  uint8_t led = led_current ();
  uint8_t mask = 0x2;
  mask <<= led;

  if ((display_pointer & mask) == 0)
    return PARAM_SIZE;

  /* return content[led]; */
  return display[led]->param_id;
}

static uint8_t param_row (uint8_t param_id)
{
  for (uint8_t i = 0; i < ROW_SIZE; ++i)
    if (display[i]->param_id == param_id)
      return i;

  return ROW_SIZE;
}

/* static uint8_t param_is_text (uint8_t param_id) */
/* { */
/*   return (param_id == PARAM_SHUTDOWN_ENABLE) ? 1 : 0; */
/* } */

/* static uint8_t param_is_decimal (uint8_t param_id) */
/* { */
/*   return ((param_id == PARAM_CURRENT) */
/*           || (param_id == PARAM_VOLTAGE)) ? 1 : 0; */
/* } */

static void param_update (uint8_t param_id, uint8_t update_id)
{
  switch (param_id) {
  case PARAM_CURRENT:
    current_update (&param_list[param_id], update_id);
    break;
  case PARAM_DEBUG_IN:
    debug_in_update (&param_list[param_id], update_id);
    break;
  case PARAM_FAN:
    fan_update (&param_list[param_id], update_id);
    break;
  /* case PARAM_VOLTAGE: */
  /*   voltage_update (&param_list[param_id], update_id); */
  /*   break; */
  case PARAM_CLOCK_HOUR:
  case PARAM_DISCONNECT_HOUR:
    clock_update (&param_list[param_id], param_id, update_id);
    break;
  case PARAM_CLOCK_MINUTE: 
  case PARAM_DISCONNECT_MINUTE:
    clock_update (&param_list[param_id], param_id, update_id);
    break;
  default:
    break;
  }
}

static void render_label (uint8_t param, struct row_t *row)
{
  row_info_clear (row);

  switch (param) {
  case PARAM_CURRENT:
    row_add (row, CHAR_I);
    break;
  case PARAM_DEBUG_IN:
    row_add (row, CHAR_d);
    row_add (row, CHAR_DASH);
    row_add (row, CHAR_I);
    break;
  case PARAM_DEBUG_OUT:
    row_add (row, CHAR_d);
    row_add (row, CHAR_DASH);
    row_add (row, CHAR_O);
    break;
  case PARAM_FAN:
    row_add (row, CHAR_F);
    break;
  case PARAM_DISCONNECT:
    row_add (row, CHAR_d);
    break;
  case PARAM_DISCONNECT_HOUR:
    row_add (row, CHAR_d);
    row_add (row, CHAR_DASH);
    row_add (row, CHAR_H);
    break;
  case PARAM_DISCONNECT_MINUTE:
    row_add (row, CHAR_d);
    row_add (row, CHAR_DASH);
    row_add (row, CHAR_I);
    break;
  case PARAM_VOLTAGE:
    row_add (row, CHAR_U);
    break;
  case PARAM_CLOCK:
    row_add (row, CHAR_C);      /* clock */
    break;
  case PARAM_CLOCK_HOUR:
    row_add (row, CHAR_C);
    row_add (row, CHAR_DASH);
    row_add (row, CHAR_H);
    break;
  case PARAM_CLOCK_MINUTE:
    row_add (row, CHAR_C);
    row_add (row, CHAR_DASH);
    row_add (row, CHAR_I);      /* mInute */
    break;
  default:
    row_add (row, CHAR_7);
    break;
    
  }

  /* add space */
  row_add (row, 0);
}

/* static void render_text (uint8_t value, struct row_t *row) */
/* { */
/*   if (value == BOOL_OFF) { */
/*     row_add (row, CHAR_O); */
/*     row_add (row, CHAR_F); */
/*     row_add (row, CHAR_F); */
/*   } else { */
/*     row_add (row, CHAR_O); */
/*     row_add (row, CHAR_n); */
/*   } */
/* } */

static uint8_t digit_image (uint8_t digit)
{
  uint8_t result = CHAR_F;

  switch (digit) {
  case 0:
    result = CHAR_0;
    break;
  case 1:
    result = CHAR_1;
    break;
  case 2:
    result = CHAR_2;
    break;
  case 3:
    result = CHAR_3;
    break;
  case 4:
    result = CHAR_4;
    break;
  case 5:
    result = CHAR_5;
    break;
  case 6:
    result = CHAR_6;
    break;
  case 7:
    result = CHAR_7;
    break;
  case 8:
    result = CHAR_8;
    break;
  case 9:
    result = CHAR_9;
    break;
  default:
    result = CHAR_P;
    break;
  }

  return result;
}

static void render_decimal (uint8_t value, struct row_t *row)
{
  if (value > 99) {
    row_add (row, digit_image (value / 100));
    value = value % 100;
  }

  if (value > 10) {
    row_add (row, digit_image (value / 10) | CHAR_POINT);
    value = value % 10;
  } else {
    row_add (row, CHAR_0 | CHAR_POINT);
  }

  row_add (row, digit_image (value));
}

static void render_uint8 (uint8_t value, struct row_t *row)
{
  if (value > 99) {
    row_add (row, digit_image (value / 100));
    value = value % 100;
  }

  if (value > 10) {
    row_add (row, digit_image (value / 10));
    value = value % 10;
  }

  row_add (row, digit_image (value));
}

static void render_clock (uint8_t hour, uint8_t minute, struct row_t *row)
{
  render_uint8 (hour, row);
  row_add (row, CHAR_DASH);
  render_uint8 (minute, row);
}

static void render_value (uint8_t param_id, struct row_t *row)
{
  uint8_t value = param_list[param_id];

  /* if (param_id == PARAM_SHUTDOWN_ENABLE) */
  /*   render_text (value, row); */
  /* else */
  if ((param_id == PARAM_CURRENT)
      || (param_id == PARAM_VOLTAGE))
    render_decimal (value, row);
  else if (param_id == PARAM_CLOCK)
    render_clock (param_list[PARAM_CLOCK_HOUR],
                  param_list[PARAM_CLOCK_MINUTE], row);
  else if (param_id == PARAM_DISCONNECT)
    render_clock (param_list[PARAM_DISCONNECT_HOUR],
                  param_list[PARAM_DISCONNECT_MINUTE], row);
  else
    render_uint8 (value, row);
}

static void render_pointer ()
{
  uint8_t led = led_current ();
  uint8_t param = param_current ();

  struct row_t *row = display[led];
  uint8_t *byte = &row->info[(param < PARAM_SIZE) ? 0 : INFO_SIZE - 1];
  *byte |= CHAR_POINT;
}

static void render ()
{
  for (uint8_t i = 0; i < ROW_SIZE; ++i) {
    struct row_t *row = display[i];
    /* uint8_t param = content[i]; */
    render_label (row->param_id, row);
    render_value (row->param_id, row);
  }

  /* invoke it after label & value */
  render_pointer ();

  /* send data to led chip */
  uint8_t data[LED_DISPLAY_SIZE];
  
  for (uint8_t r = 0; r < ROW_SIZE; ++r) {
    struct row_t *row = display[r];
    for (uint8_t c = 0; c < LED_ROW_SIZE; ++c)
      data[r * LED_ROW_SIZE + c] = row->info[c];
  }

  led_display (data);

}

void gui_update (uint8_t param_id, uint8_t value)
{
  if (param_current () == param_id)
    return;

  param_list[param_id] = value;

  const uint8_t row_id = param_row (param_id);
  if (row_id >= ROW_SIZE)
    return;

  /* the param can be displayed in more than one row, */
  /* the whole display should be refreshed */
  render ();
}

static void led_update (uint8_t led_id, uint8_t update_id)
{
  if (led_id >= ROW_SIZE)
    return;

  const uint8_t max_param = PARAM_SIZE - 1;
  uint8_t *param_id = &(display[led_id]->param_id);

  if (update_id == UPDATE_INC)
    *param_id = (*param_id < max_param) ? *param_id + 1 : 0;
  else if (update_id == UPDATE_DEC)
    *param_id = (*param_id > 0) ? *param_id - 1 : max_param;
}

void gui_rotor (uint8_t knob_id, uint8_t action)
{
  uint8_t param_id = param_current ();
  uint8_t update_id = (action == ROTOR_FORWARD) ? UPDATE_INC : UPDATE_DEC;

  if (knob_id == KNOB_POINTER) {
    if (param_id < PARAM_SIZE)
      param_update (param_id, UPDATE_SWAP);
    if (update_id == UPDATE_INC)
      display_pointer = (display_pointer < POINTER_MAX)
        ? (display_pointer << 1) : POINTER_MIN;
    else if (update_id == UPDATE_DEC) 
      display_pointer = (display_pointer > POINTER_MIN)
        ? (display_pointer >> 1) : POINTER_MAX;
  } else if (knob_id == KNOB_VALUE) {
    if (param_id < PARAM_SIZE)
      param_update (param_id, update_id);
    else
      led_update (led_current (), update_id);
  }

  render ();
}

void gui_init ()
{
  display_init ();

  for (uint8_t i = 0; i < PARAM_SIZE; ++i)
    param_list[i] = 0;

  /* content[ROW_0] = PARAM_VOLTAGE; */
  /* content[ROW_1] = PARAM_CURRENT; */
  /* content[ROW_2] = PARAM_CLOCK; */
  /* content[ROW_3] = PARAM_SHUTDOWN_ENABLE; */

  display_pointer = 1;
}

/* static void param_list_init (struct param_list_t *list) */
/* { */
/*   for (uint8_t i = 0; i < PARAM_SIZE; ++i) */
/*     list->value[i] = 0; */
/* } */

/* static struct param_list_t param_list; */
/* static struct param_list_t param_human; /\* wants to enter *\/ */
/* static struct param_list_t param_hw;    /\* received from HW *\/ */

/* there are less than 16 params, so 2 bytes should be enough */
/* #define FLAG_SIZE 2 */
/* #define BITS_PER_BYTE 8 */

/* struct flag_t { */
/*   uint8_t bit[FLAG_SIZE]; */
/* }; */

/* static void flag_byte_mask (struct flag_t *flag, */
/*                             uint8_t param_id, uint8_t **byte, uint8_t *mask) */
/* { */
/*   if (param_id < BITS_PER_BYTE) { */
/*     *byte = flag->bit; */
/*     *mask = (1 << param_id); */
/*     return; */
/*   } */

/*   *byte = flag->bit + 1; */
/*   *mask = (1 << (param_id - BITS_PER_BYTE)); */
/* } */

/* static void flag_set (struct flag_t *flag, uint8_t param_id) */
/* { */
/*   uint8_t *byte = 0; */
/*   uint8_t mask = 0; */

/*   flag_byte_mask (flag, param_id, &byte, &mask); */

/*   (*byte) |= mask; */
/* } */

/* static void flag_reset (struct flag_t *flag, uint8_t param_id) */
/* { */
/*   uint8_t *byte = 0; */
/*   uint8_t mask = 0; */

/*   flag_byte_mask (flag, param_id, &byte, &mask); */

/*   (*byte) &= ~mask; */
/* } */

/* static uint8_t flag_test (struct flag_t *flag, uint8_t param_id) */
/* { */
/*   uint8_t *byte = 0; */
/*   uint8_t mask = 0; */

/*   flag_byte_mask (flag, param_id, &byte, &mask); */

/*   return (*byte & mask) ? 1 : 0; */
/* } */

/* static uint8_t flag_any (struct flag_t *flag) */
/* { */
/*   for (uint8_t i = 0; i < FLAG_SIZE; ++i) */
/*     if (flag->bit[i] & 0xFF) */
/*       return 1; */

/*   return 0; */
/* } */

/* static void flag_init (struct flag_t *flag) */
/* { */
/*   for (uint8_t i = 0; i < FLAG_SIZE; ++i) */
/*     flag->bit[i] = 0; */
/* } */

/* static struct flag_t flag_human; */
/* static struct flag_t flag_hw; */

/* static void param_change (uint8_t param_id, uint8_t direction) */
/* { */
/*   if (param_is_text (param_id) != 0) { */
/*     param_list[param_id] = (param_list[param_id] == GUI_TEXT_ON) */
/*       ? GUI_TEXT_OFF : GUI_TEXT_ON; */
/*     return; */
/*   } */

/*   uint8_t *param = &param_list[param_id]; */
/*   if ((*param < 0xFF) */
/*       && (direction == INCREMENT)) */
/*     ++(*param); */
/*   if ((*param > 0) */
/*       && (direction == DECREMENT)) */
/*     --(*param); */
/* } */

