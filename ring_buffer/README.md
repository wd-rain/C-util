# ring_buffer

## 快速开始

```c
#include "ring_buffer.h"

// 创建256字节的环形缓冲区
uint8_t buffer_memory[256];
ring_buffer_t rb;

void app_init(void)
{
    // 初始化
    if (!rb_init(&rb, buffer_memory, 256))
    {
        // 初始化失败（大小不是2的N次方）
        return;
    }

    // 写入数据
    uint8_t data = 0x42;
    if (rb_write_byte(&rb, data))
    {
        printf("\r\n写入成功");
    }

    // 查询状态
    printf("\r\n已用: %u字节, 可用: %u字节",
           rb_get_data_size(&rb),
           rb_get_free_size(&rb));

    // 读取数据
    uint8_t read_data;
    if (rb_read_byte(&rb, &read_data))
    {
        printf("\r\n读取到: 0x%02X", read_data);
    }

    // 查询状态
    printf("\r\n已用: %u字节, 可用: %u字节",
           rb_get_data_size(&rb),
           rb_get_free_size(&rb));
}
```

## API

| 函数                    | 说明         |
| ----------------------- | ------------ |
| rb_init()               | 初始化缓冲区 |
| rb_reset()              | 清空缓冲区   |
| rb_write_byte()         | 写入字节     |
| rb_read_byte()          | 读取字节     |
| rb_is_full/empty()      | 状态检查     |
| rb_get_data/free_size() | 空间查询     |

## 注意事项

1.缓冲区大小必须是 2^N

2.非线程安全，多线程要加互斥锁