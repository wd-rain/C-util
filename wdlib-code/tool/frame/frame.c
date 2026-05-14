#include "frame.h"

#include <string.h>

#define FRAME_DATA_MAX_SIZE 255U
#define FRAME_FIXED_SIZE 5U

typedef enum frame_decoder_state_t
{
    FRAME_DECODER_WAIT_HEAD = 0,
    FRAME_DECODER_WAIT_LEN,
    FRAME_DECODER_WAIT_LEN_INV,
    FRAME_DECODER_DATA,
    FRAME_DECODER_CHECKSUM,
    FRAME_DECODER_TAIL
} FrameDecoderState;

static void _frame_decoder_clear_view_checked(FrameDecoder *self)
{
    self->view.data = NULL;
    self->view.size = 0U;
}

static void _frame_decoder_reset_checked(FrameDecoder *self)
{
    self->length = 0U;
    self->index = 0U;
    self->checksum = 0U;
    self->state = (uint8_t)FRAME_DECODER_WAIT_HEAD;
    _frame_decoder_clear_view_checked(self);
}

size_t frame_encoded_size(size_t size)
{
    return size + FRAME_FIXED_SIZE;
}

FrameStatus frame_encode(const uint8_t *data, size_t size, uint8_t *frame, size_t frame_capacity, size_t *frame_size)
{
    uint8_t length;
    uint8_t checksum;
    size_t i;

    WD_ASSERT(frame != NULL);
    WD_ASSERT(frame_size != NULL);
    WD_ASSERT(data != NULL || size == 0U);

    *frame_size = 0U;
    if (size > FRAME_DATA_MAX_SIZE || frame_capacity < frame_encoded_size(size))
    {
        return WD_FRAME_STATUS_OVERFLOW;
    }

    length = (uint8_t)size;
    checksum = length;
    for (i = 0U; i < size; ++i)
    {
        checksum = (uint8_t)(checksum + data[i]);
    }

    frame[0U] = (uint8_t)FRAME_HEAD;
    frame[1U] = length;
    frame[2U] = (uint8_t)~length;
    if (size > 0U)
    {
        (void)memcpy(&frame[3U], data, size);
    }
    frame[3U + size] = checksum;
    frame[4U + size] = (uint8_t)FRAME_TAIL;
    *frame_size = frame_encoded_size(size);

    return WD_FRAME_STATUS_OK;
}

void frame_decoder_init(FrameDecoder *self, uint8_t *buffer, size_t capacity, frame_decoder_fn callback)
{
    WD_ASSERT(self != NULL);
    WD_ASSERT(buffer != NULL);
    WD_ASSERT(capacity > 0U);

    self->buffer = buffer;
    self->capacity = capacity;
    self->callback = callback;
    _frame_decoder_reset_checked(self);
}

void frame_decoder_reset(FrameDecoder *self)
{
    WD_ASSERT(self != NULL);

    _frame_decoder_reset_checked(self);
}

FrameStatus frame_decoder_decode_byte(FrameDecoder *self, uint8_t byte)
{
    FrameStatus status;

    WD_ASSERT(self != NULL);

    status = WD_FRAME_STATUS_PENDING;
    _frame_decoder_clear_view_checked(self);

    if (self->state == (uint8_t)FRAME_DECODER_WAIT_HEAD)
    {
        if (byte == (uint8_t)FRAME_HEAD)
        {
            // 识别到帧头后进入长度解析。
            self->state = (uint8_t)FRAME_DECODER_WAIT_LEN;
        }
    }
    else if (self->state == (uint8_t)FRAME_DECODER_WAIT_LEN)
    {
        self->length = (size_t)byte;
        self->index = 0U;
        self->checksum = byte;

        if (self->length > self->capacity)
        {
            // 长度超过接收缓存时丢弃当前帧。
            _frame_decoder_reset_checked(self);
            status = WD_FRAME_STATUS_OVERFLOW;
        }
        else
        {
            self->state = (uint8_t)FRAME_DECODER_WAIT_LEN_INV;
        }
    }
    else if (self->state == (uint8_t)FRAME_DECODER_WAIT_LEN_INV)
    {
        if (byte != (uint8_t)~((uint8_t)self->length))
        {
            // 长度反码不匹配时立即重新寻找帧头。
            _frame_decoder_reset_checked(self);
            status = WD_FRAME_STATUS_LEN_ERROR;
        }
        else if (self->length == 0U)
        {
            self->state = (uint8_t)FRAME_DECODER_CHECKSUM;
        }
        else
        {
            self->state = (uint8_t)FRAME_DECODER_DATA;
        }
    }
    else if (self->state == (uint8_t)FRAME_DECODER_DATA)
    {
        self->buffer[self->index] = byte;
        self->checksum = (uint8_t)(self->checksum + byte);
        self->index++;

        if (self->index >= self->length)
        {
            // 数据区接收完毕后转入校验字节判断。
            self->state = (uint8_t)FRAME_DECODER_CHECKSUM;
        }
    }
    else if (self->state == (uint8_t)FRAME_DECODER_CHECKSUM)
    {
        if (byte != self->checksum)
        {
            // 校验错误时丢弃当前帧。
            _frame_decoder_reset_checked(self);
            status = WD_FRAME_STATUS_CHECKSUM_ERROR;
        }
        else
        {
            self->state = (uint8_t)FRAME_DECODER_TAIL;
        }
    }
    else if (self->state == (uint8_t)FRAME_DECODER_TAIL)
    {
        if (byte != (uint8_t)FRAME_TAIL)
        {
            // 尾帧错误时丢弃当前帧。
            _frame_decoder_reset_checked(self);
            status = WD_FRAME_STATUS_TAIL_ERROR;
        }
        else
        {
            // 先发布视图再触发用户回调。
            self->view.data = self->buffer;
            self->view.size = self->length;
            if (self->callback != NULL)
            {
                self->callback(self->view.data, self->view.size);
            }
            self->state = (uint8_t)FRAME_DECODER_WAIT_HEAD;
            self->length = 0U;
            self->index = 0U;
            self->checksum = 0U;
            status = WD_FRAME_STATUS_COMPLETE;
        }
    }
    else
    {
        _frame_decoder_reset_checked(self);
    }

    return status;
}

const FrameView *frame_decoder_view(const FrameDecoder *self)
{
    WD_ASSERT(self != NULL);

    return &self->view;
}

void frame_decoder_deinit(FrameDecoder *self)
{
    WD_ASSERT(self != NULL);

    _frame_decoder_reset_checked(self);
    self->buffer = NULL;
    self->capacity = 0U;
    self->callback = NULL;
}
