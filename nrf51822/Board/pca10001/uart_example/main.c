/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
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
 * @brief Example project on UART usage to communicate with PC.
 * @defgroup uart_example_pca10001 UART example
 * @{
 * @ingroup nrf_examples_pca10001
 *
 * @brief Example of basic UART usage.
 *
 * Simple UART example that transmits and receives through the configured pins as serial device.
 * The configure pins needs to be redirected to a COM port (for some terminal program like putty which
 * can listen to this COM port through a terminal session)
 * When the program start it will transmit "START: " through this serial device using @ref simple_uart_putstring
 * and this should be visible on the terminal.
 * All typed characters on this terminal will be transmitted to this program through @ref simple_uart_get and
 * when an exit character 'q' or 'Q' is typed this program will end into an infinite loop after transmitting 
 * "EXIT!" on the new line of the terminal.
 * @note This example is not just for COM ports but can be used for any UART connection, COM port redirection
 *       is for visual verification.
 * @note Setting the define ENABLE_LOOPBACK_TEST will assume that the TX_PIN_NUMBER is connected to RX_PIN_NUMBER
 *       and this example is used to test the loopback. In this case no com port can be used as the data flows
 *       from TX to RX and ERROR_PIN is set high for any loss of data.
 * @note Configure your terminal application for 38400 bauds, 8 data bits and 1 stop bit.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "simple_uart.h"
#include "../acc/acc_driver.h"
#include "boards.h"
#include "../pedometer/step_counter.h"

#define FIFO_INTERRUPT_PIN_NUMBER (7) 

/**
 * Global data for counting steps  
 **/
measurements data;
acc_data_t acc_arr[100];
int collected_data;

void gpiote_init(void) {

    // Configure fifo interrupt pin
    nrf_gpio_cfg_input(FIFO_INTERRUPT_PIN_NUMBER, NRF_GPIO_PIN_NOPULL);
    
    // Configure GPIOTE channel 0 to generate event when 
    // MOTION_INTERRUPT_PIN_NUMBER goes from Low to High
    nrf_gpiote_event_config(0, FIFO_INTERRUPT_PIN_NUMBER, NRF_GPIOTE_POLARITY_HITOLO);
    
    // Enable interrupt for NRF_GPIOTE->EVENTS_IN[0] event
    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Msk;
}


void itoa(int num, uint8_t *buf, int buf_len)
{
    int i = 0;
    int mask = 1000000000;

    if (num == 0) {
        *buf = '0';
        *(buf+1) = 0;
        return;
    }

    if (num < 0) {
        num = -num;
        *buf = '-';
        i++; 
    }

    while (mask > num)
    {
        mask = mask / 10;
    }

    while (mask > 0 && i < buf_len-1) {
        *(buf + i) = num / mask + '0';
        num = num % mask;
        mask = mask / 10;
        i++;
    }

    *(buf+i) = 0;
}

void printData(uint8_t *label, int32_t data)
{
	uint8_t str_buf[33];
	itoa(data, str_buf, 33);
	simple_uart_putstring(label);
	simple_uart_putstring(str_buf);
	simple_uart_putstring("\r\n");
}

void fill_data(acc_data_t* acc_array) {
    simple_uart_putstring("Filling data...");
    int i;
    if(collected_data >= SAMPLE_SIZE) {
        collected_data = 0;
    }

    for(i = 0; i < 25; i++) {
        update_acc_data((acc_array + (collected_data++ + i)));
    }
}

/** GPIOTE interrupt handler.
* Triggered on motion interrupt pin input low-to-high transition.
*/
void GPIOTE_IRQHandler(void)
{
    int steps;
    fill_data(acc_arr);
    if(collected_data >= SAMPLE_SIZE) {
        filter(acc_arr, SAMPLE_SIZE);
        get_max_min(&data, acc_arr, SAMPLE_SIZE);
        steps = count_steps(&data, acc_arr, SAMPLE_SIZE);
        printData("Steps: ", steps);
    }
    
    // Event causing the interrupt must be cleared
    NRF_GPIOTE->EVENTS_IN[0] = 0;
}




/**
 * main() function
 * @return 0. int return type required by ANSI/ISO standard.
 */
int main(void)
{
    simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, HWFC);

	uint8_t cr = simple_uart_get();
	acc_init();
	gpiote_init();
    NVIC_EnableIRQ(GPIOTE_IRQn);
    __enable_irq();


	cr = simple_uart_get();
	simple_uart_putstring("Starting...\r\n");
	
    while(true)
    {
    	cr = simple_uart_get();
    	//int i;
    	// acc_data_t* acc;
    	// for(i = 0; i < 50; i++){
    	// 	 acc = update_acc_data();
    	// 	 printData("X: ", acc->x);
    	// }
        if(cr == 'q' || cr == 'Q')
        {
          while(1){}
        }
    }

}

/**
 *@}
 **/
