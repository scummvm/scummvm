#include "ti99_4a_terp.h"
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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "glk/scott/scott.h"
#include "glk/scott/types.h"
#include "glk/scott/globals.h"
#include "glk/scott/ti99_4a_terp.h"

namespace Glk {
namespace Scott {

ActionResultType performTI99Line(uint8_t *actionLine) {
	if (actionLine == nullptr)
        return ACT_FAILURE;

    uint8_t *ptr = actionLine;
    int runCode = 0;
    int index = 0;
    ActionResultType result = ACT_FAILURE;
    int opcode, param;

    int tryIndex;
    int tryArr[32];

    tryIndex = 0;

    while (runCode == 0) {
        opcode = *(ptr++);

        switch (opcode) {
        case 183: /* is p in inventory? */
            if (_G(_items)[*(ptr++)]._location != CARRIED) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 184: /* is p in room? */
            if (_G(_items)[*(ptr++)]._location != MY_LOC) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 185: /* is p available? */
            if (_G(_items)[*ptr]._location != CARRIED && _G(_items)[*ptr]._location != MY_LOC) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            ptr++;
            break;

        case 186: /* is p here? */
            if (_G(_items)[*(ptr++)]._location == MY_LOC) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 187: /* is p NOT in inventory? */
            if (_G(_items)[*(ptr++)]._location == CARRIED) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 188: /* is p NOT available? */
            if (_G(_items)[*ptr]._location == CARRIED || _G(_items)[*ptr]._location == MY_LOC) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            ptr++;
            break;

        case 189: /* is p in play? */
            if (_G(_items)[*(ptr++)]._location == 0) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 190: /* Is object p NOT in play? */
            if (_G(_items)[*(ptr++)]._location != 0) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 191: /* Is player is in room p? */
            if (MY_LOC != *(ptr++)) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 192: /* Is player NOT in room p? */
            if (MY_LOC == *(ptr++)) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 193: /* Is bitflag p clear? */
            if ((_G(_bitFlags) & (1 << *(ptr++))) == 0) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 194: /* Is bitflag p set? */
            if (_G(_bitFlags) & (1 << *(ptr++))) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 195: /* Does the player carry anything? */
            if (g_scott->countCarried() == 0) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 196: /* Does the player carry nothing? */
            if (g_scott->countCarried()) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 197: /* Is _G(_currentCounter) <= p? */
            if (_G(_currentCounter) > *(ptr++)) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 198: /* Is _G(_currentCounter) > p? */
            if (_G(_currentCounter) <= *(ptr++)) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 199: /* Is _G(_currentCounter) == p? */
            if (_G(_currentCounter) != *(ptr++)) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            break;

        case 200: /* Is item p still in initial room? */
            if (_G(_items)[*ptr]._location != _G(_items)[*ptr]._initialLoc) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            ptr++;
            break;

        case 201: /* Has item p been moved? */
            if (_G(_items)[*ptr]._location == _G(_items)[*ptr]._initialLoc) {
                runCode = 1;
                result = ACT_FAILURE;
            }
            ptr++;
            break;

        case 212: /* clear screen */
            g_scott->glk_window_clear(_G(_bottomWindow));
            break;

        case 214: /* inv */
            _G(_autoInventory) = 1;
            break;

        case 215: /* !inv */
            _G(_autoInventory) = 0;
            break;

        case 216:
        case 217:
            break;

        case 218:
            if (tryIndex >= 32) {
                g_scott->fatal("ERROR Hit upper limit on try method.");
            }
            tryArr[tryIndex++] = ptr - actionLine + *ptr;
            ptr++;
            break;

        case 219: /* get item */
            if (g_scott->countCarried() >= _G(_gameHeader)->_maxCarry) {
                g_scott->output(_G(_sys)[YOURE_CARRYING_TOO_MUCH]);
                runCode = 1;
                result = ACT_FAILURE;
                break;
            } else {
                _G(_items)[*ptr]._location = CARRIED;
            }
            ptr++;
            break;

        case 220: /* drop item */
            _G(_items)[*(ptr++)]._location = MY_LOC;
            _G(_shouldLookInTranscript) = 1;
            break;

        case 221: /* go to room */
            MY_LOC = *(ptr++);
            _G(_shouldLookInTranscript) = 1;
            g_scott->look();
            break;

        case 222: /* move item p to room 0 */
            _G(_items)[*(ptr++)]._location = 0;
            break;

        case 223: /* darkness */
            _G(_bitFlags) |= 1 << DARKBIT;
            break;

        case 224: /* light */
            _G(_bitFlags) &= ~(1 << DARKBIT);
            break;

        case 225: /* set flag p */
            _G(_bitFlags) |= (1 << *(ptr++));
            break;

        case 226: /* clear flag p */
            _G(_bitFlags) &= ~(1 << *(ptr++));
            break;

        case 227: /* set flag 0 */
            _G(_bitFlags) |= (1 << 0);
            break;

        case 228: /* clear flag 0 */
            _G(_bitFlags) &= ~(1 << 0);
            break;

        case 229: /* die */
            g_scott->playerIsDead();
			g_scott->doneIt();
            result = ACT_GAMEOVER;
            break;

        case 230: /* move item p2 to room p */
            param = *(ptr++);
			g_scott->putItemAInRoomB(*(ptr++), param);
            break;

        case 231: /* quit */
			g_scott->doneIt();
            return ACT_GAMEOVER;

        case 232: /* print score */
			if (g_scott->printScore() == 1)
                return ACT_GAMEOVER;
			_G(_stopTime) = 2;
            break;

        case 233: /* list contents of inventory */
			g_scott->listInventory();
			_G(_stopTime) = 2;
            break;

        case 234: /* refill lightsource */
            _G(_gameHeader)->_lightTime = _G(_lightRefill);
            _G(_items)[LIGHT_SOURCE]._location = CARRIED;
            _G(_bitFlags) &= ~(1 << LIGHTOUTBIT);
            break;

        case 235: /* save */
			g_scott->saveGame();
			_G(_stopTime) = 2;
            break;

        case 236: /* swap items p and p2 around */
            param = *(ptr++);
			g_scott->swapItemLocations(param, *(ptr++));
            break;

        case 237: /* move item p to the inventory */
            _G(_items)[*(ptr++)]._location = CARRIED;
            break;

        case 238: /* make item p same room as item p2 */
            param = *(ptr++);
            g_scott->moveItemAToLocOfItemB(param, *(ptr++));
            break;

        case 239: /* nop */
            break;

        case 240: /* look at room */
			g_scott->look();
            _G(_shouldLookInTranscript) = 1;
            break;

        case 241: /* unknown */
            break;

        case 242: /* add 1 to current counter */
            _G(_currentCounter)++;
            break;

        case 243: /* sub 1 from current counter */
            if (_G(_currentCounter) >= 1)
                _G(_currentCounter)--;
            break;

        case 244: /* print current counter */
			g_scott->outputNumber(_G(_currentCounter));
			g_scott->output(" ");
            break;

        case 245: /* set current counter to p */
            _G(_currentCounter) = *(ptr++);
            break;

        case 246: /*  add to current counter */
            _G(_currentCounter) += *(ptr++);
            break;

        case 247: /* sub from current counter */
            _G(_currentCounter) -= *(ptr++);
            if (_G(_currentCounter) < -1)
                _G(_currentCounter) = -1;
            break;

        case 248: /* go to stored location */
            g_scott->goToStoredLoc();
            break;

        case 249: /* swap room and counter */
            g_scott->swapLocAndRoomFlag(*(ptr++));
            break;

        case 250: /* swap current counter */
            g_scott->swapCounters(*(ptr++));
            break;

        case 251: /* print noun */
            g_scott->printNoun();
            break;

        case 252: /* print noun + newline */
			g_scott->printNoun();
			g_scott->output("\n");
            break;

        case 253: /* print newline */
			g_scott->output("\n");
            break;

        case 254: /* delay */
			g_scott->delay(1);
            break;

        case 255: /* end of code block. */
            result = ACT_SUCCESS;
            runCode = 1;
            tryIndex = 0; /* drop out of all try blocks! */
            break;

        default:
            if (opcode <= 182 && opcode <= _G(_gameHeader)->_numMessages + 1) {
                g_scott->printMessage(opcode);
            } else {
                index = ptr - actionLine;
                error("Unknown action %d [Param begins %d %d]", opcode, actionLine[index], actionLine[index + 1]);
                break;
            }
            break;
        }

        /* we are on the 0xff opcode, or have fallen through */
        if (runCode == 1 && tryIndex > 0) {
            if (opcode == 0xff) {
                runCode = 1;
            } else {
                /* dropped out of TRY block */
                /* or at end of TRY block */
                index = tryArr[tryIndex - 1];

                tryIndex -= 1;
                tryArr[tryIndex] = 0;
                runCode = 0;
                ptr = actionLine + index;
            }
        }
    }

    return result;
}

void runImplicitTI99Actions() {
	int probability;
	uint8_t *ptr;
	int loopFlag;

	ptr = _G(_ti99ImplicitActions);
	loopFlag = 0;

	/* bail if no auto acts in the game. */
	if (*ptr == 0x0)
		loopFlag = 1;

	while (loopFlag == 0) {
		/*
		 p + 0 = chance of happening
		 p + 1 = size of code chunk
		 p + 2 = start of code
		 */

		probability = ptr[0];

		if (g_scott->randomPercent(probability))
			performTI99Line(ptr + 2);

		if (ptr[1] == 0 || static_cast<size_t>(ptr - _G(_ti99ImplicitActions)) >= _G(_ti99ImplicitExtent))
			loopFlag = 1;

		/* skip code chunk */
		ptr += 1 + ptr[1];
	}
}

ExplicitResultType runExplicitTI99Actions(int verbNum, int nounNum) {
	uint8_t *p;
	ExplicitResultType flag = ER_NO_RESULT;
	int match = 0;
	ActionResultType runcode;

	p = _G(_verbActionOffsets)[verbNum];

	/* process all code blocks for this verb
	 until success or end. */

	while (flag == ER_NO_RESULT) {
		/* we match VERB NOUN or VERB ANY */
		if (p != nullptr && (p[0] == nounNum || p[0] == 0)) {
			match = 1;
			runcode = performTI99Line(p + 2);

			if (runcode == ACT_SUCCESS) {
				return ER_SUCCESS;
			} else { /* failure */
				if (p[1] == 0)
					flag = ER_RAN_ALL_LINES;
				else
					p += 1 + p[1];
			}
		} else {
			if (p == nullptr || p[1] == 0)
				flag = ER_RAN_ALL_LINES_NO_MATCH;
			else
				p += 1 + p[1];
		}
	}

	if (match)
		flag = ER_RAN_ALL_LINES;

	return flag;
}

} // End of namespace Scott
} // End of namespace Glk
