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
#include "chewy/rooms/room10.h"

namespace Chewy {
namespace Rooms {

void Room10::entry() {
	if (!_G(spieler).R10Surimy) {
		_G(out)->setze_zeiger(_G(workptr));
		_G(out)->map_spr2screen(_G(ablage)[room_blk.AkAblage], _G(spieler).scrollx, _G(spieler).scrolly);
		_G(out)->setze_zeiger(nullptr);
		fx->blende1(_G(workptr), _G(screen0), _G(pal), 150, 0, 0);
		_G(spieler).R10Surimy = true;
		_G(fx_blend) = BLEND_NONE;
		flic_cut(FCUT_004, CFO_MODE);
		set_person_pos(0, 130, P_CHEWY, P_RIGHT);
		auto_move(2, P_CHEWY);
		hide_cur();
		start_spz(CH_TALK6, 255, false, P_CHEWY);
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
	flc->set_custom_user_function(cut_serv);
	flic_cut(FCUT_006, CFO_MODE);
	flc->remove_custom_user_function();
	_G(spieler).R10SurimyOk = true;
	room->set_timer_status(3, TIMER_STOP);
	atds->set_ats_str(77, TXT_MARK_LOOK, 1, ATS_DATEI);
	invent_2_slot(18);

	del_inventar(_G(spieler).AkInvent);
	show_cur();
}

int16 Room10::cut_serv(int16 frame) {
	atds->print_aad(_G(spieler).scrollx, _G(spieler).scrolly);
	if (frame == 31)
		start_aad(107, 0);

	return 0;
}

} // namespace Rooms
} // namespace Chewy
