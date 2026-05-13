#include "event.h"

static const TimerTick _event_tick_half_range = (TimerTick)(UINT32_MAX / 2U + 1U);

static inline EventId _event_id_encode(uint16_t generation, uint16_t index)
{
    return ((EventId)EVENT_ID_MAGIC << 24) | ((EventId)(generation & 0xFFFU) << 12) | (EventId)(index & 0xFFFU);
}

static inline uint8_t _event_id_magic(EventId id)
{
    return (uint8_t)(id >> 24);
}

static inline uint16_t _event_id_index(EventId id)
{
    return (uint16_t)(id & 0xFFFU);
}

static inline int _event_handler_is_free(const EventHandler *handler)
{
    return handler->_magic != EVENT_ID_MAGIC;
}

static EventHandler *_event_find_handler(EventScheduler *self, EventId id)
{
    uint16_t index;
    EventHandler *handler;

    if (_event_id_magic(id) != EVENT_ID_MAGIC)
    {
        return NULL;
    }

    index = _event_id_index(id);
    if (index >= EVENT_HANDLER_POOL_SIZE)
    {
        return NULL;
    }

    handler = &self->handlers[index];
    if (handler->id != id)
    {
        return NULL;
    }

    return handler;
}

static void _event_assert_timer_ops(const TimerOps *ops)
{
    WD_ASSERT(ops != NULL);
    WD_ASSERT(ops->get_tick != NULL);
}

static void _event_assert_ready(EventScheduler *self)
{
    WD_ASSERT(self != NULL);
    _event_assert_timer_ops(self->timer.ops);
}

static void _event_assert_event_id(EventId id)
{
    WD_ASSERT(_event_id_magic(id) == EVENT_ID_MAGIC);
}

static void _event_assert_event_exists(EventScheduler *self, EventId id)
{
    WD_ASSERT(_event_find_handler(self, id) != NULL);
}

static void _event_assert_delay(TimerTick delay_ticks)
{
    WD_ASSERT(delay_ticks < _event_tick_half_range);
}

static void _event_assert_period(TimerTick period_ticks)
{
    WD_ASSERT(period_ticks != 0U);
    WD_ASSERT(period_ticks < _event_tick_half_range);
}

static void _event_assert_monitor_id(EventMonitorId id)
{
    WD_ASSERT(id != EVENT_MONITOR_ID_INVALID);
}

static void _event_assert_timer_id(TimerId id)
{
    WD_ASSERT(id != TIMER_ID_INVALID);
}

static EventMonitorId _event_next_monitor_id(EventScheduler *self)
{
    EventMonitorId id;

    id = self->next_monitor_id;
    if (id == 0U || id == EVENT_MONITOR_ID_INVALID)
    {
        id = 1U;
    }

    self->next_monitor_id = id + 1U;
    if (self->next_monitor_id == 0U || self->next_monitor_id == EVENT_MONITOR_ID_INVALID)
    {
        self->next_monitor_id = 1U;
    }

    return id;
}

static void _event_active_push(EventScheduler *self, EventHandler *handler)
{
    if (handler->value != 0U && handler->active == 0U)
    {
        // value 从空变为非空时进入触发链表，避免调度器扫描整个 handler 池。
        handler->active = 1U;
        handler->next = NULL;
        if (self->active_tail == NULL)
        {
            self->active_head = handler;
            self->active_tail = handler;
        }
        else
        {
            self->active_tail->next = handler;
            self->active_tail = handler;
        }
    }
}

