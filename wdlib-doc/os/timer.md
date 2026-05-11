---
aliases:
  - timer
  - timer.h
depends:
  - "[[until]]"
tags:
  - c
  - clib
  - os
  - timer
---

# timer

`timer` 是 `[[os]]` 目录下的独立定时器模块。它不属于 `slco` 私有实现，也不依赖 event 或 slco。用户可以单独使用 timer，也可以在未来让 event 通过 timer action 把到期转换成事件。

## 依赖关系

代码路径：

- `wdlib-code/os/timer/timer.h`
- `wdlib-code/os/timer/timer.c`

`timer` 只依赖 `wdlib-code/until/until.h`，使用其中的 `WD_ASSERT` 暴露编程错误。

## 接口总览

| 类别   | 接口                           | 功能                              |
| ---- | ---------------------------- | ------------------------------- |
| 类型   | `TimerTick`                  | tick 计数类型，当前为 `uint32_t`        |
| 类型   | `TimerId`                    | timer 标识类型，当前为 `uint32_t`       |
| 宏    | `TIMER_POOL_SIZE`            | 每个 scheduler 内置 timer 池容量（≤4096）|
| 宏    | `TIMER_ID_INVALID`           | 无效 timer id，值为 `UINT32_MAX`     |
| 宏    | `TIMER_ID_MAGIC`             | id 编码魔数，值为 `0xA5`，不可为 `0xFF`   |
| Ops  | `timer_get_tick_fn`          | 获取当前 tick                       |
| 回调   | `timer_action_fn`            | timer 到期动作                      |
| 类型   | `TimerOps`                   | 平台操作集合                          |
| 类型   | `Timer`                      | 内部 timer 槽位结构                   |
| 类型   | `TimerScheduler`             | timer 调度器实例                     |
| 初始化  | `timer_scheduler_init`       | 初始化调度器和内部 timer 池               |
| 调度   | `timer_scheduler_run_once`   | 非阻塞触发一个到期 timer                 |
| 查询   | `timer_scheduler_now`        | 获取当前 tick                       |
| 查询   | `timer_scheduler_next_delay` | 查询距离最近到期 timer 的 tick 数         |
| 生命周期 | `timer_new`                  | 从内部池分配 timer id                 |
| 生命周期 | `timer_delete`               | 删除 timer                        |
| 控制   | `timer_start`                | 启动或重启 timer                     |
| 控制   | `timer_stop`                 | 停止 timer                        |
| 查询   | `timer_is_running`           | 查询 timer 是否运行                   |
| 查询   | `timer_remaining`            | 查询剩余 tick                       |

## 配置

### `TIMER_POOL_SIZE`

```c
#ifndef TIMER_POOL_SIZE
#define TIMER_POOL_SIZE 16U
#endif
```

该宏控制每个 `TimerScheduler` 实例内置 timer 池的容量。取值范围为 1–4096（受 `TimerId` 编码中 12 位 index 字段限制）。每个 scheduler 都有独立池，因此可以创建多个互不影响的 timer 调度器实例。

### `TIMER_ID_MAGIC`

```c
#define TIMER_ID_MAGIC 0xA5U
```

`TimerId` 编码中的魔数字段，用于快速判断一个 `uint32_t` 值是否为合法的 timer id。编译期强制该值不能为 `0xFF`，否则无法与 `TIMER_ID_INVALID`（`0xFFFFFFFF`）区分。

## 线程模型

`timer` 是单线程调度器模块。一个 `TimerScheduler` 应由一个线程拥有，同一个 scheduler 不允许被多个线程并发调用。多线程场景下，每个线程应创建并使用自己的独立 `TimerScheduler` 实例。

模块内部不提供锁、临界区或线程归属检查；如果用户跨线程共享同一个 scheduler，需要在调用层自行保证不会并发访问。

## 类型

### `TimerTick`

```c
typedef uint32_t TimerTick;
```

