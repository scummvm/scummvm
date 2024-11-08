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
#include "chewy/rooms/room66.h"

namespace Chewy {
namespace Rooms {

void Room66::entry(int16 eib_nr) {
	_G(gameState).ScrollxStep = 2;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(gameState).ZoomXy[P_HOWARD][0] = 30;
	_G(gameState).ZoomXy[P_HOWARD][1] = 50;
	_G(gameState).ZoomXy[P_NICHELLE][0] = 36;
	_G(gameState).ZoomXy[P_NICHELLE][1] = 50;
	_G(zoom_horizont) = 130;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	if (!_G(gameState).flags26_4) {
		_G(det)->showStaticSpr(4);
		_G(det)->showStaticSpr(8);
		hideCur();
		_G(gameState).flags26_4 = true;
		_G(gameState).scrollx = 476;
		setPersonPos(598, 101, P_CHEWY, P_RIGHT);
		setPersonPos(644, 82, P_NICHELLE, P_LEFT);
		setPersonPos(623, 81, P_HOWARD, P_LEFT);
		startAadWait(413);
		autoMove(9, P_CHEWY);
		_G(SetUpScreenFunc) = setup_func;
		startAadWait(403);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(0, 1, ANI_FRONT);
		_G(det)->showStaticSpr(14);
		waitShowScreen(15);
		_G(det)->hideStaticSpr(14);
		startSetAILWait(1, 1, ANI_FRONT);
		load_chewy_taf(CHEWY_NORMAL);
		_G(gameState)._personHide[P_CHEWY] = false;
		start_spz(CH_TALK12, 255, false, P_CHEWY);
		startAadWait(404);
		startAadWait(415);
		showCur();
	} else if (!_G(flags).LoadGame && _G(gameState)._personRoomNr[P_HOWARD] == 66) {
		switch (eib_nr) {
		case 96:
			setPersonPos(488, 114, P_HOWARD, P_RIGHT);
			setPersonPos(520, 114, P_NICHELLE, P_RIGHT);
			break;
		case 97:
			setPersonPos(22, 114, P_HOWARD, P_RIGHT);
			setPersonPos(50, 114, P_NICHELLE, P_RIGHT);
			break;
		case 101:
			setPersonPos(150, 114, P_HOWARD, P_RIGHT);
			setPersonPos(182, 114, P_NICHELLE, P_RIGHT);
			break;
		default:
			break;
		}
		
	}
	_G(SetUpScreenFunc) = setup_func;
}

void Room66::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;
	_G(atds)->setControlBit(415, ATS_ACTIVE_BIT);
	_G(atds)->setControlBit(417, ATS_ACTIVE_BIT);
	if (_G(gameState)._personRoomNr[P_HOWARD] != 66)
		return;

	switch (eib_nr) {
	case 98:
		_G(gameState)._personRoomNr[P_HOWARD] = 69;
		_G(gameState)._personRoomNr[P_NICHELLE] = 69;
		break;

	case 99:
		_G(gameState)._personRoomNr[P_HOWARD] = 68;
		_G(gameState)._personRoomNr[P_NICHELLE] = 68;
		break;

	case 100:
		_G(gameState)._personRoomNr[P_HOWARD] = 67;
		_G(gameState)._personRoomNr[P_NICHELLE] = 67;
		break;

	default:
		break;
	}
}

void Room66::setup_func() {
	calc_person_look();
	const int posX = _G(moveState)[P_CHEWY].Xypos[0];

	int edx, esi;
	if (posX < 30) {
		edx = 57;
		esi = 97;
	} else if (posX < 260){
		edx = 170;
		esi = 263;
	} else if (posX < 370) {
		edx = 314;
		esi = 398;
	} else if (posX < 500) {
		edx = 517;
		esi = 556;
	} else {
		edx = 607;
		esi = 690;
	}

	goAutoXy(edx, 114, P_HOWARD, ANI_GO);
	goAutoXy(esi, 114, P_NICHELLE, ANI_GO);

	if (posX >= 500 || _G(gameState).flags26_8)
		return;

	_G(gameState).flags26_8 = true;
	_G(det)->startDetail(9, 5, false);
	startAadWait(405);
}

void Room66::talk1() {
	hideCur();
	autoMove(5, P_CHEWY);
	startAadWait(407);
	showCur();
}

void Room66::talk2() {
	proc8(6, 10, 11, 408);
}

void Room66::talk3() {
	proc8(8, 6, 7, 409);
}

void Room66::talk4() {
	proc8(7, 2, 3, _G(gameState).flags26_20 ? 414 : 410);
}

int Room66::proc2() {
	hideCur();
	autoMove(0, P_CHEWY);
	_G(gameState).flags26_40 = true;
	_G(gameState).room_e_obj[100].Attribut = 3;
	_G(atds)->set_all_ats_str(423, 1, ANI_GO);
	showCur();
	
	return 0;
}

int Room66::proc7() {
	if (!isCurInventory(ARTIFACT_INV))
		return 0;

	hideCur();
	autoMove(7, P_CHEWY);
	if (_G(gameState).changedArtifactOrigin) {
		delInventory(_G(cur)->getInventoryCursor());
		invent_2_slot(92);
		invent_2_slot(93);
		invent_2_slot(94);
		_G(gameState).flags26_20 = true;
	}

	proc8(7, 2, 3, 411 + (_G(gameState).changedArtifactOrigin ? 1 : 0));
	hideCur();
	if (_G(gameState).flags26_20)
		startSetAILWait(4, 1, ANI_FRONT);
	showCur();

	return 1;
}

void Room66::proc8(int chewyAutoMovNr, int restartAniNr, int transitionAniNr, int transitionDiaNr) {
	hideCur();

	if (chewyAutoMovNr != -1)
		autoMove(chewyAutoMovNr, P_CHEWY);

	_G(room)->set_timer_status(restartAniNr, TIMER_STOP);
	_G(det)->del_static_ani(restartAniNr);
	_G(det)->set_static_ani(transitionAniNr, -1);
	startAadWait(transitionDiaNr);
	_G(det)->del_static_ani(transitionAniNr);
	_G(det)->set_static_ani(restartAniNr, -1);
	_G(room)->set_timer_status(restartAniNr, TIMER_START);
	showCur();
}

} // namespace Rooms
} // namespace Chewy
