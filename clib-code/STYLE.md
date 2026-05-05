# clib-code 代码风格指南

---

## 第一部分：风格不一致审计

以下为对全部 10 个源文件逐行审查后发现的 12 处风格不一致。每条给出当前状态、统一结论和修改建议。

---

### 1. 头文件保护方式

| 文件 | 当前写法 |
|------|----------|
| `until/until.h:1` | `#pragma once` |
| 其余 5 个 `.h` 文件 | `#ifndef _NAME_H_` / `#define _NAME_H_` / `#endif` |

**结论**：统一使用 `#ifndef` 风格。`#pragma once` 不是 C 标准，在部分嵌入式工具链上不可靠。

```c
// ✗ 不推荐
#pragma once

// ✓ 推荐
#ifndef _UNTIL_H_
#define _UNTIL_H_
// ...
#endif
```

---

### 2. 区域注释用语

| 文件 | 当前写法 |
|------|----------|
| `tool/cmdline/cmdline.h:15` | `// 类定义` |
| `platform/gpio/gpio.h:34`、`platform/i2c/i2c.h:25`、`os/timer/timer.h:21` | `// 类型定义` |

**结论**：统一使用 `// 类型定义`。

---

### 3. typedef 闭合花括号空格

| 文件 | 当前写法 |
|------|----------|
| `tool/cmdline/cmdline.h:23` | `}CmdLine;` |
| 其余所有文件 | `} TypeName;` |

**结论**：`}` 与类型名之间保留一个空格。

```c
// ✗
}CmdLine;

// ✓
} CmdLine;
```

---

### 4. 指针星号对齐

| 文件 | 当前写法 |
|------|----------|
| `os/timer/timer.c` 全文件 (~70 处) | `Type *name`（星号靠近变量名） |
| `os/timer/timer.h` 及其余所有文件 | `Type *name`（星号靠近变量名） |

**结论**：统一星号靠近变量名。

```c
// ✗
const Timer *timer;
TimerScheduler *self;

// ✓
const Timer *timer;
TimerScheduler *self;
```

---

### 5. 函数指针 typedef 括号空格

| 文件 | 当前写法 |
|------|----------|
| `tool/cmdline/cmdline.h:16` | `typedef int (*cmd_line_fn)(...)` |
| 其余所有文件 | `typedef void (*gpio_config_fn)(...)` |

**结论**：`(*` 与函数名之间不加空格。

```c
// ✗
typedef int (*cmd_line_fn)(int argc, char **argv);

// ✓
typedef int (*cmd_line_fn)(int argc, char **argv);
```

---

### 6. 模块命名前缀不一致

| 模块 | 函数前缀 | 结构体 tag | typedef |
|------|----------|-----------|---------|
| gpio | `gpio_` | `gpio_t` | `Gpio` |
| i2c | `i2c_` | `i2c_t` | `I2c` |
| timer | `timer_` | `timer_t` | `Timer` |
| cmdline | `cmd_line_` | `cmdline_t` | `CmdLine` |

cmdline 模块函数用 `cmd_line_` 前缀（双词带下划线），但结构体 tag 用 `cmdline_t`（单词无下划线）。

**结论**：已有模块不做破坏性重命名。新模块必须保证函数前缀、结构体 tag、typedef 的词法拆分完全一致。例如：若函数用 `cmd_line_`，则 tag 应为 `cmd_line_t`，typedef 应为 `CmdLine`。

---

### 7. 错误处理哲学

| 模块 | 返回类型 | 校验方式 |
|------|----------|----------|
| gpio | `void` | `ASSERT` |
| i2c | `I2cStatus` 枚举 | `ASSERT` |
| timer | `int`（-1/0） | `ASSERT` |
| cmdline | `int`（0 为失败） | `if (x == NULL) return;` |

**结论**：两种错误处理方式均可接受，按每个函数自身是否"必须执行"来决定，同一模块内允许混用：

