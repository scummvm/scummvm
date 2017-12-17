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
#include "common/system.h"

namespace AGDS {

Process::Process(AGDSEngine *engine, Object* object, unsigned ip) :
	_engine(engine), _object(object), _ip(ip), _status(kStatusActive), _exitCode(kExitCodeDestroy),
	_glyphWidth(16), _glyphHeight(16) {
}

void Process::debug(const char *str, ...) {
	va_list va;
	va_start(va, str);

	Common::String format = Common::String::format("%s %04x: %s", _object->getName().c_str(), _lastIp + 7, str);
	Common::String buf = Common::String::vformat(format.c_str(), va);

	buf += '\n';

	if (g_system)
		g_system->logMessage(LogMessageType::kDebug, buf.c_str());

	va_end(va);
}

void Process::push(int32 value) {
	_stack.push(value);
}

int32 Process::pop() {
	if (_stack.empty())
		error("stack underflow at %s:%04x", _object->getName().c_str(), _lastIp + 7);
	return _stack.pop();
}

int32 Process::top() {
	if (_stack.empty())
		error("stack underflow, %s:%04x", _object->getName().c_str(), _lastIp + 7);
	return _stack.top();
}

Common::String Process::getString(int id) {
	if (id == -1)
		return Common::String();
	else if (id <= -2 && id > -12)
		return _engine->getSharedStorage(id);
	else
		return _object->getString(id).string;
}

Common::String Process::popFilename() {
	return _engine->loadText(popString());
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
			error("process in invalid state %d", _status);
	}
}

}
