/*
 *
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "adc.h"
#include "counter.h"
#include "current.h"
#include "debug.h"
#include "fan.h"
#include "gui.h"
#include "power.h"
#include "rotor.h"
#include "shutdown.h"
#include "twi.h"
#include "voltage.h"
#include "watch.h"

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
  fan_init ();
  shutdown_init ();
  voltage_init ();

  adc_init ();
  counter_init ();
  rotor_init ();
  twi_init ();                  /* twi before watch & power */

  power_init ();
  watch_init ();
}

int main ()
{
  decelerate ();

  init ();

  while (1) {
    adc_try ();
    counter_try ();
    fan_try ();
    rotor_try ();
    twi_try ();
  }
}
