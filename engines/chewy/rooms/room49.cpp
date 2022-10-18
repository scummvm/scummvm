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
#include "chewy/rooms/room49.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK34[2] = {
	{ 7, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 6, 1, ANI_FRONT, ANI_WAIT, 0 },
};


void Room49::entry(int16 eib_nr) {
	_G(zoom_horizont) = 110;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(gameState).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;

	if (!_G(gameState).R49BoyWeg) {
		_G(timer_nr)[0] = _G(room)->set_timer(255, 2);
	} else {
		_G(det)->del_static_ani(0);
		_G(det)->hideStaticSpr(5);
		_G(det)->del_static_ani(0);
		_G(det)->del_static_ani(1);
	}

	_G(gameState).ZoomXy[P_HOWARD][0] = 30;
	_G(gameState).ZoomXy[P_HOWARD][1] = 30;
	_G(spieler_mi)[P_HOWARD].Mode = true;

	if (!_G(flags).LoadGame) {
		if (_G(gameState).R48TaxiEntry) {
			_G(gameState).R48TaxiEntry = false;
			setPersonPos(527, 76, P_HOWARD, P_LEFT);
			setPersonPos(491, 98, P_CHEWY, P_LEFT);
			_G(mouseLeftClick) = false;
			_G(gameState).scrollx = 320;
			_G(gameState).scrolly = 0;
			_G(det)->startDetail(5, 1, ANI_FRONT);
		} else if (eib_nr == 83) {
			setPersonPos(377, 78, P_HOWARD, P_LEFT);
		}
	}
}

void Room49::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 49) {
		_G(spieler_mi)[P_HOWARD].Mode = false;
		if (eib_nr == 80) {
			_G(gameState)._personRoomNr[P_HOWARD] = 50;
		} else if (eib_nr == 81) {
			_G(SetUpScreenFunc) = nullptr;
			startAadWait(268);
		}
	}
}

bool Room49::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		calc_boy_ani();

	return false;
}

void Room49::gedAction(int index) {
	if (!index)
		calc_boy();
}

void Room49::calc_boy_ani() {
	if (!_G(gameState).R49BoyAniCount) {
		_G(gameState).R49BoyAniCount = 3;
		_G(gameState).R49BoyAni = true;
		_G(det)->del_static_ani(0);
	} else {
		--_G(gameState).R49BoyAniCount;
		_G(gameState).R49BoyAni = false;
		_G(det)->del_static_ani(1);
	}

	_G(det)->set_static_ani(_G(gameState).R49BoyAni ? 1 : 0, -1);
	_G(det)->startDetail(_G(gameState).R49BoyAni ? 1 : 0, 1, ANI_FRONT);
	_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
}

void Room49::calc_boy() {
	if (!_G(gameState).R49WegFrei) {
		hideCur();
		_G(room)->set_timer_status(255, TIMER_STOP);
		_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
		stopPerson(P_CHEWY);
		stopPerson(P_HOWARD);
		_G(person_end_phase)[P_CHEWY] = P_LEFT;
		_G(det)->stopDetail(_G(gameState).R49BoyAni ? 1 : 0);
		_G(det)->del_static_ani(_G(gameState).R49BoyAni ? 1 : 0);
		_G(det)->set_static_ani(2, -1);

		_G(SetUpScreenFunc) = nullptr;
		startAadWait(262);
		_G(SetUpScreenFunc) = nullptr;
		autoMove(3, P_CHEWY);
		goAutoXy(374, 79, P_HOWARD, ANI_WAIT);
		setPersonSpr(P_LEFT, P_HOWARD);
		_G(det)->del_static_ani(2);
		startSetAILWait(3, 1, ANI_FRONT);

		_G(det)->showStaticSpr(9);
		_G(gameState)._personHide[P_HOWARD] = true;
		startSetAILWait(8, 1, ANI_FRONT);
		_G(gameState)._personHide[P_HOWARD] = false;
		_G(det)->hideStaticSpr(9);
		startSetAILWait(4, 1, ANI_GO);

		_G(SetUpScreenFunc) = setup_func;
		_G(det)->set_static_ani(_G(gameState).R49BoyAni ? 1 : 0, -1);
		_G(room)->set_timer_status(255, TIMER_START);
		showCur();
	}
}

int16 Room49::use_boy() {
	int16 action_ret = false;

	if (isCurInventory(SPARK_INV)) {
		action_ret = true;
		hideCur();
		autoMove(3, P_CHEWY);
		delInventory(_G(cur)->getInventoryCursor());
		talk_boy(265);
		_G(gameState).R49WegFrei = true;
		_G(gameState).room_e_obj[80].Attribut = EXIT_TOP;
		showCur();

	} else if (isCurInventory(CIGAR_INV)) {
		action_ret = true;
		use_boy_cigar();
	}

	return action_ret;
}

