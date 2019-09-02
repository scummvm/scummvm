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

#include "common/file.h"
#include "common/system.h"

#include "image/bmp.h"

namespace Griffon {

// memo
/*
 monsters
 1 - baby dragon
 2 - one wing
 3 - boss 1
 4 - black knight
 5 - fire hydra
 6 - red dragon
 7 - priest
 8 - yellow fire dragon
 9 - two wing
10 - dragon2
11 - final boss
12 - bat kitty

 chests
  0 - regular flask
 11 - key chest
 14 - blue flask chest
 15 - lightning chest
 16 - armour chest
 17 - citadel master key
 18 - sword3
 19 - shield3
 20 - armour3

*/

// element tile locations
const int elementmap[15][20] = {
	{  2, 2, 2, 2, -1, -1, -1, 2, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1 },
	{  2, -1, -1, -1, -1, -1, -1, 2, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1 },
	{  2, -1, 2, 2, -1, -1, -1, 2, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1 },
	{  2, -1, 2, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{  2, 2, 2, 2, 2, -1, -1, -1, 2, -1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, 2, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, 0, 0, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
};

// HELPER MACRO ---------------------------------------------------------------
#define INPUT(A, B)                 \
	do {                            \
		Common::String line;        \
		line = file.readLine();     \
		sscanf(line.c_str(), A, B); \
	} while(0)

// CODE GOES HERE -------------------------------------------------------------

Graphics::TransparentSurface *GriffonEngine::loadImage(const char *name, bool colorkey) {
	Common::File file;

	file.open(name);
	if (!file.isOpen()) {
		error("Cannot open file %s", name);
	}

	debug(1, "Loading: %s", name);

	Image::BitmapDecoder bitmapDecoder;
	bitmapDecoder.loadStream(file);
	file.close();

	Graphics::TransparentSurface *surface = new Graphics::TransparentSurface(*bitmapDecoder.getSurface()->convertTo(g_system->getScreenFormat()));

	if (colorkey)
		surface->applyColorKey(255, 0, 255);

	return surface;
}

// copypaste from hRnd_CRT()
float GriffonEngine::RND() {
	/* return between 0 and 1 (but never 1) */
	return (float)_rnd->getRandomNumber(32767) * (1.0f / 32768.0f);
}

void GriffonEngine::addFloatIcon(int ico, float xloc, float yloc) {
	for (int i = 0; i < kMaxFloat; i++) {
		if (ABS(_floaticon[i][0]) < kEpsilon) {
			_floaticon[i][0] = 32;
			_floaticon[i][1] = xloc;
			_floaticon[i][2] = yloc;
			_floaticon[i][3] = ico;
			return;
		}
	}
}

void GriffonEngine::addFloatText(const char *stri, float xloc, float yloc, int col) {
	for (int i = 0; i < kMaxFloat; i++) {
		if (ABS(_floattext[i][0]) < kEpsilon) {
			_floattext[i][0] = 32;
			_floattext[i][1] = xloc;
			_floattext[i][2] = yloc;
			_floattext[i][3] = col;
			strcpy(_floatstri[i], stri);
			return;
		}
	}
}

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

	if (attacking || (_forcepause && _itemselon == 0))
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
			if (_itemselon == 0 && _itemticks < _ticks)
				attack();

			if (_itemselon == 1 && _itemticks < _ticks) {
				if (_curitem == 0 && _player.inventory[kInvFlask] > 0) {
					_itemticks = _ticks + ntickdelay;

					int heal = 50;
					int maxh = _player.maxhp - _player.hp;

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

					_itemselon = 0;
					_forcepause = false;
				}

				if (_curitem == 1 && _player.inventory[kInvDoubleFlask] > 0) {
					_itemticks = _ticks + ntickdelay;

					int heal = 200;
					int maxh = _player.maxhp - _player.hp;

					if (heal > maxh)
						heal = maxh;

					_player.hp = _player.hp + heal;

					char text[256];
					sprintf(text, "+%i", heal);
					addFloatText(text, _player.px + 16 - 4 * strlen(text), _player.py + 16, 5);

					_player.inventory[kInvDoubleFlask]--;

					if (config.effects) {
						int snd = playSound(_sfx[kSndPowerUp]);
						setChannelVolume(snd, config.effectsvol);
					}

					_itemselon = 0;
					_forcepause = false;
				}

				if (_curitem == 2 && _player.inventory[kInvShock] > 0) {
					castSpell(8, _player.px, _player.py, _npcinfo[_curenemy].x, _npcinfo[_curenemy].y, 0);

					_forcepause = true;

					_player.inventory[kInvShock]--;

					_itemticks = _ticks + ntickdelay;
					_selenemyon = 0;
					_itemselon = 0;

				}

				if (_curitem == 3 && _player.inventory[kInvNormalKey] > 0 && _canUseKey && _lockType == 1) {
					_roomLocks[_roomToUnlock] = 0;
					eventText("UnLocked!");

					_player.inventory[kInvNormalKey]--;

					_itemticks = _ticks + ntickdelay;
					_selenemyon = 0;
					_itemselon = 0;
					return;
				}

				if (_curitem == 4 && _player.inventory[kInvMasterKey] > 0 && _canUseKey && _lockType == 2) {
					_roomLocks[_roomToUnlock] = 0;
					eventText("UnLocked!");

					_player.inventory[kInvMasterKey]--;

					_itemticks = _ticks + ntickdelay;
					_selenemyon = 0;
					_itemselon = 0;
					return;
				}

				if (_curitem == 5 && _player.spellcharge[0] == 100) {
					castSpell(5, _player.px, _player.py, _npcinfo[_curenemy].x, _npcinfo[_curenemy].y, 0);

					_player.spellcharge[0] = 0;

					_forcepause = true;

					_itemticks = _ticks + ntickdelay;
					_selenemyon = 0;
					_itemselon = 0;
				}

				if (_curitem > 5 && _selenemyon == 1) {
					if (_curenemy <= _lastnpc) {
						castSpell(_curitem - 6, _player.px, _player.py, _npcinfo[_curenemy].x, _npcinfo[_curenemy].y, 0);
					} else {
						int pst = _curenemy - _lastnpc - 1;
						castSpell(_curitem - 6, _player.px, _player.py, postinfo[pst][0], postinfo[pst][1], 0);
					}

					_player.spellcharge[_curitem - 5] = 0;

					_player.spellstrength = 0;

					_itemticks = _ticks + ntickdelay;
					_selenemyon = 0;
					_itemselon = 0;
					_forcepause = false;
				}

				if (_curitem > 5 && _selenemyon == 0 && _itemselon == 1) {
					if (ABS(_player.spellcharge[_curitem - 5] - 100) < kEpsilon) {
						_itemticks = _ticks + ntickdelay;

						_selenemyon = 1;

						int i = 0;
						do {
							if (_npcinfo[i].hp > 0) {
								_curenemy = i;
								goto __exit_do;
							}
							i = i + 1;
							if (i == _lastnpc + 1) {
								_selenemyon = 0;
								goto __exit_do;
							}
						} while (1);
__exit_do:

						if (nposts > 0 && _selenemyon == 0) {
							_selenemyon = 1;
							_curenemy = _lastnpc + 1;
						}
					}

				}
			}
		} else if (_event.kbd.hasFlags(Common::KBD_ALT)) {
			if (_itemticks < _ticks) {
				_selenemyon = 0;
				if (_itemselon == 1) {
					_itemselon = 0;
					_itemticks = _ticks + 220;
					_forcepause = false;
				} else {
					_itemselon = 1;
					_itemticks = _ticks + 220;
					_forcepause = true;
					_player.itemselshade = 0;
				}
			}
		}
	}

