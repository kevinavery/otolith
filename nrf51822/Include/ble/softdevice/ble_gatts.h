/* Copyright (c) 2011 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */
/**
  @addtogroup BLE_GATTS Generic Attribute Profile (GATT) Server.
  @{
  @brief  Definitions and prototypes for the GATTS interface.
 */

#ifndef BLE_GATTS_H__
#define BLE_GATTS_H__

#include "ble_types.h"
#include "ble_ranges.h"
#include "ble_l2cap.h"
#include "ble_gap.h"
#include "ble_gatt.h"
#include "nrf_svc.h"

/** @brief Only the default MTU size of 23 is currently supported. */
#define GATT_RX_MTU 23

/**
 * @brief GATTS API SVC numbers.
 */
enum
{
  SD_BLE_GATTS_SERVICE_ADD = BLE_GATTS_SVC_BASE, /**< Add a service. */
  SD_BLE_GATTS_INCLUDE_ADD,                      /**< Add an included service. */
  SD_BLE_GATTS_CHARACTERISTIC_ADD,               /**< Add a characteristic. */
  SD_BLE_GATTS_DESCRIPTOR_ADD,                   /**< Add a generic attribute. */
  SD_BLE_GATTS_VALUE_SET,                        /**< Set an attribute value. */
  SD_BLE_GATTS_VALUE_GET,                        /**< Get an attribute value. */
  SD_BLE_GATTS_HVX,                              /**< Handle Value Notification or Indication. */
  SD_BLE_GATTS_SERVICE_CHANGED,                  /**< Perform a Service Changed Indication to one or more peers. */
  SD_BLE_GATTS_RW_AUTHORIZE_REPLY,               /**< Reply to an authorization request for a read or write operation on one or more attributes. */ 
  SD_BLE_GATTS_SYS_ATTR_SET,                     /**< Set the persistent system attributes for a connection */  
  SD_BLE_GATTS_SYS_ATTR_GET,                     /**< Get updated persistent system attributes after terminating a connection */
};


/** @defgroup BLE_ERRORS_GATTS SVC return values specific to GATTS.
 * @{ */
#define BLE_ERROR_GATTS_INVALID_ATTR_TYPE   (NRF_GATTS_ERR_BASE + 0x000)
#define BLE_ERROR_GATTS_SYS_ATTR_MISSING    (NRF_GATTS_ERR_BASE + 0x001)
/** @} */


/** @defgroup BLE_GATTS_SRVC_TYPES GATT Server Service Types.
 * @{ */
#define BLE_GATTS_SRVC_TYPE_INVALID          0x00  /**< Invalid Service Type. */
#define BLE_GATTS_SRVC_TYPE_PRIMARY          0x01  /**< Primary Service. */
#define BLE_GATTS_SRVC_TYPE_SECONDARY        0x02  /**< Secondary Type. */
/** @} */


/** @defgroup BLE_GATTS_ATTR_TYPES GATT Server Attribute Types.
 * @{ */
#define BLE_GATTS_ATTR_TYPE_INVALID         0x00  /**< Invalid Attribute Type. */
#define BLE_GATTS_ATTR_TYPE_PRIM_SRVC_DECL  0x01  /**< Primary Service Declaration. */
#define BLE_GATTS_ATTR_TYPE_SEC_SRVC_DECL   0x02  /**< Secondary Service Declaration. */
#define BLE_GATTS_ATTR_TYPE_INC_DECL        0x03  /**< Include Declaration. */
#define BLE_GATTS_ATTR_TYPE_CHAR_DECL       0x04  /**< Characteristic Declaration. */
#define BLE_GATTS_ATTR_TYPE_CHAR_VAL        0x05  /**< Characteristic Value. */
#define BLE_GATTS_ATTR_TYPE_DESC            0x06  /**< Descriptor. */
#define BLE_GATTS_ATTR_TYPE_OTHER           0x07  /**< Other, non-GATT specific type. */
/** @} */


/** @defgroup BLE_GATTS_OPS GATT Server Operations
 * @{ */
