

#include "ble_as.h"
#include <string.h>
#include "ble_srv_common.h"
#include "util.h"


/**@brief Write event handler.
 */
static void on_write(ble_as_t * p_as, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
//  mlog_str("on_write\r\n");

    uint16_t alarm_time = (p_evt_write->data[1] << 8) | p_evt_write->data[0];
    
    // Call application event handler, passing alarm_time
    p_as->evt_handler(alarm_time);
}


void ble_as_on_ble_evt(ble_as_t * p_as, ble_evt_t * p_ble_evt)
{
//	mlog_str("ble_as_on_ble_evt\r\n");
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTS_EVT_WRITE:
            on_write(p_as, p_ble_evt);
            break;
            
        default:
            break;
    }
}


/**@brief Add Alarm Time characteristic.
 */
static uint32_t alarm_time_char_add(ble_as_t * p_as)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint8_t             initial_alarm_time;
    
//	mlog_str("alarm_time_char_add\r\n");
    
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;
    
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_ALARM_TIME_CHAR);
    
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    initial_alarm_time = 0; // May want to change this
    
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint16_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint16_t);
    attr_char_value.p_value      = &initial_alarm_time;
    
    return sd_ble_gatts_characteristic_add(p_as->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_as->alarm_time_handles);
}


uint32_t ble_as_init(ble_as_t * p_as, const ble_as_init_t * p_as_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    if (p_as_init->evt_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    p_as->evt_handler = p_as_init->evt_handler;
    
    // Add service
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_ALARM_SERVICE);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_as->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return alarm_time_char_add(p_as);
}

uint32_t ble_as_alarm_time_get(ble_as_t * p_as, uint16_t * p_alarm_time)
{
    uint32_t err_code;
    uint16_t len = sizeof(uint16_t);
    uint8_t buf[len];
    
    err_code = sd_ble_gatts_value_get(p_as->alarm_time_handles.value_handle, 0, &len, buf);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // set alarm_time (two byte value)
    *p_alarm_time = (buf[1] << 8) | buf[0];
    
    return err_code;
}
