

#include "ble_acc.h"

#include <stdlib.h>
#include <string.h>
#include "app_error.h"
#include "ble_gatts.h"
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"


#define BLE_UUID_ACC_SERVICE                                 0x181B

static uint16_t                 service_handle;
static ble_gatts_char_handles_t manufact_name_handles;



static uint32_t char_add(uint16_t                        uuid,
                         uint8_t *                       p_char_value,
                         uint16_t                        char_len,
                         const ble_srv_security_mode_t * dis_attr_md,
                         ble_gatts_char_handles_t *      p_handles)
{
    ble_uuid_t          ble_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;

    APP_ERROR_CHECK_BOOL(p_char_value != NULL);
    APP_ERROR_CHECK_BOOL(char_len > 0);
    
    // The ble_gatts_char_md_t structure uses bit fields. So we reset the memory to zero.
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read  = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, uuid);
    
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = dis_attr_md->read_perm;
    attr_md.write_perm = dis_attr_md->write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = char_len;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = char_len;
    attr_char_value.p_value      = p_char_value;

    return sd_ble_gatts_characteristic_add(service_handle, &char_md, &attr_char_value, p_handles);
}

uint32_t ble_acc_init(const ble_acc_init_t * p_acc_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Add service
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_ACC_SERVICE);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add characteristics
    if (p_acc_init->manufact_name_str.length > 0)
    {
        err_code = char_add(BLE_UUID_MANUFACTURER_NAME_STRING_CHAR,
                            p_acc_init->manufact_name_str.p_str, 
                            p_acc_init->manufact_name_str.length,
                            &p_acc_init->acc_attr_md,
                            &manufact_name_handles);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }

    return NRF_SUCCESS;
}
