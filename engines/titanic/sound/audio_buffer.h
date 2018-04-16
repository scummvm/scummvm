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

#include "titanic/support/fixed_queue.h"
#include "common/mutex.h"

namespace Titanic {

#define AUDIO_SAMPLING_RATE 22050

class CAudioBuffer {
private:
	Common::Mutex _mutex;
	FixedQueue<int16, AUDIO_SAMPLING_RATE * 4> _data;
private:
	/**
	 * Enters a critical section
	 */
	void enterCriticalSection();

	/**
	 * Leave a critical section
	 */
	void leaveCriticalSection();
private:
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
	int size() const { return _data.size(); }

	/**
	 * Returns the number of entries free in the buffer
	 */
	int freeSize() const { return _data.freeSize(); }

	/**
	 * Returns true if the buffer is full
	 */
	bool full() const { return _data.full(); }

	/**
	 * Returns true if the audio buffering is finished
	 */
	bool isFinished() const { return _finished && empty(); }

	/**
	 * Adds a value to the buffer
	 */
	void push(int16 value);

	/**
	 * Adds a value to the buffer
	 */
	void push(const int16 *values, int count);

	/**
	 * Removes a value from the buffer
	 */
	int16 pop();

	/**
	 * Reads out a specified number of samples
	 */
	int read(int16 *values, int count);

	/**
	 * Marks the buffer as finishing, and that no more new data will arrive
	 */
	void finalize() { _finished = true; }
};

} // End of namespace Titanic

#endif /* TITANIC_AUDIO_BUFFER_H */
