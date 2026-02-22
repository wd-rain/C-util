#ifndef MACHINE_H
#define MACHINE_H

#include <stdint.h>

typedef struct machine machine_t;
typedef void (*StateFunction_t)(machine_t *machine);

struct machine
{
    StateFunction_t currentState;
    void (*reset)(machine_t *machine);
};

/// @brief 创建状态机
/// @param name 状态机名字
/// @param data 状态机参数
#define Machine(name)                                                            \
    void name##_reset(machine_t *self);                                          \
    machine_t name = {NULL, name##_reset};                                       \
    __attribute__((constructor)) void name##_init(void) { machineReset(&name); } \
    void name##_reset(machine_t *self)

/// @brief 创建状态
/// @param name 状态名字
#define State(name) void name(machine_t *self)

/// @brief 状态定义
/// @param name 状态名字
#define stateDefine(name) void name(machine_t *self)

/// @brief 获取状态机数据
/// @param type 数据类型
#define machineData(name, type) type *name = ((type *)self->data)

/// @brief 复位状态机
/// @param name 状态机句柄
void machineReset(machine_t *machine);

/// @brief 状态机切换状态
/// @param name 状态机句柄
void machineNext(machine_t *machine, StateFunction_t funcName);

/// @brief 状态机运行
/// @param name 状态机句柄
void machineRun(machine_t *machine);

#endif // MACHINE_H
