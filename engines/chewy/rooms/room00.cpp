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
#include "chewy/main.h"
#include "chewy/room.h"
#include "chewy/rooms/room00.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

#define FLAP_SPRITE 5
#define CH_FLASH 8
#define FLAP_DETAIL 10
#define HOSE_DETAIL 11
#define CH_THROWS_SLIME 12
#define CH_TO_FEEDER 13
#define CH_THROWS_PILLOW 14
#define FEEDER_HOSE 15
#define STERNE_STEHEN 16
#define HOSE1 11
#define HOSE2 38
#define HOSE3 59
#define HEAD1 39
#define HEAD2 46
#define HEAD3 48

void Room0::entry() {
	if (isCurInventory(0) || _G(gameState).R0PillowThrow || _G(obj)->checkInventory(0))
		_G(det)->hideStaticSpr(6);

	if (!_G(flags).LoadGame) {
		setPersonPos(150, 100, P_CHEWY, P_RIGHT);
		hideCur();
		_G(timer_nr)[0] = _G(room)->set_timer(255, 3);

		while (!_G(ani_timer)[_G(timer_nr)[0]]._timeFlag && !SHOULD_QUIT) {
			setupScreen(DO_SETUP);
		}

		start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
		startAadWait(2);

		showCur();
	}
}

bool Room0::timer(int16 timerNr, int16 aniNr) {
	bool retval = false;
	if (aniNr == 1) {
		if (_G(timer_action_ctr) > 0) {
			_G(uhr)->resetTimer(timerNr, 0);
			--_G(timer_action_ctr);
		} else if (!is_chewy_busy() && _G(flags).AutoAniPlay == 0) {
			if (!_G(gameState).R0FueterLab)
				_G(timer_action_ctr) = 2;

			_G(flags).AutoAniPlay = true;
			if (!_G(gameState).R0SlimeUsed) {
				startAadWait(42);
				autoMove(5, P_CHEWY);
				setPersonSpr(P_LEFT, P_CHEWY);

				if (_G(gameState).R0FueterLab < 3) {
					start_spz(CH_TALK3, 255, false, P_CHEWY);
					if (_G(gameState).R0FueterLab)
						startAadWait(618);
					else
						startAadWait(43);

					++_G(gameState).R0FueterLab;
				}

				eyeAnim();
			} else if (!_G(gameState).R0PillowThrow) {
				startAadWait(42);
				start_spz(CH_TALK3, 255, false, P_CHEWY);

				if (_G(gameState).R0FueterLab < 3) {
					startAadWait(43);
					++_G(gameState).R0FueterLab;
				}

				autoMove(3, P_CHEWY);
				setPersonPos(191, 120, P_CHEWY, P_LEFT);
			}

			if (!_G(gameState).R0PillowThrow)
				feederAni();

			_G(uhr)->resetTimer(timerNr, 0);
			_G(flags).AutoAniPlay = false;
		}
	} else if (timerNr != 3) {
		retval = true;
	}

	return retval;
}

