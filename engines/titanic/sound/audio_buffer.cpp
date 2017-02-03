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

namespace Titanic {

CAudioBuffer::CAudioBuffer(int bufferSize) : _flag(true), _field18(0) {
	_buffer.resize(bufferSize);
	reset();
}

CAudioBuffer::~CAudioBuffer() {
	_buffer.clear();
}

void CAudioBuffer::reset() {
	_flag = true;
	_fieldC = _field10 = _buffer.size() / 2;
}

byte *CAudioBuffer::getDataPtr1() {
	return _flag ? &_buffer[_buffer.size() / 2] : &_buffer[0];
}

byte *CAudioBuffer::getDataPtr2() {
	return _flag ? &_buffer[0] : &_buffer[_buffer.size() / 2];
}

byte *CAudioBuffer::getPtr1() {
	byte *ptr = getDataPtr1();
	return ptr + (_buffer.size() / 2 - _fieldC);
}

byte *CAudioBuffer::getPtr2() {
	byte *ptr = getDataPtr2();
	return ptr + (_buffer.size() / 2 - _field10);
}

void CAudioBuffer::setC(int val) {
	_fieldC -= val;
	if (_fieldC < 0) {
		_fieldC = 0;
	} else if (val && !_field10) {
		update();
	}
}

void CAudioBuffer::set10(int val) {
	_field10 -= val;
	if (_field10 < 0) {
		_field10 = 0;
	} else if (val && !_field10) {
		update();
	}
}

void CAudioBuffer::update() {
	_flag = !_flag;
	_fieldC = _field10 = _buffer.size() / 2;
}

void CAudioBuffer::enterCriticalSection() {
	_mutex.lock();
}

void CAudioBuffer::leaveCriticalSection() {
	_mutex.unlock();
}

} // End of namespace Titanic
