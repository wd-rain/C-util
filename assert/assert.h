#ifndef ASSERT_H
#define ASSERT_H

#include <string.h>
#include <stdio.h>

/// @brief 断言宏，表达式为假时执行指定操作
/// @param expr 表达式
/// @param action 断言失败时执行的操作
#define assertAction(expr, action) do{if (!(expr)) { action; }}while(0)

/// @brief 断言宏，表达式为假时将消息复制到缓冲区
/// @param expr 表达式
/// @param buffer 缓冲区
/// @param message 断言失败时显示的消息
#define assertMessage(expr, buffer, message) do{if (!(expr)) { strncpy(buffer, message, strlen(message)); }}while(0)

/// @brief 断言宏，表达式为假时打印格式化消息
/// @param expr 表达式
/// @param format 格式化字符串
/// @param ... 格式化参数
#define assertPrintf(expr, format, ...) do{if (!(expr)) { printf(format, __VA_ARGS__); }}while(0)

/// @brief 断言宏，表达式为假时停止程序
/// @param expr 表达式
#define assertStop(expr) do{if (!(expr)) { while(1); }}while(0)

/// @brief 断言宏，表达式为假时执行代码
/// @param expr 表达式
/// @param code 代码
#define assertCode(expr, code) (if (!(expr)){code;})


#endif // ASSERT_H
