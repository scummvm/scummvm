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
#include "chewy/rooms/room68.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK38[2] = {
	{ 5, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 6, 1, ANI_FRONT, ANI_WAIT, 0 },
};

static const AniBlock ABLOCK39[3] = {
	{ 15, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 17, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 15, 1, ANI_BACK, ANI_WAIT, 0 },
};


void Room68::entry() {
	_G(gameState).ScrollxStep = 2;
	_G(gameState).DiaAMov = 1;
	_G(SetUpScreenFunc) = setup_func;
	_G(r68HohesC) = -1;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	
	if (_G(gameState).R68Parrot) {
		_G(det)->showStaticSpr(12);
		_G(det)->startDetail(21, 255, ANI_FRONT);
	}

	if (!_G(gameState).R68DivaAway) {
		_G(timer_nr)[0] = _G(room)->set_timer(255, 10);
		_G(det)->set_static_ani(18, -1);
	} else
		_G(det)->hideStaticSpr(3);
	
	if (!_G(flags).LoadGame) {
		hideCur();
		if (_G(gameState)._personRoomNr[P_HOWARD] == 68) {
			setPersonPos(524, 51, P_HOWARD, P_LEFT);
			setPersonPos(550, 54, P_NICHELLE, P_LEFT);
		}
		autoMove(7, P_CHEWY);
		showCur();
	}
}

void Room68::xit() {
	_G(gameState).ScrollxStep = 1;
	if (_G(gameState)._personRoomNr[P_HOWARD] == 68) {
		_G(gameState)._personRoomNr[P_HOWARD] = 66;
		_G(gameState)._personRoomNr[P_NICHELLE] = 66;
	}
}

bool Room68::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		calcDiva();
	else
		return true;

	return false;
}

void Room68::setup_func() {
	switch (_G(r68HohesC)) {
	case 0:
		if (_G(det)->get_ani_status(_G(r68HohesC)) == false) {
			_G(r68HohesC) = 1;
			_G(det)->startDetail(_G(r68HohesC), 1, ANI_FRONT);
		}
		break;

	case 1:
		if (_G(det)->get_ani_status(_G(r68HohesC)) == false) {
			_G(r68HohesC) = 2;
			if (g_engine->_sound->speechEnabled()) {
				_G(det)->playSound(2, 0);
				_G(det)->playSound(_G(r68HohesC), 0);
				_G(det)->startDetail(_G(r68HohesC), 255, ANI_FRONT);

				// TODO
				//while (_G(sndPlayer)->getSampleStatus(1) == 4)
				//	setupScreen(DO_SETUP);
				_G(det)->stopDetail(_G(r68HohesC));
			} else {
				_G(det)->startDetail(_G(r68HohesC), 3, ANI_FRONT);
			}
			
			
			_G(det)->startDetail(_G(r68HohesC), 3, ANI_FRONT);
		}
		break;

	case 2:
		if (_G(det)->get_ani_status(_G(r68HohesC)) == false) {
			_G(r68HohesC) = -1;
			_G(det)->stopDetail(18);
			_G(det)->showStaticSpr(3);
			_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
		}
		break;

	default:
		break;
	}
	calc_person_look();
	int16 ho_y = 75;
	int16 ni_y = 75;
	int16 ch_x = _G(moveState)[P_CHEWY].Xypos[0];
	int16 ho_x, ni_x;

	if (ch_x < 130) {
		ho_x = 223;
		ni_x = 260;
	} else {
		ho_x = 320;
		ni_x = 350;
	}
	goAutoXy(ho_x, ho_y, P_HOWARD, ANI_GO);
	goAutoXy(ni_x, ni_y, P_NICHELLE, ANI_GO);
}

void Room68::lookAtCactus() {
	hideCur();
	autoMove(6, P_CHEWY);
	startAadWait(383);
	showCur();
}

void Room68::talkToIndigo() {
	talkToIndigo(-1);
}

void Room68::talkToIndigo(int16 aad_nr) {
	hideCur();
	autoMove(3, P_CHEWY);
	_G(room)->set_timer_status(8, TIMER_STOP);
	_G(det)->del_static_ani(8);
	if (aad_nr == -1) {
		_G(det)->set_static_ani(9, -1);
		startAadWait(384 + (int16)_G(gameState).R68IndigoDia);
		_G(gameState).R68IndigoDia ^= 1;
		_G(det)->del_static_ani(9);
	} else {
		_G(det)->set_static_ani(12, -1);
		startAadWait(aad_nr);
		_G(det)->del_static_ani(12);
	}
	_G(room)->set_timer_status(8, TIMER_START);
	_G(det)->set_static_ani(8, -1);
	showCur();
}

