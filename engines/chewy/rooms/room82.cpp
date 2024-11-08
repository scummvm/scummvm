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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/room.h"

#include "chewy/rooms/room82.h"

#include "chewy/cursor.h"
#include "chewy/rooms/room66.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room82::entry() {
	_G(det)->playSound(0, 0);
	_G(gameState).ScrollxStep = 2;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;

	if (_G(gameState).R88UsedMonkey) {
		_G(det)->showStaticSpr(6);
		_G(atds)->setControlBit(473, ATS_ACTIVE_BIT);
		_G(atds)->setControlBit(467, ATS_ACTIVE_BIT);
	}

	if (_G(gameState).flags37_20)
		_G(det)->showStaticSpr(7);

	if (_G(flags).LoadGame) {
		_G(SetUpScreenFunc) = setup_func;
		return;
	}

	if (_G(gameState).flags30_1) {
		setPersonPos(635, 144, P_CHEWY, P_LEFT);
		setPersonPos(592, 115, P_HOWARD, P_RIGHT);
		setPersonPos(543, 110, P_NICHELLE, P_RIGHT);
		_G(gameState).flags30_1 = false;
		_G(gameState).scrollx = 479;
	} else if (_G(gameState).flags31_8) {
		setPersonPos(130, 138, P_CHEWY, P_LEFT);
		setPersonPos(104, 111, P_HOWARD, P_RIGHT);
		setPersonPos(153, 110, P_NICHELLE, P_RIGHT);
		_G(gameState).flags31_8 = false;
		_G(gameState).scrollx = 0;
	}

	if (_G(gameState).flags30_8) {
		_G(SetUpScreenFunc) = setup_func;
		proc4();
	}

	_G(mouseLeftClick) = false;
	_G(SetUpScreenFunc) = setup_func;
}

void Room82::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;

	if (eib_nr == 122) {
		_G(gameState).R79Val[P_CHEWY] = 1;
		_G(gameState).R79Val[P_HOWARD] = 1;
		_G(gameState).R79Val[P_NICHELLE] = 1;
		_G(gameState)._personRoomNr[P_HOWARD] = 80;
		_G(gameState)._personRoomNr[P_NICHELLE] = 80;
	}
}

void Room82::setup_func() {
	calc_person_look();

	const int posX = _G(moveState)[P_CHEWY].Xypos[0];
	int howDestX, nicDestX;
	
	if (posX < 200) {
		howDestX = 104;
		nicDestX = 150;
	} else if (posX < 387) {
		howDestX = 280;
		nicDestX = 379;
	} else {
		howDestX = 592;
		nicDestX = 543;
	}

	goAutoXy(howDestX, 111, P_HOWARD, ANI_GO);
	goAutoXy(nicDestX, 110, P_NICHELLE, ANI_GO);
}

void Room82::talkWithDirector() {
	if (_G(gameState).flags30_40)
		return;

	int transitionDiaNr, transitionAniNr;
	if (!_G(gameState).flags30_10) {
		transitionDiaNr = 446;
		transitionAniNr = 8;
	} else if (!_G(gameState).flags30_20) {
		transitionDiaNr = 454;
		transitionAniNr = 7;
	} else {
		transitionDiaNr = 449;
		transitionAniNr = 7;

		_G(gameState).flags30_40 = true;
		_G(atds)->set_all_ats_str(473, 1, ATS_DATA);
		_G(atds)->set_all_ats_str(467, 1, ATS_DATA);		
	}

	Room66::proc8(2, 7, transitionAniNr, transitionDiaNr);
}

void Room82::talkWithFilmDiva() {
	hideCur();
	autoMove(3, P_CHEWY);
	startAadWait(453);
	showCur();
}

void Room82::talk3() {
	Room66::proc8(4, 0, 1, 452);
}

int Room82::proc3() {
	if (!isCurInventory(FOOD_CERTIFICATE_INV))
		return 0;

	hideCur();
	autoMove(4, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	_G(room)->set_timer_status(0, TIMER_STOP);
	_G(det)->del_static_ani(0);
	startSetAILWait(2, 1, ANI_FRONT);
	startSetAILWait(3, 1, ANI_FRONT);
	_G(det)->set_static_ani(0, -1);
	start_spz_wait(13, 1, false, P_CHEWY);

	delInventory(_G(cur)->getInventoryCursor());
	new_invent_2_cur(104);
	showCur();

	return 1;
}

void Room82::proc4() {
	hideCur();
	_G(gameState).flags30_8 = true;
	autoMove(5, P_CHEWY);
	_G(flags).NoScroll = true;
	auto_scroll(270, 0);
	_G(room)->set_timer_status(7, TIMER_STOP);
	_G(det)->del_static_ani(7);
	_G(det)->set_static_ani(8, -1);
	_G(room)->set_timer_status(4, TIMER_STOP);
	_G(det)->del_static_ani(4);
	_G(det)->set_static_ani(5, 3);
	startAadWait(445);
	_G(det)->del_static_ani(5);
	_G(det)->set_static_ani(4, -1);
	_G(room)->set_timer_status(4, TIMER_START);
	_G(det)->del_static_ani(8);
	_G(det)->set_static_ani(7, -1);
	_G(room)->set_timer_status(7, TIMER_START);
	_G(flags).NoScroll = false;

	showCur();
}

int Room82::proc6() {
	if (!isCurInventory(105) && !isCurInventory(106))
		return 0;

	hideCur();
	autoMove(2, P_CHEWY);

	if (_G(gameState).flags30_10) {
		startAadWait(450);
		_G(out)->fadeOut();
		_G(out)->set_partialpalette(_G(pal), 255, 1);
		_G(atds)->enableEvents(false);
		startAadWait(598);
		_G(atds)->enableEvents(true);
		_G(det)->showStaticSpr(7);
		_G(fx_blend) = BLEND3;
		setupScreen(DO_SETUP);
		Room66::proc8(2, 7, 7, 451);
		_G(gameState).flags30_20 = true;
		_G(gameState).flags37_20 = true;
		delInventory(_G(cur)->getInventoryCursor());
		remove_inventory(105);
		remove_inventory(106);
	} else {
		start_spz(CH_TALK12, 255, false, P_CHEWY);
		startAadWait(276);
	}

	showCur();
	return 1;
}

void Room82::proc8() {
	hideCur();
	_G(gameState).flags30_10 = true;
	startAadWait(447);
	autoMove(6, P_CHEWY);

	while (_G(moveState)[P_NICHELLE].Count != 0)
		setupScreen(DO_SETUP);

	_G(gameState)._personHide[P_NICHELLE] = true;
	_G(det)->startDetail(10, 255, false);
	startAadWait(625);
	_G(det)->stopDetail(10);
	_G(gameState)._personHide[P_NICHELLE] = false;
	startAadWait(448);
	_G(det)->del_static_ani(4);
	startSetAILWait(6, 2, ANI_FRONT);
	_G(det)->set_static_ani(4, -1);
	showCur();
}

int Room82::procClimbLadderToGorilla() {
	if (_G(cur)->usingInventoryCursor() || !_G(gameState).flags30_40)
		return 0;

	autoMove(7, P_CHEWY);
	_G(gameState)._personRoomNr[P_HOWARD] = 88;
	_G(gameState)._personRoomNr[P_NICHELLE] = 88;
	switchRoom(88);
	_G(menu_item) = CUR_WALK;
	cursorChoice(CUR_WALK);

	return 1;
}

} // namespace Rooms
} // namespace Chewy