- **必须成功的函数**（有明确前置条件，调用者负责保证）：使用 `ASSERT`。违反前置条件属于调用者 bug，应在开发阶段立即暴露。
- **可选函数**（本身不被要求一定调用，空调用应静默通过）：使用防御性检查，无效调用不执行、不报错。

返回值策略按场景分层，见第二部分 §6。

---

### 8. 参数校验方式

| 文件 | 当前写法 |
|------|----------|
| `tool/cmdline/cmdline.c:99,114,173` | `if (self == NULL) { return; }` |
| 其余所有 `.c` 文件 | `ASSERT(self != NULL)` |

**结论**：与第 7 条一致，按每个函数自身是否"必须执行"来选择校验方式，同一模块内允许混用。

---

### 9. const 正确性

| 文件 | 情况 |
|------|------|
| `tool/cmdline/cmdline.h:28-29` | getter 函数使用 `const CmdLine *self` |
| `platform/gpio/gpio.h:123` | `gpio_read` 取 `Gpio *self`（非 const） |
| timer.h、i2c.h | 无 const self |

**结论**：纯查询函数（不修改 self 状态）应使用 `const Self *self`。`gpio_read` 因内部更新 `self->config.level` 缓存，技术上不适用 const，属于合理的设计选择。

---

### 10. 文件尾部多余空行

| 文件 | 情况 |
|------|------|
| `tool/cmdline/cmdline.h:32-33` | `#endif` 前有 2 行空行 |
| `until/until.h:95` | 文件尾有 1 行空行 |
| 其余文件 | `#endif` 前无多余空行 |

**结论**：`#endif` 前不留空行，文件末尾保留恰好 1 个换行符。

---

### 11. 标准头文件包含位置

| 文件 | 情况 |
|------|------|
| `tool/cmdline/cmdline.c:3` | 在 `.c` 文件中包含 `<stddef.h>` |
| `tool/cmdline/cmdline.h` | 未包含任何标准头文件 |
| 其余模块（gpio.h、i2c.h、timer.h） | 在 `.h` 文件中包含所需标准头 |

**结论**：头文件应自包含——在 `.h` 文件中包含其公开接口所需的全部标准头。

---

### 12. static inline 用法

| 文件 | 情况 |
|------|------|
| `tool/cmdline/cmdline.c:7` | 直接使用 `static inline` |
| `until/until.h:41` 等 | 定义了跨编译器 `INLINE` 宏 |

**结论**：如果模块已包含 `until.h`，应优先使用 `INLINE` 宏以保持跨编译器兼容性。若模块设计上不依赖 `until.h`，则 `static inline` 可接受。

---

## 第二部分：统一风格规范

以下规范以 gpio 模块为参考标准，其风格最为一致。

---

### 1. 文件组织

#### 1.1 目录结构

每个模块一个目录，目录名与模块名一致，包含同名 `.h` 和 `.c` 文件：

```
模块名/
├── 模块名.h
└── 模块名.c
```

#### 1.2 头文件结构

头文件按以下顺序组织，每个区域用中文注释分隔：

```c
#ifndef _MODULE_H_
#define _MODULE_H_

#include <stddef.h>      // 标准头文件

// 依赖
#include "../../until/until.h"

// 配置
#ifndef MODULE_CONFIG_VALUE
#define MODULE_CONFIG_VALUE 默认值
#endif

// 类型定义
typedef enum module_status_t { ... } ModuleStatus;
typedef struct module_config_t { ... } ModuleConfig;
typedef ReturnType (*module_action_fn)(...);
typedef struct module_ops_t { ... } ModuleOps;
typedef struct module_t { ... } Module;

// 接口
void module_init(Module *self, ...);
void module_deinit(Module *self);

#endif
```

#### 1.3 源文件结构

```c
#include "module.h"

// 静态辅助函数（按依赖顺序排列）
// 1. _module_assert_*    参数校验
// 2. _module_default_*   默认值构造
// 3. _module_*_checked   内部操作（含状态断言）
// 4. 其余内部逻辑

// 公开接口实现（与 .h 声明顺序一致）
```

