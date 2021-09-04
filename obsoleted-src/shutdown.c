/*
 *
 */

#include "gui.h"
#include "shutdown.h"
#include "param.h"

void shutdown_enable_update (uint8_t *param, uint8_t update_id)
{
  /* fixme */
}

void shutdown_hour_update (uint8_t *param, uint8_t update_id)
{
  /* fixme */
}

void shutdown_minute_update (uint8_t *param, uint8_t update_id)
{
  /* fixme */
}

void shutdown_init ()
{
  gui_update (PARAM_SHUTDOWN_HOUR, 0);
  gui_update (PARAM_SHUTDOWN_MINUTE, 0);
  /* fixme */
}
