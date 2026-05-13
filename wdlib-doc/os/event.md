---
aliases:
  - event
  - event.h
  - OS 事件调度
depends:
  - "[[timer]]"
tags:
  - c
  - clib
  - os
  - event
---

# event

`event` 是 `[[os]]` 目录下位于 `timer` 之上的 value 驱动事件调度模块。事件不再构造公开 `Event` 载荷，也不再携带 source 或事件级 `user_data`；调度器只维护每个 `EventId` 当前待处理的 `uint32_t value`。

`value == 0U` 表示当前无待处理事件，非 0 value 表示事件需要触发。除 `event_trigger` 外，post、event timer 和 monitor 都只把 value 合并进事件槽，handler 由 `event_scheduler_run_once` 统一调度。

## 依赖与配置

代码路径：

- `wdlib-code/os/event/event.h`
- `wdlib-code/os/event/event.c`

`event.h` 直接包含 `../timer/timer.h`。如果用户没有先定义 `TIMER_POOL_SIZE`，`EVENT_TIMER_POOL_SIZE` 会转发为内部 timer 池容量。

| 配置 | 作用 |
| --- | --- |
| `EVENT_QUEUE_SIZE` | 延时 post 槽容量；每个 `EventId + bit` 占用一个槽 |
| `EVENT_HANDLER_POOL_SIZE` | event handler 池容量，最大 4096 |
| `EVENT_MONITOR_POOL_SIZE` | monitor 池容量 |
| `EVENT_TIMER_POOL_SIZE` | event timer 池容量，并默认作为内部 `TIMER_POOL_SIZE` |

## 接口总览

| 类别 | 接口 | 功能 |
| --- | --- | --- |
| 类型 | `EventId` | 事件句柄 |
| 类型 | `EventMonitorId` | monitor 句柄 |
| 回调 | `event_handler_fn` | 处理事件 value 并返回新 value |
| 回调 | `event_monitor_fn` | 检查条件并返回要合并的 value |
| 初始化 | `event_scheduler_init` | 初始化事件调度器和内部 timer |
| 调度 | `event_scheduler_run_once` | 推进一次事件调度 |
| 调度 | `event_scheduler_next_delay` | 查询内部 timer 最近延迟 |
| 事件 | `event_new` | 创建事件并注册 handler |
| 事件 | `event_delete` | 删除事件及关联资源 |
| 事件 | `event_post` | 立即或延时合并 value |
| 事件 | `event_trigger` | 立即同步触发 handler |
| timer | `event_timer_add` | 周期合并 value |
| timer | `event_timer_remove` | 删除周期 value 任务 |
| monitor | `event_monitor_add` | 添加 monitor |
| monitor | `event_monitor_remove` | 删除 monitor |

## 类型

### `EventId`

```c
typedef uint32_t EventId;

#define EVENT_ID_INVALID UINT32_MAX
#define EVENT_ID_MAGIC 0x5AU
```

`EventId` 由 `event_new` 分配，内部编码为 `[magic:8][generation:12][index:12]`：

```
Bit 31-24  magic       固定为 EVENT_ID_MAGIC
Bit 23-12  generation  槽位释放时递增，用于识别旧 id
Bit 11-0   index       handler 池下标
```

公开 API 不接受用户手写的事件 id。`event_delete` 删除事件后会递增 generation，旧 `EventId` 不会匹配新分配的槽位。

### `event_handler_fn`

```c
typedef uint32_t (*event_handler_fn)(EventScheduler *scheduler, EventId id, uint32_t value, void *user_data);
```

handler 接收本次待处理的 value 位图和 `event_new` 注册的 `user_data`。返回值会写回事件槽：返回 `0U` 表示本轮处理完成；返回非 0 表示仍有 value 需要后续触发。

handler 执行期间可以调用 `event_post` 追加新的 value。调度器会把 handler 返回值与执行期间新增的 value 按位 OR 合并，再决定是否重新入队。

### `event_monitor_fn`

```c
typedef uint32_t (*event_monitor_fn)(EventScheduler *scheduler, EventId id);
```

monitor 不保存单独 `user_data`。返回 `0U` 表示无事件；返回非 0 表示把该 value 合并到绑定的 `EventId`。

### `EventScheduler`

`EventScheduler` 静态持有内部 `TimerScheduler`、handler 池、延时 post 池、monitor 池和 event timer 池。handler 使用 active 链表保存当前 value 非 0 的事件；monitor 和 event timer 也使用链表保存已分配节点，避免每次调度扫描整个池。

## 接口

### `event_scheduler_init`

```c
void event_scheduler_init(EventScheduler *self, const TimerOps *timer_ops, void *timer_user_data);
```

初始化事件调度器和内部 timer。`self`、`timer_ops`、`timer_ops->get_tick` 必须有效。

### `event_scheduler_run_once`

```c
void event_scheduler_run_once(EventScheduler *self);
```

