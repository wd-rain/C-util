#include "bsp.h"

Gpio led0;
Gpio led1;


void bsp_init(void)
{
    gpio_init(&led0, bsp_gpio_ops(), BSP_GPIO_PIN_PF9);
    gpio_init(&led1, bsp_gpio_ops(), BSP_GPIO_PIN_PF10);
}
