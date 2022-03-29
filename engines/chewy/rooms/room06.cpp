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
#include "chewy/rooms/room06.h"

namespace Chewy {
namespace Rooms {

static const int16 ROBO_PHASEN[4][2] = {
	{ 86, 86 },
	{ 86, 86 },
	{ 86, 86 },
	{ 86, 86 }
};

static const MovLine ROBO_MPKT[3] = {
	{ { 168,  71, 180 }, 1,  1 },
	{ { 180,  71, 100 }, 1,  2 },
	{ {  60, 210, 110 }, 1, 16 }
};

static const AniBlock ABLOCK6[3] = {
	{  3, 1, ANI_FRONT, ANI_GO, 0 },
	{ 13, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 14, 4, ANI_FRONT, ANI_GO, 0 },
};

static const AniBlock ABLOCK7[2] = {
	{ 1, 2, ANI_FRONT, ANI_WAIT, 0 },
	{ 2, 1, ANI_FRONT, ANI_GO, 0 },
};


void Room6::entry() {
	_G(zoom_horizont) = 80;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 2;

	if (_G(gameState).R6BolaSchild && _G(gameState).R6RaumBetreten < 2) {
		_G(det)->startDetail(7, 255, ANI_FRONT);
		_G(atds)->delControlBit(44, ATS_ACTIVE_BIT);
		if (!_G(flags).LoadGame)
			++_G(gameState).R6RaumBetreten;

		if (_G(gameState).R6RaumBetreten == 2) {
			hideCur();
			_G(det)->stop_detail(7);
			init_robo();
			wait_auto_obj(0);
			_G(gameState).R6BolaOk = true;
			_G(obj)->show_sib(SIB_BOLA_BUTTON_R6);
			_G(obj)->hide_sib(SIB_BOLA_R6);
			_G(atds)->setControlBit(44, ATS_ACTIVE_BIT);
			showCur();
		}
	}
}

void Room6::init_robo() {
#define ROBO_OBJ 0
	_G(auto_obj) = 1;
	_G(mov_phasen)[ROBO_OBJ].AtsText = 44;
	_G(mov_phasen)[ROBO_OBJ].Lines = 3;
	_G(mov_phasen)[ROBO_OBJ].Repeat = 1;
	_G(mov_phasen)[ROBO_OBJ].ZoomFak = 0;
	_G(auto_mov_obj)[ROBO_OBJ].Id = AUTO_OBJ0;
	_G(auto_mov_vector)[ROBO_OBJ].Delay = _G(gameState).DelaySpeed;
	_G(auto_mov_obj)[ROBO_OBJ].Mode = true;
	init_auto_obj(ROBO_OBJ, &ROBO_PHASEN[0][0], 3, (const MovLine *)ROBO_MPKT);
}

void Room6::bola_button() {
	if (!_G(gameState).R6BolaBecher) {
		_G(det)->hideStaticSpr(0);
		startSetAILWait(0, 1, ANI_FRONT);
		if (_G(gameState).R6BolaOk) {
			_G(gameState).R6BolaBecher = true;
			_G(det)->showStaticSpr(0);
			startAniBlock(2, ABLOCK7);
			_G(obj)->calc_rsi_flip_flop(SIB_BOLA_FLECK_R6);
			waitDetail(2);
			_G(obj)->calc_rsi_flip_flop(SIB_BOLA_SCHACHT);
			_G(atds)->delControlBit(42, ATS_ACTIVE_BIT);
			_G(atds)->set_ats_str(41, TXT_MARK_LOOK, 1, ATS_DATA);
			_G(obj)->calc_rsi_flip_flop(SIB_BOLA_BUTTON_R6);
			_G(obj)->hide_sib(SIB_BOLA_BUTTON_R6);
		} else {
			_G(gameState)._personHide[P_CHEWY] = true;
			startAniBlock(3, ABLOCK6);
			while (_G(det)->get_ani_status(3) && !SHOULD_QUIT) {
				if (!_G(det)->get_ani_status(14)) {
					setPersonPos(220, 89, P_CHEWY, P_LEFT);
					_G(gameState)._personHide[P_CHEWY] = false;
				}
				setupScreen(DO_SETUP);
			}
			_G(det)->showStaticSpr(0);
			++_G(gameState).R6BolaJoke;
			int16 diaNr = (_G(gameState).R6BolaJoke < 3) ? 3 : 4;
			start_spz(CH_TALK5, 244, false, 0);
			startAadWait(diaNr);
		}
		_G(obj)->calc_rsi_flip_flop(SIB_BOLA_BUTTON_R6);
	}
}

} // namespace Rooms
} // namespace Chewy
