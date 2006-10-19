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

	// Confirmed
	op[8] = &AGOSEngine::oe1_isNotAt;
	op[9] = &AGOSEngine::oe1_sibling;
	op[10] = &AGOSEngine::oe1_notSibling;
	op[24] = &AGOSEngine::oe1_isPlayer;
	op[29] = &AGOSEngine::oe1_canPut;
	op[34] = &AGOSEngine::oe1_copyof;
	op[37] = &AGOSEngine::oe1_whatO;
	op[35] = &AGOSEngine::oe1_copyfo;
	op[54] = &AGOSEngine::oww_moveDirn;
	op[55] = &AGOSEngine::oww_goto;
	op[70] = &AGOSEngine::o1_printLongText;
	op[73] = &AGOSEngine::oe1_pObj;
	op[74] = &AGOSEngine::oe1_pName;
	op[75] = &AGOSEngine::oe1_pcName;
	op[83] = &AGOSEngine::o1_rescan;
	op[85] = &AGOSEngine::oww_whereTo;
	op[89] = &AGOSEngine::oe2_loadUserGame;
	op[94] = &AGOSEngine::oe1_findMaster;
	op[95] = &AGOSEngine::oe1_nextMaster;
	op[98] = &AGOSEngine::o1_animate;
	op[99] = &AGOSEngine::o1_stopAnimate;
	op[105] = &AGOSEngine::oww_menu;
	op[106] = &AGOSEngine::oww_textMenu;
	op[123] = &AGOSEngine::oe1_setTime;
	op[127] = &AGOSEngine::o1_playTune;
	op[144] = &AGOSEngine::oe2_setDoorOpen;
	op[145] = &AGOSEngine::oe2_setDoorClosed;
	op[146] = &AGOSEngine::oe2_setDoorLocked;
	op[147] = &AGOSEngine::oe2_setDoorClosed;
	op[148] = &AGOSEngine::oe2_ifDoorOpen;
	op[149] = &AGOSEngine::oe2_ifDoorClosed;
	op[150] = &AGOSEngine::oe2_ifDoorLocked;
	op[162] = &AGOSEngine::oe2_screenTextMsg;
	op[175] = &AGOSEngine::o_getDollar2;
	op[179] = &AGOSEngine::o_isAdjNoun;
	op[180] = &AGOSEngine::o_b2Set;
	op[181] = &AGOSEngine::o_b2Clear;
	op[182] = &AGOSEngine::o_b2Zero;
	op[183] = &AGOSEngine::o_b2NotZero;
	op[184] = &AGOSEngine::oww_boxMessage;
	op[185] = &AGOSEngine::oww_boxMsg;
	op[186] = &AGOSEngine::oww_boxLongText;
	op[187] = &AGOSEngine::oww_printBox;
	op[188] = &AGOSEngine::oww_boxPObj;
	op[189] = &AGOSEngine::o_lockZones;
	op[190] = &AGOSEngine::o_unlockZones;

	// Code difference, check if triggered
	op[161] = NULL;
	op[163] = NULL;
	op[164] = NULL;
	op[165] = NULL;
	op[166] = NULL;
	op[167] = NULL;
	op[168] = NULL;
	op[169] = NULL;
	op[170] = NULL;
	op[171] = NULL;
	op[172] = NULL;
	op[173] = NULL;
	op[174] = NULL;
	op[176] = NULL;
	op[177] = NULL;
	op[178] = NULL;
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
	// 188: print object flag name to box
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);

	if (subObject != NULL && subObject->objectFlags & kOFText)
		boxTextMsg((const char *)getStringPtrByID(subObject->objectFlagValue[0]));
}

} // End of namespace AGOS
