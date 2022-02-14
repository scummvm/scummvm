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
#include "chewy/rooms/room94.h"
#include "chewy/rooms/room66.h"

namespace Chewy {
namespace Rooms {

void Room94::entry() {
	_G(spieler).flags33_80 = true;
	_G(zoom_horizont) = 140;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 2;
	_G(spieler).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler).ZoomXy[P_HOWARD][0] = 34;
	_G(spieler).ZoomXy[P_HOWARD][1] = 48;
	spieler_mi[P_HOWARD].Mode = true;
	if (_G(spieler).flags35_10) {
		_G(room)->set_timer_status(3, TIMER_STOP);
		_G(det)->del_static_ani(3);
	}

	if (flags.LoadGame)
		return;

	if (_G(spieler).flags35_8) {
		_G(zoom_horizont) = 140;
		_G(spieler).flags35_8 = false;
		_G(spieler).scrollx = 0;
		set_person_pos(156, 149, P_CHEWY, P_RIGHT);
		set_person_pos(153, 122, P_HOWARD, P_RIGHT);
		_G(out)->setze_zeiger(nullptr);
		_G(out)->cls();
		flic_cut(120, CFO_MODE);
		register_cutscene(30);
		_G(fx_blend) = BLEND_NONE;
		Room66::proc8(2, 3, 0, 539);
		_G(spieler).r94Scrollx = _G(spieler).scrollx;
		switch_room(93);
		return;
	}

	if (_G(spieler).flags35_40) {
		set_person_pos(373, 122, P_CHEWY, P_LEFT);
		set_person_pos(393, 94, P_HOWARD, P_LEFT);
		_G(spieler).flags35_40 = false;
		_G(spieler).scrollx = 150;
		if (_G(spieler).flags37_40) {
			_G(spieler).flags37_40 = true;
			flags.NoScroll = true;
			_G(out)->setze_zeiger(nullptr);
			_G(out)->cls();
			flic_cut(121, CFO_MODE);
			register_cutscene(32);

			_G(fx_blend) = BLEND_NONE;
			_G(SetUpScreenFunc) = nullptr;
			Room66::proc8(2, 3, 0, 551);
			_G(SetUpScreenFunc) = setup_func;
			flags.NoScroll = false;
		}
	} else {
		hide_cur();
		_G(spieler).scrollx = 120;
		set_person_pos(255, 86, P_HOWARD, P_LEFT);
		auto_move(3, P_CHEWY);
		show_cur();
	}
}

void Room94::xit() {
	_G(spieler).ScrollxStep = 1;
	_G(spieler).PersonRoomNr[P_HOWARD] = 95;
}

void Room94::setup_func() {
	calc_person_look();
	int destX, destY = 122;

	if (spieler_vector[P_CHEWY].Xypos[0] >= 370)
		destX = 399;
	else
		destX = 255;

	if (flags.ExitMov) {
		destX = 255;
		destY = 86;
	}
	
	go_auto_xy(destX, destY, P_HOWARD, ANI_GO);
}

void Room94::gedAction(int index) {
	if (!index && !_G(spieler).flags35_10)
		switch_room(93);
}

void Room94::talk1() {
	hide_cur();
	Room66::proc8(3, 3, 2, 552);
	show_cur();
}

int Room94::giveGhostBottle() {
	if (!is_cur_inventar(LIKOER2_INV))
		return 0;

	hide_cur();
	auto_move(2, P_CHEWY);
	auto_scroll(216, 0);
	del_inventar(_G(spieler).AkInvent);
	_G(out)->setze_zeiger(nullptr);
	_G(out)->cls();
	flags.NoPalAfterFlc = true;
	flic_cut(62, CFO_MODE);
	_G(fx_blend) = BLEND3;
	_G(det)->start_detail(5, 255, false);
	start_aad_wait(541, -1);
	_G(det)->stop_detail(5);
	_G(det)->show_static_spr(5);
	Room66::proc8(3, 3, 2, 617);
	hide_cur();
	_G(det)->hide_static_spr(5);
	_G(det)->start_detail(5, 255, false);
	start_aad_wait(622, -1);
	_G(det)->stop_detail(5);
	_G(det)->start_detail(6, 1, false);	
	_G(room)->set_timer_status(3, TIMER_STOP);
	_G(det)->del_static_ani(3);
	start_detail_wait(4, 1, ANI_VOR);
	_G(spieler).flags35_10 = true;
	_G(spieler).room_e_obj[138].Attribut = AUSGANG_OBEN;
	_G(atds)->set_steuer_bit(522, ATS_AKTIV_BIT, ATS_DATEI);
	new_invent_2_cur(114);
	
	show_cur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