	if (_itemselon == 0) {
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

		if (_selenemyon == 1) {
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

void GriffonEngine::checkTrigger() {
	int npx = _player.px + 12;
	int npy = _player.py + 20;

	int lx = (int)npx / 16;
	int ly = (int)npy / 16;

	_canUseKey = false;

	if (_triggerloc[lx][ly] > -1)
		processTrigger(_triggerloc[lx][ly]);
}

void GriffonEngine::eventText(const char *stri) {
	_videobuffer2->fillRect(Common::Rect(0, 0, _videobuffer2->w, _videobuffer2->h), 0);
	_videobuffer3->fillRect(Common::Rect(0, 0, _videobuffer3->w, _videobuffer3->h), 0);

	int x = 160 - 4 * strlen(stri);

	_ticks = g_system->getMillis();
	int pause_ticks = _ticks + 500;
	int b_ticks = _ticks;

	_videobuffer->blit(*_videobuffer3);
	_videobuffer->blit(*_videobuffer2);

	do {
		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_KEYDOWN && pause_ticks < _ticks)
			break;
		_videobuffer2->blit(*_videobuffer);

		int fr = 192;

		if (pause_ticks > _ticks)
			fr = 192 * (_ticks - b_ticks) / 500;
		if (fr > 192)
			fr = 192;

		_windowimg->setAlpha(fr, true);

		_windowimg->blit(*_videobuffer);
		if (pause_ticks < _ticks)
			drawString(_videobuffer, stri, x, 15, 0);

		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24.0;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		g_system->delayMillis(10);
	} while (1);

	_videobuffer3->blit(*_videobuffer);

	_itemticks = _ticks + 210;
}


void GriffonEngine::handleWalking() {
	int xmax = 20 * 16 - 25;
	int ymax = 15 * 16 - 25;

	float px = _player.px;
	float py = _player.py;
	float opx = px;
	float opy = py;

	float spd = _player.walkspd * _fpsr;

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
		_player.walkdir = 0;
	if (movingdown)
		_player.walkdir = 1;
	if (movingleft)
		_player.walkdir = 2;
	if (movingright)
		_player.walkdir = 3;

	if (movingup && _clipsurround[1][0] == 0) {
		py -= spd;
		_player.walkdir = 0;
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
		_player.walkdir = 1;
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
		_player.walkdir = 2;
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
		_player.walkdir = 3;
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

				if (_player.walkdir == 0) {
					if (abs(xdif) <= 8 && ydif > 0 && ydif < 8)
						_npcinfo[i].y -= spd;
				} else if (_player.walkdir == 1) {
					if (abs(xdif) <= 8 && ydif < 0 && ydif > -8)
						_npcinfo[i].y += spd;
				} else if (_player.walkdir == 2) {
					if (abs(ydif) <= 8 && xdif > 0 && xdif < 8)
						_npcinfo[i].x -= spd;
				} else if (_player.walkdir == 3) {
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
		_player.walkframe += _animspd * _fpsr;
	if (_player.walkframe >= 16)
		_player.walkframe -= 16;

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

void GriffonEngine::loadMap(int mapnum) {
	debug(2, "Loaded map %d", mapnum);

	unsigned int ccc = _clipbg->format.RGBToColor(255, 255, 255);

	_curmap = mapnum;
	Common::Rect trect(320, 240);

	_mapbg->fillRect(trect, 0);
	_clipbg->fillRect(trect, ccc);
	_clipbg2->fillRect(trect, ccc);

	_forcepause = false;
	cloudson = 0;
	if (mapnum < 6)
		cloudson = 1;
	if (mapnum > 41)
		cloudson = 1;
	if (mapnum > 47)
		cloudson = 0;
	if (mapnum == 52)
		cloudson = 1;
	if (mapnum == 60)
		cloudson = 1;
	if (mapnum == 50)
		cloudson = 1;
	if (mapnum == 54)
		cloudson = 1;
	if (mapnum == 58)
		cloudson = 1;
	if (mapnum == 62)
		cloudson = 1;
	if (mapnum == 83)
		cloudson = 1;

	// -----------special case
	dontdrawover = 0;
	if (mapnum == 24)
		dontdrawover = 1;

	if ((mapnum == 53 || mapnum == 57 || mapnum == 61 || mapnum == 65 || mapnum == 62) && _scriptflag[kScriptLever][0] > 0)
		mapnum = mapnum + 100;
	if ((mapnum == 161 || mapnum == 162) && _scriptflag[kScriptLever][0] == 2)
		mapnum = mapnum + 100;

	for (int i = 0; i < kMaxSpell; i++)
		spellinfo[i].frame = 0;

	_roomlock = false;

	char name[256];
	// read *.map file
	sprintf(name, "mapdb/%04i.map", mapnum);
	debug(1, "Reading %s", name);

	Common::File file;
	file.open(name);

	if (!file.isOpen())
		error("Cannot open file %s", name);

	int tempmap[320][200];
	for (int x = 0; x <= 319; x++) {
		for (int y = 0; y <= 199; y++)
			INPUT("%i", &tempmap[x][y]);
	}
	file.close();

	for (int x = 0; x <= 319; x++) {
		for (int y = 0; y <= 239; y++)
			_triggerloc[x][y] = -1;
	}

	// read *.trg file
	sprintf(name, "mapdb/%04i.trg", mapnum);
	debug(1, "Reading %s", name);
	file.open(name);

	if (!file.isOpen())
		error("Cannot open file %s", name);

	INPUT("%i", &_ntriggers);

	for (int i = 0; i < _ntriggers; i++) {
		int mapx, mapy, trig;

		INPUT("%i", &mapx);
		INPUT("%i", &mapy);
		INPUT("%i", &trig);

		_triggerloc[mapx][mapy] = trig;
	}
	file.close();


	for (int y = 0; y <= 23; y++) {
		for (int x = 0; x <= 39; x++)
			_rampdata[x][y] = tempmap[3 * 40 + x][y + 40];
	}

	for (int y = 0; y <= 23; y++) {
		for (int x = 0; x <= 39; x++) {
			for (int l = 0; l <= 2; l++) {
				for (int a = 0; a <= 2; a++)
					_tileinfo[l][x][y][a] = 0;
			}
		}
	}

	if (_scriptflag[kScriptFindShield][0] == 1 && _curmap == 4) {
		_triggerloc[9][7] = 5004;
		tempmap[9][7] = 41;
		tempmap[9][7 + 40] = 0;
	}

	for (int y = 0; y <= 23; y++) {
		for (int x = 0; x <= 39; x++) {
			for (int l = 0; l <= 2; l++) {
				int ly = y;
				int lx = x + l * 40;

				// tile
				int curtile = tempmap[lx][ly];
				int curtilelayer = tempmap[lx][ly + 40];

				if (curtile > 0) {
					curtile = curtile - 1;
					int curtilel = curtilelayer;
					int curtilex = curtile % 20;
					int curtiley = (curtile - curtilex) / 20;

					_tileinfo[l][x][y][0] = curtile + 1;
					_tileinfo[l][x][y][1] = curtilelayer;

					rcSrc.left = curtilex * 16;
					rcSrc.top = curtiley * 16;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					rcDest.left = x * 16;
					rcDest.top = y * 16;
					rcDest.setWidth(16);
					rcDest.setHeight(16);

					if (l == 2 && curtilel == 1) {
						for (int ff = 0; ff <= 5; ff++) {
							int ffa = 20 * 5 - 1 + ff * 20;
							int ffb = 20 * 5 + 4 + ff * 20;
							if (curtile > ffa && curtile < ffb) {
								_tiles[curtilel]->setAlpha(128, true);
							}
						}
					}
					if (l == 1 && curtilel == 2) {
						for (int ff = 0; ff <= 4; ff++) {
							int ffa = 20 * (5 + ff) + 3;
							if (curtile == ffa) {
								_tiles[curtilel]->setAlpha(192, true);
							}
						}
					}

					_tiles[curtilel]->blit(*_mapbg, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					_tiles[curtilel]->setAlpha(255, true);

					rcDest.left = x * 8;
					rcDest.top = y * 8;
					rcDest.setWidth(8);
					rcDest.setHeight(8);

					_clipbg->fillRect(rcDest, 0);
				}
			}
		}
	}

	for (int x = 0; x <= 39; x++) {
		for (int y = 0; y <= 23; y++) {
			int d = tempmap[3 * 40 + x][y];

			if (_scriptflag[kScriptFindShield][0] == 1 && x == 9 && y == 7)
				d = 99;

			if (d > 0) {
				int clip = d % 2;
				d = (d - clip) / 2;
				int npc = d % 2;
				d = (d - npc) / 2;

				if (d == 99 && x == 9 && y == 7)
					clip = 1;

				if (clip) {
					if (d != 99)
						d = tempmap[6 * 40 + x][y];
					if (d == 99)
						d = 1;

					int x1 = x * 8;
					int y1 = y * 8;

					if (d == 1) {
						for (int i = 0; i <= 7; i++) {
							drawLine(_clipbg, x1, y1 + i, x1 + 7 - i, y1 + i, ccc);
						}
					} else if (d == 2) {
						drawLine(_clipbg, x1, y1, x1 + 7, y1, ccc);
						drawLine(_clipbg, x1, y1 + 1, x1 + 7, y1 + 1, ccc);
					} else if (d == 3) {
						for (int i = 0; i <= 7; i++) {
							drawLine(_clipbg, x1 + i, y1 + i, x1 + 7, y1 + i, ccc);
						}
					} else if (d == 4) {
						drawLine(_clipbg, x1, y1, x1, y1 + 7, ccc);
						drawLine(_clipbg, x1 + 1, y1, x1 + 1, y1 + 7, ccc);
					} else if (d == 5) {
						rcDest.left = x1;
						rcDest.top = y1;
						rcDest.setWidth(8);
						rcDest.setHeight(8);
						_clipbg->fillRect(rcDest, ccc);
					} else if (d == 6) {
						drawLine(_clipbg, x1 + 7, y1, x1 + 7, y1 + 7, ccc);
						drawLine(_clipbg, x1 + 6, y1, x1 + 6, y1 + 7, ccc);
					} else if (d == 7) {
						for (int i = 0; i <= 7; i++) {
							drawLine(_clipbg, x1, y1 + i, x1 + i, y1 + i, ccc);
						}
					} else if (d == 8) {
						drawLine(_clipbg, x1, y1 + 7, x1 + 7, y1 + 7, ccc);
						drawLine(_clipbg, x1, y1 + 7, x1 + 6, y1 + 6, ccc);
					} else if (d == 9) {
						for (int i = 0; i <= 7; i++) {
							drawLine(_clipbg, x1 + 7 - i, y1 + i, x1 + 7, y1 + i, ccc);
						}
					}
				}
			}
		}
	}

	_lastObj = 0;
	_lastnpc = 0;

	for (int i = 0; i < kMaxNPC; i++)
		_npcinfo[i].onmap = 0;

	for (int x = 0; x <= 19; x++) {
		for (int y = 0; y <= 19; y++) {
			int d = tempmap[3 * 40 + x][y];

			int npc = 0;
			int obj = 0;
			if (d > 0) {
				int clip = d % 2;
				d = (d - clip) / 2;
				npc = d % 2;
				d = (d - npc) / 2;
				obj = d % 2;
			}

			_objectMap[x][y] = -1;

			if (obj == 1) {

				int o = tempmap[5 * 40 + x][y];

				if (_objmapf[_curmap][x][y] == 0) {
					_objectMap[x][y] = o;

					if (_objectInfo[o][0] > 1) {
						if (o > _lastObj)
							_lastObj = o;
					}

					int x1 = x * 8;
					int y1 = y * 8;

					rcDest.left = x1;
					rcDest.top = y1;
					rcDest.setWidth(8);
					rcDest.setHeight(8);

					if (_objectInfo[o][4] == 1)
						_clipbg->fillRect(rcDest, ccc);
					if (_objectInfo[o][4] == 3)
						_clipbg->fillRect(rcDest, ccc);
				}
			}
			if (npc == 1) {
				int o = tempmap[4 * 40 + x][y];

				if (o > _lastnpc)
					_lastnpc = o;

				_npcinfo[o].x = x * 16 - 4;
				_npcinfo[o].y = y * 16 - 5;

				_npcinfo[o].walkdir = 1;
				_npcinfo[o].onmap = 1;
			}
		}
	}


	if (_curmap == 62 && _scriptflag[kScriptGardenMasterKey][0] > 0)
		_lastnpc = 0;
	if (_curmap == 73 && _scriptflag[kScriptArmourChest][0] > 0)
		_lastnpc = 0;
	if (_curmap == 81 && _scriptflag[kScriptCitadelMasterKey][0] > 0)
		_lastnpc = 0;

	if (_curmap == 73 && _scriptflag[kScriptArmourChest][0] == 0)
		_roomlock = true;
	if (_curmap == 81 && _scriptflag[kScriptCitadelMasterKey][0] == 0)
		_roomlock = true;
	if (_curmap == 83 && _scriptflag[kScriptGetSword3][0] == 0)
		_roomlock = true;
	if (_curmap == 82)
		_roomlock = true;

	// read *.npc file
	sprintf(name, "mapdb/%04i.npc", mapnum);
	debug(1, "Reading %s", name);
	file.open(name);

	if (!file.isOpen())
		error("Cannot open file %s", name);

	for (int i = 0; i < kMaxNPC; i++) {
		INPUT("%i", &_npcinfo[i].spriteset);
		INPUT("%i", &_npcinfo[i].x1);
		INPUT("%i", &_npcinfo[i].y1);
		INPUT("%i", &_npcinfo[i].x2);
		INPUT("%i", &_npcinfo[i].y2);
		INPUT("%i", &_npcinfo[i].movementmode);
		INPUT("%i", &_npcinfo[i].hp);
		INPUT("%i", &_npcinfo[i].item1);
		INPUT("%i", &_npcinfo[i].item2);
		INPUT("%i", &_npcinfo[i].item3);
		INPUT("%i", &_npcinfo[i].script);

		// baby dragon
		if (_npcinfo[i].spriteset == 1) {
			_npcinfo[i].hp = 12;
			_npcinfo[i].attackdelay = 2000;

			_npcinfo[i].attackdamage = 2;
			_npcinfo[i].spelldamage = 0;

			_npcinfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				_npcinfo[i].hp = 0;
		}

		// onewing
		if (_npcinfo[i].spriteset == 2) {
			_npcinfo[i].hp = 200;
			_npcinfo[i].attackdelay = 2000;
			_npcinfo[i].swayspd = 1;

			_npcinfo[i].attackdamage = 24;
			_npcinfo[i].spelldamage = 30;

			_npcinfo[i].walkspd = 1.4f;
			_npcinfo[i].castpause = _ticks;
		}

		// boss1
		if (_npcinfo[i].spriteset == 3) {
			_npcinfo[i].hp = 300;
			_npcinfo[i].attackdelay = 2200;

			_npcinfo[i].attackdamage = 0;
			_npcinfo[i].spelldamage = 30;

			_npcinfo[i].walkspd = 1.2f;
		}

		// black knights
		if (_npcinfo[i].spriteset == 4) {
			_npcinfo[i].hp = 200;
			_npcinfo[i].attackdelay = 2800;

			_npcinfo[i].attackdamage = 0;
			_npcinfo[i].spelldamage = 30;

			_npcinfo[i].walkspd = 1;
		}

		// boss2 firehydra
		if (_npcinfo[i].spriteset == 5) {
			_npcinfo[i].hp = 600;
			_npcinfo[i].attackdelay = 2200;

			_npcinfo[i].attackdamage = 50;
			_npcinfo[i].spelldamage = 30;

			_npcinfo[i].walkspd = 1.3f;

			_npcinfo[i].swayangle = 0;
		}

		// baby fire dragon
		if (_npcinfo[i].spriteset == 6) {
			_npcinfo[i].hp = 20;
			_npcinfo[i].attackdelay = 1500;

			_npcinfo[i].attackdamage = 0;
			_npcinfo[i].spelldamage = 12;

			_npcinfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				_npcinfo[i].hp = 0;
		}

		// priest1
		if (_npcinfo[i].spriteset == 7) {
			_npcinfo[i].hp = 40;
			_npcinfo[i].attackdelay = 5000;

			_npcinfo[i].attackdamage = 0;
			_npcinfo[i].spelldamage = 8;

			_npcinfo[i].walkspd = 1;

			if (RND() * 8 == 0)
				_npcinfo[i].hp = 0;
		}

		// yellow fire dragon
		if (_npcinfo[i].spriteset == 8) {
			_npcinfo[i].hp = 100;
			_npcinfo[i].attackdelay = 1500;

			_npcinfo[i].attackdamage = 0;
			_npcinfo[i].spelldamage = 24;

			_npcinfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				_npcinfo[i].hp = 0;
		}

		// twowing
		if (_npcinfo[i].spriteset == 9) {
			_npcinfo[i].hp = 140;
			_npcinfo[i].attackdelay = 2000;
			_npcinfo[i].swayspd = 1;

			_npcinfo[i].attackdamage = 30;
			_npcinfo[i].spelldamage = 0;

			_npcinfo[i].walkspd = 1;

			_npcinfo[i].castpause = 0;
		}

		// dragon2
		if (_npcinfo[i].spriteset == 10) {
			_npcinfo[i].hp = 80;
			_npcinfo[i].attackdelay = 1500;

			_npcinfo[i].attackdamage = 24;
			_npcinfo[i].spelldamage = 0;

			_npcinfo[i].walkspd = 1;

			_npcinfo[i].floating = RND() * 16;
		}

		// end boss
		if (_npcinfo[i].spriteset == 11) {
			_npcinfo[i].hp = 1200;
			_npcinfo[i].attackdelay = 2000;

			_npcinfo[i].attackdamage = 100;
			_npcinfo[i].spelldamage = 60;

			_npcinfo[i].walkspd = 1;

			_npcinfo[i].floating = RND() * 16;
		}

		// bat kitty
		if (_npcinfo[i].spriteset == 12) {
			_npcinfo[i].hp = 800;
			_npcinfo[i].attackdelay = 2000;

			_npcinfo[i].attackdamage = 100;
			_npcinfo[i].spelldamage = 50;

			_npcinfo[i].walkspd = 1;

			_npcinfo[i].floating = RND() * 16;
		}

		if (_npcinfo[i].onmap == 0)
			_npcinfo[i].hp = 0;

		_npcinfo[i].maxhp = _npcinfo[i].hp;

		_npcinfo[i].attacking = 0;
		_npcinfo[i].attackframe = 0;
		_npcinfo[i].cattackframe = 0;
		_npcinfo[i].attackspd = 1.5;
		_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay * (1 + RND() * 2);

		if (_npcinfo[i].spriteset == 2 || _npcinfo[i].spriteset == 9) {
			_npcinfo[i].bodysection[0].sprite = 0;
			_npcinfo[i].bodysection[1].sprite = 1;
			_npcinfo[i].bodysection[2].sprite = 2;
			_npcinfo[i].bodysection[3].sprite = 3;
			_npcinfo[i].bodysection[4].sprite = 4;
			_npcinfo[i].bodysection[5].sprite = 3;
			_npcinfo[i].bodysection[6].sprite = 3;
			_npcinfo[i].bodysection[7].sprite = 5;

			_npcinfo[i].bodysection[0].bonelength = 8;
			_npcinfo[i].bodysection[1].bonelength = 7;
			_npcinfo[i].bodysection[2].bonelength = 6;
			_npcinfo[i].bodysection[3].bonelength = 4;
			_npcinfo[i].bodysection[4].bonelength = 4;
			_npcinfo[i].bodysection[5].bonelength = 4;
			_npcinfo[i].bodysection[6].bonelength = 4;

			for (int f = 0; f <= 7; f++) {
				_npcinfo[i].bodysection[f].x = _npcinfo[i].x + 12;
				_npcinfo[i].bodysection[f].y = _npcinfo[i].y + 14;
			}

			_npcinfo[i].headtargetx[0] = _npcinfo[i].x + 12;
			_npcinfo[i].headtargety[0] = _npcinfo[i].y + 14;

		}

		if (_npcinfo[i].spriteset == 5) {
			for (int f = 0; f <= 29; f++) {
				_npcinfo[i].bodysection[f].x = _npcinfo[i].x + 12;
				_npcinfo[i].bodysection[f].y = _npcinfo[i].y + 14;
			}

			for (int f = 0; f <= 2; f++) {
				_npcinfo[i].headtargetx[f] = _npcinfo[i].x + 12;
				_npcinfo[i].headtargety[f] = _npcinfo[i].y + 14;

				_npcinfo[i].attacking2[f] = 0;
				_npcinfo[i].attackframe2[f] = 0;
			}
		}

		if (_npcinfo[i].script == kScriptMasterKey) {
			_roomlock = true;
			if (_scriptflag[kScriptMasterKey][0] > 0) {
				_roomlock = false;
				_npcinfo[i].hp = 0;
			}
		}

		if (_npcinfo[i].script == kScriptFindCtystal) {
			_roomlock = true;
			if (_scriptflag[kScriptFindCtystal][0] > 0) {
				_roomlock = false;
				_npcinfo[i].hp = 0;
			}
		}

		if (_npcinfo[i].script == kScriptFindSword) {
			_roomlock = true;
			if (_scriptflag[kScriptFindSword][0] > 0) {
				_roomlock = false;
				_npcinfo[i].hp = 0;
			}
		}

		if (_npcinfo[i].script == kScriptGetSword3) {
			_roomlock = true;
			if (_scriptflag[kScriptGetSword3][0] > 0) {
				_roomlock = false;
				_npcinfo[i].hp = 0;
			}
		}

		_npcinfo[i].pause = _ticks;
	}

	file.close();


	int cx, cy, npx, npy, lx, ly;

	// academy master key
	if (_curmap == 34 && _scriptflag[kScriptMasterKey][0] == 1) {
		cx = 9;
		cy = 7;

		_objectMap[cx][cy] = 5;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	// academy crystal
	if (_curmap == 24 && _player.foundspell[0] == 0 && _scriptflag[kScriptFindCtystal][0] == 1) {
		cx = 9;
		cy = 7;

		_objectMap[cx][cy] = 6;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	// gardens master key
	if (_curmap == 62 && _scriptflag[kScriptGardenMasterKey][0] == 1) {
		cx = 13;
		cy = 7;

		_objectMap[cx][cy] = 5;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	// gardens fidelis sword
	if (_curmap == 66 && _scriptflag[kScriptFindSword][0] == 1 && _player.sword == 1) {
		cx = 9;
		cy = 6;

		_objectMap[cx][cy] = 9;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	// citadel armour
	if (_curmap == 73 && _scriptflag[kScriptArmourChest][0] == 1 && _player.armour == 1) {
		cx = 8;
		cy = 7;

		_objectMap[cx][cy] = 16;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	// citadel master key
	if (_curmap == 81 && _scriptflag[kScriptCitadelMasterKey][0] == 1) {
		cx = 11;
		cy = 10;

		_objectMap[cx][cy] = 5;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}


	// max ups
	if (_curmap == 83 && _scriptflag[kScriptGetSword3][0] == 1 && _player.sword < 3) {
		cx = 6;
		cy = 8;

		_objectMap[cx][cy] = 18;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	if (_curmap == 83 && _scriptflag[kScriptShield3][0] == 1 && _player.shield < 3) {
		cx = 9;
		cy = 8;

		_objectMap[cx][cy] = 19;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	if (_curmap == 83 && _scriptflag[kScriptArmour3][0] == 1 && _player.armour < 3) {
		cx = 12;
		cy = 8;

		_objectMap[cx][cy] = 20;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	_clipbg2->copyRectToSurface(_clipbg->getPixels(), _clipbg->pitch, 0, 0, _clipbg->w, _clipbg->h);
}

void GriffonEngine::newGame() {
	intro();

	if (_shouldQuit)
		return;

	_player.px = 0;
	_player.py = 0;
	_player.opx = 0;
	_player.opy = 0;
	_player.walkdir = 0;
	_player.walkframe = 0;
	_player.walkspd = 0;
	_player.attackframe = 0;
	_player.attackspd = 0;
	_player.hp = 0;
	_player.maxhp = 0;
	_player.hpflash = 0;
	_player.level = 0;
	_player.maxlevel = 0;
	_player.sword = 0;
	_player.shield = 0;
	_player.armour = 0;
	for (int i = 0; i < 5; i++) {
		_player.foundspell[i] = 0;
		_player.spellcharge[i] = 0;
		_player.inventory[i] = 0;
	}
	_player.attackstrength = 0;
	_player.spelldamage = 0;
	_player.sworddamage = 0;
	_player.exp = 0;
	_player.nextlevel = 0;

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

	_player.walkspd = 1.1f;
	_animspd = 0.5f;
	attacking = false;
	_player.attackspd = 1.5f;

	_player.sword = 1;
	_player.level = 1;
	_player.maxlevel = 22;
	_player.nextlevel = 50;
	_player.shield = 1;
	_player.armour = 1;
	_player.hp = 14;
	_player.maxhp = _player.hp;

	_player.sworddamage = _player.level * 2;
	_player.spelldamage = _player.level * 3 / 2;

	_player.px = 15 * 16 - 4;
	_player.py = 6 * 16 - 4;
	_player.walkdir = 1;

	_pgardens = false;
	_ptown = false;
	_pboss = false;
	_pacademy = false;
	_pcitadel = false;

	loadMap(2);

	mainLoop();
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

void GriffonEngine::updateAnims() {
	for (int i = 0; i <= _lastObj; i++) {
		int nframes = _objectInfo[i][0];
		int o_animspd = _objectInfo[i][3];
		float frame = _objectFrame[i][0];
		int cframe = _objectFrame[i][1];
		// _objectinfo[i][6] = 0; // ?? out of bounds

		if (nframes > 1) {
			frame = frame + o_animspd / 50 * _fpsr;
			while (frame >= nframes)
				frame = frame - nframes;

			cframe = (int)frame; // truncate fractional part
			if (cframe > nframes)
				cframe = nframes - 1;
			if (cframe < 0)
				cframe = 0;

			_objectFrame[i][0] = frame;
			_objectFrame[i][1] = cframe;
		}
	}
}

void GriffonEngine::updateY() {
	for (int i = 0; i <= 2400; i++)
		_ysort[i] = -1;

	int ff = (int)(_player.py * 10);
	if (ff < 0) // HACKFIX or _ysort[yy] may go out of bounds
		ff = 0;
	_player.ysort = ff;
	_ysort[ff] = 0;

	_firsty = 2400;
	_lasty = 0;

	for (int i = 1; i <= _lastnpc; i++) {
		int yy = (int)(_npcinfo[i].y * 10);

		do {
			if (_ysort[yy] == -1 || yy == 2400)
				break;
			yy = yy + 1;
		} while (1);

		_ysort[yy] = i;
		if (yy < _firsty)
			_firsty = yy;
		if (yy > _lasty)
			_lasty = yy;
	}
}

void GriffonEngine::updateNPCs() {
	for (int i = 1; i <= _lastnpc; i++) {
		if (_npcinfo[i].hp > 0) {
			//  is npc walking
			int pass = 0;
			if (_npcinfo[i].attacking == 0)
				pass = 1;
			if (_npcinfo[i].spriteset == 5)
				pass = 1;
			if (pass == 1) {
				int moveup = 0;
				int movedown = 0;
				int moveleft = 0;
				int moveright = 0;

				float npx = _npcinfo[i].x;
				float npy = _npcinfo[i].y;

				float onpx = npx;
				float onpy = npy;

				float wspd = _npcinfo[i].walkspd / 4;

				if (_npcinfo[i].spriteset == 10)
					wspd = wspd * 2;
				int wdir = _npcinfo[i].walkdir;

				int mode = _npcinfo[i].movementmode;

				float xdif = _player.px - npx;
				float ydif = _player.py - npy;

				if (abs(xdif) < 4 * 16 && abs(ydif) < 4 * 16 && mode < 3)
					mode = 0;
				if (_npcinfo[i].hp < _npcinfo[i].maxhp * 0.25)
					mode = 3;

				if (_npcinfo[i].pause > _ticks)
					mode = -1;
				if (_npcinfo[i].spriteset == 2 && _npcinfo[i].castpause > _ticks)
					mode = -1;

				if (mode == 3) {
					mode = 1;
					if (abs(xdif) < 4 * 16 && abs(ydif) < 4 * 16)
						mode = 3;
				}

				bool checkpass = false;

				// npc  AI CODE
				// --------------

				// *** aggressive
				if (mode == 0) {
					wspd = _npcinfo[i].walkspd / 2;

					xdif = _player.px - npx;
					ydif = _player.py - npy;

					if (abs(xdif) > abs(ydif)) {
						if (xdif < 4)
							wdir = 2;
						if (xdif > -4)
							wdir = 3;
					} else {
						if (ydif < 4)
							wdir = 0;
						if (ydif > -4)
							wdir = 1;
					}

					if (xdif < 4)
						moveleft = 1;
					if (xdif > -4)
						moveright = 1;
					if (ydif < 4)
						moveup = 1;
					if (ydif > -4)
						movedown = 1;
				}
				// *******************

				// *** defensive
				if (mode == 1) {

					int movingdir = _npcinfo[i].movingdir;

					if (_npcinfo[i].ticks > _ticks + 100000)
						_npcinfo[i].ticks = _ticks;

					if (_npcinfo[i].ticks < _ticks) {
						_npcinfo[i].ticks = _ticks + 2000;
						movingdir = (int)(RND() * 8);
						_npcinfo[i].movingdir = movingdir;
					}

					if (movingdir == 0) {
						wdir = 2; // left
						moveup = 1;
						moveleft = 1;
					} else if (movingdir == 1) {
						wdir = 0; // up
						moveup = 1;
					} else if (movingdir == 2) {
						wdir = 3; // right
						moveup = 1;
						moveright = 1;
					} else if (movingdir == 3) {
						wdir = 3; // right
						moveright = 1;
					} else if (movingdir == 4) {
						wdir = 3; // right
						moveright = 1;
						movedown = 1;
					} else if (movingdir == 5) {
						wdir = 1; // down
						movedown = 1;
					} else if (movingdir == 6) {
						wdir = 2; // left
						movedown = 1;
						moveleft = 1;
					} else if (movingdir == 7) {
						wdir = 2; // left
						moveleft = 1;
					}

					checkpass = true;
				}
				// *******************

				// *** run away
				if (mode == 3) {
					wspd = _npcinfo[i].walkspd / 2;

					xdif = _player.px - npx;
					ydif = _player.py - npy;

					if (abs(xdif) > abs(ydif)) {
						if (xdif < 4)
							wdir = 3;
						if (xdif > -4)
							wdir = 2;
					} else {
						if (ydif < 4)
							wdir = 1;
						if (ydif > -4)
							wdir = 0;
					}

					if (xdif < 4)
						moveright = 1;
					if (xdif > -4)
						moveleft = 1;
					if (ydif < 4)
						movedown = 1;
					if (ydif > -4)
						moveup = 1;
				}
				// *******************

				// -------------- ?? move*** vs movin***
				int movinup = 0;
				int movindown = 0;
				int movinleft = 0;
				int movinright = 0;

				float xp = (npx / 2 + 6);
				float yp = (npy / 2 + 10);

				if (_npcinfo[i].spriteset == 10)
					wspd = wspd * 2;

				float ii = wspd * _fpsr;
				if (ii < 1)
					ii = 1;

				if (moveup) {
					int sx = xp;
					int sy = yp - ii;
					uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (_npcinfo[i].spriteset == 11)
						dq = 0;

					if (dq == 0)
						movinup = 1;
					if (dq > 0) {
						sx = xp - ii;
						sy = yp - ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinup = 1;
							movinleft = 1;
						}
					}
					if (dq > 0) {
						sx = xp + ii;
						sy = yp - ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinup = 1;
							movinright = 1;
						}
					}
				}

				if (movedown) {
					int sx = xp;
					int sy = yp + ii;
					uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (_npcinfo[i].spriteset == 11)
						dq = 0;
					if (dq == 0)
						movindown = 1;
					if (dq > 0) {
						sx = xp - ii;
						sy = yp + ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movindown = 1;
							movinleft = 1;
						}
					}
					if (dq > 0) {
						sx = xp + ii;
						sy = yp + ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movindown = 1;
							movinright = 1;
						}
					}
				}

				if (moveleft) {
					int sx = xp - ii;
					int sy = yp;
					uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (_npcinfo[i].spriteset == 11)
						dq = 0;
					if (dq == 0)
						movinleft = 1;
					if (dq > 0) {
						sx = xp - ii;
						sy = yp - ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinleft = 1;
							movinup = 1;
						}
					}
					if (dq > 0) {
						sx = xp - ii;
						sy = yp + ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinleft = 1;
							movindown = 1;
						}
					}
				}

				if (moveright) {
					int sx = xp + ii;
					int sy = yp;
					uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (_npcinfo[i].spriteset == 11)
						dq = 0;
					if (dq == 0)
						movinright = 1;
					if (dq > 0) {
						sx = xp + ii;
						sy = yp - ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinright = 1;
							movinup = 1;
						}
					}
					if (dq > 0) {
						sx = xp + ii;
						sy = yp + ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinright = 1;
							movindown = 1;
						}
					}
				}

				if (movinup)
					npy = npy - wspd * _fpsr;
				if (movindown)
					npy = npy + wspd * _fpsr;
				if (movinleft)
					npx = npx - wspd * _fpsr;
				if (movinright)
					npx = npx + wspd * _fpsr;

				if (checkpass) {
					pass = 0;
					if (npx >= _npcinfo[i].x1 * 16 - 8 && npx <= _npcinfo[i].x2 * 16 + 8 && npy >= _npcinfo[i].y1 * 16 - 8 && npy <= _npcinfo[i].y2 * 16 + 8)
						pass = 1;
					if (pass == 0) {
						npx = onpx;
						npy = onpy;
						_npcinfo[i].ticks = _ticks;
					}
				}

				float aspd = wspd;

				if (_npcinfo[i].spriteset == 10)
					aspd = wspd / 2;

				xp = (npx / 2 + 6);
				yp = (npy / 2 + 10);

				int sx = xp;
				int sy = yp;
				uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
				uint32 bgc = *temp;

				float anpx = npx + 12;
				float anpy = npy + 20;

				int lx = (int)anpx / 16;
				int ly = (int)anpy / 16;

				if (_triggerloc[lx][ly] > -1)
					bgc = 1;
				if (_npcinfo[i].spriteset == 11)
					bgc = 0;

				int rst = 0;

				if (_npcinfo[i].spriteset == 11) {
					if (npx < 40 || npx > 280 || npy < 36 || npy > 204)
						rst = 1;
				}

				if (bgc > 0 || rst == 1) {
					npx = onpx;
					npy = onpy;
				}

				_npcinfo[i].x = npx;
				_npcinfo[i].y = npy;

				_npcinfo[i].walkdir = wdir;
				_npcinfo[i].moving = 0;

				if (npx != onpx || npy != onpy)
					_npcinfo[i].moving = 1;

				if (_npcinfo[i].moving == 1) {
					float frame = _npcinfo[i].frame;
					int cframe = _npcinfo[i].cframe;

					frame = frame + aspd * _fpsr;
					while (frame >= 16)
						frame = frame - 16;

					cframe = (int)(frame);
					if (cframe > 16)
						cframe = 16 - 1;
					if (cframe < 0)
						cframe = 0;

					_npcinfo[i].frame = frame;
					_npcinfo[i].cframe = cframe;
				}

				// spriteset1 specific
				if (_npcinfo[i].spriteset == 1 && _npcinfo[i].attackattempt < _ticks) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && _npcinfo[i].attacking == 0) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 20 && abs(ydif) < 20) {
							_npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								if (config.effects) {
									int snd = playSound(_sfx[kSndEnemyHit]);
									setChannelVolume(snd, config.effectsvol);
								}

								_npcinfo[i].attacking = 1;
								_npcinfo[i].attackframe = 0;
							}
						}
					}
				}

				bool dospell = false;

				// onewing specific
				if (_npcinfo[i].spriteset == 2) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && _npcinfo[i].attacking == 0) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 24 && abs(ydif) < 24) {
							float dist = sqrt(xdif * xdif + ydif * ydif);

							if ((dist) < 24) {
								if (config.effects) {
									int snd = playSound(_sfx[kSndBite]);
									setChannelVolume(snd, config.effectsvol);
								}

								_npcinfo[i].attacking = 1;
								_npcinfo[i].attackframe = 0;

								_npcinfo[i].headtargetx[0] = _player.px + 12;
								_npcinfo[i].headtargety[0] = _player.py - 4;
							}
						}

					}

					dospell = false;

					if (_npcinfo[i].attacking == 0 && _npcinfo[i].castpause < _ticks) {
						_npcinfo[i].swayspd = _npcinfo[i].swayspd + _npcinfo[i].swayspd / 200 * _fpsr;
						if (_npcinfo[i].swayspd > 15) {
							dospell = true;
							_npcinfo[i].swayspd = 1;
						}

						// sway code
						_npcinfo[i].swayangle = _npcinfo[i].swayangle + _npcinfo[i].swayspd * _fpsr;
						if (_npcinfo[i].swayangle >= 360)
							_npcinfo[i].swayangle = _npcinfo[i].swayangle - 360;

						_npcinfo[i].headtargetx[0] = _npcinfo[i].x + (24 - _npcinfo[i].swayspd / 2) * sin(3.14159 / 180 * _npcinfo[i].swayangle) + 12;
						_npcinfo[i].headtargety[0] = _npcinfo[i].y - 36 + 16 + 8 * sin(3.14159 * 2 / 180 * _npcinfo[i].swayangle);
					}

					if (dospell) {
						_npcinfo[i].pause = _ticks + 3000;
						_npcinfo[i].attacknext = _ticks + 4500;
						_npcinfo[i].castpause = _ticks + 4500;

						castSpell(3, _npcinfo[i].x, _npcinfo[i].y, _npcinfo[i].x, _npcinfo[i].y, i);

						_npcinfo[i].headtargetx[0] = _npcinfo[i].x;
						_npcinfo[i].headtargety[0] = _npcinfo[i].y - 36 + 16;
					}

					// targethead code
					xdif = _npcinfo[i].bodysection[7].x - _npcinfo[i].headtargetx[0];
					ydif = _npcinfo[i].bodysection[7].y - _npcinfo[i].headtargety[0];


					_npcinfo[i].bodysection[7].x = _npcinfo[i].bodysection[7].x  - xdif * 0.4 * _fpsr;
					_npcinfo[i].bodysection[7].y = _npcinfo[i].bodysection[7].y  - ydif * 0.4 * _fpsr;


					_npcinfo[i].bodysection[0].x = _npcinfo[i].x + 12;
					_npcinfo[i].bodysection[0].y = _npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = _npcinfo[i].bodysection[f + 1].x - _npcinfo[i].bodysection[f - 1].x;
						ydif = _npcinfo[i].bodysection[f + 1].y - _npcinfo[i].bodysection[f - 1].y;

						float tx = _npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = _npcinfo[i].bodysection[f - 1].y + ydif / 2;

						_npcinfo[i].bodysection[f].x = _npcinfo[i].bodysection[f].x - (_npcinfo[i].bodysection[f].x - tx) / 3;
						_npcinfo[i].bodysection[f].y = _npcinfo[i].bodysection[f].y - (_npcinfo[i].bodysection[f].y - ty) / 3;
					}
				}