非阻塞推进一次事件调度，顺序固定为：

1. 检查一个 `period_ticks == 0U` 的 monitor。
2. 推进一次内部 `timer_scheduler_run_once`。
3. 分发一个 active event。

该接口没有返回值。没有可处理事件时直接返回。

### `event_scheduler_next_delay`

```c
TimerTick event_scheduler_next_delay(EventScheduler *self);
```

返回内部 timer 距离最近到期任务的延迟；没有运行中的内部 timer 时返回 `(TimerTick)-1`。

### `event_new`

```c
EventId event_new(EventScheduler *self, event_handler_fn handler, void *user_data);
```

创建一个事件并注册 handler。`user_data` 是 handler 注册上下文，会作为 handler 第四个参数传回。池满时返回 `EVENT_ID_INVALID`。

### `event_delete`

```c
int event_delete(EventScheduler *self, EventId id);
```

删除事件，清理该 `EventId` 关联的延时 post、event timer、monitor 和 active value。成功返回 `0`，找不到事件返回 `-1`。

### `event_post`

```c
int event_post(EventScheduler *self, EventId event_id, TimerTick delay_ticks, uint32_t value);
```

`value == 0U` 是 no-op，返回 `0`。

`delay_ticks == 0U` 时，`value` 会按位 OR 到事件当前 value，并把事件放入 active 链表等待 `event_scheduler_run_once` 分发。

`delay_ticks != 0U` 时，调度器按 set bit 拆分 value。每个 `EventId + bit` 拥有独立内部 timer；同一 bit 重复 post 会重启该 bit timer，不影响其他 bit。bit 到期后只把该 bit OR 回事件 value，然后释放对应延时 post 槽和内部 timer。

### `event_trigger`

```c
int event_trigger(EventScheduler *self, EventId event_id, uint32_t value);
```

同步触发指定事件。`value == 0U` 是 no-op，返回 `0`；非零 value 会立即调用 handler，并按 handler 返回值更新事件 value，成功触发返回 `1`。

### `event_timer_add`

```c
TimerId event_timer_add(EventScheduler *self, EventId event_id, TimerTick period_ticks, uint32_t value);
```

添加周期 value 任务。`period_ticks` 必须非 0，`value == 0U` 返回 `TIMER_ID_INVALID`。内部 timer 到期时只把 value OR 到事件槽，不直接调用 handler。

### `event_timer_remove`

```c
int event_timer_remove(EventScheduler *self, TimerId id);
```

删除 event timer 并释放其内部 timer。成功返回 `0`，找不到返回 `-1`。

### `event_monitor_add`

```c
EventMonitorId event_monitor_add(EventScheduler *self, EventId event_id, event_monitor_fn monitor, TimerTick period_ticks);
```

添加 monitor。`period_ticks == 0U` 时 monitor 进入轮询链表，每次 `event_scheduler_run_once` 检查一个；非 0 时占用内部 timer，周期到期后检查一次。monitor 返回非 0 value 时只合并 value，不直接调用 handler。

### `event_monitor_remove`

```c
int event_monitor_remove(EventScheduler *self, EventMonitorId id);
```

删除 monitor，并在需要时释放内部 timer。成功返回 `0`，找不到返回 `-1`。

## 调度行为

- event 的触发完全依赖事件槽中的 value；0 表示不触发。
- `event_post` 使用按位 OR 合并 value，不覆盖旧 value。
- 延时 post 以 bit 为最小计时单位，最多表达 32 个独立触发位。
- `event_timer_add` 和 monitor 都只修改 value，handler 统一由 active 链表触发。
- `event_trigger` 是唯一同步触发入口。
- handler 返回值会写回事件槽，并与 handler 执行期间新增的 value 合并。
- 删除事件会清理所有关联资源，避免旧 timer 或旧 post 在 id 复用后重新生效。

## 示例

```c
#define BUTTON_DOWN 1U
#define BUTTON_UP   2U

static uint32_t button_handler(EventScheduler *scheduler, EventId id, uint32_t value, void *user_data)
{
    (void)user_data;

    if ((value & BUTTON_DOWN) != 0U)
    {
        event_post(scheduler, id, 20U, BUTTON_UP);
    }

    if ((value & BUTTON_UP) != 0U)
    {
        /* 处理释放 */
    }

    return 0U;
}

void example(EventScheduler *scheduler)
{
    EventId button_event;

    button_event = event_new(scheduler, button_handler, NULL);
    event_post(scheduler, button_event, 0U, BUTTON_DOWN);
    event_scheduler_run_once(scheduler);
}
```

## 检查

- `event.h` 不公开 `Event` 或 `EventSource`。
- `event_post_delay` 和 `event_is_posted` 不再存在。
- `event_new` 保留注册级 `user_data`，handler 调用时传回。
- 私有非 `_event_assert_*` 函数不包含断言。
- 公开接口入口覆盖参数校验。
- `event` 不包含 `slco.h`。
