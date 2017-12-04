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

#include "agds/process.h"
#include "common/debug.h"

namespace AGDS {

enum Opcode {
	kEnter				= 5,
	kPushImm8			= 18,
	kOp142				= 142,
	kLoadPicture		= 198,
	kMax				= 248
};


//fixme: add trace here
#define OP(NAME, METHOD) \
	case NAME: METHOD (); break

#define OP_C(NAME, METHOD) \
	case NAME: { int8 arg = next(); METHOD (arg); } break

#define OP_B(NAME, METHOD) \
	case NAME: { uint8 arg = next(); METHOD (arg); } break

#define OP_U(NAME, METHOD) \
	case NAME: { uint16 arg = next16(); METHOD (arg); } break

#define OP_UU(NAME, METHOD) \
	case NAME: { uint16 arg1 = next16(); uint16 arg2 = next16(); METHOD (arg1, arg2); } break

Process::Process(Object* object) : _object(object), _ip(0), _failed(false) { }

void Process::push(int32 value) {
	_stack.push(value);
}

int32 Process::pop() {
	if (_stack.empty())
		error("stack underflow, ip: %08x", _ip);
	return _stack.pop();
}

void Process::execute() {
	const Object::CodeType &code = _object->getCode();
	while(!_failed && _ip < code.size()) {
		uint8 op = next();
		switch(op) {
			OP_UU	(kEnter, enter);
			OP_C	(kPushImm8, push);
			OP		(kLoadPicture, loadPicture);
			OP		(kOp142, stub142);
		default:
			debug("%08x: unknown opcode 0x%02x (%u)", _ip - 1, (unsigned)op, (unsigned)op);
			_failed = true;
			break;
		}
	}
}

}
