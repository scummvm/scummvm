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
#include "chewy/rooms/room64.h"

namespace Chewy {
namespace Rooms {

void Room64::entry() {
	_G(SetUpScreenFunc) = setup_func;
	_G(r62Delay) = 0;
	_G(r64TalkAni) = _G(gameState).R64Moni1Ani;

	if (!_G(gameState).R64AshtrayTaken)
		_G(det)->startDetail(0, 255, false);
	
	if (!_G(gameState).R64ManAway) {
		_G(timer_nr)[0] = _G(room)->set_timer(1, 10);
		_G(det)->set_static_ani(1, -1);
		chewy_entry();
		if (_G(gameState).R63Fire) {
			flic_cut(FCUT_078);
			_G(gameState).R64ManAway = true;
			_G(det)->del_static_ani(1);
			_G(room)->set_timer_status(1, TIMER_STOP);
			_G(atds)->setControlBit(376, ATS_ACTIVE_BIT);
			_G(gameState).R64Moni1Ani = 5;
			calc_monitor();
			hideCur();
			startAadWait(354);
			showCur();
		}
	} else
		chewy_entry();
}

void Room64::chewy_entry() {
	calc_monitor();
	if (!_G(flags).LoadGame) {
		hideCur();
		_G(det)->showStaticSpr(3);
		autoMove(2, P_CHEWY);
		_G(det)->hideStaticSpr(3);
		showCur();
	}
}

void Room64::calc_monitor() {
	int16 str_nr = 0;
	switch (_G(gameState).R64Moni1Ani) {
	case 0:
		str_nr = 2;
		break;

	case 3:
		str_nr = 0;
		_G(det)->set_static_ani(3, -1);
		break;

	case 5:
		str_nr = 1;
		_G(det)->startDetail(5, 255, ANI_FRONT);
		break;

	default:
		break;
	}

	_G(atds)->set_all_ats_str(373, str_nr, ATS_DATA);
	switch (_G(gameState).R64Moni2Ani) {
	case 0:
		str_nr = 1;
		break;

	case 4:
		str_nr = 0;
		_G(det)->set_static_ani(4, -1);
		break;

	default:
		break;
	}
	_G(atds)->set_all_ats_str(374, str_nr, ATS_DATA);
}

void Room64::setup_func() {
	if (_G(r62Delay) <= 0 && _G(gameState).R62TShow) {
		_G(r62Delay) = (_G(gameState).DelaySpeed + 1) * 60;
		if (_G(r64TalkAni) == 3 || _G(r64TalkAni) == 4)
			_G(det)->stopDetail(_G(r64TalkAni));
		
		if (_G(r64TalkAni) == 4)
			_G(r64TalkAni) = _G(gameState).R64Moni1Ani;
		else
			_G(r64TalkAni) = _G(gameState).R64Moni2Ani;

		if (_G(r64TalkAni) != 0)
			_G(det)->startDetail(_G(r64TalkAni), 255, ANI_FRONT);
	} else
		--_G(r62Delay);
}

void Room64::talk_man(int16 aad_nr) {
	if (!_G(gameState).R64ManAway) {
		hideCur();
		autoMove(4, P_CHEWY);
		_G(room)->set_timer_status(1, TIMER_STOP);
		_G(det)->del_static_ani(1);
		_G(det)->stopDetail(1);
		_G(det)->set_static_ani(2, -1);
		startAadWait(aad_nr);
		_G(det)->del_static_ani(2);
		_G(room)->set_timer_status(1, TIMER_START);
		_G(det)->set_static_ani(1, -1);
		showCur();
	}
}

int16 Room64::useBag() {
	int16 action_ret = false;
	hideCur();
	if (!_G(cur)->usingInventoryCursor()) {
		if (_G(gameState).R64ManAway) {
			if (!_G(atds)->getControlBit(375, ATS_ACTIVE_BIT)) {
				autoMove(3, P_CHEWY);
				start_spz_wait(CH_ROCK_GET1, 1, false, P_CHEWY);
				new_invent_2_cur(GERAET_INV);
				_G(atds)->setControlBit(375, ATS_ACTIVE_BIT);
				startAadWait(353);
			} else {
				showCur();
				return 0;
			}
		} else {
			autoMove(3, P_CHEWY);
			_G(room)->set_timer_status(1, TIMER_STOP);
			_G(det)->del_static_ani(1);
			_G(det)->stopDetail(1);
			_G(det)->startDetail(6, 255, false);
			startAadWait(352);
			_G(det)->stopDetail(6);
			_G(room)->set_timer_status(0, TIMER_START);
			_G(det)->set_static_ani(1, -1);
		}
		
		action_ret = true;
	}
	showCur();
	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