#### 1.4 头文件保护

使用 `#ifndef` 风格，格式为 `_MODULENAME_H_`：

```c
#ifndef _GPIO_H_
#define _GPIO_H_
// ...
#endif
```

#### 1.5 Include 顺序

1. 自身头文件（仅 `.c` 文件）
2. 标准库头文件（`<stddef.h>`、`<stdint.h>` 等）
3. 项目内部依赖（`until.h`、其他模块头文件）

---

### 2. 命名规范

| 元素 | 风格 | 示例 |
|------|------|------|
| 文件名 | 小写，与模块同名 | `gpio.h`、`timer.c` |
| 公开函数 | `模块_动作` snake_case | `gpio_init`、`timer_start` |
| 静态函数 | `_模块_名称` 下划线前缀 | `_gpio_assert_ops`、`_timer_clear` |
| 局部变量 | snake_case | `timer_tick`、`active_head` |
| 宏常量 | UPPER_CASE | `TIMER_POOL_SIZE`、`WD_GPIO_LEVEL_LOW` |
| 函数式宏 | 通用工具用小写，模块宏用 UPPER_CASE | `min()`、`BIT()`、`SET_BIT()` |
| 枚举值 | `WD_MODULE_PREFIX_NAME` UPPER_CASE | `WD_GPIO_MODE_INPUT`、`WD_I2C_STATUS_OK` |
| typedef 类型名 | PascalCase | `GpioConfig`、`TimerTick`、`I2cStatus` |
| struct/enum tag | `snake_case_t` | `gpio_config_t`、`i2c_status_t` |
| 函数指针 typedef | `模块_描述_fn` | `gpio_config_fn`、`timer_action_fn` |
| ops 结构体 | `模块_ops_t` / `ModuleOps` | `gpio_ops_t` / `GpioOps` |

**模块前缀规则**：函数前缀、struct tag 前缀、typedef 前缀必须词法拆分一致。

---

### 3. 格式规范

#### 3.1 缩进

4 个空格，不使用 Tab。

#### 3.2 花括号风格

Allman 风格——`{` 独占一行：

```c
typedef struct gpio_config_t
{
    GpioMode mode;
    GpioPull pull;
} GpioConfig;

void gpio_init(Gpio *self, const GpioOps *ops, size_t pin)
{
    ASSERT(self != NULL);
    // ...
}

if (status != WD_I2C_STATUS_OK)
{
    return status;
}
```

#### 3.3 指针星号

星号靠近变量名：

```c
Gpio *self;
const GpioConfig *config;
const GpioOps *ops;
Timer **current;
```

#### 3.4 typedef 花括号

`}` 与类型名之间保留一个空格：

```c
typedef struct gpio_t
{
    const GpioOps *ops;
    size_t pin;
    GpioConfig config;
} Gpio;
```

#### 3.5 函数指针 typedef

`(*` 与名称之间不加空格：

```c
typedef void (*gpio_config_fn)(size_t pin, const GpioConfig *config);
typedef I2cStatus (*i2c_write_fn)(size_t bus, uint8_t address, const uint8_t *data, size_t len, uint32_t timeout_ms);
```

#### 3.6 空行

- 函数之间空 1 行
- 同类静态函数之间空 1 行
- `#endif` 前不留空行
- 文件末尾保留恰好 1 个换行符

#### 3.7 行宽

无硬性限制。函数参数过长时不强制换行，保持声明在一行内可读即可。

#### 3.8 变量声明

C89 风格——变量声明集中在函数/块的开头：

```c
int timer_delete(TimerScheduler *self, TimerId id)
{
    Timer *timer;
    int result;

    result = -1;
    // ...
}
```

#### 3.9 无符号字面量

整型字面量加 `U` 后缀：`0U`、`1U`、`UINT32_MAX`。

---

### 4. 预处理器

#### 4.1 配置宏