#define BLE_GATTS_OP_INVALID                0x00  /**< Invalid Operation. */
#define BLE_GATTS_OP_WRITE_REQ              0x01  /**< Write Request. */
#define BLE_GATTS_OP_WRITE_CMD              0x02  /**< Write Command. */
#define BLE_GATTS_OP_SIGN_WRITE_CMD         0x03  /**< Signed Write Command. */
/** @} */

/** @defgroup BLE_GATTS_VLOCS GATT Value Locations
 * @{ */
#define BLE_GATTS_VLOC_INVALID       0x00  /**< Invalid Location. */
#define BLE_GATTS_VLOC_STACK         0x01  /**< Value is located in stack memory. */
#define BLE_GATTS_VLOC_USER          0x02  /**< Value is located in user memory. */
/** @} */

/** @defgroup BLE_GATTS_AUTHORIZE_TYPES GATT Server Authorization Types
 * @{ */
#define BLE_GATTS_AUTHORIZE_TYPE_INVALID    0x00  /**< Invalid Type. */
#define BLE_GATTS_AUTHORIZE_TYPE_READ       0x01  /**< Authorize a Read Operation */
#define BLE_GATTS_AUTHORIZE_TYPE_WRITE      0x02  /**< Authorize a Write Request Operation */
/** @} */


/**@brief Attribute metadata. */
typedef struct
{
  ble_gap_conn_sec_mode_t read_perm;       /**< Read permissions. */
  ble_gap_conn_sec_mode_t write_perm;      /**< Write permissions. */
  uint8_t                 vlen       :1;   /**< Variable length attribute. */
  uint8_t                 vloc       :2;   /**< Value location, see @ref BLE_GATTS_VLOCS.*/
  uint8_t                 rd_auth    :1;   /**< Value will be requested from application on every read. */ 
  uint8_t                 wr_auth    :1;   /**< Value will be reported to application on every write. */
} ble_gatts_attr_md_t;


/**@brief GATT Attribute. */
typedef struct
{
  ble_uuid_t*          p_uuid;          /**< Pointer to the attribute UUID. */
  ble_gatts_attr_md_t* p_attr_md;       /**< Pointer to the attribute metadata structure. */
  uint16_t             init_len;        /**< Initial attribute value length in bytes. */
  uint16_t             init_offs;       /**< Initial attribute value offset in bytes. If different from zero, the first init_offs bytes of the attribute value will be left uninitialized. */
  uint16_t             max_len;         /**< Maximum attribute value length in bytes. */
  uint8_t*             p_value;         /**< Pointer to the attribute data. */
} ble_gatts_attr_t;


/**@brief GATT Attribute Context. */
typedef struct
{
  ble_uuid_t           srvc_uuid;       /**< Service UUID. */
  ble_uuid_t           char_uuid;       /**< Characteristic UUID if applicable (BLE_UUID_TYPE_UNKNOWN if N/A). */
  ble_uuid_t           desc_uuid;       /**< Descriptor UUID if applicable (BLE_UUID_TYPE_UNKNOWN if N/A). */
  uint16_t             srvc_handle;     /**< Service Handle. */
  uint16_t             value_handle;    /**< Characteristic Handle if applicable (BLE_GATT_HANDLE_INVALID if N/A). */
  uint8_t              type;            /**< Attribute Type, see @ref BLE_GATTS_ATTR_TYPES. */
} ble_gatts_attr_context_t;


/**@brief GATT Characteristic Presentation Format. */
typedef struct
{
  uint8_t          format;      /**< Format of the value, see @ref BLE_GATT_CPF_FORMATS. */
  int8_t           exponent;    /**< Exponent for integer data types. */
  uint16_t         unit;        /**< UUID from Bluetooth Assigned Numbers. */
  uint8_t          name_space;  /**< Namespace from Bluetooth Assigned Numbers, see @ref BLE_GATT_CPF_NAMESPACES. */
  uint16_t         desc;        /**< Namespace description from Bluetooth Assigned Numbers, see @ref BLE_GATT_CPF_NAMESPACES. */
} ble_gatts_char_pf_t;


