/**
 * @file      ring_buffer.c
 * @brief     环形缓冲区
 * @author    Aki
 * @version   1.0
 * @date      2025-08-29
 */

#include "ring_buffer.h"
#include <stddef.h>

/// @brief 清空环形缓冲区
/// @param rb 句柄
void rb_reset(ring_buffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

/// @brief 写入单个字节
/// @param rb 句柄
/// @param byte 字节
/// @return 写入成功返回true，否则返回false
bool rb_write_byte(ring_buffer_t *rb, uint8_t byte)
{
    if (rb_is_full(rb))
    {
        return false; // 缓冲区已满
    }

    // 使用位掩码代替取模运算，提高效率
    rb->buffer[rb->head & (rb->size - 1)] = byte;
    rb->head++; // 利用uint32_t自然回绕

    return true;
}

/// @brief 写入字节数组
/// @param rb 句柄
/// @param bytes 字节数组
/// @param size 大小
/// @return 写入成功字节数
uint32_t rb_write(ring_buffer_t *rb, const uint8_t *bytes, uint32_t size)
{
    uint32_t cnt = 0;
    for(uint32_t i = 0; i < size; i++)
    {
        if(rb_write_byte(rb, bytes[i]))
        {
            cnt++;
        }
    }
    return cnt;
}

/// @brief 读取单个字节
/// @param rb 句柄
/// @param byte 字节
/// @return 读取成功返回true，否则返回false
bool rb_read_byte(ring_buffer_t *rb, uint8_t *byte)
{
    if (rb_is_empty(rb))
    {
        return false; // 缓冲区为空
    }

    *byte = rb->buffer[rb->tail & (rb->size - 1)];
    rb->tail++;

    return true;
}

/// @brief 读取字节数组
/// @param rb 句柄
/// @param bytes 字节数组
/// @param size 大小
/// @return 读取成功字节数
uint32_t rb_read(ring_buffer_t *rb, uint8_t *bytes, uint32_t size)
{
		uint32_t cnt = 0;
    for(uint32_t i = 0; i < size; i++)
    {
        if(rb_read_byte(rb, bytes + i))
				{
					cnt++;
				}
    }
		return cnt;
}

/// @brief 已用空间
/// @param rb 句柄
/// @return 已用空间大小
uint32_t rb_get_data_size(const ring_buffer_t *rb)
{
    return (rb->head - rb->tail); // head-tail差值即为已用空间
}

/// @brief 可用空间
/// @param rb 句柄
/// @return 可用空间大小
uint32_t rb_get_free_size(const ring_buffer_t *rb)
{
    return (rb->size - rb_get_data_size(rb));
}

/// @brief 是否已满
/// @param rb 句柄
/// @return 满返回true，否则返回false
bool rb_is_full(const ring_buffer_t *rb)
{
    return (rb_get_data_size(rb) == rb->size);
}

/// @brief 是否为空
/// @param rb 句柄
/// @return 空返回true，否则返回false
bool rb_is_empty(const ring_buffer_t *rb)
{
    return (rb->head == rb->tail);
}

