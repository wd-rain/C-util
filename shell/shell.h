#ifndef __LOVE_FISH_SHELL_H__
#define __LOVE_FISH_SHELL_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// Config
#define SHELL_PARAMETER_MAX_NUMBER 3 // 最大参数数量
#define SHELL_HISTORY_MAX_NUMBER 5   // 历史命令记录数量
#define SHELL_KEEP_RETURN_VALUE 0    // 是否保留命令返回值
#define SHELL_PRINT_BUFFER 128       // 打印缓存大小
#define SHELL_TASK_WHILE 0           // Task使用循环
// Define
#define KEYUPANSICODE 0x1B5B4100
#define KEYDOWNANSICODE 0x1B5B4200
#define KEYLEFTANSICODE 0x1B5B4400
#define KEYRIGHTANSICODE 0x1B5B4300

// Private
#if SHELL_PARAMETER_MAX_NUMBER < 0 || SHELL_PARAMETER_MAX_NUMBER > 16
#error "SHELL_PARAMETER_MAX_NUMBER must be between 0 and 15"
#endif

typedef struct shell_cmd_def shell_cmd_t;
typedef struct shell_def shell_t;

typedef enum
{
    NUM_TYPE_DEC,  /**< 十进制整型 */
    NUM_TYPE_BIN,  /**< 二进制整型 */
    NUM_TYPE_OCT,  /**< 八进制整型 */
    NUM_TYPE_HEX,  /**< 十六进制整型 */
    NUM_TYPE_FLOAT /**< 浮点型 */
} ShellNumType;

enum shell_cmd_func_type
{
    SHELL_TYPE_FUNC_MAIN,
    SHELL_TYPE_FUNC_FUNC_AUTO
};

enum shell_cmd_func_return_type
{
    SHELL_TYPE_FUNC_RETURN_NONE,
    SHELL_TYPE_FUNC_RETURN_INT,
    SHELL_TYPE_FUNC_RETURN_FLOAT,
    SHELL_TYPE_FUNC_RETURN_STRING,
};

enum shell_cmd_var_type
{
    SHELL_TYPE_VAR_AUTO,
    SHELL_TYPE_VAR_NUMBER,
    SHELL_TYPE_VAR_STRING,
};

enum shell_cmd_func_return_type_simple
{
    RETURN_NONE,
    RETURN_INT,
    RETURN_FLOAT,
    RETURN_STRING,
};

struct shell_cmd_def
{
    union
    {
        struct
        {
            const char *name;  /**< 命令名 */
            int (*function)(); /**< 命令执行函数 */
            const char *desc;  /**< 命令描述 */
        } func;
        struct
        {
            const char *name;            /**< 按键名 */
            int value;                   /**< 按键键值 */
            void (*function)(shell_t *); /**< 按键执行函数 */
            const char *desc;            /**< 按键描述 */
        } key;                           /**< 按键定义 */
        struct
        {
            const char *name; /**< 变量名 */
            void *value;      /**< 变量值 */
            const char *desc; /**< 变量描述 */
        } var;
        struct
        {
            const char *name;            /**< 变量名 */
            const char *password;        /**< 密码变量 */
            void (*function)(shell_t *); /**< 变量执行函数 */
            const char *desc;            /**< 变量描述 */
        } sys;

    } cmd;
    union
    {
        struct
        {
            struct
            {
                enum shell_cmd_type
                {
                    KEYCMD,
                    VARCMD,
                    FUNCMD,
                    SYSCMD,
                } main : 3;
                uint8_t returnDisplay : 2;
                uint8_t addition : 3;
            } type;
            uint8_t helpDisplay : 1;
            uint8_t permission : 3;
            uint8_t paramNum : 4;
        } attrs;
        uint16_t value;
    } attr;
    shell_cmd_t *next; // 指向下一个命令
};

struct shell_def
{
#if SHELL_KEEP_RETURN_VALUE
    int retVal; // 返回值
#endif

