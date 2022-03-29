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
#include "chewy/rooms/room76.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

int Room76::_state;


void Room76::entry() {
	g_engine->_sound->playSound(0,0);
	g_engine->_sound->playSound(0, 1);
	g_engine->_sound->playSound(0);
	g_engine->_sound->playSound(0, 1, false);
	_G(gameState).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_state = 0;

	setPersonPos(308, 84, P_NICHELLE, P_RIGHT);
	setPersonPos(365, 84, P_HOWARD, P_RIGHT);
	_G(gameState)._personHide[P_HOWARD] = true;
	_G(gameState)._personHide[P_NICHELLE] = true;

	if (!_G(gameState).flags29_4) {
		_G(atds)->delControlBit(453, ATS_ACTIVE_BIT);
		_G(atds)->delControlBit(457, ATS_ACTIVE_BIT);
		_G(atds)->delControlBit(458, ATS_ACTIVE_BIT);

	} else {
		_G(det)->del_static_ani(2);
		for (int i = 0; i < 3; ++i)
			_G(det)->hideStaticSpr(8 + i);

		_G(atds)->setControlBit(453, ATS_ACTIVE_BIT);
		_G(atds)->setControlBit(457, ATS_ACTIVE_BIT);
		_G(atds)->setControlBit(458, ATS_ACTIVE_BIT);
	}

	if (_G(flags).LoadGame)
		return;

	if (!_G(gameState).flags29_2) {
		hideCur();
		_G(flags).NoScroll = true;
		_G(gameState).scrollx = 122;
		_G(gameState).flags29_2 = true;
		setPersonPos(128, 135, P_CHEWY, P_RIGHT);
		proc3(420);
		proc5();

	} else if (!_G(gameState).r76State) {
		hideCur();
		_G(flags).NoScroll = true;
		_G(gameState).scrollx = 122;
		_G(gameState).r76State = -1;
		setPersonPos(128, 135, P_CHEWY, P_RIGHT);
		proc3(422);
		_G(flags).NoScroll = false;
	}
}

void Room76::xit() {
	_G(gameState).ScrollxStep = 1;
	_G(gameState).r76State = -1;
}

void Room76::setup_func() {
	if (_state != 1 || _G(gameState).scrollx < 300)
		return;

	_state = 0;
	_G(det)->startDetail(11, 1, false);
	_G(det)->startDetail(12, 1, false);
}

void Room76::talk1() {
	hideCur();
	autoMove(4, P_CHEWY);
	auto_scroll(172, 0);
	const int diaNr = 425 + (_G(gameState).flags32_4 ? 1 : 0);
	startAadWait(diaNr);
	showCur();
}

void Room76::talk2() {
	hideCur();
	autoMove(3, P_CHEWY);
	int diaNr;
	if (!_G(gameState).flags29_8)
		diaNr = 423;
	else if (!_G(gameState).flags29_40)
		diaNr = 438;
	else
		diaNr = 421;

	proc3(diaNr);
	if (diaNr == 421)
		proc5();

	showCur();
}

void Room76::proc3(int diaNr) {
	startAadWait(diaNr);
	_G(det)->del_static_ani(2);
	startSetAILWait(3, 1, ANI_FRONT);
	startSetAILWait(4, 2, ANI_FRONT);
	_G(det)->set_static_ani(2, -1);
}

void Room76::proc5() {
	_state = 1;
	_G(det)->del_static_ani(2);
	_G(det)->startDetail(6, 1, false);
	_G(flags).NoScroll = false;
	waitShowScreen(15);
	autoMove(5, P_CHEWY);
	_G(det)->hideStaticSpr(10);
	_G(det)->startDetail(13, 1, false);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	goAutoXy(669, 127, P_CHEWY, ANI_WAIT);
	waitShowScreen(20);
	_G(spieler_mi)[P_CHEWY].Mode = false;
	showCur();
	setPersonPos(30, 150, P_CHEWY, P_RIGHT);
	switchRoom(78);
}

int Room76::proc6() {
	int retVal = 0;

	if (isCurInventory(93)) {
		hideCur();
		delInventory(_G(gameState).AkInvent);
		_G(gameState).flags29_8 = true;
		retVal = 1;
		autoMove(3, P_CHEWY);
		proc3(424);
		start_spz_wait(13, 1, false, P_CHEWY);
		_G(det)->del_static_ani(2);
		startSetAILWait(5, 1, ANI_FRONT);
		_G(det)->set_static_ani(2, -1);
		startSetAILWait(9, 1, ANI_FRONT);
		startSetAILWait(10, 1, ANI_FRONT);
		showCur();
	} else if (isCurInventory(94)) {
		hideCur();
		retVal = 1;
		setPersonSpr(P_RIGHT, P_CHEWY);
		startAadWait(436);
		showCur();
	}

	return retVal;
}

int Room76::proc7() {
	if (!isCurInventory(96))
		return 0;
	
	if (_G(gameState).flags29_4) {
		hideCur();
		autoMove(6, P_CHEWY);
		start_spz_wait(13, 1, false, P_CHEWY);
		delInventory(_G(gameState).AkInvent);
		invent_2_slot(106);
		invent_2_slot(105);
		startAadWait(444);
		_G(out)->ausblenden(0);
		_G(out)->set_partialpalette(_G(pal), 255, 1);
		startAadWait(443);
		_G(gameState).scrollx = 0;
		_G(fx_blend) = BLEND3;
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		setPersonPos(128, 135, P_CHEWY, P_RIGHT);
		_G(det)->set_static_ani(2, -1);

		for (int i = 0; i < 3; ++i)
			_G(det)->showStaticSpr(8 + i);

		startAadWait(427);
		showCur();
		startAdsWait(21);
		hideCur();
		startAadWait(428);
		showCur();
		_G(gameState).flags29_80 = true;
		switchRoom(78);
	} else {
		hideCur();
		start_spz(CH_TALK12, 255, false, P_CHEWY);
		startAadWait(577);
		showCur();
	}

	return 1;
}

} // namespace Rooms
} // namespace Chewy
