
/*
 * Otolith Service. 
 * 
 * Based on Battery Service Example ;)
 *
 */

#ifndef BLE_OTO_H__
#define BLE_OTO_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief Otolith Service event type. */
typedef enum
{
    BLE_OTO_EVT_NOTIFICATION_ENABLED,
    BLE_OTO_EVT_NOTIFICATION_DISABLED
} ble_oto_evt_type_t;

/**@brief Otolith Service event. */
typedef struct
{
    ble_oto_evt_type_t evt_type;                                  /**< Type of event. */
} ble_oto_evt_t;

// Forward declaration of the ble_oto_t type. 
typedef struct ble_oto_s ble_oto_t;

typedef void (*ble_oto_evt_handler_t) (ble_oto_t * p_oto, ble_oto_evt_t * p_evt);

typedef struct
{
    ble_oto_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Otolith Service. */
    bool                          support_notification;           /**< TRUE if notification of Step Count is supported. */
    ble_srv_report_ref_t *        p_report_ref;                   /**< If not NULL, a Report Reference descriptor with the specified value will be added to the Step Count characteristic */
    uint8_t                       initial_step_count;            
    ble_srv_cccd_security_mode_t  step_count_char_attr_md;        /**< Initial security level for step count characteristics attribute */
    ble_gap_conn_sec_mode_t       step_count_report_read_perm;    /**< Initial security level for step count report read attribute */
} ble_oto_init_t;

/**@brief Otolith Service structure. */
typedef struct ble_oto_s
{
    ble_oto_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Otolith Service. */
    uint16_t                      service_handle;                 /**< Handle of Otolith Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      step_count_handles;             /**< Handles related to the Step Count characteristic. */
    uint16_t                      report_ref_handle;              /**< Handle of the Report Reference descriptor. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    bool                          is_notification_supported;      /**< TRUE if notification of Step Count is supported. */
} ble_oto_t;


uint32_t ble_oto_init(ble_oto_t * p_oto, const ble_oto_init_t * p_oto_init);

void ble_oto_on_ble_evt(ble_oto_t * p_oto, ble_evt_t * p_ble_evt);

uint32_t ble_oto_send_step_count(ble_oto_t * p_oto, uint8_t step_count);

#endif // BLE_OTO_H__

/** @} */
