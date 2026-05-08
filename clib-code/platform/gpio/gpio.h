#ifndef _GPIO_H_
#define _GPIO_H_

#include <stddef.h>

// 依赖
#include "../../until/until.h"

// 配置
#ifndef GPIO_DEFAULT_MODE
#define GPIO_DEFAULT_MODE WD_GPIO_MODE_INPUT
#endif

#ifndef GPIO_DEFAULT_PULL
#define GPIO_DEFAULT_PULL WD_GPIO_PULL_NONE
#endif

#ifndef GPIO_DEFAULT_SPEED
#define GPIO_DEFAULT_SPEED WD_GPIO_SPEED_LOW
#endif

#ifndef GPIO_DEFAULT_OUTPUT_TYPE
#define GPIO_DEFAULT_OUTPUT_TYPE WD_GPIO_OUTPUT_PUSH_PULL
#endif

#ifndef GPIO_DEFAULT_ALTERNATE
#define GPIO_DEFAULT_ALTERNATE WD_GPIO_ALTERNATE_NONE
#endif

#ifndef GPIO_DEFAULT_LEVEL
#define GPIO_DEFAULT_LEVEL WD_GPIO_LEVEL_LOW
#endif

// 类型定义
typedef enum gpio_level_t
{
    WD_GPIO_LEVEL_LOW = 0,
    WD_GPIO_LEVEL_HIGH
} GpioLevel;

typedef enum gpio_mode_t
{
    WD_GPIO_MODE_INPUT = 0,
    WD_GPIO_MODE_OUTPUT,
    WD_GPIO_MODE_ALTERNATE,
    WD_GPIO_MODE_ANALOG
} GpioMode;

typedef enum gpio_pull_t
{
    WD_GPIO_PULL_NONE = 0,
    WD_GPIO_PULL_UP,
    WD_GPIO_PULL_DOWN
} GpioPull;

typedef enum gpio_speed_t
{
    WD_GPIO_SPEED_LOW = 0,
    WD_GPIO_SPEED_MEDIUM,
    WD_GPIO_SPEED_HIGH,
    WD_GPIO_SPEED_VERY_HIGH
} GpioSpeed;

typedef enum gpio_output_type_t
{
    WD_GPIO_OUTPUT_PUSH_PULL = 0,
    WD_GPIO_OUTPUT_OPEN_DRAIN
} GpioOutputType;

typedef enum gpio_alternate_t
{
    WD_GPIO_ALTERNATE_NONE = 0,
    WD_GPIO_ALTERNATE_0,
    WD_GPIO_ALTERNATE_1,
    WD_GPIO_ALTERNATE_2,
    WD_GPIO_ALTERNATE_3,
    WD_GPIO_ALTERNATE_4,
    WD_GPIO_ALTERNATE_5,
    WD_GPIO_ALTERNATE_6,
    WD_GPIO_ALTERNATE_7,
    WD_GPIO_ALTERNATE_8,
    WD_GPIO_ALTERNATE_9,
    WD_GPIO_ALTERNATE_10,
    WD_GPIO_ALTERNATE_11,
    WD_GPIO_ALTERNATE_12,
    WD_GPIO_ALTERNATE_13,
    WD_GPIO_ALTERNATE_14,
    WD_GPIO_ALTERNATE_15
} GpioAlternate;

typedef struct gpio_config_t
{
    GpioMode mode;
    GpioPull pull;
    GpioSpeed speed;
    GpioOutputType output_type;
    GpioAlternate alternate;
    GpioLevel level;
} GpioConfig;

typedef void (*gpio_config_fn)(size_t pin, const GpioConfig *config);
typedef void (*gpio_write_fn)(size_t pin, GpioLevel level);
typedef GpioLevel (*gpio_read_fn)(size_t pin);

typedef struct gpio_ops_t
{
    gpio_config_fn config;
    gpio_write_fn write;
    gpio_read_fn read;
} GpioOps;

typedef struct gpio_t
{
    const GpioOps *ops;
    size_t pin;
    GpioConfig config;
} Gpio;

// 接口
void gpio_init(Gpio *self, const GpioOps *ops, size_t pin);
void gpio_config(Gpio *self, const GpioConfig *config);
const GpioConfig *gpio_get_config(const Gpio *self);
void gpio_write(Gpio *self, GpioLevel level);
GpioLevel gpio_read(Gpio *self);
void gpio_toggle(Gpio *self);
void gpio_deinit(Gpio *self);

#endif
