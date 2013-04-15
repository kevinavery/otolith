#include <spi_master.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "common.h"
#include "acc_driver.h"

#define MSG_LENGTH  100
#define WAIT_LENGTH  1<<20

static uint8_t tx_data[MSG_LENGTH]; /*!< SPI TX buffer */
static uint8_t rx_data[MSG_LENGTH]; /*!< SPI RX buffer */
static uint32_t* spi_ba = 0;
static acc_data_t acc_data;

int acc_init() {
  // Use SPI0, mode0 with lsb shifted as requested  
  volatile uint32_t counter = 0;

  spi_ba = spi_master_init(SPI0, SPI_MODE3, false);
  if (spi_ba == 0)
  {
    return -1;
  }

  //Put the ADXL345 into +/- 4G range by writing the value 0x01
  // to the DATA_FORMAT register.
  write_register(ADXL345_DATA_FORMAT, 0x00);

  //Put the ADXL345 into Measurement Mode by writing 0x08 
  // to the POWER_CTL register.
  write_register(ADXL345_POWER_CTL, 0x08);  //Measurement mode  
	return 0;
}

int write_register(uint8_t register_address, uint8_t value) 
{
  uint8_t tx[2];
  tx[0] = register_address;
  tx[1] = value;
	uint8_t rx[2];
  
	if(!spi_master_tx_rx( spi_ba, 2, tx, rx)) 
	{
		return -1;
	}
	
	return 0;
}

void clear_buffer(uint8_t *b, int blen)
{
	for(int i = 0; i < blen; i++)
	  b[i] = 0;
}

int read_register(uint8_t register_address, int num_bytes, uint8_t* values) 
{
  if(num_bytes > 1)   //  (uint8_t*)values
		tx_data[0] = 0xC0 | register_address;
	else
		tx_data[0] = 0x80 | register_address;

  if(!spi_master_tx_rx(spi_ba, num_bytes, tx_data, values)) 
	{
    return -1;
  }

  return 0;
}

acc_data_t * update_acc_data(void)
{
	uint8_t buffer[7];
	read_register(ADXL345_DATAX0, 7, buffer);
	acc_data.x = (buffer[2] << 8) | buffer[1];
	acc_data.y = (buffer[4] << 8) | buffer[3];
	acc_data.z = (buffer[6] << 8) | buffer[5];
	return &acc_data;
}