int16 Room68::useIndigo() {
	int16 action_flag = false;
	hideCur();
	if (isCurInventory(CLINT_500_INV)) {
		action_flag = true;
		if (_G(gameState).R68Song) {
			hideCur();
			autoMove(3, P_CHEWY);
			auto_scroll(78, 0);
			delInventory(_G(cur)->getInventoryCursor());
			talkToIndigo(394);
			hideCur();
			_G(room)->set_timer_status(8, TIMER_STOP);
			_G(det)->del_static_ani(8);
			_G(det)->stopDetail(8);
			startSetAILWait(13, 3, ANI_FRONT);
			startSetAILWait(25, 1, ANI_FRONT);
			_G(det)->set_static_ani(12, -1);
			talkToIndigo(398);
			hideCur();
			_G(room)->set_timer_status(8, TIMER_STOP);
			_G(det)->del_static_ani(8);
			_G(det)->stopDetail(8);
			startSetAILWait(26, 1, ANI_FRONT);
			_G(room)->set_timer_status(8, TIMER_START);
			_G(det)->set_static_ani(8, -1);
			new_invent_2_cur(KARTE_INV);
			_G(gameState).R68KarteDa = true;
		} else {
			talkToIndigo(397);
		}
	} else if (isCurInventory(CLINT_1500_INV) || isCurInventory(CLINT_3000_INV)) {
		action_flag = true;
		startAadWait(393);
	}
	showCur();
	return action_flag;
}

void Room68::talkToBartender() {
	hideCur();
	autoMove(2, P_CHEWY);
	_G(room)->set_timer_status(20, TIMER_STOP);
	_G(det)->del_static_ani(20);
	startSetAILWait(15, 1, ANI_FRONT);
	_G(det)->set_static_ani(16, -1);
	showCur();
	int16 x = _G(moveState)[P_CHEWY].Xypos[0] - _G(gameState).scrollx + _G(spieler_mi)[P_CHEWY].HotX;
	int16 y = _G(moveState)[P_CHEWY].Xypos[1] - _G(gameState).scrolly;
	_G(atds)->set_split_win(3, x, y);
	startDialogCloseupWait(20);
	hideCur();
	_G(det)->del_static_ani(16);
	startSetAILWait(15, 1, ANI_BACK);
	_G(room)->set_timer_status(20, TIMER_START);
	_G(det)->set_static_ani(20, -1);
	showCur();
}

