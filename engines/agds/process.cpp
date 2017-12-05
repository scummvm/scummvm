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
#include "agds/agds.h"
#include "common/debug.h"

namespace AGDS {

enum Opcode {
	kEnter							= 5,
	kPop							= 10,
	kPushImm8						= 18,
	kScreenLoadObject				= 76,
	kSetSystemVariable				= 142,
	kGetRegionWidth					= 146,
	kGetRegionHeight				= 147,
	kAppendToSharedStorage			= 175,
	kLoadPicture					= 198,
	kMax							= 248
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

Process::Process(AGDSEngine *engine, Object* object) :
	_engine(engine), _object(object), _ip(0), _status(kStatusActive), _exitCode(kExitCodeDestroy) {
}

void Process::push(int32 value) {
	_stack.push(value);
}

int32 Process::pop() {
	if (_stack.empty())
		error("stack underflow, ip: %08x", _ip);
	return _stack.pop();
}

Common::String Process::getString(int id) {
	if (id <= -2 && id > -12)
		return _engine->getSharedStorage(id);
	else
		return _object->getString(id).string;
}


ProcessExitCode Process::execute() {
	_exitCode = kExitCodeDestroy;

	const Object::CodeType &code = _object->getCode();
	while(_status == kStatusActive && _ip < code.size()) {
		uint8 op = next();
		switch(op) {
			OP_UU	(kEnter, enter);
			OP		(kPop, pop);
			OP_C	(kPushImm8, push);
			OP		(kScreenLoadObject, loadScreenObject);
			OP		(kSetSystemVariable, setSystemVariable);
			OP		(kAppendToSharedStorage, appendToSharedStorage);
			OP		(kLoadPicture, loadPicture);
		default:
			debug("%08x: unknown opcode 0x%02x (%u)", _ip - 1, (unsigned)op, (unsigned)op);
			_status = kStatusError;
			break;
		}
	}

	return _exitCode;
}

}
