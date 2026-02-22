#include "machine.h"

/// @brief 复位状态机
/// @param name 状态机句柄
void machineReset(machine_t *machine)
{
    if (machine->reset)
    {
        machine->reset(machine);
    }
}

/// @brief 状态机切换状态
/// @param name 状态机句柄
void machineNext(machine_t *machine, StateFunction_t funcName)
{
    machine->currentState = funcName;
}

/// @brief 状态机运行
/// @param name 状态机句柄
void machineRun(machine_t *machine)
{
    if (machine->currentState)
    {
        machine->currentState(machine);
    }
}


