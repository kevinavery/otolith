
#include "user_alarm.h"
#include "nordic_common.h"
#include "util.h"


#define MINUTE_IN_TICKS          APP_TIMER_TICKS(60*1000, 0)
#define DAY_IN_MINUTES           24*60;

static app_timer_id_t            m_user_alarm_timer_id;
static user_alarm_evt_handler_t  m_evt_handler;
static uint16_t                  m_remaining_minutes;


static void minute_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    m_remaining_minutes--;
    
    // if it has expired
    if (!m_remaining_minutes)
    {
        //mlog_str("User alarm expired!\r\n");
        // invoke the application event
        if (m_evt_handler)
            m_evt_handler();
    
        m_remaining_minutes = DAY_IN_MINUTES;
    }
}

uint32_t user_alarm_init(user_alarm_evt_handler_t evt_handler)
{
    m_evt_handler = evt_handler;
    
    m_remaining_minutes = 0;
    
    // Create a repeating alarm that expires every minute
    return app_timer_create(&m_user_alarm_timer_id,
                            APP_TIMER_MODE_REPEATED,
                            minute_timeout_handler);
}

void user_alarm_set(uint16_t minutes_until_expire)
{
    // Stop current timer (although it may not even be running)
    app_timer_stop(m_user_alarm_timer_id);
    
    // Set remaining minutes
    m_remaining_minutes = minutes_until_expire;
    
    // Start timer
    app_timer_start(m_user_alarm_timer_id, MINUTE_IN_TICKS, NULL);
}

void user_alarm_clear()
{
    app_timer_stop(m_user_alarm_timer_id);
}


