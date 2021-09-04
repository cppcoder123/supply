/*
 *
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "adc.h"
#include "clock.h"
#include "cron.h"
#include "current.h"
#include "dac.h"
#include "debug.h"
#include "fan.h"
#include "gui.h"
#include "led.h"
#include "relay.h"
#include "rotor.h"
#include "timer.h"
#include "twi.h"
#include "voltage.h"

static void decelerate ()
{
  /* enable clock change */
  CLKPR |= (1 << CLKPCE);

  /* set new clock prescaler => divide clock speed by 4*/
  /* CLKPR |= (1 << CLKPS1); */
  CLKPR |= (1 << CLKPS3);
  /* CLKPR |= (1 << CLKPS0) | (1 << CLKPS0); */
}

static void init ()
{
  gui_init ();                  /* gui is first */

  current_init ();
  debug_init ();
  led_init ();
  relay_init ();

  cron_init ();                 /* cron before adc & fan */
  adc_init ();
  fan_init ();
  rotor_init ();
  timer_init ();
  twi_init ();                  /* twi before clock & dac */
  voltage_init ();

  clock_init ();
  dac_init ();
}

int main ()
{
  decelerate ();

  init ();

  while (1) {
    adc_try ();
    cron_try ();
    fan_try ();
    rotor_try ();
    timer_try ();
    twi_try ();
  }
}
