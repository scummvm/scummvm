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
#include "chewy/rooms/room09.h"

namespace Chewy {
namespace Rooms {

static const int16 SURIMY_PHASEN[4][2] = {
	{ 91, 98 },
	{ 91, 98 },
	{ 91, 98 },
	{ 91, 98 }
};

static const MovLine SURIMY_MPKT[2] = {
	{ {  80, 170, 199 }, 2, 6 },
	{ { 210, 162, 199 }, 2, 6 }
};

static const MovLine SURIMY_MPKT1[2] = {
	{ { 224, 158, 199 }, 2, 6 },
	{ { 330, 162, 199 }, 2, 6 }
};

void Room9::entry() {
	_G(gameState).R7ChewyFlug = false;

	if (!_G(gameState).R9Grid)
		setPersonPos(138, 91, P_CHEWY, P_LEFT);
	else
		_G(det)->showStaticSpr(5);

	if (_G(gameState).R9Surimy) {
		_G(det)->hideStaticSpr(4);
		_G(room)->set_timer_status(7, TIMER_STOP);
	}
}

void Room9::gtuer() {
	_G(gameState).R9Grid = true;
	_G(det)->showStaticSpr(5);
	startSetAILWait(6, 1, ANI_FRONT);
	setPersonPos(74, 93, P_CHEWY, P_LEFT);
	_G(atds)->delControlBit(74, ATS_ACTIVE_BIT);
	_G(atds)->delControlBit(75, ATS_ACTIVE_BIT);
	_G(atds)->delControlBit(76, ATS_ACTIVE_BIT);
	_G(obj)->show_sib(34);
	_G(atds)->set_all_ats_str(73, 1, ATS_DATA);
}

void Room9::surimy() {
	hideCur();
	_G(gameState).R9Surimy = true;
	const int16 tmp = _G(moveState)[P_CHEWY].Count;
	stopPerson(P_CHEWY);
	_G(atds)->setControlBit(75, ATS_ACTIVE_BIT);
	_G(det)->hideStaticSpr(4);
	_G(room)->set_timer_status(7, TIMER_STOP);
	surimy_ani();
	start_spz(CH_TALK11, 255, false, P_CHEWY);
	startAadWait(56);
	_G(gameState).room_e_obj[17].Attribut = EXIT_RIGHT;
	_G(moveState)[P_CHEWY].Count = tmp;
	get_phase(&_G(moveState)[P_CHEWY], &_G(spieler_mi)[P_CHEWY]);
	_G(mov)->continue_auto_go();
	showCur();
}

void Room9::surimy_ani() {
	_G(det)->load_taf_seq(91, 8, nullptr);
	_G(auto_obj) = 1;
	_G(mov_phasen)[SURIMY_OBJ].AtsText = 0;
	_G(mov_phasen)[SURIMY_OBJ].Lines = 2;
	_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
	_G(mov_phasen)[SURIMY_OBJ].ZoomFak = 0;
	_G(auto_mov_obj)[SURIMY_OBJ].Id = AUTO_OBJ0;
	_G(auto_mov_vector)[SURIMY_OBJ].Delay = _G(gameState).DelaySpeed;
	_G(auto_mov_obj)[SURIMY_OBJ].Mode = true;
	init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], 2, (const MovLine *)SURIMY_MPKT);
	wait_auto_obj(SURIMY_OBJ);
	startDetailFrame(0, 1, ANI_FRONT, 15);
	_G(det)->startDetail(2, 1, ANI_FRONT);
	waitDetail(0);
	startSetAILWait(1, 1, ANI_FRONT);
	start_spz(CH_EKEL, 2, ANI_FRONT, P_CHEWY);
	_G(det)->hideStaticSpr(4);
	_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
	init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], _G(mov_phasen)[SURIMY_OBJ].Lines, (const MovLine *)SURIMY_MPKT1);
	wait_auto_obj(SURIMY_OBJ);
	_G(det)->del_taf_tbl(91, 8, nullptr);
}

void Room9::gedAction(int index) {
	if (index == 0 && !_G(gameState).R9Surimy)
		surimy();
}

} // namespace Rooms
} // namespace Chewy
