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
#include "chewy/rooms/room12.h"

namespace Chewy {
namespace Rooms {

#define R12_BORK_OBJ 0

static const int16 R12_BORK_PHASEN[4][2] = {
	{ 74, 79 },
	{ 80, 85 },
	{ 62, 67 },
	{ 68, 73 }
};

static const MovLine R12_BORK_MPKT[5] = {
	{ { 207, 220, 199 }, 2, 6 },
	{ { 207, 145, 199 }, 2, 6 },
	{ {  30, 145, 199 }, 0, 6 },
	{ { 207, 145, 199 }, 1, 6 },
	{ { 207, 220, 199 }, 3, 6 }
};

static const MovLine R12_BORK_MPKT1[2] = {
	{ { 207, 220, 199 }, 0, 6 },
	{ { 170, 145, 199 }, 0, 6 }
};

static const MovLine R12_BORK_MPKT2[3] = {
	{ { 170, 145, 199 }, 1, 8 },
	{ { 180, 145, 120 }, 1, 8 },
	{ { 300,  80, 120 }, 1, 8 }
};

static const AniBlock ABLOCK16[2] = {
	{ 0, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 1, 1, ANI_FRONT, ANI_WAIT, 0 },
};


void Room12::entry() {
	_G(zoom_horizont) = 150;
	_G(timer_nr)[1] = _G(room)->set_timer(254, 20);

	if (!_G(gameState).R12Betreten) {
		_G(gameState).R12Betreten = true;
		hideCur();

		for (int16 i = 7; i < 10; i++)
			_G(det)->showStaticSpr(i);

		_G(flags).NoScroll = true;
		auto_scroll(60, 0);
		flic_cut(FCUT_015);
		_G(flags).NoScroll = false;

		for (int16 i = 7; i < 10; i++)
			_G(det)->hideStaticSpr(i);

		_G(obj)->show_sib(SIB_TALISMAN_R12);
		_G(obj)->calc_rsi_flip_flop(SIB_TALISMAN_R12);
		_G(obj)->calc_all_static_detail();
		autoMove(5, P_CHEWY);
		start_spz(CH_TALK12, 255, false, 0);
		startAadWait(109);
		showCur();

	} else if (_G(gameState).R12Talisman && !_G(gameState).R12BorkInRohr)
		_G(timer_nr)[0] = _G(room)->set_timer(255, 20);
	else if (_G(gameState).R12BorkInRohr && !_G(gameState).R12RaumOk)
		_G(det)->showStaticSpr(12);
}

bool Room12::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0]) {
		if (!is_chewy_busy())
			init_bork();
	} else if (t_nr == _G(timer_nr)[1] && _G(gameState).R12TransOn) {
		_G(gameState).R12TransOn = false;
		startAadWait(30);
	}

	return false;
}

void Room12::init_bork() {
	if (!auto_obj_status(R12_BORK_OBJ) && !_G(gameState).R12BorkTalk) {
		if (!_G(auto_obj))
			_G(det)->load_taf_seq(62, 24, nullptr);

		if (!_G(flags).AutoAniPlay && !_G(flags).ChAutoMov) {
			_G(auto_obj) = 1;
			_G(mov_phasen)[R12_BORK_OBJ].AtsText = 120;
			_G(mov_phasen)[R12_BORK_OBJ].Lines = 5;
			_G(mov_phasen)[R12_BORK_OBJ].Repeat = 1;
			_G(mov_phasen)[R12_BORK_OBJ].ZoomFak = (int16)_G(room)->_roomInfo->_zoomFactor + 20;
			_G(auto_mov_obj)[R12_BORK_OBJ].Id = AUTO_OBJ0;
			_G(auto_mov_vector)[R12_BORK_OBJ].Delay = _G(gameState).DelaySpeed;
			_G(auto_mov_obj)[R12_BORK_OBJ].Mode = true;
			init_auto_obj(R12_BORK_OBJ, &R12_BORK_PHASEN[0][0], 5, (const MovLine *)R12_BORK_MPKT);

			if (!_G(gameState).R12TalismanOk) {
				hideCur();
				_G(auto_mov_vector)[R12_BORK_OBJ]._delayCount = 1000;
				autoMove(5, P_CHEWY);
				_G(auto_mov_vector)[R12_BORK_OBJ]._delayCount = 0;

				if (_G(gameState).R12BorkCount < 3) {
					++_G(gameState).R12BorkCount;
					_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
					waitShowScreen(10);
					start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
					startAadWait(14);
				}

				wait_auto_obj(R12_BORK_OBJ);
				showCur();
			} else {
				bork_ok();
			}
		}

		_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
	}
}

void Room12::talk_bork() {
	if (!_G(gameState).R12TalismanOk) {
		startAadWait(28);
	}
}