bool Room0::getPillow() {
	bool retval = false;
	
	if (!_G(cur)->usingInventoryCursor()) {
		hideCur();
		_G(flags).AutoAniPlay = true;
		autoMove(1, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		invent_2_slot(0);
		_G(menu_item) = CUR_WALK;
		cursorChoice(CUR_WALK);
		_G(atds)->setControlBit(174, ATS_ACTIVE_BIT);
		_G(det)->hideStaticSpr(6);

		_G(flags).AutoAniPlay = false;
		showCur();
		retval = true;
	}

	return retval;
}

bool Room0::pullSlime() {
	bool retval = false;
	if (!_G(cur)->usingInventoryCursor()) {
		hideCur();
		
		_G(flags).AutoAniPlay = true;
		autoMove(2, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(3, 1, ANI_FRONT);
		startSetAILWait(17, 2, ANI_FRONT);
		setPersonPos(222, 106, P_CHEWY, P_LEFT);
		_G(gameState)._personHide[P_CHEWY] = false;
		invent_2_slot(1);
		_G(menu_item) = CUR_WALK;
		cursorChoice(CUR_WALK);
		_G(atds)->setControlBit(175, ATS_ACTIVE_BIT);

		_G(flags).AutoAniPlay = false;
		showCur();
		retval = true;
	}

	return retval;
}

void Room0::eyeAnim() {
	if (!_G(gameState).R0SlimeUsed) {
		// Start the eye animation
		eyeStart(EYE_START);
		if (!_G(gameState).R0SlimeUsed)
			eyeWait();

		if (_G(gameState).R0SlimeUsed) {
			start_aad(124);
			checkSlimeEye();
			eyeSlimeBack();
			autoMove(FUETTER_POS, P_CHEWY);
			setPersonPos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
		} else {
			eyeShoot();
			setPersonPos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
			eyeStart(EYE_END);
		}
	}
}

void Room0::eyeStart(EyeMode mode) {
	AniDetailInfo *adi = _G(det)->getAniDetail(HOSE_DETAIL);
	if (mode == EYE_START)
		adi->ani_count = adi->start_ani;
	else
		adi->ani_count = 38;

	if (mode == EYE_START) {
		trapDoorOpen();
	}

	bool ende = false;
	_G(flags).AniUserAction = true;

	if (mode == EYE_START) {
		_G(det)->playSound(FLAP_DETAIL, 0);
		_G(det)->stopSound(1);
		_G(det)->playSound(HOSE_DETAIL, 0);
		_G(det)->stopSound(2);
	} else {
		_G(det)->stopSound(0);
		_G(det)->playSound(FLAP_DETAIL, 1);
		_G(det)->stopSound(0);
		_G(det)->playSound(HOSE_DETAIL, 2);
	}

	while (!ende) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;
		if ((adi->ani_count > 11) && (adi->ani_count < 19)) {
			_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, HOSE1, ANI_HIDE);
			_G(spr_info)[1]._zLevel = 191;
		}
		
		if (adi->ani_count == 38) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, HEAD1, ANI_HIDE);
			_G(spr_info)[2]._zLevel = 192;
		}

		_G(spr_info)[3] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, adi->ani_count, ANI_HIDE);
		_G(spr_info)[3]._zLevel = 193;
		get_user_key(NO_SETUP);
		setupScreen(NO_SETUP);
		SHOULD_QUIT_RETURN;

		_G(cur)->updateCursor();
		calcEyeClick(3);
		_G(out)->copyToScreen();

		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(gameState).DelaySpeed;
			if (mode == EYE_START) {
				++adi->ani_count;
				if (adi->ani_count > 38)
					ende = true;
			} else {
				--adi->ani_count;
				if (adi->ani_count == adi->start_ani - 1)
					ende = true;
			}
		}
	}

	clear_prog_ani();
	_G(flags).AniUserAction = false;

	if (mode == EYE_END) {
		trapDoorClose();
	}
}

void Room0::eyeWait() {
	AniDetailInfo *adi = _G(det)->getAniDetail(HOSE_DETAIL);
	adi->ani_count = 39;
	adi->delay_count = 15;
	_G(flags).AniUserAction = true;

	while (adi->ani_count < 46) {
		clear_prog_ani();

		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, HOSE2, ANI_HIDE);
		_G(spr_info)[1]._zLevel = 191;
		_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, adi->ani_count, ANI_HIDE);
		_G(spr_info)[2]._zLevel = 192;
		get_user_key(NO_SETUP);
		setupScreen(NO_SETUP);
		_G(cur)->updateCursor();
		calcEyeClick(2);
		_G(out)->copyToScreen();

		if (adi->delay_count > 0) {
			--adi->delay_count;
		} else {
			adi->delay_count = adi->delay + _G(gameState).DelaySpeed;
			++adi->ani_count;
		}

		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;
	}

	_G(flags).AniUserAction = false;
	clear_prog_ani();
}

void Room0::calcEyeClick(int16 aniNr) {
	if (mouse_on_prog_ani() == aniNr) {
		if (_G(minfo).button != 1 && g_events->_kbInfo._keyCode != Common::KEYCODE_RETURN) {
			const uint8 roomNum = _G(room)->_roomInfo->_roomNr;
			Common::StringArray desc = _G(atds)->getTextArray(roomNum, 172, ATS_DATA);
			if (desc.size() > 0) {
				_G(fontMgr)->setFont(_G(font8));
				int16 x = g_events->_mousePos.x;
				int16 y = g_events->_mousePos.y;
				calcTxtXy(&x, &y, desc);
				for (int16 i = 0; i < (int16)desc.size(); i++)
					printShadowed(x, y + i * 10, 255, 300, 0, _G(scr_width), desc[i].c_str());
			}
		} else if (_G(minfo).button == 1 || g_events->_kbInfo._keyCode == Common::KEYCODE_RETURN) {
			if (isCurInventory(SLIME_INV)) {
				delInventory(_G(cur)->getInventoryCursor());
				_G(gameState).R0SlimeUsed = true;
			} else if (isCurInventory(PILLOW_INV)) {
				startAtsWait(172, TXT_MARK_WALK, 14, ATS_DATA);
			}
		}
	}
}

