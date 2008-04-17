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

#include "kyra/kyra_v3.h"
#include "kyra/timer.h"

namespace Kyra {

#define TimerV3(x) new Functor1Mem<int, void, KyraEngine_v3>(this, &KyraEngine_v3::x)

void KyraEngine_v3::setupTimers() {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v3::setupTimers()");

	_timer->addTimer(0, TimerV3(timerRestoreCommandLine), -1, 1);
	for (int i = 1; i <= 3; ++i)
		_timer->addTimer(i, TimerV3(timerRunSceneScript7), -1, 0);
	_timer->addTimer(4, TimerV3(timerFleaDeath), -1, 0);
	for (int i = 5; i <= 11; ++i)
		_timer->addTimer(i, TimerV3(timerRunSceneScript7), -1, 0);
	for (int i = 12; i <= 13; ++i)
		_timer->addTimer(i, TimerV3(timerRunSceneScript7), 0, 0);
}

void KyraEngine_v3::timerRestoreCommandLine(int arg) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v3::timerRestoreCommandLine(%d)", arg);
	if (_shownMessage)
		_restoreCommandLine = true;
}

void KyraEngine_v3::timerRunSceneScript7(int arg) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v3::timerRunSceneScript7(%d)", arg);
	_sceneScriptState.regs[1] = _mouseX;
	_sceneScriptState.regs[2] = _mouseY;
	_sceneScriptState.regs[3] = 0;
	_sceneScriptState.regs[4] = _itemInHand;
	_scriptInterpreter->startScript(&_sceneScriptState, 7);

	while (_scriptInterpreter->validScript(&_sceneScriptState))
		_scriptInterpreter->runScript(&_sceneScriptState);
}

void KyraEngine_v3::timerFleaDeath(int arg) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v3::timerFleaDeath(%d)", arg);
	warning("STUB timerFleaDeath");
}

void KyraEngine_v3::setWalkspeed(uint8 speed) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v3::setWalkspeed(%d)", speed);

	if (speed < 5)
		speed = 3;
	else
		speed = 5;

	_mainCharacter.walkspeed = speed;
}

void KyraEngine_v3::setCommandLineRestoreTimer(int secs) {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "KyraEngine_v3::setCommandLineRestoreTimer(%d)", secs);
	if (secs == -1)
		secs = 32000;
	_timer->setCountdown(0, secs*60);
}

} // end of namespace Kyra
