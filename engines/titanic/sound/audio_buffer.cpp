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

#include "titanic/sound/audio_buffer.h"
#include "common/algorithm.h"

namespace Titanic {

CAudioBuffer::CAudioBuffer(int maxSize) : _finished(false) {
	reset();
}

void CAudioBuffer::reset() {
	_data.clear();
}

void CAudioBuffer::push(int16 value) {
	enterCriticalSection();
	_data.push(value);
	leaveCriticalSection();
}

void CAudioBuffer::push(const int16 *values, int count) {
	enterCriticalSection();

	for (; count > 0; --count, ++values)
		_data.push(*values);

	leaveCriticalSection();
}

int16 CAudioBuffer::pop() {
	enterCriticalSection();
	int16 value = _data.pop();
	leaveCriticalSection();

	return value;
}

int CAudioBuffer::read(int16 *values, int count) {
	enterCriticalSection();

	int bytesRead = 0;
	for (; count > 0 && !_data.empty(); --count, ++bytesRead)
		*values++ = _data.pop();

	leaveCriticalSection();
	return bytesRead;
}

void CAudioBuffer::enterCriticalSection() {
	_mutex.lock();
}

void CAudioBuffer::leaveCriticalSection() {
	_mutex.unlock();
}

} // End of namespace Titanic
