#include "app.h"

#include <stdbool.h>
#include <stdint.h>

#include "bsp.h"
#include "main.h"
#include "os.h"

#define APP_BLINK_PERIOD_MS 500U

static bool flag = false;
static TimerScheduler timer_scheduler;
static TimerId blink_timer_id = TIMER_ID_INVALID;

static TimerTick _app_timer_get_tick(void *user_data)
{
    (void)user_data;
    return (TimerTick)HAL_GetTick();
}

static const TimerOps timer_ops = {
    _app_timer_get_tick,
};

void gpio_callback(GpioIsr *self)
{
    (void)self;
    flag = !flag;
}

static void _app_blink_timer_callback(TimerScheduler *scheduler, TimerId id, void *user_data)
{
    (void)scheduler;
    (void)id;
    (void)user_data;

    if (flag)
    {
        gpio_toggle(&led0);
    }
    else
    {
        gpio_toggle(&led1);
    }
}

void app_init(void)
{
    GpioConfig config;

    config = bsp_gpio_output_config(WD_GPIO_OUTPUT_PUSH_PULL, WD_GPIO_SPEED_LOW, WD_GPIO_LEVEL_LOW);
    gpio_config(&led0, &config);
    gpio_config(&led1, &config);
    timer_scheduler_init(&timer_scheduler, &timer_ops, NULL);
    gpio_isr_set_callback(&pe3_isr, gpio_callback);

    blink_timer_id = timer_new(&timer_scheduler);
    WD_ASSERT(blink_timer_id != TIMER_ID_INVALID);
    WD_ASSERT(timer_start(&timer_scheduler, blink_timer_id, APP_BLINK_PERIOD_MS, APP_BLINK_PERIOD_MS, _app_blink_timer_callback, NULL) == 0);
}

void app_task(void)
{
    (void)timer_scheduler_run_once(&timer_scheduler);
}
