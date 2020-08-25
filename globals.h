#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#ifndef DEFINE_GLOBALS
#define EXTERN extern
#else
#define EXTERN
#endif

EXTERN int g_timezone;

typedef enum {
    SHOW_TIME,
    SELECT_TIMEZONE
} state;

EXTERN state g_state;

#endif
