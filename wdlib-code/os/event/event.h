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
#define EVENT_TIMER_ID_INVALID UINT32_MAX

// 类型定义
typedef uint32_t EventId;
typedef uint32_t EventMonitorId;
typedef uint32_t EventTimerId;

typedef struct event_scheduler_t EventScheduler;

typedef enum event_source_t
{
    WD_EVENT_SOURCE_EXTERNAL = 0,
    WD_EVENT_SOURCE_TIMER,
    WD_EVENT_SOURCE_MONITOR
} EventSource;

typedef struct event_t
{
    EventId id;
    EventSource source;
    uint32_t value;
    void *user_data;
} Event;

typedef void (*event_handler_fn)(EventScheduler *scheduler, const Event *event, void *user_data);
typedef uint32_t (*event_monitor_fn)(EventScheduler *scheduler, void **event_user_data, void *user_data);

typedef struct event_handler_t
{
    EventId id;
    event_handler_fn handler;
    void *user_data;
} EventHandler;

typedef struct event_post_t
{
    EventScheduler *scheduler;
    EventId event_id;
    TimerId timer_id;
    EventSource source;
    uint8_t state;
    uint32_t value;
    void *user_data;
} EventPost;

typedef struct event_monitor_t
{
    EventScheduler *scheduler;
    EventMonitorId id;
    EventId event_id;
    event_monitor_fn monitor;
    TimerId timer_id;
    TimerTick period_ticks;
    void *user_data;
} EventMonitor;

typedef struct event_timer_t
{
    EventScheduler *scheduler;
    EventTimerId id;
    EventId event_id;
    TimerId timer_id;
    uint32_t value;
    void *user_data;
} EventTimer;

struct event_scheduler_t
{
    TimerScheduler timer;
    EventPost posts[EVENT_QUEUE_SIZE];
    EventHandler handlers[EVENT_HANDLER_POOL_SIZE];
    EventMonitor monitors[EVENT_MONITOR_POOL_SIZE];
    EventTimer event_timers[TIMER_POOL_SIZE];
    EventId next_event_id;
    EventMonitorId next_monitor_id;
    EventTimerId next_timer_id;
    size_t post_scan_index;
    size_t monitor_scan_index;
    int internal_error;
};

// 接口
void event_scheduler_init(EventScheduler *self, const TimerOps *timer_ops, void *timer_user_data);
int event_scheduler_run_once(EventScheduler *self);
TimerTick event_scheduler_next_delay(EventScheduler *self);

EventId event_new(EventScheduler *self, event_handler_fn handler, void *user_data);
int event_delete(EventScheduler *self, EventId id);

int event_post(EventScheduler *self, EventId event_id, TimerTick delay_ticks, uint32_t value);
uint32_t event_is_posted(EventScheduler *self, EventId event_id);
int event_trigger(EventScheduler *self, EventId event_id, uint32_t value, void *user_data);

EventTimerId event_timer_add(EventScheduler *self, EventId event_id, TimerTick period_ticks, uint32_t value);
int event_timer_remove(EventScheduler *self, EventTimerId id);

EventMonitorId event_monitor_add(EventScheduler *self, EventId event_id, event_monitor_fn monitor, TimerTick period_ticks);
int event_monitor_remove(EventScheduler *self, EventMonitorId id);
#endif