/**@brief GATT Characteristic Meta data. */
typedef struct
{
  ble_gatt_char_props_t       char_props;               /**< Characteristic Properties. */
  ble_gatt_char_ext_props_t   char_ext_props;           /**< Characteristic Extended Properties. */
  uint8_t*                    p_char_user_desc;         /**< Pointer to a UTF-8, NULL if the descriptor is not required. */
  uint16_t                    char_user_desc_max_size;  /**< The max size of the user description */
  uint16_t                    char_user_desc_size;      /**< The current size of the user description, <= char_user_desc_max_size */ 
  ble_gatts_char_pf_t*        p_char_pf;                /**< Pointer to a presentation format structure or NULL if the descriptor is not required. */
  ble_gatts_attr_md_t*        p_user_desc_md;           /**< Attribute Metadata for the User Description descriptor, or NULL for default values. */
  ble_gatts_attr_md_t*        p_cccd_md;                /**< Attribute Metadata for the Client Characteristic Configuration Descriptor, or NULL for default values. */
  ble_gatts_attr_md_t*        p_sccd_md;                /**< Attribute Metadata for the Server Characteristic Configuration Descriptor, or NULL for default values. */
} ble_gatts_char_md_t;


/**@brief GATT Characteristic Definition Handles. */
typedef struct
{
  uint16_t          value_handle;       /**< Handle to the characteristic value. */
  uint16_t          user_desc_handle;   /**< Handle to the User Description descriptor, or BLE_GATT_HANDLE_INVALID if not present. */
  uint16_t          cccd_handle;        /**< Handle to the Client Characteristic Configuration Descriptor, or BLE_GATT_HANDLE_INVALID if not present. */
  uint16_t          sccd_handle;        /**< Handle to the Server Characteristic Configuration Descriptor, or BLE_GATT_HANDLE_INVALID if not present. */
} ble_gatts_char_handles_t;


/**@brief GATT HVx parameters. */
typedef struct
{
  uint16_t          handle;             /**< Characteristic Value Handle. */
  uint8_t           type;               /**< Indication or Notification, see @ref BLE_GATT_HVX_TYPES. */
  uint16_t          offset;             /**< Offset within the attribute value. */
  uint16_t*         p_len;              /**< Length in bytes to be written, length in bytes written after successful return. */
  uint8_t*          p_data;             /**< Actual data content, use NULL to use the current attribute value. */
} ble_gatts_hvx_params_t;

/**@brief GATT Read Authorization parameters. */
typedef struct
{
  uint16_t          gatt_status;        /**< GATT status code for the operation, see @ref BLE_GATT_STATUS_CODES */
  uint8_t           update : 1;         /**< If set, data supplied in p_data will be used in the ATT response */
  uint16_t          offset;             /**< Offset of the attribute value being updated */
  uint16_t          len;                /**< Length in bytes of the value in p_data pointer */
  uint8_t*          p_data;             /**< Pointer to new value used to update the attribute value */
} ble_gatts_read_authorize_params_t;

/**@brief GATT Write Authorisation parameters. */
typedef struct
{
  uint16_t          gatt_status;        /**< GATT status code for the operation, see @ref BLE_GATT_STATUS_CODES */
} ble_gatts_write_authorize_params_t;

/**@brief GATT Read or Write Authorize Reply parameters. */
typedef struct
{
  uint8_t                               type;   /**< Type of authorize operation, see @ref BLE_GATTS_AUTHORIZE_TYPES. */
  union {
    ble_gatts_read_authorize_params_t   read;   /**< Read authorization parameters */
    ble_gatts_write_authorize_params_t  write;  /**< Write authorization parameters */
  } params;
} ble_gatts_rw_authorize_reply_params_t;

/**
 * @brief GATT Server Event IDs.
 */
enum
{
  BLE_GATTS_EVT_WRITE = BLE_GATTS_EVT_BASE,       /**< Write operation performed. */
  BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST,             /**< Read/Write Authorization request. */
  BLE_GATTS_EVT_SYS_ATTR_MISSING,                 /**< A persistent system attribute access is pending, awaiting a sd_ble_gatts_sys_attr_set(). */
  BLE_GATTS_EVT_HVC,                              /**< Handle Value Confirmation. */
  BLE_GATTS_EVT_SC_CONFIRM,                       /**< Service Changed Confirmation. */
  BLE_GATTS_EVT_TIMEOUT                           /**< Timeout. */
};


