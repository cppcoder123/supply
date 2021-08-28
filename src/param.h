/*
 * List param ids here
 */
#ifndef SUPPLY_PARAM_H
#define SUPPLY_PARAM_H

enum {
  PARAM_CURRENT,                /* decimal point number */
  PARAM_DEBUG_IN,
  PARAM_DEBUG_OUT,              /* 'read-only' */
  PARAM_FAN,
  PARAM_SHUTDOWN,               /* not for update ! */
  PARAM_SHUTDOWN_ENABLE,        /* text */
  PARAM_SHUTDOWN_HOUR,
  PARAM_SHUTDOWN_MINUTE,
  PARAM_VOLTAGE,                /* decimal point number */
  PARAM_WATCH,                  /* not for 'update ! */
  PARAM_WATCH_HOUR,
  PARAM_WATCH_MINUTE,
  PARAM_SIZE,                   /* keep it last ! */
};

#endif
