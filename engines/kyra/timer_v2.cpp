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
 
#include "kyra/kyra_v2.h"
#include "kyra/timer.h"

namespace Kyra {

#define TimerV2(x) new Functor1Mem<int, void, KyraEngine_v2>(this, &KyraEngine_v2::x)

void KyraEngine_v2::setupTimers() {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v2::setupTimers()");
	
	_timer->addTimer(0, 0, 5, 1);
	_timer->addTimer(1, TimerV2(timerFunc2), -1, 1);
	_timer->addTimer(2, TimerV2(timerFunc3), 1, 1);
	_timer->addTimer(3, TimerV2(timerFunc4), 1, 0);
	_timer->addTimer(4, TimerV2(timerFunc5), 1, 0);
	_timer->addTimer(5, TimerV2(timerFunc6), 1, 0);
}

void KyraEngine_v2::timerFunc2(int arg) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v2::timerFunc2(%d)", arg);
	if (_shownMessage)
		_msgUnk1 = 1;
}

void KyraEngine_v2::timerFunc3(int arg) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v2::timerFunc3(%d)", arg);
	// XXX
}

void KyraEngine_v2::timerFunc4(int arg) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v2::timerFunc4(%d)", arg);
	_timer->disable(3);
	setGameFlag(0xD8);
}

void KyraEngine_v2::timerFunc5(int arg) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v2::timerFunc5(%d)", arg);
	// XXX
}

void KyraEngine_v2::timerFunc6(int arg) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v2::timerFunc6(%d)", arg);
	// XXX
}

void KyraEngine_v2::setTimer1DelaySecs(int secs) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v2::setTimer1DelaySecs(%d)", secs);

	if (secs == -1)
		secs = 32000;
	
	_timer->setCountdown(1, secs * 60);
}

} // end of namespace Kyra
