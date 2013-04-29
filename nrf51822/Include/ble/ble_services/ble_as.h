
/*
 * Alarm Service. 
 * 
 * Based on Immediate Alert Service example ;)
 *
 */


#ifndef BLE_AS_H__
#define BLE_AS_H__

#include <stdint.h>
#include "ble.h"

// Forward declaration of the ble_as_t type. 
typedef struct ble_as_s ble_as_t;

/* Alarm Service event handler type. */
typedef void (*ble_as_evt_handler_t) (uint16_t updated_alarm_time);

/* Alarm Service init structure. */
typedef struct
{
    ble_as_evt_handler_t evt_handler;                  /**< Event handler to be called for handling events in the service. */
} ble_as_init_t;

/*
 * Alarm Service structure. This contains various status information for the service. 
 */
typedef struct ble_as_s
{
    ble_as_evt_handler_t      evt_handler;              
    uint16_t                  service_handle;           
    ble_gatts_char_handles_t  alarm_time_handles;      
} ble_as_t;

/*
 * Initialize the Alarm Service.
 */
uint32_t ble_as_init(ble_as_t * p_as, const ble_as_init_t * p_as_init);

/*
 * Alarm Service BLE stack event handler.
 */
void ble_as_on_ble_evt(ble_as_t * p_as, ble_evt_t * p_ble_evt);

/*
 * Get current value of the Alarm Time characteristic.
 */
uint32_t ble_as_alarm_time_get(ble_as_t * p_as, uint16_t * p_alarm_time);

#endif // BLE_AS_H__

