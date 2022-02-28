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
#include "chewy/rooms/room27.h"

namespace Chewy {
namespace Rooms {

void Room27::entry() {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 27) {
		_G(timer_nr)[0] = _G(room)->set_timer(0, 5);
		_G(det)->set_static_ani(0, -1);
		_G(atds)->del_steuer_bit(274, ATS_AKTIV_BIT, ATS_DATA);
	} else {
		_G(atds)->set_steuer_bit(274, ATS_AKTIV_BIT, ATS_DATA);
	}

	_G(spieler).PersonHide[P_HOWARD] = true;
	_G(spieler).ScrollxStep = 2;
}

void Room27::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;
	hideCur();

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 27) {
		if (eib_nr == 55) {
			start_aad_wait(175, -1);
			_G(room)->set_timer_status(0, TIMER_STOP);
			_G(det)->del_static_ani(0);
			start_detail_wait(2, 1, ANI_FRONT);
			_G(spieler).PersonRoomNr[P_HOWARD] = 28;
			_G(spieler_mi)[P_HOWARD].Id = HOWARD_OBJ;
		} else if (_G(spieler).R27HowardGed < 3 && eib_nr != -1) {
			++_G(spieler).R27HowardGed;
			start_aad_wait(174, -1);
		}
	}

	showCur();
}

void Room27::get_surimy() {
	_G(obj)->calc_all_static_detail();
	hideCur();
	autoMove(4, P_CHEWY);

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 27) {
		start_aad_wait(171, -1);
	}

	showCur();
	_G(obj)->hide_sib(SIB_SURIMY_R27);
}

void Room27::talk_howard() {
	hideCur();
	autoMove(4, P_CHEWY);
	start_aad_wait(173, -1);
	showCur();
}

void Room27::howard_ged() {
	if (_G(spieler).R27HowardGed < 3) {
		++_G(spieler).R27HowardGed;
	}
}

} // namespace Rooms
} // namespace Chewy
