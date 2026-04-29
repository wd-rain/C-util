#include "i2c.h"

static void _i2c_assert_mem_address(uint16_t mem_address, I2cMemAddressSize mem_address_size)
{
    ASSERT(mem_address_size >= I2C_MEM_ADDRESS_SIZE_8BIT && mem_address_size <= I2C_MEM_ADDRESS_SIZE_16BIT);

    if (mem_address_size == I2C_MEM_ADDRESS_SIZE_8BIT)
    {
        ASSERT(mem_address <= UINT8_MAX);
    }
}

static void _i2c_assert_config(const I2cConfig* config)
{
    ASSERT(config != NULL);
    ASSERT(config->clock_hz > 0U);
}

static I2cConfig _i2c_default_config(void)
{
    I2cConfig config = {
        I2C_DEFAULT_CLOCK_HZ
    };

    _i2c_assert_config(&config);
    return config;
}

static void _i2c_assert_ops(const I2cOps* ops)
{
    ASSERT(ops != NULL);
    ASSERT(ops->config != NULL);
    ASSERT(ops->write != NULL);
    ASSERT(ops->read != NULL);
    ASSERT(ops->mem_write != NULL);
    ASSERT(ops->mem_read != NULL);
    ASSERT(ops->deinit != NULL);
}

static void _i2c_assert_gpio_pin_config(const GpioConfig* config)
{
    ASSERT(config != NULL);
    ASSERT(config->mode >= GPIO_MODE_INPUT && config->mode <= GPIO_MODE_ANALOG);
    ASSERT(config->pull >= GPIO_PULL_NONE && config->pull <= GPIO_PULL_DOWN);
    ASSERT(config->speed >= GPIO_SPEED_LOW && config->speed <= GPIO_SPEED_VERY_HIGH);
    ASSERT(config->output_type >= GPIO_OUTPUT_PUSH_PULL && config->output_type <= GPIO_OUTPUT_OPEN_DRAIN);
    ASSERT(config->alternate >= GPIO_ALTERNATE_NONE && config->alternate <= GPIO_ALTERNATE_15);
    ASSERT(config->level >= GPIO_LEVEL_LOW && config->level <= GPIO_LEVEL_HIGH);
}

static void _i2c_assert_gpio_config(const I2cGpioConfig* gpio_cfg)
{
    if (gpio_cfg != NULL)
    {
        ASSERT(gpio_cfg->gpio_ops != NULL);
        _i2c_assert_gpio_pin_config(&gpio_cfg->scl_config);
        _i2c_assert_gpio_pin_config(&gpio_cfg->sda_config);
    }
}

static void _i2c_assert_ready(const I2c* self)
{
    ASSERT(self != NULL);
    ASSERT(self->ops != NULL);
}

static void _i2c_clear(I2c* self)
{
    ASSERT(self != NULL);

    self->ops = NULL;
    self->bus = 0U;
    self->config = _i2c_default_config();
    self->scl.ops = NULL;
    self->scl.pin = 0U;
    self->sda.ops = NULL;
    self->sda.pin = 0U;
}

static void _i2c_init_gpio(I2c* self, const I2cGpioConfig* gpio_cfg)
{
    ASSERT(self != NULL);
    ASSERT(gpio_cfg != NULL);
    ASSERT(gpio_cfg->gpio_ops != NULL);

    gpio_init(&self->scl, gpio_cfg->gpio_ops, gpio_cfg->scl_pin);
    gpio_config(&self->scl, &gpio_cfg->scl_config);
    gpio_init(&self->sda, gpio_cfg->gpio_ops, gpio_cfg->sda_pin);
    gpio_config(&self->sda, &gpio_cfg->sda_config);
}

static void _i2c_deinit_gpio(I2c* self)
{
    ASSERT(self != NULL);

    if (self->scl.ops != NULL || self->sda.ops != NULL)
    {
        ASSERT(self->scl.ops != NULL);
        ASSERT(self->sda.ops != NULL);
        gpio_deinit(&self->scl);
        gpio_deinit(&self->sda);
    }
}

I2cStatus i2c_init(I2c* self, const I2cOps* ops, size_t bus, const I2cConfig* config, const I2cGpioConfig* gpio_cfg)
{
    I2cStatus status;

    ASSERT(self != NULL);
    _i2c_assert_ops(ops);
    _i2c_assert_config(config);
    _i2c_assert_gpio_config(gpio_cfg);

    _i2c_clear(self);
    self->ops = ops;
    self->bus = bus;

    if (gpio_cfg != NULL)
    {
        _i2c_init_gpio(self, gpio_cfg);
    }

    status = self->ops->config(self->bus, config);
    ASSERT(status >= I2C_STATUS_OK && status <= I2C_STATUS_OVERFLOW);

    if (status != I2C_STATUS_OK)
    {
        _i2c_deinit_gpio(self);
        _i2c_clear(self);
        return status;
    }

    self->config = *config;
    return I2C_STATUS_OK;
}

