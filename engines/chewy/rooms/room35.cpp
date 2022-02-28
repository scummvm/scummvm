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
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room35.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK28[2] = {
	{ 4, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 5, 3, ANI_FRONT, ANI_WAIT, 0 },
};


void Room35::entry() {
	if (_G(spieler).R35Schublade)
		_G(det)->showStaticSpr(1);
}

int16 Room35::schublade() {
	int16 action_flag = false;
	hideCur();

	if (_G(spieler).ChewyAni != CHEWY_ROCKER && !_G(spieler).inv_cur) {
		if (!_G(spieler).R35Schublade) {
			action_flag = true;
			autoMove(3, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			_G(det)->showStaticSpr(1);
			_G(spieler).R35Schublade = true;
			_G(atds)->set_ats_str(234, 1, ATS_DATA);
		} else if (!_G(spieler).R35Falle) {
			action_flag = true;
			autoMove(3, P_CHEWY);
			_G(spieler).R35Falle = true;
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_ani_block(2, ABLOCK28);
			_G(spieler).PersonHide[P_CHEWY] = false;
			set_person_pos(33, 90, P_CHEWY, P_LEFT);
			start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
			start_aad_wait(93, -1);
			_G(spieler).PersonHide[P_CHEWY] = false;
			_G(atds)->set_ats_str(234, 2, ATS_DATA);
		}
	}

	showCur();
	return action_flag;
}

int16 Room35::use_cat() {
	int16 action_flag = false;
	hideCur();

	if (_G(spieler).ChewyAni != CHEWY_ROCKER) {
		if (is_cur_inventar(TRANSLATOR_INV)) {
			action_flag = true;
			autoMove(4, P_CHEWY);
			_G(spieler).R35TransCat = true;
			start_spz_wait(CH_TRANS, 1, false, P_CHEWY);
			flic_cut(FCUT_045);
			start_spz(CH_TRANS, 1, ANI_FRONT, P_CHEWY);
			start_aad_wait(94, -1);
		} else if (is_cur_inventar(PUTENKEULE_INV)) {
			action_flag = true;
			disable_timer();
			autoMove(4, P_CHEWY);
			_G(spieler).R35CatEat = true;
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			del_inventar(_G(spieler).AkInvent);
			_G(det)->stop_detail(0);
			_G(det)->del_static_ani(0);
			start_detail_wait(1, 1, ANI_FRONT);
			_G(det)->start_detail(2, 1, ANI_FRONT);
			start_detail_wait(3, 1, ANI_FRONT);
			_G(obj)->show_sib(SIB_KNOCHEN_R35);
			_G(obj)->calc_rsi_flip_flop(SIB_KNOCHEN_R35);
			_G(det)->showStaticSpr(7);
			_G(atds)->del_steuer_bit(237, ATS_AKTIV_BIT, ATS_DATA);

			while (_G(det)->get_ani_status(2) && !SHOULD_QUIT) {
				set_up_screen(DO_SETUP);
			}

			_G(det)->set_static_ani(0, -1);
			enable_timer();
			start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			start_aad_wait(95, -1);
		}
	}

	showCur();
	return action_flag;
}

void Room35::talk_cat() {
	hideCur();
	autoMove(4, P_CHEWY);

	if (_G(spieler).R35TransCat) {
		int16 dia_nr;
		if (!_G(spieler).R35CatEat) {
			dia_nr = 7;
		} else {
			dia_nr = 8;
		}

		_G(spieler).PersonHide[P_CHEWY] = true;
		switch_room(36);
		showCur();
		start_ads_wait(dia_nr);
		_G(spieler).PersonHide[P_CHEWY] = false;
		switch_room(35);

	} else {
		start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
		start_aad_wait(96, -1);
	}

	showCur();
}

} // namespace Rooms
} // namespace Chewy
