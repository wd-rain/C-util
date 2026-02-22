/**
 * @file      ring_buffer.h
 * @brief     环形缓冲区
 * @author    Aki
 * @version   1.0
 * @date      2025-08-29
 */

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 环形缓冲区结构体
 */
typedef struct
{
    uint8_t *buffer; // 数据存储区
    uint32_t size;   // 缓冲区大小（必须是2的N次方）
    uint32_t head;   // 写指针
    uint32_t tail;   // 读指针
} ring_buffer_t;

// 初始化和管理
/// @brief 清空环形缓冲区
/// @param name 句柄
/// @param size 大小（2的幂次）
/// @note 失败会进入死循环
#define Rb(name, size)                                      \
    uint8_t name##RB_BUFFER[size];                          \
    ring_buffer_t name = {name##RB_BUFFER, size, 0, 0};     \
    void Rb_INIT_##name(void) __attribute__((constructor)); \
    void Rb_INIT_##name(void)                               \
    {                                                       \
        if (!((size != 0) && ((size & (size - 1)) == 0)))   \
        {                                                   \
            while (1)                                       \
                ;                                           \
        }                                                   \
    }

void rb_reset(ring_buffer_t *rb);

// 数据读写
bool rb_write_byte(ring_buffer_t *rb, uint8_t byte);
bool rb_read_byte(ring_buffer_t *rb, uint8_t *byte);
uint32_t rb_read(ring_buffer_t *rb, uint8_t *bytes, uint32_t size);
uint32_t rb_write(ring_buffer_t *rb, const uint8_t *bytes, uint32_t size);

// 状态查询
uint32_t rb_get_data_size(const ring_buffer_t *rb); // 已用空间
uint32_t rb_get_free_size(const ring_buffer_t *rb); // 可用空间
bool rb_is_full(const ring_buffer_t *rb);
bool rb_is_empty(const ring_buffer_t *rb);

#endif // __RING_BUFFER_H__