/**@brief Event structure for BLE_GATTS_EVT_WRITE. */
typedef struct
{
  uint16_t                    handle;             /**< Attribute Handle. */
  uint8_t                     op;                 /**< Type of write operation, see @ref BLE_GATTS_OPS. */
  ble_gatts_attr_context_t    context;            /**< Attribute Context. */
  uint16_t                    offset;             /**< Offset for the write operation. */
  uint16_t                    len;                /**< Length of the incoming data. */
  uint8_t                     data[1];            /**< Incoming data, variable length */
} ble_gatts_evt_write_t;

/**@brief Event structure for authorize read request. */
typedef struct
{
  uint16_t                    handle;             /**< Attribute Handle. */
  ble_gatts_attr_context_t    context;            /**< Attribute Context. */
  uint16_t                    offset;             /**< Offset for the read operation. */
} ble_gatts_evt_read_t;

/**@brief Event structure for BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST. */
typedef struct
{
  uint8_t                     type;             /**< Type of authorize operation, see @ref BLE_GATTS_AUTHORIZE_TYPES. */
  union {
    ble_gatts_evt_read_t      read;             /**< Attribute Read Parameters */
    ble_gatts_evt_write_t     write;            /**< Attribute Write Parameters */
  } request;
} ble_gatts_evt_rw_authorize_request_t;

/**@brief Event structure for BLE_GATTS_EVT_SYS_ATTR_MISSING. */
typedef struct
{
  uint8_t hint;
} ble_gatts_evt_sys_attr_missing_t;


/**@brief Event structure for BLE_GATTS_EVT_HVC. */
typedef struct
{
  uint16_t          handle;                       /**< Attribute Handle. */
} ble_gatts_evt_hvc_t;

/**@brief Event structure for BLE_GATTS_EVT_TIMEOUT. */
typedef struct
{
  uint8_t          src;                       /**< Timeout source, see @ref BLE_GATT_TIMEOUT_SOURCES. */
} ble_gatts_evt_timeout_t;


/**@brief GATT Server event callback event structure. */
typedef struct
{
  uint16_t conn_handle;                                       /**< Connection Handle on which event occurred. */
  union
  {
    ble_gatts_evt_write_t                 write;              /**< Write Event Parameters. */
    ble_gatts_evt_rw_authorize_request_t  authorize_request;  /**< Read or Write Authorize Request Parameters. */
    ble_gatts_evt_sys_attr_missing_t      sys_attr_missing;   /**< System attributes missing. */
    ble_gatts_evt_hvc_t                   hvc;                /**< Handle Value Confirmation Event Parameters. */
    ble_gatts_evt_timeout_t               timeout;            /**< Timeout Event. */
  } params;
} ble_gatts_evt_t;


/**@brief Add a service declaration to the local server database.
 *
 * @param[in] type      Toggles between primary and secondary services, see @ref BLE_GATTS_SRVC_TYPES.
 * @param[in] p_uuid    Pointer to service UUID.
 * @param[out] p_handle Pointer to a 16-bit word where the assigned handle will be stored.
 *
 * @return @ref NRF_SUCCESS Success.
 * @return @ref NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @return @ref NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 * @return @ref NRF_ERROR_FORBIDDEN Forbidden value supplied.
 * @return @ref NRF_ERROR_NO_MEM Not enough memory to complete operation.
 */
SVCALL(SD_BLE_GATTS_SERVICE_ADD, uint32_t, sd_ble_gatts_service_add(uint8_t type, ble_uuid_t const*const p_uuid, uint16_t *const p_handle));


/**@brief Add an include declaration to the local server database.
 *
 * @note It is currently only possible to add an include declaration to the last added service (i.e. only sequential addition is supported at this time). 
 *
 * @param[in] service_handle    Handle of the service where the included service is to be placed, if BLE_GATT_HANDLE_INVALID is used, it will be placed sequentially.
 * @param[in] inc_srvc_handle   Handle of the included service.
 * @param[out] p_include_handle Pointer to a 16-bit word where the assigned handle will be stored.
 *
 * @return @ref NRF_SUCCESS Success.
 * @return @ref NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @return @ref NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 * @return @ref NRF_ERROR_INVALID_STATE Invalid state to perform operation.
 * @return @ref NRF_ERROR_FORBIDDEN Forbidden value supplied.
 * @return @ref NRF_ERROR_NO_MEM Not enough memory to complete operation.
 * @return @ref NRF_ERROR_NOT_FOUND Attribute not found.
 */
