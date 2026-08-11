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

#include "chewy/cursor.h"
#include "chewy/defines.h"
#include "chewy/detail.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/room.h"
#include "chewy/rooms/room86.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room86::entry(int16 eib_nr) {
	_G(gameState).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(gameState).ZoomXy[P_HOWARD][0] = 20;
	_G(gameState).ZoomXy[P_HOWARD][1] = 20;
	_G(gameState).ZoomXy[P_NICHELLE][0] = 20;
	_G(gameState).ZoomXy[P_NICHELLE][1] = 24;
	_G(zoom_horizont) = 110;
	_G(gameState).DiaAMov = 0;
	if (_G(gameState).flags32_2) {
		_G(det)->startDetail(0, 255, false);
		_G(det)->setStaticPos(0, 352, 107, false, false);
		_G(det)->showStaticSpr(0);
	}

	if (_G(flags).LoadGame)
		return;

	if (eib_nr == 127) {
		setPersonPos(82, 56, P_HOWARD, P_RIGHT);
		setPersonPos(134, 56, P_NICHELLE, P_LEFT);
		return;
	}

	if (!_G(gameState).flags32_10) {
		setPersonPos(260, 66, P_CHEWY, P_RIGHT);
		setPersonPos(298, 44, P_HOWARD, P_LEFT);
		setPersonPos(320, 42, P_NICHELLE, P_LEFT);
		_G(gameState).scrollx = 164;
	} else {
		_G(spieler_mi)[P_CHEWY].Vorschub = 16;
		hideCur();
		_G(flags).ZoomMov = false;
		_G(gameState).scrollx = 246;
		_G(gameState).ScrollxStep = 8;
		setPersonPos(443, 66, P_CHEWY, P_RIGHT);
		_G(gameState)._personRoomNr[P_HOWARD] = 84;
		_G(gameState)._personRoomNr[P_NICHELLE] = 0;
		autoMove(2, P_CHEWY);
		_G(flags).NoScroll = true;
		_G(gameState).ScrollxStep = 2;
		auto_scroll(30, 0);
		start_spz_wait(13, 1, false, P_CHEWY);
		_G(flags).NoScroll = false;
		_G(spieler_mi)[P_CHEWY].Vorschub = 8;
		_G(det)->stopDetail(0);
		_G(det)->showStaticSpr(4);
		_G(det)->showStaticSpr(5);
		invent_2_slot(94);
		autoMove(4, P_CHEWY);
		_G(flags).NoScroll = true;
		auto_scroll(246, 0);
		proc3(false);
		flic_cut(FCUT_092);
		_G(flags).NoScroll = false;
		autoMove(0, P_CHEWY);
		_G(gameState).flags32_20 = true;
		switchRoom(85);
		showCur();
	}
}

void Room86::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;
	switch (eib_nr) {
	case 128:
		if (_G(gameState)._personRoomNr[P_HOWARD] == 86)
			_G(gameState)._personRoomNr[P_HOWARD] = 85;
		
		if (_G(gameState)._personRoomNr[P_NICHELLE] == 86)
			_G(gameState)._personRoomNr[P_NICHELLE] = 85;
		break;
	case 132:
		if (_G(gameState)._personRoomNr[P_HOWARD] == 86)
			_G(gameState)._personRoomNr[P_HOWARD] = 87;

		if (_G(gameState)._personRoomNr[P_NICHELLE] == 86)
			_G(gameState)._personRoomNr[P_NICHELLE] = 87;
		break;
	default:
		break;
	}
}

void Room86::setup_func() {
	calc_person_look();

	int nicDestX;
	int howDestY = 56;
	int howDestX;
	int nicDestY = 56;

	int xyPos = _G(moveState)[P_CHEWY].Xypos[0];	
	if (xyPos > 390) {
		howDestX = 298;
		howDestY = 44;
		nicDestX = 320;
		nicDestY = 42;
	} else if (xyPos > 250) {
		howDestX = 216;
		nicDestX = 240;
	} else {
		howDestX = 82;
		nicDestX = 134;
	}

	goAutoXy(howDestX, howDestY, P_HOWARD, ANI_GO);
	goAutoXy(nicDestX, nicDestY, P_NICHELLE, ANI_GO);
}

int Room86::proc2() {
	if (!isCurInventory(94))
		return 0;

	hideCur();
	autoMove(2, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	_G(det)->startDetail(0, 255, false);
	_G(det)->playSound(0, 0);
	delInventory(_G(cur)->getInventoryCursor());
	autoMove(3, P_CHEWY);
	proc3(true);
	_G(atds)->delControlBit(499, ATS_ACTIVE_BIT);
	_G(atds)->set_all_ats_str(497, 1, ATS_DATA);
	_G(atds)->set_all_ats_str(498, 1, ATS_DATA);
	_G(gameState).flags32_2 = true;
	_G(gameState).room_e_obj[132].Attribut = EXIT_RIGHT;
	start_spz(CH_TALK12, 255, false, P_CHEWY);
	startAadWait(468);

	showCur();
	return 1;
}

void Room86::proc3(bool cond) {
	int destY, deltaY;

	if (cond) {
		destY = 199;
		deltaY = -2;
	} else {
		destY = 104;
		deltaY = 2;
	}

	if (_G(flags).NoScroll)
		auto_scroll(196, 0);

	_G(det)->setStaticPos(0, 352, destY, false, false);
	_G(det)->showStaticSpr(0);
	_G(det)->playSound(0, 1);
	_G(det)->playSound(0, 2);

	for (int i = 0; i < 48; ++i) {
		setupScreen(NO_SETUP);
		_G(det)->setStaticPos(0, 352, destY, false, false);
		destY += deltaY;
		_G(out)->setPointer(nullptr);
		_G(out)->copyToScreen();
	}

	_G(det)->stopSound(1);
	_G(det)->stopSound(2);
	_G(flags).NoScroll = false;
}

} // namespace Rooms
} // namespace Chewy
