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

void Process::loadPicture() {
	Common::String name = popString();
	debug("loadPicture stub %s", name.c_str());
	push(100500); //dummy
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
	push(0);
}

void Process::stub128() {
	debug("processCleanupStub128");
}

void Process::stub182() {
	int arg2 = pop();
	int arg1 = pop();
	debug("stub182 %d %d", arg1, arg2);
}

void Process::exitProcess() {
	debug("exit");
	_status = kStatusDone;
	_exitCode = kExitCodeDestroy;
}

void Process::clearScreen() {
	debug("clearScreen");
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


void Process::onKey(unsigned size) {
	Common::String key = popString();
	debug("onKey %s handler, %u instructions", key.c_str(), size);
	_ip += size;
}

void Process::enableUser() {
	//screen loading block user interaction until this instruction
	debug("enableUser");
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
			OP		(kPop, pop);
			OP		(kExitProcess, exitProcess);
			OP_C	(kPushImm8, push);
			OP_W	(kPushImm16, push);
			OP_C	(kPushImm8_2, push);
			OP_W	(kPushImm16_2, push);
			OP_B	(kGetGlobalImm8, getGlobal);
			OP		(kSetGlobal, setGlobal);
			OP		(kStub98, stub98);
			OP		(kEnableUser, enableUser);
			OP		(kClearScreen, clearScreen);
			OP		(kLoadMouse, loadMouse);
			OP		(kScreenLoadObject, loadScreenObject);
			OP		(kScreenRemoveObject, removeScreenObject);
			OP		(kScreenChangeScreenPatch, changeScreenPatch);
			OP		(kSetSystemVariable, setSystemVariable);
			OP		(kSetIntegerVariable, setIntegerVariable);
			OP		(kAppendToSharedStorage, appendToSharedStorage);
			OP		(kLoadPicture, loadPicture);
			OP		(kProcessCleanupStub128, stub128);
			OP		(kStub182, stub182);
			OP		(kLoadFont, loadFont);
			OP_U	(kStub202ScreenHandler, stub202);
			OP		(kPlayFilm, stub203);
			OP_U	(kOnKey, onKey);
		default:
			debug("%s: %08x: unknown opcode 0x%02x (%u)", _object->getName().c_str(), _ip - 1, (unsigned)op, (unsigned)op);
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