SVCALL(SD_BLE_GATTS_INCLUDE_ADD, uint32_t, sd_ble_gatts_include_add(uint16_t service_handle, uint16_t inc_srvc_handle, uint16_t *const p_include_handle));


/**@brief Add a characteristic declaration, a characteristic value declaration and optional characteristic descriptor declarations to the local server database.
 *
 * @note It is currently only possible to add a characteristic to the last added service (i.e. only sequential addition is supported at this time). 
 *
 * @param[in] service_handle    Handle of the service where the characteristic is to be placed, if BLE_GATT_HANDLE_INVALID is used, it will be placed sequentially.
 * @param[in] p_char_md         Characteristic metadata.
 * @param[in] p_attr_char_value Pointer to the attribute structure corresponding to the characteristic value.
 * @param[out] p_handles        Pointer to the structure where the assigned handles will be stored.
 *
 * @return @ref NRF_SUCCESS Success.
 * @return @ref NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @return @ref NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 * @return @ref NRF_ERROR_INVALID_STATE Invalid state to perform operation.
 * @return @ref NRF_ERROR_FORBIDDEN Forbidden value supplied.
 * @return @ref NRF_ERROR_NO_MEM Not enough memory to complete operation.
 * @return @ref NRF_ERROR_DATA_SIZE Invalid data size(s) supplied.
 */
SVCALL(SD_BLE_GATTS_CHARACTERISTIC_ADD, uint32_t, sd_ble_gatts_characteristic_add(uint16_t service_handle, ble_gatts_char_md_t const*const p_char_md, ble_gatts_attr_t const*const p_attr_char_value, ble_gatts_char_handles_t *const p_handles));


/**@brief Add a descriptor to the local server database.
 *
 * @note It is currently only possible to add a descriptor to the last added characteristic (i.e. only sequential addition is supported at this time). 
 *
 * @param[in] char_handle   Handle of the characteristic where the descriptor is to be placed, if BLE_GATT_HANDLE_INVALID is used, it will be placed sequentially.
 * @param[in] p_attr        Pointer to the attribute structure.
 * @param[out] p_handle     Pointer to a 16-bit word where the assigned handle will be stored.
 * @return @ref NRF_SUCCESS Success.
 * @return @ref NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @return @ref NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 * @return @ref NRF_ERROR_INVALID_STATE Invalid state to perform operation.
 * @return @ref NRF_ERROR_FORBIDDEN Forbidden value supplied.
 * @return @ref NRF_ERROR_NO_MEM Not enough memory to complete operation.
 * @return @ref NRF_ERROR_DATA_SIZE Invalid data size(s) supplied.
 */
SVCALL(SD_BLE_GATTS_DESCRIPTOR_ADD, uint32_t, sd_ble_gatts_descriptor_add(uint16_t char_handle, ble_gatts_attr_t const * const p_attr, uint16_t* const p_handle));

/**@brief Set the value of a given attribute.
 *
 * @param[in] handle    Attribute handle.
 * @param[in] offset    Offset in bytes to write from.
 * @param[in,out] p_len Length in bytes to be written, length in bytes written after successful return.
 * @param[in] p_value   Pointer to a buffer (at least len bytes long) containing the desired attribute value.
 *
 * @return @ref NRF_SUCCESS Success.
 * @return @ref NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @return @ref NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 * @return @ref NRF_ERROR_NOT_FOUND Attribute not found.
 * @return @ref NRF_ERROR_DATA_SIZE Invalid data size(s) supplied.
 */
SVCALL(SD_BLE_GATTS_VALUE_SET, uint32_t, sd_ble_gatts_value_set(uint16_t handle, uint16_t offset, uint16_t* const p_len, uint8_t const * const p_value));

/**@brief Get the value of a given attribute.
 *
 * @param[in] handle     Attribute handle.
 * @param[in] offset     Offset in bytes to read from.
 * @param[in,out] p_len  Length in bytes to be read, length in bytes read after successful return.
 * @param[in,out] p_data Pointer to a buffer (at least len bytes long) where to store the attribute value.
 * 
 * @return @ref NRF_SUCCESS Success.
 * @return @ref NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @return @ref NRF_ERROR_NOT_FOUND Attribute not found.
 */
