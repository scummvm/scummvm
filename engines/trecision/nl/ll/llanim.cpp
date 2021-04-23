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

#include "common/util.h"
#include "trecision/dialog.h"
#include "trecision/nl/define.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/trecision.h"
#include "trecision/graphics.h"
#include "trecision/video.h"

namespace Trecision {

struct ElevatorAction {
	uint16 dialog;
	uint16 choice;
	uint16 action;
	uint16 newRoom;
};

void RedrawRoom() {
	const uint16 curDialog = g_vm->_dialogMgr->_curDialog;
	const uint16 curChoice = g_vm->_dialogMgr->_curChoice;
	const uint16 bgAnim = g_vm->_room[g_vm->_curRoom]._bkgAnim;
	const ElevatorAction elevatorActions[6] = {
		{ dASCENSORE12,  3, a129PARLACOMPUTERESCENDE, kRoom13 },
		{ dASCENSORE12,  4, a129PARLACOMPUTERESCENDE, kRoom16 },
		{ dASCENSORE13, 17,      a139CHIUDONOPORTESU, kRoom12 },
		{ dASCENSORE13, 18,    a1316CHIUDONOPORTEGIU, kRoom16 },
		{ dASCENSORE16, 32,    a1616SALECONASCENSORE, kRoom12 },
		{ dASCENSORE16, 33,    a1616SALECONASCENSORE, kRoom13 },
	};

	g_vm->_flagShowCharacter = g_vm->_dialogMgr->showCharacterAfterDialog();

	for (int i = 0; i < 6; i++) {
		if (curDialog == elevatorActions[i].dialog && curChoice == elevatorActions[i].choice) {
			StartCharacterAction(elevatorActions[i].action, elevatorActions[i].newRoom, 20, 0);
			break;
		}
	}

	g_vm->_curSortTableNum = 0;
	for (int i = 0; i < MAXOBJINROOM; ++i) {
		OldObjStatus[i] = false;
		VideoObjStatus[i] = false;
	}

	g_vm->_graphicsMgr->resetScreenBuffer();

	if (bgAnim)
		g_vm->_animMgr->startSmkAnim(bgAnim);

	if (g_vm->_curRoom == kRoom4P && curDialog == dF4PI) {
		g_vm->_animMgr->smkGoto(kSmackerBackground, 21);
	}

	RegenRoom();

	TextStatus = TEXT_OFF;
	g_vm->_flagPaintCharacter = true;
	PaintScreen(1);
	g_vm->_graphicsMgr->copyToScreen(0, 0, 640, 480);
}

} // End of namespace Trecision
