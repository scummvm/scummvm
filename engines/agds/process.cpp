/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "agds/process.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "common/debug.h"
#include "common/system.h"

namespace AGDS {

Process::Process(AGDSEngine *engine, const ObjectPtr &object, unsigned ip, bool v2) : _engine(engine), _parentScreen(engine->getCurrentScreenName()), _object(object),
																					  _entryPoint(ip), _ip(ip), _lastIp(ip),
																					  _status(kStatusActive), _exited(false), _exitCode(kExitCodeDestroy),
																					  _tileWidth(16), _tileHeight(16), _tileResource(-1), _tileIndex(0),
																					  _timer(0),
																					  _animationCycles(1), _animationLoop(false), _animationZ(0), _animationDelay(-1), _animationRandom(0),
																					  _phaseVarControlled(false), _animationSpeed(100),
																					  _samplePeriodic(false), _sampleAmbient(false), _sampleVolume(100),
																					  _filmSubtitlesResource(-1), _v2(v2) {
	updateWithCurrentMousePosition();
}

void Process::debug(const char *str, ...) {
	va_list va;
	va_start(va, str);

	Common::String format = Common::String::format("%s %04x[%u]: %s", _object->getName().c_str(), _lastIp, _stack.size(), str);
	Common::String buf = Common::String::vformat(format.c_str(), va);

	buf += '\n';

	if (g_system)
		g_system->logMessage(LogMessageType::kDebug, buf.c_str());

	va_end(va);
}

// fixme: remove copy-paste
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

void Process::suspend(ProcessExitCode exitCode, const Common::String &arg1, const Common::String &arg2) {
	debug("suspend %d", exitCode);
	_exited = true;
	_exitCode = exitCode;
	_exitIntArg1 = 0;
	_exitIntArg2 = 0;
	_exitArg1 = arg1;
	_exitArg2 = arg2;
}

void Process::suspend(ProcessExitCode exitCode, int arg1, int arg2) {
	debug("suspend %d", exitCode);
	_exited = true;
	_exitCode = exitCode;
	_exitIntArg1 = arg1;
	_exitIntArg2 = arg2;
	_exitArg1.clear();
	_exitArg2.clear();
}

uint8 Process::next() {
	const Object::CodeType &code = _object->getCode();
	if (_ip < code.size()) {
		return code[_ip++];
	} else {
		_status = kStatusError;
		return 0;
	}
}

void Process::jump(int16 delta) {
	_ip += delta;
}

void Process::jumpz(int16 delta) {
	int value = pop();
	if (value == 0) {
		debug("jumpz %d %+d", value, delta);
		_ip += delta;
	} else
		debug("jumpz ignored %d %+d", value, delta);
}

void Process::suspend() {
	suspend(kExitCodeSuspend);
}

void Process::suspendIfPassive() {
	if (passive())
		suspend();
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

uint32 Process::popColor() {
	int b = pop();
	int g = pop();
	int r = pop();
	return _engine->pixelFormat().RGBToColor(r, g, b);
}

void Process::updateWithCurrentMousePosition() {
	setMousePosition(_engine->mousePosition());
}

void Process::setupAnimation(const AnimationPtr &animation) {
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

void Process::removeProcessAnimation() {
	if (_processAnimation) {
		auto *screen = _engine->getCurrentScreen();
		if (screen)
			screen->remove(_processAnimation);
		_processAnimation = nullptr;
	}
}

void Process::activate() {
	switch (status()) {
	case kStatusPassive:
		_status = Process::kStatusActive;
		break;
	case kStatusDone:
	case kStatusError:
		debug("finished");
		break;
	default:
		break;
	}
}

void Process::deactivate() {
	switch (status()) {
	case kStatusActive:
		debug("deactivated");
		_status = Process::kStatusPassive;
		break;
	case kStatusDone:
	case kStatusError:
		break;
	default:
		break;
	}
}

void Process::done() {
	if (_status != kStatusDone) {
		debug("done");
		_status = kStatusDone;
		if (!_stack.empty())
			warning("process %s finished with non-empty stack", getName().c_str());
	}
}

void Process::fail() {
	_status = kStatusError;
}

void Process::run() {
	bool restart = true;
	while (_status != kStatusDone && _status != kStatusError && restart) {
		restart = false;
		ProcessExitCode code = resume();
		switch (code) {
		case kExitCodeDestroy:
			debug("process %s returned destroy exit code", getName().c_str());
			done();
			if (!getObject()->alive() && !_engine->hasActiveProcesses(getName())) {
				removeScreenObject(getName());
			}
			break;
		case kExitCodeLoadScreenObjectAs:
		case kExitCodeLoadScreenObject:
			deactivate();
			_object->lock();
			_engine->runObject(getExitArg1(), getExitArg2());
			_object->unlock();
			activate();
			restart = true;
			break;
		case kExitCodeRunDialog:
			deactivate();
			_object->lock();
			_engine->runObject(getExitArg1());
			_object->unlock();
			break;
		case kExitCodeSetNextScreen:
		case kExitCodeSetNextScreenSaveOrLoad:
			done();
			debug("process %s launches screen: %s, saveorload: ", getName().c_str(), getExitArg1().c_str(), code == kExitCodeSetNextScreenSaveOrLoad);
			_engine->setNextScreenName(getExitArg1(), code == kExitCodeSetNextScreenSaveOrLoad ? ScreenLoadingType::SaveOrLoad : ScreenLoadingType::Normal);
			break;
		case kExitCodeMouseAreaChange:
			deactivate();
			_object->lock();
			_engine->changeMouseArea(getExitIntArg1(), getExitIntArg2());
			_object->unlock();
			activate();
			restart = true;
			break;
		case kExitCodeLoadInventoryObject:
			deactivate();
			_object->lock();
			_engine->inventory().add(getExitArg1());
			_object->unlock();
			activate();
			restart = true;
			break;
		case kExitCodeCloseInventory:
			_object->lock();
			_engine->inventory().enable(false);
			updateWithCurrentMousePosition();
			_object->unlock();
			break;
		case kExitCodeSuspend:
			updateWithCurrentMousePosition();
			break;
		case kExitCodeNewGame:
			deactivate();
			_object->lock();
			debug("exitProcessNewGame");
			_engine->newGame();
			_object->unlock();
			activate();
			restart = true;
			break;
		case kExitCodeLoadGame:
			deactivate();
			_object->lock();
			if (_engine->loadGameState(getExitIntArg1()).getCode() == Common::kNoError) {
				done();
			} else {
				debug("save loading failed, resuming execution...");
			}
			_object->unlock();
			activate();
			restart = true;
			break;
		case kExitCodeSaveGame:
			break;
		default:
			error("unknown process exit code %d", code);
		}
	}
}

#define UNARY_OP(NAME, OP)       \
	void Process::NAME() {       \
		int32 arg = pop();         \
		debug(#NAME " %d", arg); \
		push(OP arg);            \
	}
#define BINARY_OP(NAME, OP)                    \
	void Process::NAME() {                     \
		int32 arg2 = pop();                      \
		int32 arg1 = pop();                      \
		debug(" %d " #NAME " %d", arg1, arg2); \
		push(arg1 OP arg2);                    \
	}

UNARY_OP(boolNot, !)
UNARY_OP(bitNot, ~)
UNARY_OP(negate, -)
BINARY_OP(shl, <<)
BINARY_OP(shr, >>)
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

// fixme: add trace here
#define AGDS_OP(NAME, METHOD) \
	case NAME:                \
		METHOD();             \
		break;

#define AGDS_OP_C(NAME, METHOD) \
	case NAME: {                \
		int8 arg = next();      \
		METHOD(arg);            \
	} break;

#define AGDS_OP_B(NAME, METHOD) \
	case NAME: {                \
		uint8 arg = next();     \
		METHOD(arg);            \
	} break;

#define AGDS_OP_W(NAME, METHOD) \
	case NAME: {                \
		int16 arg = next16();   \
		METHOD(arg);            \
	} break;

#define AGDS_OP_U(NAME, METHOD) \
	case NAME: {                \
		uint16 arg = next16();  \
		METHOD(arg);            \
	} break;

#define AGDS_OP_UU(NAME, METHOD) \
	case NAME: {                 \
		uint16 arg1 = next16();  \
		uint16 arg2 = next16();  \
		METHOD(arg1, arg2);      \
	} break;

#define AGDS_OP_UD(NAME, METHOD)                         \
	case NAME: {                                         \
		uint16 arg1 = next16();                          \
		uint32 arg2 = next16();                          \
		METHOD(static_cast<int32>(arg1 | (arg2 << 16))); \
	} break;

ProcessExitCode Process::resume() {
	_exitCode = kExitCodeDestroy;
	if (_timer) {
		--_timer;
		return kExitCodeSuspend;
	}

	const Object::CodeType &code = _object->getCode();
	while (!_exited && _ip < code.size()) {
		if (_timer) {
			return kExitCodeSuspend;
		}
		_lastIp = _ip;
		uint16 op = next();
		if (_v2) {
			if (op & 1) {
				op |= next() << 8;
				op >>= 1;
			}
			op -= 7995;
		}
		// debug("CODE %04x: %u", _lastIp, (uint)op);
		switch (op) {
			AGDS_OPCODE_LIST(
				AGDS_OP,
				AGDS_OP_C, AGDS_OP_B,
				AGDS_OP_W, AGDS_OP_U,
				AGDS_OP_UD, AGDS_OP_UU)
		default:
			error("%s: %08x: unknown opcode 0x%02x (%u)", _object->getName().c_str(), _lastIp, (unsigned)op, (unsigned)op);
			fail();
			break;
		}
	}
	_exited = false;

	return _exitCode;
}

#define AGDS_DIS(NAME, METHOD)                           \
	case NAME:                                           \
		source += Common::String::format("%s\n", #NAME); \
		break;

#define AGDS_DIS_C(NAME, METHOD)                                      \
	case NAME: {                                                      \
		int8 arg = code[ip++];                                        \
		source += Common::String::format("%s %d\n", #NAME, (int)arg); \
	} break;

#define AGDS_DIS_B(NAME, METHOD)                                       \
	case NAME: {                                                       \
		uint8 arg = code[ip++];                                        \
		source += Common::String::format("%s %u\n", #NAME, (uint)arg); \
	} break;

#define AGDS_DIS_W(NAME, METHOD)                                      \
	case NAME: {                                                      \
		int16 arg = code[ip++];                                       \
		arg |= ((int16)code[ip++]) << 8;                              \
		source += Common::String::format("%s %d\n", #NAME, (int)arg); \
	} break;

#define AGDS_DIS_U(NAME, METHOD)                                       \
	case NAME: {                                                       \
		uint16 arg = code[ip++];                                       \
		arg |= ((uint16)code[ip++]) << 8;                              \
		source += Common::String::format("%s %u\n", #NAME, (uint)arg); \
	} break;

#define AGDS_DIS_UU(NAME, METHOD)                                                      \
	case NAME: {                                                                       \
		uint16 arg1 = code[ip++];                                                      \
		arg1 |= ((uint16)code[ip++]) << 8;                                             \
		uint16 arg2 = code[ip++];                                                      \
		arg2 |= ((uint16)code[ip++]) << 8;                                             \
		source += Common::String::format("%s %u %u\n", #NAME, (uint)arg1, (uint)arg2); \
	} break;

#define AGDS_DIS_UD(NAME, METHOD)                                                        \
	case NAME: {                                                                         \
		uint16 arg1 = code[ip++];                                                        \
		arg1 |= ((uint16)code[ip++]) << 8;                                               \
		uint16 arg2 = code[ip++];                                                        \
		arg2 |= ((uint16)code[ip++]) << 8;                                               \
		source += Common::String::format("%s %u\n", #NAME, (uint)(arg1 | (arg2 << 16))); \
	} break;

Common::String Process::disassemble(const ObjectPtr &object, bool v2) {
	Common::String source = Common::String::format("Object %s disassembly:\n", object->getName().c_str());

	const auto &code = object->getCode();
	uint ip = 0;
	while (ip < code.size()) {
		uint16 op = code[ip++];
		if (v2) {
			if (op & 1) {
				op |= code[ip++] << 8;
				op >>= 1;
			}
		}

		source += Common::String::format("%04x: %02x: ", ip - 1, op);
		switch (op) {
			AGDS_OPCODE_LIST(
				AGDS_DIS,
				AGDS_DIS_C, AGDS_DIS_B,
				AGDS_DIS_W, AGDS_DIS_U,
				AGDS_DIS_UD, AGDS_DIS_UU)
		default:
			source += Common::String::format("unknown opcode 0x%02x (%u)\n", (unsigned)op, (unsigned)op);
			break;
		}
	}
	source += Common::String::format("Object %s disassembly end\n", object->getName().c_str());
	return source;
}

} // namespace AGDS
