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

	r97_bool18DB32 = false;
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

	r97_word18DB2E = 0;
	r97_word18DB34 = 332;
	r97_word18DB36 = 110;
	r97_word18DB38 = 132;

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
	Sdi[18].z_ebene = 6;
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
		
	}
	show_cur();
}

void Room97::proc4() {
}

int Room97::proc5() {
	return 0;
}

int Room97::proc6() {
	return 0;
}

int Room97::proc7() {
	return 0;
}

int Room97::proc8() {
	return 0;
}

int Room97::proc9() {
	return 0;
}

int Room97::proc10() {
	return 0;
}

int Room97::proc11() {
	return 0;
}

void Room97::proc12() {
}

void Room97::proc13() {
}

int Room97::proc14() {
	return 0;
}

void Room97::proc15() {
}

} // namespace Rooms
} // namespace Chewy
