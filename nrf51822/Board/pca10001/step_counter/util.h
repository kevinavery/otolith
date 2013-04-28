

#ifndef UTIL_H__
#define UTIL_H__

#include <stdint.h>
#include "simple_uart.h"

// For UART (debugging)
#define RX_PIN_NUMBER  11
#define TX_PIN_NUMBER  9
#define CTS_PIN_NUMBER 10
#define RTS_PIN_NUMBER 8
#define HWFC           true

// Logging functions
void mlog_init(void);
void mlog_str(const char * msg);
void mlog_num(int num);
void mlog_print(const char * msg, int num);

void itoa(int num, uint8_t *buf, uint8_t buf_len);

#endif