void Room12::bork_ok() {
	hideCur();
	_G(flags).MouseLeft = true;
	_G(auto_mov_vector)[R12_BORK_OBJ]._delayCount = 1000;
	autoMove(5, P_CHEWY);
	_G(auto_mov_vector)[R12_BORK_OBJ]._delayCount = 0;
	_G(gameState).R12BorkTalk = true;

	_G(mov_phasen)[R12_BORK_OBJ].Repeat = 1;
	_G(mov_phasen)[R12_BORK_OBJ].Lines = 2;
	init_auto_obj(R12_BORK_OBJ, &R12_BORK_PHASEN[0][0], 2, (const MovLine *)R12_BORK_MPKT1);
	wait_auto_obj(R12_BORK_OBJ);

	_G(gameState).R12BorkInRohr = true;
	_G(det)->setDetailPos(3, 170, 145);
	_G(det)->startDetail(3, 255, ANI_FRONT);
	startAadWait(57);
	_G(det)->stopDetail(3);

	_G(mov_phasen)[R12_BORK_OBJ].Repeat = 1;
	_G(mov_phasen)[R12_BORK_OBJ].Lines = 3;
	init_auto_obj(R12_BORK_OBJ, &R12_BORK_PHASEN[0][0], 3, (const MovLine *)R12_BORK_MPKT2);
	wait_auto_obj(R12_BORK_OBJ);
	_G(det)->hideStaticSpr(10);
	startSetAILWait(4, 1, ANI_FRONT);
	_G(talk_hide_static) = -1;
	_G(det)->showStaticSpr(12);
	_G(atds)->set_ats_str(118, TXT_MARK_LOOK, 2, ATS_DATA);
	_G(obj)->calc_rsi_flip_flop(SIB_ROEHRE_R12);

	_G(flags).MouseLeft = false;
	showCur();
}

int16 Room12::use_terminal() {
	int16 action_flag = false;
	if (!_G(cur)->usingInventoryCursor()) {
		action_flag = true;

		if (!_G(gameState).R12ChewyBork) {
			autoMove(6, P_CHEWY);
			startAadWait(110);

			if (_G(gameState).R12BorkInRohr && !_G(gameState).R12RaumOk) {
				start_spz(CH_TALK5, 255, false, P_CHEWY);
				startAadWait(112);
				_G(flags).NoScroll = true;
				auto_scroll(46, 0);
				flic_cut(FCUT_016);
				register_cutscene(5);

				load_chewy_taf(CHEWY_BORK);
				_G(flags).NoScroll = false;
				_G(atds)->set_all_ats_str(118, 0, ATS_DATA);
				_G(det)->hideStaticSpr(12);
				_G(menu_item) = CUR_WALK;
				cursorChoice(_G(menu_item));
				setPersonPos(108, 90, P_CHEWY, -1);
				_G(gameState).R12ChewyBork = true;
				_G(gameState).R12RaumOk = true;
				autoMove(4, P_CHEWY);
				start_spz(68, 255, false, P_CHEWY);
				startAadWait(113);

			} else if (_G(gameState).R12TalismanOk && !_G(gameState).R12RaumOk) {
				use_linke_rohr();

			} else {
				_G(gameState).R12TransOn = true;
				_G(uhr)->resetTimer(_G(timer_nr)[1], 0);
			}
		} else {
			start_aad(114, 0);
		}
	}

	return action_flag;
}

void Room12::use_linke_rohr() {
	_G(gameState).R12TalismanOk = false;
	_G(gameState).R12ChainLeft = true;
	_G(uhr)->disableTimer();
	_G(obj)->calc_rsi_flip_flop(SIB_L_ROEHRE_R12);
	_G(obj)->calc_rsi_flip_flop(SIB_ROEHRE_R12);
	_G(obj)->calc_all_static_detail();
	_G(atds)->set_ats_str(118, TXT_MARK_LOOK, 0, ATS_DATA);
	_G(atds)->set_ats_str(117, TXT_MARK_LOOK, 1, ATS_DATA);
	start_aad(111, 0);
}

int16 Room12::chewy_trans() {
	int16 action_flag = false;
	if (!_G(cur)->usingInventoryCursor() && _G(gameState).R12TransOn) {
		action_flag = true;
		_G(flags).AutoAniPlay = true;
		autoMove(9, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startAniBlock(2, ABLOCK16);
		setPersonPos(108, 82, P_CHEWY, P_RIGHT);
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(gameState).R12TransOn = false;
		_G(flags).AutoAniPlay = false;
	}
	return action_flag;
}

int16 Room12::useTransformerTube() {
	bool result = false;

	if (!_G(cur)->usingInventoryCursor()) {
		result = true;

		if (_G(gameState).R12ChainLeft) {
			_G(gameState).R12ChainLeft = false;
			_G(uhr)->enableTimer();
			_G(atds)->set_ats_str(117, TXT_MARK_LOOK, 0, ATS_DATA);
		} else {
			autoMove(7, P_CHEWY);
			startAadWait(29);
		}
	}

	return result;
}

} // namespace Rooms
} // namespace Chewy