				// boss1 specific and blackknight
				if (_npcinfo[i].spriteset == 3 || _npcinfo[i].spriteset == 4) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && _npcinfo[i].attacking == 0) {
						_npcinfo[i].attacking = 1;
						_npcinfo[i].attackframe = 0;

						castSpell(1, _npcinfo[i].x, _npcinfo[i].y, _player.px, _player.py, i);
					}

					if (_npcinfo[i].castpause < _ticks) {
						castSpell(6, _npcinfo[i].x, _npcinfo[i].y, _player.px, _player.py, i);
						_npcinfo[i].castpause = _ticks + 12000;
					}
				}


				// firehydra specific
				if (_npcinfo[i].spriteset == 5) {
					_npcinfo[i].swayspd = 4;

					// sway code
					_npcinfo[i].swayangle = _npcinfo[i].swayangle + _npcinfo[i].swayspd * _fpsr;
					if (_npcinfo[i].swayangle >= 360)
						_npcinfo[i].swayangle = _npcinfo[i].swayangle - 360;

					for (int ff = 0; ff <= 2; ff++) {
						if (_npcinfo[i].hp > 10 * ff * 20) {
							if (_npcinfo[i].pause < _ticks && _npcinfo[i].attacking2[ff] == 0 && _npcinfo[i].attacknext2[ff] < _ticks) {
								npx = _npcinfo[i].x;
								npy = _npcinfo[i].y;

								xdif = _player.px - npx;
								ydif = _player.py - npy;

								if (abs(xdif) < 48 && abs(ydif) < 48) {
									float dist = sqrt(xdif * xdif + ydif * ydif);

									if ((dist) < 36) {
										if (config.effects) {
											int snd = playSound(_sfx[kSndBite]);
											setChannelVolume(snd, config.effectsvol);
										}

										_npcinfo[i].attacking = 1;
										_npcinfo[i].attacking2[ff] = 1;
										_npcinfo[i].attackframe2[ff] = 0;

										_npcinfo[i].headtargetx[ff] = _player.px + 12;
										_npcinfo[i].headtargety[ff] = _player.py - 4;

										_npcinfo[i].swayangle = 0;
									}
								}

							}

							if (_npcinfo[i].attacking2[ff] == 0) {
								_npcinfo[i].headtargetx[ff] = _npcinfo[i].x + 38 * sin(3.14159 / 180 * (_npcinfo[i].swayangle + 120 * ff)) + 12;
								_npcinfo[i].headtargety[ff] = _npcinfo[i].y - 46 + 16 + 16 * sin(3.14159 * 2 / 180 * (_npcinfo[i].swayangle + 120 * ff));
							}

							// targethead code
							xdif = _npcinfo[i].bodysection[10 * ff + 9].x - _npcinfo[i].headtargetx[ff];
							ydif = _npcinfo[i].bodysection[10 * ff + 9].y - _npcinfo[i].headtargety[ff];

							_npcinfo[i].bodysection[10 * ff + 9].x = _npcinfo[i].bodysection[10 * ff + 9].x  - xdif * 0.4 * _fpsr;
							_npcinfo[i].bodysection[10 * ff + 9].y = _npcinfo[i].bodysection[10 * ff + 9].y  - ydif * 0.4 * _fpsr;

							_npcinfo[i].bodysection[10 * ff].x = _npcinfo[i].x + 12 + 8 * cos(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));
							_npcinfo[i].bodysection[10 * ff].y = _npcinfo[i].y + 12 + 8 * sin(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));

							for (int f = 8; f >= 1; f--) {
								xdif = _npcinfo[i].bodysection[ff * 10 + f + 1].x - _npcinfo[i].bodysection[ff * 10 + f - 1].x;
								ydif = _npcinfo[i].bodysection[ff * 10 + f + 1].y - _npcinfo[i].bodysection[ff * 10 + f - 1].y;

								float tx = _npcinfo[i].bodysection[ff * 10 + f - 1].x + xdif / 2;
								float ty = _npcinfo[i].bodysection[ff * 10 + f - 1].y + ydif / 2;

								_npcinfo[i].bodysection[ff * 10 + f].x = _npcinfo[i].bodysection[ff * 10 + f].x - (_npcinfo[i].bodysection[ff * 10 + f].x - tx) / 3;
								_npcinfo[i].bodysection[ff * 10 + f].y = _npcinfo[i].bodysection[ff * 10 + f].y - (_npcinfo[i].bodysection[ff * 10 + f].y - ty) / 3;
							}
						}
					}
				}

				// spriteset6 specific
				if (_npcinfo[i].spriteset == 6 && _npcinfo[i].attackattempt < _ticks) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && _npcinfo[i].attacking == 0) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						pass = 0;
						if (abs(xdif) < 48 && abs(ydif) < 6)
							pass = 1;
						if (abs(ydif) < 48 && abs(xdif) < 6)
							pass = 2;

						if (pass > 0) {
							_npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								_npcinfo[i].attacking = 1;
								_npcinfo[i].attackframe = 0;
								float nnxa = 0, nnya = 0, nnxb = 0, nnyb = 0;

								if (pass == 1 && xdif < 0) {
									nnxa = npx - 8;
									nnya = npy + 4;
									nnxb = npx - 48 - 8;
									nnyb = npy + 4;
								} else if (pass == 1 && xdif > 0) {
									nnxa = npx + 16;
									nnya = npy + 4;
									nnxb = npx + 16 + 48;
									nnyb = npy + 4;
								} else if (pass == 2 && ydif < 0) {
									nnya = npy;
									nnxa = npx + 4;
									nnyb = npy - 48;
									nnxb = npx + 4;
								} else if (pass == 2 && ydif > 0) {
									nnya = npy + 20;
									nnxa = npx + 4;
									nnyb = npy + 20 + 48;
									nnxb = npx + 4;
								}

								castSpell(7, nnxa, nnya, nnxb, nnyb, i);
							}
						}
					}
				}

				// wizard1 specific
				if (_npcinfo[i].spriteset == 7) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && _npcinfo[i].attacking == 0) {
						_npcinfo[i].attacking = 1;
						_npcinfo[i].attackframe = 0;

						castSpell(9, _npcinfo[i].x, _npcinfo[i].y, _player.px, _player.py, i);
					}

					if (_npcinfo[i].castpause < _ticks) {
						// castSpell 6, _npcinfo[i].x, _npcinfo[i].y, _player.px, _player.py, i
						// _npcinfo[i].castpause = _ticks + 12000
					}

				}

				// spriteset6 specific
				if (_npcinfo[i].spriteset == 8 && _npcinfo[i].attackattempt < _ticks) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && _npcinfo[i].attacking == 0) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						pass = 0;
						if (abs(xdif) < 56 && abs(ydif) < 6)
							pass = 1;
						if (abs(ydif) < 56 && abs(xdif) < 6)
							pass = 2;

						if (pass > 0) {
							_npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								_npcinfo[i].attacking = 1;
								_npcinfo[i].attackframe = 0;

								float nnxa = 0, nnya = 0, nnxb = 0, nnyb = 0;
								if (pass == 1 && xdif < 0) {
									nnxa = npx - 8;
									nnya = npy + 4;
									nnxb = npx - 56 - 8;
									nnyb = npy + 4;
									_npcinfo[i].walkdir = 2;
								} else if (pass == 1 && xdif > 0) {
									nnxa = npx + 16;
									nnya = npy + 4;
									nnxb = npx + 16 + 56;
									nnyb = npy + 4;
									_npcinfo[i].walkdir = 3;
								} else if (pass == 2 && ydif < 0) {
									nnya = npy;
									nnxa = npx + 4;
									nnyb = npy - 56;
									nnxb = npx + 4;
									_npcinfo[i].walkdir = 0;
								} else if (pass == 2 && ydif > 0) {
									nnya = npy + 20;
									nnxa = npx + 4;
									nnyb = npy + 20 + 56;
									nnxb = npx + 4;
									_npcinfo[i].walkdir = 1;
								}

								castSpell(7, nnxa, nnya, nnxb, nnyb, i);
							}
						}
					}
				}

				// twowing specific
				if (_npcinfo[i].spriteset == 9) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && _npcinfo[i].attacking == 0) {
						npx = _npcinfo[i].bodysection[7].x;
						npy = _npcinfo[i].bodysection[7].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 24 && abs(ydif) < 24) {
							float dist = sqrt(xdif * xdif + ydif * ydif);

							if ((dist) < 24) {
								if (config.effects) {
									int snd = playSound(_sfx[kSndBite]);
									setChannelVolume(snd, config.effectsvol);
								}

								_npcinfo[i].attacking = 1;
								_npcinfo[i].attackframe = 0;

								_npcinfo[i].headtargetx[0] = _player.px + 12;
								_npcinfo[i].headtargety[0] = _player.py - 4;
							}
						}

					}

					if (_npcinfo[i].attacking == 0 && _npcinfo[i].castpause < _ticks) {
						_npcinfo[i].swayspd = 4;

						// sway code
						_npcinfo[i].swayangle = _npcinfo[i].swayangle + _npcinfo[i].swayspd * _fpsr;
						if (_npcinfo[i].swayangle >= 360)
							_npcinfo[i].swayangle = _npcinfo[i].swayangle - 360;

						_npcinfo[i].headtargetx[0] = _npcinfo[i].x + (24 - _npcinfo[i].swayspd / 2) * sin(3.14159 / 180 * _npcinfo[i].swayangle) + 12;
						_npcinfo[i].headtargety[0] = _npcinfo[i].y - 36 + 16 + 8 * sin(3.14159 * 2 / 180 * _npcinfo[i].swayangle);
					}

					if (dospell) {
						_npcinfo[i].pause = _ticks + 3000;
						_npcinfo[i].attacknext = _ticks + 5000;
						_npcinfo[i].castpause = _ticks + 3000;

						castSpell(3, _npcinfo[i].x, _npcinfo[i].y, _npcinfo[i].x, _npcinfo[i].y, i);

						_npcinfo[i].headtargetx[0] = _npcinfo[i].x;
						_npcinfo[i].headtargety[0] = _npcinfo[i].y - 36 + 16;
					}

					// targethead code
					xdif = _npcinfo[i].bodysection[7].x - _npcinfo[i].headtargetx[0];
					ydif = _npcinfo[i].bodysection[7].y - _npcinfo[i].headtargety[0];


					_npcinfo[i].bodysection[7].x = _npcinfo[i].bodysection[7].x  - xdif * 0.4 * _fpsr;
					_npcinfo[i].bodysection[7].y = _npcinfo[i].bodysection[7].y  - ydif * 0.4 * _fpsr;

					_npcinfo[i].bodysection[0].x = _npcinfo[i].x + 12;
					_npcinfo[i].bodysection[0].y = _npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = _npcinfo[i].bodysection[f + 1].x - _npcinfo[i].bodysection[f - 1].x;
						ydif = _npcinfo[i].bodysection[f + 1].y - _npcinfo[i].bodysection[f - 1].y;

						float tx = _npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = _npcinfo[i].bodysection[f - 1].y + ydif / 2;

						_npcinfo[i].bodysection[f].x = _npcinfo[i].bodysection[f].x - (_npcinfo[i].bodysection[f].x - tx) / 3;
						_npcinfo[i].bodysection[f].y = _npcinfo[i].bodysection[f].y - (_npcinfo[i].bodysection[f].y - ty) / 3;
					}

				}

				// dragon2 specific
				if (_npcinfo[i].spriteset == 10 && _npcinfo[i].attackattempt < _ticks) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && _npcinfo[i].attacking == 0) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 32 && abs(ydif) < 32) {
							_npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								if (config.effects) {
									int snd = playSound(_sfx[kSndEnemyHit]);
									setChannelVolume(snd, config.effectsvol);
								}

								_npcinfo[i].attacking = 1;
								_npcinfo[i].attackframe = 0;
							}
						}
					}
				}


				// endboss specific
				if (_npcinfo[i].spriteset == 11 && _npcinfo[i].attackattempt < _ticks) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && _npcinfo[i].attacking == 0) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 38 && abs(ydif) < 38) {
							_npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								if (config.effects) {
									int snd = playSound(_sfx[kSndIce]);
									setChannelVolume(snd, config.effectsvol);
								}
								_npcinfo[i].attacking = 1;
								_npcinfo[i].attackframe = 0;
							}
						}
					}
				}
			}


			float npx = _npcinfo[i].x;
			float npy = _npcinfo[i].y;

			int xp = (npx / 2 + 6);
			int yp = (npy / 2 + 10);

			rcSrc.left = xp - 1;
			rcSrc.top = yp - 1;
			rcSrc.setWidth(3);
			rcSrc.setHeight(3);

			if (_npcinfo[i].pause < _ticks)
				_clipbg->fillRect(rcSrc, i);


			pass = 0;
			if (_npcinfo[i].attacking == 1)
				pass = 1;
			if (_npcinfo[i].spriteset == 5) {
				if (_npcinfo[i].attacking2[0] == 1)
					pass = 1;
				if (_npcinfo[i].attacking2[1] == 1)
					pass = 1;
				if (_npcinfo[i].attacking2[2] == 1)
					pass = 1;
			}

			if (pass == 1) {
				int dist;
				float damage;
				// spriteset1 specific
				if (_npcinfo[i].spriteset == 1) {
					_npcinfo[i].attackframe = _npcinfo[i].attackframe + _npcinfo[i].attackspd * _fpsr;
					if (_npcinfo[i].attackframe >= 16) {
						_npcinfo[i].attackframe = 0;
						_npcinfo[i].attacking = 0;
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
					}

					_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

					npx = _npcinfo[i].x;
					npy = _npcinfo[i].y;

					float xdif = _player.px - npx;
					float ydif = _player.py - npy;

					dist = 10;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
						// _npcinfo[i].attackframe = 0;
						// _npcinfo[i].attacking = 0;

						damage = (float)_npcinfo[i].attackdamage * (0.5 + RND() * 1.0);

						if (_player.hp > 0)
							damagePlayer(damage);
					}
				}

				if (_npcinfo[i].spriteset == 2) {
					// targethead code
					float xdif = _npcinfo[i].bodysection[7].x - _npcinfo[i].headtargetx[0];
					float ydif = _npcinfo[i].bodysection[7].y - _npcinfo[i].headtargety[0];

					_npcinfo[i].bodysection[7].x = _npcinfo[i].bodysection[7].x  - xdif * 0.4 * _fpsr;
					_npcinfo[i].bodysection[7].y = _npcinfo[i].bodysection[7].y  - ydif * 0.4 * _fpsr;

					_npcinfo[i].bodysection[0].x = _npcinfo[i].x + 12;
					_npcinfo[i].bodysection[0].y = _npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = _npcinfo[i].bodysection[f + 1].x - _npcinfo[i].bodysection[f - 1].x;
						ydif = _npcinfo[i].bodysection[f + 1].y - _npcinfo[i].bodysection[f - 1].y;

						float tx = _npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = _npcinfo[i].bodysection[f - 1].y + ydif / 2;

						_npcinfo[i].bodysection[f].x = _npcinfo[i].bodysection[f].x - (_npcinfo[i].bodysection[f].x - tx);
						_npcinfo[i].bodysection[f].y = _npcinfo[i].bodysection[f].y - (_npcinfo[i].bodysection[f].y - ty);
					}

					_npcinfo[i].attackframe = _npcinfo[i].attackframe + _npcinfo[i].attackspd * _fpsr;
					if (_npcinfo[i].attackframe >= 16) {
						_npcinfo[i].attackframe = 0;
						_npcinfo[i].attacking = 0;
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
					}

					_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

					npx = _npcinfo[i].bodysection[7].x;
					npy = (_npcinfo[i].bodysection[7].y + 16);

					xdif = (_player.px + 12) - npx;
					ydif = (_player.py + 12) - npy;

					dist = 8;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
						// _npcinfo[i].attackframe = 0
						// _npcinfo[i].attacking = 0
						damage = (float)_npcinfo[i].attackdamage * (1.0 + (RND() * 0.5));
						if (_player.hp > 0)
							damagePlayer(damage);
					}
				}


				// firehydra
				if (_npcinfo[i].spriteset == 5) {
					for (int ff = 0; ff <= 2; ff++) {
						if (_npcinfo[i].attacking2[ff] == 1) {
							float xdif = _npcinfo[i].bodysection[10 * ff + 9].x - _npcinfo[i].headtargetx[ff];
							float ydif = _npcinfo[i].bodysection[10 * ff + 9].y - _npcinfo[i].headtargety[ff];

							_npcinfo[i].bodysection[10 * ff + 9].x = _npcinfo[i].bodysection[10 * ff + 9].x  - xdif * .2 * _fpsr;
							_npcinfo[i].bodysection[10 * ff + 9].y = _npcinfo[i].bodysection[10 * ff + 9].y  - ydif * .2 * _fpsr;

							_npcinfo[i].bodysection[10 * ff].x = _npcinfo[i].x + 12 + 8 * cos(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));
							_npcinfo[i].bodysection[10 * ff].y = _npcinfo[i].y + 12 + 8 * sin(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));

							for (int f = 8; f >= 1; f--) {
								xdif = _npcinfo[i].bodysection[ff * 10 + f + 1].x - _npcinfo[i].bodysection[ff * 10 + f - 1].x;
								ydif = _npcinfo[i].bodysection[ff * 10 + f + 1].y - _npcinfo[i].bodysection[ff * 10 + f - 1].y;

								float tx = _npcinfo[i].bodysection[ff * 10 + f - 1].x + xdif / 2;
								float ty = _npcinfo[i].bodysection[ff * 10 + f - 1].y + ydif / 2;

								_npcinfo[i].bodysection[ff * 10 + f].x = _npcinfo[i].bodysection[ff * 10 + f].x - (_npcinfo[i].bodysection[ff * 10 + f].x - tx) / 3;
								_npcinfo[i].bodysection[ff * 10 + f].y = _npcinfo[i].bodysection[ff * 10 + f].y - (_npcinfo[i].bodysection[ff * 10 + f].y - ty) / 3;
							}

							_npcinfo[i].attackframe2[ff] = _npcinfo[i].attackframe2[ff] + _npcinfo[i].attackspd * _fpsr;
							if (_npcinfo[i].attackframe2[ff] >= 16) {
								_npcinfo[i].attackframe2[ff] = 0;
								_npcinfo[i].attacking2[ff] = 0;
								_npcinfo[i].attacknext2[ff] = _ticks + _npcinfo[i].attackdelay;
							}

							_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

							npx = _npcinfo[i].bodysection[10 * ff + 9].x;
							npy = (_npcinfo[i].bodysection[10 * ff + 9].y + 16);

							xdif = (_player.px + 12) - npx;
							ydif = (_player.py + 12) - npy;

							dist = 8;

							if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
								_npcinfo[i].attacknext2[ff] = _ticks + _npcinfo[i].attackdelay;
								// _npcinfo[i].attackframe2(ff) = 0
								// _npcinfo[i].attacking2(ff) = 0
								damage = (float)_npcinfo[i].attackdamage * (1.0 + RND() * 0.5);
								if (_player.hp > 0)
									damagePlayer(damage);
							}
						}
					}

				}

				// twowing specific
				if (_npcinfo[i].spriteset == 9) {
					// targethead code
					float xdif = _npcinfo[i].bodysection[7].x - _npcinfo[i].headtargetx[0];
					float ydif = _npcinfo[i].bodysection[7].y - _npcinfo[i].headtargety[0];

					_npcinfo[i].bodysection[7].x = _npcinfo[i].bodysection[7].x - xdif * 0.4 * _fpsr;
					_npcinfo[i].bodysection[7].y = _npcinfo[i].bodysection[7].y - ydif * 0.4 * _fpsr;

					_npcinfo[i].bodysection[0].x = _npcinfo[i].x + 12;
					_npcinfo[i].bodysection[0].y = _npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = _npcinfo[i].bodysection[f + 1].x - _npcinfo[i].bodysection[f - 1].x;
						ydif = _npcinfo[i].bodysection[f + 1].y - _npcinfo[i].bodysection[f - 1].y;

						float tx = _npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = _npcinfo[i].bodysection[f - 1].y + ydif / 2;

						_npcinfo[i].bodysection[f].x = _npcinfo[i].bodysection[f].x - (_npcinfo[i].bodysection[f].x - tx);
						_npcinfo[i].bodysection[f].y = _npcinfo[i].bodysection[f].y - (_npcinfo[i].bodysection[f].y - ty);
					}

					_npcinfo[i].attackframe = _npcinfo[i].attackframe + _npcinfo[i].attackspd * _fpsr;
					if (_npcinfo[i].attackframe >= 16) {
						_npcinfo[i].attackframe = 0;
						_npcinfo[i].attacking = 0;
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
					}

					_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

					npx = _npcinfo[i].bodysection[7].x;
					npy = (_npcinfo[i].bodysection[7].y + 16);

					xdif = (_player.px + 12) - npx;
					ydif = (_player.py + 12) - npy;

					dist = 8;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
						// _npcinfo[i].attackframe = 0
						// _npcinfo[i].attacking = 0
						damage = (float)_npcinfo[i].attackdamage * (1.0 + RND() * 0.5);
						if (_player.hp > 0)
							damagePlayer(damage);
					}
				}

				// dragon 2 specific
				if (_npcinfo[i].spriteset == 10) {

					_npcinfo[i].attackframe = _npcinfo[i].attackframe + _npcinfo[i].attackspd * _fpsr;
					if (_npcinfo[i].attackframe >= 16) {
						_npcinfo[i].attackframe = 0;
						_npcinfo[i].attacking = 0;
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
					}

					_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

					npx = _npcinfo[i].x;
					npy = _npcinfo[i].y;

					float xdif = _player.px - npx;
					float ydif = _player.py - npy;

					dist = 16 + _npcinfo[i].attackframe;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
						// _npcinfo[i].attackframe = 0
						// _npcinfo[i].attacking = 0
						damage = (float)_npcinfo[i].attackdamage * (0.5 + RND() * 1.0);
						if (_player.hp > 0)
							damagePlayer(damage);
					}
				}

				// endboss specific
				if (_npcinfo[i].spriteset == 11) {
					_npcinfo[i].attackframe = _npcinfo[i].attackframe + _npcinfo[i].attackspd * _fpsr;
					if (_npcinfo[i].attackframe >= 16) {
						_npcinfo[i].attackframe = 0;
						_npcinfo[i].attacking = 0;
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
					}

					_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

					npx = _npcinfo[i].x;
					npy = _npcinfo[i].y;

					float xdif = _player.px - npx;
					float ydif = _player.py - npy;

					dist = 36;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
						// _npcinfo[i].attackframe = 0
						// _npcinfo[i].attacking = 0
						damage = (float)_npcinfo[i].attackdamage * (0.5 + RND() * 1.0);
						if (_player.hp > 0)
							damagePlayer(damage);
					}
				}
			}
			// -------end fight code
		}
	}
}

