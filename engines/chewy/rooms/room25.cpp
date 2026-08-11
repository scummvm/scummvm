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
#include "chewy/rooms/room23.h"
#include "chewy/rooms/room25.h"

#include "chewy/cursor.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const int16 SURIMY_PHASEN[4][2] = {
	{ 56, 63 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 }
};

static const MovLine SURIMY_MPKT[2] = {
	{ { 100, 150, 150 }, 0, 6 },
	{ { -20, 150, 150 }, 0, 6 }
};


void Room25::entry() {
	if (!_G(gameState).R25GliderFlamesExtinguished) {
		_G(det)->playSound(0, 0);

		for (int i = 0; i < 9; ++i)
			_G(det)->startDetail(i, 255, ANI_FRONT);
	}

	if (!_G(gameState).R29WaterHose) {
		_G(det)->hideStaticSpr(0);
		_G(det)->hideStaticSpr(1);
		_G(atds)->setControlBit(219, ATS_ACTIVE_BIT);
	} else {
		_G(det)->showStaticSpr(0);
		_G(det)->showStaticSpr(1);
		_G(atds)->delControlBit(219, ATS_ACTIVE_BIT);
	}

	if (!_G(gameState).R25FirstEntry) {
		hideCur();

		if (_G(obj)->checkInventory(TRANSLATOR_INV)) {
			_G(obj)->calc_rsi_flip_flop(SIB_TRANSLATOR_23);
			_G(atds)->set_all_ats_str(113, 0, ATS_DATA);

			remove_inventory(TRANSLATOR_INV);
			_G(cur)->setInventoryCursor(-1);
			_G(menu_item) = CUR_WALK;
			_G(cur)->setInventoryCursor(-1);
			cursorChoice(_G(menu_item));
		}

		_G(gameState).R25FirstEntry = true;
		_G(gameState)._personHide[P_CHEWY] = true;
		flic_cut(FCUT_029);
		_G(det)->playSound(0, 0);
		_G(fx_blend) = BLEND_NONE;
		setPersonPos(219, 141, P_CHEWY, P_RIGHT);
		_G(gameState)._personHide[P_CHEWY] = false;
		start_spz(CH_TALK11, 255, ANI_FRONT, P_CHEWY);
		startAadWait(64);
		showCur();
	} else if (_G(gameState).R25GliderExit && !_G(flags).LoadGame) {
		setPersonPos(127, 122, P_CHEWY, P_LEFT);

		if (!_G(gameState).R25SurimyGo) {
			_G(gameState).R25SurimyGo = 1;
			xit_glider();
		}
	}

	_G(gameState).R25GliderExit = false;
}

int16 Room25::extinguishGliderFlames() {
	int16 action_flag = false;
	hideCur();

	if (!_G(gameState).R25GliderFlamesExtinguished && _G(gameState).R29WaterHose) {
		if (!_G(cur)->usingInventoryCursor()) {
			action_flag = true;
			_G(gameState).R25GliderFlamesExtinguished = true;
			autoMove(2, P_CHEWY);
			flic_cut(FCUT_030);
			_G(obj)->calc_rsi_flip_flop(SIB_SCHLAUCH_R25);
			_G(atds)->set_all_ats_str(219, 1, ATS_DATA);
			_G(atds)->set_all_ats_str(187, 1, ATS_DATA);
			_G(det)->stopSound(0);

			for (int i = 0; i < 9; ++i)
				_G(det)->stopDetail(i);
		}

	} else if (_G(gameState).R25GliderFlamesExtinguished) {
		if (isCurInventory(EMPTY_MILK_BOTTLE_INV)) {
			action_flag = true;
			autoMove(2, P_CHEWY);
			start_spz_wait((_G(gameState).ChewyAni == CHEWY_ROCKER) ? 28 : 14, 1, false, P_CHEWY);

			delInventory(_G(cur)->getInventoryCursor());
			_G(obj)->addInventory(WATER_FILLED_BOTTLE_INV, &_G(room_blk));
			inventory_2_cur(WATER_FILLED_BOTTLE_INV);
			startAadWait(253);
		}
	}

	showCur();
	return action_flag;
}

int16 Room25::useGlider() {
	int16 action_flag = false;

	if (!_G(cur)->usingInventoryCursor() && _G(gameState).R25GliderFlamesExtinguished) {
		action_flag = true;
		hideCur();
		autoMove(3, P_CHEWY);
		showCur();

		_G(gameState).R23GliderExit = 25;
		Room23::cockpit();
	}
	return action_flag;
}

void Room25::xit_glider() {
	if (!_G(gameState).R25SurimyLauf) {
		hideCur();
		_G(gameState).R25SurimyLauf = true;
		_G(det)->load_taf_seq(56, 8, nullptr);
		_G(auto_obj) = 1;
		_G(mov_phasen)[SURIMY_OBJ].AtsText = 0;
		_G(mov_phasen)[SURIMY_OBJ].Lines = 2;
		_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
		_G(mov_phasen)[SURIMY_OBJ].ZoomFak = 0;
		_G(auto_mov_obj)[SURIMY_OBJ].Id = AUTO_OBJ0;
		_G(auto_mov_vector)[SURIMY_OBJ].Delay = _G(gameState).DelaySpeed + 2;
		_G(auto_mov_obj)[SURIMY_OBJ].Mode = true;
		init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], 2, (const MovLine *)SURIMY_MPKT);
		_G(fx_blend) = BLEND1;
		setupScreen(DO_SETUP);

		start_spz(CH_TALK12, 255, ANI_FRONT, P_CHEWY);
		startAadWait(65);
		_G(fx_blend) = BLEND_NONE;
		wait_auto_obj(SURIMY_OBJ);
		_G(auto_obj) = 0;

		showCur();
	}
}

} // namespace Rooms
} // namespace Chewy
