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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/illusions.h"
#include "illusions/scriptstack.h"

namespace Illusions {

// ScriptStack

ScriptStack::ScriptStack() {
	clear();
}

void ScriptStack::clear() {
	for (uint i = 0; i < 256; ++i) {
		_stack[i] = (int16)0xEEEE;
	}
	_stackPos = 256;
}

void ScriptStack::push(int16 value) {
	--_stackPos;
	if (_stackPos > 0)
		_stack[_stackPos] = value;
}

int16 ScriptStack::pop() {
	int16 value = 0;
	if (_stackPos < 256) {
		value = _stack[_stackPos];
		_stack[_stackPos] = (int16)0xEEEE;
		++_stackPos;
	}
	return value;
}

int16 ScriptStack::peek() {
	int16 value = 0;
	if (_stackPos < 256)
		value = _stack[_stackPos];
	return value;
}

int16 *ScriptStack::topPtr() {
	return &_stack[_stackPos];
}

} // End of namespace Illusions
