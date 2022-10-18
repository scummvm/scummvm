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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "glk/scott/ring_buffer.h"

namespace Glk {
namespace Scott {

// The hidden definition of our circular buffer structure
struct CircularBuf {
	uint8_t *_buffer;
	size_t _head;
	size_t _tail;
	size_t _max; // of the buffer
	bool _full;
};

// Return a pointer to a struct instance
cbuf_handle_t circularBufInit(uint8_t *buffer, size_t size) {
	cbuf_handle_t cbuf = new CircularBuf;
	cbuf->_buffer = buffer;
	cbuf->_max = size;
	circularBufReset(cbuf);

	return cbuf;
}

void circularBufReset(cbuf_handle_t me) {
	me->_head = 0;
	me->_tail = 0;
	me->_full = false;
}

void circularBufFree(cbuf_handle_t me) {
	delete me;
}

bool circularBufFull(cbuf_handle_t me) {
	return me->_full;
}

bool circularBufEmpty(cbuf_handle_t me) {
	return (!me->_full && (me->_head == me->_tail));
}

size_t circularBufCapacity(cbuf_handle_t me) {
	return me->_max;
}

size_t circularBufSize(cbuf_handle_t me) {
	size_t size = me->_max;

	if (!me->_full) {
		if (me->_head >= me->_tail) {
			size = (me->_head - me->_tail);
		} else {
			size = (me->_max + me->_head - me->_tail);
		}
	}

	return size;
}

static void advancePointer(cbuf_handle_t me) {
	if (me->_full) {
		if (++(me->_tail) == me->_max) {
			me->_tail = 0;
		}
	}

	if (++(me->_head) == me->_max) {
		me->_head = 0;
	}
	me->_full = (me->_head == me->_tail);
}

static void retreatPointer(cbuf_handle_t me) {

	me->_full = false;
	if (++(me->_tail) == me->_max) {
		me->_tail = 0;
	}
}

int circularBufPut(cbuf_handle_t me, uint8_t x, uint8_t y) {
	int r = -1;

	if (!circularBufFull(me)) {
		me->_buffer[me->_head] = x;
		advancePointer(me);
		if (!circularBufFull(me)) {
			me->_buffer[me->_head] = y;
			advancePointer(me);
			r = 0;
		}
	}
	return r;
}

int circularBufGet(cbuf_handle_t me, int *x, int *y) {
	int r = -1;

	if (!circularBufEmpty(me)) {
		*x = me->_buffer[me->_tail];
		retreatPointer(me);
		if (!circularBufEmpty(me)) {
			*y = me->_buffer[me->_tail];
			retreatPointer(me);
			r = 0;
		}
	}
	return r;
}

} // End of namespace Scott
} // End of namespace Glk
