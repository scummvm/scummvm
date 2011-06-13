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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#include "agi/agi.h"
#include "agi/opcodes.h"

namespace Agi {

#define ip (state->_curLogic->cIP)
#define code (state->_curLogic->data)

#define getvar(a) state->_vm->getvar(a)
#define getflag(a) state->_vm->getflag(a)

#define testEqual(v1, v2)		(getvar(v1) == (v2))
#define testLess(v1, v2)		(getvar(v1) < (v2))
#define testGreater(v1, v2)	(getvar(v1) > (v2))
#define testIsSet(flag)		(getflag(flag))
#define testHas(obj)			(state->_vm->objectGetLocation(obj) == EGO_OWNED)
#define testObjInRoom(obj, v)	(state->_vm->objectGetLocation(obj) == getvar(v))

void cond_end(AgiGame *state, uint8 *p) {
	state->endTest = true;
	state->ec = true;
}

void cond_equal(AgiGame *state, uint8 *p) {
	if (p[0] == 11)
		state->_vm->_timerHack++;
	ip += 2;
	state->ec = testEqual(p[0], p[1]);
}

void cond_equalv(AgiGame *state, uint8 *p) {
	if (p[0] == 11 || p[1] == 11)
		state->_vm->_timerHack++;
	ip += 2;
	state->ec = testEqual(p[0], getvar(p[1]));
}

void cond_less(AgiGame *state, uint8 *p) {
	if (p[0] == 11)
		state->_vm->_timerHack++;
	ip += 2;
	state->ec = testLess(p[0], p[1]);
}

void cond_lessv(AgiGame *state, uint8 *p) {
	if (p[0] == 11 || p[1] == 11)
		state->_vm->_timerHack++;
	ip += 2;
	state->ec = testLess(p[0], getvar(p[1]));
}

void cond_greater(AgiGame *state, uint8 *p) {
	if (p[0] == 11)
		state->_vm->_timerHack++;
	ip += 2;
	state->ec = testGreater(p[0], p[1]);
}

void cond_greaterv(AgiGame *state, uint8 *p) {
	if (p[0] == 11 || p[1] == 11)
		state->_vm->_timerHack++;
	ip += 2;
	state->ec = testGreater(p[0], getvar(p[1]));
}

void cond_isset(AgiGame *state, uint8 *p) {
	ip += 1;
	state->ec = testIsSet(p[0]);
}

void cond_issetv(AgiGame *state, uint8 *p) {
	ip += 1;
	state->ec = testIsSet(getvar(p[1]));
}

void cond_has(AgiGame *state, uint8 *p) {
	ip += 1;
	state->ec = testHas(p[0]);
}

void cond_obj_in_room(AgiGame *state, uint8 *p) {
	ip += 2;
	state->ec = testObjInRoom(p[0], p[1]);
}

void cond_posn(AgiGame *state, uint8 *p) {
	ip += 5;
	state->ec = state->_vm->testPosn(p[0], p[1], p[2], p[3], p[4]);
}

void cond_controller(AgiGame *state, uint8 *p) {
	ip += 1;
	state->ec = state->_vm->testController(p[0]);
}

void cond_have_key(AgiGame *state, uint8 *p) {
	state->ec = state->_vm->testKeypressed();
}

void cond_said(AgiGame *state, uint8 *p) {
	int ec = state->_vm->testSaid(p[0], p + 1);
	ip += p[0] * 2;	// skip num_words * 2
	ip++;	// skip num_words opcode
	state->ec = ec;
}

void cond_compare_strings(AgiGame *state, uint8 *p) {
	debugC(7, kDebugLevelScripts, "comparing [%s], [%s]", state->strings[p[0]], state->strings[p[1]]);
	ip += 2;
	state->ec = state->_vm->testCompareStrings(p[0], p[1]);
}

void cond_obj_in_box(AgiGame *state, uint8 *p) {
	ip += 5;
	state->ec = state->_vm->testObjInBox(p[0], p[1], p[2], p[3], p[4]);
}

void cond_center_posn(AgiGame *state, uint8 *p) {
	ip += 5;
	state->ec = state->_vm->testObjCenter(p[0], p[1], p[2], p[3], p[4]);
}

void cond_right_posn(AgiGame *state, uint8 *p) {
	ip += 5;
	state->ec = state->_vm->testObjRight(p[0], p[1], p[2], p[3], p[4]);
}

void cond_unknown_13(AgiGame *state, uint8 *p) {
	// My current theory is that this command checks whether the ego is currently moving
	// and that that movement has been caused using the mouse and not using the keyboard.
	// I base this theory on the game's behavior on an Amiga emulator, not on disassembly.
	// This command is used at least in the Amiga version of Gold Rush! v2.05 1989-03-09
	// (AGI 2.316) in logics 1, 3, 5, 6, 137 and 192 (Logic.192 revealed this command's nature).
	// TODO: Check this command's implementation using disassembly just to be sure.
	int ec = state->viewTable[0].flags & ADJ_EGO_XY;
	debugC(7, kDebugLevelScripts, "op_test: in.motion.using.mouse = %s (Amiga-specific testcase 19)", ec ? "true" : "false");
	state->ec = ec;
}

void cond_unknown(AgiGame *state, uint8 *p) {
	state->endTest = true;
}

void cond_not(AgiGame *state, uint8 *p) {
	state->notTest = !state->notTest;
}

void cond_or(AgiGame *state, uint8 *p) {
	// if or_test is ON and we hit 0xFC, end of OR, then
	// or is STILL false so break.
	if (state->orTest) {
		state->orTest = false;
		state->testVal &= state->orVal;
	} else {
		state->orTest = true;
		state->orVal = false;
	}
}

uint8 AgiEngine::testCompareStrings(uint8 s1, uint8 s2) {
	char ms1[MAX_STRINGLEN];
	char ms2[MAX_STRINGLEN];
	int j, k, l;

	strcpy(ms1, _game.strings[s1]);
	strcpy(ms2, _game.strings[s2]);

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
			ms1[j++] = toupper(ms1[k]);
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
			ms2[j++] = toupper(ms2[k]);
			break;
		}
	}
	ms2[j] = 0x0;

	return !strcmp(ms1, ms2);
}

