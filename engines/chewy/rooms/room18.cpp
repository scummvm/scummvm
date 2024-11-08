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
#include "chewy/rooms/room18.h"

namespace Chewy {
namespace Rooms {

static const int16 BORK_AWAKE_SPR[5] = { 15, 16, 17, 24, 25 };
static const int16 BORK_FAINTED_SPR[4] = { 20, 21, 18, 19 };

static const int16 SURIMY_PHASES[4][2] = {
	{ 245, 252 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 }
};

static const MovLine SURIMY_MPKT[2] = {
	{ { 453, 170, 190 }, 0, 6 },
	{ { 392, 170, 190 }, 0, 6 }
};

static const MovLine SURIMY_MPKT1[2] = {
	{ { 392, 170, 190 }, 0, 6 },
	{ { 143, 170, 190 }, 0, 6 }
};

static const MovLine SURIMY_MPKT2[2] = {
	{ { 143, 170, 190 }, 0, 6 },
	{ {   0, 170, 190 }, 0, 6 }
};

static const MovLine SURIMY_MPKT3[2] = {
	{ { 500, 100, 190 }, 0, 12 },
	{ { 392, 170, 190 }, 0, 12 }
};

static const AniBlock ABLOCK20[3] = {
	{ 5, 3, ANI_FRONT, ANI_GO, 0 },
	{ 6, 3, ANI_FRONT, ANI_GO, 0 },
	{ 7, 3, ANI_FRONT, ANI_WAIT, 0 }
};


static const AniBlock ABLOCK21[2] = {
	{  8, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 23, 255, ANI_BACK, ANI_GO, 0 },
};

static const AniBlock ABLOCK22[2] = {
	{ 13, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 14, 13, ANI_FRONT, ANI_GO, 0 },
};

static const AniBlock ABLOCK23[2] = {
	{ 11, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 12, 1, ANI_FRONT, ANI_GO, 0 },
};

static const AniBlock ABLOCK24[2] = {
	{ 15, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 16, 7, ANI_FRONT, ANI_GO, 0 },
};


void Room18::entry() {
	_G(gameState).R18MonitorSwitch = false;
	_G(atds)->set_ats_str(151, TXT_MARK_LOOK, 0, ATS_DATA);
	_G(gameState).ScrollxStep = 2;

	if (_G(gameState).R18CartTerminal)
		_G(det)->showStaticSpr(23);

	if (!_G(gameState).R18SurimyThrown) {
		initBorks();
	} else {
		for (int16 i = 0; i < 5; i++)
			_G(det)->hideStaticSpr(BORK_AWAKE_SPR[i]);

		for (int16 i = 0; i < (4 - (_G(gameState).R18Crown ? 1 : 0)); i++)
			_G(det)->showStaticSpr(BORK_FAINTED_SPR[i]);
	}

	if (_G(gameState).R16F5Exit) {
		// Hide cyber crown
		_G(det)->hideStaticSpr(19);
	}

	if (!_G(gameState).R18SurimyThrown) {
		// Hide cyber crown
		_G(det)->hideStaticSpr(19);
		_G(atds)->setControlBit(158, ATS_ACTIVE_BIT);
	} else {
		// Show cyber crown
		_G(det)->showStaticSpr(19);
		_G(atds)->set_all_ats_str(153, 1, ATS_DATA);
		_G(atds)->set_ats_str(149, TXT_MARK_LOOK, 1, ATS_DATA);

		for (int16 i = 0; i < 3; i++)
			_G(atds)->delControlBit(158 + i, ATS_ACTIVE_BIT);
	}

	if (_G(gameState).R17EnergyOut) {
		_G(det)->stopDetail(0);
		_G(atds)->set_ats_str(150, TXT_MARK_LOOK, 1, ATS_DATA);
	} else {
		_G(atds)->set_ats_str(150, TXT_MARK_LOOK, 0, ATS_DATA);
	}

	if (!_G(gameState).R18FirstEntry && !_G(gameState).R18Grid) {
		startAadWait(39);
		_G(gameState).R18FirstEntry = true;
	}

	if (_G(gameState).R18Grid)
		_G(gameState).scrolly = 0;
}

bool Room18::timer(int16 t_nr, int16 ani_nr) {
	if (!_G(gameState).R18SurimyThrown && !_G(flags).AutoAniPlay) {
		_G(flags).AutoAniPlay = true;

		if (t_nr == _G(timer_nr)[0]) {
			_G(det)->hideStaticSpr(16);
			startSetAILWait(10, 1, ANI_FRONT);
			_G(det)->showStaticSpr(16);
			_G(uhr)->resetTimer(_G(timer_nr)[0], 10);
		} else if (t_nr == _G(timer_nr)[1]) {
			hideCur();
			_G(det)->hideStaticSpr(17);
			startSetAILWait(9, 1, ANI_FRONT);
			_G(det)->showStaticSpr(17);

			if (!_G(gameState).R18SondeMoni) {
				_G(gameState).R18SondeMoni = true;
				startSetAILWait(2, 1, ANI_FRONT);
				_G(det)->showStaticSpr(9);
				startSetAILWait(4, 1, ANI_FRONT);
				_G(det)->showStaticSpr(11);
				waitShowScreen(50);
				_G(det)->hideStaticSpr(9);
				_G(det)->hideStaticSpr(11);
			} else {
				monitor();
			}

			showCur();
			_G(uhr)->resetTimer(_G(timer_nr)[1], 15);
		}

		_G(flags).AutoAniPlay = false;
	}

	return false;
}

void Room18::gedAction(int index) {
	if (!index && !_G(gameState).R18SurimyThrown) {
		stopPerson(P_CHEWY);
		autoMove(1, P_CHEWY);
		start_aad(40, 0);
	}
}

void Room18::initBorks() {
	for (int16 i = 0; i < 5; i++)
		_G(det)->showStaticSpr(BORK_AWAKE_SPR[i]);
	
	for (int16 i = 0; i < 4; i++)
		_G(det)->hideStaticSpr(BORK_FAINTED_SPR[i]);

	_G(timer_nr)[0] = _G(room)->set_timer(255, 10);
	_G(timer_nr)[1] = _G(room)->set_timer(255, 15);
	_G(gameState).scrollx = 276;
	_G(gameState).scrolly = 0;
	_G(flags).NoScroll = true;
}

void Room18::monitor() {
	_G(gameState).R18MonitorSwitch ^= 1;

	int16 nr = 0;
	if (_G(gameState).R18MonitorSwitch) {
		startAniBlock(2, ABLOCK21);
		nr = (_G(gameState).R17EnergyOut) ? 2 : 1;
	} else {
		_G(det)->stopDetail(23);
		_G(atds)->set_ats_str(41, TXT_MARK_LOOK, 1, ATS_DATA);
	}

	_G(atds)->set_ats_str(151, TXT_MARK_LOOK, nr, ATS_DATA);
}

int16 Room18::sonden_moni() {
	if (!_G(cur)->usingInventoryCursor() && !_G(gameState).R18Grid) {
		hideCur();
		autoMove(8, P_CHEWY);
		startSetAILWait(3, 1, ANI_FRONT);
		_G(det)->showStaticSpr(10);
		startAniBlock(3, ABLOCK20);

		for (int16 i = 0; i < 3; i++)
			_G(det)->showStaticSpr(i + 12);

		waitShowScreen(40);

		for (int16 i = 0; i < 5; i++)
			_G(det)->hideStaticSpr(i + 10);

		showCur();

		return true;
	} else {
		return false;
	}
}

int16 Room18::calcSurimy() {
	if (isCurInventory(SURIMY_INV)) {
		hideCur();
		delInventory(_G(cur)->getInventoryCursor());
		_G(gameState).R18SurimyThrown = true;
		_G(det)->load_taf_seq(245, 50, nullptr);
		_G(det)->load_taf_seq(116, 55, nullptr);
		_G(auto_obj) = 1;
		_G(mov_phasen)[SURIMY_OBJ].AtsText = 0;
		_G(mov_phasen)[SURIMY_OBJ].Lines = 2;
		_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
		_G(mov_phasen)[SURIMY_OBJ].ZoomFak = 0;
		_G(auto_mov_obj)[SURIMY_OBJ].Id = AUTO_OBJ0;
		_G(auto_mov_vector)[SURIMY_OBJ].Delay = _G(gameState).DelaySpeed;
		_G(auto_mov_obj)[SURIMY_OBJ].Mode = true;

		if (_G(moveState)[P_CHEWY].Xypos[1] < 150) {
			startDetailFrame(18, 1, ANI_FRONT, 8);

			init_auto_obj(SURIMY_OBJ, &SURIMY_PHASES[0][0], _G(mov_phasen)[SURIMY_OBJ].Lines, (const MovLine *)SURIMY_MPKT3);
			waitDetail(18);
		} else {
			autoMove(1, P_CHEWY);
			stopPerson(P_CHEWY);
			_G(gameState)._personHide[P_CHEWY] = true;
			startDetailFrame(17, 1, ANI_FRONT, 12);
			_G(mouseLeftClick) = false;

			init_auto_obj(SURIMY_OBJ, &SURIMY_PHASES[0][0], _G(mov_phasen)[SURIMY_OBJ].Lines, (const MovLine *)SURIMY_MPKT);
			waitDetail(17);
		}

		_G(gameState)._personHide[P_CHEWY] = false;
		wait_auto_obj(SURIMY_OBJ);

		_G(det)->setDetailPos(21, 392, 170);
		_G(det)->setDetailPos(22, 447, 154);
		startDetailFrame(21, 1, ANI_FRONT, 14);
		_G(det)->startDetail(22, 1, ANI_FRONT);
		waitDetail(21);
		_G(det)->setStaticPos(26, 392, 170, false, true);
		_G(det)->showStaticSpr(26);

		_G(det)->hideStaticSpr(24);
		startAniBlock(2, ABLOCK22);
		_G(det)->showStaticSpr(20);

		for (int16 i = 0; i < 3; i++)
			_G(det)->hideStaticSpr(i + 15);

		startAniBlock(2, ABLOCK23);
		_G(det)->showStaticSpr(18);
		_G(det)->showStaticSpr(19);
		_G(det)->hideStaticSpr(26);

		_G(flags).NoScroll = true;
		_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
		init_auto_obj(SURIMY_OBJ, &SURIMY_PHASES[0][0], _G(mov_phasen)[SURIMY_OBJ].Lines, (const MovLine *)SURIMY_MPKT1);
		auto_scroll(70, 0);
		wait_auto_obj(SURIMY_OBJ);

		_G(det)->setDetailPos(21, 143, 170);
		_G(det)->setDetailPos(22, 198, 154);
		startDetailFrame(21, 1, ANI_FRONT, 14);
		_G(det)->startDetail(22, 1, ANI_FRONT);
		waitDetail(21);
		_G(det)->setStaticPos(26, 143, 170, false, true);
		_G(det)->showStaticSpr(26);

		_G(det)->hideStaticSpr(25);
		startAniBlock(2, ABLOCK24);
		_G(det)->showStaticSpr(21);

		_G(det)->hideStaticSpr(26);
		_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
		init_auto_obj(SURIMY_OBJ, &SURIMY_PHASES[0][0], _G(mov_phasen)[SURIMY_OBJ].Lines, (const MovLine *)SURIMY_MPKT2);
		auto_scroll(0, 0);
		wait_auto_obj(SURIMY_OBJ);
		_G(gameState).ScrollxStep = 6;
		_G(flags).NoScroll = false;
		auto_scroll(318, 0);
		_G(gameState).ScrollxStep = 2;
		_G(auto_obj) = 0;

		_G(atds)->set_all_ats_str(153, 1, ATS_DATA);
		_G(atds)->set_ats_str(149, TXT_MARK_LOOK, 1, ATS_DATA);

		for (int16 i = 0; i < 3; i++)
			_G(atds)->delControlBit(158 + i, ATS_ACTIVE_BIT);

		_G(atds)->delControlBit(179, ATS_ACTIVE_BIT);
		showCur();
		_G(det)->del_taf_tbl(245, 50, nullptr);

		return true;
	} else {
		return false;
	}
}

int16 Room18::calcMonitorControls() {
	if (!_G(cur)->usingInventoryCursor() && !_G(gameState).R18Grid) {
		hideCur();
		autoMove(6, P_CHEWY);
		monitor();
		showCur();
		return true;
	} else {
		return false;	
	}
}

short Room18::use_cart_moni() {
	if (!_G(cur)->usingInventoryCursor()) {
		hideCur();
		autoMove(9, P_CHEWY);
		setPersonSpr(P_LEFT, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(gameState).R18CartTerminal ^= 1;

		if (!_G(gameState).R18CartTerminal) {
			_G(atds)->set_ats_str(147, TXT_MARK_LOOK, 0, ATS_DATA);
			_G(det)->hideStaticSpr(23);
			startSetAILWait(20, 1, ANI_BACK);
		} else {
			_G(atds)->set_ats_str(147, TXT_MARK_LOOK, 1, ATS_DATA);
			startSetAILWait(20, 1, ANI_FRONT);
			_G(det)->showStaticSpr(23);

			if (_G(gameState).R18CartridgeInSlot) {
				_G(gameState).R18CartSave = true;
				_G(atds)->set_ats_str(CARTRIDGE_INV, TXT_MARK_LOOK, 1, INV_ATS_DATA);
				startAadWait(120);
			}
		}

		showCur();
		return true;
	} else {
		return false;
	}
}

int16 Room18::go_cyberspace() {
	if (!_G(cur)->usingInventoryCursor() && !_G(gameState).R18Grid) {
		hideCur();
		autoMove(7, P_CHEWY);
		cur_2_inventory();
		showCur();

		switchRoom(24);
		return true;
	} else {
		return false;
	}
}

} // namespace Rooms
} // namespace Chewy
