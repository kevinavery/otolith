

#include "motor.h"
#include "nrf_gpio.h"

#define MOTOR_PIN_NO   6

void motor_init(void)
{
    nrf_gpio_cfg_output(MOTOR_PIN_NO);
}

void motor_on(void)
{
    nrf_gpio_pin_set(MOTOR_PIN_NO);
}


void motor_off(void)
{
    nrf_gpio_pin_clear(MOTOR_PIN_NO);
}


