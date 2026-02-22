#ifndef TIMER_H
#define TIMER_H

#include "stdint.h"
#include "stdbool.h"
#include "stdarg.h"
#include "stdlib.h"

#define TICK_SIZE_BITS 32

#if TICK_SIZE_BITS == 32
typedef uint32_t tick_size_t;
#elif TICK_SIZE_BITS == 64
typedef uint64_t tick_size_t;
#else
#error "Unsupported tick size"
#endif

typedef tick_size_t (*PlatformTicksFunction_t)(void);

typedef struct timer
{
    struct timer *next;
    tick_size_t deadline;
    tick_size_t interval;
    void (*func)(void);
} timer_t;

typedef struct timeline_task
{
    timer_t *timer;
    tick_size_t start;
    tick_size_t end;
    struct timeline_task *next;
} timeline_task_t;

typedef struct timeline
{
    tick_size_t cycle;
    timeline_task_t *task_head;
} timeline_t;

void _timeline_load_task(timeline_t *t, ...);

// API
void Timer_Tick_Trigger(void);
tick_size_t Timer_Default_GetTick(void);
void LoadTimerGetTick(PlatformTicksFunction_t func);

// 创建超时任务
#define Timeout(name)                            \
    void name##Function(void);                   \
    timer_t name = {NULL, 0, 0, name##Function}; \
    void name##Function(void)
// 创建间隔任务
#define Interval(name, interval)                        \
    void name##Function(void);                          \
    timer_t name = {NULL, 0, interval, name##Function}; \
    void name##Function(void)
void Timer_Start(timer_t *timer, tick_size_t timing);
void Timer_Stop(timer_t *timer);
void Timer_Yield(void);
void Timeline_Yield(timeline_t *t);
/// @brief 创建时间线
/// @param cycle 时间线周期
#define Timeline(name, cycle)        \
    timeline_t name = {cycle, NULL}; \
    Interval(name##Interval, 1) { Timeline_Yield(&name); }

/// @brief 为时间线加载任务
/// @param t 时间线句柄
#define TimelineStart(t, ...)                  \
    _timeline_load_task(t, __VA_ARGS__, NULL); \
    Timer_Start(t##Interval, 0)

/// @brief 创建时间线-时刻任务
/// @param time 时间
#define TimeTask(name, time)                                       \
    void name##Function(void);                                     \
    timer_t name##TimelineTimer = {NULL, time, 0, name##Function}; \
    timeline_task_t name = {&name##TimelineTimer, time, 0, NULL};  \
    void name##Function(void)

/// @brief 创建时间线-窗口任务
/// @param name 任务名字
/// @param start 开始时间
/// @param end 结束时间
/// @param interval 执行周期
#define TimelineTask(name, start, end, interval)                           \
    void name##Function(void);                                             \
    timer_t name##TimelineTimer = {NULL, start, interval, name##Function}; \
    timeline_task_t name = {&name##TimelineTimer, start, end, NULL};       \
    void name##Function(void)

#endif /* TIMER_H */
