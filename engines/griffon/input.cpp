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

#include "common/system.h"

#include "griffon/griffon.h"
#include "griffon/console.h"

namespace Griffon {

void GriffonEngine::checkInputs() {
	int ntickdelay = 175;
	bool _keyPressed = false;

	if (g_system->getEventManager()->pollEvent(_event))
		_keyPressed = true;

	_postInfoNbr = 0;

	for (int i = 0; i <= 20; i++) {
		_postInfo[i][0] = 0;
		_postInfo[i][1] = 0;
	}

	for (int x = 0; x <= 19; x++) {
		for (int y = 0; y <= 14; y++) {
			int o = _objectMap[x][y];
			if (_objectInfo[o].type == 3) {
				_postInfo[_postInfoNbr][0] = x * 16;
				_postInfo[_postInfoNbr][1] = y * 16;
				_postInfoNbr = _postInfoNbr + 1;
			}
		}
	}

	if (_attacking || (_forcePause && !_itemSelOn))
		return;

	if (_event.type == Common::EVENT_QUIT) {
		_shouldQuit = true;
		return;
	}

	if (_event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
		if (_event.customType == kGriffonMenu) {
			if (_itemTicks < _ticks)
				title(1);
		} else if (_event.customType == kGriffonAttack) {
			if (!_itemSelOn && (_itemTicks < _ticks))
				attack();

			if (_itemSelOn && _itemTicks < _ticks) {
				if (_curItem == 0 && _player.inventory[kInvFlask] > 0) {
					_itemTicks = _ticks + ntickdelay;

					int heal = 50;
					int maxh = _player.maxHp - _player.hp;

					if (heal > maxh)
						heal = maxh;

					_player.hp = _player.hp + heal;

					char text[256];
					sprintf(text, "+%i", heal);
					addFloatText(text, _player.px + 16 - 4 * strlen(text), _player.py + 16, 5);

					_player.inventory[kInvFlask]--;

					if (config.effects) {
						int snd = playSound(_sfx[kSndPowerUp]);
						setChannelVolume(snd, config.effectsVol);
					}

					_itemSelOn = false;
					_forcePause = false;
				}

				if (_curItem == 1 && _player.inventory[kInvDoubleFlask] > 0) {
					_itemTicks = _ticks + ntickdelay;

					int heal = 200;
					int maxHeal = _player.maxHp - _player.hp;

					if (heal > maxHeal)
						heal = maxHeal;

					_player.hp += heal;

					char text[256];
					sprintf(text, "+%i", heal);
					addFloatText(text, _player.px + 16 - 4 * strlen(text), _player.py + 16, 5);

					_player.inventory[kInvDoubleFlask]--;

					if (config.effects) {
						int snd = playSound(_sfx[kSndPowerUp]);
						setChannelVolume(snd, config.effectsVol);
					}

					_itemSelOn = false;
					_forcePause = false;
				}

				if (_curItem == 2 && _player.inventory[kInvShock] > 0) {
					castSpell(8, _player.px, _player.py, _npcInfo[_curEnemy].x, _npcInfo[_curEnemy].y, 0);

					_forcePause = true;

					_player.inventory[kInvShock]--;

					_itemTicks = _ticks + ntickdelay;
					_selEnemyOn = false;
					_itemSelOn = false;

				}

				if (_curItem == 3 && _player.inventory[kInvNormalKey] > 0 && _canUseKey && _lockType == 1) {
					_roomLocks[_roomToUnlock] = 0;
					eventText("UnLocked!");

					_player.inventory[kInvNormalKey]--;

					_itemTicks = _ticks + ntickdelay;
					_selEnemyOn = false;
					_itemSelOn = false;
					return;
				}

				if (_curItem == 4 && _player.inventory[kInvMasterKey] > 0 && _canUseKey && _lockType == 2) {
					_roomLocks[_roomToUnlock] = 0;
					eventText("UnLocked!");

					_player.inventory[kInvMasterKey]--;

					_itemTicks = _ticks + ntickdelay;
					_selEnemyOn = false;
					_itemSelOn = false;
					return;
				}

				if (_curItem == 5 && _player.spellCharge[0] == 100) {
					castSpell(5, _player.px, _player.py, _npcInfo[_curEnemy].x, _npcInfo[_curEnemy].y, 0);

					_player.spellCharge[0] = 0;

					_forcePause = true;

					_itemTicks = _ticks + ntickdelay;
					_selEnemyOn = false;
					_itemSelOn = false;
				}

				if (_curItem > 5 && _selEnemyOn) {
					if (_curEnemy <= _lastNpc) {
						castSpell(_curItem - 6, _player.px, _player.py, _npcInfo[_curEnemy].x, _npcInfo[_curEnemy].y, 0);
					} else {
						int pst = _curEnemy - _lastNpc - 1;
						castSpell(_curItem - 6, _player.px, _player.py, _postInfo[pst][0], _postInfo[pst][1], 0);
					}

					_player.spellCharge[_curItem - 5] = 0;

					_player.spellStrength = 0;

					_itemTicks = _ticks + ntickdelay;
					_selEnemyOn = false;
					_itemSelOn = false;
					_forcePause = false;
				}

				if (_curItem > 5 && !_selEnemyOn && _itemSelOn) {
					if (ABS(_player.spellCharge[_curItem - 5] - 100) < kEpsilon) {
						_itemTicks = _ticks + ntickdelay;

						_selEnemyOn = true;

						int i = 0;
						do {
							if (_npcInfo[i].hp > 0) {
								_curEnemy = i;
								goto __exit_do;
							}
							i = i + 1;
							if (i == _lastNpc + 1) {
								_selEnemyOn = false;
								goto __exit_do;
							}
						} while (1);
__exit_do:

						if (_postInfoNbr > 0 && !_selEnemyOn) {
							_selEnemyOn = true;
							_curEnemy = _lastNpc + 1;
						}
					}

				}
			}
		} else if (_event.customType == kGriffonInventory) {
			if (_itemTicks < _ticks) {
				_selEnemyOn = false;
				if (_itemSelOn) {
					_itemSelOn = false;
					_itemTicks = _ticks + 220;
					_forcePause = false;
				} else {
					_itemSelOn = true;
					_itemTicks = _ticks + 220;
					_forcePause = true;
					_player.itemselshade = 0;
				}
			}
		}
	}

	if (!_itemSelOn) {
		_movingUp = false;
		_movingDown = false;
		_movingLeft = false;
		_movingRight = false;
		// We continue moving even after the key has been released until we receive a different event
		if (_event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START || _event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
			if (_event.customType == kGriffonUp)
				_movingUp = true;
			if (_event.customType == kGriffonDown)
				_movingDown = true;
			if (_event.customType == kGriffonLeft)
				_movingLeft = true;
			if (_event.customType == kGriffonRight)
				_movingRight = true;
		}
	} else {
		_movingUp = false;
		_movingDown = false;
		_movingLeft = false;
		_movingRight = false;

		if (_selEnemyOn) {
			if (_itemTicks < _ticks) {
				if (_event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
					if (_event.customType == kGriffonLeft) {
						int origin = _curEnemy;
						do {
							_curEnemy = _curEnemy - 1;
							if (_curEnemy < 1)
								_curEnemy = _lastNpc + _postInfoNbr;
							if (_curEnemy == origin)
								break;
							if (_curEnemy <= _lastNpc && _npcInfo[_curEnemy].hp > 0)
								break;
							if (_curEnemy > _lastNpc)
								break;
						} while (1);
						_itemTicks = _ticks + ntickdelay;
					}
					if (_event.customType == kGriffonRight) {
						int origin = _curEnemy;
						do {
							_curEnemy = _curEnemy + 1;
							if (_curEnemy > _lastNpc + _postInfoNbr)
								_curEnemy = 1;
							if (_curEnemy == origin)
								break;
							if (_curEnemy <= _lastNpc && _npcInfo[_curEnemy].hp > 0)
								break;
							if (_curEnemy > _lastNpc)
								break;
						} while (1);
						_itemTicks = _ticks + ntickdelay;
					}
				}

				if (_curEnemy > _lastNpc + _postInfoNbr)
					_curEnemy = 1;
				if (_curEnemy < 1)
					_curEnemy = _lastNpc + _postInfoNbr;
			}
		} else {
			if (_keyPressed && _event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				if (_event.customType == kGriffonUp) {
					_curItem = _curItem - 1;
					_itemTicks = _ticks + ntickdelay;
					if (_curItem == 4)
						_curItem = 9;
					if (_curItem == -1)
						_curItem = 4;
				}
				if (_event.customType == kGriffonDown) {
					_curItem = _curItem + 1;
					_itemTicks = _ticks + ntickdelay;
					if (_curItem == 5)
						_curItem = 0;
					if (_curItem == 10)
						_curItem = 5;
				}
				if (_event.customType == kGriffonLeft) {
					_curItem = _curItem - 5;
					_itemTicks = _ticks + ntickdelay;
				}
				if (_event.customType == kGriffonRight) {
					_curItem = _curItem + 5;
					_itemTicks = _ticks + ntickdelay;
				}
			}

			if (_curItem > 9)
				_curItem = _curItem - 10;
			if (_curItem < 0)
				_curItem = _curItem + 10;
		}
	}
}

void GriffonEngine::handleWalking() {
	int xmax = 20 * 16 - 25;
	int ymax = 15 * 16 - 25;

	float px = _player.px;
	float py = _player.py;
	float opx = px;
	float opy = py;

	float spd = _player.walkSpeed * _fpsr;

	float nx = (px / 2 + 6);
	float ny = (py / 2 + 10);

	float npx = px + 12;
	float npy = py + 20;
	int lx = (int)npx / 16;
	int ly = (int)npy / 16;

	int ramp = _rampData[lx][ly];
	if (ramp == 1 && _movingUp)
		spd *= 2;
	if (ramp == 1 && _movingDown)
		spd *= 2;

	if (ramp == 2 && _movingLeft)
		_movingUp = true;
	if (ramp == 2 && _movingRight)
		_movingDown = true;

	if (ramp == 3 && _movingRight)
		_movingUp = true;
	if (ramp == 3 && _movingLeft)
		_movingDown = true;

	uint32 *temp/*, c*/, bgc;

	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			int sx = nx + x;
			int sy = ny + y;

			_clipSurround[x + 1][y + 1] = 0;
			if (sx > -1 && sx < 320 && sy > -1 && sy < 192) {
				temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
				_clipSurround[x + 1][y + 1] = *temp;
			}
		}
	}

	if (_movingUp)
		_player.walkDir = 0;
	if (_movingDown)
		_player.walkDir = 1;
	if (_movingLeft)
		_player.walkDir = 2;
	if (_movingRight)
		_player.walkDir = 3;

	if (_movingUp && _clipSurround[1][0] == 0) {
		py -= spd;
		_player.walkDir = 0;
	} else if (_movingUp && _clipSurround[1][0] > 0) {
		// move upleft
		if (!_movingRight && _clipSurround[0][0] == 0) {
			py -= spd;
			px -= spd;
		}

		// move upright
		if (!_movingLeft && _clipSurround[2][0] == 0) {
			py -= spd;
			px += spd;
		}
	}
	if (_movingDown && _clipSurround[1][2] == 0) {
		py += spd;
		_player.walkDir = 1;
	} else if (_movingDown && _clipSurround[1][2] > 0) {
		// move downleft
		if (_movingRight == 0 && _clipSurround[0][2] == 0) {
			py += spd;
			px -= spd;
		}

		// move downright
		if (_movingLeft == 0 && _clipSurround[2][2] == 0) {
			py += spd;
			px += spd;
		}
	}
	if (_movingLeft && _clipSurround[0][1] == 0) {
		px -= spd;
		_player.walkDir = 2;
	} else if (_movingLeft && _clipSurround[0][1] > 0) {
		// move leftup
		if (!_movingDown && _clipSurround[0][0] == 0) {
			py -= spd;
			px -= spd;
		}

		// move leftdown
		if (!_movingUp && _clipSurround[0][2] == 0) {
			py += spd;
			px -= spd;
		}
	}
	if (_movingRight && _clipSurround[2][1] == 0) {
		px += spd;
		_player.walkDir = 3;
	} else if (_movingRight && _clipSurround[2][1] > 0) {
		// move rightup
		if (!_movingDown && _clipSurround[2][0] == 0) {
			px += spd;
			py -= spd;
		}

		// move rightdown
		if (!_movingUp && _clipSurround[2][2] == 0) {
			py += spd;
			px += spd;
		}
	}

	if (px < -8)
		px = -8;
	if (px > xmax)
		px = xmax;
	if (py < -8)
		py = -8;
	if (py > ymax)
		py = ymax;

	int pass = 1;

	int sx = (px / 2 + 6);
	int sy = (py / 2 + 10);
	temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
	bgc = *temp;
	if (bgc > 0 && bgc != 1000) {
		px = opx;
		py = opy;
		pass = 0;
	}

	// push npc
	if (pass == 1) {
		for (int i = 1; i <= _lastNpc; i++) {
			if (_npcInfo[i].hp > 0) {
				npx = _npcInfo[i].x;
				npy = _npcInfo[i].y;

				opx = npx;
				opy = npy;

				int xdif = _player.px - npx;
				int ydif = _player.py - npy;

				if (_player.walkDir == 0) {
					if (ABS(xdif) <= 8 && ydif > 0 && ydif < 8)
						_npcInfo[i].y -= spd;
				} else if (_player.walkDir == 1) {
					if (ABS(xdif) <= 8 && ydif < 0 && ydif > -8)
						_npcInfo[i].y += spd;
				} else if (_player.walkDir == 2) {
					if (ABS(ydif) <= 8 && xdif > 0 && xdif < 8)
						_npcInfo[i].x -= spd;
				} else if (_player.walkDir == 3) {
					if (ABS(ydif) <= 8 && xdif < 0 && xdif > -8)
						_npcInfo[i].x += spd;
				}

				npx = _npcInfo[i].x;
				npy = _npcInfo[i].y;

				sx = (int)(npx / 2 + 6);
				sy = (int)(npy / 2 + 10);
				temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
				bgc = *temp;

				if (bgc > 0) {
					_npcInfo[i].x = opx;
					_npcInfo[i].y = opy;
				}
			}
		}
	}

	_player.opx = _player.px;
	_player.opy = _player.py;
	_player.px = px;
	_player.py = py;

	if (_player.px != _player.opx || _player.py != _player.opy)
		_player.walkFrame += _animSpeed * _fpsr;
	if (_player.walkFrame >= 16)
		_player.walkFrame -= 16;

	// walking over items to pickup :::
	int o = _objectMap[lx][ly];

	if (o > -1) {
		// fsk
		if (_objectInfo[o].type == 2 && _player.inventory[kInvFlask] < 9) {
			_objectMap[lx][ly] = -1;

			_player.inventory[kInvFlask]++;
			addFloatIcon(6, lx * 16, ly * 16);

			_objectMapFull[_curMap][lx][ly] = 1;

			if (config.effects) {
				int snd = playSound(_sfx[kSndPowerUp]);
				setChannelVolume(snd, config.effectsVol);
			}
		}

		if (_objectInfo[o].script == 7 && _player.inventory[kInvDoubleFlask] < 9) {
			_objectMap[lx][ly] = -1;

			_player.inventory[kInvDoubleFlask]++;
			addFloatIcon(12, lx * 16, ly * 16);

			_objectMapFull[_curMap][lx][ly] = 1;

			if (config.effects) {
				int snd = playSound(_sfx[kSndPowerUp]);
				setChannelVolume(snd, config.effectsVol);
			}
		}

		if (_objectInfo[o].script == 9 && _player.inventory[kInvShock] < 9 && (_curMap == 41 && _scriptFlag[kScriptLightningBomb][1] == 0)) {
			_objectMap[lx][ly] = -1;

			_player.inventory[kInvShock]++;
			addFloatIcon(17, lx * 16, ly * 16);

			_objectMapFull[_curMap][lx][ly] = 1;
			if (_curMap == 41)
				_scriptFlag[kScriptLightningBomb][1] = 1;

			if (config.effects) {
				int snd = playSound(_sfx[kSndPowerUp]);
				setChannelVolume(snd, config.effectsVol);
			}

		}

		if (_objectInfo[o].script == 9 && _player.inventory[kInvShock] < 9) {
			_objectMap[lx][ly] = -1;

			_player.inventory[kInvShock]++;
			addFloatIcon(17, lx * 16, ly * 16);

			_objectMapFull[_curMap][lx][ly] = 1;

			if (config.effects) {
				int snd = playSound(_sfx[kSndPowerUp]);
				setChannelVolume(snd, config.effectsVol);
			}

		}
	}
}