    uint32_t (*read)(uint8_t *, uint32_t);        // 读缓冲区
    uint32_t (*write)(const uint8_t *, uint32_t); // 写缓冲区

    uint8_t permission : 3;
    uint8_t keyListFlag : 1;
    uint8_t funcListFlag : 1;
    uint8_t varListFlag : 1;
    uint8_t sysListFlag : 1;

    shell_cmd_t *head;
    struct
    {
        uint8_t isChecked : 1; /**< 需要密码验证 */
        uint8_t tabFlag : 1;   /**< tab标志 */
    } status;
    struct
    {
        char *item[SHELL_HISTORY_MAX_NUMBER]; /**< 历史记录 */
        unsigned short number;                /**< 历史记录数 */
        unsigned short record;                /**< 当前记录位置 */
        signed short offset;                  /**< 当前历史记录偏移 */
    } history;
    struct
    {
        uint32_t length;                         /**< 输入数据长度 */
        uint32_t cursor;                         /**< 当前光标位置 */
        uint8_t *buffer;                         /**< 输入缓冲 */
        char *param[SHELL_PARAMETER_MAX_NUMBER]; /**< 参数 */
        uint32_t bufferSize;                     /**< 输入缓冲大小 */
        uint32_t paramCount;                     /**< 参数数量 */
        int keyValue;                            /**< 输入按键键值 */
    } parser;
    struct
    {
        uint8_t state;
        const char *password;
        void (*func)(shell_t *shell);
    } system;
};

extern shell_cmd_t shellEnter2;
#define SHELLSYSTEMHEAD &shellEnter2

// API

/// @brief 返回浮点型
/// @param x 浮点型数据
#define SHELL_RETURE_FLOAT(x)      \
    float reture_data_sdjahkh = x; \
    return *((int *)&reture_data_sdjahkh)

/// @brief 返回字符串
/// @param x 字符串首地址
#define SHELL_RETURE_STRING(x)     \
    char *reture_data_sdjahkh = x; \
    return (int)reture_data_sdjahkh

/// @brief 返回整型
/// @param x 整型数据
#define SHELL_RETURE_INT(x) \
    return x

/// @brief 执行命令
/// @param param 终端对象
void shellTask(void *param);

/// @brief 用于
/// @param shell
/// @param format
/// @param ...
void shellPrintf(shell_t *shell, const char *format, ...);


