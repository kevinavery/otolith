/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_lib_bond_manager Bonds and Persistent Data Manager
 * @{
 * @ingroup ble_sdk_lib
 * @brief Module for handling multiple bonds.
 *
 * @details The Bond Manager interacts with the BLE stack by reacting to a set of BLE stack events,
 *          thus helping the application to bond with multiple masters.
 *
 *          Applications using the Bond Manager must have a configuration file named
 *          ble_bondmngr_cfg.h (see below for details).
 *
 *          Sequence diagrams illustrating the flow of events when connecting to a master using the
 *          Bond Manager:
 *
 *          @image html bond_manager_unbonded_master.jpg Figure 1: Connecting to an unbonded master
 *          @n
 *          @image html bond_manager_bonded_master.jpg Figure 2: Connecting to a bonded master
 *
 * @section ble_sdk_lib_bond_manager_cfg Configuration File
 * Applications using the Bond Manager must have a configuration file named ble_bondmngr_cfg.h.
 * Here is an example of this file:
 * 
 * @code
 * #ifndef BLE_BONDMNGR_CFG_H__
 * #define BLE_BONDMNGR_CFG_H__
 * 
 * #define BLE_BONDMNGR_CCCD_COUNT          1
 * #define BLE_BONDMNGR_MAX_BONDED_MASTERS  4
 * 
 * #endif // BLE_BONDMNGR_CFG_H__
 * @endcode
 * 
 * BLE_BONDMNGR_CCCD_COUNT is the number of CCCDs used in the application, and
 * BLE_BONDMNGR_MAX_BONDED_MASTERS is the maximum number of bonded masters to be supported by the
 * application.
 */

#ifndef BLE_BONDMNGR_H__
#define BLE_BONDMNGR_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#define INVALID_MASTER_HANDLE       (-1)                        /**< Invalid handle, used to indicate that the master is not a known bonded master. */

/**@brief Bond Manager Module event type. */
typedef enum
{
    BLE_BONDMNGR_EVT_NEW_BOND,                                  /**< New bond has been created. */
    BLE_BONDMNGR_EVT_CONN_TO_BONDED_MASTER,                     /**< Connected to a previously bonded master. */
    BLE_BONDMNGR_EVT_ENCRYPTED,                                 /**< Current link is encrypted. */
    BLE_BONDMNGR_EVT_AUTH_STATUS_UPDATED,                       /**< Authentication status updated for current master. */
    BLE_BONDMNGR_EVT_BOND_FLASH_FULL                            /**< Flash block for storing bonding information is full. */
} ble_bondmngr_evt_type_t;

/**@brief Bond Manager Module event. */
typedef struct
{
    ble_bondmngr_evt_type_t evt_type;                           /**< Type of event. */
    int8_t                  master_handle;                      /**< Handle to the current master. */
} ble_bondmngr_evt_t;

/**@brief Bond Manager Module event handler type. */
typedef void (*ble_bondmngr_evt_handler_t) (ble_bondmngr_evt_t * p_evt);

/**@brief Bond Manager Module init structure. This contains all options and data needed for
 *        initialization of the Bond Manager module. */
typedef struct
{
    uint8_t                     flash_page_num_bond;            /**< Flash page number to use for storing bonding information. */
    uint8_t                     flash_page_num_sys_attr;        /**< Flash page number to use for storing system attribute information. */
    bool                        bonds_delete;                   /**< TRUE if bonding and system attribute information for all masters is to be deleted from flash during initialization, FALSE otherwise. */
    ble_bondmngr_evt_handler_t  evt_handler;                    /**< Event handler to be called for handling events in bond manager. */
    ble_srv_error_handler_t     error_handler;                  /**< Function to be called in case of an error. */
} ble_bondmngr_init_t;

/**@brief Initializes the Bond Manager.
 *
 * @param[in]  p_init  This contains information needed to initialize this module.
 *
 * @return     NRF_SUCCESS on successful initialization, otherwise an error code.
 */
uint32_t ble_bondmngr_init(ble_bondmngr_init_t * p_init);

/**@brief Handles all events from the BLE stack that are of interest to this module.
 *
 * @param[in]  p_ble_evt  The event received from the BLE stack.
 *
 * @return     NRF_SUCCESS if all operations went successfully,
 *             NRF_ERROR_NO_MEM if the maximum number of bonded master has been reached.
 *             Other error codes in other situations.
 */
void ble_bondmngr_on_ble_evt(ble_evt_t * p_ble_evt);

/**@brief Stores the bonded masters data including bonding info and system attributes into
 *        flash memory.
 *
 * @details This function will erase the flash pages (if the data to store is different from the one
 *          already stored) and then write into flash. This operation could prevent the radio to
 *          run.
 *
 * @note This function MUST be called ONLY on the Disconnected event, before advertising is started.
 *       If it is called from any other place, or if it is not called at all, the behavior is
 *       undefined.
 *
 * @return     NRF_SUCCESS on success, an error_code otherwise.
 */
uint32_t ble_bondmngr_bonded_masters_store(void);

/**@brief Deletes the bonded master database from flash.
 *
 * @details After calling this function you should call ble_bondmngr_init() to re-initialize 
 *          the RAM database.
 *
 * @return     NRF_SUCCESS on success, an error_code otherwise.
 */
uint32_t ble_bondmngr_bonded_masters_delete(void);

/**@brief Get the whitelist containing all currently bonded masters.
 *
 * @details This function populates the whitelist with either the irks or the public adresses
 *          of all bonded masters.
 *
 * @param[out]  p_whitelist  Whitelist structure with all bonded masters.
 *
 * @return     NRF_SUCCESS on success, an error_code otherwise.
 */
uint32_t ble_bondmngr_whitelist_get(ble_gap_whitelist_t * p_whitelist);

/**@brief Get the master's address corresponding to a given master_handle.
 *
 * @note This function returns NRF_ERROR_INVALID_PARAM if the given master has a private address.
 *
 * @param[in]   master_handle  Master's handle.
 * @param[out]  p_master_addr  Pointer to the master's address which can be used for 
 *                             directed advertising.
 */
uint32_t ble_bondmngr_master_addr_get(int8_t master_handle, ble_gap_addr_t * p_master_addr);

/**@brief Function to store system attribute of a newly connected master.
 *
 * @details This function fetches the system attributes of the current master from the stack, adds it 
 *          to the database in memory and also stores it in the flash (without erasing any flash 
 *          page). 
 *          This function is intended to facilitate the storage of system attributes when connected 
 *          to new master (whose system attributes are NOT yet stored in flash) even in connected 
 *          state without affecting radio link. This function can, for example, be called after the
 *          CCCD is written by a master. The function will succeed if the master is a new master.
 *          See @ref ble_sdk_app_hids_keyboard or @ref ble_sdk_app_hids_mouse for sample usage.
 *
 * @return     NRF_SUCCESS on success, otherwise an error code.  
 *             NRF_ERROR_INVALID_STATE is returned if the system attributes of the current master is
 *             already present in the flash because it is a previously known master.
 */
uint32_t ble_bondmngr_sys_attr_store(void);

#endif // BLE_BONDMNGR_H__

/** @} */