int16 Room68::useParrot() {
	int16 action_flag = false;
	if (isCurInventory(PARROT_INV)) {
		hideCur();
		action_flag = true;
		_G(gameState).R68Parrot = true;
		delInventory(_G(cur)->getInventoryCursor());
		autoMove(5, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(det)->showStaticSpr(12);
		_G(det)->startDetail(21, 255, ANI_FRONT);
		_G(atds)->delControlBit(408, ATS_ACTIVE_BIT);
		_G(atds)->set_all_ats_str(407, 1, ATS_DATA);
		showCur();
	}
	return action_flag;
}

void Room68::calcDiva() {
	if (!_G(gameState).R68DivaAway) {
		if (!_G(gameState).R68Parrot) {
			if (_G(r68HohesC) == -1) {
				_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
				_G(r68HohesC) = 0;
				_G(det)->hideStaticSpr(3);
				_G(det)->startDetail(_G(r68HohesC), 1, ANI_BACK);
				_G(det)->startDetail(18, 255, ANI_FRONT);
			}
		} else if (!_G(gameState).R68DrinkCoupon && !is_chewy_busy()) {
			hideCur();
			_G(gameState).R68DrinkCoupon = true;
			autoMove(4, P_CHEWY);
			startAadWait(386);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			new_invent_2_cur(DRINK_COUPON_INV);
			_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
			showCur();
		}
	}
}

int16 Room68::useBartender() {
	int16 action_flag = false;
	if (isCurInventory(DRINK_COUPON_INV)) {
		hideCur();
		delInventory(_G(cur)->getInventoryCursor());
		action_flag = true;
		autoMove(2, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(room)->set_timer_status(20, TIMER_STOP);
		_G(det)->del_static_ani(20);
		startAniBlock(3, ABLOCK39);
		_G(room)->set_timer_status(20, TIMER_START);
		_G(det)->set_static_ani(20, -1);
		new_invent_2_cur(B_MARY_INV);
		showCur();
	}
	return action_flag;
}

int16 Room68::useDiva() {
	int16 action_flag;
	hideCur();
	if (isCurInventory(B_MARY_INV)) {
		delInventory(_G(cur)->getInventoryCursor());
		action_flag = 1;
		autoMove(4, P_CHEWY);
		_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
		_G(det)->hideStaticSpr(3);
		startSetAILWait(4, 1, ANI_FRONT);
		_G(gameState).R68DrinkCoupon = false;
		_G(det)->showStaticSpr(3);
	} else if (isCurInventory(B_MARY2_INV)) {
		delInventory(_G(cur)->getInventoryCursor());
		action_flag = 1;
		autoMove(4, P_CHEWY);
		_G(det)->hideStaticSpr(3);
		_G(gameState).R68DivaAway = true;
		startAniBlock(2, ABLOCK38);
		flic_cut(FCUT_083);
		_G(det)->del_static_ani(18);
		register_cutscene(22);
		_G(det)->del_static_ani(18);
		startSetAILWait(7, 1, ANI_FRONT);
		_G(atds)->setControlBit(407, ATS_ACTIVE_BIT);
		_G(atds)->setControlBit(412, ATS_ACTIVE_BIT);
		setPersonSpr(P_RIGHT, P_CHEWY);
		startAadWait(402);
	} else
		action_flag = useParrot();
	showCur();
	return action_flag;
}

void Room68::useDressOnNichelle(int16 aad_nr) {
	hideCur();
	if (_G(gameState).DiaAMov != -1) {
		autoMove(_G(gameState).DiaAMov, P_CHEWY);
	}

	startAadWait(aad_nr);

	if (!_G(gameState).R68DivaAway)
		startAadWait(388);
	else if (!_G(gameState).R67SongOk)
		startAadWait(389);
	else {
		_G(SetUpScreenFunc) = nullptr;
		delInventory(_G(cur)->getInventoryCursor());
		goAutoXy(150, -13, P_NICHELLE, ANI_WAIT);
		_G(gameState)._personHide[P_NICHELLE] = true;
		goAutoXy(161, 59, P_HOWARD, ANI_GO);
		autoMove(4, P_CHEWY);
		startAadWait(390);
		startSetAILWait(22, 1, ANI_FRONT);
		_G(gameState)._personHide[P_HOWARD] = true;
		_G(det)->startDetail(27, 255, ANI_FRONT);

		g_engine->_sound->stopMusic();

		_G(det)->startDetail(23, 255, ANI_FRONT);
		g_engine->_sound->playSound(109, 1);	// piano intro
		waitShowScreen(150);

		g_engine->_sound->waitForSpeechToFinish();

		_G(det)->stopDetail(23);

		g_engine->_sound->playSound(108, 1);	// Nichelle singing

		_G(det)->startDetail(24, 255, ANI_FRONT);
		setPersonPos(26, 40, P_NICHELLE, P_RIGHT);

		waitShowScreen(100);
		startAadWait(602);
		waitShowScreen(100);

		_G(room)->set_timer_status(8, TIMER_STOP);
		_G(det)->del_static_ani(8);
		startSetAILWait(10, 1, ANI_FRONT);
		_G(det)->startDetail(11, 255, ANI_FRONT);
		startAadWait(396);
		_G(det)->stopDetail(11);
		_G(det)->set_static_ani(12, -1);
		startAadWait(611);
		_G(det)->del_static_ani(12);
		_G(det)->startDetail(14, 255, ANI_FRONT);
		startAadWait(395);
		_G(det)->stopDetail(14);
		_G(room)->set_timer_status(8, TIMER_START);
		_G(det)->set_static_ani(8, -1);
		startAadWait(392);

		g_engine->_sound->waitForSpeechToFinish();
		_G(det)->stopSound(1);

		_G(gameState)._personHide[P_HOWARD] = false;
		_G(det)->stopDetail(27);
		_G(det)->stopDetail(24);
		_G(det)->showStaticSpr(13);
		_G(gameState).R68Song = true;
		autoMove(1, P_CHEWY);
		auto_scroll(216, 0);
		_G(det)->hideStaticSpr(13);
		_G(gameState)._personHide[P_NICHELLE] = false;
		setPersonPos(140, 59, P_NICHELLE, P_RIGHT);

		g_engine->_sound->playRoomMusic(_G(gameState)._personRoomNr[P_CHEWY]);
	}
	showCur();
}

void Room68::talkWithParrot() {
	hideCur();
	autoMove(5, P_CHEWY);
	showCur();

	_G(atds)->set_split_win(2, 60, 80);
	startDialogCloseupWait(18);
}

} // namespace Rooms
} // namespace Chewy
