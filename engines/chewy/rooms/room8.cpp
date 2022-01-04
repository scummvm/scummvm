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
#include "chewy/rooms/room8.h"

namespace Chewy {
namespace Rooms {

void Room8::entry() {
	_G(spieler).R7ChewyFlug = true;
	if (!_G(spieler).R8Folter)
		start_folter();
	else
		stop_folter();
	atds->set_ats_str(60, 0, ATS_DATEI);
}

void Room8::start_folter() {
	atds->set_ats_str(67, 1, ATS_DATEI);

	det->stop_detail(19);

	det->start_detail(13, 255, ANI_VOR);
}

void Room8::stop_folter() {
	atds->set_ats_str(67, 0, ATS_DATEI);
	det->start_detail(19, 255, ANI_VOR);

	det->stop_detail(13);

	_G(spieler).R8Folter = true;
	obj->hide_sib(SIB_FOLTER_R8);
}

void Room8::hole_kohle() {
	if (!_G(spieler).R8Kohle) {
		hide_cur();
		_G(spieler).R8Kohle = true;
		auto_move(4, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(12, 1, ANI_VOR);
		cur_2_inventory();
		invent_2_slot(KOHLE_HEISS_INV);
		_G(spieler).PersonHide[P_CHEWY] = false;
		show_cur();
	}
}

void Room8::start_verbrennen() {
	int16 ende;
	hide_cur();
	if (!_G(spieler).inv_cur) {
		auto_move(3, P_CHEWY);
		start_aad(102, 0);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_ani_block(2, ablock12);
		ende = 0;
		while (!ende && det->get_ani_status(9)) {
			set_up_screen(DO_SETUP);
			SHOULD_QUIT_RETURN;

			if (minfo.button == 1 || kbinfo.key_code == ENTER) {
				if (minfo.x > 146 && minfo.x < 208 &&
					minfo.y > 107 && minfo.y < 155)
					ende = 1;
			}
		}
		det->stop_detail(9);
		set_person_pos(129, 246, P_CHEWY, P_RIGHT);
		start_ani_block(2, ablock13);
		atds->set_ats_str(60, TXT_MARK_LOOK, 1, ATS_DATEI);
		_G(spieler).PersonHide[P_CHEWY] = false;
	}
	show_cur();
}

bool Room8::gips_wurf() {
	if (!is_cur_inventar(11)) {
		hide_cur();
		det->load_taf_seq(116, 30, 0);
		auto_move(2, P_CHEWY);
		_G(maus_links_click) = 0;
		_G(spieler).PersonHide[P_CHEWY] = true;
		del_inventar(GIPS_EIMER_INV);
		start_detail_wait(4, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		start_detail_frame(5, 1, ANI_VOR, 16);
		start_detail_wait(6, 1, ANI_VOR);
		obj->show_sib(33);
		det->show_static_spr(14);
		wait_detail(5);
		_G(spieler).R8GipsWurf = true;
		_G(spieler).room_m_obj[MASKE_INV].ZEbene = 0;
		obj->set_inventar(MASKE_INV, 181, 251, 8, &room_blk);
		det->del_taf_tbl(116, 30, 0);
		auto_move(8, P_CHEWY);
		flags.AtsAction = false;
		menu_item = CUR_USE;
		look_invent_screen(INVENTAR_NORMAL, 178);
		flags.AtsAction = true;
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(20, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		invent_2_slot(MASKE_INV);
		cursor_wahl(menu_item);
		show_cur();

		return true;
	}

	return false;
}

void Room8::open_gdoor() {
	_G(spieler).PersonHide[P_CHEWY] = true;
	det->show_static_spr(17);
	set_up_screen(DO_SETUP);
	start_detail_wait(7, 1, ANI_VOR);
	det->show_static_spr(15);
	det->hide_static_spr(17);
	_G(spieler).PersonHide[P_CHEWY] = false;
	set_person_pos(204, 274, P_CHEWY, P_LEFT);
	atds->del_steuer_bit(69, ATS_AKTIV_BIT, ATS_DATEI);
	obj->hide_sib(31);
	_G(spieler).R8GTuer = true;
	_G(spieler).room_e_obj[15].Attribut = AUSGANG_UNTEN;
}

void Room8::talk_nimoy() {
	int16 tmp;
	auto_move(9, P_CHEWY);
	flags.NoScroll = true;
	auto_scroll(0, 120);
	if (_G(spieler).R8Folter) {
		if (!_G(spieler).R8GipsWurf)
			tmp = 1;
		else
			tmp = 2;
		if (!_G(spieler).R8GTuer)
			load_ads_dia(tmp);
		else
			start_aad_wait(61, -1);
	} else
		load_ads_dia(6);
	flags.NoScroll = false;
}

} // namespace Rooms
} // namespace Chewy
