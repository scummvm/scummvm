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

namespace Chewy {
namespace Rooms {

static int16 R6_ROBO_PHASEN[4][2] = {
	{ 86, 86 },
	{ 86, 86 },
	{ 86, 86 },
	{ 86, 86 }
};

static MovLine R6_ROBO_MPKT[3] = {
	{ { 168,  71, 180 }, 1,  1 },
	{ { 180,  71, 100 }, 1,  2 },
	{ {  60, 210, 110 }, 1, 16 }
};

void Room6::entry() {
	_G(zoom_horizont) = 80;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 2;
	if (_G(spieler).R6BolaSchild) {
		if (_G(spieler).R6RaumBetreten < 3) {
			det->start_detail(7, 255, ANI_VOR);
			atds->del_steuer_bit(44, ATS_AKTIV_BIT, ATS_DATEI);
			if (!flags.LoadGame)
				++_G(spieler).R6RaumBetreten;
			if (_G(spieler).R6RaumBetreten == 3) {
				det->stop_detail(7);
				init_robo();
				wait_auto_obj(0);
				_G(spieler).R6BolaOk = true;
				obj->show_sib(SIB_BOLA_KNOPF_R6);
				obj->hide_sib(SIB_BOLA_R6);
				atds->set_steuer_bit(44, ATS_AKTIV_BIT, ATS_DATEI);
			}
		}
	}
}

void Room6::init_robo() {
#define ROBO_OBJ 0
	_G(auto_obj) = 1;
	mov_phasen[ROBO_OBJ].AtsText = 44;
	mov_phasen[ROBO_OBJ].Lines = 3;
	mov_phasen[ROBO_OBJ].Repeat = 1;
	mov_phasen[ROBO_OBJ].ZoomFak = 0;
	auto_mov_obj[ROBO_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[ROBO_OBJ].Delay = _G(spieler).DelaySpeed;
	auto_mov_obj[ROBO_OBJ].Mode = 1;
	init_auto_obj(ROBO_OBJ, &R6_ROBO_PHASEN[0][0], mov_phasen[ROBO_OBJ].Lines, (MovLine
		*)R6_ROBO_MPKT);
}

void Room6::bola_knopf() {
	int16 tmp;
	if (!_G(spieler).R6BolaBecher) {
		det->hide_static_spr(0);
		start_detail_wait(0, 1, ANI_VOR);
		if (_G(spieler).R6BolaOk) {
			_G(spieler).R6BolaBecher = true;
			det->show_static_spr(0);
			start_ani_block(2, ablock7);
			obj->calc_rsi_flip_flop(SIB_BOLA_FLECK_R6);
			wait_detail(2);
			obj->calc_rsi_flip_flop(SIB_BOLA_SCHACHT);
			atds->del_steuer_bit(42, ATS_AKTIV_BIT, ATS_DATEI);
			atds->set_ats_str(41, TXT_MARK_LOOK, 1, ATS_DATEI);
			obj->calc_rsi_flip_flop(SIB_BOLA_KNOPF_R6);
			obj->hide_sib(SIB_BOLA_KNOPF_R6);
		} else {
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_ani_block(3, ablock6);
			while (det->get_ani_status(3) && !SHOULD_QUIT) {
				if (!det->get_ani_status(14)) {
					set_person_pos(220, 89, P_CHEWY, P_LEFT);
					_G(spieler).PersonHide[P_CHEWY] = false;
				}
				set_up_screen(DO_SETUP);
			}
			det->show_static_spr(0);
			++_G(spieler).R6BolaJoke;
			if (_G(spieler).R6BolaJoke < 3)
				tmp = 3;
			else
				tmp = 4;
			start_aad_wait(tmp, -1);
		}
		obj->calc_rsi_flip_flop(SIB_BOLA_KNOPF_R6);
	}
}

int16 Room6::cut_serv1(int16 frame) {
	atds->print_aad(_G(spieler).scrollx, _G(spieler).scrolly);
	if (frame == 44)
		start_aad(108, 0);

	return 0;
}

int16 Room6::cut_serv2(int16 frame) {
	atds->print_aad(_G(spieler).scrollx, _G(spieler).scrolly);
	if (frame == 31)
		start_aad(107, 0);

	return 0;
}

} // namespace Rooms
} // namespace Chewy