void GriffonEngine::updateSpells() {
	int foundel[5];
	float npx, npy;
	long cl1, cl2, cl3;
	int ll[4][2];

	float xloc = 0, yloc = 0, xst, yst, xdif, ydif;

	for (int i = 0; i < kMaxSpell; i++) {
		if (spellinfo[i].frame > 0) {
			int spellnum = spellinfo[i].spellnum;

			// water
			if (spellnum == 0 && !_forcepause) {
				float fr = (32 - spellinfo[i].frame);

				ll[0][0] = -2;
				ll[0][1] = -3;
				ll[1][0] = 2;
				ll[1][1] = -3;
				ll[2][0] = -4;
				ll[2][1] = -2;
				ll[3][0] = 4;
				ll[3][1] = -2;

				for (int f = 0; f <= 3; f++) {
					if (fr > f * 4 && fr < f * 4 + 16) {
						float alf = 255;

						if (fr < f * 4 + 8) {
							int fi = (int)((fr - f * 4) * 3) % 4;
							rcSrc.left = 32 + fi * 16;
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							xloc = spellinfo[i].enemyx + 12 + ll[f][0] * 16;
							yloc = spellinfo[i].enemyy + 16 + ll[f][1] * 16;

							rcDest.left = xloc;
							rcDest.top = yloc;

							alf = 255 * ((fr - f * 4) / 8);
						}

						if (fr >= f * 4 + 8) {
							int fi = 0; // ??

							if (f == 0 || f == 2)
								fi = 0;
							if (f == 1 || f == 3)
								fi = 1;
							rcSrc.left = 32 + fi * 16;
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							xst = spellinfo[i].enemyx + 12 + ll[f][0] * 16;
							yst = spellinfo[i].enemyy + 16 + ll[f][1] * 16;

							float xi = (spellinfo[i].enemyx - xst) * 2 / 8;
							float yi = (spellinfo[i].enemyy - yst) * 2 / 8;

							float fl = (fr - f * 4 - 8) / 2;
							xloc = xst + xi * fl * fl;
							yloc = yst + yi * fl * fl;

							rcDest.left = xloc;
							rcDest.top = yloc;

							alf = 255;
						}

						if (xloc > -16 && xloc < 304 && yloc > -16 && yloc < 224) {
							_spellimg->setAlpha(alf, true);
							_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
							_spellimg->setAlpha(255, true);

							if (spellinfo[i].damagewho == 0) {
								for (int e = 1; e <= _lastnpc; e++) {

									xdif = (xloc + 16) - (_npcinfo[e].x + 12);
									ydif = (yloc + 16) - (_npcinfo[e].y + 12);

									if ((abs(xdif) < 16 && abs(ydif) < 16)) {
										float damage = (float)_player.spelldamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

										if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks) {
											damageNPC(e, damage, 1);
											if (config.effects) {
												int snd = playSound(_sfx[kSndIce]);
												setChannelVolume(snd, config.effectsvol);
											}
										}
									}
								}
							}

							// check for post damage
							if (nposts > 0) {
								for (int e = 0; e <= nposts - 1; e++) {
									xdif = (xloc + 16) - (postinfo[e][0] + 8);
									ydif = (yloc + 16) - (postinfo[e][1] + 8);

									if ((abs(xdif) < 16 && abs(ydif) < 16)) {
										_objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
										_objectMap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

										rcSrc.left = postinfo[e][0] / 2;
										rcSrc.top = postinfo[e][1] / 2;
										rcSrc.setWidth(8);
										rcSrc.setHeight(8);

										_clipbg2->fillRect(rcSrc, 0);

										addFloatIcon(99, postinfo[e][0], postinfo[e][1]);

										if (config.effects) {
											int snd = playSound(_sfx[kSndIce]);
											setChannelVolume(snd, config.effectsvol);
										}
									}
								}
							}
						}
					}
				}
			}

			// metal
			if (spellnum == 1 && !_forcepause) {
				int fr = (int)((32 - spellinfo[i].frame) * 4) % 3;

				rcSrc.left = fr * 48;
				rcSrc.top = 0;
				rcSrc.setWidth(48);
				rcSrc.setHeight(48);

				float c1 = (32 - spellinfo[i].frame) / 16;

				float halfx = (spellinfo[i].homex - 12) + ((spellinfo[i].enemyx - 12) - (spellinfo[i].homex - 12)) / 2;
				float halfy = (spellinfo[i].homey - 12) + ((spellinfo[i].enemyy - 12) - (spellinfo[i].homey - 12)) / 2;

				float wdth = (halfx - spellinfo[i].homex) * 1.2;
				float hight = (halfy - spellinfo[i].homey) * 1.2;

				xloc = halfx + wdth * cos(3.14159 + 3.14159 * 2 * c1);
				yloc = halfy + hight * sin(3.14159 + 3.14159 * 2 * c1);

				rcDest.left = xloc;
				rcDest.top = yloc;

				_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;

				if (spellinfo[i].damagewho == 0) {
					for (int e = 1; e <= _lastnpc; e++) {
						xdif = (xloc + 24) - (_npcinfo[e].x + 12);
						ydif = (yloc + 24) - (_npcinfo[e].y + 12);

						if ((abs(xdif) < 24 && abs(ydif) < 24)) {
							float damage = (float)_player.spelldamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

							if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks) {
								damageNPC(e, damage, 1);
								if (config.effects) {
									int snd = playSound(_sfx[kSndMetalHit]);
									setChannelVolume(snd, config.effectsvol);
								}
							}
						}
					}
				}

				if (spellinfo[i].damagewho == 1) {
					// --------- boss 1 specific
					if (ABS(spellinfo[i].frame) < 0 && _npcinfo[spellinfo[i].npc].spriteset == 3) {
						int npc = spellinfo[i].npc;
						_npcinfo[npc].attackframe = 0;
						_npcinfo[npc].attacking = 0;

						_npcinfo[npc].pause = _ticks + 1000;
						_npcinfo[npc].attacknext = _ticks + 4000;
					}
					// ---------------

					// --------- blackknight specific
					if (ABS(spellinfo[i].frame) < 0 && _npcinfo[spellinfo[i].npc].spriteset == 4) {
						int npc = spellinfo[i].npc;
						_npcinfo[npc].attackframe = 0;
						_npcinfo[npc].attacking = 0;

						_npcinfo[npc].pause = _ticks + 1000;
						_npcinfo[npc].attacknext = _ticks + 3500;
					}
					// ---------------

					xdif = (xloc + 24) - (_player.px + 12);
					ydif = (yloc + 24) - (_player.py + 12);

					if ((abs(xdif) < 24 && abs(ydif) < 24) && _player.pause < _ticks) {
						npx = _player.px;
						npy = _player.py;

						float damage = (float)_npcinfo[spellinfo[i].npc].spelldamage * (1.0 + RND() * 0.5);

						if (_player.hp > 0) {
							damagePlayer(damage);
							if (config.effects) {
								int snd = playSound(_sfx[kSndMetalHit]);
								setChannelVolume(snd, config.effectsvol);
							}
						}
					}
				}


				// check for(int post damage
				if (nposts > 0) {
					for (int e = 0; e <= nposts - 1; e++) {
						xdif = (xloc + 24) - (postinfo[e][0] + 8);
						ydif = (yloc + 24) - (postinfo[e][1] + 8);

						if ((abs(xdif) < 24 && abs(ydif) < 24)) {
							_objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
							_objectMap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

							rcSrc.left = postinfo[e][0] / 2;
							rcSrc.top = postinfo[e][1] / 2;
							rcSrc.setWidth(8);
							rcSrc.setHeight(8);

							_clipbg2->fillRect(rcSrc, 0);

							addFloatIcon(99, postinfo[e][0], postinfo[e][1]);

							if (config.effects) {
								int snd = playSound(_sfx[kSndMetalHit]);
								setChannelVolume(snd, config.effectsvol);
							}
						}
					}
				}
			}

			// earth
			if (spellnum == 2 && !_forcepause) {
				float hght = 240 - spellinfo[i].enemyy;

				for (int f = 8; f >= 0; f--) {

					float fr = (32 - spellinfo[i].frame);

					if (fr > f && fr < f + 16) {
						rcSrc.left = 32 * spellinfo[i].rockimg[f];
						rcSrc.top = 48;
						rcSrc.setWidth(32);
						rcSrc.setHeight(32);

						int scatter = 0;
						if (fr < 8 + f) {
							xloc = spellinfo[i].enemyx - 4;
							yloc = spellinfo[i].enemyy * (1 - cos(3.14159 / 2 * (fr - f) / 8)); // ^ 2;
							yloc *= yloc;
						} else {
							scatter = 1;
							xloc = spellinfo[i].enemyx - 4 - spellinfo[i].rockdeflect[f] * sin(3.14159 / 2 * ((fr - f) - 8) / 8);
							yloc = spellinfo[i].enemyy + hght * (1 - cos(3.14159 / 2 * ((fr - f) - 8) / 8));
						}

						rcDest.left = xloc;
						rcDest.top = yloc;

						if (xloc > -16 && xloc < 304 && yloc > -16 && yloc < 224) {
							_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

							if (scatter == 1) {
								if (spellinfo[i].damagewho == 0) {
									for (int e = 1; e <= _lastnpc; e++) {
										xdif = (xloc + 16) - (_npcinfo[e].x + 12);
										ydif = (yloc + 16) - (_npcinfo[e].y + 12);

										if ((abs(xdif) < 16 && abs(ydif) < 16)) {
											float damage = (float)_player.spelldamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

											if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks) {
												damageNPC(e, damage, 1);
												if (config.effects) {
													int snd = playSound(_sfx[kSndRocks]);
													setChannelVolume(snd, config.effectsvol);
												}
											}
										}
									}
								}


								// check for(int post damage
								if (nposts > 0) {
									for (int e = 0; e <= nposts - 1; e++) {
										xdif = (xloc + 16) - (postinfo[e][0] + 8);
										ydif = (yloc + 16) - (postinfo[e][1] + 8);

										if ((abs(xdif) < 16 && abs(ydif) < 16)) {
											_objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
											_objectMap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

											rcSrc.left = postinfo[e][0] / 2;
											rcSrc.top = postinfo[e][1] / 2;
											rcSrc.setWidth(8);
											rcSrc.setHeight(8);

											_clipbg2->fillRect(rcSrc, 0);

											addFloatIcon(99, postinfo[e][0], postinfo[e][1]);

											if (config.effects) {
												int snd = playSound(_sfx[kSndRocks]);
												setChannelVolume(snd, config.effectsvol);
											}
										}
									}
								}
							}
						}
					}
				}

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;
			}

			// crystal
			if (spellnum == 5) {

				float fra = (32 - spellinfo[i].frame);
				int fr = (int)((spellinfo[i].frame) * 2) % 8;

				rcSrc.left = fr * 32;
				rcSrc.top = 96 + 48;
				rcSrc.setWidth(32);
				rcSrc.setHeight(64);

				rcDest.left = _player.px - 4;
				rcDest.top = _player.py + 16 - 48;

				int f = 160;
				if (fra < 8)
					f = 192 * fra / 8;
				if (fra > 24)
					f = 192 * (1 - (fra - 24) / 8);

				_spellimg->setAlpha(f, true);
				_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
				_spellimg->setAlpha(255, true);

				spellinfo[i].frame = spellinfo[i].frame - 0.3 * _fpsr;
				if (spellinfo[i].frame < 0) {
					spellinfo[i].frame = 0;
					_forcepause = false;

					npx = _player.px + 12;
					npy = _player.py + 20;

					int lx = (int)npx / 16;
					int ly = (int)npy / 16;

					for (int f1 = 0; f1 < 5; f1++) { // !! f < 5
						foundel[f1] = 0;
					}

					for (int xo = -2; xo <= 2; xo++) {
						for (int yo = -2; yo <= 2; yo++) {

							int sx = lx + xo;
							int sy = ly + yo;

							if (sx > -1 && sx < 20 && sy > -1 && sy < 15) {
								for (int l = 0; l <= 2; l++) {
									int curtile = _tileinfo[l][sx][sy][0];
									int curtilel = _tileinfo[l][sx][sy][1];

									if (curtile > 0) {
										curtile = curtile - 1;
										int curtilex = curtile % 20;
										int curtiley = (curtile - curtilex) / 20;

										int element = elementmap[curtiley][curtilex];
										if (element > -1 && curtilel == 0)
											foundel[element + 1] = 1;
									}
								}

								int o = _objectMap[sx][sy];
								if (o > -1) {
									if (_objectInfo[o][4] == 1)
										foundel[2] = 1;
									if (o == 1 || o == 2) {
										foundel[2] = 1;
										foundel[4] = 1;
									}
								}
							}
						}
					}

					char line[256];
					strcpy(line, "Found... nothing...");

					for (int f1 = 0; f1 < 5; f1++) {
						if (foundel[f1] == 1 && _player.foundspell[f1] == 0) {
							_player.foundspell[f1] = 1;
							_player.spellcharge[f1] = 0;
							if (f1 == 1)
								strcpy(line, "Found... Water Essence");
							if (f1 == 2)
								strcpy(line, "Found... Metal Essence");
							if (f1 == 3)
								strcpy(line, "Found... Earth Essence");
							if (f1 == 4)
								strcpy(line, "Found... Fire Essence");
							break;
						}
					}

					eventText(line);
				}
			}

			// room fireballs
			if (spellnum == 6 && !_forcepause) {

				if (spellinfo[i].frame > 16) {
					float fr = (32 - spellinfo[i].frame);

					_spellimg->setAlpha(192 * sin(3.14159 * fr / 4), true);

					rcSrc.left = 16 * (int)(RND() * 2);
					rcSrc.top = 80;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					for (int ff = 0; ff <= spellinfo[i].nfballs - 1; ff++) {

						xloc = spellinfo[i].fireballs[ff][0] + (int)(RND() * 3) - 1;
						yloc = spellinfo[i].fireballs[ff][1] + (int)(RND() * 3) - 1;

						rcDest.left = xloc;
						rcDest.top = yloc;

						_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}

					_spellimg->setAlpha(255, true);
				} else {
					_spellimg->setAlpha(192, true);

					rcSrc.left = 16 * (int)(RND() * 2);
					rcSrc.top = 80;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					for (int ff = 0; ff <= spellinfo[i].nfballs - 1; ff++) {
						float ax = spellinfo[i].fireballs[ff][0];
						float ay = spellinfo[i].fireballs[ff][1];
						float bx = _player.px + 4;
						float by = _player.py + 4;
						float d = sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));

						float tx = (bx - ax) / d;
						float ty = (by - ay) / d;

						spellinfo[i].fireballs[ff][2] += tx * 1.2 * _fpsr;
						spellinfo[i].fireballs[ff][3] += ty * 1.2 * _fpsr;

						if (spellinfo[i].ballon[ff] == 1) {
							spellinfo[i].fireballs[ff][0] = ax + spellinfo[i].fireballs[ff][2] * 0.2 * _fpsr;
							spellinfo[i].fireballs[ff][1] = ay + spellinfo[i].fireballs[ff][3] * 0.2 * _fpsr;

							xloc = spellinfo[i].fireballs[ff][0] + (int)(RND() * 3) - 1;
							yloc = spellinfo[i].fireballs[ff][1] + (int)(RND() * 3) - 1;

							rcDest.left = xloc;
							rcDest.top = yloc;

							_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
						}

						if (xloc < -1 || yloc < -1 || xloc > 304 || yloc > 224)
							spellinfo[i].ballon[ff] = 0;
					}

					_spellimg->setAlpha(255, true);
				}

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;

				if (spellinfo[i].damagewho == 1) {
					for (int ff = 0; ff <= spellinfo[i].nfballs - 1; ff++) {
						if (spellinfo[i].ballon[ff] == 1) {
							xloc = spellinfo[i].fireballs[ff][0] + (int)(RND() * 3) - 1;
							yloc = spellinfo[i].fireballs[ff][1] + (int)(RND() * 3) - 1;

							xdif = (xloc + 8) - (_player.px + 12);
							ydif = (yloc + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
								float damage = _npcinfo[spellinfo[i].npc].spelldamage * (1 + RND() * 0.5) / 3;

								if (_player.hp > 0)
									damagePlayer(damage);

								if (config.effects) {
									int snd = playSound(_sfx[kSndFire]);
									setChannelVolume(snd, config.effectsvol);
								}
							}
						}
					}
				}
			}

			// lightning bomb
			if (spellnum == 8) {

				cl1 = _videobuffer->format.RGBToColor(0, 32, 204);
				cl2 = _videobuffer->format.RGBToColor(142, 173, 191);
				cl3 = _videobuffer->format.RGBToColor(240, 240, 240);

				float px = _player.px + 12;
				float py = _player.py + 12;

				int apx = px + (int)(RND() * 5 - 2);
				int apy = py + (int)(RND() * 5 - 2);

				for (int f = 0; f <= 0; f++) { // ??
					int y = apy;
					int orn = 0;
					for (int x = apx; x <= 319; x++) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							y = y - 1;
						if (orn == 2)
							y = y + 1;

						drawLine(_videobuffer, x, y - 1, x, y + 2, cl1);
						drawLine(_videobuffer, x, y, x, y + 1, cl3);

						if (rn == 0)
							drawLine(_videobuffer, x, y + 1, x, y + 1, cl2);
						if (rn == 2)
							drawLine(_videobuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= _lastnpc; e++) {

								xdif = (x + 16) - (_npcinfo[e].x + 12);
								ydif = (y + 16) - (_npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks)
										damageNPC(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								xdif = (xloc + 16) - (postinfo[e][0] + 8);
								ydif = (yloc + 16) - (postinfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									_objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
									_objectMap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

									rcSrc.left = postinfo[e][0] / 2;
									rcSrc.top = postinfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipbg2->fillRect(rcSrc, 0);

									addFloatIcon(99, postinfo[e][0], postinfo[e][1]);
								}
							}
						}
					}

					y = apy;
					orn = 0;
					for (int x = apx; x >= 0; x--) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							y = y - 1;
						if (orn == 2)
							y = y + 1;

						drawLine(_videobuffer, x, y - 1, x, y + 2, cl1);
						drawLine(_videobuffer, x, y, x, y + 1, cl3);

						if (rn == 0)
							drawLine(_videobuffer, x, y + 1, x, y + 1, cl2);
						if (rn == 2)
							drawLine(_videobuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= _lastnpc; e++) {

								xdif = (x + 16) - (_npcinfo[e].x + 12);
								ydif = (y + 16) - (_npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks)
										damageNPC(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								xdif = (xloc + 16) - (postinfo[e][0] + 8);
								ydif = (yloc + 16) - (postinfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									_objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
									_objectMap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

									rcSrc.left = postinfo[e][0] / 2;
									rcSrc.top = postinfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipbg2->fillRect(rcSrc, 0);

									addFloatIcon(99, postinfo[e][0], postinfo[e][1]);
								}
							}
						}
					}

					int x = apx;
					orn = 0;
					for (y = apy; y <= 239; y++) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							x = x - 1;
						if (orn == 2)
							x = x + 1;

						drawLine(_videobuffer, x - 1, y, x + 2, y, cl1);
						drawLine(_videobuffer, x, y, x + 1, y, cl3);

						if (rn == 0)
							drawLine(_videobuffer, x + 1, y, x + 1, y, cl2);
						if (rn == 2)
							drawLine(_videobuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= _lastnpc; e++) {
								xdif = (x + 16) - (_npcinfo[e].x + 12);
								ydif = (y + 16) - (_npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks)
										damageNPC(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								xdif = (xloc + 16) - (postinfo[e][0] + 8);
								ydif = (yloc + 16) - (postinfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									_objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
									_objectMap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

									rcSrc.left = postinfo[e][0] / 2;
									rcSrc.top = postinfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipbg2->fillRect(rcSrc, 0);

									addFloatIcon(99, postinfo[e][0], postinfo[e][1]);
								}
							}
						}
					}

					x = apx;
					orn = 0;
					for (y = apy; y >= 0; y--) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							x = x - 1;
						if (orn == 2)
							x = x + 1;

						drawLine(_videobuffer, x - 1, y, x + 2, y, cl1);
						drawLine(_videobuffer, x, y, x + 1, y, cl3);

						if (rn == 0)
							drawLine(_videobuffer, x + 1, y, x + 1, y, cl2);
						if (rn == 2)
							drawLine(_videobuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= _lastnpc; e++) {
								xdif = (x + 16) - (_npcinfo[e].x + 12);
								ydif = (y + 16) - (_npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks)
										damageNPC(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								xdif = (xloc + 16) - (postinfo[e][0] + 8);
								ydif = (yloc + 16) - (postinfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									_objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
									_objectMap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

									rcSrc.left = postinfo[e][0] / 2;
									rcSrc.top = postinfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipbg2->fillRect(rcSrc, 0);

									addFloatIcon(99, postinfo[e][0], postinfo[e][1]);
								}
							}
						}
					}
				}

				spellinfo[i].frame -= 0.5 * _fpsr;
				if (spellinfo[i].frame < 0) {
					spellinfo[i].frame = 0;
					_forcepause = false;
				}
			}

			// wizard 1 lightning
			if (spellnum == 9) {

				cl1 = _videobuffer->format.RGBToColor(0, 32, 204);
				cl2 = _videobuffer->format.RGBToColor(142, 173, 191);
				cl3 = _videobuffer->format.RGBToColor(240, 240, 240);

				int px = spellinfo[i].enemyx + 12;
				int py = spellinfo[i].enemyy + 24;

				int apx = px + (int)(RND() * 20 - 10);
				int apy = py + (int)(RND() * 20 - 10);

				int x = apx;
				int orn = 0;
				for (int y = 0; y <= apy; y++) {
					if (y < 240) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							x = x - 1;
						if (orn == 2)
							x = x + 1;

						drawLine(_videobuffer, x - 1, y, x + 2, y, cl1);
						drawLine(_videobuffer, x, y, x + 1, y, cl3);

						if (rn == 0)
							drawLine(_videobuffer, x + 1, y, x + 1, y, cl2);
						if (rn == 2)
							drawLine(_videobuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 1) {

							xdif = (x + 8) - (_player.px + 12);
							ydif = (y + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
								float damage = ((float)_player.hp * 0.75) * (RND() * 0.5 + 0.5);
								if (damage < 5)
									damage = 5;

								if (_npcinfo[spellinfo[i].npc].spriteset == 12) {
									if (damage < 50)
										damage = 40 + (int)(RND() * 40);
								}

								if (_player.hp > 0)
									damagePlayer(damage);
							}
						}
					}
				}

				spellinfo[i].frame -= 0.5 * _fpsr;
				if (spellinfo[i].frame < 0) {
					spellinfo[i].frame = 0;

					_npcinfo[spellinfo[i].npc].attacking = 0;
					_npcinfo[spellinfo[i].npc].attacknext = _ticks + _npcinfo[spellinfo[i].npc].attackdelay;
				}
			}
		}
	}
}



