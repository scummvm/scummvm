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
#include "chewy/rooms/room97.h"

namespace Chewy {
namespace Rooms {

void Room97::entry() {
	det->enable_sound(0, 0);
	det->play_sound(0, 0);
	SetUpScreenFunc = setup_func;
	_G(zoom_horizont) = 0;

	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 24;
	_G(spieler).ZoomXy[P_NICHELLE][1] = 43;
	_G(spieler).ZoomXy[P_HOWARD][0] = 24;
	_G(spieler).ZoomXy[P_HOWARD][1] = 42;

	_G(spieler).r97_bool18DB32 = false;
	_G(spieler).PersonRoomNr[P_NICHELLE] = 97;
	_G(zoom_mov_fak) = 0;
	_G(spieler).ScrollxStep = 2;
	det->hide_static_spr(14);

	if (_G(spieler).flags36_4)
		det->show_static_spr(17);

	if (_G(spieler).flags36_10) {
		det->start_detail(11, 255, ANI_VOR);
		det->stop_detail(5);
		det->start_detail(6, 255, ANI_VOR);
		det->start_detail(12, 255, ANI_VOR);
	}

	if (_G(spieler).flags36_40) {
		det->start_detail(14, 255, ANI_VOR);
		det->show_static_spr(13);
		atds->del_steuer_bit(543, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		atds->set_steuer_bit(543, ATS_AKTIV_BIT, ATS_DATEI);
	}

	if (_G(spieler).flags36_20) {
		det->set_detail_pos(27, 272, 110);
		for (int i = 0; i < 2; ++i) {
			det->stop_detail(23 + i);
			det->start_detail(27 + i, 255, ANI_VOR);
		}
		det->disable_sound(26, 0);
		det->disable_sound(27, 0);
		det->disable_sound(28, 0);
	}

	if (_G(spieler).flags37_1)
		det->show_static_spr(19);

	if (_G(spieler).flags37_4)
		det->show_static_spr(20);

	_G(spieler).r97_word18DB2E = 0;
	_G(spieler).r97_word18DB34 = 332;
	_G(spieler).r97_word18DB36 = 110;
	_G(spieler).r97_word18DB38 = 132;

	if (!flags.ExitMov) {
		hide_cur();
		out->setze_zeiger(nullptr);
		out->cls();

		_G(spieler).PersonRoomNr[P_HOWARD] = _G(spieler).PersonRoomNr[P_NICHELLE] = 89;
		_G(spieler).SVal2 = 0;
		_G(spieler).flags35_4 = true;
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(19, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		show_cur();
		switch_room(89);
	} else {
		_G(spieler).scrollx = 800;
		set_person_pos(1067, 98, P_CHEWY, P_LEFT);
		set_person_pos(1092, 83, P_HOWARD, P_LEFT);
		set_person_pos(130, 29, P_NICHELLE, P_RIGHT);
	}
}

void Room97::xit() {
	_G(spieler).ScrollxStep = 1;
}

void Room97::setup_func() {
	warning("STUB Room97::setup_func");
}

void Room97::proc2() {
	if (flags.AutoAniPlay)
		return;

	flags.AutoAniPlay = true;
	stop_person(P_CHEWY);
	hide_cur();
	Sdi[6].z_ebene = 6;
	Sdi[24].z_ebene = 6;
	_G(spieler).flags36_1 = false;
	spieler_mi[P_CHEWY].Mode = true;
	go_auto_xy(455, 59, P_CHEWY, ANI_VOR);
	go_auto_xy(496, 99, P_CHEWY, ANI_VOR);
	go_auto_xy(530, 99, P_CHEWY, ANI_VOR);
	go_auto_xy(572, 139, P_CHEWY, ANI_VOR);
	go_auto_xy(588, 129, P_CHEWY, ANI_VOR);
	spieler_mi[P_CHEWY].Mode = false;
	Sdi[6].z_ebene = 166;
	Sdi[18].z_ebene = 157;
	show_cur();
	flags.AutoAniPlay = false;
}

void Room97::proc3() {
	if (flags.AutoAniPlay)
		return;

	flags.AutoAniPlay = true;
	hide_cur();
	stop_person(P_CHEWY);
	spieler_mi[P_CHEWY].Mode = true;
	
	if (!_G(spieler).flags36_20) {
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		start_aad_wait(557, -1);
		go_auto_xy(588, 129, P_CHEWY, ANI_VOR);
	} else if (!_G(spieler).flags36_40) {
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		start_aad_wait(567, -1);
		go_auto_xy(588, 129, P_CHEWY, ANI_VOR);
	} else {
		_G(spieler).flags36_1 = true;
		HowardMov = 1;
		go_auto_xy(572, 139, P_CHEWY, ANI_VOR);
		Sdi[6].z_ebene = 6;
		Sdi[24].z_ebene = 6;

		go_auto_xy(530, 99, P_CHEWY, ANI_VOR);
		go_auto_xy(496, 99, P_CHEWY, ANI_VOR);
		go_auto_xy(455, 59, P_CHEWY, ANI_VOR);
		go_auto_xy(440, 52, P_CHEWY, ANI_VOR);
		if (!_G(spieler).flags36_80) {
			go_auto_xy(572, 122, P_HOWARD, ANI_VOR);
			go_auto_xy(526, 83, P_HOWARD, ANI_VOR);
			go_auto_xy(500, 83, P_HOWARD, ANI_VOR);
		}

		auto_move(6, P_CHEWY);
		if (!_G(spieler).flags36_80) {
			go_auto_xy(442, 35, P_HOWARD, ANI_VOR);
			go_auto_xy(497, 31, P_HOWARD, ANI_VOR);
			start_aad_wait(562, -1);
		}
		spieler_mi[P_CHEWY].Mode = false;
		Sdi[6].z_ebene = 166;
		Sdi[24].z_ebene = 157;
		_G(spieler).flags36_80 = true;
	}
	show_cur();
	flags.AutoAniPlay = false;
}

void Room97::proc4() {
	if (flags.AutoAniPlay)
		return;

	if (!_G(spieler).flags35_80) {
		flags.AutoAniPlay = true;
		hide_cur();
		spieler_mi[P_CHEWY].Mode = true;
		stop_person(P_CHEWY);
		det->enable_sound(9, 0);
		det->disable_sound(9, 1);
		start_detail_wait(9, 1, ANI_VOR);
		det->show_static_spr(21);

		while (spieler_vector[P_HOWARD].Xypos[0] > 996)
			set_up_screen(DO_SETUP);

		det->show_static_spr(2);
		HowardMov = 1;
		
		go_auto_xy(967, 111, P_CHEWY, ANI_VOR);
		go_auto_xy(1008, 93, P_CHEWY, ANI_VOR);
		go_auto_xy(1037, 99, P_CHEWY, ANI_VOR);

		go_auto_xy(995, 77, P_HOWARD, ANI_VOR);
		go_auto_xy(1047, 87, P_HOWARD, ANI_VOR);

		start_detail_wait(29, 1, ANI_VOR);
		det->hide_static_spr(21);
		det->enable_sound(9, 1);
		det->disable_sound(9, 0);
		start_detail_wait(9, 0, ANI_RUECK);

		go_auto_xy(1008, 93, P_CHEWY, ANI_VOR);
		go_auto_xy(967, 111, P_CHEWY, ANI_VOR);
		go_auto_xy(995, 82, P_HOWARD, ANI_VOR);
		det->hide_static_spr(2);
		spieler_mi[P_CHEWY].Mode = false;
		start_spz(4, 255, false, P_CHEWY);
		start_aad_wait(553, -1);
		HowardMov = 0;
		show_cur();
	}
		
	flags.AutoAniPlay = false;
}

int Room97::proc5() {
	if (!is_cur_inventar(114))
		return 0;

	hide_cur();
	auto_move(0, P_CHEWY);
	auto_scroll(800, 0);
	
	while (spieler_vector[P_HOWARD].Xypos[0] < 1080)
		set_up_screen(DO_SETUP);

	flic_cut(122, CFO_MODE);

	start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
	start_detail_wait(0, 1, ANI_VOR);
	det->show_static_spr(13);
	start_detail_wait(13, 1, ANI_VOR);
	det->start_detail(14, 255, false);
	start_aad_wait(555, -1);
	_G(spieler).flags36_40 = true;
	atds->set_steuer_bit(539, ATS_AKTIV_BIT, ATS_DATEI);
	atds->del_steuer_bit(537, ATS_AKTIV_BIT, ATS_DATEI);
	atds->del_steuer_bit(535, ATS_AKTIV_BIT, ATS_DATEI);
	atds->del_steuer_bit(543, ATS_AKTIV_BIT, ATS_DATEI);
	
	show_cur();
	return 1;
}

int Room97::proc6() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	auto_move(1, P_CHEWY);
	spieler_mi[P_CHEWY].Mode = true;
	go_auto_xy(298, 120, P_CHEWY, ANI_VOR);
	set_person_spr(P_LEFT, P_CHEWY);
	start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
	det->hide_static_spr(15);
	start_detail_wait(1, 1, ANI_VOR);
	det->show_static_spr(28);
	auto_move(1, P_CHEWY);
	det->hide_static_spr(18);
	start_detail_wait(15, 1, ANI_VOR);
	det->show_static_spr(17);
	start_spz(4, 255, false, P_CHEWY);
	start_aad_wait(558, -1);
	det->hide_static_spr(28);
	det->stop_detail(5);
	det->show_static_spr(15);
	_G(spieler).flags36_2 = true;
	atds->del_steuer_bit(538, ATS_AKTIV_BIT, ATS_DATEI);
	atds->set_steuer_bit(531, ATS_AKTIV_BIT, ATS_DATEI);
	atds->set_ats_str(530, 1, ATS_DATEI);
	atds->set_steuer_bit(532, ATS_AKTIV_BIT, ATS_DATEI);
	spieler_mi[P_CHEWY].Mode = false;
	_G(spieler).flags36_8 = true;

	show_cur();
	return 1;
}

int Room97::proc7() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	auto_move(2, P_CHEWY);
	spieler_mi[P_CHEWY].Mode = true;
	_G(spieler).PersonHide[P_CHEWY] = true;
	det->hide_static_spr(17);
	start_detail_wait(22, 1, ANI_VOR);
	atds->set_steuer_bit(538, ATS_AKTIV_BIT, ATS_DATEI);
	atds->set_ats_str(530, 2, ATS_DATEI);
	new_invent_2_cur(SLIME_INV);
	_G(spieler).flags36_4 = true;
	set_person_pos(304, 102, 0, P_RIGHT);
	_G(spieler).PersonHide[P_CHEWY] = false;
	auto_move(1, P_CHEWY);
	spieler_mi[P_CHEWY].Mode = false;

	show_cur();
	return 1;
}

int Room97::proc8() {
	if (!is_cur_inventar(94))
		return 0;

	hide_cur();
	auto_move(2, P_CHEWY);
	int diaNr = -1;
	if (!_G(spieler).flags36_2)
		diaNr = 559;
	else if (!_G(spieler).flags36_4)
		diaNr = 560;
	else {
		start_spz_wait(14, 1, false, P_CHEWY);
		det->start_detail(11, 255, false);
		del_inventar(_G(spieler).AkInvent);
		_G(spieler).flags36_10 = true;
		_G(spieler).flags36_8 = false;
		det->start_detail(12, 255, false);
		atds->set_ats_str(530, 3, ATS_DATEI);
		atds->del_steuer_bit(532, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_ats_str(532, 1, ATS_DATEI);
		det->start_detail(6, 255, false);
		det->hide_static_spr(14);
		auto_move(3, P_CHEWY);
		auto_scroll(406, 0);
		wait_show_screen(40);
		det->stop_detail(24);
		det->enable_sound(26, 0);
		det->play_sound(26, 0);
		start_detail_wait(25, 1, ANI_VOR);
		det->start_detail(26, 255, false);
		det->stop_detail(23);
		det->start_detail(27, 255, false);
		wait_show_screen(80);
		auto_move(4, P_CHEWY);
		auto_scroll(646, 0);
		start_aad_wait(566, -1);
		wait_show_screen(60);
		start_aad_wait(567, -1);
		det->stop_detail(26);
		det->stop_detail(27);
		_G(spieler).flags36_20 = true;
		det->start_detail(28, 255, false);
		det->set_detail_pos(27, 272, 110);
		det->start_detail(27, 255, false);
		atds->set_steuer_bit(533, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_steuer_bit(534, ATS_AKTIV_BIT, ATS_DATEI);
	}

	if (diaNr != -1) {
		start_spz(4, 255, false, P_CHEWY);
		start_aad_wait(diaNr, -1);
	}

	show_cur();
	return 1;
}

int Room97::proc9() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	auto_move(5, P_CHEWY);
	
	if (!_G(spieler).flags36_20) {
		start_spz(4, 255, false, P_CHEWY);
		start_aad_wait(556, -1);
	} else {
		start_spz_wait(13, 1, false, P_CHEWY);
		start_detail_wait(0, 1, ANI_VOR);
		det->show_static_spr(13);
		start_detail_wait(13, 1, ANI_VOR);
		det->start_detail(14, 255, false);
		start_aad_wait(555, -1);
		_G(spieler).flags36_40 = true;
		atds->set_steuer_bit(539, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(537, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(535, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(543, ATS_AKTIV_BIT, ATS_DATEI);
	}

	show_cur();
	return 1;
}

int Room97::proc10() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	auto_move(8, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	det->enable_sound(7, 0);
	det->disable_sound(7, 1);
	start_detail_wait(7, 1, ANI_VOR);
	det->show_static_spr(19);
	_G(spieler).flags37_1 = true;
	atds->set_steuer_bit(543, ATS_AKTIV_BIT, ATS_DATEI);
	menu_item = CUR_WALK;
	cursor_wahl(CUR_WALK);

	show_cur();
	return 1;
}

int Room97::proc11() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();

	if (!_G(spieler).flags37_2) {
		start_spz(4, 255, false, P_CHEWY);
		start_aad_wait(569, -1);
	} else {
		auto_move(9, P_CHEWY);
		start_aad_wait(570, -1);
		start_spz_wait(13, 1, false, P_CHEWY);
		auto_move(7, P_CHEWY);
		start_spz(5, 255, false, P_CHEWY);
		start_aad_wait(571, -1);
		det->enable_sound(4, 0);
		det->play_sound(4, 0);
		det->start_detail(4, 1, false);
		auto_move(12, P_CHEWY);
		start_spz_wait(64, 1, false, P_CHEWY);
		start_spz(65, 255, false, P_CHEWY);
		start_aad_wait(561, -1);
		_G(spieler).SVal1 = 97;
		_G(spieler).SVal2 = 523;
		cur_2_inventory();
		_G(spieler).flags37_8 = true;
		switch_room(92);
		show_cur(); // probably useless, but present in the original
	}

	show_cur();
	return 1;
}

void Room97::proc12() {
	if (!_G(spieler).flags37_1) {
		stop_person(P_CHEWY);
		set_person_pos(491, 42, P_CHEWY, P_RIGHT);
		return;
	}

	if (flags.AutoAniPlay)
		return;

	flags.AutoAniPlay = true;
	hide_cur();
	stop_person(P_CHEWY);
	det->show_static_spr(26);
	go_auto_xy(481, 39, P_CHEWY, ANI_VOR);
	_G(spieler).PersonHide[0] = true;
	set_person_pos(327, 42, P_CHEWY, P_LEFT);
	wait_show_screen(50);
	set_person_pos(347, 42, P_CHEWY, P_LEFT);
	_G(spieler).PersonHide[0] = false;
	det->hide_static_spr(26);
	spieler_mi[P_CHEWY].Mode = false;
	show_cur();
	flags.AutoAniPlay = false;
}

void Room97::proc13() {
	if (!_G(spieler).flags37_1 || flags.AutoAniPlay)
		return;

	flags.AutoAniPlay = true;
	hide_cur();
	stop_person(P_CHEWY);
	spieler_mi[P_CHEWY].Mode = true;
	det->show_static_spr(26);
	_G(spieler).PersonHide[P_CHEWY] = true;
	set_person_pos(508, 41, P_CHEWY, P_LEFT);
	wait_show_screen(50);
	set_person_pos(488, 41, P_CHEWY, P_LEFT);
	_G(spieler).PersonHide[P_CHEWY] = false;
	auto_move(6, P_CHEWY);
	det->hide_static_spr(26);
	spieler_mi[P_CHEWY].Mode = false;
	show_cur();
	flags.AutoAniPlay = false;
}

int Room97::proc14() {
	if (!is_cur_inventar(SLIME_INV))
		return 0;

	_G(spieler).r97_bool18DB30 = true;
	hide_cur();
	del_inventar(_G(spieler).AkInvent);
	menu_item = CUR_USE;
	cursor_wahl(CUR_USE);
	
	return 1;
}

void Room97::proc15() {
	if (_G(spieler).flags37_4 || flags.AutoAniPlay)
		return;

	flags.AutoAniPlay = true;
	stop_person(P_CHEWY);
	menu_item = CUR_USE;
	cursor_wahl(CUR_USE);
	set_person_pos(294, 42, P_CHEWY, P_LEFT);
	atds->del_steuer_bit(541, ATS_AKTIV_BIT, ATS_DATEI);
	
//	mov     edi, 1

	while (true) {
		if (det->get_ani_detail(16) == nullptr) {
			start_detail_wait(17, 1, ANI_VOR);
			det->start_detail(16, 1, true);
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_detail_wait(20, 1, false);
			set_person_pos(318, 42, P_CHEWY, P_LEFT);
			_G(spieler).PersonHide[P_CHEWY] = false;
			break;
		}
	}

	atds->set_steuer_bit(541, ATS_AKTIV_BIT, ATS_DATEI);
	show_cur();
	flags.AutoAniPlay = false;
}

} // namespace Rooms
} // namespace Chewy
