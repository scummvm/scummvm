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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/eobcommon.h"
#include "kyra/timer.h"

#include "common/system.h"

namespace Kyra {

void LolEobBaseEngine::enableSysTimer(int sysTimer) {
	if (sysTimer != 2)
		return;

	for (int i = 0; i < getNumClock2Timers(); i++)
		_timer->pauseSingleTimer(getClock2Timer(i), false);
}

void LolEobBaseEngine::disableSysTimer(int sysTimer) {
	if (sysTimer != 2)
		return;

	for (int i = 0; i < getNumClock2Timers(); i++)
		_timer->pauseSingleTimer(getClock2Timer(i), true);
}

void LolEobBaseEngine::enableTimer(int id) {
	_timer->enable(id);
	_timer->setCountdown(id, _timer->getDelay(id));
}

void LolEobBaseEngine::timerProcessDoors(int timerNum) {
	for (int i = 0; i < 3; i++) {
		uint16 b = _openDoorState[i].block;
		if (!b)
			continue;

		int v = _openDoorState[i].state;
		int c = _openDoorState[i].wall;

		_levelBlockProperties[b].walls[c] += v;
		_levelBlockProperties[b].walls[c ^ 2] += v;

		int snd = 3;
		int flg = _wllWallFlags[_levelBlockProperties[b].walls[c]];
		if (flg & 0x20)
			snd = 5;
		else if (v == -1)
			snd = 4;

		if (_flags.gameID == GI_LOL) {
			if (!(_updateFlags & 1)) {
				snd_processEnvironmentalSoundEffect(snd + 28, b);
				if (!checkSceneUpdateNeed(b))
					updateEnvironmentalSfx(0);
			}
		} else {
			checkSceneUpdateNeed(b);
			updateEnvironmentalSfx(snd);
		}

		if (flg & 0x30)
			_openDoorState[i].block = 0;
	}
}

} // namespace Kyra

#endif
#ifdef ENABLE_EOB