`TimerTick` 是 timer 使用的 tick 类型。模块使用无符号差值处理 tick 回绕，单次 `delay_ticks` 和非零 `period_ticks` 应小于半个 `uint32_t` tick 范围。

### `TimerId`

```c
typedef uint32_t TimerId;

#define TIMER_ID_INVALID UINT32_MAX
#define TIMER_ID_MAGIC   0xA5U
```

`TimerId` 是外部引用 timer 的唯一标识。`timer_new` 成功返回有效 id，失败返回 `TIMER_ID_INVALID`。

#### 编码布局

`TimerId` 内部按位域编码为 `[magic:8][generation:12][index:12]`：

```
Bit 31-24  magic       固定为 TIMER_ID_MAGIC (0xA5)，用于快速判断合法性
Bit 23-12  generation  代数计数器，每次释放槽位时递增，检测 use-after-free
Bit 11-0   index       池数组下标，实现 O(1) 查找
```

- **magic 校验**：随机 `uint32_t` 或 `TIMER_ID_INVALID`（magic = 0xFF）会在第一步被拒绝。
- **generation 校验**：已删除的 timer 其 generation 已递增，持有旧 id 的调用者无法匹配新 generation，`_timer_find` 返回 NULL。generation 为 12 位，同一槽位需 4096 次分配/释放后才会回绕。
- **index 定位**：直接以 index 下标访问 `timers[]` 数组，所有依赖 `_timer_find` 的操作（`timer_start`、`timer_stop`、`timer_delete`、`timer_is_running`、`timer_remaining`）均为 O(1)。

### `TimerOps`

```c
typedef TimerTick (*timer_get_tick_fn)(void* user_data);

typedef struct timer_ops_t
{
    timer_get_tick_fn get_tick;
} TimerOps;
```

`get_tick` 必须实现。`ops_user_data` 会作为参数传入 `get_tick`。`timer` 不提供内部临界区回调，同一个 scheduler 的调用方必须遵守单线程拥有模型。

### `timer_action_fn`

```c
typedef void (*timer_action_fn)(TimerScheduler* scheduler, TimerId id, void* user_data);
```

timer 到期后，`timer_scheduler_run_once` 会在 timer 从活动链表移除并标记触发中后调用 action。`user_data` 来自 `timer_start`。

### `Timer`

```c
typedef struct timer_t
{
    union
    {
        TimerId id;
        struct
        {
            uint32_t _index : 12;
            uint32_t generation : 12;
            uint32_t _magic : 8;
        };
    };
    TimerTick deadline;
    TimerTick period;
    timer_action_fn action;
    void *user_data;
    struct timer_t *next;
} Timer;
```

`Timer` 在头文件中公开是为了支持静态分配 `TimerScheduler`，但其字段由 timer 模块内部管理。用户不要直接修改这些字段。

`id` 与 `_magic`/`generation`/`_index` 通过匿名联合体共享同一块 32 位内存，无额外开销。`_magic != TIMER_ID_MAGIC` 表示空槽，timer 是否在 `active_head` 链表中表示是否正在运行，`next == timer` 是模块内部的触发中标记。释放槽位时只清除 `_magic`（置为 `0xFF`），`generation` 保留并递增，供下一次分配使用。

### `TimerScheduler`

```c
struct timer_scheduler_t
{
    const TimerOps *ops;
    void *ops_user_data;
    Timer timers[TIMER_POOL_SIZE];
    Timer *active_head;
};
```

`TimerScheduler` 持有 ops、内部 timer 池和按 `deadline` 排序的活动链表。id 分配不再需要全局递增计数器，每个槽位通过自身的 `generation` 和数组下标直接编码出唯一 id。

## 接口

### `timer_scheduler_init`

```c
void timer_scheduler_init(TimerScheduler* self, const TimerOps* ops, void* ops_user_data);
```

初始化 timer 调度器。`self`、`ops` 和 `ops->get_tick` 必须有效，否则触发 `WD_ASSERT`。

