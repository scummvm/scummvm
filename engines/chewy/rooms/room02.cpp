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
#include "chewy/rooms/room02.h"

namespace Chewy {
namespace Rooms {

#define ANI_5 5
#define GITTER_BLITZEN 7

static const AniBlock ABLOCK4[2] = {
	{ GITTER_BLITZEN, 3, ANI_VOR, ANI_WAIT, 0 },
	{ GITTER_BLITZEN, 12, ANI_VOR, ANI_GO, 0 }
};


void Room2::entry() {
	if (!_G(spieler).R2ElectrocutedBork)
		_G(det)->start_detail(5, 255, ANI_VOR);
}

void Room2::jump_out_r1(int16 nr) {
	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(nr, 1, ANI_VOR);
	set_up_screen(DO_SETUP);
	_G(det)->stop_detail(6);
	set_person_pos(32, 127, P_CHEWY, P_LEFT);
	_G(spieler).PersonHide[P_CHEWY] = false;
	clear_prog_ani();
	switch_room(1);
	check_shad(2, 1);
}

void Room2::electrifyWalkway1() {
	_G(det)->start_detail(ANI_5, 255, ANI_VOR);
	start_spz(CH_TALK6, 255, false, ANI_VOR);
	start_aad_wait(49, -1);
	_G(det)->stop_detail(ANI_5);

	_G(det)->start_detail(GITTER_BLITZEN, 12, ANI_VOR);
	_G(spieler).R2ElectrocutedBork = true;
	del_inventar(_G(spieler).AkInvent);

	_G(atds)->del_steuer_bit(11, ATS_COUNT_BIT, ATS_DATEI);
	_G(atds)->del_steuer_bit(11, ATS_ACTION_BIT, ATS_DATEI);
	_G(atds)->del_steuer_bit(19, ATS_COUNT_BIT, ATS_DATEI);
	_G(atds)->del_steuer_bit(25, ATS_AKTIV_BIT, ATS_DATEI);
	_G(atds)->set_steuer_bit(8, ATS_COUNT_BIT, ATS_DATEI);
	_G(atds)->set_ats_str(11, 1, ATS_DATEI);
}

void Room2::electrifyWalkway2() {
	start_spz(CH_TALK12, 255, false, P_CHEWY);
	start_aad_wait(47, -1);
}

void Room2::gedAction(int index) {
	if (index == 0) {
		_G(det)->stop_detail(5);
		if (!_G(spieler).R2ElectrocutedBork)
			_G(det)->start_detail(6, 2, ANI_VOR);
		else
			start_ani_block(2, ABLOCK4);

		jump_out_r1(9);
	}
}

} // namespace Rooms
} // namespace Chewy
