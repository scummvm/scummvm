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
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room53.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK35[7] = {
	{ 2, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 7, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 8, 3, ANI_FRONT, ANI_WAIT, 0 },
	{ 4, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 5, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 9, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 5, 1, ANI_FRONT, ANI_WAIT, 0 },
};


void Room53::entry() {
	hideCur();
	_G(obj)->hide_sib(SIB_VISIT_R53);
	_G(atds)->delControlBit(319, ATS_ACTIVE_BIT);
	startSetAILWait(0, 1, ANI_FRONT);
	_G(det)->set_static_ani(1, -1);
	_G(timer_nr)[0] = _G(room)->set_timer(1, 7);
	showCur();
}

void Room53::man_go() {
	_G(room)->set_timer_status(1, TIMER_STOP);
	_G(det)->del_static_ani(1);
	startSetAILWait(5, 1, ANI_FRONT);
	_G(atds)->setControlBit(319, ATS_ACTIVE_BIT);
	if (!_G(gameState).R53Visit)
		_G(obj)->show_sib(SIB_VISIT_R53);
}

void Room53::talk_man() {
	hideCur();
	autoMove(2, P_CHEWY);
	_G(room)->set_timer_status(1, TIMER_STOP);
	_G(det)->del_static_ani(1);
	startSetAILWait(2, 1, ANI_FRONT);
	_G(det)->set_static_ani(3, -1);
	startAadWait(269 + (_G(gameState).R53Kostuem ? 1 : 0));
	_G(det)->del_static_ani(3);
	startSetAILWait(4, 1, ANI_FRONT);
	man_go();
	showCur();
}

int16 Room53::use_man() {
	int16 action_ret = false;

	if (isCurInventory(BESTELL_INV)) {
		action_ret = true;
		hideCur();
		_G(gameState).R53Kostuem = true;
		delInventory(_G(gameState).AkInvent);
		autoMove(2, P_CHEWY);
		startAadWait(271);
		_G(room)->set_timer_status(1, TIMER_STOP);
		_G(det)->del_static_ani(1);
		startAniBlock(7, ABLOCK35);
		_G(obj)->addInventory(JMKOST_INV, &_G(room_blk));
		inventory_2_cur(JMKOST_INV);
		_G(atds)->setControlBit(319, ATS_ACTIVE_BIT);
		showCur();
	}

	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
