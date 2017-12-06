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

Process::Process(AGDSEngine *engine, Object* object, unsigned ip) :
	_engine(engine), _object(object), _ip(ip), _status(kStatusActive), _exitCode(kExitCodeDestroy) {
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
	if (id == -1)
		return Common::String();
	else if (id <= -2 && id > -12)
		return _engine->getSharedStorage(id);
	else
		return _object->getString(id).string;
}

void Process::activate() {
	switch(_status)
	{
		case kStatusActive:
			break;
		case kStatusPassive:
			_status = kStatusActive;
			break;
		default:
			error("process in invalid state");
	}
}

}