void Room0::eyeShoot() {
	AniDetailInfo *adi = _G(det)->getAniDetail(HOSE_DETAIL);
	adi->ani_count = 47;

	bool endLoopFl = false;
	_G(det)->startDetail(CH_FLASH, 1, ANI_FRONT);

	while (!endLoopFl) {
		clear_prog_ani();
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, HOSE2, ANI_HIDE);
		_G(spr_info)[1]._zLevel = 191;

		if (adi->ani_count < 53) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, adi->ani_count, ANI_HIDE);
			_G(spr_info)[2]._zLevel = 192;
		} else {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, 47, ANI_HIDE);
			_G(spr_info)[2]._zLevel = 192;
			if (!_G(det)->get_ani_status(CH_FLASH))
				endLoopFl = true;
		}

		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN;

		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(gameState).DelaySpeed;
			++adi->ani_count;
		}
	}

	_G(det)->startDetail(STERNE_STEHEN, 255, ANI_FRONT);
	clear_prog_ani();
	_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
	_G(spr_info)[0]._zLevel = 190;
	_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, HOSE2, ANI_HIDE);
	_G(spr_info)[1]._zLevel = 191;
	_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, HEAD2, ANI_HIDE);
	_G(spr_info)[2]._zLevel = 192;

	waitShowScreen(30);
	clear_prog_ani();
	setPersonPos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
	_G(gameState)._personHide[P_CHEWY] = false;
}

void Room0::eyeSlimeBack() {
	AniDetailInfo *adi = _G(det)->getAniDetail(HOSE_DETAIL);
	adi->ani_count = 53;

	bool endLoopFl = false;
	_G(flags).AniUserAction = true;

	while (!endLoopFl) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;

		if ((adi->ani_count > 52) && (adi->ani_count < 59)) {
			_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, HOSE2, ANI_HIDE);
			_G(spr_info)[1]._zLevel = 191;
		}
		if (adi->ani_count == 61) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, HOSE3, ANI_HIDE);
			_G(spr_info)[2]._zLevel = 192;
		}

		_G(spr_info)[3] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, adi->ani_count, ANI_HIDE);
		_G(spr_info)[3]._zLevel = 193;
		set_ani_screen();
		SHOULD_QUIT_RETURN;

		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(gameState).DelaySpeed;
			++adi->ani_count;
			if (adi->ani_count == 77)
				endLoopFl = true;
		}
	}

	_G(flags).AniUserAction = false;
	clear_prog_ani();
}

void Room0::checkSlimeEye() {
	AniDetailInfo *adi = _G(det)->getAniDetail(CH_THROWS_SLIME);
	adi->ani_count = adi->start_ani;
	if (adi->load_flag) {
		_G(det)->load_taf_seq(adi->start_ani, (adi->end_ani - adi->start_ani) + 1, nullptr);
	}

	while (adi->ani_count < adi->end_ani && !SHOULD_QUIT) {
		clear_prog_ani();
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, HOSE2, ANI_HIDE);
		_G(spr_info)[1]._zLevel = 191;
		_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, HOSE_DETAIL, HEAD2, ANI_HIDE);
		_G(spr_info)[2]._zLevel = 192;
		_G(spr_info)[3] = _G(det)->plot_detail_sprite(0, 0, CH_THROWS_SLIME, adi->ani_count, ANI_HIDE);
		_G(spr_info)[3]._zLevel = 193;
		setupScreen(DO_SETUP);

		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(gameState).DelaySpeed;
			++adi->ani_count;
		}
	}

	if (adi->load_flag) {
		_G(det)->del_taf_tbl(adi->start_ani, (adi->end_ani - adi->start_ani) + 1, nullptr);
	}

	clear_prog_ani();
	_G(gameState)._personHide[P_CHEWY] = false;
}

