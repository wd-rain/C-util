#include "stm32f4xx_hal.h"
#include "bsp_gpio.h"

static GPIO_TypeDef *_bsp_gpio_port(size_t pin)
{
    GPIO_TypeDef *ports[] = {
        GPIOA,
        GPIOB,
        GPIOC,
        GPIOD,
        GPIOE,
        GPIOF,
        GPIOG,
        GPIOH,
        GPIOI
    };
    size_t port_index;

    port_index = (pin >> 8U) & 0xFFU;
    ASSERT(port_index < (sizeof(ports) / sizeof(ports[0])));

    return ports[port_index];
}

static uint16_t _bsp_gpio_pin_mask(size_t pin)
{
    size_t pin_index;

    pin_index = pin & 0xFFU;
    ASSERT(pin_index <= 15U);

    return (uint16_t)(1U << pin_index);
}

static void _bsp_gpio_enable_clock(size_t pin)
{
    size_t port_index;

    port_index = (pin >> 8U) & 0xFFU;
    switch (port_index)
    {
    case 0U:
        __HAL_RCC_GPIOA_CLK_ENABLE();
        break;
    case 1U:
        __HAL_RCC_GPIOB_CLK_ENABLE();
        break;
    case 2U:
        __HAL_RCC_GPIOC_CLK_ENABLE();
        break;
    case 3U:
        __HAL_RCC_GPIOD_CLK_ENABLE();
        break;
    case 4U:
        __HAL_RCC_GPIOE_CLK_ENABLE();
        break;
    case 5U:
        __HAL_RCC_GPIOF_CLK_ENABLE();
        break;
    case 6U:
        __HAL_RCC_GPIOG_CLK_ENABLE();
        break;
    case 7U:
        __HAL_RCC_GPIOH_CLK_ENABLE();
        break;
    case 8U:
        __HAL_RCC_GPIOI_CLK_ENABLE();
        break;
    default:
        ASSERT(0);
    }
}

static uint32_t _bsp_gpio_hal_mode(const GpioConfig *config)
{
    ASSERT(config != NULL);

    switch (config->mode)
    {
    case WD_GPIO_MODE_INPUT:
        return GPIO_MODE_INPUT;
    case WD_GPIO_MODE_OUTPUT:
        return config->output_type == WD_GPIO_OUTPUT_OPEN_DRAIN ? GPIO_MODE_OUTPUT_OD : GPIO_MODE_OUTPUT_PP;
    case WD_GPIO_MODE_ALTERNATE:
        return config->output_type == WD_GPIO_OUTPUT_OPEN_DRAIN ? GPIO_MODE_AF_OD : GPIO_MODE_AF_PP;
    case WD_GPIO_MODE_ANALOG:
        return GPIO_MODE_ANALOG;
    default:
        ASSERT(0);
    }
}

static uint32_t _bsp_gpio_hal_pull(GpioPull pull)
{
    switch (pull)
    {
    case WD_GPIO_PULL_NONE:
        return GPIO_NOPULL;
    case WD_GPIO_PULL_UP:
        return GPIO_PULLUP;
    case WD_GPIO_PULL_DOWN:
        return GPIO_PULLDOWN;
    default:
        ASSERT(0);
    }
}

static uint32_t _bsp_gpio_hal_speed(GpioSpeed speed)
{
    switch (speed)
    {
    case WD_GPIO_SPEED_LOW:
        return GPIO_SPEED_FREQ_LOW;
    case WD_GPIO_SPEED_MEDIUM:
        return GPIO_SPEED_FREQ_MEDIUM;
    case WD_GPIO_SPEED_HIGH:
        return GPIO_SPEED_FREQ_HIGH;
    case WD_GPIO_SPEED_VERY_HIGH:
        return GPIO_SPEED_FREQ_VERY_HIGH;
    default:
        ASSERT(0);
    }
}

static uint32_t _bsp_gpio_hal_alternate(GpioAlternate alternate)
{
    ASSERT(alternate <= WD_GPIO_ALTERNATE_15);

    if (alternate == WD_GPIO_ALTERNATE_NONE)
    {
        return 0U;
    }

    return (uint32_t)(alternate - WD_GPIO_ALTERNATE_0);
}

static void _bsp_gpio_config(size_t pin, const GpioConfig *config)
{
    GPIO_InitTypeDef init;

    ASSERT(config != NULL);

    _bsp_gpio_enable_clock(pin);

    init.Pin = _bsp_gpio_pin_mask(pin);
    init.Mode = _bsp_gpio_hal_mode(config);
    init.Pull = _bsp_gpio_hal_pull(config->pull);
    init.Speed = _bsp_gpio_hal_speed(config->speed);
    init.Alternate = _bsp_gpio_hal_alternate(config->alternate);

    HAL_GPIO_WritePin(_bsp_gpio_port(pin), init.Pin, config->level == WD_GPIO_LEVEL_HIGH ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_Init(_bsp_gpio_port(pin), &init);
}

static void _bsp_gpio_write(size_t pin, GpioLevel level)
{
    HAL_GPIO_WritePin(_bsp_gpio_port(pin), _bsp_gpio_pin_mask(pin), level == WD_GPIO_LEVEL_HIGH ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static GpioLevel _bsp_gpio_read(size_t pin)
{
    GPIO_PinState state;

    state = HAL_GPIO_ReadPin(_bsp_gpio_port(pin), _bsp_gpio_pin_mask(pin));

    return state == GPIO_PIN_SET ? WD_GPIO_LEVEL_HIGH : WD_GPIO_LEVEL_LOW;
}

const GpioOps *bsp_gpio_ops(void)
{
    static const GpioOps ops = {
        _bsp_gpio_config,
        _bsp_gpio_write,
        _bsp_gpio_read
    };

    return &ops;
}
