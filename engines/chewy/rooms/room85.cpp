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
#include "chewy/rooms/room85.h"
#include "chewy/rooms/room87.h"

namespace Chewy {
namespace Rooms {

void Room85::entry(int16 eib_nr) {
	det->enable_sound(0, 0);
	det->play_sound(0, 0);
	det->enable_sound(0, 1);
	det->play_sound(0, 1);
	_G(spieler).ScrollxStep = 2;
	SetUpScreenFunc = setup_func;
	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;
	_G(spieler).ZoomXy[P_HOWARD][0] = 22;
	_G(spieler).ZoomXy[P_HOWARD][1] = 37;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 22;
	_G(spieler).ZoomXy[P_NICHELLE][1] = 38;

	_G(zoom_horizont) = 0;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 1;

	if (_G(spieler).flags32_10) {
		det->show_static_spr(4);
		atds->del_steuer_bit(495, ATS_AKTIV_BIT, ATS_DATEI);
	}

	if (flags.LoadGame)
		return;

	if (_G(spieler).flags30_1 || eib_nr == 124) {
		hide_cur();
		set_person_pos(278, 157, P_CHEWY, P_RIGHT);
		set_person_pos(296, 142, P_HOWARD, P_RIGHT);
		set_person_pos(322, 142, P_NICHELLE, P_RIGHT);
		_G(spieler).flags30_1 = false;
		_G(maus_links_click) = false;
		_G(spieler).scrollx = 78;
		if (_G(spieler).flags32_40) {
			atds->del_steuer_bit(506, ATS_AKTIV_BIT, ATS_DATEI);
			det->start_detail(1, 255, false);
			set_person_pos(195, 146, P_CHEWY, P_RIGHT);
			set_person_pos(186, 142, P_HOWARD, P_RIGHT);
		}
		show_cur();
	} else if (!_G(spieler.flags32_20)) {
		set_person_pos(148, 152, P_CHEWY, P_RIGHT);
		set_person_pos(98, 142, P_HOWARD, P_RIGHT);
		set_person_pos(128, 142, P_NICHELLE, P_RIGHT);
		_G(spieler).scrollx = 0;
	} else {
		hide_cur();
		_G(spieler).scrollx = 0;
		set_person_pos(133, 152, P_CHEWY, P_RIGHT);
		_G(spieler).room_e_obj[127].Attribut = 255;
		det->show_static_spr(5);
		det->show_static_spr(6);
		start_aad_wait(474, -1);
		flic_cut(90, CFO_MODE);
		out->setze_zeiger(nullptr);
		out->cls();
		flags.NoPalAfterFlc = true;
		flic_cut(91, CFO_MODE);
		det->hide_static_spr(6);
		_G(spieler).scrollx = 25;
		fx_blend = BLEND3;
		start_aad_wait(475, -1);
		start_aad(476, -1);
		flc->set_custom_user_function(Room87::proc5);
		flic_cut(89, CFO_MODE);
		test_intro(25);

		flc->remove_custom_user_function();
		switch_room(84);
		show_cur();
	}
}

void Room85::xit(int16 eib_nr) {
	switch (eib_nr) {
	case 126:
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 85)
			_G(spieler).PersonRoomNr[P_HOWARD] = 84;
		
		if (_G(spieler).PersonRoomNr[P_NICHELLE] == 85)
			_G(spieler).PersonRoomNr[P_NICHELLE] = 84;
		break;
	case 127:
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 85)
			_G(spieler).PersonRoomNr[P_HOWARD] = 86;

		if (_G(spieler).PersonRoomNr[P_NICHELLE] == 85)
			_G(spieler).PersonRoomNr[P_NICHELLE] = 86;
		break;
	default:
		break;
	}
}

void Room85::setup_func() {
	calc_person_look();

	const int xyPos = spieler_vector[P_CHEWY].Xypos[0];
	int howDestX, nicDestX;
	
	if (xyPos > 255) {
		howDestX = 296;
		nicDestX = 322;
	} else if (xyPos > 150) {
		howDestX = 186;
		nicDestX = 212;
	} else {
		howDestX = 98;
		nicDestX = 128;
	}

	if (HowardMov == 1) {
		howDestX = 98;
		nicDestX = 128;
	}

	go_auto_xy(howDestX, 142, P_HOWARD, ANI_GO);
	go_auto_xy(nicDestX, 142, P_NICHELLE, ANI_GO);
}

int Room85::proc2() {
	if (_G(spieler).inv_cur)
		return 0;

	auto_move(2, P_CHEWY);
	det->stop_detail(1);
	start_detail_wait(2, 1, ANI_VOR);
	_G(spieler).PersonRoomNr[P_HOWARD] = 89;
	cur_2_inventory();
	remove_inventory(109);
	remove_inventory(98);
	remove_inventory(87);
	remove_inventory(89);
	
	show_cur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
