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
#include "agds/opcode.h"
#include "agds/region.h"
#include "common/debug.h"

namespace AGDS {

void Process::enter(uint16 magic, uint16 size) {
	if (magic != 0xdead || size != 0x0c)
		error("invalid enter() magic: 0x%04x or size: %u", magic, size);
	uint16 unk1			= next16();
	uint16 unk2			= next16();
	uint16 unk3			= next16();
	unsigned resOffset	= next16();
	uint16 resCount		= next16();
	uint16 unk4			= next16();
	debug("resource block %04x %04x %04x %04x,"
		" resources table with %u entries", unk1, unk2, unk3, unk4, resCount);

	_object->readStringTable(resOffset, resCount);
}

void Process::setSystemVariable() {
	int16 valueIndex = pop();
	Common::String name = popString();

	if (valueIndex != -1) {
		Common::String value = getString(valueIndex);
		debug("setSystemVariable %s to %s", name.c_str(), value.c_str());
	} else {
		debug("resetSystemVariable %s", name.c_str());
	}
}

void Process::getIntegerSystemVariable() {
	Common::String name = popString();
	int value = 0;
	debug("getIntegerSystemVariable: %s -> %d", name.c_str(), value);
	push(value);
}

void Process::getRegionWidth() {
	Common::String name = popString();
	Region *reg = _engine->loadRegion(name);
	int value = reg->width;
	push(value);
	delete reg;
	debug("getRegionWidth %s -> %d", name.c_str(), value);
}

void Process::getRegionHeight() {
	Common::String name = popString();
	Region *reg = _engine->loadRegion(name);
	int value = reg->height;
	push(value);
	delete reg;
	debug("getRegionHeight %s -> %d", name.c_str(), value);
}


void Process::loadPicture() {
	Common::String name = popString();
	debug("loadPicture stub %s", name.c_str());
	push(100500); //dummy
}

void Process::loadAnimation() {
	Common::String name = popString();
	debug("loadAnimation %s", name.c_str());
}

void Process::loadSample() {
	Common::String name = popString();
	debug("loadSample %s", name.c_str());
}

void Process::loadScreenObject() {
	Common::String name = popString();
	debug("loadScreenObject: %s", name.c_str());
	_exitValue = name;
	suspend(kExitCodeLoadScreenObject);
}

void Process::removeScreenObject() {
	Common::String name = popString();
	debug("removeScreenObject: %s", name.c_str());
}

void Process::loadFont() {
	Common::String name = popString();
	int id = pop();
	debug("loadFont %s %d stub", name.c_str(), id);
}

void Process::loadMouse() {
	Common::String name = popString();
	debug("loadMouse %s", name.c_str());
}

void Process::setIntegerVariable() {
	int value = pop();
	Common::String name = popString();
	debug("setIntegerVariable stub: %s -> %d", name.c_str(), value);
}

void Process::setGlobal() {
	Common::String name = popString();
	int value = pop();
	debug("setting global %s -> %d", name.c_str(), value);
	_engine->setGlobal(name, value);
}

void Process::getGlobal(unsigned index) {
	const Common::String & name = _object->getString(index).string;
	int value = _engine->getGlobal(name);
	debug("get global %s -> %d", name.c_str(), value);
	push(value);
}

void Process::hasGlobal() {
	Common::String name = popString();
	int result = _engine->hasGlobal(name)? 1: 0;
	debug("hasGlobal %s %d", name.c_str(), result);
	push(result);
}

void Process::postIncrementGlobal() {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	debug("increment global %s %d", name.c_str(), value);
	push(value);
	_engine->setGlobal(name, value + 1);
}

void Process::incrementGlobal(int inc) {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	debug("increment global %s %d", name.c_str(), value);
	_engine->setGlobal(name, value + inc);
}

void Process::decrementGlobal(int dec) {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	debug("decrement global %s %d", name.c_str(), value);
	_engine->setGlobal(name, value - dec);
}

void Process::appendToSharedStorage() {
	Common::String value = popString();
	int index = _engine->appendToSharedStorage(value);
	debug("appendToSharedStorage %s -> %d", value.c_str(), index);
	push(index);
}

void Process::stub98() {
	debug("stub98");
}

void Process::changeScreenPatch() {
	Common::String res2 = popString();
	Common::String res1;
	int index = pop();
	if (index != -1)
		res1 = getString(index);
	//change screen patch
	debug("stub140: %d '%s' '%s'", index, res1.c_str(), res2.c_str());
	push(140);
}

void Process::loadMouseStub66() {
	Common::String name = popString();
	debug("loadMouseStub66 %s", name.c_str());
}

void Process::fadeObject() {
	int arg = pop();
	Common::String name = popString();
	debug("fadeObject %s %d", name.c_str(), arg);
}


void Process::stub128() {
	debug("processCleanupStub128");
}

void Process::stub129() {
	int value = pop();
	debug("stub129 %d", value);
}

void Process::stub130() {
	int value = pop();
	debug("stub130 %d", value);
}
void Process::stub133() {
	int pan = pop();
	int volume = pop();
	debug("stub133: pan? %d volume? %d", pan, volume);
}

void Process::stub134() {
	int arg2 = pop();
	int arg1 = pop();
	debug("stub134, font related %d %d", arg1, arg2);
}

void Process::stub136() {
	debug("stub136 sets value of stub130 to 1000000000");
}

void Process::stub182() {
	int arg2 = pop();
	int arg1 = pop();
	debug("stub182 %d %d", arg1, arg2);
}

void Process::stub188() {
	int arg3 = pop();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("stub188 %s %s %d", arg1.c_str(), arg2.c_str(), arg3);
}


void Process::exitProcess() {
	debug("exit");
	_status = kStatusDone;
	_exitCode = kExitCodeDestroy;
}

void Process::clearScreen() {
	debug("clearScreen");
}

void Process::stub165() {
	int arg3 = pop();
	int arg2 = pop();
	Common::String arg1 = popString();
	debug("stub165 %s %d %d", arg1.c_str(), arg2, arg3);
	_engine->loadObject(arg1);
}

void Process::stub190() {
	int value = pop();
	debug("stub190 %d", value);
}

void Process::stub195() {
	Common::String value = popString();
	debug("stub195(getPictureWidth) %s", value.c_str());
	_engine->loadObject(value);
	push(195);
}

void Process::stub196() {
	Common::String value = popString();
	debug("stub196(getPictureHeight) %s", value.c_str());
	_engine->loadObject(value);
	push(196);
}

void Process::stub202(unsigned size) {
	debug("stub203, %u instructions", size);
	_ip += size;
}

void Process::stub203() {
	//arg3 is optional and stored in process struct, offset 0x210
	Common::String audio = popString();
	Common::String video = popString();

	debug("playFilm %s %s", video.c_str(), audio.c_str());
}

void Process::stub206() {
	int arg2 = pop();
	int arg1 = pop();
	debug("stub206 (mouse?) %d %d", arg1, arg2);
}

void Process::exitProcessSetNextScreen() {
	_exitValue = popString();
	_exitCode = kExitCodeDestroyProcessSetNextScreen;
	_status = kStatusPassive;
}

void Process::exitScreen()
{
	debug("exitScreen? reactivating process...");
	if (_status != kStatusDone)
		_status = kStatusActive;
}

void Process::setScreenHeight() {
	int height = pop();
	debug("setScreenHeight %d", height);
}

void Process::updateScreenHeightToDisplay() {
	debug("updateScreenHeightToDisplay");
}


void Process::suspendProcess() {
	debug("suspendProcess");
	_status = kStatusPassive;
	_exitCode = kExitCodeSuspend;
}

void Process::call(uint16 addr) {
	debug("call %04x", addr);
	//original engine just create new process, save exit code in screen object
	//and on stack, then just ignore return code, fixme?
	Process callee(_engine, _object, addr);
	ProcessExitCode code = callee.execute();
	debug("call returned %d", code);
}

void Process::onKey(unsigned size) {
	Common::String key = popString();
	debug("onKey %s handler, %u instructions", key.c_str(), size);
	_ip += size;
}

void Process::onUse(unsigned size) {
	debug("use? handler, %u instructions", size);
	_ip += size;
}


void Process::enableUser() {
	//screen loading block user interaction until this instruction
	debug("enableUser");
}

void Process::findObjectInMouseArea() {
	Common::String arg3 = popString();
	Common::String arg2 = popString();
	Common::String arg1 = popString();

	debug("findObjectInMouseArea %s %s %s", arg1.c_str(), arg2.c_str(), arg3.c_str());
	Region *reg = _engine->loadRegion(arg1);
	//_engine->loadObject(arg2);
	//_engine->loadObject(arg3);
	delete reg;
	push(205);
}

void Process::loadRegionFromObject() {
	Common::String name = popString();
	debug("loadRegionFromObject %s", name.c_str());
	Region *reg = _engine->loadRegion(name);
	delete reg;
}


void Process::loadPictureFromObject() {
	Common::String name = popString();
	debug("loadPictureFromObject %s", name.c_str());
}

void Process::loadAnimationFromObject() {
	Common::String name = popString();
	debug("loadAnimationFromObject %s", name.c_str());
}

void Process::setCounter() {
	int value = pop();
	debug("setCounter127 %d", value);
}



//fixme: add trace here
#define OP(NAME, METHOD) \
	case NAME: METHOD (); break

#define OP_C(NAME, METHOD) \
	case NAME: { int8 arg = next(); METHOD (arg); } break

#define OP_B(NAME, METHOD) \
	case NAME: { uint8 arg = next(); METHOD (arg); } break

#define OP_W(NAME, METHOD) \
	case NAME: { int16 arg = next16(); METHOD (arg); } break

#define OP_U(NAME, METHOD) \
	case NAME: { uint16 arg = next16(); METHOD (arg); } break

#define OP_UU(NAME, METHOD) \
	case NAME: { uint16 arg1 = next16(); uint16 arg2 = next16(); METHOD (arg1, arg2); } break

ProcessExitCode Process::execute() {
	_exitCode = kExitCodeDestroy;

	const Object::CodeType &code = _object->getCode();
	while(_status == kStatusActive && _ip < code.size()) {
		uint8 op = next();
		switch(op) {
			OP_UU	(kEnter, enter);
			OP_W	(kJumpZImm16, jumpz);
			OP_W	(kJumpImm16, jump);
			OP		(kPop, pop);
			OP		(kExitProcess, exitProcess);
			OP		(kSuspendProcess, suspendProcess);
			OP_C	(kPushImm8, push);
			OP_W	(kPushImm16, push);
			OP_C	(kPushImm8_2, push);
			OP_W	(kPushImm16_2, push);
			OP_B	(kGetGlobalImm8, getGlobal);
			OP		(kPostIncrementGlobal, postIncrementGlobal);
			OP		(kIncrementGlobalByTop, incrementGlobalByTop);
			OP		(kDecrementGlobalByTop, decrementGlobalByTop);
			OP		(kEquals, equals);
			OP		(kNotEquals, notEquals);
			OP		(kGreater, greater);
			OP		(kLess, less);
			OP		(kGreaterOrEquals, greaterOrEquals);
			OP		(kLessOrEquals, lessOrEquals);
			OP		(kAdd, add);
			OP		(kSub, sub);
			OP		(kMul, mul);
			OP		(kDiv, div);
			OP		(kSetGlobal, setGlobal);
			OP		(kBoolOr, boolOr);
			OP		(kBoolAnd, boolAnd);
			OP		(kNot, bitNot);
			OP		(kBoolNot, boolNot);
			OP_U	(kCallImm16, call);
			OP_U	(kObjectRegisterUseHandler, onUse);
			OP		(kStub66, loadMouseStub66);
			OP		(kLoadRegionFromObject, loadRegionFromObject);
			OP		(kLoadPictureFromObject, loadPictureFromObject);
			OP		(kLoadAnimationFromObject, loadAnimationFromObject);
			OP		(kStub98, stub98);
			OP		(kEnableUser, enableUser);
			OP		(kClearScreen, clearScreen);
			OP		(kLoadMouse, loadMouse);
			OP		(kSetScreenHeight, setScreenHeight);
			OP		(kUpdateScreenHeightToDisplay, updateScreenHeightToDisplay);
			OP		(kScreenLoadObject, loadScreenObject);
			OP		(kExitProcessSetNextScreen, exitProcessSetNextScreen);
			OP		(kScreenRemoveObject, removeScreenObject);
			OP		(kLoadAnimation, loadAnimation);
			OP		(kLoadSample, loadSample);
			OP		(kSetCounter, setCounter);
			OP		(kProcessCleanupStub128, stub128);
			OP		(kStub129, stub129);
			OP		(kStub130, stub130);
			OP		(kStub133, stub133);
			OP		(kStub134, stub134);
			OP		(kStub136, stub136);
			OP		(kScreenChangeScreenPatch, changeScreenPatch);
			OP		(kSetSystemVariable, setSystemVariable);
			OP		(kSetIntegerVariable, setIntegerVariable);
			OP		(kGetRegionWidth, getRegionWidth);
			OP		(kGetRegionHeight, getRegionHeight);
			OP		(kGetIntegerSystemVariable, getIntegerSystemVariable);
			OP		(kAppendToSharedStorage, appendToSharedStorage);
			OP		(kExitScreen, exitScreen);
			OP		(kStub165, stub165);
			OP		(kStub182, stub182);
			OP		(kStub188, stub188);
			OP		(kStub190, stub190);
			OP		(kStub195, stub195);
			OP		(kStub196, stub196);
			OP		(kLoadPicture, loadPicture);
			OP		(kFadeObject, fadeObject);
			OP		(kLoadFont, loadFont);
			OP_U	(kStub202ScreenHandler, stub202);
			OP		(kPlayFilm, stub203);
			OP		(kFindObjectInMouseArea, findObjectInMouseArea);
			OP		(kStub206, stub206);
			OP_U	(kOnKey, onKey);
			OP		(kHasGlobal, hasGlobal);
		default:
			error("%s: %08x: unknown opcode 0x%02x (%u)", _object->getName().c_str(), _ip - 1, (unsigned)op, (unsigned)op);
			_status = kStatusError;
			break;
		}
	}

	if (_status == kStatusActive) {
		debug("code ended, exiting...");
	}

	return _exitCode;
}

}
