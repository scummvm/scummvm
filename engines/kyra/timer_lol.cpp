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
	_timer->addTimer(0, TimerV2(timerProcessDoors), 15, true);
	_timer->addTimer(0x10, TimerV2(timerProcessMonsters), 6, true);
	_timer->addTimer(0x11, TimerV2(timerProcessMonsters), 6, true);
	_timer->setNextRun(0x11, _system->getMillis() + 3 * _tickLength);
	_timer->addTimer(3, TimerV2(timerSpecialCharacterUpdate), 15, true);
	_timer->addTimer(4, TimerV2(timerProcessFlyingObjects), 1, true);
	_timer->addTimer(0x50, TimerV2(timerRunSceneAnimScript), 0, false);
	_timer->addTimer(0x51, TimerV2(timerRunSceneAnimScript), 0, false);
	_timer->addTimer(0x52, TimerV2(timerRunSceneAnimScript), 0, false);
	_timer->addTimer(8, TimerV2(timerRegeneratePoints), 1200, true);
	_timer->addTimer(9, TimerV2(timerUpdatePortraitAnimations), 10, true);
	_timer->addTimer(10, TimerV2(timerUpdateLampState), 360, true);
	_timer->addTimer(11, TimerV2(timerFadeMessageText), 360, false);
}

void LoLEngine::enableTimer(int id) {
	_timer->enable(id);
	_timer->setCountdown(id, _timer->getDelay(id));
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
		int c = _openDoorState[i].wall;

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
	for (int i = timerNum & 0x0f; i < 30; i += 2)
		updateMonster(&_monsters[i]);
}

void LoLEngine::timerSpecialCharacterUpdate(int timerNum) {
	int v = 0;
	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1))
			continue;

		for (int ii = 0; ii < 5; ii++) {
			if (!(_characters[i].characterUpdateEvents[ii]))
				continue;

			if (--_characters[i].characterUpdateDelay[ii] > 0) {
				if (_characters[i].characterUpdateDelay[ii] > v)
					v = _characters[i].characterUpdateDelay[ii];
				continue;
			}

			switch (_characters[i].characterUpdateEvents[ii] - 1) {
			case 0:
				if (_characters[i].weaponHit) {
					_characters[i].weaponHit = 0;
					_characters[i].characterUpdateDelay[ii] = calcMonsterSkillLevel(i, 6);
					if (_characters[i].characterUpdateDelay[ii] > v)
						v = _characters[i].characterUpdateDelay[ii];
				} else {
					_characters[i].flags &= 0xfffb;
				}

				gui_drawCharPortraitWithStats(i);
				break;

			case 1:
				_characters[i].damageSuffered = 0;
				gui_drawCharPortraitWithStats(i);
				break;

			case 2:
				_characters[i].flags &= 0xffbf;
				gui_drawCharPortraitWithStats(i);
				break;

			case 3:
				v = rollDice(1, 2);
				if (inflictDamage(i, v, 0x8000, 0, 0x80)) {
					_txt->printMessage(2, getLangString(0x4022), _characters[i].name);
					_characters[i].characterUpdateDelay[ii] = 10;
					if (_characters[i].characterUpdateDelay[ii] > v)
						v = _characters[i].characterUpdateDelay[ii];
				}
				break;

			case 4:
				_characters[i].flags &= 0xfeff;
				_txt->printMessage(0, getLangString(0x4027), _characters[i].name);
				gui_drawCharPortraitWithStats(i);
				break;

			case 5:
				setTemporaryFaceFrame(i, 0, 0, 1);
				break;

			case 6:
				_characters[i].flags &= 0xefff;
				gui_drawCharPortraitWithStats(i);
				break;

			case 7:
				restoreSwampPalette();
				break;

			default:
				break;
			}

			if (_characters[i].characterUpdateDelay[ii] <= 0)
				_characters[i].characterUpdateEvents[ii] = 0;
		}
	}

	if (v) {
		_timer->enable(3);
		_timer3Para = v * 15;
	} else {
		_timer->disable(3);
	}
}

void LoLEngine::timerProcessFlyingObjects(int timerNum) {
	for (int i = 0; i < 8; i++) {
		if (!_flyingObjects[i].enable)
			continue;
		updateFlyingObject(&_flyingObjects[i]);
	}
}

void LoLEngine::timerRunSceneAnimScript(int timerNum) {
	runLevelScript(0x401 + (timerNum & 0x0f), -1);
}

void LoLEngine::timerRegeneratePoints(int timerNum) {
	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1))
			continue;

		// check for Duble ring
		int hInc = (_characters[i].flags & 8) ? 0 : (itemEquipped(i, 228) ? 4 : 1);
		// check for Talba ring
		int mInc = _drainMagic ? ((_characters[i].magicPointsMax >> 5) * -1) :
			((_characters[i].flags & 8) ? 0 : (itemEquipped(i, 227) ? (_characters[i].magicPointsMax / 10) : 1));

		_characters[i].magicPointsCur = CLIP<int16>(_characters[i].magicPointsCur + mInc, 0, _characters[i].magicPointsMax);

		if (!(_characters[i].flags & 0x80))
			increaseCharacterHitpoints(i, hInc, false);

		gui_drawCharPortraitWithStats(i);
	}
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
			_characters[i].nextAnimUpdateCountdown = rollDice(1, 12) + 6;
		}
	}
}

void LoLEngine::timerUpdateLampState(int timerNum) {
	if ((_flagsTable[31] & 0x08) && (_flagsTable[31] & 0x04) && _lampOilStatus > 0)
		_lampOilStatus--;
}

void LoLEngine::timerFadeMessageText(int timerNum) {
	_timer->disable(timerNum);
	initTextFading(0, 0);
}

} // End of namespace Kyra

#endif // ENABLE_LOL

