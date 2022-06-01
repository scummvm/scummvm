/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GLK_SCOTT_RINGBUFFER_H
#define GLK_SCOTT_RINGBUFFER_H

#include "glk/scott/types.h"

namespace Glk {
namespace Scott {

// Opaque circular buffer structure
struct CircularBuf;
// Handle type, the way users interact with the API
typedef CircularBuf *cbuf_handle_t;

/// Pass in a storage buffer and size
/// Returns a circular buffer handle
cbuf_handle_t circularBufInit(uint8_t *buffer, size_t size);

/// Free a circular buffer structure.
/// Does not free data buffer; owner is responsible for that
void circularBufFree(cbuf_handle_t me);

/// Reset the circular buffer to empty, head == tail
void circularBufReset(cbuf_handle_t me);

/// Rejects new data if the buffer is full
/// Returns 0 on success, -1 if buffer is full
int circularBufPut(cbuf_handle_t me, uint8_t x, uint8_t y);

/// Retrieve a value from the buffer
/// Returns 0 on success, -1 if the buffer is empty
int circularBufGet(cbuf_handle_t me, int *x, int *y);

/// Returns true if the buffer is empty
bool circularBufEmpty(cbuf_handle_t me);

/// Returns true if the buffer is full
bool circularBufFull(cbuf_handle_t me);

/// Returns the maximum capacity of the buffer
size_t circularBufCapacity(cbuf_handle_t me);

/// Returns the current number of elements in the buffer
size_t circularBufSize(cbuf_handle_t me);

} // End of namespace Scott
} // End of namespace Glk

#endif