I2cStatus i2c_config(I2c* self, const I2cConfig* config)
{
    I2cStatus status;

    _i2c_assert_ready(self);
    _i2c_assert_config(config);
    ASSERT(self->ops->config != NULL);

    status = self->ops->config(self->bus, config);
    ASSERT(status >= I2C_STATUS_OK && status <= I2C_STATUS_OVERFLOW);

    if (status == I2C_STATUS_OK)
    {
        self->config = *config;
    }

    return status;
}

I2cStatus i2c_write(I2c* self, uint8_t address, const uint8_t* data, size_t len, uint32_t timeout_ms)
{
    I2cStatus status;

    _i2c_assert_ready(self);
    ASSERT(self->ops->write != NULL);
    ASSERT(address <= I2C_ADDRESS_MAX);
    ASSERT(data != NULL || len == 0U);

    status = self->ops->write(self->bus, address, data, len, timeout_ms);
    ASSERT(status >= I2C_STATUS_OK && status <= I2C_STATUS_OVERFLOW);

    return status;
}

I2cStatus i2c_read(I2c* self, uint8_t address, uint8_t* data, size_t len, uint32_t timeout_ms)
{
    I2cStatus status;

    _i2c_assert_ready(self);
    ASSERT(self->ops->read != NULL);
    ASSERT(address <= I2C_ADDRESS_MAX);
    ASSERT(data != NULL || len == 0U);

    status = self->ops->read(self->bus, address, data, len, timeout_ms);
    ASSERT(status >= I2C_STATUS_OK && status <= I2C_STATUS_OVERFLOW);

    return status;
}

I2cStatus i2c_mem_write(I2c* self, uint8_t address, uint16_t mem_address, I2cMemAddressSize mem_address_size, const uint8_t* data, size_t len, uint32_t timeout_ms)
{
    I2cStatus status;

    _i2c_assert_ready(self);
    ASSERT(self->ops->mem_write != NULL);
    ASSERT(address <= I2C_ADDRESS_MAX);
    _i2c_assert_mem_address(mem_address, mem_address_size);
    ASSERT(data != NULL || len == 0U);

    status = self->ops->mem_write(self->bus, address, mem_address, mem_address_size, data, len, timeout_ms);
    ASSERT(status >= I2C_STATUS_OK && status <= I2C_STATUS_OVERFLOW);

    return status;
}

I2cStatus i2c_mem_read(I2c* self, uint8_t address, uint16_t mem_address, I2cMemAddressSize mem_address_size, uint8_t* data, size_t len, uint32_t timeout_ms)
{
    I2cStatus status;

    _i2c_assert_ready(self);
    ASSERT(self->ops->mem_read != NULL);
    ASSERT(address <= I2C_ADDRESS_MAX);
    _i2c_assert_mem_address(mem_address, mem_address_size);
    ASSERT(data != NULL || len == 0U);

    status = self->ops->mem_read(self->bus, address, mem_address, mem_address_size, data, len, timeout_ms);
    ASSERT(status >= I2C_STATUS_OK && status <= I2C_STATUS_OVERFLOW);

    return status;
}

I2cStatus i2c_is_device_ready(I2c* self, uint8_t address, size_t trials, uint32_t timeout_ms)
{
    I2cStatus status;
    size_t i;

    _i2c_assert_ready(self);
    ASSERT(address <= I2C_ADDRESS_MAX);
    ASSERT(trials > 0U);

    status = I2C_STATUS_ERROR;
    for (i = 0U; i < trials; ++i)
    {
        status = i2c_write(self, address, NULL, 0U, timeout_ms);
        if (status == I2C_STATUS_OK)
        {
            return I2C_STATUS_OK;
        }
    }

    return status;
}

I2cStatus i2c_scan(I2c* self, uint8_t* addresses, size_t capacity, size_t* found_count, uint32_t timeout_ms)
{
    I2cStatus status;
    I2cStatus probe;
    size_t address;
    size_t count;

    _i2c_assert_ready(self);
    ASSERT(I2C_SCAN_ADDRESS_MIN <= I2C_SCAN_ADDRESS_MAX);
    ASSERT(I2C_SCAN_ADDRESS_MAX <= I2C_ADDRESS_MAX);
    ASSERT(addresses != NULL || capacity == 0U);
    ASSERT(found_count != NULL);

    status = I2C_STATUS_OK;
    count = 0U;
    for (address = I2C_SCAN_ADDRESS_MIN; address <= I2C_SCAN_ADDRESS_MAX; ++address)
    {
        probe = i2c_is_device_ready(self, (uint8_t)address, 1U, timeout_ms);
        if (probe == I2C_STATUS_OK)
        {
            if (count >= capacity)
            {
                status = I2C_STATUS_OVERFLOW;
                break;
            }
            addresses[count] = (uint8_t)address;
            ++count;
        }
        else if (probe != I2C_STATUS_NACK)
        {
            status = probe;
            break;
        }
    }

    *found_count = count;
    return status;
}

void i2c_deinit(I2c* self)
{
    _i2c_assert_ready(self);
    ASSERT(self->ops->deinit != NULL);

    self->ops->deinit(self->bus);
    _i2c_deinit_gpio(self);
    _i2c_clear(self);
}
