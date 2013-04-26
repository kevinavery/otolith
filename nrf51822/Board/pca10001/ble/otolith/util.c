
#include "util.h"

void mlog_init(void)
{
	simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, HWFC);
}

void mlog_str(const char * msg)
{
	simple_uart_putstring((const uint8_t *) msg);
}

void mlog_num(int num)
{
	// 12 characters is enough to hold the largest 
	// 32-bit null-terminated signed number
	uint8_t buf[12];
	itoa(num, buf, 12);
	mlog_str((const char *) buf);
}

void itoa(int num, uint8_t *buf, uint8_t buf_len)
{
    int i = 0;
    int mask = 1000000000;

    if (num == 0) 
		{
        *buf = '0';
        *(buf+1) = 0;
        return;
    }

    if (num < 0) 
		{
        num = -num;
        *buf = '-';
        i++; 
    }

    while (mask > num)
    {
        mask = mask / 10;
    }

    while (mask > 0 && i < buf_len-1) 
		{
        *(buf + i) = num / mask + '0';
        num = num % mask;
        mask = mask / 10;
        i++;
    }

    *(buf+i) = 0;
}

