#ifndef _EVENT_H_
#define _EVENT_H_

#include <stddef.h>
#include <stdint.h>

// 配置
#ifndef EVENT_QUEUE_SIZE
#define EVENT_QUEUE_SIZE 16U
#endif

#ifndef EVENT_HANDLER_POOL_SIZE
#define EVENT_HANDLER_POOL_SIZE 16U
#endif

#ifndef EVENT_MONITOR_POOL_SIZE
#define EVENT_MONITOR_POOL_SIZE 16U
#endif

#ifndef EVENT_TIMER_POOL_SIZE
#define EVENT_TIMER_POOL_SIZE 16U
#endif

#if EVENT_QUEUE_SIZE == 0U
#error "EVENT_QUEUE_SIZE must be greater than 0"
#endif

#if EVENT_HANDLER_POOL_SIZE == 0U
#error "EVENT_HANDLER_POOL_SIZE must be greater than 0"
#endif

#if EVENT_HANDLER_POOL_SIZE > 4096U
#error "EVENT_HANDLER_POOL_SIZE must be <= 4096"
#endif

#if EVENT_MONITOR_POOL_SIZE == 0U
#error "EVENT_MONITOR_POOL_SIZE must be greater than 0"
#endif

#if EVENT_TIMER_POOL_SIZE == 0U
#error "EVENT_TIMER_POOL_SIZE must be greater than 0"
#endif

#ifndef TIMER_POOL_SIZE
#define TIMER_POOL_SIZE EVENT_TIMER_POOL_SIZE
#endif

// 依赖
#include "../timer/timer.h"

#define EVENT_ID_INVALID UINT32_MAX
#define EVENT_MONITOR_ID_INVALID UINT32_MAX
#define EVENT_ID_MAGIC 0x5AU

#if (EVENT_ID_MAGIC & 0xFFU) == 0xFFU
#error "EVENT_ID_MAGIC must not be 0xFF"
#endif

// 类型定义
typedef uint32_t EventId;
typedef uint32_t EventMonitorId;

typedef struct event_scheduler_t EventScheduler;

typedef uint32_t (*event_handler_fn)(EventScheduler *scheduler, EventId id, uint32_t value, void *user_data);
typedef uint32_t (*event_monitor_fn)(EventScheduler *scheduler, EventId id);

typedef struct event_handler_t
{
    union
    {
        EventId id;
        struct
        {
            uint32_t _index : 12;
            uint32_t generation : 12;
            uint32_t _magic : 8;
        };
    };
    event_handler_fn handler;
    void *user_data;
    uint32_t value;
    uint8_t active;
    struct event_handler_t *next;
} EventHandler;

typedef struct event_post_t
{
    EventScheduler *scheduler;
    EventId event_id;
    TimerId timer_id;
    uint32_t bit;
} EventPost;

typedef struct event_monitor_t
{
    EventScheduler *scheduler;
    EventMonitorId id;
    EventId event_id;
    event_monitor_fn monitor;
    TimerId timer_id;
    TimerTick period_ticks;
    struct event_monitor_t *next;
    struct event_monitor_t *poll_next;
} EventMonitor;

typedef struct event_timer_t
{
    EventScheduler *scheduler;
    EventId event_id;
    TimerId timer_id;
    uint32_t value;
    struct event_timer_t *next;
} EventTimer;

struct event_scheduler_t
{
    TimerScheduler timer;
    EventPost posts[EVENT_QUEUE_SIZE];
    EventHandler handlers[EVENT_HANDLER_POOL_SIZE];
    EventMonitor monitors[EVENT_MONITOR_POOL_SIZE];
    EventTimer event_timers[EVENT_TIMER_POOL_SIZE];
    EventHandler *active_head;
    EventHandler *active_tail;
    EventMonitor *monitor_head;
    EventMonitor *poll_monitor_head;
    EventMonitor *poll_monitor_cursor;
    EventTimer *event_timer_head;
    EventMonitorId next_monitor_id;
};

// 接口
void event_scheduler_init(EventScheduler *self, const TimerOps *timer_ops, void *timer_user_data);
void event_scheduler_run_once(EventScheduler *self);
TimerTick event_scheduler_next_delay(EventScheduler *self);

EventId event_new(EventScheduler *self, event_handler_fn handler, void *user_data);
int event_delete(EventScheduler *self, EventId id);

int event_post(EventScheduler *self, EventId event_id, TimerTick delay_ticks, uint32_t value);
int event_trigger(EventScheduler *self, EventId event_id, uint32_t value);

TimerId event_timer_add(EventScheduler *self, EventId event_id, TimerTick period_ticks, uint32_t value);
int event_timer_remove(EventScheduler *self, TimerId id);

EventMonitorId event_monitor_add(EventScheduler *self, EventId event_id, event_monitor_fn monitor, TimerTick period_ticks);
int event_monitor_remove(EventScheduler *self, EventMonitorId id);
#endif
