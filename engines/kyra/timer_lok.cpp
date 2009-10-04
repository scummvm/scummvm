/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v1.h"
#include "kyra/kyra_lok.h"
#include "kyra/screen.h"
#include "kyra/animator_lok.h"
#include "kyra/timer.h"

#include "common/system.h"

namespace Kyra {

#define TimerV1(x) new Common::Functor1Mem<int, void, KyraEngine_LoK>(this, &KyraEngine_LoK::x)

void KyraEngine_LoK::setupTimers() {
	for (int i = 0; i <= 4; ++i)
		_timer->addTimer(i, 0, -1, 1);

	_timer->addTimer(5, 0,  5, 1);
	_timer->addTimer(6, 0,  7, 1);
	_timer->addTimer(7, 0,  8, 1);
	_timer->addTimer(8, 0,  9, 1);
	_timer->addTimer(9, 0,  7, 1);

	for (int i = 10; i <= 13; ++i)
		_timer->addTimer(i, 0, 420, 1);

	_timer->addTimer(14, TimerV1(timerCheckAnimFlag2), 600, 1);
	_timer->addTimer(15, TimerV1(timerUpdateHeadAnims), 11, 1);
	_timer->addTimer(16, TimerV1(timerSetFlags1), 7200, 1);
	_timer->addTimer(17, 0 /*sub_15120*/, 7200, 1);
	_timer->addTimer(18, TimerV1(timerCheckAnimFlag1), 600, 1);
	_timer->addTimer(19, TimerV1(timerRedrawAmulet), 600, 1);

	_timer->addTimer(20, 0, 7200, 1);
	_timer->addTimer(21, 0/*sub_1517C*/, 18000, 1);
	_timer->addTimer(22, 0, 7200, 1);

	for (int i = 23; i <= 27; ++i)
		_timer->addTimer(i, 0, 10800, 1);

	_timer->addTimer(28, 0, 21600, 1);
	_timer->addTimer(29, 0, 7200, 1);
	_timer->addTimer(30, 0, 10800, 1);

	_timer->addTimer(31, TimerV1(timerFadeText), -1, 1);
	_timer->addTimer(32, TimerV1(updateAnimFlag1), 9, 1);
	_timer->addTimer(33, TimerV1(updateAnimFlag2), 3, 1);
}

void KyraEngine_LoK::timerUpdateHeadAnims(int timerNum) {
	static int8 currentFrame = 0;
	static const int8 frameTable[] = {4, 5, 4, 5, 4, 5, 0, 1, 4, 5,
								4, 4, 6, 4, 8, 1, 9, 4, -1};

	if (_talkingCharNum < 0)
		return;

	_currHeadShape = frameTable[currentFrame];
	currentFrame++;

	if (frameTable[currentFrame] == -1)
		currentFrame = 0;

	_animator->animRefreshNPC(0);
	_animator->animRefreshNPC(_talkingCharNum);
}

void KyraEngine_LoK::timerSetFlags1(int timerNum) {
	if (_currentCharacter->sceneId == 0x1C)
		return;

	int rndNr = _rnd.getRandomNumberRng(0, 3);

	for (int i = 0; i < 4; i++) {
		if (!queryGameFlag(rndNr + 17)) {
			setGameFlag(rndNr + 17);
			break;
		} else {
			rndNr++;
			if (rndNr > 3)
				rndNr = 0;
		}
	}
}

void KyraEngine_LoK::timerFadeText(int timerNum) {
	_fadeText = true;
}

void KyraEngine_LoK::updateAnimFlag1(int timerNum) {
	if (_brandonStatusBit & 2) {
		_brandonStatusBit0x02Flag = 1;
	}
}

void KyraEngine_LoK::updateAnimFlag2(int timerNum) {
	if (_brandonStatusBit & 0x20) {
		_brandonStatusBit0x20Flag = 1;
	}
}

void KyraEngine_LoK::setTextFadeTimerCountdown(int16 countdown) {
	if (countdown == -1)
		countdown = 32000;

	_timer->setCountdown(31, countdown*60);
}

void KyraEngine_LoK::timerSetFlags2(int timerNum) {
	if (!((uint32*)(_flagsTable+0x2D))[timerNum])
		((uint32*)(_flagsTable+0x2D))[timerNum] = 1;
}

void KyraEngine_LoK::timerCheckAnimFlag1(int timerNum) {
	if (_brandonStatusBit & 0x20) {
		checkAmuletAnimFlags();
		_timer->setCountdown(18, -1);
	}
}

void KyraEngine_LoK::timerCheckAnimFlag2(int timerNum) {
	if (_brandonStatusBit & 0x2) {
		checkAmuletAnimFlags();
		_timer->setCountdown(14, -1);
	}
}

void KyraEngine_LoK::timerRedrawAmulet(int timerNum) {
	if (queryGameFlag(0xF1)) {
		drawAmulet();
		_timer->setCountdown(19, -1);
	}
}

void KyraEngine_LoK::setWalkspeed(uint8 newSpeed) {
	static const uint8 speeds[] = { 11, 9, 6, 5, 3 };

	assert(newSpeed < ARRAYSIZE(speeds));
	_timer->setDelay(5, speeds[newSpeed]);
}

} // End of namespace Kyra

