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
#include "chewy/globals.h"
#include "chewy/room.h"
#include "chewy/rooms/room31.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const MovLine SURIMY_MPKT[2] = {
	{ {  79, 152, 150 }, 1, 6 },
	{ { 273, 220, 150 }, 1, 6 }
};

static const AniBlock ABLOCK30[3] = {
	{ 0, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 1, 4, ANI_FRONT, ANI_WAIT, 0 },
	{ 2, 1, ANI_FRONT, ANI_WAIT, 0 },
};


void Room31::entry() {
	calc_luke();
	surimy_go();
}

void Room31::surimy_go() {
	if (!_G(spieler).R39ScriptOk && _G(spieler).R25SurimyLauf) {
		if (_G(spieler).R31SurimyGo >= 3) {
			hideCur();
			_G(spieler).R31SurimyGo = 0;
			_G(det)->load_taf_seq(39, 8, nullptr);
			_G(auto_obj) = 1;
			_G(mov_phasen)[SURIMY_OBJ].AtsText = 0;
			_G(mov_phasen)[SURIMY_OBJ].Lines = 2;
			_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
			_G(mov_phasen)[SURIMY_OBJ].ZoomFak = 0;
			_G(auto_mov_obj)[SURIMY_OBJ].Id = AUTO_OBJ0;
			_G(auto_mov_vector)[SURIMY_OBJ].Delay = _G(spieler).DelaySpeed + 2;
			_G(auto_mov_obj)[SURIMY_OBJ].Mode = true;
			init_auto_obj(SURIMY_OBJ, &SURIMY_TAF19_PHASEN[0][0], 2, (const MovLine *)SURIMY_MPKT);
			start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
			start_aad_wait(157, -1);
			wait_auto_obj(SURIMY_OBJ);
			_G(auto_obj) = 0;
			showCur();
		} else {
			++_G(spieler).R31SurimyGo;
		}
	}
}

void Room31::calc_luke() {
	if (!_G(spieler).R31KlappeZu) {
		for (int16 i = 0; i < 3; i++)
			_G(det)->show_static_spr(5 + i);

		_G(atds)->set_ats_str(244, 1, ATS_DATEI);
		_G(atds)->del_steuer_bit(245, ATS_AKTIV_BIT, ATS_DATEI);
		_G(spieler).room_e_obj[75].Attribut = AUSGANG_UNTEN;

	} else {
		for (int16 i = 0; i < 3; i++)
			_G(det)->hide_static_spr(5 + i);

		_G(atds)->set_ats_str(244, 0, ATS_DATEI);
		_G(atds)->set_steuer_bit(245, ATS_AKTIV_BIT, ATS_DATEI);
		_G(spieler).room_e_obj[75].Attribut = 255;
	}
}

int16 Room31::open_luke() {
	int16 action_flag = false;
	
	if (!_G(spieler).inv_cur && _G(spieler).R31KlappeZu) {
		action_flag = true;
		hideCur();
		autoMove(2, P_CHEWY);
		start_spz_wait(CH_LGET_U, 1, false, P_CHEWY);
		_G(spieler).R31KlappeZu = false;
		g_engine->_sound->playSound(3);
		calc_luke();
		showCur();
	}

	return action_flag;
}

int16 Room31::close_luke_proc1() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur && !_G(spieler).R31KlappeZu) {
		action_flag = true;
		hideCur();
		autoMove(2, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(spieler).R31KlappeZu = true;
		g_engine->_sound->playSound(3);
		calc_luke();
		showCur();
	}

	return action_flag;
}

void Room31::close_luke_proc3() {
	if (!_G(spieler).R31KlappeZu) {
		hideCur();
		autoMove(2, P_CHEWY);
		start_spz_wait(13, 1, false, P_CHEWY);
		_G(spieler).R31KlappeZu = true;
		g_engine->_sound->playSound(3);
		g_engine->_sound->playSound(3, 1, false);
		calc_luke();
		showCur();
	}
}

int16 Room31::use_topf() {
	int16 dia_nr = -1;
	int16 ani_nr = 0;
	int16 action_flag = false;

	hideCur();
	if (_G(spieler).inv_cur) {
		if (_G(spieler).R31PflanzeWeg) {
			if (is_cur_inventar(K_KERNE_INV)) {
				_G(spieler).R31KoernerDa = true;
				autoMove(1, P_CHEWY);
				start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
				del_inventar(_G(spieler).AkInvent);
				ani_nr = CH_TALK3;
				dia_nr = 150;
				_G(atds)->set_ats_str(242, 2, ATS_DATEI);

			} else if (is_cur_inventar(MILCH_WAS_INV)) {
				if (_G(spieler).R31KoernerDa) {
					_G(spieler).R31Wasser = true;
					autoMove(1, P_CHEWY);
					_G(spieler).PersonHide[P_CHEWY] = true;
					start_ani_block(3, ABLOCK30);
					_G(spieler).PersonHide[P_CHEWY] = false;
					del_inventar(_G(spieler).AkInvent);
					_G(obj)->addInventory(MILCH_LEER_INV, &_G(room_blk));
					inventory_2_cur(MILCH_LEER_INV);
					ani_nr = CH_TALK6;
					dia_nr = 151;
					_G(atds)->set_ats_str(242, 3, ATS_DATEI);
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
							close_luke_proc3();
							autoMove(3, P_CHEWY);
							flic_cut(FCUT_046);
							register_cutscene(13);
							_G(spieler).R31SurFurz = true;
							ani_nr = CH_TALK6;
							dia_nr = 156;
							_G(atds)->set_ats_str(242, 4, ATS_DATEI);
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
		start_spz(ani_nr, 255, ANI_FRONT, P_CHEWY);
		start_aad_wait(dia_nr, -1);
		action_flag = true;
	}

	showCur();
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
