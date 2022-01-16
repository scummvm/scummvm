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
#include "chewy/rooms/room6.h"
#include "chewy/rooms/room10.h"

namespace Chewy {
namespace Rooms {

void Room10::entry() {
	if (!_G(spieler).R10Surimy) {
		out->setze_zeiger(workptr);
		out->map_spr2screen(ablage[room_blk.AkAblage], _G(spieler).scrollx, _G(spieler).scrolly);
		out->setze_zeiger(0);
		fx->blende1(workptr, screen0, pal, 150, 0, 0);
		_G(spieler).R10Surimy = true;
		fx_blend = BLEND_NONE;
		flic_cut(FCUT_004, CFO_MODE);
		set_person_pos(0, 130, P_CHEWY, P_RIGHT);
		auto_move(2, P_CHEWY);
		hide_cur();
		start_spz(5, 255, 0, 0);
		start_aad_wait(101, 0);
		show_cur();

	} else if (_G(spieler).R10SurimyOk) {
		room->set_timer_status(3, TIMER_STOP);
	}
}

void Room10::get_surimy() {
	hide_cur();
	auto_move(4, P_CHEWY);
	start_aad(104, 0);
	flc->set_custom_user_function(Room6::cut_serv2);
	flic_cut(FCUT_006, CFO_MODE);
	flc->remove_custom_user_function();
	_G(spieler).R10SurimyOk = true;
	room->set_timer_status(3, TIMER_STOP);
	atds->set_ats_str(77, TXT_MARK_LOOK, 1, ATS_DATEI);
	invent_2_slot(18);

	del_inventar(_G(spieler).AkInvent);
	show_cur();
}

} // namespace Rooms
} // namespace Chewy
