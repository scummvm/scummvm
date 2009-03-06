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

#ifdef ENABLE_LOL

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/timer.h"

namespace Kyra {

#define TimerV2(x) new Common::Functor1Mem<int, void, LoLEngine>(this, &LoLEngine::x)

void LoLEngine::setupTimers() {
	debugC(9, kDebugLevelMain | kDebugLevelTimer, "LoLEngine::setupTimers()");

	_timer->addTimer(0, TimerV2(timerProcessDoors), 15, true);
	_timer->addTimer(0x10, TimerV2(timerProcessMonsters), 6, true);
	_timer->addTimer(0x11, TimerV2(timerProcessMonsters), 6, true);
	_timer->setNextRun(0x11, _system->getMillis() + 3 * _tickLength);
	_timer->addTimer(3, TimerV2(timerSub3), 15, true);
	_timer->addTimer(4, TimerV2(timerSub4), 1, true);
	_timer->addTimer(0x50, TimerV2(timerSub5), 0, false);
	_timer->addTimer(0x51, TimerV2(timerSub5), 0, false);
	_timer->addTimer(0x52, TimerV2(timerSub5), 0, false);
	_timer->addTimer(8, TimerV2(timerSub6), 1200, true);
	_timer->addTimer(9, TimerV2(timerUpdatePortraitAnimations), 10, true);
	_timer->addTimer(10, TimerV2(timerUpdateLampState), 360, true);
	_timer->addTimer(11, TimerV2(timerFadeMessageText), 360, false);
}

void LoLEngine::enableTimer(int id) {
	_timer->enable(id);
	_timer->setNextRun(id, _system->getMillis() + _timer->getDelay(id) * _tickLength);
}

void LoLEngine::enableSysTimer(int sysTimer) {
	if (sysTimer != 2)
		return;

	for (int i = 0; i < _numClock2Timers; i++)
		_timer->pauseSingleTimer(_clock2Timers[i], false);
}

void LoLEngine::disableSysTimer(int sysTimer) {
	if (sysTimer != 2)
		return;

	for (int i = 0; i < _numClock2Timers; i++)
		_timer->pauseSingleTimer(_clock2Timers[i], true);
}

void LoLEngine::timerProcessDoors(int timerNum) {
	for (int i = 0; i < 3; i++) {
		uint16 b = _openDoorState[i].block;
		if (!b)
			continue;

		int v = _openDoorState[i].state;
		int c = _openDoorState[i].field_2;

		_levelBlockProperties[b].walls[c] += v;
		_levelBlockProperties[b].walls[c ^ 2] += v;

		int snd = 31;

		int flg = _wllWallFlags[_levelBlockProperties[b].walls[c]];
		if (flg & 0x20)
			snd = 33;
		else if (v == -1)
			snd = 32;

		if (!(_updateFlags & 1)) {
			snd_processEnvironmentalSoundEffect(snd, b);
			if (!checkSceneUpdateNeed(b))
				updateEnvironmentalSfx(0);
		}

		if (flg & 0x30)
			_openDoorState[i].block = 0;
	}
}

void LoLEngine::timerProcessMonsters(int timerNum) {
//	if (!_updateMonsters)
//		return;

	for (int i = timerNum & 0x0f; i < 30; i += 2)
		updateMonster(&_monsters[i]);
}

void LoLEngine::timerSub3(int timerNum) {

}

void LoLEngine::timerSub4(int timerNum) {

}

void LoLEngine::timerSub5(int timerNum) {
	runLevelScript(0x401 + (timerNum & 0x0f), -1);
}

void LoLEngine::timerSub6(int timerNum) {

}

void LoLEngine::timerUpdatePortraitAnimations(int skipUpdate) {
	if (skipUpdate != 1)
		skipUpdate = 0;

	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1) || (_characters[i].flags & 8) || (_characters[i].curFaceFrame > 1))
			continue;

		if (_characters[i].curFaceFrame != 1) {
			if (--_characters[i].nextAnimUpdateCountdown <= 0 && !skipUpdate) {
				_characters[i].curFaceFrame = 1;
				gui_drawCharPortraitWithStats(i);
				_timer->setCountdown(9, 10);
			}
		} else {
			_characters[i].curFaceFrame = 0;
			gui_drawCharPortraitWithStats(i);
			_characters[i].nextAnimUpdateCountdown = (int16) _rnd.getRandomNumberRng(1, 12) + 6;
		}
	}
}

void LoLEngine::timerUpdateLampState(int timerNum) {
	if ((_screen->_drawGuiFlag & 0x800) && (_screen->_drawGuiFlag & 0x400) && _lampStatusUnk)
		_lampStatusUnk--;
}

void LoLEngine::timerFadeMessageText(int timerNum) {
	_timer->disable(timerNum);
	initTextFading(0, 0);
}

} // end of namespace Kyra

#endif // ENABLE_LOL

