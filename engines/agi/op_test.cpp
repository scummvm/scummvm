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


#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/opcodes.h"
#include "agi/words.h"

#include "common/endian.h"

namespace Agi {

#define ip (state->_curLogic->cIP)
#define code (state->_curLogic->data)

void condEqual(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 varNr1 = p[0];
	uint16 varVal1 = vm->getVar(varNr1);
	uint16 value2 = p[1];
	state->testResult = (varVal1 == value2);
}

void condEqualV(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 varNr1 = p[0];
	uint16 varNr2 = p[1];
	uint16 varVal1 = vm->getVar(varNr1);
	uint16 varVal2 = vm->getVar(varNr2);
	state->testResult = (varVal1 == varVal2);
}

void condLess(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 varNr1 = p[0];
	uint16 varVal1 = vm->getVar(varNr1);
	uint16 value2 = p[1];
	state->testResult = (varVal1 < value2);
}

void condLessV(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 varNr1 = p[0];
	uint16 varNr2 = p[1];
	uint16 varVal1 = vm->getVar(varNr1);
	uint16 varVal2 = vm->getVar(varNr2);
	state->testResult = (varVal1 < varVal2);
}

void condGreater(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 varNr1 = p[0];
	uint16 varVal1 = vm->getVar(varNr1);
	uint16 value2 = p[1];
	state->testResult = (varVal1 > value2);
}

void condGreaterV(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 varNr1 = p[0];
	uint16 varNr2 = p[1];
	uint16 varVal1 = vm->getVar(varNr1);
	uint16 varVal2 = vm->getVar(varNr2);
	state->testResult = (varVal1 > varVal2);
}

void condIsSet(AgiGame *state, AgiEngine *vm, uint8 *p) {
	state->testResult = vm->getFlag(p[0]);
}

void condIsSetV(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 varNr = p[0];
	uint16 varVal = vm->getVar(varNr);
	state->testResult = vm->getFlag(varVal);
}

void condIsSetV1(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 varNr = p[0];
	uint16 varVal = vm->getVar(varNr);
	state->testResult = varVal > 0;
}

void condHas(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 objectNr = p[0];
	state->testResult = (vm->objectGetLocation(objectNr) == EGO_OWNED);
}

void condHasV1(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 objectNr = p[0];
	state->testResult = (vm->objectGetLocation(objectNr) == EGO_OWNED_V1);
}

void condObjInRoom(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 objectNr = p[0];
	uint16 varNr = p[1];
	uint16 varVal = vm->getVar(varNr);
	state->testResult = (vm->objectGetLocation(objectNr) == varVal);
}

void condPosn(AgiGame *state, AgiEngine *vm, uint8 *p) {
	state->testResult = vm->testPosn(p[0], p[1], p[2], p[3], p[4]);
}

void condController(AgiGame *state, AgiEngine *vm, uint8 *p) {
	state->testResult = vm->testController(p[0]);
}

void condHaveKey(AgiGame *state, AgiEngine *vm, uint8 *p) {
	// Only check for key when there is not already one set by scripts
	if (vm->getVar(VM_VAR_KEY)) {
		state->testResult = 1;
		return;
	}
	// we are not really an inner loop, but we stop processAGIEvents() from doing regular cycle work by setting it up
	vm->cycleInnerLoopActive(CYCLE_INNERLOOP_HAVEKEY);
	uint16 key = vm->processAGIEvents();
	vm->cycleInnerLoopInactive();
	if (key) {
		debugC(5, kDebugLevelScripts | kDebugLevelInput, "keypress = %02x", key);
		vm->setVar(VM_VAR_KEY, key);
		state->testResult = 1;
		return;
	}
	state->testResult = 0;
}

void condSaid(AgiGame *state, AgiEngine *vm, uint8 *p) {
	int ec = vm->testSaid(p[0], p + 1);
	state->testResult = ec;
}

void condSaid1(AgiGame *state, AgiEngine *vm, uint8 *p) {
	state->testResult = false;

	if (!vm->getFlag(VM_FLAG_ENTERED_CLI))
		return;

	int id0 = READ_LE_UINT16(p);

	if ((id0 == 1 || id0 == vm->_words->getEgoWordId(0)))
		state->testResult = true;
}

void condSaid2(AgiGame *state, AgiEngine *vm, uint8 *p) {
	state->testResult = false;

	if (!vm->getFlag(VM_FLAG_ENTERED_CLI))
		return;

	int id0 = READ_LE_UINT16(p);
	int id1 = READ_LE_UINT16(p + 2);

	if ((id0 == 1 || id0 == vm->_words->getEgoWordId(0)) &&
	        (id1 == 1 || id1 == vm->_words->getEgoWordId(1)))
		state->testResult = true;
}

void condSaid3(AgiGame *state, AgiEngine *vm, uint8 *p) {
	state->testResult = false;

	if (!vm->getFlag(VM_FLAG_ENTERED_CLI))
		return;

	int id0 = READ_LE_UINT16(p);
	int id1 = READ_LE_UINT16(p + 2);
	int id2 = READ_LE_UINT16(p + 4);

	if ((id0 == 1 || id0 == vm->_words->getEgoWordId(0)) &&
	        (id1 == 1 || id1 == vm->_words->getEgoWordId(1)) &&
	        (id2 == 1 || id2 == vm->_words->getEgoWordId(2)))
		state->testResult = true;
}

void condBit(AgiGame *state, AgiEngine *vm, uint8 *p) {
	uint16 value1 = p[0];
	uint16 varNr2 = p[1];
	uint16 varVal2 = vm->getVar(varNr2);
	state->testResult = (varVal2 >> value1) & 1;
}

void condCompareStrings(AgiGame *state, AgiEngine *vm, uint8 *p) {
	debugC(7, kDebugLevelScripts, "comparing [%s], [%s]", state->strings[p[0]], state->strings[p[1]]);
	state->testResult = vm->testCompareStrings(p[0], p[1]);
}

void condObjInBox(AgiGame *state, AgiEngine *vm, uint8 *p) {
	state->testResult = vm->testObjInBox(p[0], p[1], p[2], p[3], p[4]);
}

void condCenterPosn(AgiGame *state, AgiEngine *vm, uint8 *p) {
	state->testResult = vm->testObjCenter(p[0], p[1], p[2], p[3], p[4]);
}

void condRightPosn(AgiGame *state, AgiEngine *vm, uint8 *p) {
	state->testResult = vm->testObjRight(p[0], p[1], p[2], p[3], p[4]);
}

void condUnknown13(AgiGame *state, AgiEngine *vm, uint8 *p) {
	// My current theory is that this command checks whether the ego is currently moving
	// and that that movement has been caused using the mouse and not using the keyboard.
	// I base this theory on the game's behavior on an Amiga emulator, not on disassembly.
	// This command is used at least in the Amiga version of Gold Rush! v2.05 1989-03-09
	// (AGI 2.316) in logics 1, 3, 5, 6, 137 and 192 (Logic.192 revealed this command's nature).
	// TODO: Check this command's implementation using disassembly just to be sure.
	int ec = state->screenObjTable[SCREENOBJECTS_EGO_ENTRY].flags & fAdjEgoXY;
	debugC(7, kDebugLevelScripts, "op_test: in.motion.using.mouse = %s (Amiga-specific testcase 19)", ec ? "true" : "false");
	state->testResult = ec;
}

void condUnknown(AgiGame *state, AgiEngine *vm, uint8 *p) {
	warning("Skipping unknown test command %2X", *(code + ip - 1));
	state->testResult = false;
}

uint8 AgiEngine::testCompareStrings(uint8 s1, uint8 s2) {
	char ms1[MAX_STRINGLEN];
	char ms2[MAX_STRINGLEN];
	int j, k, l;

	Common::strlcpy(ms1, _game.strings[s1], MAX_STRINGLEN);
	Common::strlcpy(ms2, _game.strings[s2], MAX_STRINGLEN);

	l = strlen(ms1);
	for (k = 0, j = 0; k < l; k++) {
		switch (ms1[k]) {
		case 0x20:
		case 0x09:
		case '-':
		case '.':
		case ',':
		case ':':
		case ';':
		case '!':
		case '\'':
			break;

		default:
			ms1[j++] = tolower(ms1[k]);
			break;
		}
	}
	ms1[j] = 0x0;

	l = strlen(ms2);
	for (k = 0, j = 0; k < l; k++) {
		switch (ms2[k]) {
		case 0x20:
		case 0x09:
		case '-':
		case '.':
		case ',':
		case ':':
		case ';':
		case '!':
		case '\'':
			break;

		default:
			ms2[j++] = tolower(ms2[k]);
			break;
		}
	}
	ms2[j] = 0x0;

	return !strcmp(ms1, ms2);
}

uint8 AgiEngine::testController(uint8 cont) {
	return (_game.controllerOccured[cont] ? true : false);
}

uint8 AgiEngine::testPosn(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2) {
	ScreenObjEntry *v = &_game.screenObjTable[n];
	uint8 r;

	r = v->xPos >= x1 && v->yPos >= y1 && v->xPos <= x2 && v->yPos <= y2;

	debugC(7, kDebugLevelScripts, "(%d,%d) in (%d,%d,%d,%d): %s", v->xPos, v->yPos, x1, y1, x2, y2, r ? "true" : "false");

	return r;
}

uint8 AgiEngine::testObjInBox(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2) {
	ScreenObjEntry *v = &_game.screenObjTable[n];

	return v->xPos >= x1 &&
	       v->yPos >= y1 && v->xPos + v->xSize - 1 <= x2 && v->yPos <= y2;
}

// if n is in center of box
uint8 AgiEngine::testObjCenter(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2) {
	ScreenObjEntry *v = &_game.screenObjTable[n];

	return v->xPos + v->xSize / 2 >= x1 &&
	       v->xPos + v->xSize / 2 <= x2 && v->yPos >= y1 && v->yPos <= y2;
}

// if nect N is in right corner
uint8 AgiEngine::testObjRight(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2) {
	ScreenObjEntry *v = &_game.screenObjTable[n];

	return v->xPos + v->xSize - 1 >= x1 &&
	       v->xPos + v->xSize - 1 <= x2 && v->yPos >= y1 && v->yPos <= y2;
}

// When player has entered something, it is parsed elsewhere
uint8 AgiEngine::testSaid(uint8 nwords, uint8 *cc) {
	AgiGame *state = &_game;
	AgiEngine *vm = state->_vm;
	Words *words = vm->_words;
	int c, n = words->getEgoWordCount();
	int z = 0;

	if (vm->getFlag(VM_FLAG_SAID_ACCEPTED_INPUT) || !vm->getFlag(VM_FLAG_ENTERED_CLI))
		return false;

	// FR:
	// I think the reason for the code below is to add some speed....
	//
	//      if (nwords != num_ego_words)
	//              return false;
	//
	// In the disco scene in Larry 1 when you type "examine blonde",
	// inside the logic is expected ( said("examine", "blonde", "rol") )
	// where word("rol") = 9999
	//
	// According to the interpreter code 9999 means that whatever the
	// user typed should be correct, but it looks like code 9999 means that
	// if the string is empty at this point, the entry is also correct...
	//
	// With the removal of this code, the behavior of the scene was
	// corrected

	for (c = 0; nwords && n; c++, nwords--, n--) {
		z = READ_LE_UINT16(cc);
		cc += 2;

		switch (z) {
		case 9999:  // rest of line (empty string counts to...)
			nwords = 1;
			break;
		case 1: // any word
			break;
		default:
			if (words->getEgoWordId(c) != z)
				return false;
			break;
		}
	}

	// The entry string should be entirely parsed, or last word = 9999
	if (n && z != 9999)
		return false;

	// The interpreter string shouldn't be entirely parsed, but next
	// word must be 9999.
	if (nwords != 0 && READ_LE_UINT16(cc) != 9999)
		return false;

	setFlag(VM_FLAG_SAID_ACCEPTED_INPUT, true);

	return true;
}

bool AgiEngine::testIfCode(int16 logicNr) {
	AgiGame *state = &_game;
	uint8 op;
	uint8 p[16];

	int notMode = false;
	int orMode = false;
	int endTest = false;
	int result = true;

	while (!(shouldQuit() || _restartGame) && !endTest) {
		if (_debug.enabled && (_debug.logic0 || logicNr))
			debugConsole(logicNr, lTEST_MODE, NULL);

		op = *(code + ip++);
		memmove(p, (code + ip), 16);

		switch (op) {
		case 0xFC:
			if (orMode) {
				// We have reached the end of an OR expression without
				// a single test command evaluating as true. Thus the OR
				// expression evalutes as false which means the whole
				// expression evaluates as false. So skip until the
				// ending 0xFF and return.
				skipInstructionsUntil(0xFF);
				result = false;
				endTest = true;
			} else {
				orMode = true;
			}
			continue;
		case 0xFD:
			notMode = true;
			continue;
		case 0x00:
		case 0xFF:
			endTest = true;
			continue;

		default:
			// Evaluate the command and skip the rest of the instruction
			_opCodesCond[op].functionPtr(state, this, p);
			if (state->exitAllLogics) {
				// required even here, because of at least the timer heuristic
				// which when triggered waits a bit and processes ScummVM events and user may therefore restore a saved game
				// fixes bug #9707
				// TODO: maybe delay restoring the game instead, when GMM is used?
 				return true;
			}
			skipInstruction(op);

			// NOT mode is enabled only for one instruction
			if (notMode)
				state->testResult = !state->testResult;
			notMode = false;

			if (orMode) {
				if (state->testResult) {
					// We are in OR mode and the last test command evaluated
					// as true, thus the whole OR expression evaluates as
					// true. So skip the rest of the OR expression and
					// continue normally.
					skipInstructionsUntil(0xFC);
					orMode = false;
					continue;
				}
			} else {
				result &= state->testResult;
				if (!result) {
					// Since we are in AND mode and the last test command
					// evaluated as false, the whole expression also evaluates
					// as false. So skip until the ending 0xFF and return.
					skipInstructionsUntil(0xFF);
					endTest = true;
					continue;
				}
			}
			break;
		}
	}

	// Skip the following IF block if the condition evaluates as false
	if (result)
		ip += 2;
	else
		ip += READ_LE_UINT16(code + ip) + 2;

	if (_debug.enabled && (_debug.logic0 || logicNr))
		debugConsole(logicNr, 0xFF, result ? "=true" : "=false");

	return result;
}

void AgiEngine::skipInstruction(byte op) {
	AgiGame *state = &_game;
	if (op >= 0xFC)
		return;
	if (op == 0x0E && state->_vm->getVersion() >= 0x2000) // said
		ip += *(code + ip) * 2 + 1;
	else {
		ip += _opCodesCond[op].parameterSize;
	}
}

void AgiEngine::skipInstructionsUntil(byte v) {
	AgiGame *state = &_game;
	int originalIP = state->_curLogic->cIP;

	while (1) {
		byte op = *(code + ip++);
		if (op == v)
			return;

		if (op < 0xFC) {
			if (!_opCodesCond[op].functionPtr) {
				// security-check
				error("illegal opcode %x during skipinstructions in script %d at %d (triggered at %d)", op, state->curLogicNr, ip, originalIP);
			}
		}
		skipInstruction(op);
	}
}

} // End of namespace Agi
