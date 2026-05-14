#ifndef _FRAME_H_
#define _FRAME_H_

#include <stddef.h>
#include <stdint.h>

// 依赖
#include "../../until/until.h"

// 配置
#ifndef FRAME_HEAD
#define FRAME_HEAD 0xAAU
#endif

#ifndef FRAME_TAIL
#define FRAME_TAIL 0x55U
#endif

#if FRAME_HEAD > 0xFFU
#error "FRAME_HEAD must be less than or equal to 0xFF"
#endif

#if FRAME_TAIL > 0xFFU
#error "FRAME_TAIL must be less than or equal to 0xFF"
#endif

// 类型定义
typedef enum frame_status_t
{
    WD_FRAME_STATUS_OK = 0,
    WD_FRAME_STATUS_PENDING,
    WD_FRAME_STATUS_COMPLETE,
    WD_FRAME_STATUS_OVERFLOW,
    WD_FRAME_STATUS_LEN_ERROR,
    WD_FRAME_STATUS_CHECKSUM_ERROR,
    WD_FRAME_STATUS_TAIL_ERROR
} FrameStatus;

typedef void (*frame_decoder_fn)(const uint8_t *data, size_t size);

typedef struct frame_view_t
{
    const uint8_t *data;
    size_t size;
} FrameView;

typedef struct frame_decoder_t
{
    FrameView view;
    uint8_t *buffer;
    size_t capacity;
    size_t length;
    size_t index;
    uint8_t checksum;
    uint8_t state;
    frame_decoder_fn callback;
} FrameDecoder;

// 接口
size_t frame_encoded_size(size_t size);
FrameStatus frame_encode(const uint8_t *data, size_t size, uint8_t *frame, size_t frame_capacity, size_t *frame_size);

void frame_decoder_init(FrameDecoder *self, uint8_t *buffer, size_t capacity, frame_decoder_fn callback);
void frame_decoder_reset(FrameDecoder *self);
FrameStatus frame_decoder_decode_byte(FrameDecoder *self, uint8_t byte);
const FrameView *frame_decoder_view(const FrameDecoder *self);
void frame_decoder_deinit(FrameDecoder *self);
#endif
