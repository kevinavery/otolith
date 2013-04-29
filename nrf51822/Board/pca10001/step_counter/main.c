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

#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "simple_uart.h"
#include "acc_driver.h"
#include "step_counter.h"
#include "util.h"

#define FIFO_INTERRUPT_PIN_NUMBER (7) 

/**
 * Global data for counting steps  
 **/
measurements data;
const int size =  SAMPLE_SIZE;
acc_data_t acc_arr[size];
int collected_data;
int total_steps = 0;

void gpiote_init(void) {

    // Configure fifo interrupt pin
    nrf_gpio_cfg_input(FIFO_INTERRUPT_PIN_NUMBER, NRF_GPIO_PIN_NOPULL);
    
    // Configure GPIOTE channel 0 to generate event when 
    // MOTION_INTERRUPT_PIN_NUMBER goes from Low to High
    nrf_gpiote_event_config(0, FIFO_INTERRUPT_PIN_NUMBER, NRF_GPIOTE_POLARITY_LOTOHI);
    
    // Enable interrupt for NRF_GPIOTE->EVENTS_IN[0] event
    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Msk;
}


void printData(uint8_t *label, int32_t data)
{
	uint8_t str_buf[33];
	itoa(data, str_buf, 33);
	simple_uart_putstring(label);
	simple_uart_putstring(str_buf);
	//simple_uart_putstring("\r\n");
}

void print_measure_data(measurements* measure) {
    printData("AXIS: ", measure->axis);
    printData(" MAX: ", measure->max);
    printData(" MIN: ", measure->min);
	  printData(" INTER: ", measure->interval);
    printData(" THRESH: ", measure->threshold);
    printData(" PREC: ", measure->precision);
		simple_uart_putstring("\r\n");
}

void print_csv(int num_step) {
	int i;
	for(i = 0; i < collected_data; i++)
	{	
 		mlog_num(acc_arr[i].x);
 		mlog_str(",");
		mlog_num(acc_arr[i].y);
		mlog_str(",");
		mlog_num(acc_arr[i].z);
		mlog_str(",");
		mlog_num(data.axis);
		mlog_str(",");
//     mlog_num(data.max);
// 		mlog_str(",");
//     mlog_num(data.min);
// 		mlog_str(",");
    mlog_num(data.threshold);
		mlog_str(",");
//     mlog_num(data.precision);
// 		mlog_str(",");
		mlog_num(num_step);
 		//mlog_str(",");
// 		mlog_num(total_steps);
		mlog_str("\r\n");
	}
}
void print_csv_header() {
		mlog_str("X");
		mlog_str(",");
		mlog_str("Y");
		mlog_str(",");
		mlog_str("Z");
		mlog_str(",");
		mlog_str("AXIS");
		mlog_str(",");
	
//     mlog_str("MAX");
// 		mlog_str(",");
//     mlog_str("MIN");
// 		mlog_str(",");
    mlog_str("THRESH");
		mlog_str(",");
//     mlog_str("PREC");
// 		mlog_str(",");
		mlog_str("STEPS");
// 		mlog_str(",");
// 		mlog_str("TOTAL_STEPS");
		mlog_str("\r\n");
}

void print_acc_data_array(acc_data_t* acc_data_array, int size) {
  int i;
  for(i = 0; i < size; i++) {
		printData("x: ", acc_data_array[i].x);
    printData("y: ", acc_data_array[i].y);
    printData("z: ", acc_data_array[i].z);
  }
}

int fill_data(acc_data_t* acc_array) {
    int max, temp;
    if(collected_data >= SAMPLE_SIZE) {
        collected_data = 0;
    }
    
    temp = collected_data + FIFO_SAMPLES;
    max = (temp < SAMPLE_SIZE) ? temp : SAMPLE_SIZE;
    for(; collected_data < max; collected_data++) {
        update_acc_data(acc_array + collected_data);
    }
    if(collected_data >= SAMPLE_SIZE)
        return 1;
    return 0;
}

/** GPIOTE interrupt handler.
* Triggered on motion interrupt pin input low-to-high transition.
*/
void GPIOTE_IRQHandler(void)
{
	//simple_uart_putstring("Handling\r\n");
    int steps;
    if(fill_data(acc_arr)) {				
				filter(acc_arr, SAMPLE_SIZE);	
        get_max_min(&data, acc_arr, SAMPLE_SIZE);
        steps = count_steps1(&data, acc_arr, SAMPLE_SIZE);
				total_steps += steps;
				printData("STEPS:" , total_steps);
				mlog_str(" ");
				print_measure_data(&data);
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
	NVIC_DisableIRQ(GPIOTE_IRQn);
	data.interval = 10;
	data.temp_steps = 0;
  mlog_init();
	mlog_str("Waiting for Key...\r\n");
	uint8_t cr = simple_uart_get();
	mlog_str("Starting after key...\r\n");
	gpiote_init();
  NVIC_EnableIRQ(GPIOTE_IRQn);
  __enable_irq();
	acc_init();
	while(true) {}
}

/**
 *@}
 **/