SVCALL(SD_BLE_GATTS_VALUE_GET, uint32_t, sd_ble_gatts_value_get(uint16_t handle, uint16_t offset, uint16_t *const p_len, uint8_t* const p_data));

/**@brief Notify or Indicate an attribute value.
 *
 * @details This function checks for the relevant Client Characteristic Configuration descriptor value to verify that the relevant operation
 *          (notification or indication) has been enabled by the client. It is also able to update the attribute value before issuing the PDU, so that
 *          the application can atomically perform a value update and a server initiated transaction with a single API call.
 *          If the application chooses to indicate an attribute value, a @ref BLE_GATTS_EVT_HVC will be sent up as soon as the confirmation arrives from
 *          the peer.
 *
 * @note    The local attribute value may be updated even if an outgoing packet is not sent to the peer due to an error during execution. 
 *          When receiveing the error codes @ref NRF_ERROR_INVALID_STATE, @ref NRF_ERROR_BUSY, @ref BLE_ERROR_GATTS_SYS_ATTR_MISSING and 
 *          @ref BLE_ERROR_NO_TX_BUFFERS the database has been updated.
 *          The caller can check whether the value has been updated by looking at the contents of *(p_hvx_params->p_len).
 *
 * @note    It is important to note that a notification will <b>consume an application buffer</b>, and will therefore 
 *          generate a @ref BLE_EVT_TX_COMPLETE event when the packet has been transmitted. An indication on the other hand will use the 
 *          standard server internal buffer and thus will only generate a @ref BLE_GATTS_EVT_HVC event as soon as the confirmation 
 *          has been received from the peer. Please see the documentation of @ref sd_ble_tx_buffer_count_get for more details.
 *
 * @param[in] conn_handle  Connection handle.
 * @param[in] p_hvx_params Pointer to an HVx parameters structure. If the p_data member contains a non-NULL pointer the attribute value will be updated with
 *                         the contents pointed by it before sending the notification or indication.
 *
 * @return @ref NRF_SUCCESS Success.
 * @return @ref BLE_ERROR_INVALID_CONN_HANDLE Invalid Connection Handle
 * @return @ref NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @return @ref NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 * @return @ref BLE_ERROR_INVALID_ATTR_HANDLE Invalid attribute handle(s) supplied.
 * @return @ref BLE_ERROR_GATTS_INVALID_ATTR_TYPE Invalid attribute type(s) supplied.
 * @return @ref NRF_ERROR_NOT_FOUND Attribute not found.
 * @return @ref NRF_ERROR_DATA_SIZE Invalid data size(s) supplied.
 * @return @ref NRF_ERROR_INVALID_STATE Invalid state to perform operation.
 * @return @ref NRF_ERROR_BUSY Procedure already in progress.
 * @return @ref BLE_ERROR_GATTS_SYS_ATTR_MISSING System attributes missing.
 * @return @ref BLE_ERROR_NO_TX_BUFFERS There are no available buffers to send the data.
 */
SVCALL(SD_BLE_GATTS_HVX, uint32_t, sd_ble_gatts_hvx(uint16_t conn_handle, ble_gatts_hvx_params_t const*const p_hvx_params));

/**@brief Indicate the Service Changed attribute value.
 *
 * @details This call will send a Handle Value Indication to one or more peers connected to inform them that the attribute
 *          table layout has changed. As soon as the peer has confirmed the indication, a @ref BLE_GATTS_EVT_SC_CONFIRM event will
 *          be issued.
 *
 * @note    Some of the restrictions and limitations that apply to @ref sd_ble_gatts_hvx also apply here.
 *
 * @param[in] conn_handle  Connection handle.
 * @param[in] start_handle Start of affected attribute handle range.
 * @param[in] end_handle   End of affected attribute handle range.
 *
 * @return @ref NRF_SUCCESS Success.
 * @return @ref BLE_ERROR_INVALID_CONN_HANDLE Invalid Connection Handle
 * @return @ref NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @return @ref NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 * @return @ref BLE_ERROR_INVALID_ATTR_HANDLE Invalid attribute handle(s) supplied.
 * @return @ref NRF_ERROR_INVALID_STATE Invalid state to perform operation.
 * @return @ref NRF_ERROR_BUSY Procedure already in progress.
 * @return @ref BLE_ERROR_GATTS_SYS_ATTR_MISSING System attributes missing.
 */
