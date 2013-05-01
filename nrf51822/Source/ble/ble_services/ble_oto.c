
#include "ble_oto.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"  
#include "main.h" // for debug logging


/**@brief Connect event handler.
 */
static void on_connect(ble_oto_t * p_oto, ble_evt_t * p_ble_evt)
{
    p_oto->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Disconnect event handler.
 */
static void on_disconnect(ble_oto_t * p_oto, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_oto->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Write event handler.
 */
static void on_write(ble_oto_t * p_oto, ble_evt_t * p_ble_evt)
{
    if (p_oto->is_notification_supported)
    {
        ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
        
        if (
            (p_evt_write->handle == p_oto->step_count_handles.cccd_handle)
            &&
            (p_evt_write->len == 2)
        )
        {
            // CCCD written, call application event handler
            if (p_oto->evt_handler != NULL)
            {
                ble_oto_evt_t evt;
                
                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_OTO_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_OTO_EVT_NOTIFICATION_DISABLED;
                }

                p_oto->evt_handler(p_oto, &evt);
            }
        }
    }
}


void ble_oto_on_ble_evt(ble_oto_t * p_oto, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_oto, p_ble_evt);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_oto, p_ble_evt);
            break;
            
        case BLE_GATTS_EVT_WRITE:
            on_write(p_oto, p_ble_evt);
            break;
            
        default:
            break;
    }
}


/*
 * Add characteristic.
 */
static uint32_t oto_char_add(ble_oto_t * p_oto, const ble_oto_init_t * p_oto_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
	  uint8_t             initial_step_count;
    
    memset(&cccd_md, 0, sizeof(cccd_md));
        
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    cccd_md.write_perm = p_oto_init->step_count_char_attr_md.cccd_write_perm;
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.read   = 1;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;
    
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_STEP_COUNT_CHAR);
    
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_oto_init->step_count_char_attr_md.read_perm;
    attr_md.write_perm = p_oto_init->step_count_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    initial_step_count = p_oto_init->initial_step_count;

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint32_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint32_t);
    attr_char_value.p_value      = &initial_step_count;
    
    return sd_ble_gatts_characteristic_add(p_oto->service_handle, 
                                           &char_md,
                                           &attr_char_value,
                                           &p_oto->step_count_handles);
}


uint32_t ble_oto_init(ble_oto_t * p_oto, const ble_oto_init_t * p_oto_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_oto->evt_handler               = p_oto_init->evt_handler;
    p_oto->conn_handle               = BLE_CONN_HANDLE_INVALID;
    
    // Add service
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_OTOLITH_SERVICE);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_oto->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Add characteristic
    return oto_char_add(p_oto, p_oto_init);
}


uint32_t ble_oto_send_step_count(ble_oto_t * p_oto, uint32_t step_count)
{
    uint32_t err_code = NRF_SUCCESS;
    
    // Send value if connected and notifying
    if (p_oto->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        ble_gatts_hvx_params_t hvx_params;
        uint16_t hvx_len;
        hvx_len = sizeof(uint32_t);
			  uint8_t buf[hvx_len];
			
			  uint32_encode(step_count, buf);
        
        memset(&hvx_params, 0, sizeof(hvx_params));
        
        hvx_params.handle   = p_oto->step_count_handles.value_handle;
        hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset   = 0;
        hvx_params.p_len    = &hvx_len;
        hvx_params.p_data   = buf;
        
        err_code = sd_ble_gatts_hvx(p_oto->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}
