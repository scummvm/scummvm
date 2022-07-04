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
#include "chewy/rooms/room46.h"
#include "chewy/sound.h"
#include "chewy/video/video_player.h"

namespace Chewy {
namespace Rooms {

void Room46::entry(int16 eib_nr) {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 45) {
		_G(gameState)._personRoomNr[P_HOWARD] = 46;
		_G(SetUpScreenFunc) = setup_func;

		if (eib_nr == 79) {
			setPersonPos(140, 90, P_HOWARD, P_LEFT);
			autoMove(3, P_CHEWY);
			setPersonSpr(P_LEFT, P_CHEWY);
		}
	}

	if (_G(gameState)._personRoomNr[P_HOWARD] == 46) {
		_G(spieler_mi)[P_HOWARD].Mode = true;
		_G(SetUpScreenFunc) = setup_func;

		if (_G(gameState).flags33_8) {
			_G(mouseLeftClick) = false;
			hideCur();
			setPersonPos(175, 73, P_HOWARD, P_LEFT);
			setPersonPos(147, 109, P_CHEWY, P_LEFT);
			_G(gameState).flags33_8 = false;
			_G(gameState).SVal1 = 46;
			waitShowScreen(10);

			start_spz_wait(64, 1, false, P_CHEWY);
			start_spz(65, 255, false, P_CHEWY);
			startAadWait(494);
			_G(gameState).SVal2 = 504;
			switchRoom(92);
			start_spz(65, 255, false, P_CHEWY);
			startAadWait(495);
			_G(gameState).room_e_obj[78].Exit = 90;
			showCur();
		}
	}

	if (_G(gameState).R47SchlossOk && !_G(gameState).R46GetLeder)
		bodo();
}

void Room46::xit() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 46) {
		_G(spieler_mi)[P_HOWARD].Mode = false;
	}
}

void Room46::setup_func() {
	calc_person_look();
	int16 y = 64;
	const int16 ch_x = _G(moveState)[P_CHEWY].Xypos[0];

	int16 x;
	if (ch_x > 160) {
		x = 260;
	} else {
		x = 120;
	}

	if (_G(HowardMov) && _G(flags).ExitMov) {
		x = 160;
		y = 200;
	}

	goAutoXy(x, y, P_HOWARD, ANI_GO);
}

void Room46::bodo() {
	hideCur();
	_G(det)->showStaticSpr(0);
	_G(person_end_phase)[P_CHEWY] = P_RIGHT;
	startAadWait(240);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	goAutoXy(42, 76, P_CHEWY, ANI_WAIT);
	startAadWait(250);
	autoMove(1, P_CHEWY);
	setPersonSpr(P_RIGHT, P_CHEWY);
	startAadWait(251);
	autoMove(2, P_CHEWY);

	_G(spieler_mi)[P_CHEWY].Mode = false;
	startSetAILWait(0, 1, ANI_FRONT);
	_G(det)->startDetail(5, 255, ANI_FRONT);
	startAadWait(241);
	_G(det)->stop_detail(5);
	_G(det)->showStaticSpr(5);
	startAadWait(609);
	_G(det)->hideStaticSpr(5);
	startSetAILWait(2, 1, ANI_FRONT);
	startSetAILWait(3, 1, ANI_FRONT);
	_G(det)->startDetail(4, 255, ANI_FRONT);
	startAadWait(242);
	_G(det)->stop_detail(4);

	_G(det)->showStaticSpr(6);
	startAadWait(610);
	_G(det)->hideStaticSpr(6);
	startSetAILWait(6, 1, ANI_FRONT);
	_G(det)->showStaticSpr(3);
	_G(gameState)._personHide[P_CHEWY] = true;
	startSetAILWait(1, 1, ANI_FRONT);
	flic_cut(FCUT_065);
	_G(out)->setPointer(nullptr);
	_G(out)->cls();
	register_cutscene(16);
	
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(det)->hideStaticSpr(0);
	_G(det)->hideStaticSpr(3);
	load_chewy_taf(CHEWY_ROCKER);
	_G(gameState).R28ChewyPump = false;
	
	setPersonPos(213, 118, P_CHEWY, P_RIGHT);
	startAadWait(243);
	invent_2_slot(UHR_INV);
	invent_2_slot(RING_INV);
	_G(gameState).R46GetLeder = true;
	showCur();
}

void Room46::kloppe() {
	//_G(sndPlayer)->setLoopMode(1);
	for (int16 i = 0; i < 4; i++) {
		g_engine->_video->playVideo(FCUT_065);
		
		_G(out)->setPointer(nullptr);
		_G(out)->cls();

		start_aad(244 + i, -1);
		int16 delay = _G(gameState).DelaySpeed * 50;
		_G(atds)->print_aad(0, 0);

		_G(disableScreen) = true;

		if (g_engine->_sound->speechEnabled()) {
			g_engine->_sound->waitForSpeechToFinish();
		} else {
			while (_G(in)->getSwitchCode() == Common::KEYCODE_INVALID && delay) {
				--delay;
				EVENTS_UPDATE;
				SHOULD_QUIT_RETURN;
			}
		}

		_G(disableScreen) = false;
	}

	g_engine->_video->playVideo(FCUT_066);
	// TODO: Reimplement
	//_G(sndPlayer)->fadeOut(0);
	_G(out)->cls();

	//while (_G(sndPlayer)->musicPlaying() && !SHOULD_QUIT) {
	//	EVENTS_UPDATE;
	//}
}

int16 Room46::use_schloss() {
	int16 action_ret = false;

	if (!_G(cur)->usingInventoryCursor()) {
		action_ret = true;

		if (!_G(gameState).R46GetLeder) {
			autoMove(1, P_CHEWY);
			switchRoom(47);
		} else {
			startAadWait(252);
		}
	}

	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
