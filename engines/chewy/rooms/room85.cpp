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
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/room.h"
#include "chewy/rooms/room85.h"
#include "chewy/rooms/room87.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room85::entry(int16 eib_nr) {
	_G(det)->playSound(0, 0);
	_G(det)->playSound(0, 1);
	_G(gameState).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(gameState).ZoomXy[P_HOWARD][0] = 22;
	_G(gameState).ZoomXy[P_HOWARD][1] = 37;
	_G(gameState).ZoomXy[P_NICHELLE][0] = 22;
	_G(gameState).ZoomXy[P_NICHELLE][1] = 38;

	_G(zoom_horizont) = 0;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 1;

	if (_G(gameState).flags32_10) {
		_G(det)->showStaticSpr(4);
		_G(atds)->delControlBit(495, ATS_ACTIVE_BIT);
	}

	if (_G(flags).LoadGame)
		return;

	if (_G(gameState).flags30_1 || eib_nr == 124) {
		hideCur();
		setPersonPos(278, 157, P_CHEWY, P_RIGHT);
		setPersonPos(296, 142, P_HOWARD, P_RIGHT);
		setPersonPos(322, 142, P_NICHELLE, P_RIGHT);
		_G(gameState).flags30_1 = false;
		_G(mouseLeftClick) = false;
		_G(gameState).scrollx = 78;
		if (_G(gameState).flags32_40) {
			_G(atds)->delControlBit(506, ATS_ACTIVE_BIT);
			_G(det)->startDetail(1, 255, false);
			setPersonPos(195, 146, P_CHEWY, P_RIGHT);
			setPersonPos(186, 142, P_HOWARD, P_RIGHT);
		}
		showCur();
	} else if (!_G(gameState).flags32_20) {
		setPersonPos(148, 152, P_CHEWY, P_RIGHT);
		setPersonPos(98, 142, P_HOWARD, P_RIGHT);
		setPersonPos(128, 142, P_NICHELLE, P_RIGHT);
		_G(gameState).scrollx = 0;
	} else {
		hideCur();
		_G(gameState).scrollx = 0;
		setPersonPos(133, 152, P_CHEWY, P_RIGHT);
		_G(gameState).room_e_obj[127].Attribut = 255;
		_G(det)->showStaticSpr(5);
		_G(det)->showStaticSpr(6);
		startAadWait(474);
		flic_cut(FCUT_090);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_091);
		_G(det)->hideStaticSpr(6);
		_G(gameState).scrollx = 25;
		_G(fx_blend) = BLEND3;
		startAadWait(475);
		start_aad(476, -1);
		flic_cut(FCUT_089);
		register_cutscene(25);

		switchRoom(84);
		showCur();
	}
}

void Room85::xit(int16 eib_nr) {
	switch (eib_nr) {
	case 126:
		if (_G(gameState)._personRoomNr[P_HOWARD] == 85)
			_G(gameState)._personRoomNr[P_HOWARD] = 84;
		
		if (_G(gameState)._personRoomNr[P_NICHELLE] == 85)
			_G(gameState)._personRoomNr[P_NICHELLE] = 84;
		break;
	case 127:
		if (_G(gameState)._personRoomNr[P_HOWARD] == 85)
			_G(gameState)._personRoomNr[P_HOWARD] = 86;

		if (_G(gameState)._personRoomNr[P_NICHELLE] == 85)
			_G(gameState)._personRoomNr[P_NICHELLE] = 86;
		break;
	default:
		break;
	}
}

void Room85::setup_func() {
	calc_person_look();

	const int xyPos = _G(moveState)[P_CHEWY].Xypos[0];
	int howDestX, nicDestX;
	
	if (xyPos > 255) {
		howDestX = 296;
		nicDestX = 322;
	} else if (xyPos > 150) {
		howDestX = 186;
		nicDestX = 212;
	} else {
		howDestX = 98;
		nicDestX = 128;
	}

	if (_G(HowardMov) == 1) {
		howDestX = 98;
		nicDestX = 128;
	}

	goAutoXy(howDestX, 142, P_HOWARD, ANI_GO);
	goAutoXy(nicDestX, 142, P_NICHELLE, ANI_GO);
}

int Room85::proc2() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	autoMove(2, P_CHEWY);
	_G(det)->stop_detail(1);
	startSetAILWait(2, 1, ANI_FRONT);
	_G(gameState)._personRoomNr[P_HOWARD] = 89;
	cur_2_inventory();
	remove_inventory(109);
	remove_inventory(98);
	remove_inventory(87);
	switchRoom(89);
	
	showCur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
