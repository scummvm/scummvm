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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 *              Originally written by Syn9 in FreeBASIC with SDL
 *              http://syn9.thehideoutgames.com/index_backup.php
 *
 *            Ported to plain C for GCW-Zero handheld by Dmitry Smagin
 *                http://github.com/dmitrysmagin/griffon_legend
 *
 *
 *                 Programming/Graphics: Daniel "Syn9" Kennedy
 *                     Music/Sound effects: David Turner
 *
 *                   Beta testing and gameplay design help:
 *                    Deleter, Cha0s, Aether Fox, and Kiz
 *
 */

#include "griffon/griffon.h"
#include "griffon/console.h"

#include "common/system.h"

namespace Griffon {

void Player::reset() {
	px = 0;
	py = 0;
	opx = 0;
	opy = 0;
	walkDir = 0;
	walkFrame = 0;
	walkSpeed = 0;
	attackFrame = 0;
	attackSpeed = 0;
	hp = 0;
	maxHp = 0;
	hpflash = 0;
	hpflashb = 0;
	level = 0;
	maxLevel = 0;
	sword = 0;
	shield = 0;
	armour = 0;
	for (int i = 0; i < 5; i++) {
		foundSpell[i] = 0;
		spellCharge[i] = 0;
		inventory[i] = 0;
	}
	attackStrength = 0;
	itemselshade = 0;
	spellDamage = 0;
	swordDamage = 0;
	exp = 0;
	nextLevel = 0;
}

// copypaste from hRnd_CRT()
float GriffonEngine::RND() {
	/* return between 0 and 1 (but never 1) */
	return (float)_rnd->getRandomNumber(32767) * (1.0f / 32768.0f);
}

void GriffonEngine::mainLoop() {
	swash();

	if (_pmenu) {
		haltSoundChannel(_menuChannel);
		_pmenu = false;
	}

	_gameMode = kGameModePlay;

	do {
		if (!_forcePause) {
			updateAnims();
			updateNPCs();
		}

		checkTrigger();
		checkInputs();

		if (!_forcePause)
			handleWalking();

		updateY();
		drawView();

		updateMusic();
		updateEngine();
	} while (!_shouldQuit && _gameMode == kGameModePlay);
}

void GriffonEngine::updateEngine() {
	g_system->updateScreen();

	_ticksPassed = _ticks;
	_ticks = g_system->getMillis();

	_ticksPassed = _ticks - _ticksPassed;
	_fpsr = (float)_ticksPassed / 24.0;

	_fp++;
	if (_ticks > _nextTicks) {
		_nextTicks = _ticks + 1000;
		_fps = _fp;
		_fp = 0;
		_secsInGame = _secsInGame + 1;
	}

	if (_attacking) {
		_player.attackFrame += _player.attackSpeed * _fpsr;
		if (_player.attackFrame >= 16) {
			_attacking = false;
			_player.attackFrame = 0;
			_player.walkFrame = 0;
		}

		int pa = (int)(_player.attackFrame);

		for (int i = 0; i <= pa; i++) {
			if (!_playerAttackOfs[_player.walkDir][i].completed) {
				_playerAttackOfs[_player.walkDir][i].completed = true;

				float opx = _player.px;
				float opy = _player.py;

				_player.px += _playerAttackOfs[_player.walkDir][i].x;
				_player.py += _playerAttackOfs[_player.walkDir][i].y;

				int sx = (int)(_player.px / 2 + 6);
				int sy = (int)(_player.py / 2 + 10);
				uint32 *temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
				uint32 bgc = *temp;
				if (bgc > 0) {
					_player.px = opx;
					_player.py = opy;
				}
			}
		}

		_player.opx = _player.px;
		_player.opy = _player.py;

		checkHit();

		if (_gameMode != kGameModePlay)
			return;
	}

	for (int i = 0; i < kMaxFloat; i++) {
		if (_floatText[i].framesLeft > 0) {
			float spd = 0.5 * _fpsr;
			_floatText[i].framesLeft -= spd;
			_floatText[i].y -= spd;
			if (_floatText[i].framesLeft < 0)
				_floatText[i].framesLeft = 0;
		}

		if (_floatIcon[i].framesLeft > 0) {
			float spd = 0.5 * _fpsr;
			_floatIcon[i].framesLeft -= spd;
			_floatIcon[i].y -= spd;
			if (_floatIcon[i].framesLeft < 0)
				_floatIcon[i].framesLeft = 0;
		}
	}

	if (_player.level == _player.maxLevel)
		_player.exp = 0;

	if (_player.exp >= _player.nextLevel) {
		_player.level = _player.level + 1;
		addFloatText("LEVEL UP!", _player.px + 16 - 36, _player.py + 16, 3);
		_player.exp = _player.exp - _player.nextLevel;
		_player.nextLevel = _player.nextLevel * 3 / 2; // 1.5
		_player.maxHp = _player.maxHp + _player.level * 3;
		if (_player.maxHp > 999)
			_player.maxHp = 999;
		_player.hp = _player.maxHp;

		_player.swordDamage = _player.level * 14 / 10;
		_player.spellDamage = _player.level * 13 / 10;

		if (config.effects) {
			int snd = playSound(_sfx[kSndPowerUp]);
			setChannelVolume(snd, config.effectsVol);
		}
	}

	_clipBg->copyRectToSurface(_clipBg2->getPixels(), _clipBg2->pitch, 0, 0, _clipBg2->w, _clipBg2->h);

	Common::Rect rc;

	rc.left = _player.px - 2;
	rc.top = _player.py - 2;
	rc.setWidth(5);
	rc.setHeight(5);

	_clipBg->fillRect(rc, 1000);

	if (!_forcePause) {
		for (int i = 0; i < 5; i++) {
			if (_player.foundSpell[i])
				_player.spellCharge[i] += 1 * _player.level * 0.01 * _fpsr;
			if (_player.spellCharge[i] > 100)
				_player.spellCharge[i] = 100;
		}

		if (_player.foundSpell[0]) {
			_player.spellStrength += 3 * _player.level * .01 * _fpsr;
		}

		_player.attackStrength += (30 + 3 * (float)_player.level) / 50 * _fpsr;
	}

	if (_player.attackStrength > 100)
		_player.attackStrength = 100;

	if (_player.spellStrength > 100)
		_player.spellStrength = 100;

	_itemyloc += 0.75 * _fpsr;
	while (_itemyloc >= 16)
		_itemyloc -= 16;

	if (_player.hp <= 0) {
		theEnd();

		_gameMode = kGameModeIntro;

		return;
	}

	if (_roomLock) {
		_roomLock = false;
		for (int i = 1; i <= _lastNpc; i++)
			if (_npcInfo[i].hp > 0)
				_roomLock = true;
	}

	_cloudAngle += 0.1 * _fpsr;
	while (_cloudAngle >= 360)
		_cloudAngle = _cloudAngle - 360;

	_player.hpflash = _player.hpflash + 0.1 * _fpsr;
	if (_player.hpflash >= 2) {
		_player.hpflash = 0;
		_player.hpflashb = _player.hpflashb + 1;
		if (_player.hpflashb == 2)
			_player.hpflashb = 0;
		if (config.effects && _player.hpflashb == 0 && _player.hp < _player.maxHp / 4) {
			int snd = playSound(_sfx[kSndBeep]);
			setChannelVolume(snd, config.effectsVol);
		}
	}

	// _cloudsOn = false

	if (_itemSelOn)
		_player.itemselshade = _player.itemselshade + 2 * _fpsr;
	if (_player.itemselshade > 24)
		_player.itemselshade = 24;

	for (int i = 0; i <= 4; i++)
		if (_player.inventory[i] > 9)
			_player.inventory[i] = 9;
}

void GriffonEngine::newGame() {
	intro();

	if (_shouldQuit)
		return;

	_player.reset();
	_playera.reset();

	memset(_scriptFlag, 0, sizeof(_scriptFlag));
	memset(_objectMapFull, 0, sizeof(_objectMapFull));
	memset(_roomLocks, 0, sizeof(_roomLocks));

	_roomLocks[66] = 2;
	_roomLocks[24] = 2;
	_roomLocks[17] = 1;
	_roomLocks[34] = 1;
	_roomLocks[50] = 1;
	_roomLocks[73] = 1;
	_roomLocks[82] = 2;

	_player.walkSpeed = 1.1f;
	_animSpeed = 0.5f;
	_attacking = false;
	_player.attackSpeed = 1.5f;

	_player.sword = 1;
	_player.level = 1;
	_player.maxLevel = 22;
	_player.nextLevel = 50;
	_player.shield = 1;
	_player.armour = 1;
	_player.hp = 14;
	_player.maxHp = _player.hp;

	_player.swordDamage = _player.level * 2;
	_player.spellDamage = _player.level * 3 / 2;

	_player.px = 15 * 16 - 4;
	_player.py = 6 * 16 - 4;
	_player.walkDir = 1;

	_playingGardens = false;
	_playingBoss = false;

	_player.spellStrength = 0;

	loadMap(2);

	mainLoop();
}


} // end of namespace Griffon
