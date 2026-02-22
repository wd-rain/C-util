#include "timer.h"

static tick_size_t __self_timeline_tick = 0;
static PlatformTicksFunction_t __get_tick = Timer_Default_GetTick;

/// @brief 默认tick触发
void Timer_Tick_Trigger(void)
{
    ++__self_timeline_tick;
}

/// @brief 加载tick获取函数
/// @param func tick获取函数
void LoadTimerGetTick(PlatformTicksFunction_t func)
{
    __get_tick = func;
}

/// @brief timer默认获取tick函数
/// @return 当前tick
tick_size_t Timer_Default_GetTick(void)
{
    return __self_timeline_tick;
}

void _timeline_load_task(timeline_t *t, ...)
{
    va_list args;
    va_start(args, t);
    timeline_task_t *task = (timeline_task_t *)va_arg(args, timeline_task_t *);
    t->task_head = task;
    while (task)
    {
        timeline_task_t *next = (timeline_task_t *)va_arg(args, timeline_task_t *);
        task->next = next;
        task = next;
    }
}

/// @brief 时间线运行
/// @param t 时间线句柄
void Timeline_Yield(timeline_t *t)
{
    uint32_t tick = __get_tick();
    timeline_task_t *task = t->task_head;
    while (task)
    {
        if (tick >= task->start)
        {
            task->start = tick + t->cycle;
            Timer_Start(task->timer, 0);
        }
        if (tick >= task->end)
        {
            if (task->timer->interval)
            {
                task->end = tick + t->cycle;
                Timer_Stop(task->timer);
            }
        }
        task = task->next;
    }
}

// Timer
static timer_t *timerList = NULL;

/// @brief 开启定时器任务
/// @param timer 任务句柄
/// @param timing 开始时间
void Timer_Start(timer_t *timer, tick_size_t timing)
{
    timer->deadline = __get_tick() + timing;

    timer_t **current = &timerList;
    while (*current && ((*current)->deadline < timer->deadline)) // 排序-减少循环开销
    {
        current = &(*current)->next;
    }
    timer->next = *current;
    *current = timer;
}

/// @brief 关闭定时器任务
/// @param timer 任务句柄
void Timer_Stop(timer_t *timer)
{
    timer_t **current = &timerList;
    while (*current)
    {
        if (*current == timer)
        {
            *current = timer->next;
            break;
        }
        current = &(*current)->next;
    }
}

/// @brief 定时器运行
void Timer_Yield(void)
{
    tick_size_t currentTicks = __get_tick();
    while (timerList && (currentTicks >= timerList->deadline))
    {
        timer_t *timer = timerList;
        timerList = timer->next;
        if (timer->interval)
        {
            Timer_Start(timer, timer->interval);
        }
        timer->func();
    }
}
