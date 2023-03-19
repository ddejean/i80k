// The MIT License(MIT)
//
// Copyright(c) 2014 Anders Kalør
// Copyright(c) 2023 Damien Dejean
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish , distribute, sublicense, and / or sell
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

#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stddef.h>
#include <stdint.h>

#define RING_BUFFER_ASSERT(x) x

// Checks if the buffer_size is a power of two.  Due to the design only
// RING_BUFFER_SIZE-1 items can be contained in the buffer.  buffer_size must be
// a power of two.
#define RING_BUFFER_IS_POWER_OF_TWO(size) ((size & (size - 1)) == 0)

// Used as a modulo operator as a % b = (a & (b − 1)) where a is a positive
// index in the buffer and b is the (power of two) size of the buffer.
#define RING_BUFFER_MASK(rb) (rb->buffer_mask)

// Simplifies the use of struct ring_buffer_t.
typedef struct ring_buffer_t ring_buffer_t;

// Structure which holds a ring buffer.  The buffer contains a buffer array as
// well as metadata for the ring buffer.
struct ring_buffer_t {
    // Buffer memory.
    char *buffer;
    // Buffer mask.
    size_t buffer_mask;
    // Index of tail.
    size_t tail_index;
    // Index of head.
    size_t head_index;
};

// Initializes the ring buffer pointed to by <buffer>. This function can also be
// used to empty/reset the buffer.
// @param buffer The ring buffer to initialize.
// @param buf The buffer allocated for the ringbuffer.
// @param buf_size The size of the allocated ringbuffer.
void ring_buffer_init(ring_buffer_t *buffer, char *buf, size_t buf_size);

// Adds a byte to a ring buffer.
// @param buffer The buffer in which the data should be placed.
// @param data The byte to place.
void ring_buffer_queue(ring_buffer_t *buffer, char data);

// Adds an array of bytes to a ring buffer.
// @param buffer The buffer in which the data should be placed.
// @param data A pointer to the array of bytes to place in the queue.
// @param size The size of the array.
void ring_buffer_queue_arr(ring_buffer_t *buffer, char *data, size_t size);

// Returns the oldest byte in a ring buffer.
// @param buffer The buffer from which the data should be returned.
// @param data A pointer to the location at which the data should be placed.
// @return 1 if data was returned; 0 otherwise.
uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data);

// Returns the <len> oldest bytes in a ring buffer.
// @param buffer The buffer from which the data should be returned.
// @param data A pointer to the array at which the data should be placed.
// @param len The maximum number of bytes to return.
// @return The number of bytes returned.
size_t ring_buffer_dequeue_arr(ring_buffer_t *buffer, char *data, size_t len);

// Peeks a ring buffer, i.e. returns an element without removing it.
// @param buffer The buffer from which the data should be returned.
// @param data A pointer to the location at which the data should be placed.
// @param index The index to peek.
// @return 1 if data was returned; 0 otherwise.
uint8_t ring_buffer_peek(ring_buffer_t *buffer, char *data, size_t index);

// Returns whether a ring buffer is empty.
// @param buffer The buffer for which it should be returned whether it is empty.
// @return 1 if empty; 0 otherwise.
uint8_t ring_buffer_is_empty(ring_buffer_t *buffer);

// Returns whether a ring buffer is full.
// @param buffer The buffer for which it should be returned whether it is full.
// @return 1 if full; 0 otherwise.
uint8_t ring_buffer_is_full(ring_buffer_t *buffer);

// Returns the number of items in a ring buffer.
// @param buffer The buffer for which the number of items should be returned.
// @return The number of items in the ring buffer.
size_t ring_buffer_num_items(ring_buffer_t *buffer);

#endif  // _RINGBUFFER_H_
