// The MIT License(MIT)
//
// Copyright(c) 2014 Anders Kalør
// Copyright(c) 2023 Damien Dejean
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Source: https://github.com/AndersKaloer/Ring-Buffer

#include "ringbuffer.h"

void ring_buffer_init(ring_buffer_t *buffer, char *buf, size_t buf_size) {
    buffer->buffer = buf;
    buffer->buffer_mask = buf_size - 1;
    buffer->tail_index = 0;
    buffer->head_index = 0;
}

void ring_buffer_queue(ring_buffer_t *buffer, const char data) {
    // Is buffer full?
    if (ring_buffer_is_full(buffer)) {
        // Is going to overwrite the oldest byte
        // Increase tail index
        buffer->tail_index =
            ((buffer->tail_index + 1) & RING_BUFFER_MASK(buffer));
    }

    // Place data in buffer
    buffer->buffer[buffer->head_index] = data;
    buffer->head_index = ((buffer->head_index + 1) & RING_BUFFER_MASK(buffer));
}

void ring_buffer_queue_arr(ring_buffer_t *buffer, const char *data,
                           size_t size) {
    // Add bytes; one by one
    size_t i;
    for (i = 0; i < size; i++) {
        ring_buffer_queue(buffer, data[i]);
    }
}

uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data) {
    if (ring_buffer_is_empty(buffer)) {
        // No items
        return 0;
    }

    *data = buffer->buffer[buffer->tail_index];
    buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK(buffer));
    return 1;
}

size_t ring_buffer_dequeue_arr(ring_buffer_t *buffer, char *data, size_t len) {
    char *data_ptr = data;
    size_t cnt = 0;

    if (ring_buffer_is_empty(buffer)) {
        // No items
        return 0;
    }

    while ((cnt < len) && ring_buffer_dequeue(buffer, data_ptr)) {
        cnt++;
        data_ptr++;
    }
    return cnt;
}

uint8_t ring_buffer_peek(ring_buffer_t *buffer, char *data, size_t index) {
    size_t data_index;
    if (index >= ring_buffer_num_items(buffer)) {
        // No items at index
        return 0;
    }

    // Add index to pointer
    data_index = ((buffer->tail_index + index) & RING_BUFFER_MASK(buffer));
    *data = buffer->buffer[data_index];
    return 1;
}

inline uint8_t ring_buffer_is_empty(ring_buffer_t *buffer) {
    return (buffer->head_index == buffer->tail_index);
}

inline uint8_t ring_buffer_is_full(ring_buffer_t *buffer) {
    return ((buffer->head_index - buffer->tail_index) &
            RING_BUFFER_MASK(buffer)) == RING_BUFFER_MASK(buffer);
}

inline size_t ring_buffer_num_items(ring_buffer_t *buffer) {
    return ((buffer->head_index - buffer->tail_index) &
            RING_BUFFER_MASK(buffer));
}
