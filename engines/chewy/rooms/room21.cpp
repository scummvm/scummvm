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
#include "chewy/rooms/room21.h"

namespace Chewy {
namespace Rooms {

#define SPINNE1_OBJ 0
#define SPINNE2_OBJ 1
#define ENERGIE_OBJ 2

static const int16 SPINNE_PHASEN[4][2] = {
	{ 58, 65 },
	{ 50, 57 },
	{ 42, 49 },
	{ 66, 89 }
};

static const MovLine SPINNE_MPKT[3] = {
	{ { 550, 275, 0 }, 0, 2 },
	{ { 396, 275, 0 }, 0, 2 },
	{ { 530, 275, 0 }, 1, 2 }
};

static const MovLine SPINNE_MPKT1[2] = {
	{ { 104,  50, 0 }, 3, 3 },
	{ { 104, 430, 0 }, 3, 3 }
};

static const MovLine SPINNE_MPKT2[2] = {
	{ { 115, 140, 0 }, 2, 1 },
	{ { 115, 190, 0 }, 2, 1 }
};

static const AniBlock ABLOCK19[3] = {
	{ 12, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 13, 4, ANI_VOR, ANI_WAIT, 0 },
	{ 14, 1, ANI_VOR, ANI_WAIT, 0 },
};

void Room21::entry() {
	_G(spieler).ScrollxStep = 2;
	_G(spieler).ScrollyStep = 2;
	load_chewy_taf(CHEWY_MINI);
	calc_laser();
	init_spinne();
	_G(timer_nr)[2] = room->set_timer(255, 1);
	flags.NoEndPosMovObj = true;
	SetUpScreenFunc = setup_func;
}

bool Room21::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0]) {
		restart_spinne2();
	} else if (t_nr == _G(timer_nr)[2])
		chewy_kolli();

	return false;
}