每个模块通过可覆盖宏提供编译期配置，使用 `#ifndef` 包裹默认值：

```c
#ifndef TIMER_POOL_SIZE
#define TIMER_POOL_SIZE 16U
#endif
```

#### 4.2 编译期校验

对不合法的配置值使用 `#error` 报错：

```c
#if TIMER_POOL_SIZE == 0U
#error "TIMER_POOL_SIZE must be greater than 0"
#endif
```

#### 4.3 条件编译

平台层通过 `PLATFORM_USE_模块` 控制模块启用，并检查依赖关系：

```c
#if PLATFORM_USE_I2C && !PLATFORM_USE_GPIO
#error "PLATFORM_USE_I2C requires PLATFORM_USE_GPIO"
#endif
```

#### 4.4 编译器兼容宏

跨编译器属性统一通过 `until.h` 中的宏提供：`WEAK`、`UNUSED`、`PACKED`、`INLINE`、`NORETURN`、`DEPRECATED`、`ALIGNED(n)`、`SECTION(s)`、`LIKELY(x)`、`UNLIKELY(x)`。

---

### 5. 类型与结构体

#### 5.1 typedef 模板

```c
// 简单类型别名
typedef uint32_t TimerTick;

// 枚举
typedef enum gpio_level_t
{
    WD_GPIO_LEVEL_LOW = 0,
    WD_GPIO_LEVEL_HIGH
} GpioLevel;

// 结构体
typedef struct gpio_config_t
{
    GpioMode mode;
    GpioPull pull;
    GpioSpeed speed;
} GpioConfig;

// 前向声明（用于不透明类型）
typedef struct timer_scheduler_t TimerScheduler;
```

#### 5.2 Ops 结构体（策略模式）

硬件抽象和平台适配统一使用函数指针表：

```c
// 先定义函数指针类型
typedef void (*gpio_config_fn)(size_t pin, const GpioConfig *config);
typedef void (*gpio_write_fn)(size_t pin, GpioLevel level);
typedef GpioLevel (*gpio_read_fn)(size_t pin);

// 再定义 ops 结构体
typedef struct gpio_ops_t
{
    gpio_config_fn config;
    gpio_write_fn write;
    gpio_read_fn read;
} GpioOps;
```

#### 5.3 模块主结构体

包含 ops 指针、标识、配置/状态：

```c
typedef struct gpio_t
{
    const GpioOps *ops;   // ops 始终为 const 指针
    size_t pin;
    GpioConfig config;
} Gpio;
```

---

### 6. 错误处理

#### 6.1 校验方式选择

根据函数的必要性选择参数校验方式：

**必须成功的函数**——使用 `ASSERT`：

适用于底层驱动、资源管理等核心函数，前置条件由调用者保证，违反即属于编程错误：

```c
void gpio_init(Gpio *self, const GpioOps *ops, size_t pin)
{
    ASSERT(self != NULL);
    _gpio_assert_ops(ops);
    // ...
}
```

**允许容错的函数**——使用防御性检查：

适用于工具类函数，输入来源不确定或失败是合理的运行时情况：

```c
void cmd_line_init(CmdLine *self, cmd_line_fn fn, const char *name, const char *desc)
{
    if (self == NULL)
    {
        return;
    }
    // ...
}
```

同一模块内允许混用，以每个函数自身的必要性为准。

#### 6.2 返回值策略分层

根据模块层级和错误复杂度选择返回值策略：

| 场景 | 返回类型 | 示例 |
|------|----------|------|
| 不会运行时失败的操作 | `void` | `gpio_init`、`gpio_write` |
| 有多种运行时失败模式的 IO 操作 | 专用状态枚举 | `I2cStatus`：OK/ERROR/BUSY/TIMEOUT/NACK |
| 简单成功/失败的资源管理操作 | `int`（0 成功，-1 失败） | `timer_start`、`timer_stop` |
| 工具类可选操作 | `int`（0 表示未匹配/无操作） | `cmd_line_exe` |

