#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>
#include <stdbool.h>

// Config
#define TIMESTARTYEAR 1970
#define TIMESTARTBCDYEAR 2000
#define USE_UINT64_FOR_TIME_T 1
#define USE_BCD_STRUCT 1

#if USE_UINT64_FOR_TIME_T
typedef  uint64_t time_size_t;
#else
typedef  uint32_t time_size_t;
#endif

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} DateTimeDCB;
typedef struct
{
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
} DateTimeHEX;

#if USE_BCD_STRUCT
typedef DateTimeDCB DateTime;
#else
typedef DateTimeHEX DateTime;
#endif



//API
DateTime gmtime(time_size_t seconds);           // 转换UTC时间的秒数为DateTime结构体
time_size_t mktime(const DateTime *dt);         // seconds 输入的UTC时间的秒数

#endif // _TIME_H_
