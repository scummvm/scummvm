/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "KeysBuffer.h"

namespace CEKEYS {

	KeysBuffer::KeysBuffer(int capacity):
	_capacity(capacity), _buffer(new Key[capacity]), _currentAdd(0), _currentGet(0) {
	}

	bool KeysBuffer::add(const Key *key) {
		if (_currentAdd == _capacity - 1)
			return false;

		_currentGet = _currentAdd;
		_buffer[_currentAdd++] = *key;

		return true;
	}

	KeysBuffer* KeysBuffer::Instance(int capacity) {		
		if (!_instance)
			_instance = new KeysBuffer(capacity);
		
		return _instance;		
	}


	Key* KeysBuffer::get() {
		Key *result;

		if (_currentAdd <= _currentGet)
			return NULL;
		result = &_buffer[_currentGet++];
		if (_currentGet == _currentAdd) {
			_currentGet = 0;
			_currentAdd = 0;
		}
		return result;
	}

	KeysBuffer::~KeysBuffer() {
		delete [] _buffer;
	}

	KeysBuffer *KeysBuffer::_instance = NULL;
}