static void _event_active_remove(EventScheduler *self, EventHandler *handler)
{
    EventHandler *previous;
    EventHandler *current;

    previous = NULL;
    current = self->active_head;
    while (current != NULL)
    {
        if (current == handler)
        {
            // 从单链表中摘除当前事件，handler 执行期间产生的新 value 会重新入队。
            if (previous == NULL)
            {
                self->active_head = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            if (self->active_tail == current)
            {
                self->active_tail = previous;
            }

            current->active = 0U;
            current->next = NULL;
            return;
        }
        previous = current;
        current = current->next;
    }
}

static void _event_post_value(EventScheduler *self, EventId id, uint32_t value)
{
    EventHandler *handler;

    if (value == 0U)
    {
        return;
    }

    handler = _event_find_handler(self, id);
    if (handler != NULL)
    {
        // 所有异步来源只合并 value，真正触发统一交给 active 链表调度。
        handler->value |= value;
        _event_active_push(self, handler);
    }
}

static void _event_dispatch(EventScheduler *self, EventHandler *handler, uint32_t value)
{
    EventId id;
    EventHandler *current;

    handler->value |= value;
    _event_active_remove(self, handler);

    id = handler->id;
    value = handler->value;
    handler->value = 0U;

    // handler 的返回值描述处理后仍需保留的 value。
    value = handler->handler(self, id, value, handler->user_data);

    current = _event_find_handler(self, id);
    if (current != NULL)
    {
        current->value |= value;
        _event_active_push(self, current);
    }
}

static void _event_clear_post(EventPost *post)
{
    post->event_id = EVENT_ID_INVALID;
    post->timer_id = TIMER_ID_INVALID;
    post->bit = 0U;
}

static void _event_post_timer_action(TimerScheduler *timer_scheduler, TimerId id, void *user_data)
{
    EventPost *post;
    EventScheduler *scheduler;
    EventId event_id;
    uint32_t bit;

    (void)timer_scheduler;

    post = (EventPost *)user_data;
    scheduler = post->scheduler;
    event_id = post->event_id;
    bit = post->bit;

    // 延时 bit 到期后释放内部 timer，再把 bit 合并到事件 value。
    (void)timer_delete(&scheduler->timer, id);
    _event_clear_post(post);
    _event_post_value(scheduler, event_id, bit);
}

static EventPost *_event_find_post(EventScheduler *self, EventId event_id, uint32_t bit)
{
    size_t i;

    for (i = 0U; i < EVENT_QUEUE_SIZE; ++i)
    {
        if (self->posts[i].event_id == event_id && self->posts[i].bit == bit)
        {
            return &self->posts[i];
        }
    }

    return NULL;
}

static EventPost *_event_find_free_post(EventScheduler *self)
{
    size_t i;

    for (i = 0U; i < EVENT_QUEUE_SIZE; ++i)
    {
        if (self->posts[i].timer_id == TIMER_ID_INVALID)
        {
            return &self->posts[i];
        }
    }

    return NULL;
}

static int _event_start_post_timer(EventScheduler *self, EventPost *post, EventId event_id, TimerTick delay_ticks, uint32_t bit)
{
    TimerId timer_id;
    int result;

    result = -1;

    if (post->timer_id == TIMER_ID_INVALID)
    {
        timer_id = timer_new(&self->timer);
        if (timer_id == TIMER_ID_INVALID)
        {
            return -1;
        }
        post->timer_id = timer_id;
    }

    post->scheduler = self;
    post->event_id = event_id;
    post->bit = bit;

    // 相同 EventId+bit 重复延时 post 时复用并重启原 timer。
    if (timer_start(&self->timer, post->timer_id, delay_ticks, 0U, _event_post_timer_action, post) == 0)
    {
        result = 0;
    }

    if (result != 0)
    {
        (void)timer_delete(&self->timer, post->timer_id);
        _event_clear_post(post);
    }

    return result;
}

static int _event_post_delay_bits(EventScheduler *self, EventId event_id, TimerTick delay_ticks, uint32_t value)
{
    EventPost *post;
    uint32_t bit;
    size_t i;
    int result;

    result = 0;

    for (i = 0U; i < 32U; ++i)
    {
        bit = (uint32_t)1U << i;
        if ((value & bit) != 0U)
        {
            post = _event_find_post(self, event_id, bit);
            if (post == NULL)
            {
                post = _event_find_free_post(self);
            }

            if (post == NULL || _event_start_post_timer(self, post, event_id, delay_ticks, bit) != 0)
            {
                result = -1;
            }
        }
    }

    return result;
}

static EventHandler *_event_find_free_handler(EventScheduler *self)
{
    size_t i;

    for (i = 0U; i < EVENT_HANDLER_POOL_SIZE; ++i)
    {
        if (_event_handler_is_free(&self->handlers[i]))
        {
            return &self->handlers[i];
        }
    }

    return NULL;
}

static void _event_clear_handler(EventHandler *handler)
{
    handler->_magic = 0xFFU;
    handler->handler = NULL;
    handler->user_data = NULL;
    handler->value = 0U;
    handler->active = 0U;
    handler->next = NULL;
}

static EventMonitor *_event_find_free_monitor(EventScheduler *self)
{
    size_t i;

    for (i = 0U; i < EVENT_MONITOR_POOL_SIZE; ++i)
    {
        if (self->monitors[i].id == EVENT_MONITOR_ID_INVALID)
        {
            return &self->monitors[i];
        }
    }

    return NULL;
}

static EventMonitor *_event_find_monitor(EventScheduler *self, EventMonitorId id)
{
    EventMonitor *current;

    current = self->monitor_head;
    while (current != NULL)
    {
        if (current->id == id)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

static void _event_insert_monitor(EventScheduler *self, EventMonitor *monitor)
{
    monitor->next = self->monitor_head;
    self->monitor_head = monitor;

    if (monitor->period_ticks == 0U)
    {
        // 0 周期 monitor 进入轮询链表，run_once 每次只检查一个节点。
        monitor->poll_next = self->poll_monitor_head;
        self->poll_monitor_head = monitor;
        if (self->poll_monitor_cursor == NULL)
        {
            self->poll_monitor_cursor = monitor;
        }
    }
}

static void _event_unlink_monitor(EventScheduler *self, EventMonitor *monitor)
{
    EventMonitor *previous;
    EventMonitor *current;
    EventMonitor *poll_next;

    previous = NULL;
    current = self->monitor_head;
    while (current != NULL)
    {
        if (current == monitor)
        {
            if (previous == NULL)
            {
                self->monitor_head = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            break;
        }
        previous = current;
        current = current->next;
    }

    poll_next = monitor->poll_next;
    previous = NULL;
    current = self->poll_monitor_head;
    while (current != NULL)
    {
        if (current == monitor)
        {
            // poll cursor 指向被删节点时前移到下一个可轮询 monitor。
            if (previous == NULL)
            {
                self->poll_monitor_head = current->poll_next;
            }
            else
            {
                previous->poll_next = current->poll_next;
            }
            break;
        }
        previous = current;
        current = current->poll_next;
    }

    if (self->poll_monitor_cursor == monitor)
    {
        self->poll_monitor_cursor = poll_next;
        if (self->poll_monitor_cursor == NULL)
        {
            self->poll_monitor_cursor = self->poll_monitor_head;
        }
    }

    monitor->next = NULL;
    monitor->poll_next = NULL;
}

static void _event_clear_monitor(EventMonitor *monitor)
{
    monitor->scheduler = NULL;
    monitor->id = EVENT_MONITOR_ID_INVALID;
    monitor->event_id = EVENT_ID_INVALID;
    monitor->monitor = NULL;
    monitor->timer_id = TIMER_ID_INVALID;
    monitor->period_ticks = 0U;
    monitor->next = NULL;
    monitor->poll_next = NULL;
}

static void _event_release_monitor(EventScheduler *self, EventMonitor *monitor)
{
    if (monitor->timer_id != TIMER_ID_INVALID)
    {
        // 删除周期 monitor 时同步释放它占用的内部 timer。
        (void)timer_delete(&self->timer, monitor->timer_id);
    }

    _event_unlink_monitor(self, monitor);
    _event_clear_monitor(monitor);
}

static void _event_monitor_timer_action(TimerScheduler *timer_scheduler, TimerId id, void *user_data)
{
    EventMonitor *monitor;
    uint32_t value;

    (void)timer_scheduler;
    (void)id;

    monitor = (EventMonitor *)user_data;
    value = monitor->monitor(monitor->scheduler, monitor->event_id);
    if (value != 0U)
    {
        // 周期 monitor 只修改事件 value，不直接调用 handler。
        _event_post_value(monitor->scheduler, monitor->event_id, value);
    }
}

static void _event_poll_monitor(EventScheduler *self)
{
    EventMonitor *monitor;
    EventMonitor *next;
    uint32_t value;

    monitor = self->poll_monitor_cursor;
    if (monitor == NULL)
    {
        monitor = self->poll_monitor_head;
    }

    if (monitor != NULL)
    {
        next = monitor->poll_next;
        if (next == NULL)
        {
            next = self->poll_monitor_head;
        }
        self->poll_monitor_cursor = next;

        value = monitor->monitor(self, monitor->event_id);
        if (value != 0U)
        {
            _event_post_value(self, monitor->event_id, value);
        }
    }
}

static EventTimer *_event_find_free_timer(EventScheduler *self)
{
    size_t i;

    for (i = 0U; i < EVENT_TIMER_POOL_SIZE; ++i)
    {
        if (self->event_timers[i].timer_id == TIMER_ID_INVALID)
        {
            return &self->event_timers[i];
        }
    }

    return NULL;
}

static EventTimer *_event_find_timer(EventScheduler *self, TimerId id)
{
    EventTimer *current;

    current = self->event_timer_head;
    while (current != NULL)
    {
        if (current->timer_id == id)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

static void _event_insert_timer(EventScheduler *self, EventTimer *event_timer)
{
    event_timer->next = self->event_timer_head;
    self->event_timer_head = event_timer;
}

static void _event_unlink_timer(EventScheduler *self, EventTimer *event_timer)
{
    EventTimer *previous;
    EventTimer *current;

    previous = NULL;
    current = self->event_timer_head;
    while (current != NULL)
    {
        if (current == event_timer)
        {
            // event timer 链表只保存已分配节点，remove 不再扫描空槽。
            if (previous == NULL)
            {
                self->event_timer_head = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            break;
        }
        previous = current;
        current = current->next;
    }

    event_timer->next = NULL;
}

static void _event_clear_timer(EventTimer *event_timer)
{
    event_timer->scheduler = NULL;
    event_timer->event_id = EVENT_ID_INVALID;
    event_timer->timer_id = TIMER_ID_INVALID;
    event_timer->value = 0U;
    event_timer->next = NULL;
}

static void _event_release_timer(EventScheduler *self, EventTimer *event_timer)
{
    if (event_timer->timer_id != TIMER_ID_INVALID)
    {
        // 周期事件 timer 移除时释放内部 timer，避免旧 value 后续复活。
        (void)timer_delete(&self->timer, event_timer->timer_id);
    }

    _event_unlink_timer(self, event_timer);
    _event_clear_timer(event_timer);
}

static void _event_timer_action(TimerScheduler *timer_scheduler, TimerId id, void *user_data)
{
    EventTimer *event_timer;

    (void)timer_scheduler;
    (void)id;

    event_timer = (EventTimer *)user_data;
    // event timer 只合并 value，handler 由 event_scheduler_run_once 统一触发。
    _event_post_value(event_timer->scheduler, event_timer->event_id, event_timer->value);
}

static void _event_delete_posts(EventScheduler *self, EventId id)
{
    size_t i;

    for (i = 0U; i < EVENT_QUEUE_SIZE; ++i)
    {
        if (self->posts[i].event_id == id)
        {
            // 删除事件时清掉所有尚未到期的 bit 延时 post。
            (void)timer_delete(&self->timer, self->posts[i].timer_id);
            _event_clear_post(&self->posts[i]);
        }
    }
}

static void _event_delete_monitors(EventScheduler *self, EventId id)
{
    EventMonitor *current;
    EventMonitor *next;

    current = self->monitor_head;
    while (current != NULL)
    {
        next = current->next;
        if (current->event_id == id)
        {
            _event_release_monitor(self, current);
        }
        current = next;
    }
}

static void _event_delete_timers(EventScheduler *self, EventId id)
{
    EventTimer *current;
    EventTimer *next;

    current = self->event_timer_head;
    while (current != NULL)
    {
        next = current->next;
        if (current->event_id == id)
        {
            _event_release_timer(self, current);
        }
        current = next;
    }
}

void event_scheduler_init(EventScheduler *self, const TimerOps *timer_ops, void *timer_user_data)
{
    size_t i;

    WD_ASSERT(self != NULL);
    _event_assert_timer_ops(timer_ops);

    timer_scheduler_init(&self->timer, timer_ops, timer_user_data);

    self->active_head = NULL;
    self->active_tail = NULL;
    self->monitor_head = NULL;
    self->poll_monitor_head = NULL;
    self->poll_monitor_cursor = NULL;
    self->event_timer_head = NULL;
    self->next_monitor_id = 1U;

    for (i = 0U; i < EVENT_HANDLER_POOL_SIZE; ++i)
    {
        self->handlers[i].id = 0U;
        self->handlers[i].handler = NULL;
        self->handlers[i].user_data = NULL;
        self->handlers[i].value = 0U;
        self->handlers[i].active = 0U;
        self->handlers[i].next = NULL;
    }

    for (i = 0U; i < EVENT_QUEUE_SIZE; ++i)
    {
        self->posts[i].scheduler = self;
        _event_clear_post(&self->posts[i]);
    }

    for (i = 0U; i < EVENT_MONITOR_POOL_SIZE; ++i)
    {
        _event_clear_monitor(&self->monitors[i]);
    }

    for (i = 0U; i < EVENT_TIMER_POOL_SIZE; ++i)
    {
        _event_clear_timer(&self->event_timers[i]);
    }
}

void event_scheduler_run_once(EventScheduler *self)
{
    EventHandler *handler;

    _event_assert_ready(self);

    _event_poll_monitor(self);
    timer_scheduler_run_once(&self->timer);

    handler = self->active_head;
    if (handler != NULL)
    {
        _event_dispatch(self, handler, 0U);
    }
}

TimerTick event_scheduler_next_delay(EventScheduler *self)
{
    _event_assert_ready(self);

    return timer_scheduler_next_delay(&self->timer);
}

EventId event_new(EventScheduler *self, event_handler_fn handler, void *user_data)
{
    EventHandler *slot;
    EventId id;
    uint16_t index;

    _event_assert_ready(self);
    WD_ASSERT(handler != NULL);

    slot = _event_find_free_handler(self);
    if (slot == NULL)
    {
        return EVENT_ID_INVALID;
    }

    index = (uint16_t)(slot - self->handlers);
    id = _event_id_encode((uint16_t)slot->generation, index);
    slot->id = id;
    slot->handler = handler;
    slot->user_data = user_data;
    slot->value = 0U;
    slot->active = 0U;
    slot->next = NULL;

    return id;
}

int event_delete(EventScheduler *self, EventId id)
{
    EventHandler *handler;
    int result;

    _event_assert_ready(self);
    _event_assert_event_id(id);

    result = -1;
    handler = _event_find_handler(self, id);
    if (handler != NULL)
    {
        // 先清理所有外部触发源，再释放事件槽位，防止旧 value 复活。
        _event_delete_posts(self, id);
        _event_delete_timers(self, id);
        _event_delete_monitors(self, id);
        _event_active_remove(self, handler);
        handler->generation++;
        _event_clear_handler(handler);
        result = 0;
    }

    return result;
}

int event_post(EventScheduler *self, EventId event_id, TimerTick delay_ticks, uint32_t value)
{
    int result;

    _event_assert_ready(self);
    _event_assert_event_exists(self, event_id);
    _event_assert_delay(delay_ticks);

    result = 0;
    if (value != 0U)
    {
        if (delay_ticks == 0U)
        {
            _event_post_value(self, event_id, value);
        }
        else
        {
            result = _event_post_delay_bits(self, event_id, delay_ticks, value);
        }
    }

    return result;
}

int event_trigger(EventScheduler *self, EventId event_id, uint32_t value)
{
    EventHandler *handler;
    int result;

    _event_assert_ready(self);
    _event_assert_event_exists(self, event_id);

    result = 0;
    if (value != 0U)
    {
        // trigger 是唯一同步触发入口，会立即执行对应 handler。
        handler = _event_find_handler(self, event_id);
        _event_dispatch(self, handler, value);
        result = 1;
    }

    return result;
}

TimerId event_timer_add(EventScheduler *self, EventId event_id, TimerTick period_ticks, uint32_t value)
{
    EventTimer *event_timer;
    TimerId timer_id;

    _event_assert_ready(self);
    _event_assert_event_exists(self, event_id);
    _event_assert_period(period_ticks);

    if (value == 0U)
    {
        return TIMER_ID_INVALID;
    }

    event_timer = _event_find_free_timer(self);
    if (event_timer == NULL)
    {
        return TIMER_ID_INVALID;
    }

    timer_id = timer_new(&self->timer);
    if (timer_id == TIMER_ID_INVALID)
    {
        return TIMER_ID_INVALID;
    }

    event_timer->scheduler = self;
    event_timer->event_id = event_id;
    event_timer->timer_id = timer_id;
    event_timer->value = value;
    event_timer->next = NULL;

    if (timer_start(&self->timer, timer_id, period_ticks, period_ticks, _event_timer_action, event_timer) != 0)
    {
        (void)timer_delete(&self->timer, timer_id);
        _event_clear_timer(event_timer);
        return TIMER_ID_INVALID;
    }

    _event_insert_timer(self, event_timer);

    return timer_id;
}

int event_timer_remove(EventScheduler *self, TimerId id)
{
    EventTimer *event_timer;
    int result;

    _event_assert_ready(self);
    _event_assert_timer_id(id);

    result = -1;
    event_timer = _event_find_timer(self, id);
    if (event_timer != NULL)
    {
        _event_release_timer(self, event_timer);
        result = 0;
    }

    return result;
}

EventMonitorId event_monitor_add(EventScheduler *self, EventId event_id, event_monitor_fn monitor, TimerTick period_ticks)
{
    EventMonitor *slot;
    TimerId timer_id;
    EventMonitorId id;

    _event_assert_ready(self);
    _event_assert_event_exists(self, event_id);
    WD_ASSERT(monitor != NULL);
    WD_ASSERT(period_ticks == 0U || period_ticks < _event_tick_half_range);

    slot = _event_find_free_monitor(self);
    if (slot == NULL)
    {
        return EVENT_MONITOR_ID_INVALID;
    }

    timer_id = TIMER_ID_INVALID;
    if (period_ticks != 0U)
    {
        timer_id = timer_new(&self->timer);
        if (timer_id == TIMER_ID_INVALID)
        {
            return EVENT_MONITOR_ID_INVALID;
        }
    }

    id = _event_next_monitor_id(self);
    slot->scheduler = self;
    slot->id = id;
    slot->event_id = event_id;
    slot->monitor = monitor;
    slot->timer_id = timer_id;
    slot->period_ticks = period_ticks;
    slot->next = NULL;
    slot->poll_next = NULL;

    if (period_ticks != 0U &&
        timer_start(&self->timer, timer_id, period_ticks, period_ticks, _event_monitor_timer_action, slot) != 0)
    {
        (void)timer_delete(&self->timer, timer_id);
        _event_clear_monitor(slot);
        return EVENT_MONITOR_ID_INVALID;
    }

    _event_insert_monitor(self, slot);

    return id;
}

int event_monitor_remove(EventScheduler *self, EventMonitorId id)
{
    EventMonitor *monitor;
    int result;

    _event_assert_ready(self);
    _event_assert_monitor_id(id);

    result = -1;
    monitor = _event_find_monitor(self, id);
    if (monitor != NULL)
    {
        _event_release_monitor(self, monitor);
        result = 0;
    }

    return result;
}