void Room0::feederStart(int16 mode) {
	AniDetailInfo *adi = _G(det)->getAniDetail(FEEDER_HOSE);
	if (!mode)
		adi->ani_count = adi->start_ani;
	else
		adi->ani_count = 135;

	if (!mode) {
		trapDoorOpen();
		_G(det)->playSound(FLAP_DETAIL, 0);
		_G(det)->stopSound(1);
		_G(det)->playSound(FEEDER_HOSE, 0);
		_G(det)->stopSound(2);
	} else {
		_G(det)->stopSound(0);
		_G(det)->playSound(FLAP_DETAIL, 1);
		_G(det)->stopSound(0);
		_G(det)->playSound(FEEDER_HOSE, 2);
	}

	bool endLoopFl = false;
	if (_G(gameState).R0SlimeUsed)
		_G(flags).AniUserAction = true;

	while (!endLoopFl) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, adi->ani_count, ANI_HIDE);
		_G(spr_info)[1]._zLevel = 191;

		if (_G(flags).AniUserAction)
			get_user_key(NO_SETUP);

		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;

		setupScreen(NO_SETUP);
		_G(cur)->updateCursor();
		if (!mode)
			calcPillowClick(1);

		_G(out)->copyToScreen();
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(gameState).DelaySpeed;
			if (!mode) {
				++adi->ani_count;
				if (adi->ani_count > 135)
					endLoopFl = true;
			} else {
				--adi->ani_count;
				if (adi->ani_count == adi->start_ani - 1)
					endLoopFl = true;
			}
		}
	}

	clear_prog_ani();
	_G(flags).AniUserAction = false;

	if (mode) {
		_G(det)->startDetail(FLAP_DETAIL, 1, ANI_BACK);
		while (_G(det)->get_ani_status(FLAP_DETAIL))
			set_ani_screen();

	}
}

void Room0::feederExtend() {
	for (int16 i = 0; i < 30 && !_G(gameState).R0PillowThrow; i++) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, 136, ANI_HIDE);
		_G(spr_info)[1]._zLevel = 191;
		get_user_key(NO_SETUP);
		setupScreen(NO_SETUP);
		_G(cur)->updateCursor();
		calcPillowClick(1);
		_G(out)->copyToScreen();
	}

	clear_prog_ani();
}

void Room0::calcPillowClick(int16 aniNr) {
	if (mouse_on_prog_ani() == aniNr) {
		if (_G(minfo).button != 1 && g_events->_kbInfo._keyCode != Common::KEYCODE_RETURN) {
			const uint8 roomNum = _G(room)->_roomInfo->_roomNr;
			Common::StringArray desc = _G(atds)->getTextArray(roomNum, 173, ATS_DATA);
			if (desc.size() > 0) {
				_G(fontMgr)->setFont(_G(font8));
				int16 x = g_events->_mousePos.x;
				int16 y = g_events->_mousePos.y;
				calcTxtXy(&x, &y, desc);
				for (int16 i = 0; i < (int16)desc.size(); i++)
					printShadowed(x, y + i * 10, 255, 300, 0, _G(scr_width), desc[i].c_str());
			}
		} else if (_G(minfo).button == 1 || g_events->_kbInfo._keyCode == Common::KEYCODE_RETURN) {
			if (isCurInventory(PILLOW_INV) && _G(gameState).R0SlimeUsed) {
				delInventory(_G(cur)->getInventoryCursor());
				_G(gameState).R0PillowThrow = true;
			} else if (isCurInventory(SLIME_INV)) {
				startAtsWait(173, TXT_MARK_WALK, 14, ATS_DATA);
			}
		}
	}
}

void Room0::checkFeed() {
	AniDetailInfo *adi = _G(det)->getAniDetail(FEEDER_HOSE);
	adi->ani_count = 136;

	int16 i = 152;
	bool endLoopFl = false;

	if (_G(gameState).R0SlimeUsed)
		_G(flags).AniUserAction = true;

	while (!endLoopFl) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;

		if (adi->ani_count == 136) {
			_G(gameState)._personHide[P_CHEWY] = true;
			if (!_G(gameState).R0SlimeUsed)
				_G(det)->stop_detail(16);
		}

		if (adi->ani_count > 138) {
			_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, 138, ANI_HIDE);
			_G(spr_info)[1]._zLevel = 191;
		}
		
		if (adi->ani_count > 141) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, i, ANI_HIDE);
			_G(spr_info)[2]._zLevel = 192;
		}

		if (adi->ani_count == 138) {
			_G(spr_info)[3] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, 139, ANI_HIDE);
			_G(spr_info)[3]._zLevel = 193;
		}

		_G(spr_info)[4] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, adi->ani_count, ANI_HIDE);
		_G(spr_info)[4]._zLevel = 194;
		set_ani_screen();
		SHOULD_QUIT_RETURN;

		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(gameState).DelaySpeed;
			if (adi->ani_count > 141)
				++i;
			++adi->ani_count;
			if (adi->ani_count == 152)
				endLoopFl = true;
		}
	}

	adi->ani_count = 138;
	_G(det)->startDetail(CH_TO_FEEDER, 2, ANI_FRONT);

	endLoopFl = false;
	while (!endLoopFl) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;

		if (adi->ani_count > 138) {
			_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, 138, ANI_HIDE);
			_G(spr_info)[1]._zLevel = 191;
		}

		if (adi->ani_count == 138) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, 139, ANI_HIDE);
			_G(spr_info)[2]._zLevel = 192;
		}

		_G(spr_info)[3] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, adi->ani_count, ANI_HIDE);
		_G(spr_info)[3]._zLevel = 193;
		set_ani_screen();
		SHOULD_QUIT_RETURN;

		if (!_G(det)->get_ani_status(CH_TO_FEEDER))
			endLoopFl = true;

		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(gameState).DelaySpeed;
			if (adi->ani_count > 135)
				--adi->ani_count;
		}
	}

	_G(gameState)._personHide[P_CHEWY] = false;
	_G(flags).AniUserAction = false;
	clear_prog_ani();
}

