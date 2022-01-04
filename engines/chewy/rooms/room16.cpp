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
#include "chewy/rooms/room16.h"
#include "chewy/rooms/room23.h"

namespace Chewy {
namespace Rooms {

void Room16::entry() {
	_G(zoom_horizont) = 140;
	if (!_G(spieler).R16F5Exit) {
		det->show_static_spr(4);
		_G(spieler).room_e_obj[32].Attribut = 255;
		atds->del_steuer_bit(124, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		det->hide_static_spr(4);
		_G(spieler).room_e_obj[32].Attribut = AUSGANG_LINKS;
		atds->set_steuer_bit(124, ATS_AKTIV_BIT, ATS_DATEI);
	}
}

int16 Room16::use_gleiter() {
	int16 action_flag = false;
	if (!_G(spieler).inv_cur) {
		action_flag = true;
		auto_move(6, P_CHEWY);
		_G(spieler).R23GleiterExit = 16;
		Room23::cockpit();
	}
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