void Room21::calc_laser() {
	if (_G(spieler).R21Hebel1 && !_G(spieler).R21Hebel2 && _G(spieler).R21Hebel3) {
		_G(spieler).R21Laser1Weg = true;
		det->stop_detail(3);
		atds->set_steuer_bit(134, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(133, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		_G(spieler).R21Laser1Weg = false;
		det->start_detail(3, 255, ANI_VOR);
		atds->del_steuer_bit(134, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_steuer_bit(133, ATS_AKTIV_BIT, ATS_DATEI);
	}

	if (!_G(spieler).R21Hebel1 && _G(spieler).R21Hebel2 && !_G(spieler).R21Hebel3) {
		if (!obj->check_inventar(SEIL_INV) && !_G(spieler).R17Seil) {
			obj->show_sib(SIB_SEIL_R21);
			atds->del_steuer_bit(129, ATS_AKTIV_BIT, ATS_DATEI);
		}

		_G(spieler).R21Laser2Weg = true;
		det->stop_detail(4);
		atds->set_steuer_bit(135, ATS_AKTIV_BIT, ATS_DATEI);

	} else {
		obj->hide_sib(SIB_SEIL_R21);
		atds->set_steuer_bit(129, ATS_AKTIV_BIT, ATS_DATEI);
		_G(spieler).R21Laser2Weg = false;
		det->start_detail(4, 255, ANI_VOR);
		atds->del_steuer_bit(135, ATS_AKTIV_BIT, ATS_DATEI);
	}
}

void Room21::init_spinne() {
	det->load_taf_seq(42, 48, nullptr);
	_G(auto_obj) = 2;

	mov_phasen[SPINNE1_OBJ].AtsText = 130;
	mov_phasen[SPINNE1_OBJ].Lines = 3;
	mov_phasen[SPINNE1_OBJ].Repeat = 255;
	mov_phasen[SPINNE1_OBJ].ZoomFak = 0;
	auto_mov_obj[SPINNE1_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[SPINNE1_OBJ].Delay = _G(spieler).DelaySpeed;
	auto_mov_obj[SPINNE1_OBJ].Mode = true;
	init_auto_obj(SPINNE1_OBJ, &SPINNE_PHASEN[0][0], 3, (const MovLine *)SPINNE_MPKT);

	mov_phasen[SPINNE2_OBJ].AtsText = 130;
	mov_phasen[SPINNE2_OBJ].Lines = 2;
	mov_phasen[SPINNE2_OBJ].Repeat = 1;
	mov_phasen[SPINNE2_OBJ].ZoomFak = 0;
	auto_mov_obj[SPINNE2_OBJ].Id = AUTO_OBJ1;
	auto_mov_vector[SPINNE2_OBJ].Delay = _G(spieler).DelaySpeed;
	auto_mov_obj[SPINNE2_OBJ].Mode = true;
	init_auto_obj(SPINNE2_OBJ, &SPINNE_PHASEN[0][0], 2, (const MovLine *)SPINNE_MPKT1);
	_G(timer_nr)[0] = room->set_timer(255, 21);
	_G(e_streifen) = false;
}

void Room21::restart_spinne2() {
	mov_phasen[SPINNE2_OBJ].Repeat = 1;
	init_auto_obj(SPINNE2_OBJ, &SPINNE_PHASEN[0][0], mov_phasen[SPINNE2_OBJ].Lines, (const MovLine *)SPINNE_MPKT1);
	uhr->reset_timer(_G(timer_nr)[0], 0);
	_G(e_streifen) = false;
}

void Room21::setup_func() {
	if (auto_mov_vector[SPINNE2_OBJ].Xypos[1] >= 190 && !_G(e_streifen)) {
		_G(e_streifen) = true;

		_G(auto_obj) = 3;
		mov_phasen[ENERGIE_OBJ].AtsText = 0;
		mov_phasen[ENERGIE_OBJ].Lines = 2;
		mov_phasen[ENERGIE_OBJ].Repeat = 1;
		mov_phasen[ENERGIE_OBJ].ZoomFak = 0;
		auto_mov_obj[ENERGIE_OBJ].Id = AUTO_OBJ2;
		auto_mov_vector[ENERGIE_OBJ].Delay = _G(spieler).DelaySpeed;
		auto_mov_obj[ENERGIE_OBJ].Mode = true;
		init_auto_obj(ENERGIE_OBJ, &SPINNE_PHASEN[0][0], 2, (const MovLine *)SPINNE_MPKT2);
	}
}

void Room21::chewy_kolli() {
	int16 kolli = false;

	for (int16 i = 0; i < 3 && !kolli; i++) {
		int16 spr_nr = mov_phasen[i].Phase[auto_mov_vector[i].Phase][0] + auto_mov_vector[i].PhNr;
		int16 *xy = (int16 *)room_blk.DetImage[spr_nr];
		int16 *Cxy = room_blk.DetKorrekt + (spr_nr << 1);
		int16 xoff = xy[0];
		int16 yoff = xy[1];
		if (i == 2)
			xoff += 10;
		xoff += auto_mov_vector[i].Xzoom;
		yoff += auto_mov_vector[i].Yzoom;

		if ((!i && spieler_vector[P_CHEWY].Xypos[0] < 516) ||
			(i == 1 && spieler_vector[P_CHEWY].Xypos[1] > 70) ||
			(i == 2)) {
			if (spieler_vector[P_CHEWY].Xypos[0] + 12 >= auto_mov_vector[i].Xypos[0] + Cxy[0] &&
				spieler_vector[P_CHEWY].Xypos[0] + 12 <= auto_mov_vector[i].Xypos[0] + xoff + Cxy[0] &&
				spieler_vector[P_CHEWY].Xypos[1] + 12 >= auto_mov_vector[i].Xypos[1] + Cxy[1] &&
				spieler_vector[P_CHEWY].Xypos[1] + 12 <= auto_mov_vector[i].Xypos[1] + yoff + Cxy[1] &&
				mov_phasen[i].Start == 1)
				kolli = true;
		}
	}

	if (kolli && !flags.AutoAniPlay) {
		const int16 tmp = spieler_vector[P_CHEWY].Count;
		stop_person(P_CHEWY);
		flags.AutoAniPlay = true;
		_G(spieler).PersonHide[P_CHEWY] = true;
		int16 ani_nr = (spieler_vector[P_CHEWY].Xyvo[0] < 0) ? 10 : 11;
		det->set_detail_pos(ani_nr, spieler_vector[P_CHEWY].Xypos[0], spieler_vector[P_CHEWY].Xypos[1]);
		start_detail_wait(ani_nr, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		flags.AutoAniPlay = false;
		spieler_vector[P_CHEWY].Count = tmp;
		get_phase(&spieler_vector[P_CHEWY], &spieler_mi[P_CHEWY]);
		mov->continue_auto_go();
	}
}

void Room21::salto() {
	if (!_G(spieler).inv_cur && atds->get_ats_str(134, TXT_MARK_USE, ATS_DATEI) == 8
		&& !_G(spieler).R21Salto && !flags.AutoAniPlay) {
		_G(spieler).R21Salto = true;
		flags.AutoAniPlay = true;
		_G(spieler).PersonHide[P_CHEWY] = true;

		for (int16 i = 0; i < 3; i++) {
			det->set_detail_pos(12 + i, spieler_vector[P_CHEWY].Xypos[0],
				spieler_vector[P_CHEWY].Xypos[1]);
		}

		start_ani_block(3, ABLOCK19);
		_G(spieler).PersonHide[P_CHEWY] = false;
		start_aad_wait(36, -1);
		flags.AutoAniPlay = false;
	}
}

void Room21::use_gitter_energie() {
	_G(spieler).R21GitterEnergie = exit_flip_flop(-1, 47, -1, 131, 138, -1,
		AUSGANG_UNTEN, AUSGANG_OBEN, (int16)_G(spieler).R21GitterEnergie);
	_G(auto_obj) = 0;
	_G(spieler).R17Location = 1;
	_G(spieler).PersonHide[P_CHEWY] = true;

	switch_room(17);
	det->hide_static_spr(5);
	start_detail_wait(9, 1, ANI_VOR);
	_G(spieler).R17GitterWeg = true;
	_G(spieler).PersonHide[P_CHEWY] = false;
}

int16 Room21::use_fenster() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur && !flags.AutoAniPlay && _G(spieler).R21Laser1Weg) {
		action_flag = true;
		flags.AutoAniPlay = true;
		_G(spieler).R18Gitter = true;
		auto_move(13, P_CHEWY);
		set_person_pos(541, 66, P_CHEWY, P_LEFT);
		switch_room(18);

		if (!_G(spieler).R18FirstEntry) {
			start_aad_wait(39, -1);
			_G(spieler).R18FirstEntry = true;
		}

		_G(spieler).room_e_obj[50].Attribut = AUSGANG_OBEN;
		_G(spieler).room_e_obj[41].Attribut = 255;
		flags.AutoAniPlay = false;
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
