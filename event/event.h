#ifndef EVENT_H_
#define EVENT_H_

#include <stdio.h>

typedef void (*func_t)(void *parm);

#define WEAK __attribute__((weak))

/// @brief 提取self指针
/// @param type 类名
#define Self(type) type *self = (type *)parm

/// @brief 创建事件
/// @param name 事件名
#define Event(name) func_t name = NULL

/// @brief 事件订阅
/// @param name 事件名
#define Hook(name)                                                            \
    static void EVENTLISTEVENTFUNC_##name(void *parm);                        \
    static void EVENTLISTEVENTINIT_##name(void) __attribute__((constructor)); \
    static void EVENTLISTEVENTINIT_##name(void)                               \
    {                                                                         \
        name = EVENTLISTEVENTFUNC_##name;                                     \
    }                                                                         \
    static void EVENTLISTEVENTFUNC_##name(void *parm)

/// @brief 事件槽调用
/// @param event 事件名
#define Slot(event, parm)        \
    do                           \
    {                            \
        if (event)               \
        {                        \
            event((void *)parm); \
        }                        \
    } while (0)

#define HookIT(name) inline void name(void *parm)

#define SlotIT(event, parm) event(parm)

#define EventIT(name) extern inline void name(void *parm)

#endif /* EVENT_H_ */