void Room49::use_boy_cigar() {
	hideCur();
	delInventory(_G(cur)->getInventoryCursor());
	talk_boy(263);
	_G(SetUpScreenFunc) = nullptr;
	autoMove(5, P_CHEWY);

	const int16 zoom = _G(room)->_roomInfo->_zoomFactor;
	_G(room)->set_zoom(zoom);
	goAutoXy(416, 79, P_HOWARD, ANI_WAIT);
	setPersonSpr(P_LEFT, P_HOWARD);
	_G(flags).NoScroll = true;

	auto_scroll(164, 0);
	flic_cut(FCUT_067);
	register_cutscene(17);
	
	_G(room)->set_timer_status(255, TIMER_STOP);
	_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
	_G(det)->del_static_ani(_G(gameState).R49BoyAni ? 1 : 0);
	_G(det)->stopDetail(_G(gameState).R49BoyAni ? 1 : 0);

	_G(flags).NoScroll = false;
	setPersonSpr(P_RIGHT, P_CHEWY);
	startAadWait(264);
	_G(room)->set_zoom(zoom);

	_G(obj)->addInventory(GUM_INV, &_G(room_blk));
	inventory_2_cur(GUM_INV);
	_G(atds)->setControlBit(318, ATS_ACTIVE_BIT);

	_G(SetUpScreenFunc) = setup_func;
	_G(gameState).R49BoyWeg = true;
	showCur();
}

void Room49::talk_boy() {
	if (!_G(gameState).R49BoyWeg) {
		autoMove(3, P_CHEWY);
		talk_boy(266 + (_G(gameState).R49WegFrei ? 1 : 0));
	}
}

void Room49::talk_boy(int16 aad_nr) {
	if (!_G(gameState).R49BoyWeg) {
		_G(room)->set_timer_status(255, TIMER_STOP);
		_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
		_G(det)->stopDetail(_G(gameState).R49BoyAni ? 1 : 0);

		_G(det)->set_static_ani(_G(gameState).R49BoyAni ? 1 : 0, -1);
		_G(det)->set_static_ani(2, -1);
		_G(SetUpScreenFunc) = nullptr;
		stopPerson(P_HOWARD);
		startAadWait(aad_nr);
		_G(SetUpScreenFunc) = setup_func;
		_G(det)->del_static_ani(2);
		_G(det)->set_static_ani(_G(gameState).R49BoyAni ? 1 : 0, -1);
		_G(room)->set_timer_status(255, TIMER_START);
	}
}

void Room49::look_hotel() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 49) {
		_G(SetUpScreenFunc) = nullptr;
		stopPerson(P_HOWARD);
		startAadWait(261);
		_G(SetUpScreenFunc) = setup_func;
	}
}

int16 Room49::use_taxi() {
	int16 action_ret = false;

	if (!_G(cur)->usingInventoryCursor()) {
		action_ret = true;
		hideCur();
		startAniBlock(2, ABLOCK34);
		_G(det)->showStaticSpr(7);
		autoMove(2, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(gameState).R48TaxiPerson[P_CHEWY] = true;

		if (_G(gameState)._personRoomNr[P_HOWARD] == 49) {
			goAutoXy(507, 74, P_HOWARD, ANI_WAIT);
			goAutoXy(490, 58, P_HOWARD, ANI_WAIT);
			_G(gameState)._personHide[P_HOWARD] = true;
			_G(gameState).R48TaxiPerson[P_HOWARD] = true;
			_G(gameState)._personRoomNr[P_HOWARD] = 48;
		}

		_G(det)->hideStaticSpr(7);
		startSetAILWait(5, 1, ANI_FRONT);
		_G(det)->stopSound(0);
		switchRoom(48);
	}

	return action_ret;
}

void Room49::setup_func() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 49) {
		calc_person_look();
		const int16 ch_x = _G(moveState)[P_CHEWY].Xypos[0];

		int16 x, y;
		if (ch_x < 130) {
			x = 40;
			y = 97;
		} else if (ch_x < 312) {
			x = 221;
			y = 94;
		} else if (ch_x < 445) {
			x = 342;
			y = 93;
		} else {
			x = 536;
			y = 90;
		}

		goAutoXy(x, y, P_HOWARD, ANI_GO);
	}
}

} // namespace Rooms
} // namespace Chewy
