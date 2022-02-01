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
#include "chewy/rooms/room76.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

int Room76::_state;


void Room76::entry() {
	g_engine->_sound->playSound(0,0);
	g_engine->_sound->playSound(0, 1);
	g_engine->_sound->playSound(0);
	g_engine->_sound->playSound(0, 1, false);
	_G(spieler).ScrollxStep = 2;
	SetUpScreenFunc = setup_func;
	_state = 0;

	set_person_pos(308, 84, P_NICHELLE, P_RIGHT);
	set_person_pos(365, 84, P_HOWARD, P_RIGHT);
	_G(spieler).PersonHide[P_HOWARD] = true;
	_G(spieler).PersonHide[P_NICHELLE] = true;

	if (!_G(spieler).flags29_4) {
		atds->del_steuer_bit(453, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(457, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(458, ATS_AKTIV_BIT, ATS_DATEI);

	} else {
		det->del_static_ani(2);
		for (int i = 0; i < 3; ++i)
			det->hide_static_spr(8 + i);

		atds->set_steuer_bit(453, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_steuer_bit(457, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_steuer_bit(458, ATS_AKTIV_BIT, ATS_DATEI);
	}

	if (flags.LoadGame)
		return;

	if (!_G(spieler).flags29_2) {
		hide_cur();
		flags.NoScroll = true;
		_G(spieler).scrollx = 122;
		_G(spieler).flags29_2 = true;
		set_person_pos(128, 135, P_CHEWY, P_RIGHT);
		proc3(420);
		proc5();

	} else if (!_G(spieler).r76State) {
		hide_cur();
		flags.NoScroll = true;
		_G(spieler).scrollx = 122;
		_G(spieler).r76State = -1;
		set_person_pos(128, 135, P_CHEWY, P_RIGHT);
		proc3(422);
		flags.NoScroll = false;
	}
}

void Room76::xit() {
	_G(spieler).ScrollxStep = 1;
	_G(spieler).r76State = -1;
}

void Room76::setup_func() {
	if (_state != 1 || _G(spieler).scrollx < 300)
		return;

	_state = 0;
	det->start_detail(11, 1, false);
	det->start_detail(12, 1, false);
}

void Room76::talk1() {
	hide_cur();
	auto_move(4, P_CHEWY);
	auto_scroll(172, 0);
	const int diaNr = 425 + (_G(spieler).flags32_4 ? 1 : 0);
	start_aad_wait(diaNr, -1);
	show_cur();
}

void Room76::talk2() {
	hide_cur();
	auto_move(3, P_CHEWY);
	int diaNr;
	if (!_G(spieler).flags29_8)
		diaNr = 423;
	else if (!_G(spieler).flags29_40)
		diaNr = 438;
	else
		diaNr = 421;

	proc3(diaNr);
	if (diaNr == 421)
		proc5();

	show_cur();
}

void Room76::proc3(int diaNr) {
	start_aad_wait(diaNr, -1);
	det->del_static_ani(2);
	start_detail_wait(3, 1, ANI_VOR);
	start_detail_wait(4, 2, ANI_VOR);
	det->set_static_ani(2, -1);
}

void Room76::proc5() {
	_state = 1;
	det->del_static_ani(2);
	det->start_detail(6, 1, false);
	flags.NoScroll = false;
	wait_show_screen(15);
	auto_move(5, P_CHEWY);
	det->hide_static_spr(10);
	det->start_detail(13, 1, false);
	spieler_mi[P_CHEWY].Mode = true;
	go_auto_xy(669, 127, P_CHEWY, ANI_WAIT);
	wait_show_screen(20);
	spieler_mi[P_CHEWY].Mode = false;
	show_cur();
	set_person_pos(30, 150, P_CHEWY, P_RIGHT);
	switch_room(78);
}

int Room76::proc6() {
	int retVal = 0;

	if (is_cur_inventar(93)) {
		hide_cur();
		del_inventar(_G(spieler).AkInvent);
		_G(spieler).flags29_8 = true;
		retVal = 1;
		auto_move(3, P_CHEWY);
		proc3(424);
		start_spz_wait(13, 1, false, P_CHEWY);
		det->del_static_ani(2);
		start_detail_wait(5, 1, ANI_VOR);
		det->set_static_ani(2, -1);
		start_detail_wait(9, 1, ANI_VOR);
		start_detail_wait(10, 1, ANI_VOR);
		show_cur();
	} else if (is_cur_inventar(94)) {
		hide_cur();
		retVal = 1;
		set_person_spr(P_RIGHT, P_CHEWY);
		start_aad_wait(436, 1);
		show_cur();
	}

	return retVal;
}

int Room76::proc7() {
	if (!is_cur_inventar(96))
		return 0;
	
	if (_G(spieler).flags29_4) {
		hide_cur();
		auto_move(6, P_CHEWY);
		start_spz_wait(13, 1, false, P_CHEWY);
		del_inventar(_G(spieler).AkInvent);
		invent_2_slot(106);
		invent_2_slot(105);
		start_aad_wait(444, -1);
		out->ausblenden(0);
		out->set_teilpalette(pal, 255, 1);
		start_aad_wait(443, -1);
		_G(spieler).scrollx = 0;
		fx_blend = BLEND3;
		out->setze_zeiger(nullptr);
		out->cls();
		set_person_pos(128, 135, P_CHEWY, P_RIGHT);
		det->set_static_ani(2, -1);

		for (int i = 0; i < 3; ++i)
			det->show_static_spr(8 + i);

		start_aad_wait(427, -1);
		show_cur();
		start_ads_wait(21);
		hide_cur();
		start_aad_wait(428, -1);
		show_cur();
		_G(spieler).flags29_80 = true;
		switch_room(78);
	} else {
			hide_cur();
			start_spz(16, 255, false, P_CHEWY);
			start_aad_wait(577, -1);
			show_cur();
	}

	return 1;
}

} // namespace Rooms
} // namespace Chewy
