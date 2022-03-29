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
#include "chewy/room.h"
#include "chewy/rooms/room90.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

int Room90::_delay;

void Room90::entry(int16 eib_nr) {
	_G(gameState).ScrollxStep = 2;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_delay = 0;
	_G(SetUpScreenFunc) = setup_func;
	_G(gameState)._personRoomNr[P_HOWARD] = 90;
	_G(zoom_horizont) = 150;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 4;
	_G(gameState).ZoomXy[P_HOWARD][0] = 60;
	_G(gameState).ZoomXy[P_HOWARD][1] = 90;

	if (!_G(flags).LoadGame) {
		switch (eib_nr) {
		case 78:
		case 136:
			_G(gameState).scrollx = 0;
			setPersonPos(32, 137, P_CHEWY, P_RIGHT);
			setPersonPos(20, 121, P_HOWARD, P_RIGHT);
			break;
		case 137:
			_G(gameState).scrollx = 176;
			setPersonPos(411, 146, P_CHEWY, P_RIGHT);
			setPersonPos(440, 132, P_HOWARD, P_RIGHT);
			break;
		default:
			break;
		}
	}

	hideCur();
	_G(HowardMov) = 1;

	if (_G(gameState).flags34_40 && !_G(gameState).flags33_40) {
		_G(det)->setSetailPos(12, 329, 15);
		_G(det)->startDetail(12, 255, false);
		g_engine->_sound->playSound(12, 0);
		g_engine->_sound->playSound(12);
	}

	if (_G(gameState).flags33_40 && !_G(gameState).flags34_10) {
		_G(det)->showStaticSpr(4);
		_G(det)->showStaticSpr(5);
	}

	if (!_G(gameState).flags33_10) {
		_G(flags).ZoomMov = false;
		startAadWait(498);
		goAutoXy(-20, 121, P_HOWARD, ANI_GO);
		autoMove(1, P_CHEWY);
		startDetailFrame(2, 1, ANI_FRONT, 3);
		_G(det)->startDetail(0, 1, false);
		startSetAILWait(1, 1, ANI_FRONT);
		_G(gameState).flags33_10 = true;
		_G(gameState)._personRoomNr[P_HOWARD] = 91;
		switchRoom(91);

	} else if (!_G(gameState).flags33_20) {
		startAadWait(499);
		_G(gameState).flags33_20 = true;

	} else if (_G(gameState).flags34_20) {
		setPersonPos(411, 146, P_CHEWY, P_RIGHT);
		setPersonPos(440, 132, P_HOWARD, P_RIGHT);
		_G(gameState).scrollx = 176;
		_G(gameState).flags34_20 = false;
		startAadWait(520);
	}

	_G(HowardMov) = 0;
	showCur();
}

void Room90::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;
	if (eib_nr == 134)
		_G(gameState)._personRoomNr[P_HOWARD] = 91;
	else if (eib_nr == 135)
		_G(gameState)._personRoomNr[P_HOWARD] = 56;
}

void Room90::setup_func() {
	if (_G(menu_display))
		return;

	if (_delay)
		--_delay;
	else {
		_delay = _G(gameState).DelaySpeed - 1;
		for (int i = 0; i < 8; ++i) {
			if (_G(gameState).r90_Array187030[i][0] == 1) {
				int destY = _G(Adi)[i + 4].y;
				int destX = _G(Adi)[i + 4].x;
				if (i >= 5) {
					destX += _G(gameState).r90_Array187030[i][1];
					if (destX > 540) {
						_G(gameState).r90_Array187030[i][0] = 0;
						_G(det)->stop_detail(i + 4);
					}
				} else {
					destX -= _G(gameState).r90_Array187030[i][1];
					if (destX < -30) {
						_G(gameState).r90_Array187030[i][0] = 0;
						_G(det)->stop_detail(i + 4);
					}
				}
				_G(det)->setSetailPos(i + 4, destX, destY);
			} else {
				++_G(gameState).r90_Array187030[i][2];
				if (_G(gameState).r90_Array187030[i][2] < _G(gameState).r90_Array187030[i][3])
					continue;
				_G(gameState).r90_Array187030[i][2] = 0;
				_G(gameState).r90_Array187030[i][0] = 1;
				int destX;
				if (i >= 6)
					destX = 0;
				else
					destX = 500;

				_G(det)->setSetailPos(i + 4, destX, _G(Adi)[i + 4].y);
				
				if (!_G(gameState).flags34_40)
					_G(det)->startDetail(i + 4, 255, false);
			}
		}
	}

	if (_G(gameState)._personRoomNr[P_HOWARD] != 90 || _G(HowardMov) == 1)
		return;

	calc_person_look();
	int xyPos = _G(spieler_vector)[P_CHEWY].Xypos[0];
	int destX;

	if (xyPos > 400)
		destX = 440;
	else if (xyPos > 240)
		destX = 304;
	else if (xyPos > 95)
		destX = 176;
	else
		destX = 18;

	if (_G(HowardMov) == 2)
		destX = 18;

	goAutoXy(destX, 132, P_HOWARD, ANI_GO);
}