void GriffonEngine::updateSpellsUnder() {
	if (_forcepause)
		return;

	for (int i = 0; i < kMaxSpell; i++) {
		if (spellinfo[i].frame > 0) {
			int spellnum = spellinfo[i].spellnum;

			// water
			if (spellnum == 0) {
				int fra = (32 - spellinfo[i].frame);
				int fr = (int)((32 - spellinfo[i].frame) * 2) % 4;

				rcSrc.left = fr * 48;
				rcSrc.top = 96;
				rcSrc.setWidth(48);
				rcSrc.setHeight(48);

				rcDest.left = spellinfo[i].enemyx - 12;
				rcDest.top = spellinfo[i].enemyy - 8;

				int f = 160;
				if (fra < 8)
					f = 160 * fra / 8;
				if (fra > 24)
					f = 160 * (1 - (fra - 24) / 8);

				_spellimg->setAlpha(f, true);
				_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
				_spellimg->setAlpha(255, true);

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;


				for (f = 1; f <= _lastnpc; f++) {
					int xdif = spellinfo[i].enemyx - _npcinfo[f].x;
					int ydif = spellinfo[i].enemyy - _npcinfo[f].y;

					float dist = sqrt((float)(xdif * xdif + ydif * ydif));

					if (dist > 20)
						dist = 20;

					if (dist > 5) {
						float ratio = (1 - dist / 25);

						float newx = _npcinfo[f].x + ratio * xdif / 3 * _fpsr;
						float newy = _npcinfo[f].y + ratio * ydif / 3 * _fpsr;

						int sx = (newx / 2 + 6);
						int sy = (newy / 2 + 10);

						unsigned int *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						unsigned int dq = *temp;

						if (dq == 0) {
							_npcinfo[f].x = newx;
							_npcinfo[f].y = newy;
							// _npcinfo[f].castpause = _ticks + 200;
						} else {
							int xpass = 0;
							int ypass = 0;

							sx = (newx / 2 + 6);
							sy = (_npcinfo[f].y / 2 + 10);
							temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
							dq = *temp;

							if (dq == 0)
								xpass = 1;


							sx = (_npcinfo[f].x / 2 + 6);
							sy = (newy / 2 + 10);
							temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
							dq = *temp;

							if (dq == 0)
								ypass = 1;

							if (ypass == 1) {
								newx = _npcinfo[f].x;
							} else if (xpass == 1) {
								newy = _npcinfo[f].y;
							}

							if (xpass == 1 || ypass == 1) {
								_npcinfo[f].x = newx;
								_npcinfo[f].y = newy;
								// _npcinfo[f].castpause = _ticks + 200;
							}
						}
					}
				}
			}

			// fire
			if (spellnum == 3) {
				float fr = (32 - spellinfo[i].frame);

				fr = fr * fr * (1 - cos(3.14159 / 4 + 3.14159 / 4 * fr / 32));

				if (fr > 32)
					fr = 32;

				float s = 8;
				if (spellinfo[i].frame < 8)
					s = spellinfo[i].frame;

				int fra = (int)fr;

				for (int f = 0; f <= 4; f++) {
					for (int x = 0; x <= fra; x += 2) {
						if (spellinfo[i].legalive[f] >= x) {
							_spellimg->setAlpha(192 * sin(3.14159 * x / 32) * s / 8, true);

							float an = 360 / 5 * f + x / 32 * 180;

							rcSrc.left = 16 * (int)(RND() * 2);
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							float xloc = (float)(spellinfo[i].enemyx + 4 + x * 2 * cos(3.14159 / 180 * an) + (int)(RND() * 3) - 1);
							float yloc = (float)(spellinfo[i].enemyy + 4 + x * 2 * sin(3.14159 / 180 * an) + (int)(RND() * 3) - 1);
							rcDest.left = (int)xloc;
							rcDest.top = (int)yloc;

							if (xloc > -1 && xloc < 304 && yloc > -1 && yloc < 224) {
								_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

								int sx = (xloc / 2 + 4);
								int sy = (yloc / 2 + 8);

								unsigned int *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
								unsigned int dq = *temp;

								if (dq > 1000 && x > 4)
									spellinfo[i].legalive[f] = x;

								if (spellinfo[i].damagewho == 0) {
									for (int e = 1; e <= _lastnpc; e++) {
										float xdif = (xloc + 8) - (_npcinfo[e].x + 12);
										float ydif = (yloc + 8) - (_npcinfo[e].y + 12);

										if ((abs(xdif) < 8 && abs(ydif) < 8)) {
											float damage = (float)_player.spelldamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

											if (_npcinfo[e].spriteset == 5)
												damage = -damage;
											if (_npcinfo[e].spriteset == 11)
												damage = -damage;
											if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks) {
												damageNPC(e, damage, 1);
												if (config.effects) {
													int snd = playSound(_sfx[kSndFire]);
													setChannelVolume(snd, config.effectsvol);
												}
											}
										}
									}
								}

								if (spellinfo[i].damagewho == 1) {
									float xdif = (xloc + 8) - (_player.px + 12);
									float ydif = (yloc + 8) - (_player.py + 12);

									if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
										float damage = (float)_npcinfo[spellinfo[i].npc].spelldamage * (1.0 + RND() * 0.5);

										if (_player.hp > 0) {
											damagePlayer(damage);

											if (config.effects) {
												int snd = playSound(_sfx[kSndFire]);
												setChannelVolume(snd, config.effectsvol);
											}
										}
									}
								}

								// check for post damage
								if (nposts > 0) {
									for (int e = 0; e <= nposts - 1; e++) {
										float xdif = (xloc + 8) - (postinfo[e][0] + 8);
										float ydif = (yloc + 8) - (postinfo[e][1] + 8);

										if ((abs(xdif) < 8 && abs(ydif) < 8)) {
											_objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
											_objectMap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

											rcSrc.left = postinfo[e][0] / 2;
											rcSrc.top = postinfo[e][1] / 2;
											rcSrc.setWidth(8);
											rcSrc.setHeight(8);

											_clipbg2->fillRect(rcSrc, 0);

											if (config.effects) {
												int snd = playSound(_sfx[kSndFire]);
												setChannelVolume(snd, config.effectsvol);
											}

											addFloatIcon(99, postinfo[e][0], postinfo[e][1]);
										}
									}
								}
							}
						}
					}
				}

				_spellimg->setAlpha(255, true);

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;


			}


			// sprite 6 spitfire
			if (spellnum == 7) {
				float xspan = spellinfo[i].enemyx - spellinfo[i].homex;
				float yspan = spellinfo[i].enemyy - spellinfo[i].homey;
				float fr = (32 - spellinfo[i].frame);

				for (int f = 0; f <= 7; f++) {
					int alpha = 0;
					float xx = 0;
					if (fr > f * 2 && fr < f * 2 + 16)
						xx = fr - f * 2;
					if (xx < 8)
						alpha = 255 * xx / 8;
					if (xx > 8)
						alpha = 255 * (1 - (xx - 8) / 8);
					float yy = 16 * sin(3.141592 / 2 * xx / 16) - 8;

					if (alpha < 0)
						alpha = 0;
					if (alpha > 255)
						alpha = 255;

					_spellimg->setAlpha(alpha, true);

					rcSrc.left = 16 * (int)(RND() * 2);
					rcSrc.top = 80;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					float xloc = spellinfo[i].homex + xspan / 7 * f;
					float yloc = spellinfo[i].homey + yspan / 7 * f - yy;

					rcDest.left = xloc;
					rcDest.top = yloc;

					if (xloc > -16 && xloc < 320 && yloc > -16 && yloc < 240) {
						_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

						if (spellinfo[i].damagewho == 1) {
							float xdif = (xloc + 8) - (_player.px + 12);
							float ydif = (yloc + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks && alpha > 64) {
								float damage = (float)_npcinfo[spellinfo[i].npc].spelldamage * (1.0 + RND() * 0.5);

								if (_player.hp > 0) {
									damagePlayer(damage);
									if (config.effects) {
										int snd = playSound(_sfx[kSndFire]);
										setChannelVolume(snd, config.effectsvol);
									}
								}

							}
						}

					}

				}

				_spellimg->setAlpha(255, true);
				spellinfo[i].frame = spellinfo[i].frame - 0.5 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;

				if (ABS(spellinfo[i].frame) < 0) {
					_npcinfo[spellinfo[i].npc].attacking = 0;
					_npcinfo[spellinfo[i].npc].attacknext = _ticks + _npcinfo[spellinfo[i].npc].attackdelay;
				}
			}
		}
	}
}