#### 6.3 内部断言辅助

将断言逻辑抽取为 `_module_assert_*` 静态函数，提高可读性和复用性：

```c
static void _gpio_assert_ops(const GpioOps *ops)
{
    ASSERT(ops != NULL);
    ASSERT(ops->config != NULL);
    ASSERT(ops->write != NULL);
    ASSERT(ops->read != NULL);
}

static void _gpio_assert_ready(const Gpio *self)
{
    ASSERT(self != NULL);
    _gpio_assert_ops(self->ops);
}
```

#### 6.4 内部操作辅助

对 ops 调用的封装使用 `_module_*_checked` 命名，含状态断言：

```c
static I2cStatus _i2c_write_checked(I2c *self, uint8_t address, const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    I2cStatus status;

    status = self->ops->write(self->bus, address, data, len, timeout_ms);
    _i2c_assert_status(status);

    return status;
}
```

---

### 7. 注释

#### 7.1 语言

使用中文。

#### 7.2 区域注释

头文件中使用固定的中文区域注释分隔各部分：

```c
// 依赖
// 配置
// 类型定义
// 接口
```

#### 7.3 行内注释

对关键算法或非显而易见的设计意图使用 `//` 行内注释：

```c
void gpio_init(Gpio *self, const GpioOps *ops, size_t pin); // 会将其设置成悬空输入
```

#### 7.4 块注释

对复杂逻辑使用 `/* */` 块注释，解释 **为什么** 而非 **是什么**：

```c
/* 使用无符号差值处理 tick 回绕，比较范围限制在半个 uint32_t tick 内。 */
return (TimerTick)(now - deadline) < _timer_tick_half_range;
```

#### 7.5 不写注释的情况

- 函数名和变量名已经足够说明意图时
- 解释代码"做了什么"（代码本身已表达）
- 引用具体 issue/PR 号或调用者

---

### 8. 模块结构模式

#### 8.1 self 参数

所有公开函数的第一个参数为 `Module *self`（面向对象 C 风格）：

```c
void gpio_init(Gpio *self, const GpioOps *ops, size_t pin);
void gpio_config(Gpio *self, const GpioConfig *config);
GpioLevel gpio_read(Gpio *self);
```

纯查询且不修改 self 状态的函数使用 `const Module *self`：

```c
const char *cmd_line_name(const CmdLine *self);
```

#### 8.2 init/deinit 生命周期

- `init`：校验参数 → 设置默认值 → 应用配置
- `deinit`：恢复默认状态 → 释放资源 → 清空指针

```c
void gpio_init(Gpio *self, const GpioOps *ops, size_t pin)
{
    ASSERT(self != NULL);
    _gpio_assert_ops(ops);

    self->ops = ops;
    self->pin = pin;
    self->config = _gpio_default_config();
    _gpio_apply_config_checked(self);
}

void gpio_deinit(Gpio *self)
{
    _gpio_assert_ready(self);

    self->config = _gpio_default_config();
    _gpio_apply_config_checked(self);

    self->ops = NULL;
    self->pin = 0U;
}
```

#### 8.3 静态辅助函数分层

按职责将内部函数分为三层，均使用 `_模块_` 前缀：

| 层次 | 命名模式 | 职责 |
|------|----------|------|
| 校验层 | `_module_assert_*` | 参数/状态的 ASSERT 校验 |
| 默认值层 | `_module_default_*` | 构造默认配置 |
| 操作层 | `_module_*_checked` | 封装 ops 调用，含状态断言 |

#### 8.4 公开接口函数体结构

每个公开函数遵循统一的三段式：

```c
ReturnType module_action(Module *self, ...)
{
    // 1. 声明局部变量（C89 风格）

    // 2. 前置条件断言
    _module_assert_ready(self);
    _module_assert_xxx(参数);

    // 3. 核心逻辑（调用 _checked 辅助函数）
    return _module_action_checked(self, ...);
}
```