void Room90::proc2() {
	if (_G(gameState).flags33_80 || _G(flags).AutoAniPlay)
		return;

	_G(flags).AutoAniPlay = true;
	int diaNr = -1;

	switch (_G(gameState)._personRoomNr[0]) {
	case 46:
		diaNr = 496;
		break;
	case 56:
		diaNr = 506;
		break;
	case 90:
		diaNr = 509;
		break;
	case 91:
		diaNr = 508;
		break;
	case 94:
		diaNr = 572;
		break;
	case 95:
		diaNr = 573;
		break;
	case 96:
		diaNr = 574;
		break;
	case 97:
		diaNr = 575;
		break;
	default:
		break;
	}

	if (diaNr != -1) {
		setupScreen(DO_SETUP);
		if (_G(gameState).ChewyAni == CHEWY_ANI7) {
			start_spz_wait(66, 1, false, P_CHEWY);
			start_spz(67, 255, false, P_CHEWY);
		} else {
			start_spz_wait(64, 1, false, P_CHEWY);
			start_spz(65, 255, false, P_CHEWY);
		}

		hideCur();
		startAadWait(diaNr);
		showCur();
	}
	_G(flags).AutoAniPlay = false;
}

int Room90::getHubcaps() {
	if (_G(gameState).inv_cur)
		return 0;

	hideCur();
	autoMove(3, P_CHEWY);
	startAadWait(515);
	start_spz_wait(14, 1, false, P_CHEWY);
	start_spz_wait(43, 1, false, P_CHEWY);
	load_chewy_taf(CHEWY_ANI7);

	while (_G(spieler_vector)[P_HOWARD].Xypos[0] != 176) {
		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN0;
	}

	start_spz_wait(47, 1, false, P_HOWARD);
	_G(gameState).mi[1] = 2;
	startAadWait(516);
	_G(gameState).flags34_40 = true;
	_G(gameState).flags35_1 = true;
	autoMove(4, P_CHEWY);
	auto_scroll(176, 0);
	_G(det)->setSetailPos(12, 495, 15);
	_G(det)->startDetail(12, 255, false);
	g_engine->_sound->playSound(12, 0);
	g_engine->_sound->playSound(12);

	int destX = 495;

	while (destX > 329) {
		destX -= 2;
		_G(det)->setSetailPos(12, destX, 15);
		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN0;
	}

	startAadWait(517);
	_G(gameState).flags35_1 = false;
	_G(atds)->setControlBit(517, ATS_ACTIVE_BIT);
	_G(atds)->delControlBit(519, ATS_ACTIVE_BIT);
	showCur();

	return 1;
}	

int Room90::shootControlUnit() {
	if (!isCurInventory(111))
		return 0;

	hideCur();
	delInventory(_G(gameState).AkInvent);
	_G(HowardMov) = 2;
	_G(flags).ZoomMov = false;
	autoMove(5, P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	goAutoXy(232, 142, P_CHEWY, ANI_WAIT);
	_G(flags).NoScroll = true;
	auto_scroll(176, 0);
	startSetAILWait(13, 1, ANI_FRONT);
	_G(flags).NoPalAfterFlc = true;
	flic_cut(FCUT_107);
	_G(gameState).scrollx = 0;
	setPersonPos(76, 145, P_CHEWY, P_LEFT);
	_G(mouseLeftClick) = false;
	_G(det)->showStaticSpr(4);
	_G(det)->showStaticSpr(5);
	setupScreen(NO_SETUP);
	_G(fx_blend) = BLEND3;
	_G(spieler_mi)[P_CHEWY].Mode = false;
	_G(flags).NoScroll = false;
	_G(HowardMov) = 0;
	_G(gameState).flags33_40 = true;
	_G(det)->stop_detail(12);
	_G(atds)->setControlBit(519, ATS_ACTIVE_BIT);
	_G(atds)->delControlBit(520, ATS_ACTIVE_BIT);
	_G(flags).ZoomMov = true;

	showCur();
	return 1;
}

int Room90::useSurimyOnWreck() {
	if (!isCurInventory(18))
		return 0;

	_G(gameState).flags34_10 = true;
	hideCur();
	cur_2_inventory();
	autoMove(6, P_CHEWY);
	_G(mouseLeftClick) = false;
	_G(out)->setPointer(nullptr);
	_G(out)->cls();
	flic_cut(FCUT_109);
	register_cutscene(29);

	_G(det)->hideStaticSpr(4);
	_G(det)->hideStaticSpr(5);
	_G(gameState).scrollx = 176;
	start_spz(45, 255, false, P_CHEWY);
	startAadWait(519);
	_G(atds)->setControlBit(520, ATS_ACTIVE_BIT);

	showCur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
