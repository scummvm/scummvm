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

CAudioBuffer::CAudioBuffer(int bufferSize) : _flag(true), _disabled(false) {
	_buffer.resize(bufferSize);
	reset();
}

CAudioBuffer::~CAudioBuffer() {
	_buffer.clear();
}

void CAudioBuffer::reset() {
	_flag = true;
	_readBytesLeft = _writeBytesLeft = _buffer.size() / 2;
}

byte *CAudioBuffer::getBegin() {
	return _flag ? &_buffer[_buffer.size() / 2] : &_buffer[0];
}

byte *CAudioBuffer::getEnd() {
	return _flag ? &_buffer[0] : &_buffer[_buffer.size() / 2];
}

uint16 *CAudioBuffer::getReadPtr() {
	byte *ptr = getBegin();
	return (uint16 *)(ptr + (_buffer.size() / 2 - _readBytesLeft));
}

uint16 *CAudioBuffer::getWritePtr() {
	byte *ptr = getEnd();
	return (uint16 *)(ptr + (_buffer.size() / 2 - _writeBytesLeft));
}

void CAudioBuffer::advanceRead(int size) {
	_readBytesLeft -= size;
	if (_readBytesLeft < 0) {
		_readBytesLeft = 0;
	} else if (size && !_writeBytesLeft) {
		reverse();
	}
}

void CAudioBuffer::advanceWrite(int size) {
	_writeBytesLeft -= size;
	if (_writeBytesLeft < 0) {
		_writeBytesLeft = 0;
	} else if (size && !_readBytesLeft) {
		reverse();
	}
}

void CAudioBuffer::reverse() {
	_flag = !_flag;
	_readBytesLeft = _writeBytesLeft = _buffer.size() / 2;
}

void CAudioBuffer::enterCriticalSection() {
	_mutex.lock();
}

void CAudioBuffer::leaveCriticalSection() {
	_mutex.unlock();
}

} // End of namespace Titanic
