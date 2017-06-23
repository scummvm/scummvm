/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_AUDIO_BUFFER_H
#define TITANIC_AUDIO_BUFFER_H

#include "common/array.h"
#include "common/mutex.h"

namespace Titanic {

class CAudioBuffer {
private:
	Common::Mutex _mutex;
	Common::Array<int16> _data;
	int16 *_frontP, *_backP;

	/**
	 * Reclaims any space at the start of the array resulting from
	 * having read values off the font
	 */
	void compact();
public:
	bool _finished;
public:
	CAudioBuffer(int maxSize);

	/**
	 * Resets the audio buffer
	 */
	void reset();

	/**
	 * Returns true if the buffer is empty
	 */
	bool empty() const { return _data.empty(); }

	/**
	 * Returns the number of 16-bit entries in the buffer
	 */
	int size() const { return _backP - _frontP; }

	/**
	 * Returns true if the buffer is full
	 */
	bool full() const { return (_backP - _frontP) == (int)_data.size(); }

	/**
	 * Returns the number of entries free in the buffer
	 */
	int freeSize();

	/**
	 * Adds a value to the buffer
	 */
	void push(int16 value);

	/**
	 * Adds a value to the buffer
	 */
	void push(int16 *values, int count);

	/**
	 * Removes a value from the buffer
	 */
	int16 pop();

	/**
	 * Enters a critical section
	 */
	void enterCriticalSection();

	/**
	 * Leave a critical section
	 */
	void leaveCriticalSection();
};

} // End of namespace Titanic

#endif /* TITANIC_AUDIO_BUFFER_H */
