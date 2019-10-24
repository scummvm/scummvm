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
#include "griffon/config.h"
#include "griffon/console.h"

#include "common/system.h"

namespace Griffon {

// copypaste from hRnd_CRT()
float GriffonEngine::RND() {
	/* return between 0 and 1 (but never 1) */
	return (float)_rnd->getRandomNumber(32767) * (1.0f / 32768.0f);
}

void GriffonEngine::mainLoop() {
	swash();

	if (_pmenu) {
		haltSoundChannel(_menuchannel);
		_pmenu = false;
	}

	do {
		if (!_forcepause) {
			updateAnims();
			updateNPCs();
		}

		checkTrigger();
		checkInputs();

		if (!_forcepause)
			handleWalking();

		updateY();
		drawView();

		updateMusic();

		 _console->onFrame();

		updateEngine();
	} while (!_shouldQuit);
}

void GriffonEngine::updateEngine() {
	g_system->updateScreen();
	g_system->getEventManager()->pollEvent(_event);

	_tickspassed = _ticks;
	_ticks = g_system->getMillis();

	_tickspassed = _ticks - _tickspassed;
	_fpsr = (float)_tickspassed / 24.0;

	_fp++;
	if (_ticks > _nextticks) {
		_nextticks = _ticks + 1000;
		_fps = _fp;
		_fp = 0;
		_secsingame = _secsingame + 1;
	}

	if (attacking) {
		_player.attackFrame += _player.attackSpeed * _fpsr;
		if (_player.attackFrame >= 16) {
			attacking = false;
			_player.attackFrame = 0;
			_player.walkFrame = 0;
		}

		int pa = (int)(_player.attackFrame);

		for (int i = 0; i <= pa; i++) {
			if (ABS(_playerattackofs[_player.walkDir][i][2]) < kEpsilon) {
				_playerattackofs[_player.walkDir][i][2] = 1;

				float opx = _player.px;
				float opy = _player.py;

				_player.px = _player.px + _playerattackofs[_player.walkDir][i][0];
				_player.py = _player.py + _playerattackofs[_player.walkDir][i][1];

				int sx = (int)(_player.px / 2 + 6);
				int sy = (int)(_player.py / 2 + 10);
				uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
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
	}

	for (int i = 0; i < kMaxFloat; i++) {
		if (_floattext[i][0] > 0) {
			float spd = 0.5 * _fpsr;
			_floattext[i][0] = _floattext[i][0] - spd;
			_floattext[i][2] = _floattext[i][2] - spd;
			if (_floattext[i][0] < 0)
				_floattext[i][0] = 0;
		}

		if (_floaticon[i][0] > 0) {
			float spd = 0.5 * _fpsr;
			_floaticon[i][0] = _floaticon[i][0] - spd;
			_floaticon[i][2] = _floaticon[i][2] - spd;
			if (_floaticon[i][0] < 0)
				_floaticon[i][0] = 0;
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
			setChannelVolume(snd, config.effectsvol);
		}
	}

	_clipbg->copyRectToSurface(_clipbg2->getPixels(), _clipbg2->pitch, 0, 0, _clipbg2->w, _clipbg2->h);

	Common::Rect rc;

	rc.left = _player.px - 2;
	rc.top = _player.py - 2;
	rc.setWidth(5);
	rc.setHeight(5);

	_clipbg->fillRect(rc, 1000);

	if (!_forcepause) {
		for (int i = 0; i < 5; i++) {
			if (_player.foundSpell[i] == 1)
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

	if (_player.hp <= 0)
		theEnd();

	if (_roomlock) {
		_roomlock = false;
		for (int i = 1; i <= _lastnpc; i++)
			if (_npcinfo[i].hp > 0)
				_roomlock = true;
	}

	clouddeg += 0.1 * _fpsr;
	while (clouddeg >= 360)
		clouddeg = clouddeg - 360;

	_player.hpflash = _player.hpflash + 0.1 * _fpsr;
	if (_player.hpflash >= 2) {
		_player.hpflash = 0;
		_player.hpflashb = _player.hpflashb + 1;
		if (_player.hpflashb == 2)
			_player.hpflashb = 0;
		if (config.effects && _player.hpflashb == 0 && _player.hp < _player.maxHp / 4) {
			int snd = playSound(_sfx[kSndBeep]);
			setChannelVolume(snd, config.effectsvol);
		}
	}

	// cloudson = 0

	if (_itemselon == 1)
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

	_player.px = 0;
	_player.py = 0;
	_player.opx = 0;
	_player.opy = 0;
	_player.walkDir = 0;
	_player.walkFrame = 0;
	_player.walkSpeed = 0;
	_player.attackFrame = 0;
	_player.attackSpeed = 0;
	_player.hp = 0;
	_player.maxHp = 0;
	_player.hpflash = 0;
	_player.level = 0;
	_player.maxLevel = 0;
	_player.sword = 0;
	_player.shield = 0;
	_player.armour = 0;
	for (int i = 0; i < 5; i++) {
		_player.foundSpell[i] = 0;
		_player.spellCharge[i] = 0;
		_player.inventory[i] = 0;
	}
	_player.attackStrength = 0;
	_player.spellDamage = 0;
	_player.swordDamage = 0;
	_player.exp = 0;
	_player.nextLevel = 0;

	memset(_scriptflag, 0, sizeof(_scriptflag));
	memset(_objmapf, 0, sizeof(_objmapf));
	memset(_roomLocks, 0, sizeof(_roomLocks));

	_roomLocks[66] = 2;
	_roomLocks[24] = 2;
	_roomLocks[17] = 1;
	_roomLocks[34] = 1;
	_roomLocks[50] = 1;
	_roomLocks[73] = 1;
	_roomLocks[82] = 2;

	_player.walkSpeed = 1.1f;
	_animspd = 0.5f;
	attacking = false;
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

	_pgardens = false;
	_pboss = false;

	loadMap(2);

	mainLoop();
}


} // end of namespace Griffon