#if SHELL_HISTORY_MAX_NUMBER <= 0
/// @brief 注册终端
/// @param name 终端名称
/// @param read 读函数
/// @param write 写函数
/// @param size 缓冲区大小
#define Shell(Name, Read, Write, Size, Permission) \
    uint8_t Name##SHELL_buffer[Size] = {0};        \
    shell_t Name = {.read = Read, .write = Write, .parser.buffer = Name##SHELL_buffer, .parser.bufferSize = Size, .head = SHELLSYSTEMHEAD, .status.isChecked = 1, .permission = Permission}
#else

/// @brief 注册终端
/// @param name 终端名称
/// @param read 读函数
/// @param write 写函数
/// @param size 缓冲区大小
/// @param Permission 权限 0 = root 1-7 = user
#define Shell(Name, Read, Write, Size, Permission)                                                                                                                                                                            \
    uint8_t Name##SHELL_buffer[Size] = {0};                                                                                                                                                                                   \
    shell_t Name = {.read = Read, .write = Write, .parser.buffer = Name##SHELL_buffer, .parser.bufferSize = Size / (SHELL_HISTORY_MAX_NUMBER + 1), .head = SHELLSYSTEMHEAD, .status.isChecked = 1, .permission = Permission}; \
    __attribute__((constructor)) void Name##_init(void)                                                                                                                                                                       \
    {                                                                                                                                                                                                                         \
        for (short i = 0; i < SHELL_HISTORY_MAX_NUMBER; i++)                                                                                                                                                                  \
        {                                                                                                                                                                                                                     \
            Name.history.item[i] = (char *)Name##SHELL_buffer + Name.parser.bufferSize * (i + 1);                                                                                                                             \
        }                                                                                                                                                                                                                     \
    }
#endif

/// @brief 注册按键-Ex
/// @param shell 终端
/// @param Value 按键键值
/// @param permission 权限
/// @param name 按键名称
/// @param desc 按键描述
/// @param code 按键执行代码
#define ShellKeyEx(Shell, Value, Permission, Name, Desc, Display) \
    shell_cmd_t Shell##Value;                                     \
    void Shell##Value##_function(shell_t *shell);                 \
    __attribute__((constructor)) void shell##Value##_init(void)   \
    {                                                             \
        Shell##Value.cmd.key.value = Value;                       \
        Shell##Value.cmd.key.function = shell##Value##_function;  \
        Shell##Value.cmd.key.desc = #Desc;                        \
        Shell##Value.cmd.key.name = #Name;                        \
        Shell##Value.attr.attrs.permission = Permission;          \
        Shell##Value.attr.attrs.type.main = KEYCMD;               \
        Shell##Value.attr.attrs.helpDisplay = Display;            \
        (Shell).keyListFlag = 1;                                  \
        Shell##Value.next = Shell.head;                           \
        Shell.head = &Shell##Value;                               \
    }                                                             \
    void Shell##Value##_function(shell_t *shell)

/// @brief 注册按键
/// @param Shell 终端
/// @param Value 按键键值
/// @param Permission 权限
#define ShellKey(Shell, Value, Permission) ShellKeyEx(Shell, Value, Permission, NULL, NULL, 0)

/// @brief 注册Main函数
/// @param Shell 终端
/// @param Name 函数名
/// @param Desc 函数描述
/// @param Permission 权限
#define ShellMain(Shell, Name, Desc, Permission)                                \
    shell_cmd_t Shell##Name;                                                    \
    int Shell##Name##_function(int argc, char *argv[]);                         \
    __attribute__((constructor)) void shell##Name##_init(void)                  \
    {                                                                           \
        Shell##Name.cmd.func.name = #Name;                                      \
        Shell##Name.cmd.func.function = Shell##Name##_function;                 \
        Shell##Name.cmd.func.desc = #Desc;                                      \
        Shell##Name.attr.attrs.permission = Permission;                         \
        Shell##Name.attr.attrs.type.main = FUNCMD;                              \
        Shell##Name.attr.attrs.type.addition = SHELL_TYPE_FUNC_MAIN;            \
        Shell##Name.attr.attrs.type.returnDisplay = SHELL_TYPE_FUNC_RETURN_INT; \
        Shell##Name.attr.attrs.helpDisplay = 1;                                 \
        (Shell).funcListFlag = 1;                                               \
        Shell##Name.next = Shell.head;                                          \
        (Shell).head = &Shell##Name;                                            \
    }                                                                           \
    int Shell##Name##_function(int argc, char *argv[])

/// @brief 注册函数
/// @param Shell 终端
/// @param Name 函数名
/// @param Desc 函数描述
/// @param Permission 权限
/// @param ReturnDisplay 返回值显示类型
/// @param Func 函数指针
#define ShellFunc(Shell, Name, Desc, Permission, ReturnDisplay, Func)     \
    shell_cmd_t Shell##Name;                                              \
    __attribute__((constructor)) void shell##Name##_init(void)            \
    {                                                                     \
        Shell##Name.cmd.func.name = #Name;                                \
        Shell##Name.cmd.func.function = (int (*)())Func;                  \
        Shell##Name.cmd.func.desc = #Desc;                                \
        Shell##Name.attr.attrs.permission = Permission;                   \
        Shell##Name.attr.attrs.type.main = FUNCMD;                        \
        Shell##Name.attr.attrs.type.addition = SHELL_TYPE_FUNC_FUNC_AUTO; \
        Shell##Name.attr.attrs.type.returnDisplay = ReturnDisplay;        \
        Shell##Name.attr.attrs.helpDisplay = 1;                           \
        (Shell).funcListFlag = 1;                                         \
        Shell##Name.next = Shell.head;                                    \
        (Shell).head = &Shell##Name;                                      \
    }

/// @brief 注册数字变量
/// @param Shell 终端
/// @param Name 变量名
/// @param Desc 变量描述
/// @param Permission 权限
/// @param Value 变量值
#define ShellVarNumber(Shell, Name, Desc, Permission, Value)          \
    shell_cmd_t Shell##Name;                                          \
    __attribute__((constructor)) void shell##Name##_init(void)        \
    {                                                                 \
        Shell##Name.cmd.var.name = #Name;                             \
        Shell##Name.cmd.var.value = Value;                            \
        Shell##Name.cmd.var.desc = #Desc;                             \
        Shell##Name.attr.attrs.permission = Permission;               \
        Shell##Name.attr.attrs.type.main = VARCMD;                    \
        Shell##Name.attr.attrs.type.addition = SHELL_TYPE_VAR_NUMBER; \
        Shell##Name.attr.attrs.helpDisplay = 1;                       \
        (Shell).varListFlag = 1;                                      \
        Shell##Name.next = Shell.head;                                \
        (Shell).head = &Shell##Name;                                  \
    }

/// @brief 注册字符串变量
/// @param Shell 终端
/// @param Name 变量名
/// @param Desc 变量描述
/// @param Permission 权限
/// @param Value 变量值
#define ShellVarString(Shell, Name, Desc, Permission, Value)          \
    shell_cmd_t Shell##Name;                                          \
    __attribute__((constructor)) void shell##Name##_init(void)        \
    {                                                                 \
        Shell##Name.cmd.var.name = #Name;                             \
        Shell##Name.cmd.var.value = Value;                            \
        Shell##Name.cmd.var.desc = #Desc;                             \
        Shell##Name.attr.attrs.permission = Permission;               \
        Shell##Name.attr.attrs.type.main = VARCMD;                    \
        Shell##Name.attr.attrs.type.addition = SHELL_TYPE_VAR_STRING; \
        Shell##Name.attr.attrs.helpDisplay = 1;                       \
        (Shell).varListFlag = 1;                                      \
        Shell##Name.next = Shell.head;                                \
        (Shell).head = &Shell##Name;                                  \
    }

/// @brief 注册线程
/// @param Shell 终端
/// @param Name 线程名
/// @param Desc 线程描述
/// @param Permission 权限
#define ShellThread(Shell, Name, Password, Desc, Permission)   \
    shell_cmd_t Shell##Name;                                   \
    void Shell##Name##_function(shell_t *shell);               \
    __attribute__((constructor)) void shell##Name##_init(void) \
    {                                                          \
        Shell##Name.cmd.sys.name = #Name;                      \
        Shell##Name.cmd.sys.password = #Password;              \
        Shell##Name.cmd.sys.function = Shell##Name##_function; \
        Shell##Name.cmd.sys.desc = #Desc;                      \
        Shell##Name.attr.attrs.permission = Permission;        \
        Shell##Name.attr.attrs.type.main = SYSCMD;             \
        Shell##Name.attr.attrs.helpDisplay = 1;                \
        (Shell).sysListFlag = 1;                               \
        Shell##Name.next = Shell.head;                         \
        (Shell).head = &Shell##Name;                           \
    }                                                          \
    void Shell##Name##_function(shell_t *shell)

/// @brief 线程内退出线程
#define shellExit() shell->system.state = 0

/// @brief 线程内读取输入
/// @param buff 输入缓冲区
/// @param size 输入缓冲区大小
#define shellRead(buff, size) shell->read(buff, size)

/// @brief 线程内写入输出
/// @param buff 输出缓冲区
/// @param size 输出缓冲区大小
#define shellWrite(buff, size) shell->write(buff, size)

/// @brief 线程内打印输出
/// @param format 输出格式
/// @param ... 输出参数
#define shellPrint(format, ...) shellPrintf(shell, format, ##__VA_ARGS__)

#endif // __LOVE_FISH_SHELL_H__
