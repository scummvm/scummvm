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

Process::Process(AGDSEngine *engine, ObjectPtr object, unsigned ip) : _engine(engine), _parentScreen(engine->getCurrentScreenName()), _object(object), _ip(ip),
                                                                                       _status(kStatusActive), _exitCode(kExitCodeDestroy),
                                                                                       _tileWidth(16), _tileHeight(16), _tileResource(0), _tileIndex(0),
                                                                                       _timer(0),
                                                                                       _animationCycles(1), _animationLoop(false), _animationZ(0), _animationPaused(false), _animationSpeed(100)
                                                                                       {
}

void Process::debug(const char *str, ...) {
	va_list va;
	va_start(va, str);

	Common::String format = Common::String::format("%s %04x[%u]: %s", _object->getName().c_str(), _lastIp + 7, _stack.size(), str);
	Common::String buf = Common::String::vformat(format.c_str(), va);

	buf += '\n';

	if (g_system)
		g_system->logMessage(LogMessageType::kDebug, buf.c_str());

	va_end(va);
}

//fixme: remove copy-paste
void Process::error(const char *str, ...) {
	va_list va;
	va_start(va, str);

	Common::String format = Common::String::format("WARNING: %s %04x[%u]: %s", _object->getName().c_str(), _lastIp + 7, _stack.size(), str);
	Common::String buf = Common::String::vformat(format.c_str(), va);

	buf += '\n';

	if (g_system)
		g_system->logMessage(LogMessageType::kWarning, buf.c_str());

	va_end(va);
	_status = kStatusError;
}

void Process::push(int32 value) {
	_stack.push(value);
}

int32 Process::pop() {
	if (_stack.empty()) {
		error("stack underflow at %s:%04x", _object->getName().c_str(), _lastIp + 7);
		return 0;
	}
	return _stack.pop();
}

int32 Process::top() {
	if (_stack.empty()) {
		error("stack underflow, %s:%04x", _object->getName().c_str(), _lastIp + 7);
		return 0;
	}
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

Common::String Process::popText() {
	return _engine->loadText(popString());
}

void Process::activate() {
	switch(status()) {
	case kStatusPassive:
		_status = Process::kStatusActive;
		break;
	case kStatusDone:
	case kStatusError:
		debug("process %s finished", getName().c_str());
		break;
	default:
		break;
	}
}


void Process::run() {
	activate();
	while(status() == kStatusActive) {
		ProcessExitCode code = resume();
		switch (code) {
		case kExitCodeDestroy:
			debug("process %s returned destroy exit code", getName().c_str());
			//_engine->getCurrentScreen()->remove(_object); //remove if the last process exits
			done();
			break;
		case kExitCodeLoadScreenObjectAs:
		case kExitCodeLoadScreenObject:
			_engine->runObject(getExitArg1(), getExitArg2());
			activate();
			break;
		case kExitCodeRunDialog:
			_engine->runDialog(getExitArg1());
			break;
		case kExitCodeSetNextScreen:
			debug("process %s launches screen: %s", getName().c_str(), getExitArg1().c_str());
			_engine->setNextScreenName(getExitArg1(), false);
			break;
		case kExitCodeSetNextScreenSaveInHistory:
			_engine->setNextScreenName(getExitArg1(), true);
			break;
		case kExitCodeLoadPreviousScreenObject:
			_engine->returnToPreviousScreen();
			break;
		case kExitCodeMouseAreaChange:
			_engine->changeMouseArea(getExitIntArg1(), getExitIntArg2());
			activate();
			break;
		case kExitCodeLoadInventoryObject:
			_engine->inventory().add(_engine->loadObject(getExitArg1()));
			activate();
			break;
		case kExitCodeSuspend:
			break;
		case kExitCodeCreatePatchLoadResources:
			{
				debug("exitProcessCreatePatch");
				_engine->newGame();
			}
			break;
		case kExitCodeLoadSaveGame:
			if (_engine->loadGameState(getExitIntArg1()).getCode() == Common::kNoError) {
				done();
			} else {
				debug("save loading failed, resuming execution...");
				activate(); //continue
			}
			break;
		default:
			error("unknown process exit code %d", code);
		}
	}
}


} // namespace AGDS
