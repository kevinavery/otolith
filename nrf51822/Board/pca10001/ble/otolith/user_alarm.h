
#ifndef USER_ALARM_H__
#define USER_ALARM_H__

#include <stdint.h>
#include "app_timer.h"

/* user_alarm event handler type. */
typedef void (*user_alarm_evt_handler_t) (void);

/*
 Initializes the user_alarm. Should be called immediately after APP_TIMER_INIT
 */
uint32_t user_alarm_init(user_alarm_evt_handler_t);

/*
 Sets when the user_alarm should expire, clearing existing alarm if neccessary.
 */
void user_alarm_set(uint16_t minutes_until_expire);

/*
 Clears the user_alarm.
 */
void user_alarm_clear(void);

#endif
