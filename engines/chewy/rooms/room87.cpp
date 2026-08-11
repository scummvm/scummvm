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
#include "chewy/rooms/room87.h"

namespace Chewy {
namespace Rooms {

void Room87::entry() {
	_G(gameState).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(gameState).ZoomXy[P_HOWARD][0] = 18;
	_G(gameState).ZoomXy[P_HOWARD][1] = 28;
	_G(gameState).ZoomXy[P_NICHELLE][0] = 18;
	_G(gameState).ZoomXy[P_NICHELLE][1] = 28;
	_G(zoom_horizont) = 176;
	if (_G(gameState).flags32_4)
		_G(det)->showStaticSpr(3);

	if (_G(gameState).flags32_8)
		_G(det)->showStaticSpr(4);

	if (_G(flags).LoadGame)
		return;

	setPersonPos(11, 70, P_HOWARD, P_RIGHT);
	setPersonPos(-3, 81, P_NICHELLE, P_RIGHT);
	autoMove(2, P_CHEWY);
}

void Room87::setup_func() {
	calc_person_look();
	int howDestX = 0;
	int howDestY = 0;
	int nicDestX = 0;
	int nicDestY = 0;

	
	if (_G(moveState)[P_CHEWY].Xypos[0] > 186) {
		howDestX = 103;
		howDestY = 128;
		nicDestX = 134;
		nicDestY = 123;
	} else if (_G(moveState)[P_CHEWY].Xypos[0] > 20) {
		howDestX = 27;
		howDestY = 98;
		nicDestX = -2;
		nicDestY = 110;
	}

	if (_G(HowardMov) == 1) {
		howDestX = 11;
		howDestY = 70;
		nicDestX = -3;
		nicDestY = 81;
	}

	goAutoXy(howDestX, howDestY, P_HOWARD, ANI_GO);
	goAutoXy(nicDestX, nicDestY, P_NICHELLE, ANI_GO);
}

void Room87::xit(int16 eib_nr) {
	if (eib_nr != 129)
		return;

	_G(spieler_mi)[P_CHEWY].Mode = true;
	_G(zoom_horizont) = 0;
	_G(room)->set_zoom(25);
	_G(HowardMov) = 1;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 2;
	goAutoXy(20, 56, P_CHEWY, ANI_WAIT);
	goAutoXy(31, 56, P_CHEWY, ANI_WAIT);
	goAutoXy(9, 53, P_CHEWY, ANI_WAIT);
	_G(spieler_mi)[P_CHEWY].Mode = false;
	_G(gameState).ScrollxStep = 1;
	if (_G(gameState)._personRoomNr[P_HOWARD] == 87)
		_G(gameState)._personRoomNr[P_HOWARD] = 86;

	if (_G(gameState)._personRoomNr[P_NICHELLE] == 87)
		_G(gameState)._personRoomNr[P_NICHELLE] = 86;
}

int Room87::proc2(int16 txt_nr) {
	if (!isCurInventory(107) && !isCurInventory(108))
		return 0;

	hideCur();

	int diaNr = 0;
	int movNr;
	if (txt_nr == 501) {
		movNr = 3;
		if (_G(gameState).flags32_4)
			diaNr = 473;
		else
			_G(gameState).flags32_4 = true;
	} else {
		movNr = 4;
		if (_G(gameState).flags32_8)
			diaNr = 473;
		else
			_G(gameState).flags32_8 = true;
	}

	if (diaNr) {
		start_spz(CH_TALK1, 255, false, P_CHEWY);
		startAadWait(diaNr);
	} else {
		autoMove(movNr, P_CHEWY);
		delInventory(_G(cur)->getInventoryCursor());
		start_spz_wait(14, 1, false, P_CHEWY);
		_G(atds)->set_all_ats_str(txt_nr, 1, ATS_DATA);
		_G(det)->showStaticSpr(movNr);
		if (_G(gameState).flags32_4 && _G(gameState).flags32_8)
			_G(atds)->delControlBit(502, ATS_ACTIVE_BIT);
	}
	showCur();
	return 1;
}

int Room87::proc4() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();
	autoMove(1, P_CHEWY);
	flic_cut(FCUT_093);
	flic_cut(FCUT_094);
	start_aad(472);
	flic_cut(FCUT_095);
	flic_cut(FCUT_096);
	_G(det)->hideStaticSpr(2);
	_G(det)->startDetail(2, 255, false);
	startAadWait(471);
	flic_cut(FCUT_097);
	flic_cut(FCUT_098);
	flic_cut(FCUT_099);
	flic_cut(FCUT_100);
	_G(gameState).PersonGlobalDia[P_HOWARD] = -1;
	_G(gameState).PersonDiaRoom[P_HOWARD] = false;
	_G(gameState).flags32_10 = true;

	switchRoom(86);
	showCur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
