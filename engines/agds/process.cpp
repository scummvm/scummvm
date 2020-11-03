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
#include "agds/animation.h"
#include "common/debug.h"
#include "common/system.h"

namespace AGDS {

Process::Process(AGDSEngine *engine, ObjectPtr object, unsigned ip) : _engine(engine), _parentScreen(engine->getCurrentScreenName()), _object(object), _ip(ip),
                                                                                       _status(kStatusActive), _exitCode(kExitCodeDestroy),
                                                                                       _tileWidth(16), _tileHeight(16), _tileResource(0), _tileIndex(0),
                                                                                       _timer(0),
                                                                                       _animationCycles(1), _animationLoop(false), _animationZ(0), _animationDelay(-1), _animationRandom(0),
																					   _phaseVarControlled(false), _animationSpeed(100), _samplePeriodic(false)
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

void Process::jump(int16 delta) {
	debug("jump %+d", delta);
	_ip += delta;
}

void Process::jumpz(int16 delta) {
	int value = pop();
	if (value == 0) {
		debug("jumpz %d %+d", value, delta);
		_ip += delta;
	}
}

void Process::incrementGlobalByTop() {
	incrementGlobal(top());
}
void Process::decrementGlobalByTop() {
	decrementGlobal(top());
}


void Process::suspend() {
	suspend(kExitCodeSuspend);
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

void Process::updateWithCurrentMousePosition() {
	setMousePosition(_engine->mousePosition());
}


void Process::setupAnimation(Animation *animation) {
	animation->position(_animationPosition);
	animation->z(_animationZ);
	animation->process(getName());
	animation->phaseVar(_phaseVar);
	animation->loop(_animationLoop);
	animation->cycles(_animationCycles);
	animation->delay(_animationDelay);
	animation->setRandom(_animationRandom);
	animation->phaseVarControlled(_phaseVarControlled);
	if (_phaseVar.empty())
			suspend();
	else if (_phaseVarControlled)
			_engine->setGlobal(_phaseVar, 0);
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
			continue;
		case kExitCodeRunDialog:
			_engine->runDialog(getExitArg1());
			break;
		case kExitCodeSetNextScreen:
			debug("process %s launches screen: %s", getName().c_str(), getExitArg1().c_str());
			_engine->setNextScreenName(getExitArg1(), false);
			done();
			break;
		case kExitCodeSetNextScreenSaveInHistory:
			_engine->setNextScreenName(getExitArg1(), true);
			done();
			break;
		case kExitCodeLoadPreviousScreenObject:
			_engine->returnToPreviousScreen();
			break;
		case kExitCodeMouseAreaChange:
			_engine->changeMouseArea(getExitIntArg1(), getExitIntArg2());
			activate();
			continue;
		case kExitCodeLoadInventoryObject:
			_engine->inventory().add(_engine->loadObject(getExitArg1()));
			activate();
			continue;
		case kExitCodeCloseInventory:
			_engine->inventory().enable(false);
			updateWithCurrentMousePosition();
			activate();
			return; //some codes are special, they needed to exit loop and keep process active
		case kExitCodeSuspend:
			updateWithCurrentMousePosition();
			activate();
			return; //some codes are special, they needed to exit loop and keep process active
		case kExitCodeCreatePatchLoadResources:
			{
				debug("exitProcessCreatePatch");
				_engine->newGame();
				activate();
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

#define UNARY_OP(NAME, OP) void Process:: NAME () { int arg = pop(); debug(#NAME " %d", arg); push( OP arg ); }
#define BINARY_OP(NAME, OP) void Process:: NAME () { int arg2 = pop(); int arg1 = pop(); debug(#NAME " %d " #OP " %d", arg1, arg2); push(arg1 OP arg2); }

	UNARY_OP(boolNot, !)
	UNARY_OP(bitNot, ~)
	UNARY_OP(negate, -)
	BINARY_OP(boolOr, ||)
	BINARY_OP(boolAnd, &&)
	BINARY_OP(equals, ==)
	BINARY_OP(notEquals, !=)
	BINARY_OP(greater, >)
	BINARY_OP(greaterOrEquals, >=)
	BINARY_OP(less, <)
	BINARY_OP(lessOrEquals, <=)
	BINARY_OP(add, +)
	BINARY_OP(sub, -)
	BINARY_OP(mul, *)
	BINARY_OP(div, /)
	BINARY_OP(mod, %)
	BINARY_OP(bitAnd, &)
	BINARY_OP(bitOr, |)
	BINARY_OP(bitXor, ^)

#undef UNARY_OP
#undef BINARY_OP


} // namespace AGDS
