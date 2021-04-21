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
#include "trecision/nl/define.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/trecision.h"
#include "trecision/graphics.h"
#include "trecision/video.h"

namespace Trecision {

void RedrawRoom() {
	if ((_curDialog != dF321) && (_curDialog != dF431) && (_curDialog != dF4C1) && (_curDialog != dASCENSORE12) && (_curDialog != dASCENSORE13) && (_curDialog != dASCENSORE16))
		g_vm->_flagShowCharacter = true;

	switch (_curDialog) {
	case dASCENSORE12:
		if (_curChoice == 3)
			StartCharacterAction(a129PARLACOMPUTERESCENDE, kRoom13, 20, 0);
		else if (_curChoice == 4)
			StartCharacterAction(a129PARLACOMPUTERESCENDE, kRoom16, 20, 0);
		break;

	case dASCENSORE13:
		if (_curChoice == 17)
			StartCharacterAction(a139CHIUDONOPORTESU, kRoom12, 20, 0);
		else if (_curChoice == 18)
			StartCharacterAction(a1316CHIUDONOPORTEGIU, kRoom16, 20, 0);
		break;

	case dASCENSORE16:
		if (_curChoice == 32)
			StartCharacterAction(a1616SALECONASCENSORE, kRoom12, 20, 0);
		else if (_curChoice == 33)
			StartCharacterAction(a1616SALECONASCENSORE, kRoom13, 20, 0);
		break;
	}

	g_vm->_curSortTableNum = 0;
	for (int i = 0; i < MAXOBJINROOM; ++i) {
		OldObjStatus[i] = false;
		VideoObjStatus[i] = false;
	}

	const bool hasBgAnim = g_vm->_room[g_vm->_curRoom]._bkgAnim;
	g_vm->_graphicsMgr->resetScreenBuffer();

	if (hasBgAnim)
		g_vm->_animMgr->startSmkAnim(g_vm->_room[g_vm->_curRoom]._bkgAnim);

	if ((g_vm->_curRoom == kRoom4P) && (_curDialog == dF4PI)) {
		g_vm->_animMgr->smkGoto(kSmackerBackground, 21);
	}

	RegenRoom();

	TextStatus = TEXT_OFF;
	g_vm->_flagPaintCharacter = true;
	PaintScreen(1);
	g_vm->_graphicsMgr->copyToScreen(0, 0, 640, 480);
}

} // End of namespace Trecision