void Room0::checkPillow() {
	AniDetailInfo *adi = _G(det)->getAniDetail(FEEDER_HOSE);
	adi->ani_count = 161;

	bool endLoopFl = false;
	_G(gameState)._personHide[P_CHEWY] = true;
	_G(det)->startDetail(CH_THROWS_PILLOW, 1, ANI_FRONT);
	bool mode = false;

	while (!endLoopFl) {
		clear_prog_ani();
		if (!_G(det)->get_ani_status(CH_THROWS_PILLOW)) {
			mode = true;
			_G(gameState)._personHide[P_CHEWY] = false;
			setPersonPos(228 - CH_HOT_MOV_X, 143 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
		}

		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, 138, ANI_HIDE);
		_G(spr_info)[1]._zLevel = 191;

		if (mode) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, FEEDER_HOSE, adi->ani_count, ANI_HIDE);
			_G(spr_info)[2]._zLevel = 192;
		}

		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN;

		if (mode) {
			if (adi->delay_count > 0)
				--adi->delay_count;
			else {
				adi->delay_count = adi->delay + _G(gameState).DelaySpeed;
				--adi->ani_count;
				if (adi->ani_count == 151)
					endLoopFl = true;
			}
		}
	}

	clear_prog_ani();
}

void Room0::trapDoorOpen() {
	_G(det)->startDetail(FLAP_DETAIL, 1, ANI_FRONT);
	while (_G(det)->get_ani_status(FLAP_DETAIL)) {
		set_ani_screen();
		SHOULD_QUIT_RETURN;
	}

	_G(flags).AniUserAction = true;
	for (int16 i = 0; i < 25; i++) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0]._zLevel = 190;

		set_ani_screen();
		SHOULD_QUIT_RETURN;
	}

	_G(flags).AniUserAction = false;
	clear_prog_ani();
}

void Room0::trapDoorClose() {
	_G(det)->startDetail(FLAP_DETAIL, 1, ANI_BACK);

	while (_G(det)->get_ani_status(FLAP_DETAIL)) {
		set_ani_screen();
		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;
	}
}

void Room0::feederAni() {
	int16 action = false;
	feederStart(0);

	if (_G(gameState).R0SlimeUsed) {
		feederExtend();
		if (_G(gameState).R0PillowThrow) {
			checkPillow();
			feederStart(1);
			autoMove(VERSTECK_POS, P_CHEWY);
			setupScreen(DO_SETUP);
			_G(out)->cls();
			flic_cut(FCUT_000);

			register_cutscene(1);

			_G(gameState)._personRoomNr[P_CHEWY] = 1;
			_G(room)->loadRoom(&_G(room_blk), _G(gameState)._personRoomNr[P_CHEWY], &_G(gameState));
			setPersonPos(_G(Rdi)->autoMove[4]._x - CH_HOT_MOV_X,
			               _G(Rdi)->autoMove[4]._y - CH_HOT_MOV_Y, P_CHEWY, P_RIGHT);
			_G(moveState)[P_CHEWY]._delayCount = 0;

			setShadowPalette(4, false);
			_G(fx_blend) = BLEND1;
			setupScreen(DO_SETUP);
		} else {
			action = true;
		}
	} else {
		action = true;
	}

	if (action) {
		checkFeed();
		start_spz(CH_EKEL, 3, ANI_FRONT, P_CHEWY);
		start_aad(55);
		feederStart(1);
	}
}

} // namespace Rooms
} // namespace Chewy