void GriffonEngine::checkTrigger() {
	int npx = _player.px + 12;
	int npy = _player.py + 20;

	int lx = (int)npx / 16;
	int ly = (int)npy / 16;

	_canUseKey = false;

	if (_triggerLoc[lx][ly] > -1)
		processTrigger(_triggerLoc[lx][ly]);
}

void GriffonEngine::processTrigger(int trignum) {
	int trigtype = _triggers[trignum][0];

	if (_roomLock)
		return;

	// map jump------------------------------
	if (trigtype == 0) {
		int tx = _triggers[trignum][1];
		int ty = _triggers[trignum][2];
		int tmap = _triggers[trignum][3];
		int tjumpstyle = _triggers[trignum][4];

		if (_roomLocks[tmap] > 0) {
			if (!_saidLocked)
				eventText("Locked");
			_saidLocked = true;
			_canUseKey = true;
			_lockType = _roomLocks[tmap];
			_roomToUnlock = tmap;
			return;
		}

		if (tmap == 1) {
			if (!_saidJammed)
				eventText("Door Jammed!");
			_saidJammed = true;
			return;
		}

		_saidLocked = false;
		_saidJammed = false;

		// loc-sxy+oldmaploc
		if (tjumpstyle == 0) {

			int tsx = _triggers[trignum][5];
			int tsy = _triggers[trignum][6];

			_player.px += (tx - tsx) * 16;
			_player.py += (ty - tsy) * 16;

			// HACKFIX
			if (_player.px < 0)
				_player.px = 0;
			if (_player.py < 0)
				_player.py = 0;

			if (tmap > 0) {
				if (config.effects) {
					int snd = playSound(_sfx[kSndDoor]);
					setChannelVolume(snd, config.effectsVol);
				}

				loadMap(tmap);
				swash();
			}
		}
	}

	for (int i = 0; i < kMaxFloat; i++) {
		_floatText[i].framesLeft = 0;
		_floatIcon[i].framesLeft = 0;
	}
}

} // end of namespace Griffon
