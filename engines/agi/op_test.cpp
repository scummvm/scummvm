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

#define ip (_game.logics[lognum].cIP)
#define code (_game.logics[lognum].data)

#define testEqual(v1, v2)		(getvar(v1) == (v2))
#define testLess(v1, v2)		(getvar(v1) < (v2))
#define testGreater(v1, v2)	(getvar(v1) > (v2))
#define testIsSet(flag)		(getflag(flag))
#define testHas(obj)			(objectGetLocation(obj) == EGO_OWNED)
#define testObjInRoom(obj, v)	(objectGetLocation(obj) == getvar(v))

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
	int ec = true;
	int retval = true;
	uint8 op = 0;
	uint8 notTest = false;
	uint8 orTest = false;
	uint16 lastIp = ip;
	uint8 p[16] = { 0 };
	bool end_test = false;

	while (retval && !(shouldQuit() || _restartGame) && !end_test) {
		if (_debug.enabled && (_debug.logic0 || lognum))
			debugConsole(lognum, lTEST_MODE, NULL);

		lastIp = ip;
		op = *(code + ip++);
		memmove(p, (code + ip), 16);

		switch (op) {
		case 0xFF:	// END IF, TEST true
			end_test = true;
			break;
		case 0xFD:
			notTest = !notTest;
			continue;
		case 0xFC:	// OR
			// if or_test is ON and we hit 0xFC, end of OR, then
			// or is STILL false so break.
			if (orTest) {
				ec = false;
				retval = false;
				end_test = true;
			}

			orTest = true;
			continue;

		case 0x00:
			// return true?
			end_test = true;
			break;
		case 0x01:
			ec = testEqual(p[0], p[1]);
			if (p[0] == 11)
				_timerHack++;
			break;
		case 0x02:
			ec = testEqual(p[0], getvar(p[1]));
			if (p[0] == 11 || p[1] == 11)
				_timerHack++;
			break;
		case 0x03:
			ec = testLess(p[0], p[1]);
			if (p[0] == 11)
				_timerHack++;
			break;
		case 0x04:
			ec = testLess(p[0], getvar(p[1]));
			if (p[0] == 11 || p[1] == 11)
				_timerHack++;
			break;
		case 0x05:
			ec = testGreater(p[0], p[1]);
			if (p[0] == 11)
				_timerHack++;
			break;
		case 0x06:
			ec = testGreater(p[0], getvar(p[1]));
			if (p[0] == 11 || p[1] == 11)
				_timerHack++;
			break;
		case 0x07:
			ec = testIsSet(p[0]);
			break;
		case 0x08:
			ec = testIsSet(getvar(p[0]));
			break;
		case 0x09:
			ec = testHas(p[0]);
			break;
		case 0x0A:
			ec = testObjInRoom(p[0], p[1]);
			break;
		case 0x0B:
			ec = testPosn(p[0], p[1], p[2], p[3], p[4]);
			break;
		case 0x0C:
			ec = testController(p[0]);
			break;
		case 0x0D:
			ec = testKeypressed();
			break;
		case 0x0E:
			ec = testSaid(p[0], (uint8 *) code + (ip + 1));
			ip = lastIp;
			ip++;	// skip opcode
			ip += p[0] * 2;	// skip num_words * 2
			ip++;	// skip num_words opcode
			break;
		case 0x0F:
			debugC(7, kDebugLevelScripts, "comparing [%s], [%s]", _game.strings[p[0]], _game.strings[p[1]]);
			ec = testCompareStrings(p[0], p[1]);
			break;
		case 0x10:
			ec = testObjInBox(p[0], p[1], p[2], p[3], p[4]);
			break;
		case 0x11:
			ec = testObjCenter(p[0], p[1], p[2], p[3], p[4]);
			break;
		case 0x12:
			ec = testObjRight(p[0], p[1], p[2], p[3], p[4]);
			break;
		case 0x13: // Unknown test command 19
			// My current theory is that this command checks whether the ego is currently moving
			// and that that movement has been caused using the mouse and not using the keyboard.
			// I base this theory on the game's behavior on an Amiga emulator, not on disassembly.
			// This command is used at least in the Amiga version of Gold Rush! v2.05 1989-03-09
			// (AGI 2.316) in logics 1, 3, 5, 6, 137 and 192 (Logic.192 revealed this command's nature).
			// TODO: Check this command's implementation using disassembly just to be sure.
			ec = _game.viewTable[0].flags & ADJ_EGO_XY;
			debugC(7, kDebugLevelScripts, "op_test: in.motion.using.mouse = %s (Amiga-specific testcase 19)", ec ? "true" : "false");
			break;
		default:
			ec = false;
			end_test = true;
		}

		if (!end_test) {
			if (op <= 0x12)
				ip += logicNamesTest[op].numArgs;

			// exchange ec value
			if (notTest)
				ec = !ec;

			// not is only enabled for 1 test command
			notTest = false;

			if (orTest && ec) {
				// a true inside an OR statement passes
				// ENTIRE statement scan for end of OR

				// CM: test for opcode < 0xfc changed from 'op' to
				//     '*(code+ip)', to avoid problem with the 0xfd (NOT)
				//     opcode byte. Changed a bad ip += ... ip++ construct.
				//     This should fix the crash with Larry's logic.0 code:
				//
				//     if ((isset(4) ||
				//          !isset(2) ||
				//          v30 == 2 ||
				//          v30 == 1)) {
				//       goto Label1;
				//     }
				//
				//     The bytecode is:
				//     ff fc 07 04 fd 07 02 01 1e 02 01 1e 01 fc ff

				// find end of OR
				while (*(code + ip) != 0xFC) {
					if (*(code + ip) == 0x0E) {	// said
						ip++;

						// cover count + ^words
						ip += 1 + ((*(code + ip)) * 2);
						continue;
					}

					if (*(code + ip) < 0xFC)
						ip += logicNamesTest[*(code + ip)].numArgs;
					ip++;
				}
				ip++;

				orTest = false;
				retval = true;
			} else {
				retval = orTest ? retval || ec : retval && ec;
			}
		}
	}

	// if false, scan for end of IP?
	if (retval)
		ip += 2;
	else {
		ip = lastIp;
		while (*(code + ip) != 0xff) {
			if (*(code + ip) == 0x0e) {
				ip++;
				ip += (*(code + ip)) * 2 + 1;
			} else if (*(code + ip) < 0xfc) {
				ip += logicNamesTest[*(code + ip)].numArgs;
				ip++;
			} else {
				ip++;
			}
		}
		ip++;		// skip over 0xFF
		ip += READ_LE_UINT16(code + ip) + 2;
	}

	if (_debug.enabled && (_debug.logic0 || lognum))
		debugConsole(lognum, 0xFF, retval ? "=true" : "=false");

	return retval;
}

} // End of namespace Agi