void GriffonEngine::initialize() {
	// init char *_floatstri[kMaxFloat]
	for (int i = 0; i < kMaxFloat; i++)
		_floatstri[i] = (char *)malloc(64); // 64 bytes each string (should be enough)

	_video = new Graphics::TransparentSurface;
	_video->create(320, 240, g_system->getScreenFormat());
	_videobuffer = new Graphics::TransparentSurface;
	_videobuffer->create(320, 240, g_system->getScreenFormat());
	_videobuffer2 = new Graphics::TransparentSurface;
	_videobuffer2->create(320, 240, g_system->getScreenFormat());
	_videobuffer3 = new Graphics::TransparentSurface;
	_videobuffer3->create(320, 240, g_system->getScreenFormat());
	_mapbg = new Graphics::TransparentSurface;
	_mapbg->create(320, 240, g_system->getScreenFormat());
	_clipbg = new Graphics::TransparentSurface;
	_clipbg->create(320, 240, g_system->getScreenFormat());
	_clipbg2 = new Graphics::TransparentSurface;
	_clipbg2->create(320, 240, g_system->getScreenFormat());

	for (int i = 0; i <= 3; i++) {
		char name[128];

		sprintf(name, "art/map%i.bmp", i + 1);
		mapimg[i] = loadImage(name, true);
	}

	cloudimg = loadImage("art/clouds.bmp", true);
	cloudimg->setAlpha(96, true);


	saveloadimg = loadImage("art/saveloadnew.bmp", true);
	saveloadimg->setAlpha(160, true);

	_titleimg = loadImage("art/titleb.bmp");
	_titleimg2 = loadImage("art/titlea.bmp", true);
	//_titleimg2->setAlpha(204, true);

	_inventoryimg = loadImage("art/inventory.bmp", true);

	_logosimg = loadImage("art/logos.bmp");
	_theendimg = loadImage("art/theend.bmp");


	loadTiles();
	loadTriggers();
	loadObjectDB();
	loadAnims();
	loadFont();
	loadItemImgs();

	_fpsr = 1.0f;
	_nextticks = _ticks + 1000;

	for (int i = 0; i <= 15; i++) {
		_playerattackofs[0][i][0] = 0; // -1// -(i + 1)
		_playerattackofs[0][i][1] = -sin(3.14159 * 2 * (i + 1) / 16) * 2 - 1;

		_playerattackofs[1][i][0] = 0; // i + 1
		_playerattackofs[1][i][1] = -sin(3.14159 * 2 * (i + 1) / 16) * 2 + 1;

		_playerattackofs[2][i][0] = -1; // -(i + 1)
		_playerattackofs[2][i][1] = -sin(3.14159 * 2 * (i + 1) / 16) * 2;

		_playerattackofs[3][i][0] = 1; // i + 1
		_playerattackofs[3][i][1] = -sin(3.14159 * 2 * (i + 1) / 16) * 2;
	}

	setupAudio();
}

