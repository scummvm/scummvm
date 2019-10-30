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
#include "griffon/config.h"
#include "griffon/console.h"

namespace Griffon {

void GriffonEngine::checkInputs() {
	int ntickdelay = 175;

	g_system->getEventManager()->pollEvent(_event);

	nposts = 0;

	for (int i = 0; i <= 20; i++) {
		postinfo[i][0] = 0;
		postinfo[i][1] = 0;
	}

	for (int x = 0; x <= 19; x++) {
		for (int y = 0; y <= 14; y++) {
			int o = _objectMap[x][y];
			if (_objectInfo[o][4] == 3) {
				postinfo[nposts][0] = x * 16;
				postinfo[nposts][1] = y * 16;
				nposts = nposts + 1;
			}
		}
	}

	if (attacking || (_forcepause && !_itemSelOn))
		return;

	if (_event.type == Common::EVENT_QUIT) {
		_shouldQuit = true;
		return;
	}

	if (_event.type == Common::EVENT_KEYDOWN) {
		if (_event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			if (_itemticks < _ticks)
				title(1);
		} else if (_event.kbd.keycode == Common::KEYCODE_d && _event.kbd.hasFlags(Common::KBD_CTRL)) {
			_console->attach();
			_event.type = Common::EVENT_INVALID;
		} else if (_event.kbd.hasFlags(Common::KBD_CTRL)) {
			if (!_itemSelOn && (_itemticks < _ticks))
				attack();

			if (_itemSelOn && _itemticks < _ticks) {
				if (_curitem == 0 && _player.inventory[kInvFlask] > 0) {
					_itemticks = _ticks + ntickdelay;

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
						setChannelVolume(snd, config.effectsvol);
					}

					_itemSelOn = false;
					_forcepause = false;
				}

				if (_curitem == 1 && _player.inventory[kInvDoubleFlask] > 0) {
					_itemticks = _ticks + ntickdelay;

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
						setChannelVolume(snd, config.effectsvol);
					}

					_itemSelOn = false;
					_forcepause = false;
				}

				if (_curitem == 2 && _player.inventory[kInvShock] > 0) {
					castSpell(8, _player.px, _player.py, _npcinfo[_curenemy].x, _npcinfo[_curenemy].y, 0);

					_forcepause = true;

					_player.inventory[kInvShock]--;

					_itemticks = _ticks + ntickdelay;
					_selEnemyOn = false;
					_itemSelOn = false;

				}

				if (_curitem == 3 && _player.inventory[kInvNormalKey] > 0 && _canUseKey && _lockType == 1) {
					_roomLocks[_roomToUnlock] = 0;
					eventText("UnLocked!");

					_player.inventory[kInvNormalKey]--;

					_itemticks = _ticks + ntickdelay;
					_selEnemyOn = false;
					_itemSelOn = false;
					return;
				}

				if (_curitem == 4 && _player.inventory[kInvMasterKey] > 0 && _canUseKey && _lockType == 2) {
					_roomLocks[_roomToUnlock] = 0;
					eventText("UnLocked!");

					_player.inventory[kInvMasterKey]--;

					_itemticks = _ticks + ntickdelay;
					_selEnemyOn = false;
					_itemSelOn = false;
					return;
				}

				if (_curitem == 5 && _player.spellCharge[0] == 100) {
					castSpell(5, _player.px, _player.py, _npcinfo[_curenemy].x, _npcinfo[_curenemy].y, 0);

					_player.spellCharge[0] = 0;

					_forcepause = true;

					_itemticks = _ticks + ntickdelay;
					_selEnemyOn = false;
					_itemSelOn = false;
				}

				if (_curitem > 5 && _selEnemyOn) {
					if (_curenemy <= _lastnpc) {
						castSpell(_curitem - 6, _player.px, _player.py, _npcinfo[_curenemy].x, _npcinfo[_curenemy].y, 0);
					} else {
						int pst = _curenemy - _lastnpc - 1;
						castSpell(_curitem - 6, _player.px, _player.py, postinfo[pst][0], postinfo[pst][1], 0);
					}

					_player.spellCharge[_curitem - 5] = 0;

					_player.spellStrength = 0;

					_itemticks = _ticks + ntickdelay;
					_selEnemyOn = false;
					_itemSelOn = false;
					_forcepause = false;
				}

				if (_curitem > 5 && !_selEnemyOn && _itemSelOn) {
					if (ABS(_player.spellCharge[_curitem - 5] - 100) < kEpsilon) {
						_itemticks = _ticks + ntickdelay;

						_selEnemyOn = true;

						int i = 0;
						do {
							if (_npcinfo[i].hp > 0) {
								_curenemy = i;
								goto __exit_do;
							}
							i = i + 1;
							if (i == _lastnpc + 1) {
								_selEnemyOn = false;
								goto __exit_do;
							}
						} while (1);
__exit_do:

						if (nposts > 0 && !_selEnemyOn) {
							_selEnemyOn = true;
							_curenemy = _lastnpc + 1;
						}
					}

				}
			}
		} else if (_event.kbd.hasFlags(Common::KBD_ALT)) {
			if (_itemticks < _ticks) {
				_selEnemyOn = false;
				if (_itemSelOn) {
					_itemSelOn = false;
					_itemticks = _ticks + 220;
					_forcepause = false;
				} else {
					_itemSelOn = true;
					_itemticks = _ticks + 220;
					_forcepause = true;
					_player.itemselshade = 0;
				}
			}
		}
	}

	if (!_itemSelOn) {
		movingup = false;
		movingdown = false;
		movingleft = false;
		movingright = false;
		if (_event.kbd.keycode == Common::KEYCODE_UP)
			movingup = true;
		if (_event.kbd.keycode == Common::KEYCODE_DOWN)
			movingdown = true;
		if (_event.kbd.keycode == Common::KEYCODE_LEFT)
			movingleft = true;
		if (_event.kbd.keycode == Common::KEYCODE_RIGHT)
			movingright = true;
	} else {
		movingup = false;
		movingdown = false;
		movingleft = false;
		movingright = false;

		if (_selEnemyOn) {
			if (_itemticks < _ticks) {
				if (_event.kbd.keycode == Common::KEYCODE_LEFT) {
					int origin = _curenemy;
					do {
						_curenemy = _curenemy - 1;
						if (_curenemy < 1)
							_curenemy = _lastnpc + nposts;
						if (_curenemy == origin)
							break;
						if (_curenemy <= _lastnpc && _npcinfo[_curenemy].hp > 0)
							break;
						if (_curenemy > _lastnpc)
							break;
					} while (1);
					_itemticks = _ticks + ntickdelay;
				}
				if (_event.kbd.keycode == Common::KEYCODE_RIGHT) {
					int origin = _curenemy;
					do {
						_curenemy = _curenemy + 1;
						if (_curenemy > _lastnpc + nposts)
							_curenemy = 1;
						if (_curenemy == origin)
							break;
						if (_curenemy <= _lastnpc && _npcinfo[_curenemy].hp > 0)
							break;
						if (_curenemy > _lastnpc)
							break;
					} while (1);
					_itemticks = _ticks + ntickdelay;
				}


				if (_curenemy > _lastnpc + nposts)
					_curenemy = 1;
				if (_curenemy < 1)
					_curenemy = _lastnpc + nposts;
			}
		} else {
			if (_itemticks < _ticks) {
				if (_event.kbd.keycode == Common::KEYCODE_UP) {
					_curitem = _curitem - 1;
					_itemticks = _ticks + ntickdelay;
					if (_curitem == 4)
						_curitem = 9;
					if (_curitem == -1)
						_curitem = 4;
				}
				if (_event.kbd.keycode == Common::KEYCODE_DOWN) {
					_curitem = _curitem + 1;
					_itemticks = _ticks + ntickdelay;
					if (_curitem == 5)
						_curitem = 0;
					if (_curitem == 10)
						_curitem = 5;
				}
				if (_event.kbd.keycode == Common::KEYCODE_LEFT) {
					_curitem = _curitem - 5;
					_itemticks = _ticks + ntickdelay;
				}
				if (_event.kbd.keycode == Common::KEYCODE_RIGHT) {
					_curitem = _curitem + 5;
					_itemticks = _ticks + ntickdelay;
				}

				if (_curitem > 9)
					_curitem = _curitem - 10;
				if (_curitem < 0)
					_curitem = _curitem + 10;
			}
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

	int ramp = _rampdata[lx][ly];
	if (ramp == 1 && movingup)
		spd *= 2;
	if (ramp == 1 && movingdown)
		spd *= 2;

	if (ramp == 2 && movingleft)
		movingup = true;
	if (ramp == 2 && movingright)
		movingdown = true;

	if (ramp == 3 && movingright)
		movingup = true;
	if (ramp == 3 && movingleft)
		movingdown = true;

	unsigned int *temp/*, c*/, bgc;

	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			int sx = nx + x;
			int sy = ny + y;

			_clipsurround[x + 1][y + 1] = 0;
			if (sx > -1 && sx < 320 && sy > -1 && sy < 192) {
				temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
				_clipsurround[x + 1][y + 1] = *temp;
			}
		}
	}

	if (movingup)
		_player.walkDir = 0;
	if (movingdown)
		_player.walkDir = 1;
	if (movingleft)
		_player.walkDir = 2;
	if (movingright)
		_player.walkDir = 3;

	if (movingup && _clipsurround[1][0] == 0) {
		py -= spd;
		_player.walkDir = 0;
	} else if (movingup && _clipsurround[1][0] > 0) {
		// move upleft
		if (!movingright && _clipsurround[0][0] == 0) {
			py -= spd;
			px -= spd;
		}

		// move upright
		if (!movingleft && _clipsurround[2][0] == 0) {
			py -= spd;
			px += spd;
		}
	}
	if (movingdown && _clipsurround[1][2] == 0) {
		py += spd;
		_player.walkDir = 1;
	} else if (movingdown && _clipsurround[1][2] > 0) {
		// move downleft
		if (movingright == 0 && _clipsurround[0][2] == 0) {
			py += spd;
			px -= spd;
		}

		// move downright
		if (movingleft == 0 && _clipsurround[2][2] == 0) {
			py += spd;
			px += spd;
		}
	}
	if (movingleft && _clipsurround[0][1] == 0) {
		px -= spd;
		_player.walkDir = 2;
	} else if (movingleft && _clipsurround[0][1] > 0) {
		// move leftup
		if (!movingdown && _clipsurround[0][0] == 0) {
			py -= spd;
			px -= spd;
		}

		// move leftdown
		if (!movingup && _clipsurround[0][2] == 0) {
			py += spd;
			px -= spd;
		}
	}
	if (movingright && _clipsurround[2][1] == 0) {
		px += spd;
		_player.walkDir = 3;
	} else if (movingright && _clipsurround[2][1] > 0) {
		// move rightup
		if (!movingdown && _clipsurround[2][0] == 0) {
			px += spd;
			py -= spd;
		}

		// move rightdown
		if (!movingup && _clipsurround[2][2] == 0) {
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
	temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
	bgc = *temp;
	if (bgc > 0 && bgc != 1000) {
		px = opx;
		py = opy;
		pass = 0;
	}

	// push npc
	if (pass == 1) {
		for (int i = 1; i <= _lastnpc; i++) {
			if (_npcinfo[i].hp > 0) {
				npx = _npcinfo[i].x;
				npy = _npcinfo[i].y;

				opx = npx;
				opy = npy;

				int xdif = _player.px - npx;
				int ydif = _player.py - npy;

				if (_player.walkDir == 0) {
					if (abs(xdif) <= 8 && ydif > 0 && ydif < 8)
						_npcinfo[i].y -= spd;
				} else if (_player.walkDir == 1) {
					if (abs(xdif) <= 8 && ydif < 0 && ydif > -8)
						_npcinfo[i].y += spd;
				} else if (_player.walkDir == 2) {
					if (abs(ydif) <= 8 && xdif > 0 && xdif < 8)
						_npcinfo[i].x -= spd;
				} else if (_player.walkDir == 3) {
					if (abs(ydif) <= 8 && xdif < 0 && xdif > -8)
						_npcinfo[i].x += spd;
				}

				npx = _npcinfo[i].x;
				npy = _npcinfo[i].y;

				sx = (int)(npx / 2 + 6);
				sy = (int)(npy / 2 + 10);
				temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
				bgc = *temp;

				if (bgc > 0) {
					_npcinfo[i].x = opx;
					_npcinfo[i].y = opy;
				}
			}
		}
	}

	_player.opx = _player.px;
	_player.opy = _player.py;
	_player.px = px;
	_player.py = py;

	if (_player.px != _player.opx || _player.py != _player.opy)
		_player.walkFrame += _animspd * _fpsr;
	if (_player.walkFrame >= 16)
		_player.walkFrame -= 16;

	// walking over items to pickup :::
	int o = _objectMap[lx][ly];

	if (o > -1) {
		// fsk
		if (_objectInfo[o][4] == 2 && _player.inventory[kInvFlask] < 9) {
			_objectMap[lx][ly] = -1;

			_player.inventory[kInvFlask]++;
			addFloatIcon(6, lx * 16, ly * 16);

			_objmapf[_curmap][lx][ly] = 1;

			if (config.effects) {
				int snd = playSound(_sfx[kSndPowerUp]);
				setChannelVolume(snd, config.effectsvol);
			}
		}

		if (_objectInfo[o][5] == 7 && _player.inventory[kInvDoubleFlask] < 9) {
			_objectMap[lx][ly] = -1;

			_player.inventory[kInvDoubleFlask]++;
			addFloatIcon(12, lx * 16, ly * 16);

			_objmapf[_curmap][lx][ly] = 1;

			if (config.effects) {
				int snd = playSound(_sfx[kSndPowerUp]);
				setChannelVolume(snd, config.effectsvol);
			}
		}

		if (_objectInfo[o][5] == 9 && _player.inventory[kInvShock] < 9 && (_curmap == 41 && _scriptflag[kScriptLightningBomb][1] == 0)) {
			_objectMap[lx][ly] = -1;

			_player.inventory[kInvShock]++;
			addFloatIcon(17, lx * 16, ly * 16);

			_objmapf[_curmap][lx][ly] = 1;
			if (_curmap == 41)
				_scriptflag[kScriptLightningBomb][1] = 1;

			if (config.effects) {
				int snd = playSound(_sfx[kSndPowerUp]);
				setChannelVolume(snd, config.effectsvol);
			}

		}

		if (_objectInfo[o][5] == 9 && _player.inventory[kInvShock] < 9) {
			_objectMap[lx][ly] = -1;

			_player.inventory[kInvShock]++;
			addFloatIcon(17, lx * 16, ly * 16);

			_objmapf[_curmap][lx][ly] = 1;

			if (config.effects) {
				int snd = playSound(_sfx[kSndPowerUp]);
				setChannelVolume(snd, config.effectsvol);
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

	if (_triggerloc[lx][ly] > -1)
		processTrigger(_triggerloc[lx][ly]);
}

void GriffonEngine::processTrigger(int trignum) {
	int trigtype = _triggers[trignum][0];

	if (_roomlock)
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
					setChannelVolume(snd, config.effectsvol);
				}

				loadMap(tmap);
				swash();
			}
		}
	}

	for (int i = 0; i < kMaxFloat; i++) {
		_floattext[i][0] = 0;
		_floaticon[i][0] = 0;
	}
}

} // end of namespace Griffon
