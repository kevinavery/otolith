

#include <stdint.h>
#include "ble_srv_common.h"

typedef struct
{
    ble_srv_utf8_str_t             manufact_name_str;           /**< Manufacturer Name String. */
	  ble_srv_security_mode_t        acc_attr_md;                 /**< Initial Security Setting for Device Information Characteristics. */
} ble_acc_init_t;


uint32_t ble_acc_init(const ble_acc_init_t * p_acc_init);