void GriffonEngine::drawLine(Graphics::TransparentSurface *buffer, int x1, int y1, int x2, int y2, int col) {
	int xdif = x2 - x1;
	int ydif = y2 - y1;

	if (xdif == 0) {
		for (int y = y1; y <= y2; y++) {
			uint32 *temp = (uint32 *)buffer->getBasePtr(x1, y);
			*temp = col;
		}
	}

	if (ydif == 0) {
		for (int x = x1; x <= x2; x++) {
			uint32 *temp = (uint32 *)buffer->getBasePtr(x, y1);
			*temp = col;
		}
	}
}

void GriffonEngine::loadAnims() {
	_spellimg = loadImage("art/spells.bmp", true);
	_anims[0] = loadImage("art/anims0.bmp", true);
	_animsa[0] = loadImage("art/anims0a.bmp", true);
	_anims[13] = loadImage("art/anims0x.bmp", true);
	_animsa[13] = loadImage("art/anims0xa.bmp", true);
	_anims[1] = loadImage("art/anims1.bmp", true);
	_animsa[1] = loadImage("art/anims1a.bmp", true);
	_anims[2] = loadImage("art/anims2.bmp", true);

	// huge
	_animset2[0].xofs = 8;
	_animset2[0].yofs = 7;
	_animset2[0].x = 123;
	_animset2[0].y = 0;
	_animset2[0].w = 18;
	_animset2[0].h = 16;
	// big
	_animset2[1].xofs = 7;
	_animset2[1].yofs = 7;
	_animset2[1].x = 107;
	_animset2[1].y = 0;
	_animset2[1].w = 16;
	_animset2[1].h = 14;
	// med
	_animset2[2].xofs = 6;
	_animset2[2].yofs = 6;
	_animset2[2].x = 93;
	_animset2[2].y = 0;
	_animset2[2].w = 14;
	_animset2[2].h = 13;
	// small
	_animset2[3].xofs = 4;
	_animset2[3].yofs = 4;
	_animset2[3].x = 83;
	_animset2[3].y = 0;
	_animset2[3].w = 10;
	_animset2[3].h = 10;
	// wing
	_animset2[4].xofs = 4;
	_animset2[4].yofs = 20;
	_animset2[4].x = 42;
	_animset2[4].y = 0;
	_animset2[4].w = 41;
	_animset2[4].h = 33;
	// head
	_animset2[5].xofs = 20;
	_animset2[5].yofs = 18;
	_animset2[5].x = 0;
	_animset2[5].y = 0;
	_animset2[5].w = 42;
	_animset2[5].h = 36;

	_anims[9] = loadImage("art/anims9.bmp", true);

	// huge
	_animset9[0].xofs = 8;
	_animset9[0].yofs = 7;
	_animset9[0].x = 154;
	_animset9[0].y = 0;
	_animset9[0].w = 18;
	_animset9[0].h = 16;
	// big
	_animset9[1].xofs = 7;
	_animset9[1].yofs = 7;
	_animset9[1].x = 138;
	_animset9[1].y = 0;
	_animset9[1].w = 16;
	_animset9[1].h = 14;
	// med
	_animset9[2].xofs = 6;
	_animset9[2].yofs = 6;
	_animset9[2].x = 93 + 31;
	_animset9[2].y = 0;
	_animset9[2].w = 14;
	_animset9[2].h = 13;
	// small
	_animset9[3].xofs = 4;
	_animset9[3].yofs = 4;
	_animset9[3].x = 83 + 31;
	_animset9[3].y = 0;
	_animset9[3].w = 10;
	_animset9[3].h = 10;
	// wing
	_animset9[4].xofs = 36;
	_animset9[4].yofs = 20;
	_animset9[4].x = 42;
	_animset9[4].y = 0;
	_animset9[4].w = 72;
	_animset9[4].h = 33;
	// head
	_animset9[5].xofs = 20;
	_animset9[5].yofs = 18;
	_animset9[5].x = 0;
	_animset9[5].y = 0;
	_animset9[5].w = 42;
	_animset9[5].h = 36;

	_anims[3] = loadImage("art/anims3.bmp", true);
	_anims[4] = loadImage("art/anims4.bmp", true);
	_anims[5] = loadImage("art/anims5.bmp", true);
	_anims[6] = loadImage("art/anims6.bmp", true);
	_anims[7] = loadImage("art/anims7.bmp", true);
	_anims[8] = loadImage("art/anims8.bmp", true);
	_anims[10] = loadImage("art/anims10.bmp", true);
	_animsa[10] = loadImage("art/anims10a.bmp", true);
	_anims[11] = loadImage("art/anims11.bmp", true);
	_animsa[11] = loadImage("art/anims11a.bmp", true);
	_anims[12] = loadImage("art/anims12.bmp", true);
}

