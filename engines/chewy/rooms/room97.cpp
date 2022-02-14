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
#include "chewy/rooms/room97.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

int Room97::_word18DB2E;
bool Room97::_slimeThrown;
bool Room97::_bool18DB32;
int Room97::_word18DB34;
int Room97::_word18DB36;
int Room97::_word18DB38;
bool Room97::_bool18DB3A;

void Room97::entry() {
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0);
	_G(SetUpScreenFunc) = setup_func;
	_G(zoom_horizont) = 0;

	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 24;
	_G(spieler).ZoomXy[P_NICHELLE][1] = 43;
	_G(spieler).ZoomXy[P_HOWARD][0] = 24;
	_G(spieler).ZoomXy[P_HOWARD][1] = 42;

	_bool18DB32 = false;
	_G(spieler).PersonRoomNr[P_NICHELLE] = 97;
	_G(zoom_mov_fak) = 0;
	_G(spieler).ScrollxStep = 2;
	det->hide_static_spr(14);

	if (_G(spieler).flags36_2) {
		det->hide_static_spr(14);
		det->hide_static_spr(18);

		if (!_G(spieler).flags36_4)
			det->show_static_spr(17);
	}
	
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
		g_engine->_sound->stopSound(0);
		g_engine->_sound->stopSound(0);
		g_engine->_sound->stopSound(0);
	}

	if (_G(spieler).flags37_1)
		det->show_static_spr(19);

	if (_G(spieler).flags37_4)
		det->show_static_spr(20);

	_word18DB2E = 0;
	_word18DB34 = 332;
	_word18DB36 = 110;
	_word18DB38 = 132;

	if (flags.LoadGame)
		return;

	if (_G(spieler).flags37_8) {
		hide_cur();
		_G(out)->setze_zeiger(nullptr);
		_G(out)->cls();

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

void Room97::gedAction(int index) {
	switch (index) {
	case 50:
		proc2();
		break;
	case 51:
		proc3();
		break;
	case 52:
		proc13();
		break;
	case 53:
		proc12();
		break;
	case 54:
		proc4();
		break;
	case 55:
		if (_G(spieler).flags36_20)
			auto_scroll(268, 0);
		break;
	case 56:
		sensorAnim();
		break;
	default:
		break;
	}
}

void Room97::setup_func() {
	spieler_mi[P_CHEWY].Vorschub = 4;
	spieler_mi[P_HOWARD].Vorschub = 6;
	spieler_mi[P_NICHELLE].Vorschub = 4;

	if (_G(spieler).flags35_80 && spieler_vector->Xypos[P_CHEWY] > 830)
		set_person_pos(830, 98, P_CHEWY, P_RIGHT);

	if (_word18DB2E)
		--_word18DB2E;
	else {
		_word18DB2E = _G(spieler).DelaySpeed / 2;
		if (_word18DB34 != 710)
			_word18DB34 += 2;
		else if (_word18DB36 < 200) {
			_word18DB36 += 2;
			_word18DB38 += 2;
		} else {
			if (!_G(spieler).flags36_8) {
				_word18DB34 = 332;
				_word18DB36 = _G(spieler).flags36_10 ? 106 : 110;
			}
			_bool18DB3A = true;
		}

		if (_bool18DB3A) {
			if (_word18DB38 <= 132)
				_bool18DB3A = false;
			else
				_word18DB38 -= 2;
		}

		det->set_static_pos(16, 699, _word18DB38, false, false);
		if (_G(spieler).flags36_10) {
			det->set_detail_pos(12, _word18DB34, _word18DB36);
		} else {
			det->set_static_pos(14, _word18DB34, _word18DB36, false, false);
		}
	}

	if (_G(spieler).flags37_1 && _G(menu_item) == CUR_WALK) {
		if (_G(spieler).scrollx + minfo.x >= 487 && _G(spieler).scrollx + minfo.x <= 522 && minfo.y >= 23 && minfo.y <= 59)
			cursor_wahl(CUR_AUSGANG_OBEN);
		else
			cursor_wahl(CUR_WALK);
	}

	calc_person_look();

	const int chewyPosX = spieler_vector[P_CHEWY].Xypos[0];
	if (!_G(spieler).flags36_80) {
		int destX, destY;

		if (chewyPosX > 980) {
			destX = 1080;
			destY = 91;
		} else if (chewyPosX > 880) {
			destX = 994;
			destY = 98;
		} else if (chewyPosX > 780) {
			destX = 824;
			destY = 80;
		} else if (chewyPosX > 650) {
			destX = 758;
			destY = 121;
		} else if (chewyPosX > 420) {
			destX = 590;
			destY = 111;
		} else {
			destX = 412;
			destY = 112;
		}

		if (_G(HowardMov) != 1)
			go_auto_xy(destX, destY, P_HOWARD, ANI_GO);
	}

	if (!_bool18DB32)
		return;

	int destX = -1;
	int destY = -1;
	const int nichellePosX = spieler_vector[P_NICHELLE].Xypos[0];
	if (chewyPosX > 250 && nichellePosX < 232) {
		destX = 232;
		destY = 27;
	} else if (nichellePosX >= 249) {
		destX = 370;
		destY = 20;
	} else if (nichellePosX >= 232) {
		destX = 249;
		destY = 20;
	}	
	
	if (destX != -1)
		go_auto_xy(destX, destY, P_NICHELLE, ANI_GO);
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
	go_auto_xy(455, 59, P_CHEWY, ANI_WAIT);
	go_auto_xy(496, 99, P_CHEWY, ANI_WAIT);
	go_auto_xy(530, 99, P_CHEWY, ANI_WAIT);
	go_auto_xy(572, 139, P_CHEWY, ANI_WAIT);
	go_auto_xy(588, 129, P_CHEWY, ANI_WAIT);
	spieler_mi[P_CHEWY].Mode = false;
	Sdi[6].z_ebene = 166;
	Sdi[24].z_ebene = 157;
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
		go_auto_xy(588, 129, P_CHEWY, ANI_WAIT);
	} else if (!_G(spieler).flags36_40) {
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		start_aad_wait(568, -1);
		go_auto_xy(588, 129, P_CHEWY, ANI_WAIT);
	} else {
		_G(spieler).flags36_1 = true;
		_G(HowardMov) = 1;
		go_auto_xy(572, 139, P_CHEWY, ANI_WAIT);
		Sdi[6].z_ebene = 6;
		Sdi[24].z_ebene = 6;

		go_auto_xy(530, 99, P_CHEWY, ANI_WAIT);
		go_auto_xy(496, 99, P_CHEWY, ANI_WAIT);
		go_auto_xy(455, 59, P_CHEWY, ANI_WAIT);
		go_auto_xy(440, 52, P_CHEWY, ANI_WAIT);
		if (!_G(spieler).flags36_80) {
			go_auto_xy(572, 122, P_HOWARD, ANI_WAIT);
			go_auto_xy(526, 83, P_HOWARD, ANI_WAIT);
			go_auto_xy(500, 83, P_HOWARD, ANI_WAIT);
		}

		auto_move(6, P_CHEWY);
		if (!_G(spieler).flags36_80) {
			go_auto_xy(442, 35, P_HOWARD, ANI_WAIT);
			go_auto_xy(497, 31, P_HOWARD, ANI_WAIT);
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
		g_engine->_sound->playSound(9, 0);
		g_engine->_sound->stopSound(1);
		start_detail_wait(9, 1, ANI_VOR);
		det->show_static_spr(21);

		while (spieler_vector[P_HOWARD].Xypos[0] > 996) {
			set_up_screen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		det->show_static_spr(2);
		_G(HowardMov) = 1;
		
		go_auto_xy(967, 111, P_CHEWY, ANI_WAIT);
		go_auto_xy(1008, 93, P_CHEWY, ANI_WAIT);
		go_auto_xy(1037, 90, P_CHEWY, ANI_WAIT);

		go_auto_xy(995, 77, P_HOWARD, ANI_WAIT);
		go_auto_xy(1047, 87, P_HOWARD, ANI_WAIT);

		start_detail_wait(29, 1, ANI_VOR);
		det->hide_static_spr(21);
		g_engine->_sound->playSound(9, 1);
		g_engine->_sound->stopSound(0);
		start_detail_wait(9, 0, ANI_RUECK);

		go_auto_xy(1008, 93, P_CHEWY, ANI_WAIT);
		go_auto_xy(967, 111, P_CHEWY, ANI_WAIT);
		go_auto_xy(995, 82, P_HOWARD, ANI_WAIT);
		det->hide_static_spr(2);
		spieler_mi[P_CHEWY].Mode = false;
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		start_aad_wait(553, -1);
		_G(HowardMov) = 0;
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
	
	while (spieler_vector[P_HOWARD].Xypos[0] < 1080) {
		set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN0;
	}

	flic_cut(122, CFO_MODE);
	register_cutscene(34);

	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0);
	_G(spieler).scrollx = 720;
	set_person_pos(822, 98, P_CHEWY, P_LEFT);
	set_person_pos(861, 81, P_HOWARD, P_LEFT);
	del_inventar(_G(spieler).AkInvent);
	det->show_static_spr(21);
	_G(spieler).flags35_80 = true;
	start_aad_wait(546, -1);
	det->hide_static_spr(21);
	g_engine->_sound->playSound(9, 1);
	g_engine->_sound->stopSound(0);
	start_detail_wait(9, 0, ANI_GO);
	
	show_cur();
	return 1;
}

int Room97::proc6() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	auto_move(1, P_CHEWY);
	spieler_mi[P_CHEWY].Mode = true;
	go_auto_xy(298, 120, P_CHEWY, ANI_WAIT);
	set_person_spr(P_LEFT, P_CHEWY);
	start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
	det->hide_static_spr(15);
	start_detail_wait(1, 1, ANI_VOR);
	det->show_static_spr(28);
	auto_move(1, P_CHEWY);
	det->hide_static_spr(18);
	start_detail_wait(15, 1, ANI_VOR);
	det->show_static_spr(17);
	start_spz(CH_TALK5, 255, false, P_CHEWY);
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
	set_person_pos(304, 102, P_CHEWY, P_RIGHT);
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
		g_engine->_sound->playSound(26, 0);
		g_engine->_sound->playSound(26);
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
		start_spz(CH_TALK5, 255, false, P_CHEWY);
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
		start_spz(CH_TALK5, 255, false, P_CHEWY);
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
	g_engine->_sound->playSound(7, 0);
	g_engine->_sound->stopSound(1);
	start_detail_wait(7, 1, ANI_VOR);
	det->show_static_spr(19);
	_G(spieler).flags37_1 = true;
	atds->set_steuer_bit(543, ATS_AKTIV_BIT, ATS_DATEI);
	_G(menu_item) = CUR_WALK;
	cursor_wahl(CUR_WALK);

	show_cur();
	return 1;
}

int Room97::proc11() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();

	if (!_G(spieler).flags37_2) {
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		start_aad_wait(569, -1);
	} else {
		auto_move(9, P_CHEWY);
		start_aad_wait(570, -1);
		start_spz_wait(13, 1, false, P_CHEWY);
		auto_move(7, P_CHEWY);
		start_spz(CH_TALK6, 255, false, P_CHEWY);
		start_aad_wait(571, -1);
		g_engine->_sound->playSound(4, 0);
		g_engine->_sound->playSound(4);
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
	go_auto_xy(481, 39, P_CHEWY, ANI_WAIT);
	_G(spieler).PersonHide[P_CHEWY] = true;
	set_person_pos(327, 42, P_CHEWY, P_LEFT);
	wait_show_screen(50);
	set_person_pos(347, 42, P_CHEWY, P_LEFT);
	_G(spieler).PersonHide[P_CHEWY] = false;
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

int Room97::throwSlime() {
	if (!is_cur_inventar(SLIME_INV))
		return 0;

	_slimeThrown = true;
	hide_cur();
	del_inventar(_G(spieler).AkInvent);
	_G(menu_item) = CUR_USE;
	cursor_wahl(CUR_USE);
	
	return 1;
}

void Room97::sensorAnim() {
	if (_G(spieler).flags37_4 || flags.AutoAniPlay)
		return;

	flags.AutoAniPlay = true;
	stop_person(P_CHEWY);
	_G(menu_item) = CUR_USE;
	cursor_wahl(CUR_USE);
	set_person_pos(294, 42, P_CHEWY, P_LEFT);
	atds->del_steuer_bit(541, ATS_AKTIV_BIT, ATS_DATEI);
	_slimeThrown = false;
	det->start_detail(16, 1, 0);
	
	while (det->get_ani_status(16)) {
		get_user_key(NO_SETUP);
		if (minfo.button == 1 || _G(in)->get_switch_code() == 28) {
			if (_G(spieler).inv_cur)
				_G(maus_links_click) = true;
		}

		set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN;
	}

	hide_cur();
	if (!_slimeThrown) {
		start_detail_wait(17, 1, ANI_VOR);
		det->start_detail(16, 1, true);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(20, 1, false);
		set_person_pos(318, 42, P_CHEWY, P_LEFT);
		_G(spieler).PersonHide[P_CHEWY] = false;
	} else {
		_G(spieler).flags37_4 = true;
		del_inventar(_G(spieler).AkInvent);
		det->show_static_spr(27);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(21, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		det->hide_static_spr(27);
		start_detail_wait(18, 1, ANI_VOR);

		g_engine->_sound->playSound(8, 0);
		g_engine->_sound->stopSound(1);
		start_detail_wait(8, 1, ANI_VOR);
		det->show_static_spr(20);
		auto_move(10, P_CHEWY);
		auto_scroll(60, 0);
		start_aad_wait(544, -1);
		_bool18DB32 = true;
		auto_move(11, P_CHEWY);
		_bool18DB32 = false;
		flags.AutoAniPlay = false;
		proc13();
		flags.AutoAniPlay = true;
		auto_move(7, P_CHEWY);

		while (_G(spieler).scrollx < 368) {
			set_up_screen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		det->show_static_spr(26);
		set_person_pos(482, 24, P_NICHELLE, P_RIGHT);
		go_auto_xy(477, 29, P_NICHELLE, ANI_WAIT);
		det->hide_static_spr(26);
		_G(spieler).flags37_2 = true;
	}

	atds->set_steuer_bit(541, ATS_AKTIV_BIT, ATS_DATEI);
	show_cur();
	flags.AutoAniPlay = false;
}

} // namespace Rooms
} // namespace Chewy