SVCALL(SD_BLE_GATTS_SERVICE_CHANGED, uint32_t, sd_ble_gatts_service_changed(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle));

/**@brief Respond to a Read/Write authorization request.
 *
 * @param[in] conn_handle                 Connection handle.
 * @param[in] p_rw_authorize_reply_params Pointer to a structure with the attribute provided by the application.
 *
 * @return @ref NRF_SUCCESS               Success.
 * @return @ref NRF_ERROR_INVALID_STATE   No authorization request pending.
 * @return @ref NRF_ERROR_INVALID_PARAM   Authorization op invalid,
 *                                         or for Read Authorization reply: requested handles not replied with,
 *                                         or for Write Authorization reply: handle supplied does not match requested handle.
 */
SVCALL(SD_BLE_GATTS_RW_AUTHORIZE_REPLY, uint32_t, sd_ble_gatts_rw_authorize_reply(uint16_t conn_handle, ble_gatts_rw_authorize_reply_params_t const*const p_rw_authorize_reply_params));


/**@brief Update persistent system attribute info
 *
 * @details Supply to the stack information about persistent system attributes.
 *          This call is legal in the connected state only, and is usually 
 *          made immediately after a connection is established and the bond identified.
 *          usually as a response to a BLE_GATTS_EVT_SYS_ATTR_MISSING.
 *
 *          p_sysattrs may point directly to the application's flashed copy of the struct.
 *          If the pointer is NULL, the system attribute info is initialized, assuming that
 *          the application does not have any previously saved data for this bond.
 *
 * @note The state of persistent attributes is reset upon connection and then remembered for its duration. 
 *       This means that invoking this call for a second time during a connection will have no effect upon the values of the attributes.
 *
 * @param[in]  conn_handle     Connection handle.
 * @param[in]  p_sys_attr_data     Pointer to a saved copy of system attributes supplied to the stack, or NULL.
 * @param[in]  len Size of data pointed by p_sys_attr_data, in octets. 
 *
 * @return @ref NRF_SUCCESS Success.
 * @return @ref NRF_ERROR_INVALID_DATA Invalid data type(s) supplied.
 * @return @ref NRF_ERROR_NO_MEM Not enough memory to complete operation.
 */ 
SVCALL(SD_BLE_GATTS_SYS_ATTR_SET, uint32_t, sd_ble_gatts_sys_attr_set(uint16_t conn_handle, uint8_t const*const p_sys_attr_data, uint16_t len)); 

 
 /**@brief Retrieve persistent system attribute info from softdevice
 *
 * @details This call is used to retrieve information about values to be stored perisistently by the application
 *          after a connection has been terminated. When a new connection is made to the same bond, the values
 *          should be restored using sd_ble_gatts_sys_attr_set().
 *          The data should be read before any new advertising is started, or any new connection established. The connection handle for
 *          the previous now defunct connection will remain valid until a new one is created to allow this API call to refer to it.
 *
 * @param[in]     conn_handle      Connection handle of the recently terminated connection.
 * @param[in]     p_sys_attr_data  Pointer to a buffer where updated information about system attributes will be filled in. NULL can be provided to 
 *                                 obtain the length of the data
 * @param[in,out] p_len  Size of application buffer if p_sys_attr_data is not NULL. Unconditially updated to actual length of system attribute data.
 *
 * @return @ref NRF_SUCCESS Success.
 * @return @ref NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @return @ref NRF_ERROR_DATA_SIZE Invalid data size(s) supplied.
 */ 
SVCALL(SD_BLE_GATTS_SYS_ATTR_GET, uint32_t, sd_ble_gatts_sys_attr_get(uint16_t conn_handle, uint8_t * const p_sys_attr_data, uint16_t* const p_len)); 


#endif // BLE_GATTS_H__

/**
  @}
*/