### `timer_scheduler_run_once`

```c
int timer_scheduler_run_once(TimerScheduler* self);
```

非阻塞推进一次 timer 调度。该函数最多触发一个到期 timer。没有到期 timer 时也返回 `0`。

### `timer_scheduler_now`

```c
TimerTick timer_scheduler_now(TimerScheduler* self);
```

返回当前 tick。

### `timer_scheduler_next_delay`

```c
TimerTick timer_scheduler_next_delay(TimerScheduler* self);
```

查询距离最近 timer 到期还有多少 tick。没有运行中的 timer 时返回 `(TimerTick)-1`。若最近 timer 已到期但尚未被 `run_once` 处理，返回 `0`。

### `timer_new`

```c
TimerId timer_new(TimerScheduler* self);
```

从 scheduler 内部池分配一个 timer，并返回新的 `TimerId`。分配时使用槽位的当前 `generation` 和数组下标编码 id。池满时返回 `TIMER_ID_INVALID`。

### `timer_delete`

```c
int timer_delete(TimerScheduler* self, TimerId id);
```

删除指定 timer。若 timer 正在运行，会先停止再释放池槽。释放时递增槽位的 `generation`，使所有持有旧 id 的外部引用自动失效。找不到 id 返回 `-1`。

### `timer_start`

```c
int timer_start(TimerScheduler* self, TimerId id, TimerTick delay_ticks, TimerTick period_ticks, timer_action_fn action, void* user_data);
```

启动或重启 timer。`delay_ticks` 表示从当前 tick 到首次触发的延迟。`period_ticks == 0` 表示一次性 timer，非 0 表示周期 timer。

### `timer_stop`

```c
int timer_stop(TimerScheduler* self, TimerId id);
```

停止运行中的 timer。找不到 id 或 timer 未运行时返回 `-1`。

### `timer_is_running`

```c
int timer_is_running(TimerScheduler* self, TimerId id);
```

查询 timer 是否正在运行。运行中返回 `1`，未运行返回 `0`，找不到 id 返回 `-1`。

### `timer_remaining`

```c
int timer_remaining(TimerScheduler* self, TimerId id);
```

查询运行中 timer 的剩余 tick。timer 已到期但尚未被 `run_once` 处理时返回 `0`。找不到 id 或 timer 未运行时返回 `-1`。

## 调度行为

- `timer_start` 使用当前 tick 加 `delay_ticks` 得到绝对 `deadline`。
- 活动 timer 按 `deadline` 排序插入链表。
- `timer_scheduler_run_once` 只检查链表头，因此无到期 timer 时开销很低。
- 一次性 timer 触发后从活动链表移除，保持已分配但未运行。
- 周期 timer 触发后按 `deadline += period_ticks` 重新插入链表。
- action 在 timer 从活动链表移除并标记触发中后执行，允许用户在 action 内 stop、delete 或 restart 当前 timer。

## 独立使用示例

```c
static TimerTick board_get_tick(void* user_data)
{
    return *(TimerTick*)user_data;
}

static void led_timeout(TimerScheduler* scheduler, TimerId id, void* user_data)
{
    (void)scheduler;
    (void)id;
    *(int*)user_data = 1;
}

void example(void)
{
    TimerTick tick = 0U;
    int fired = 0;
    TimerScheduler timer;
    TimerOps ops = { board_get_tick };
    TimerId id;

    timer_scheduler_init(&timer, &ops, &tick);
    id = timer_new(&timer);
    if (id != TIMER_ID_INVALID)
    {
        timer_start(&timer, id, 100U, 0U, led_timeout, &fired);
    }

    while (!fired)
    {
        ++tick;
        timer_scheduler_run_once(&timer);
    }
}
```

## 与 event/slco 的关系

event 层可以为 timer 注册一个 action，在 action 中调用 `event_post`，从而把 timer 到期转换成事件。slco 层再通过 event 等待 timer 到期。timer 自身不包含任何 event 或 slco 语义。
