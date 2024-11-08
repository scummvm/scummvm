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
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room22.h"

namespace Chewy {
namespace Rooms {

#define BORK_OBJ 0

static const int16 BORK_PHASEN[4][2] = {
	{ 51, 56 },
	{ 36, 41 },
	{ 36, 41 },
	{ 36, 41 }
};

static const MovLine BORK_MPKT[2] = {
	{ { -30, 125, 170} , 1, 6 },
	{ { 155, 125, 170 }, 1, 6 },
};

static const MovLine BORK_MPKT1[2] = {
	{ { 155, 125, 170}, 2, 6 },
	{ { -30, 125, 170}, 0, 6 }
};

static const MovLine BORK_MPKT2[2] = {
	{ { -30, 125, 170 }, 1, 6 },
	{ {  90, 125, 170 }, 1, 6 }
};

AniBlock ABLOCK14[2] = {
	{0, 1, ANI_FRONT, ANI_WAIT, 0},
	{3, 255, ANI_FRONT, ANI_GO, 0},
};


void Room22::entry() {
	if (!_G(gameState).R22BorkPlatt) {
		_G(det)->load_taf_seq(36, 21, nullptr);
		_G(room)->set_timer(255, 15);
	} else if (!_G(gameState).R22GetBork) {
		_G(det)->showStaticSpr(4);
	}
}

bool Room22::timer(int16 t_nr, int16 ani_nr) {
	if (!ani_nr && !_G(flags).ExitMov) {
		bork(t_nr);
	}

	return false;
}

int16 Room22::chewy_amboss() {
	int16 action_flag = false;
	if (!_G(gameState).R22ChewyPlatt && !_G(cur)->usingInventoryCursor() && !_G(flags).AutoAniPlay) {
		action_flag = true;
		_G(flags).AutoAniPlay = true;
		hideCur();

		autoMove(5, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(1, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		autoMove(2, P_CHEWY);
		_G(flags).NoPalAfterFlc = false;
		flic_cut(FCUT_006);
		_G(gameState).R22ChewyPlatt = true;
		_G(atds)->set_all_ats_str(79, 1, ATS_DATA);
		_G(flags).AutoAniPlay = false;

		showCur();
	}
	return action_flag;
}

void Room22::bork(int16 t_nr) {
	if (!_G(flags).AutoAniPlay && !is_chewy_busy()) {
		_G(flags).AutoAniPlay = true;

		if (!_G(gameState).R22BorkPlatt) {
			hideCur();
			start_spz(CH_TALK2, 255, ANI_FRONT, P_CHEWY);
			startAadWait(10);
			autoMove(3, P_CHEWY);
			_G(auto_obj) = 1;
			_G(mov_phasen)[BORK_OBJ].AtsText = 0;
			_G(mov_phasen)[BORK_OBJ].Lines = 2;
			_G(mov_phasen)[BORK_OBJ].Repeat = 1;
			_G(mov_phasen)[BORK_OBJ].ZoomFak = 0;
			_G(auto_mov_obj)[BORK_OBJ].Id = AUTO_OBJ0;
			_G(auto_mov_vector)[BORK_OBJ].Delay = _G(gameState).DelaySpeed;
			_G(auto_mov_obj)[BORK_OBJ].Mode = true;

			if (!_G(gameState).R22Paint) {
				bork_walk1();
			} else {
				if (!_G(gameState).R22ChewyPlatt) {
					_G(atds)->setControlBit(79, ATS_ACTIVE_BIT);
					_G(gameState).R22ChewyPlatt = true;
				}

				bork_walk2();
			}

			showCur();
		}

		_G(uhr)->resetTimer(t_nr, 0);
		_G(flags).AutoAniPlay = false;
	}
}

void Room22::bork_walk1() {
	init_auto_obj(BORK_OBJ, &BORK_PHASEN[0][0], _G(mov_phasen)[BORK_OBJ].Lines, (const MovLine *)BORK_MPKT);
	wait_auto_obj(BORK_OBJ);

	startSetAILWait(2, 1, ANI_FRONT);

	_G(mov_phasen)[BORK_OBJ].Repeat = 1;
	init_auto_obj(BORK_OBJ, &BORK_PHASEN[0][0], _G(mov_phasen)[BORK_OBJ].Lines, (const MovLine *)BORK_MPKT1);
	wait_auto_obj(BORK_OBJ);
}

void Room22::bork_walk2() {
	init_auto_obj(BORK_OBJ, &BORK_PHASEN[0][0], _G(mov_phasen)[BORK_OBJ].Lines, (const MovLine *)BORK_MPKT2);
	wait_auto_obj(BORK_OBJ);

	flic_cut(FCUT_008);
	register_cutscene(3);
	_G(det)->showStaticSpr(4);
	_G(atds)->delControlBit(81, ATS_ACTIVE_BIT);
	_G(gameState).R22BorkPlatt = true;
	_G(atds)->setControlBit(79, ATS_ACTIVE_BIT);
}

void Room22::get_bork() {
	if (!_G(gameState).R22GetBork && _G(gameState).R22BorkPlatt) {
		autoMove(4, P_CHEWY);
		_G(det)->hideStaticSpr(4);
		_G(gameState)._personHide[P_CHEWY] = true;
		startAniBlock(2, ABLOCK14);
		setPersonPos(171, 120, P_CHEWY, P_LEFT);
		startAadWait(11);
		_G(det)->stopDetail(3);
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(atds)->setControlBit(81, ATS_ACTIVE_BIT);
		invent_2_slot(BORK_INV);

		_G(gameState).R22GetBork = true;
		_G(menu_item) = CUR_WALK;
		cursorChoice(_G(menu_item));
	}
}

int16 Room22::malen() {
	int16 action_flag = false;

	if (!_G(flags).AutoAniPlay && isCurInventory(17)) {
		action_flag = true;
		_G(flags).AutoAniPlay = true;
		autoMove(8, P_CHEWY);
		flic_cut(FCUT_007);
		_G(atds)->set_ats_str(82, TXT_MARK_LOOK, 1, ATS_DATA);
		_G(gameState).R22Paint = true;
		_G(obj)->calc_rsi_flip_flop(SIB_PAINT_R22);
		_G(obj)->hide_sib(SIB_PAINT_R22);
		delInventory(_G(cur)->getInventoryCursor());
		_G(obj)->calc_all_static_detail();
		_G(flags).AutoAniPlay = false;

		if (!_G(gameState).R22ChewyPlatt) {
			_G(gameState).R22ChewyPlatt = true;
			_G(atds)->setControlBit(79, ATS_ACTIVE_BIT);
		}
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
