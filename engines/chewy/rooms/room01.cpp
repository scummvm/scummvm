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
#include "chewy/global.h"
#include "chewy/room.h"
#include "chewy/rooms/room01.h"

namespace Chewy {
namespace Rooms {

void Room1::gottenCard() {
	det->hide_static_spr(2);
	start_detail_wait(4, 1, ANI_VOR);
	_G(spieler).PersonHide[P_CHEWY] = false;
	atds->del_steuer_bit(7, ATS_COUNT_BIT, ATS_DATEI);
	int16 tmp;
	atds->ats_get_txt(7, TXT_MARK_LOOK, &tmp, ATS_DATEI);
}

void Room1::gedAction(int index) {
	#define KABELABDECKUNG 1

	switch (index) {
	case 0:
		if (!_G(spieler).R2ElectrocutedBork) {
			bool flag = false;
			if (_G(spieler).AkInvent == KABEL_INV) {
				flag = true;
				del_inventar(_G(spieler).AkInvent);
			} else if (obj->check_inventar(KABEL_INV)) {
				flag = true;
				obj->del_obj_use(KABEL_INV);
				del_invent_slot(KABEL_INV);
			}

			if (flag) {
				start_aad_wait(54, -1);
				atds->set_ats_str(8, TXT_MARK_LOOK, 0, ATS_DATEI);
				_G(spieler).room_s_obj[KABELABDECKUNG].ZustandFlipFlop = 2;
				obj->calc_rsi_flip_flop(KABELABDECKUNG);
				obj->calc_all_static_detail();
			}
		}
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Chewy
