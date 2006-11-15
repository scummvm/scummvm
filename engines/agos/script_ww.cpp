/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "common/system.h"

#include "agos/agos.h"

namespace AGOS {

void AGOSEngine::setupWaxworksOpcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[8] = &AGOSEngine::oe1_isNotAt;
	op[9] = &AGOSEngine::oe1_sibling;
	op[10] = &AGOSEngine::oe1_notSibling;
	op[21] = &AGOSEngine::oe1_isIn;
	op[22] = &AGOSEngine::oe1_isNotIn;
	op[24] = &AGOSEngine::oe1_isPlayer;
	op[29] = &AGOSEngine::oe1_canPut;
	op[34] = &AGOSEngine::oe1_copyof;
	op[37] = &AGOSEngine::oe1_whatO;
	op[35] = &AGOSEngine::oe1_copyfo;
	op[39] = &AGOSEngine::oe1_weigh;
	op[54] = &AGOSEngine::oww_moveDirn;
	op[55] = &AGOSEngine::oww_goto;
	op[65] = &AGOSEngine::oww_addTextBox;
	op[66] = &AGOSEngine::oww_setShortText;
	op[67] = &AGOSEngine::oww_setLongText;
	op[70] = &AGOSEngine::oww_printLongText;
	op[83] = &AGOSEngine::oe1_rescan;
	op[85] = &AGOSEngine::oww_whereTo;
	op[89] = &AGOSEngine::oe2_loadGame;
	op[94] = &AGOSEngine::oe1_findMaster;
	op[95] = &AGOSEngine::oe1_nextMaster;
	op[98] = &AGOSEngine::oe1_animate;
	op[99] = &AGOSEngine::oe1_stopAnimate;
	op[105] = &AGOSEngine::oww_menu;
	op[106] = &AGOSEngine::oww_textMenu;
	op[127] = &AGOSEngine::os1_playTune;
	op[135] = &AGOSEngine::oww_pauseGame;
	op[144] = &AGOSEngine::oe2_setDoorOpen;
	op[145] = &AGOSEngine::oe2_setDoorClosed;
	op[146] = &AGOSEngine::oe2_setDoorLocked;
	op[147] = &AGOSEngine::oe2_setDoorClosed;
	op[148] = &AGOSEngine::oe2_ifDoorOpen;
	op[149] = &AGOSEngine::oe2_ifDoorClosed;
	op[150] = &AGOSEngine::oe2_ifDoorLocked;
	op[175] = &AGOSEngine::oe2_getDollar2;
	op[179] = &AGOSEngine::oe2_isAdjNoun;
	op[180] = &AGOSEngine::oe2_b2Set;
	op[181] = &AGOSEngine::oe2_b2Clear;
	op[182] = &AGOSEngine::oe2_b2Zero;
	op[183] = &AGOSEngine::oe2_b2NotZero;
	op[184] = &AGOSEngine::oww_boxMessage;
	op[185] = &AGOSEngine::oww_boxMsg;
	op[186] = &AGOSEngine::oww_boxLongText;
	op[187] = &AGOSEngine::oww_printBox;
	op[188] = &AGOSEngine::oww_boxPObj;
	op[189] = &AGOSEngine::oww_lockZones;
	op[190] = &AGOSEngine::oww_unlockZones;
}

// -----------------------------------------------------------------------
// Waxworks Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::oww_moveDirn() {
	// 54: move direction
	int16 d = getVarOrByte();
	moveDirn_ww(me(), d);
}

void AGOSEngine::oww_goto() {
	// 55: set itemA parent
	uint item = getNextItemID();
	if (derefItem(item) == NULL) {
		setItemParent(me(), NULL);
		loadRoomItems(item);
	}
	setItemParent(me(), derefItem(item));
}

void AGOSEngine::oww_addTextBox() {
	// 65: add hit area
	uint id = getVarOrWord();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint w = getVarOrWord();
	uint h = getVarOrWord();
	uint number = getVarOrByte();
	if (number < _numTextBoxes)
		defineBox(id, x, y, w, h, (number << 8) + 129, 208, _dummyItem2);
}

void AGOSEngine::oww_setShortText() {
	// 66: set item name
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (var < _numTextBoxes) {
		_shortText[var] = stringId;
	}
}

void AGOSEngine::oww_setLongText() {
	// 67: set item description
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (getFeatures() & GF_TALKIE) {
		uint speechId = getNextWord();
		if (var < _numTextBoxes) {
			_longText[var] = stringId;
			_longSound[var] = speechId;
		}
	} else {
		if (var < _numTextBoxes) {
			_longText[var] = stringId;
		}
	}
}

void AGOSEngine::oww_printLongText() {
	// 70: show string from array
	const char *str = (const char *)getStringPtrByID(_longText[getVarOrByte()]);
	showMessageFormat("%s\n", str);
}

void AGOSEngine::oww_whereTo() {
	// 85: where to
	Item *i = getNextItemPtr();
	int16 d = getVarOrByte();
	int16 f = getVarOrByte();

	if (f == 1)
		_subjectItem = derefItem(getExitOf(i, d));
	else
		_objectItem = derefItem(getExitOf(i, d));
}

void AGOSEngine::oww_menu() {
	// 105: set agos menu
	_agosMenu = getVarOrByte();
}

void AGOSEngine::oww_textMenu() {
	// 106: set text menu
	byte slot = getVarOrByte();
	_textMenu[slot] = getVarOrByte();
}

void AGOSEngine::oww_pauseGame() {
	// 135: pause game
	HitArea *ha;

	time_t pauseTime = time(NULL);
	haltAnimation();

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		for (;;) {
			if (_lastHitArea3 != 0)
				break;
			delay(1);
		}

		ha = _lastHitArea;

		if (ha == NULL) {
		} else if (ha->id == 200) {
			break;
		} else if (ha->id == 201) {
			break;
		}
	}

	restartAnimation();
	_gameStoppedClock = time(NULL) - pauseTime + _gameStoppedClock;
}

void AGOSEngine::oww_boxMessage() {
	// 184:  print message to box
	boxTextMessage((const char *)getStringPtrByID(getNextStringID()));
}

void AGOSEngine::oww_boxMsg() {
	// 185:  print msg to box
	boxTextMsg((const char *)getStringPtrByID(getNextStringID()));
}

void AGOSEngine::oww_boxLongText() {
	// 186: print long text to box
	boxTextMsg((const char *)getStringPtrByID(_longText[getVarOrByte()]));
}

void AGOSEngine::oww_printBox() {
	// 187: print box
	printBox();
}

void AGOSEngine::oww_boxPObj() {
	// 188: print object name to box
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);

	if (subObject != NULL && subObject->objectFlags & kOFText)
		boxTextMsg((const char *)getStringPtrByID(subObject->objectFlagValue[0]));
}

void AGOSEngine::oww_lockZones() {
	// 189: lock zone
	_vgaMemBase = _vgaMemPtr;
}

void AGOSEngine::oww_unlockZones() {
	// 190: unlock zone
	_vgaMemPtr = _vgaFrozenBase;
	_vgaMemBase = _vgaFrozenBase;
}

} // End of namespace AGOS
