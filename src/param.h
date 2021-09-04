/*
 * List param ids here
 */
#ifndef SUPPLY_PARAM_H
#define SUPPLY_PARAM_H

enum {
  PARAM_CLOCK,                  /* not for 'update ! */
  PARAM_CLOCK_HOUR,
  PARAM_CLOCK_MINUTE,
  PARAM_CURRENT,                /* decimal point number */
  PARAM_DEBUG_IN,
  PARAM_DEBUG_OUT,              /* 'read-only' */
  PARAM_DISCONNECT,
  PARAM_DISCONNECT_HOUR,
  PARAM_DISCONNECT_MINUTE,
  PARAM_FAN,
  PARAM_VOLTAGE,                /* decimal point number */
  PARAM_SIZE,                   /* keep it last ! */
};

#endif