void GriffonEngine::loadItemImgs() {
	Graphics::TransparentSurface *temp = loadImage("art/icons.bmp", true);

	for (int i = 0; i <= 20; i++) {
		_itemimg[i] = new Graphics::TransparentSurface;
		_itemimg[i]->create(16, 16, g_system->getScreenFormat());

		rcSrc.left = i * 16;
		rcSrc.top = 0;
		rcSrc.setWidth(16);
		rcSrc.setHeight(16);

		temp->blit(*_itemimg[i], 0, 0, Graphics::FLIP_NONE, &rcSrc);
	}

	temp->free();
}

void GriffonEngine::loadFont() {
	Graphics::TransparentSurface *font = loadImage("art/font.bmp", true);

	for (int i = 32; i <= 255; i++)
		for (int f = 0; f <= 4; f++) {
			int i2 = i - 32;

			_fontchr[i2][f] = new Graphics::TransparentSurface;
			_fontchr[i2][f]->create(8, 8, g_system->getScreenFormat());

			int col = i2 % 40;

			int row = (i2 - col) / 40;

			rcSrc.left = col * 8;
			rcSrc.top = row * 8 + f * 48;
			rcSrc.setWidth(8);
			rcSrc.setHeight(8);

			rcDest.left = 0;
			rcDest.top = 0;
			font->blit(*_fontchr[i2][f], rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
		}

	font->free();
}

void GriffonEngine::loadTiles() {
	_tiles[0] = loadImage("art/tx.bmp", true);
	_tiles[1] = loadImage("art/tx1.bmp", true);
	_tiles[2] = loadImage("art/tx2.bmp", true);
	_tiles[3] = loadImage("art/tx3.bmp", true);

	_windowimg = loadImage("art/window.bmp", true);
}

void GriffonEngine::loadTriggers() {
	Common::File file;
	file.open("data/triggers.dat");

	if (!file.isOpen())
		error("Cannot open file data/Triggers.dat");

	for (int i = 0; i <= 9999; i++) {
		for (int a = 0; a <= 8; a++)
			INPUT("%i", &_triggers[i][a]);
	}

	file.close();
}

void GriffonEngine::loadObjectDB() {
	Common::File file;

	file.open("objectdb.dat");
	if (!file.isOpen())
		error("Cannot open file objectdb.dat");

	for (int a = 0; a <= 32; a++) {
		for (int b = 0; b <= 5; b++) {
			INPUT("%i", &_objectInfo[a][b]);
		}

		for (int b = 0; b <= 8; b++) {
			for (int c = 0; c <= 2; c++) {
				for (int d = 0; d <= 2; d++) {
					for (int e = 0; e <= 1; e++) {
						INPUT("%i", &_objectTile[a][b][c][d][e]);
					}
				}
			}
		}
	}

	file.close();
}

void GriffonEngine::drawString(Graphics::TransparentSurface *buffer, const char *stri, int xloc, int yloc, int col) {
	int l = strlen(stri);

	for (int i = 0; i < l; i++) {
		rcDest.left = xloc + i * 8;
		rcDest.top = yloc;

		_fontchr[stri[i] - 32][col]->blit(*buffer, rcDest.left, rcDest.top);
	}
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
		_player.attackframe += _player.attackspd * _fpsr;
		if (_player.attackframe >= 16) {
			attacking = false;
			_player.attackframe = 0;
			_player.walkframe = 0;
		}

		int pa = (int)(_player.attackframe);

		for (int i = 0; i <= pa; i++) {
			if (ABS(_playerattackofs[_player.walkdir][i][2]) < kEpsilon) {
				_playerattackofs[_player.walkdir][i][2] = 1;

				float opx = _player.px;
				float opy = _player.py;

				_player.px = _player.px + _playerattackofs[_player.walkdir][i][0];
				_player.py = _player.py + _playerattackofs[_player.walkdir][i][1];

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

	if (_player.level == _player.maxlevel)
		_player.exp = 0;

	if (_player.exp >= _player.nextlevel) {
		_player.level = _player.level + 1;
		addFloatText("LEVEL UP!", _player.px + 16 - 36, _player.py + 16, 3);
		_player.exp = _player.exp - _player.nextlevel;
		_player.nextlevel = _player.nextlevel * 3 / 2; // 1.5
		_player.maxhp = _player.maxhp + _player.level * 3;
		if (_player.maxhp > 999)
			_player.maxhp = 999;
		_player.hp = _player.maxhp;

		_player.sworddamage = _player.level * 14 / 10;
		_player.spelldamage = _player.level * 13 / 10;

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
			if (_player.foundspell[i] == 1)
				_player.spellcharge[i] += 1 * _player.level * 0.01 * _fpsr;
			if (_player.spellcharge[i] > 100)
				_player.spellcharge[i] = 100;
		}

		if (_player.foundspell[0]) {
			_player.spellstrength += 3 * _player.level * .01 * _fpsr;
		}

		_player.attackstrength += (30 + 3 * (float)_player.level) / 50 * _fpsr;
	}

	if (_player.attackstrength > 100)
		_player.attackstrength = 100;

	if (_player.spellstrength > 100)
		_player.spellstrength = 100;

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
		if (config.effects && _player.hpflashb == 0 && _player.hp < _player.maxhp / 4) {
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

} // end of namespace Griffon
