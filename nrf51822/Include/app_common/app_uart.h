/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup app_uart UART module
 * @{
 * @ingroup app_common
 *
 * @brief UART module implementation.
 *
 * @details The UART module can be used for control of the UART.
 *          It has a low power mode that will be enabled when RTS/CTS flowcontrol is enabled.
 *          The UART peripheral will be turned off whenever the CTS signal from the remote side is
 *          high, and when CTS becomes low (active) the UART peripheral will be enabled.
 *
 *          <b>Asynchron nature</b>
 *
 *          The UART module will recieve bytes from the RXD register when an EVENT_RXDRDY interrupt
 *          has occured. The byte will be put into the RX FIFO and can be fetched by the 
 *          application using @ref app_uart_get. First byte recieved and placed in the RX FIFO will 
 *          trigger an APP_UART_DATA_READY event. 
 *
 *          The @ref app_uart_put will place the provided byte in the TX FIFO.
 *          Bytes in the TX FIFO will be written to the TXD register by the app_uart module.
 *          When a byte is sucessfully transfered an EVENT_TXDRDY interrupt is triggered. The 
 *          interrupt handler in the app_uart module will fetch the next byte from the FIFO and 
 *          write it to the TXD register. 
 *          The application can call @ref app_uart_put to request transmission of bytes.
 *
 *          <b>Error handling</b>
 *
 *          An error flag is set in the UART peripheral in event of an error during data reception. 
 *          The error will be propagated to the application event handler as an 
 *          @ref app_uart_evt_t containg APP_UART_ERROR_SOURCE in the evt_type field. The 
 *          data.error_source field will contain the original error source information from the 
 *          UART peripheral register. 
 *
 *          In case the RX FIFO is full when data are received an @ref app_uart_evt_t containg 
 *          APP_UART_NRF_ERROR in the evt_type field will be generated and sent to the application.
 *          The event will contain the original error code from the FIFO in the data.error_code 
 *          field.
 *
 *          <b>UART with flow control</b>
 *
 *          When UART is configured to use flow control it will use low power mode when possible.
 *          The UART peripheral will be de-activated when the CTS signal from the remote side is 
 *          set to inactive state. When CTS is set active by the remote side, the UART peripheral
 *          will be activated.
 *
 *          The STARTTX register will be enabled when data are available in the TX FIFO and CTS is
 *          active.
 *
 *          The application should use @ref app_uart_get_connection_state to ensure that the nRF51
 *          chip only power off when the UART peripheral is disconnected. When the nRF51 chip is in
 *          power off mode the remote side can wake up the nRF51 chip by setting the CTS to active 
 *          state. 
 *
 * @image   html uart_state_machine_tx_with_flow_control.png "UART with flow control enabled state machine diagram."
 *
 * @note    When using RTS/CTS flowcontrol the CTS pin will be monitored by the GPIOTE. To register
 *          the UART module as a user of app_gpiote run the GPIOTE_INIT(X) macro to initialize
 *          the module with the X number of users. X will be the number of all app_gpiote users of
 *          the whole application.
 *
 * @par     UART without flow control
 *
 *          When using the app_uart module without flow control the UART will not use GPIOTE. CTS will
 *          not work as a signal for the UART to stop the RX peripheral. Thus the UART RX will
 *          always be on.
 *
 * @image   html uart_state_machine_tx_no_flow_control.png "UART without flow control state machine diagram."
 */

#ifndef APP_UART_H__
#define APP_UART_H__

#include <stdint.h>
#include <stdbool.h>
#include "app_util.h"

#define  UART_PIN_DISCONNECTED  0xFFFFFFFF  /**< Value indicating that no pin is connected to this UART register. */

/**@brief UART communication structure holding configuration settings for the peripheral.
 */
typedef struct
{
    uint8_t   rx_pin_no;                    /**< RX pin number. */
    uint8_t   tx_pin_no;                    /**< TX pin number. */
    uint8_t   rts_pin_no;                   /**< RTS pin number, only used if flow control is enabled. */
    uint8_t   cts_pin_no;                   /**< CTS pin number, only used if flow control is enabled. */
    bool      use_hardware_flow_control;    /**< Flow control setting, if flow control is used, the system will use low power UART mode, based on CTS signal. */
    bool      use_parity;                   /**< Even parity if TRUE, no parity if FALSE. */
    uint32_t  baud_rate;                    /**< Baud rate configuration. */
} app_uart_comm_params_t;

/**@brief UART buffer for transmitting/receiving data.
 */
typedef struct
{
    uint8_t * rx_buf;                       /**< Pointer to the RX buffer. */
    uint32_t  rx_buf_size;                  /**< Size of the RX buffer. */
    uint8_t * tx_buf;                       /**< Pointer to the TX buffer. */
    uint32_t  tx_buf_size;                  /**< Size of the TX buffer. */
} app_uart_buffers_t;

/**@brief Enumeration describing current state of the UART.
 */
typedef enum
{
    APP_UART_DISCONNECTED,                  /**< State indicating that the UART is disconnected and cannot receive or transmit bytes. */
    APP_UART_CONNECTED                      /**< State indicating that the UART is connected and ready to receive or transmit bytes. If flow control is disabled, the state will always be connected. */
} app_uart_connection_state_t;

/**@brief Enumeration which defines events used by the UART module upon data reception or error.
 *
 * @details The event type is used to indicate the type of additional information in the event
 * @ref app_uart_evt_t.
 */
typedef enum
{
    APP_UART_DATA_READY,                    /**< An event indicating that UART data has been received. */
    APP_UART_NRF_ERROR,                     /**< An error in the app_uart module has occured. */
    APP_UART_ERROR_SOURCE                   /**< An error in the peripheral has occured during reception. */
} app_uart_evt_type_t;

/**@brief Struct containing events from the UART module.
 *
 * @details The app_uart_evt_t is used to notify the application of asynchronous events when data
 * are received on the UART peripheral or in case an error occured during data reception.
 */
typedef struct
{
    app_uart_evt_type_t evt_type;           /**< Type of event. */
    union
    {
        uint32_t        error_source;       /**< Field used if evt_type is: APP_UART_ERROR_SOURCE. Additional error source if byte recieved was causing an error. This field contains the value in the ERRORSRC register for the UART peripheral. */
        uint32_t        error_code;         /**< Field used if evt_type is: APP_UART_NRF_ERROR. Additional status/error code if the event was caused by app_uart module. This error code refer to errors defined in nrf_error.h. */
    } data;
} app_uart_evt_t;

/**@brief app_uart event handler callback function.
 *
 * @details Upon an event in the app_uart module this callback function will be called to notify
 *          the applicatioon about the event.
 *
 * @param[in]   p_app_uart_event Pointer to UART event.
 */
typedef void (*app_uart_event_handler_t) (app_uart_evt_t * p_app_uart_event);

/**@brief Macro for safe initialization of the UART module.
 *
 * @param[in]   P_COMM_PARAMS   Pointer to a UART communication structure: app_uart_comm_params_t
 * @param[in]   RX_BUF_SIZE     Size of desired RX buffer, must be a power of 2.
 * @param[in]   TX_BUF_SIZE     Size of desired TX buffer, must be a power of 2.
 * @param[in]   EVENT_HANDLER   Event handler function to be called when an event occurs in the
 *                              UART module.
 * @param[in]   IRQ_PRIO        IRQ priority, app_irq_priority_t, for the UART module irq handler.
 * @param[out]  ERR_CODE        The return value of the UART initialization function will be
 *                              written to this parameter.
 *
 * @note Since this macro allocates a buffer and registers the module as a GPIOTE user when flow
 *       control is enabled, it must only be called once.
 */
#define APP_UART_INIT(P_COMM_PARAMS, RX_BUF_SIZE, TX_BUF_SIZE, EVENT_HANDLER, IRQ_PRIO, ERR_CODE)  \
    do                                                                                             \
    {                                                                                              \
        static uint8_t     rx_buf[RX_BUF_SIZE];                                                    \
        static uint8_t     tx_buf[TX_BUF_SIZE];                                                    \
        app_uart_buffers_t buffers;                                                                \
                                                                                                   \
        buffers.rx_buf      = rx_buf;                                                              \
        buffers.rx_buf_size = sizeof(rx_buf);                                                      \
        buffers.tx_buf      = tx_buf;                                                              \
        buffers.tx_buf_size = sizeof(tx_buf);                                                      \
                                                                                                   \
        ERR_CODE = app_uart_init(P_COMM_PARAMS, &buffers, EVENT_HANDLER, IRQ_PRIO);                \
    } while (0)

/**@brief Initialize the UART module.
 *
 * @note Normally initialization should be done using the APP_UART_INIT() macro, as that will
 *       allocate the buffers needed by the UART module (including aligning the buffer correctly).
 *
 * @param[in]   p_comm_params     Pin and communication parameters.
 * @param[in]   p_buffers         RX and TX buffers.
 * @param[in]   error_handler     Function to be called in case of an error.
 * @param[in]   app_irq_priority  Interrupt priority level.
 *
 * @retval      NRF_SUCCESS               If successful initialization.
 * @retval      NRF_ERROR_INVALID_LENGTH  If a provided buffer is not a power of two.
 * @retval      NRF_ERROR_NULL            If one of the provided buffers is a NULL pointer.
 * @retval      NRF_ERROR_INVALID_STATE   If flow control is enabled. The GPIOTE module is not in
 *                                        a valid state when registering the UART module as a user.
 * @retval      NRF_ERROR_INVALID_PARAM   If flow control is enabled. The UART module provides an
 *                                        invalid callback function when registering the UART module
 *                                        as a user.
 * @retval      NRF_ERROR_NO_MEM          If flow control is enabled. GPIOTE module has already the
 *                                        reach the maximum number of users.
 */
uint32_t app_uart_init(const app_uart_comm_params_t * p_comm_params,
                             app_uart_buffers_t *     p_buffers,
                             app_uart_event_handler_t error_handler,
                             app_irq_priority_t       irq_priority);

/**@brief Get a byte from the UART.
 *
 * @details This function will get the next byte from the RX buffer. If the RX buffer is empty
 *          an error code will be returned and the app_uart module will generate an event upon 
 *          reception of the first byte which is added to the RX buffer.
 *
 * @param[out] p_byte    Pointer to an address where next byte received on the UART will be copied.
 *
 * @retval NRF_SUCCESS          If a byte has been received and pushed to the pointer provided.
 * @retval NRF_ERROR_NOT_FOUND  If no byte is available in the RX buffer of the app_uart module.
 */
uint32_t app_uart_get(uint8_t * p_byte);

/**@brief Put a byte on the UART.
 *
 * @details This call is non-blocking.
 *
 * @param[in] byte   Byte to be transmitted on the UART.
 *
 * @retval NRF_SUCCESS        If the byte was succesfully put on the TX buffer for transmission.
 * @retval NRF_ERROR_NO_MEM   If no more space is available in the TX buffer.
 *                            NRF_ERROR_NO_MEM may occur if flow control is enabled and CTS signal
 *                            is high for a long period and the buffer fills up.
 */
uint32_t app_uart_put(uint8_t byte);

/**@brief Get the current state of the UART.
 *
 * @details If flow control is disabled, the state is assumed to always be APP_UART_CONNECTED.
 *
 *          When using flow control the state will be controlled by the CTS. If CTS is set active 
 *          by the remote side, or the app_uart module is in the process of transmitting a byte,
 *          app_uart is in APP_UART_CONNECTED state. If CTS is set inactive by remote side app_uart
 *          will not get into APP_UART_DISCONNECTED state until the last byte in the TXD register 
 *          is fully transmitted.
 *
 *          Internal states in the state machine are mapped to the general connected/disconnected
 *          states in the following ways:
 *
 *          - UART_ON    = CONNECTED
 *          - UART_READY = CONNECTED
 *          - UART_WAIT  = CONNECTED
 *          - UART_OFF   = DISCONNECTED.
 *
 * @param[out] p_connection_state    Current connection state of the UART.
 *
 * @retval NRF_SUCCESS  The connection state was succesfully retrieved.
 */
uint32_t app_uart_get_connection_state(app_uart_connection_state_t * p_connection_state);

/**@brief Flush the RX and TX buffers.
 *
 * @retval      NRF_SUCCESS              Flushing completed.
 *                                       (Current implementation will always succeed)
 */
uint32_t app_uart_flush(void);


#endif // APP_UART_H__

/** @} */