namespace Kyra {

#define TimerV2(x) new Common::Functor1Mem<int, void, EobCoreEngine>(this, &EobCoreEngine::x)

void EobCoreEngine::setupTimers() {
	_timer->addTimer(0, TimerV2(timerProcessCharacterExchange), 9, false);
	_timer->addTimer(1, TimerV2(timerProcessFlyingObjects), 3, true);
	_timer->addTimer(0x20, TimerV2(timerProcessMonsters), 20, true);
	_timer->addTimer(0x21, TimerV2(timerProcessMonsters), 20, true);
	_timer->addTimer(0x22, TimerV2(timerProcessMonsters), 20, true);
	_timer->addTimer(0x23, TimerV2(timerProcessMonsters), 20, true);
	_timer->setNextRun(0x20, _system->getMillis());
	_timer->setNextRun(0x21, _system->getMillis() + 7 * _tickLength);
	_timer->setNextRun(0x22, _system->getMillis() + 14 * _tickLength);
	_timer->setNextRun(0x23, _system->getMillis() + 14 * _tickLength);
	_timer->addTimer(0x30, TimerV2(timerSpecialCharacterUpdate), 50, false);
	_timer->addTimer(0x31, TimerV2(timerSpecialCharacterUpdate), 50, false);
	_timer->addTimer(0x32, TimerV2(timerSpecialCharacterUpdate), 50, false);
	_timer->addTimer(0x33, TimerV2(timerSpecialCharacterUpdate), 50, false);
	_timer->addTimer(0x34, TimerV2(timerSpecialCharacterUpdate), 50, false);
	_timer->addTimer(0x35, TimerV2(timerSpecialCharacterUpdate), 50, false);
	_timer->addTimer(4, TimerV2(timerProcessDoors), 5, true);
	_timer->addTimer(5, TimerV2(timerUpdateTeleporters), 10, true);
	_timer->addTimer(6, TimerV2(timerUpdateFoodStatus), 1080, true);
	_timer->addTimer(7, TimerV2(timerUpdateMonsterIdleAnim), 25, true);
}

void EobCoreEngine::setCharEventTimer(int charIndex, uint32 countdown, int evnt, int updateExistingTimer) {
	uint32 ntime = _system->getMillis() + countdown * _tickLength;
	uint8 timerId = 0x30 | (charIndex & 0x0f);
	EobCharacter *c = &_characters[charIndex];

	if (!_timer->isEnabled(timerId)) {
		c->timers[0] = ntime;
		c->events[0] = evnt;
		_timer->setCountdown(timerId, countdown);
		enableTimer(timerId);
		return;
	}

	if (ntime < _timer->getNextRun(timerId))
		_timer->setNextRun(timerId, ntime);

	if (updateExistingTimer) {
		bool updated = false;
		int d = -1;

		for (int i = 0; i < 10 && updated == false; i++) {
			if (d == -1 && !c->timers[i])
				d = i;

			if (!updated && c->events[i] == evnt) {
				d = i;
				updated = true;
			}
		}

		assert(d != -1);

		c->timers[d] = ntime;
		c->events[d] = evnt;
	} else {
		for (int i = 0; i < 10; i++) {
			if (c->timers[i])
				continue;
			c->timers[i] = ntime;
			c->events[i] = evnt;
			return;
		}
	}
}

void EobCoreEngine::deleteCharEventTimer(int charIndex, int evnt) {
	EobCharacter *c = &_characters[charIndex];
	for (int i = 0; i < 10; i++) {
		if (c->events[i] == evnt) {
			c->events[i] = 0;
			c->timers[i] = 0;
		}
	}
	setupCharacterTimers();
}

void EobCoreEngine::setupCharacterTimers() {
	for (int i = 0; i < 6; i++) {
		EobCharacter *c = &_characters[i];
		if (!testCharacter(i, 1))
			continue;

		uint32 nextTimer = 0xffffffff;

		for (int ii = 0; ii < 10; ii++) {
			if (c->timers[ii] && c->timers[ii] < nextTimer)
				nextTimer = c->timers[ii];
		}
		uint32 ctime = _system->getMillis();

		if (nextTimer == 0xffffffff)
			_timer->disable(0x30 | i);
		else {
			enableTimer(0x30 | i);
			_timer->setCountdown(0x30 | i, (nextTimer - ctime) / _tickLength);
		}
	}
}

void EobCoreEngine::advanceTimers(uint32 millis) {
	uint32 ct = _system->getMillis();
	for (int i = 0; i < 6; i++) {
		EobCharacter *c = &_characters[i];
		for (int ii = 0; ii < 10; ii++) {
			if (c->timers[ii] > ct) {
				uint32 chrt = c->timers[ii] - ct;
				c->timers[ii] = chrt > millis ? ct + chrt - millis : ct;
			}
		}
	}

	setupCharacterTimers();

	if (_scriptTimersMode & 1) {
		for (int i = 0; i < _scriptTimersCount; i++) {
			if (_scriptTimers[i].next > ct) {
				uint32 chrt = _scriptTimers[i].next - ct;
				_scriptTimers[i].next = chrt > millis ? ct + chrt - millis : ct;
			}
		}
	}
}

void EobCoreEngine::timerProcessCharacterExchange(int timerNum) {
	_charExchangeSwap ^= 1;
	if (_charExchangeSwap) {
		int index = _exchangeCharacterId;
		_exchangeCharacterId = -1;
		gui_drawCharPortraitWithStats(index);
		_exchangeCharacterId = index;
	} else {
		gui_drawCharPortraitWithStats(_exchangeCharacterId);
	}
}

void EobCoreEngine::timerProcessFlyingObjects(int timerNum) {
	static const uint8 dirPosIndex[] = { 0x82, 0x83, 0x00, 0x01, 0x01, 0x80, 0x03, 0x82, 0x02, 0x03, 0x80, 0x81, 0x81, 0x00, 0x83, 0x02 };
	for (int i = 0; i < 10; i++) {
		EobFlyingObject *fo = &_flyingObjects[i];
		if (!fo->enable)
			continue;

		bool endFlight = fo->distance ? false : true;

		uint8 pos = dirPosIndex[(fo->direction << 2) + (fo->curPos & 3)];
		uint16 bl = fo->curBlock;
		bool newBl = (pos & 0x80) ? true : false;

		if (newBl) {
			bl = calcNewBlockPosition(fo->curBlock, fo->direction);
			pos &= 3;
			fo->u2 = 0;
		}

		if (updateObjectFlight(fo, bl, pos)) {
			if (newBl)
				runLevelScript(bl, 0x10);
			if (updateFlyingObjectHitTest(fo, bl, pos))
				endFlight = true;
		} else {
			if (fo->flags & 0x20) {
				if (!updateFlyingObjectHitTest(fo, fo->curBlock, fo->curPos))
					explodeObject(fo, fo->curBlock, fo->item);
			}
			endFlight = true;
		}

		if (endFlight)
			endObjectFlight(fo);

		_sceneUpdateRequired = true;
	}
}

void EobCoreEngine::timerProcessMonsters(int timerNum) {
	updateMonsters(timerNum & 0x0f);
}

void EobCoreEngine::timerSpecialCharacterUpdate(int timerNum) {
	int charIndex = timerNum & 0x0f;
	EobCharacter *c = &_characters[charIndex];
	uint32 ctime =  _system->getMillis();

	for (int i = 0; i < 10; i++) {
		if (!c->timers[i])
			continue;
		if (c->timers[i] > ctime)
			continue;

		c->timers[i] = 0;
		int evt = c->events[i];

		if (evt < 0) {
			removeCharacterEffect(-evt, charIndex, 1);
			continue;
		}

		switch (evt) {
		case 2:
		case 3:
			setCharEventTimer(charIndex, (c->effectFlags & 0x10000) ? 9 : 36, evt + 2, 1);
		case 0:
		case 1:
		case 4:
		case 5:
			setWeaponSlotStatus(charIndex, evt / 2, evt & 1);
			break;

		case 6:
			c->damageTaken = 0;
			gui_drawCharPortraitWithStats(charIndex);
			break;

		case 7:
			_txt->printMessage(_characterStatusStrings7[0], -1, c->name);
			c->strengthCur = c->strengthMax;
			c->strengthExtCur = c->strengthExtMax;
			if (_currentControlMode == 2)
				gui_drawCharPortraitWithStats(charIndex);
			break;

		case 8:
			if (c->flags & 2) {
				calcAndInflictCharacterDamage(charIndex, 0, 0, 5, 0x400, 5, 3);
				setCharEventTimer(charIndex, 546, 8, 1);
			} else {
				c->flags &= ~2;
				gui_drawCharPortraitWithStats(charIndex);
			}
			break;

		case 9:
			if (c->flags & 4) {
				_txt->printMessage(_characterStatusStrings9[0], -1, c->name);
				c->flags &= ~4;
				gui_drawCharPortraitWithStats(charIndex);
			}
			break;

		case 11:
			if (c->disabledSlots & 4) {
				c->disabledSlots &= ~4;
				if (_openBookChar == charIndex && _updateFlags)
					gui_drawSpellbook();
			}
			break;

		case 12:
			c->effectFlags &= ~0x1000;
			if (_characterStatusStrings12)
				_txt->printMessage(_characterStatusStrings12[0], -1, c->name);
			break;

		default:
			break;
		}
	}

	uint32 nextTimer = 0xffffffff;
	for (int i = 0; i < 10; i++) {
		if (c->timers[i] && c->timers[i] < nextTimer)
			nextTimer = c->timers[i];
	}

	if (nextTimer == 0xffffffff)
		_timer->disable(timerNum);
	else
		_timer->setCountdown(timerNum, (nextTimer - ctime) / _tickLength);
}

void EobCoreEngine::timerUpdateTeleporters(int timerNum) {
	_teleporterPulse ^= 1;
	for (int i = 0; i < 18; i++) {
		uint8 w = _visibleBlocks[i]->walls[_sceneDrawVarDown];
		if ((w == _teleporterWallId) || (_flags.gameID == GI_EOB2 && w == 74)) {
			_sceneUpdateRequired = true;
			return;
		}
	}
}

void EobCoreEngine::timerUpdateFoodStatus(int timerNum) {
	for (int i = 0; i < 6; i++) {
		if (checkInventoryForRings(i, 2))
			continue;
		EobCharacter *c = &_characters[i];
		if (c->food != 0 && c->flags & 1 && c->hitPointsCur > -10) {
			c->food--;
			gui_drawFoodStatusGraph(i);
		}
	}
}

void EobCoreEngine::timerUpdateMonsterIdleAnim(int timerNum) {
	for (int i = 0; i < 18; i++) {
		EobMonsterInPlay *m = &_monsters[i];
		if (m->mode == 7 || m->mode == 10 || (m->flags & 0x20) || (rollDice(1, 2, 0) != 1))
			continue;
		m->idleAnimState = (rollDice(1, 2, 0) << 4) | rollDice(1, 2, 0);
		checkSceneUpdateNeed(m->block);
	}
}

}	// End of namespace Kyra

#endif // ENABLE_EOB
