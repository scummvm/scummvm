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
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room5.h"

namespace Chewy {
namespace Rooms {

void Room5::entry() {
	if (_G(spieler).R5Terminal)
		det->start_detail(6, 255, 0);
}

void Room5::pushButton() {
	int16 str_nr;
	if (_G(spieler).R5Terminal) {
		if (_G(spieler).R5Tuer == false) {
			start_detail_wait(9, 1, ANI_VOR);
			_G(spieler).room_e_obj[6].Attribut = AUSGANG_OBEN;
			str_nr = 1;
		} else {
			start_detail_wait(9, 1, ANI_RUECK);
			_G(spieler).room_e_obj[6].Attribut = 255;
			str_nr = 0;
		}
		atds->set_ats_str(29, str_nr, ATS_DATEI);
		_G(spieler).R5Tuer ^= 1;
		obj->calc_rsi_flip_flop(SIB_TUERE_R5);
	} else {
		start_aad_wait(1, -1);
	}
}

} // namespace Rooms
} // namespace Chewy
