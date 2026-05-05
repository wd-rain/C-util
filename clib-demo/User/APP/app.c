#include "app.h"

#include "main.h"
#include "bsp_gpio.h"
#include "platform/gpio/gpio.h"

extern Gpio led0;
extern Gpio led1;

void app_init(void)
{
    GpioConfig config = *gpio_get_config(&led0);
    config.mode = WD_GPIO_MODE_OUTPUT;
    gpio_config(&led0, &config);
    gpio_config(&led1, &config);
}

void app_task(void)
{
    gpio_write(&led0, WD_GPIO_LEVEL_HIGH);
    gpio_write(&led1, WD_GPIO_LEVEL_LOW);
    HAL_Delay(500);
    gpio_write(&led0, WD_GPIO_LEVEL_LOW);
    gpio_write(&led1, WD_GPIO_LEVEL_HIGH);
    HAL_Delay(500);
}