uint8 AgiEngine::testKeypressed() {
	int x = _game.keypress;

	_game.keypress = 0;
	if (!x) {
		InputMode mode = _game.inputMode;

		_game.inputMode = INPUT_NONE;
		mainCycle();
		_game.inputMode = mode;
	}

	if (x)
		debugC(5, kDebugLevelScripts | kDebugLevelInput, "keypress = %02x", x);

	return x;
}

uint8 AgiEngine::testController(uint8 cont) {
	return (_game.controllerOccured[cont] ? 1 : 0);
}

uint8 AgiEngine::testPosn(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2) {
	VtEntry *v = &_game.viewTable[n];
	uint8 r;

	r = v->xPos >= x1 && v->yPos >= y1 && v->xPos <= x2 && v->yPos <= y2;

	debugC(7, kDebugLevelScripts, "(%d,%d) in (%d,%d,%d,%d): %s", v->xPos, v->yPos, x1, y1, x2, y2, r ? "true" : "false");

	return r;
}

uint8 AgiEngine::testObjInBox(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2) {
	VtEntry *v = &_game.viewTable[n];

	return v->xPos >= x1 &&
	    v->yPos >= y1 && v->xPos + v->xSize - 1 <= x2 && v->yPos <= y2;
}

// if n is in center of box
uint8 AgiEngine::testObjCenter(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2) {
	VtEntry *v = &_game.viewTable[n];

	return v->xPos + v->xSize / 2 >= x1 &&
			v->xPos + v->xSize / 2 <= x2 && v->yPos >= y1 && v->yPos <= y2;
}

// if nect N is in right corner
uint8 AgiEngine::testObjRight(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2) {
	VtEntry *v = &_game.viewTable[n];

	return v->xPos + v->xSize - 1 >= x1 &&
			v->xPos + v->xSize - 1 <= x2 && v->yPos >= y1 && v->yPos <= y2;
}

// When player has entered something, it is parsed elsewhere
uint8 AgiEngine::testSaid(uint8 nwords, uint8 *cc) {
	AgiGame *state = &_game;
	int c, n = _game.numEgoWords;
	int z = 0;

	if (getflag(fSaidAcceptedInput) || !getflag(fEnteredCli))
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
		case 9999:	// rest of line (empty string counts to...)
			nwords = 1;
			break;
		case 1:	// any word
			break;
		default:
			if (_game.egoWords[c].id != z)
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

	setflag(fSaidAcceptedInput, true);

	return true;
}

int AgiEngine::testIfCode(int lognum) {
	AgiGame *state = &_game;
	uint8 op = 0;
	uint16 lastIp = ip;
	uint8 p[16] = { 0 };

	state->ec = true;
	state->notTest = false;
	state->orTest = false;
	state->endTest = false;
	state->testVal = true;

	while (!(shouldQuit() || _restartGame) && !state->endTest) {
		if (_debug.enabled && (_debug.logic0 || lognum))
			debugConsole(lognum, lTEST_MODE, NULL);

		lastIp = ip;
		op = *(code + ip++);
		memmove(p, (code + ip), 16);

		_agiCondCommands[op](state, p);
		if (op == 0xFF || op == 0xFD || op == 0xFC)
			continue;

		// not is only enabled for 1 test command
		if (state->notTest)
			state->ec = !state->ec;
		state->notTest = false;

		if (state->orTest)
			state->orVal |= state->ec;
		else
			state->testVal &= state->ec;
	}

	// Execute the following IF block if the condition is true, otherwise
	// skip the block.
	if (state->testVal)
		ip += 2;
	else
		ip += READ_LE_UINT16(code + ip) + 2;

	if (_debug.enabled && (_debug.logic0 || lognum))
		debugConsole(lognum, 0xFF, state->testVal ? "=true" : "=false");

	return state->testVal;
}

} // End of namespace Agi
