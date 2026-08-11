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

#include "chewy/cursor.h"
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
	if (!_G(gameState).R39ScriptOk && _G(gameState).R25SurimyLauf) {
		if (_G(gameState).R31SurimyGo >= 3) {
			hideCur();
			_G(gameState).R31SurimyGo = 0;
			_G(det)->load_taf_seq(39, 8, nullptr);
			_G(auto_obj) = 1;
			_G(mov_phasen)[SURIMY_OBJ].AtsText = 0;
			_G(mov_phasen)[SURIMY_OBJ].Lines = 2;
			_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
			_G(mov_phasen)[SURIMY_OBJ].ZoomFak = 0;
			_G(auto_mov_obj)[SURIMY_OBJ].Id = AUTO_OBJ0;
			_G(auto_mov_vector)[SURIMY_OBJ].Delay = _G(gameState).DelaySpeed + 2;
			_G(auto_mov_obj)[SURIMY_OBJ].Mode = true;
			init_auto_obj(SURIMY_OBJ, &SURIMY_TAF19_PHASES[0][0], 2, (const MovLine *)SURIMY_MPKT);
			start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
			startAadWait(157);
			wait_auto_obj(SURIMY_OBJ);
			_G(auto_obj) = 0;
			showCur();
		} else {
			++_G(gameState).R31SurimyGo;
		}
	}
}

void Room31::calc_luke() {
	if (!_G(gameState).R31KlappeZu) {
		for (int16 i = 0; i < 3; i++)
			_G(det)->showStaticSpr(5 + i);

		_G(atds)->set_all_ats_str(244, 1, ATS_DATA);
		_G(atds)->delControlBit(245, ATS_ACTIVE_BIT);
		_G(gameState).room_e_obj[75].Attribut = EXIT_BOTTOM;

	} else {
		for (int16 i = 0; i < 3; i++)
			_G(det)->hideStaticSpr(5 + i);

		_G(atds)->set_all_ats_str(244, 0, ATS_DATA);
		_G(atds)->setControlBit(245, ATS_ACTIVE_BIT);
		_G(gameState).room_e_obj[75].Attribut = 255;
	}
}

int16 Room31::open_luke() {
	int16 action_flag = false;
	
	if (!_G(cur)->usingInventoryCursor() && _G(gameState).R31KlappeZu) {
		action_flag = true;
		hideCur();
		autoMove(2, P_CHEWY);
		start_spz_wait(CH_LGET_U, 1, false, P_CHEWY);
		_G(gameState).R31KlappeZu = false;
		_G(det)->playSound(3, 0);
		calc_luke();
		showCur();
	}

	return action_flag;
}

int16 Room31::close_luke_proc1() {
	int16 action_flag = false;

	if (!_G(cur)->usingInventoryCursor() && !_G(gameState).R31KlappeZu) {
		action_flag = true;
		hideCur();
		autoMove(2, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(gameState).R31KlappeZu = true;
		_G(det)->playSound(3, 0);
		calc_luke();
		showCur();
	}

	return action_flag;
}

void Room31::close_luke_proc3() {
	if (!_G(gameState).R31KlappeZu) {
		hideCur();
		autoMove(2, P_CHEWY);
		start_spz_wait(13, 1, false, P_CHEWY);
		_G(gameState).R31KlappeZu = true;
		_G(det)->playSound(3, 0);
		_G(det)->playSound(3, 1);
		calc_luke();
		showCur();
	}
}

int16 Room31::use_topf() {
	int16 dia_nr = -1;
	int16 ani_nr = 0;
	int16 action_flag = false;

	hideCur();
	if (_G(cur)->usingInventoryCursor()) {
		if (_G(gameState).R31PflanzeWeg) {
			if (isCurInventory(K_KERNE_INV)) {
				_G(gameState).R31KoernerDa = true;
				autoMove(1, P_CHEWY);
				start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
				delInventory(_G(cur)->getInventoryCursor());
				ani_nr = CH_TALK3;
				dia_nr = 150;
				_G(atds)->set_all_ats_str(242, 2, ATS_DATA);

			} else if (isCurInventory(WATER_FILLED_BOTTLE_INV)) {
				if (_G(gameState).R31KoernerDa) {
					_G(gameState).R31Wasser = true;
					autoMove(1, P_CHEWY);
					_G(gameState)._personHide[P_CHEWY] = true;
					startAniBlock(3, ABLOCK30);
					_G(gameState)._personHide[P_CHEWY] = false;
					delInventory(_G(cur)->getInventoryCursor());
					_G(obj)->addInventory(EMPTY_MILK_BOTTLE_INV, &_G(room_blk));
					inventory_2_cur(EMPTY_MILK_BOTTLE_INV);
					ani_nr = CH_TALK6;
					dia_nr = 151;
					_G(atds)->set_all_ats_str(242, 3, ATS_DATA);
				} else {
					ani_nr = CH_TALK5;
					dia_nr = 152;
				}
			} else if (isCurInventory(SURIMY_INV)) {
				if (!_G(gameState).R31SurFurz) {
					if (_G(gameState).R31Wasser) {
						if (!_G(gameState).R28SurimyCar) {
							ani_nr = CH_TALK5;
							dia_nr = 180;
						} else {
							close_luke_proc3();
							autoMove(3, P_CHEWY);
							flic_cut(FCUT_046);
							register_cutscene(13);
							_G(gameState).R31SurFurz = true;
							ani_nr = CH_TALK6;
							dia_nr = 156;
							_G(atds)->set_all_ats_str(242, 4, ATS_DATA);
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
		startAadWait(dia_nr);
		action_flag = true;
	}

	showCur();
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
