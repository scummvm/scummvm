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
#include "chewy/rooms/room31.h"

namespace Chewy {
namespace Rooms {

#undef SURIMY_OBJ
#define SURIMY_OBJ 0

static const MovLine SURIMY_MPKT[2] = {
	{ {  79, 152, 150 }, 1, 6 },
	{ { 273, 220, 150 }, 1, 6 }
};

void Room31::entry() {
	calc_luke();
	surimy_go();
}

void Room31::surimy_go() {
	if (!_G(spieler).R39ScriptOk) {
		if (_G(spieler).R31SurimyGo >= 3) {
			hide_cur();
			_G(spieler).R31SurimyGo = 0;
			det->load_taf_seq(39, 8, 0);
			_G(auto_obj) = 1;
			mov_phasen[SURIMY_OBJ].AtsText = 0;
			mov_phasen[SURIMY_OBJ].Lines = 2;
			mov_phasen[SURIMY_OBJ].Repeat = 1;
			mov_phasen[SURIMY_OBJ].ZoomFak = 0;
			auto_mov_obj[SURIMY_OBJ].Id = AUTO_OBJ0;
			auto_mov_vector[SURIMY_OBJ].Delay = _G(spieler).DelaySpeed + 2;
			auto_mov_obj[SURIMY_OBJ].Mode = true;
			init_auto_obj(SURIMY_OBJ, &SURIMY_TAF19_PHASEN[0][0], mov_phasen[SURIMY_OBJ].Lines,
				(const MovLine *)SURIMY_MPKT);
			start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(157, -1);
			wait_auto_obj(SURIMY_OBJ);
			_G(auto_obj) = 0;
			show_cur();
		} else
			++_G(spieler).R31SurimyGo;
	}
}

void Room31::calc_luke() {
	int16 i;
	if (!_G(spieler).R31KlappeZu) {
		for (i = 0; i < 3; i++)
			det->show_static_spr(5 + i);
		atds->set_ats_str(244, 1, ATS_DATEI);
		atds->del_steuer_bit(245, ATS_AKTIV_BIT, ATS_DATEI);
		_G(spieler).room_e_obj[75].Attribut = AUSGANG_UNTEN;
	} else {
		for (i = 0; i < 3; i++)
			det->hide_static_spr(5 + i);
		atds->set_ats_str(244, 0, ATS_DATEI);
		atds->set_steuer_bit(245, ATS_AKTIV_BIT, ATS_DATEI);
		_G(spieler).room_e_obj[75].Attribut = 255;
	}
}

void Room31::open_luke() {
	if (_G(spieler).R31KlappeZu) {
		auto_move(2, P_CHEWY);
		start_spz_wait(CH_LGET_U, 1, ANI_VOR, P_CHEWY);
		_G(spieler).R31KlappeZu = false;
		calc_luke();
	}
}

void Room31::close_luke() {
	if (!_G(spieler).R31KlappeZu) {
		auto_move(2, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		_G(spieler).R31KlappeZu = true;
		calc_luke();
	}
}

int16 Room31::use_topf() {
	int16 dia_nr = -1;
	int16 ani_nr = 0;
	int16 action_flag = false;

	hide_cur();
	if (_G(spieler).inv_cur) {
		if (_G(spieler).R31PflanzeWeg) {
			if (is_cur_inventar(K_KERNE_INV)) {
				_G(spieler).R31KoernerDa = true;
				auto_move(1, P_CHEWY);
				start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
				del_inventar(_G(spieler).AkInvent);
				ani_nr = CH_TALK3;
				dia_nr = 150;
				atds->set_ats_str(242, 2, ATS_DATEI);
			} else if (is_cur_inventar(MILCH_WAS_INV)) {
				if (_G(spieler).R31KoernerDa) {
					_G(spieler).R31Wasser = true;
					auto_move(1, P_CHEWY);
					_G(spieler).PersonHide[P_CHEWY] = true;
					start_ani_block(3, ablock30);
					_G(spieler).PersonHide[P_CHEWY] = false;
					del_inventar(_G(spieler).AkInvent);
					obj->add_inventar(MILCH_LEER_INV, &room_blk);
					inventory_2_cur(MILCH_LEER_INV);
					ani_nr = CH_TALK6;
					dia_nr = 151;
					atds->set_ats_str(242, 3, ATS_DATEI);
				} else {
					ani_nr = CH_TALK5;
					dia_nr = 152;
				}
			} else if (is_cur_inventar(SURIMY_INV)) {
				if (!_G(spieler).R31SurFurz) {
					if (_G(spieler).R31Wasser) {
						if (!_G(spieler).R28SurimyCar) {
							ani_nr = CH_TALK5;
							dia_nr = 180;
						} else {
							close_luke();
							auto_move(3, P_CHEWY);
							flic_cut(FCUT_046, FLC_MODE);
							_G(spieler).R31SurFurz = true;
							ani_nr = CH_TALK6;
							dia_nr = 156;
							atds->set_ats_str(242, 4, ATS_DATEI);
							cur_2_inventory();
						}
					} else {
						ani_nr = CH_TALK5;
						dia_nr = 155;
					}
				}
			} else {
				ani_nr = CH_TALK5;
				dia_nr = 153;
			}
		} else {
			ani_nr = CH_TALK5;
			dia_nr = 154;
		}
	}
	if (dia_nr != -1) {
		start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(dia_nr, -1);
		action_flag = true;
	}
	show_cur();
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
