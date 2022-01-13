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
#include "chewy/rooms/room9.h"

namespace Chewy {
namespace Rooms {

#define SURIMY_OBJ 0

static const int16 SURIMY_PHASEN[4][2] = {
	{ 91, 98 },
	{ 91, 98 },
	{ 91, 98 },
	{ 91, 98 }
};

static const MovLine SURIMY_MPKT[2] = {
	{ {  80, 170, 199 }, 2, 6 },
	{ { 210, 162, 199 }, 2, 6 }
};

static const MovLine SURIMY_MPKT1[2] = {
	{ { 224, 158, 199 }, 2, 6 },
	{ { 330, 162, 199 }, 2, 6 }
};

void Room9::entry() {
	_G(spieler).R7ChewyFlug = false;

	if (!_G(spieler).R9Gitter) {
		set_person_pos(138, 91, P_CHEWY, P_LEFT);
	} else
		det->show_static_spr(5);
	if (_G(spieler).R9Surimy) {
		det->hide_static_spr(4);
		room->set_timer_status(7, TIMER_STOP);
	}
}

void Room9::gtuer() {
	_G(spieler).R9Gitter = true;
	det->show_static_spr(5);
	start_detail_wait(6, 1, ANI_VOR);
	set_person_pos(74, 93, P_CHEWY, P_LEFT);
	atds->del_steuer_bit(74, ATS_AKTIV_BIT, ATS_DATEI);
	atds->del_steuer_bit(75, ATS_AKTIV_BIT, ATS_DATEI);
	atds->del_steuer_bit(76, ATS_AKTIV_BIT, ATS_DATEI);
	obj->show_sib(34);
	atds->set_ats_str(73, 1, ATS_DATEI);
}

void Room9::surimy() {
	int16 tmp;
	_G(spieler).R9Surimy = true;
	tmp = spieler_vector[P_CHEWY].Count;
	stop_person(P_CHEWY);
	atds->set_steuer_bit(75, ATS_AKTIV_BIT, ATS_DATEI);
	det->hide_static_spr(4);
	room->set_timer_status(7, TIMER_STOP);
	surimy_ani();
	start_aad_wait(56, -1);
	_G(spieler).room_e_obj[17].Attribut = AUSGANG_RECHTS;
	spieler_vector[P_CHEWY].Count = tmp;
	get_phase(&spieler_vector[P_CHEWY], &spieler_mi[P_CHEWY]);
	mov->continue_auto_go();
}

void Room9::surimy_ani() {
	det->load_taf_seq(91, 8, 0);
	_G(auto_obj) = 1;
	mov_phasen[SURIMY_OBJ].AtsText = 0;
	mov_phasen[SURIMY_OBJ].Lines = 2;
	mov_phasen[SURIMY_OBJ].Repeat = 1;
	mov_phasen[SURIMY_OBJ].ZoomFak = 0;
	auto_mov_obj[SURIMY_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[SURIMY_OBJ].Delay = _G(spieler).DelaySpeed;
	auto_mov_obj[SURIMY_OBJ].Mode = true;
	init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], mov_phasen[SURIMY_OBJ].Lines,
		(const MovLine *)SURIMY_MPKT);
	wait_auto_obj(SURIMY_OBJ);
	start_detail_frame(0, 1, ANI_VOR, 15);
	det->start_detail(2, 1, ANI_VOR);
	wait_detail(0);
	start_detail_wait(1, 1, ANI_VOR);
	start_spz(CH_EKEL, 2, ANI_VOR, P_CHEWY);
	det->hide_static_spr(4);
	mov_phasen[SURIMY_OBJ].Repeat = 1;
	init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], mov_phasen[SURIMY_OBJ].Lines,
		(const MovLine *)SURIMY_MPKT1);
	wait_auto_obj(SURIMY_OBJ);
	det->del_taf_tbl(91, 8, 0);
}

} // namespace Rooms
} // namespace Chewy
