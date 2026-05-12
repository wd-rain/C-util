#include "event.h"

void event_scheduler_init(EventScheduler *self, const TimerOps *timer_ops, void *timer_user_data)
{
    // 初始化
}

int event_scheduler_run_once(EventScheduler *self)
{
    // 检测监控链表 -> 或上对应的事件
    timer_scheduler_run_once(&self->timer);

    // 检测事件队列 -> 发布对应的事件
    // if(有事件)
    {
        // 对应事件value1 = 处理事件

    }
    
}

TimerTick event_scheduler_next_delay(EventScheduler *self)
{

}

EventId event_new(EventScheduler *self, event_handler_fn handler, void *user_data)
{

}

int event_delete(EventScheduler *self, EventId id)
{

}

int event_post(EventScheduler *self, EventId event_id, TimerTick delay_ticks, uint32_t value, void *user_data)
{

}

int event_trigger(EventScheduler *self, EventId event_id, uint32_t value, void *user_data)
{
    // 直接执行对应的事件
}

EventTimerId event_timer_add(EventScheduler *self, EventId event_id, TimerTick period_ticks, uint32_t value, void *user_data)
{
    EventTimerId id = timer_new(&self->timer);
    if(timer_is_running(&self->timer, id) != -1)
    {
        // timer_start(&self->timer, id, period_ticks, period_ticks, 发布事件通用函数, 事件本身)
    }
    return id;
}

int event_timer_remove(EventScheduler *self, EventTimerId id)
{

}

EventMonitorId event_monitor_add(EventScheduler *self, EventId event_id, event_monitor_fn monitor, TimerTick period_ticks, void *user_data)
{

}

int event_monitor_remove(EventScheduler *self, EventMonitorId id)
{
    _event_assert_ready(self);
    WD_ASSERT(id != EVENT_MONITOR_ID_INVALID);


}
