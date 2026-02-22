// time.c - 自定义时间库实现
#include "time.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// 常量定义
#define BCD(x) (((x) / 10) << 4 | ((x) % 10))
#define HEX(x) ((x >> 4) * 10 + ((x) & 0xF))
#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_HOUR (60 * SECONDS_PER_MINUTE)
#define SECONDS_PER_DAY (24 * SECONDS_PER_HOUR)

// 每月天数（非闰年）
static const uint8_t DAYS_IN_MONTH[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 辅助函数：判断是否是闰年
static bool is_leap_year(uint32_t year)
{
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

// 辅助函数：获取某月的天数
static uint8_t get_days_in_month(uint32_t year, uint32_t month)
{
    if (month < 1 || month > 12)
    {
        return 0;
    }

    if (month == 2 && is_leap_year(year))
    {
        return 29;
    }

    return DAYS_IN_MONTH[month - 1];
}

// 辅助函数：计算从起始年份到指定年份前一年的总天数
static uint64_t days_since_start_year(uint32_t year)
{
    uint64_t days = 0;

    for (uint32_t y = TIMESTARTYEAR; y < year; y++)
    {
        days += is_leap_year(y) ? 366 : 365;
    }

    return days;
}

/// @brief 转换UTC时间的秒数为DateTime结构体
/// @param seconds 秒数
/// @return  DateTime 结构体
DateTime gmtime(time_size_t seconds)
{
    DateTimeHEX dt = {0};
    time_size_t total_seconds = seconds;
    time_size_t days, remaining_seconds;

    // 计算天数
    days = total_seconds / SECONDS_PER_DAY;
    remaining_seconds = total_seconds % SECONDS_PER_DAY;

    // 计算时、分、秒
    dt.hour = (uint32_t)(remaining_seconds / SECONDS_PER_HOUR);
    remaining_seconds %= SECONDS_PER_HOUR;
    dt.minute = (uint32_t)(remaining_seconds / SECONDS_PER_MINUTE);
    dt.second = (uint32_t)(remaining_seconds % SECONDS_PER_MINUTE);

    // 计算年份
    dt.year = TIMESTARTYEAR;
    time_size_t days_in_year;

    while (days > 0)
    {
        days_in_year = is_leap_year(dt.year) ? 366 : 365;

        if (days >= days_in_year)
        {
            days -= days_in_year;
            dt.year++;
        }
        else
        {
            break;
        }
    }

    // 计算月份和日期
    dt.month = 1;
    for (uint32_t m = 1; m <= 12; m++)
    {
        uint8_t days_in_month = get_days_in_month(dt.year, m);

        if (days >= days_in_month)
        {
            days -= days_in_month;
            dt.month++;
        }
        else
        {
            dt.day = (uint32_t)days + 1; // 天数从1开始
            break;
        }
    }

#if USE_BCD_STRUCT
    // 转换为BCD格式
    DateTimeDCB dt_bcd;
    dt.year %= 100; // 只保留两位年份
    dt_bcd.year = BCD(dt.year);
    dt_bcd.month = BCD(dt.month);
    dt_bcd.day = BCD(dt.day);
    dt_bcd.hour = BCD(dt.hour);
    dt_bcd.minute = BCD(dt.minute);
    dt_bcd.second = BCD(dt.second);
    return dt_bcd;
#else
    return dt;
#endif
}

/// @brief 转换DateTime结构体为UTC时间的秒数
/// @param dt DateTime结构体
/// @return UTC时间的秒数
time_size_t mktime(const DateTime *dt_ptr)
{
#if USE_BCD_STRUCT
    DateTimeHEX dt_data;
    dt_data.year = HEX(dt_ptr->year) + TIMESTARTBCDYEAR;
    dt_data.month = HEX(dt_ptr->month);
    dt_data.day = HEX(dt_ptr->day);
    dt_data.hour = HEX(dt_ptr->hour);
    dt_data.minute = HEX(dt_ptr->minute);
    dt_data.second = HEX(dt_ptr->second);
    printf("year = %d, month = %d, day = %d, hour = %d, minute = %d, second = %d\n", dt_data.year, dt_data.month, dt_data.day, dt_data.hour, dt_data.minute, dt_data.second);
    DateTimeHEX *dt = &dt_data;
#else
    DateTimeHEX *dt = (DateTimeHEX *)dt_ptr;
#endif

    if (!dt || dt->month < 1 || dt->month > 12 ||
        dt->day < 1 || dt->day > get_days_in_month(dt->year, dt->month) ||
        dt->hour > 23 || dt->minute > 59 || dt->second > 59)
    {
        return 0;
    }

    time_size_t total_days = 0;

    // 计算从起始年份到指定年份前一年的总天数
    total_days = days_since_start_year(dt->year);

    // 计算指定年份中到指定月份前一月的总天数
    for (uint32_t m = 1; m < dt->month; m++)
    {
        total_days += get_days_in_month(dt->year, m);
    }

    // 加上指定月份中的天数（减1，因为日期从1开始）
    total_days += (dt->day - 1);

    // 计算总秒数
    time_size_t total_seconds = total_days * SECONDS_PER_DAY;
    total_seconds += dt->hour * SECONDS_PER_HOUR;
    total_seconds += dt->minute * SECONDS_PER_MINUTE;
    total_seconds += dt->second;

    return total_seconds;
}
