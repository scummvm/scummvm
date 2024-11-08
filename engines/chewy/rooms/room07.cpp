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
#include "chewy/rooms/room07.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK10[4] = {
	{ 15, 2, ANI_FRONT, ANI_WAIT, 0 },
	{ 16, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 17, 2, ANI_FRONT, ANI_WAIT, 0 },
	{ 18, 1, ANI_FRONT, ANI_WAIT, 0 },
};

static const AniBlock ABLOCK25[3] = {
	{ 1, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 5, 255, ANI_FRONT, ANI_GO, 0 },
	{ 4, 1, ANI_FRONT, ANI_WAIT, 0 },
};


void Room7::entry() {
	_G(gameState).ScrollxStep = 2;
}

void Room7::hook(int16 sibNr) {
	int16 diaNr;

	delInventory(8);
	_G(cur)->setInventoryCursor(-1);
	_G(menu_item) = CUR_WALK;
	cursorChoice(_G(menu_item));

	if (_G(gameState).R7RHaken) {
		_G(gameState).R7RopeOk = true;
		if (sibNr == SIB_LHAKEN_R7) {
			_G(obj)->calc_rsi_flip_flop(SIB_LHAKEN_R7);
			_G(obj)->calc_rsi_flip_flop(SIB_RHAKEN_R7);
		}
		_G(atds)->delControlBit(56, ATS_ACTIVE_BIT);
		_G(atds)->set_ats_str(55, TXT_MARK_LOOK, 1, ATS_DATA);
		diaNr = 9;
	} else {
		_G(gameState).R7RopeLeft = true;
		diaNr = 48;
	}

	_G(obj)->calc_all_static_detail();
	_G(atds)->set_ats_str(54, TXT_MARK_LOOK, 1, ATS_DATA);
	startAadWait(diaNr);
}

void Room7::bell() {
	hideCur();
	
	if ((!_G(gameState).R7BellCount) ||
		(_G(gameState).R7BellCount >= 2 && _G(gameState).R7RopeLeft && !_G(gameState).R7RopeOk)) {
		_G(gameState)._personHide[P_CHEWY] = true;
		start_aad(5, 0);
		startAniBlock(3, ABLOCK25);

		_G(det)->showStaticSpr(7);
		startSetAILWait(12, 1, ANI_FRONT);
		startSetAILWait(11, 1, ANI_FRONT);
		_G(det)->hideStaticSpr(7);
		_G(det)->stopDetail(5);
		setPersonPos(95, 94, P_CHEWY, P_RIGHT);
		_G(gameState)._personHide[P_CHEWY] = false;
	} else if (_G(gameState).R7BellCount == 1) {
		_G(gameState)._personHide[P_CHEWY] = true;
		start_aad(6, 0);
		startAniBlock(3, ABLOCK25);
		_G(det)->showStaticSpr(7);
		startSetAILWait(10, 1, ANI_FRONT);
		_G(det)->startDetail(13, 1, ANI_FRONT);
		setPersonPos(95, 94, P_CHEWY, P_RIGHT);
		_G(det)->stopDetail(5);

		_G(gameState)._personHide[P_CHEWY] = false;
		_G(flags).NoScroll = true;
		autoMove(6, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;

		_G(det)->startDetail(0, 255, ANI_FRONT);
		startSetAILWait(13, 1, ANI_FRONT);
		flic_cut(FCUT_001);
		_G(det)->stopDetail(0);
		_G(gameState).scrollx = 0;
		_G(gameState).scrolly = 0;
		setPersonPos(114, 138, P_CHEWY, -1);

		startDetailFrame(19, 1, ANI_FRONT, 6);
		startDetailFrame(9, 1, ANI_FRONT, 4);
		_G(det)->showStaticSpr(9);
		waitDetail(9);
		_G(det)->hideStaticSpr(9);
		_G(obj)->show_sib(SIB_SCHLOTT_R7);
		_G(obj)->calc_rsi_flip_flop(SIB_SCHLOTT_R7);

		_G(gameState)._personHide[P_CHEWY] = false;
		_G(flags).NoScroll = false;
		_G(det)->hideStaticSpr(7);

	} else if (!_G(gameState).R7RopeOk) {
		_G(gameState)._personHide[P_CHEWY] = true;
		start_aad(7, 0);
		startAniBlock(3, ABLOCK25);
		_G(det)->showStaticSpr(7);
		_G(det)->load_taf_seq(192, 74, nullptr);
		_G(det)->startDetail(14, 1, ANI_FRONT);
		setPersonPos(95, 94, P_CHEWY, P_RIGHT);
		_G(det)->stopDetail(5);
		_G(gameState)._personHide[P_CHEWY] = false;

		_G(flags).NoScroll = true;
		autoMove(6, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(det)->startDetail(0, 255, ANI_FRONT);
		waitDetail(14);
		startAniBlock(4, ABLOCK10);
		_G(det)->hideStaticSpr(7);
		_G(det)->stopDetail(0);
		setPersonPos(181, 130, P_CHEWY, P_RIGHT);
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(flags).NoScroll = false;
		_G(det)->del_taf_tbl(192, 74, nullptr);

	} else if (_G(gameState).R7RopeOk && !_G(gameState).R7BorkFlug) {
		_G(gameState).R7BorkFlug = true;
		_G(gameState)._personHide[P_CHEWY] = true;
		start_aad(8, 0);
		startSetAILWait(1, 1, ANI_FRONT);
		_G(obj)->set_rsi_flip_flop(SIB_TBUTTON2_R7, 255);
		_G(obj)->hide_sib(SIB_KLINGEL_R7);
		_G(flags).NoPalAfterFlc = false;
		register_cutscene(2);
		flic_cut(FCUT_002);
		setPersonPos(201, 117, P_CHEWY, P_LEFT);
		_G(gameState).scrollx = 0;
		_G(gameState).scrolly = 0;
		_G(gameState)._personHide[P_CHEWY] = false;
	}

	++_G(gameState).R7BellCount;

	showCur();
}

void Room7::gedAction(int index) {
	if (index == 0 && _G(gameState).R7BorkFlug && _G(gameState).R7ChewyFlug) {
		_G(gameState)._personHide[P_CHEWY] = true;
		setPersonPos(180, 124, P_CHEWY, P_LEFT);
		startSetAILWait(20, 1, ANI_FRONT);
		_G(det)->showStaticSpr(10);
		waitShowScreen(10 * _G(gameState).DelaySpeed);
		_G(det)->hideStaticSpr(10);
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(gameState).R7ChewyFlug = false;
	}
}

} // namespace Rooms
} // namespace Chewy
