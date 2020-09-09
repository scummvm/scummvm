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

#include "common/file.h"
#include "common/system.h"

#include "image/bmp.h"

#include "griffon/griffon.h"

namespace Griffon {

// HELPER MACRO ---------------------------------------------------------------
#define INPUT(A, B)                 \
	do {                            \
		Common::String line;        \
		line = file.readLine();     \
		sscanf(line.c_str(), A, B); \
	} while(0)


void GriffonEngine::initialize() {
	// init char *_floatstri[kMaxFloat]
	for (int i = 0; i < kMaxFloat; i++) {
		_floatText[i].text = (char *)malloc(64); // 64 bytes each string (should be enough)
		_floatText[i].framesLeft = 0;
		_floatText[i].x = 0;
		_floatText[i].y = 0;
		_floatText[i].col = 0;

		_floatIcon[i].framesLeft = 0;
		_floatIcon[i].x = 0;
		_floatIcon[i].y = 0;
		_floatIcon[i].ico = 0;
	}

	_video = new Graphics::TransparentSurface;
	_video->create(320, 240, g_system->getScreenFormat());
	_videoBuffer = new Graphics::TransparentSurface;
	_videoBuffer->create(320, 240, g_system->getScreenFormat());
	_videoBuffer2 = new Graphics::TransparentSurface;
	_videoBuffer2->create(320, 240, g_system->getScreenFormat());
	_videoBuffer3 = new Graphics::TransparentSurface;
	_videoBuffer3->create(320, 240, g_system->getScreenFormat());
	_mapBg = new Graphics::TransparentSurface;
	_mapBg->create(320, 240, g_system->getScreenFormat());
	_clipBg = new Graphics::TransparentSurface;
	_clipBg->create(320, 240, g_system->getScreenFormat());
	_clipBg2 = new Graphics::TransparentSurface;
	_clipBg2->create(320, 240, g_system->getScreenFormat());

	for (int i = 0; i <= 3; i++) {
		char name[128];

		sprintf(name, "art/map%i.bmp", i + 1);
		mapImg[i] = loadImage(name, true);
	}

	_cloudImg = loadImage("art/clouds.bmp", true);
	_cloudImg->setAlpha(64, true);

	_saveLoadImg = nullptr;

	_titleImg = loadImage("art/titleb.bmp");
	_titleImg2 = loadImage("art/titlea.bmp", true);
	//_titleimg2->setAlpha(204, true);

	_inventoryImg = loadImage("art/inventory.bmp", true);

	_logosImg = loadImage("art/logos.bmp");
	_theEndImg = loadImage("art/theend.bmp");


	loadTiles();
	loadTriggers();
	loadObjectDB();
	loadAnims();
	loadFont();
	loadItemImgs();

	_fpsr = 1.0f;
	_nextTicks = _ticks + 1000;

	for (int i = 0; i <= 15; i++) {
		_playerAttackOfs[0][i].x = 0; // -1// -(i + 1)
		_playerAttackOfs[0][i].y = -sin(3.14159 * 2 * (i + 1) / 16) * 2 - 1;

		_playerAttackOfs[1][i].x = 0; // i + 1
		_playerAttackOfs[1][i].y = -sin(3.14159 * 2 * (i + 1) / 16) * 2 + 1;

		_playerAttackOfs[2][i].x = -1; // -(i + 1)
		_playerAttackOfs[2][i].y = -sin(3.14159 * 2 * (i + 1) / 16) * 2;

		_playerAttackOfs[3][i].x = 1; // i + 1
		_playerAttackOfs[3][i].y = -sin(3.14159 * 2 * (i + 1) / 16) * 2;
	}

	setupAudio();
}

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

void GriffonEngine::loadMap(int mapnum) {
	debug(2, "Loaded map %d", mapnum);

	unsigned int ccc = _clipBg->format.RGBToColor(255, 255, 255);

	_curMap = mapnum;
	Common::Rect trect(320, 240);

	_mapBg->fillRect(trect, 0);
	_clipBg->fillRect(trect, ccc);
	_clipBg2->fillRect(trect, ccc);

	_forcePause = false;
	_cloudsOn = false;
	if (mapnum < 6)
		_cloudsOn = true;
	if (mapnum > 41)
		_cloudsOn = true;
	if (mapnum > 47)
		_cloudsOn = false;
	if (mapnum == 52)
		_cloudsOn = true;
	if (mapnum == 60)
		_cloudsOn = true;
	if (mapnum == 50)
		_cloudsOn = true;
	if (mapnum == 54)
		_cloudsOn = true;
	if (mapnum == 58)
		_cloudsOn = true;
	if (mapnum == 62)
		_cloudsOn = true;
	if (mapnum == 83)
		_cloudsOn = true;

	// -----------special case
	_dontDrawOver = false;
	if (mapnum == 24)
		_dontDrawOver = true;

	if ((mapnum == 53 || mapnum == 57 || mapnum == 61 || mapnum == 65 || mapnum == 62) && _scriptFlag[kScriptLever][0] > 0)
		mapnum = mapnum + 100;
	if ((mapnum == 161 || mapnum == 162) && _scriptFlag[kScriptLever][0] == 2)
		mapnum = mapnum + 100;

	for (int i = 0; i < kMaxSpell; i++)
		_spellInfo[i].frame = 0;

	_roomLock = false;
	_saidLocked = false;
	_saidJammed = false;
	_itemSelOn = false;
	_selEnemyOn = false;
	_curItem = 0;
	_player.itemselshade = 0;

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
			_triggerLoc[x][y] = -1;
	}

	// read *.trg file
	sprintf(name, "mapdb/%04i.trg", mapnum);
	debug(1, "Reading %s", name);
	file.open(name);

	if (!file.isOpen())
		error("Cannot open file %s", name);

	INPUT("%i", &_triggerNbr);

	for (int i = 0; i < _triggerNbr; i++) {
		int mapx, mapy, trig;

		INPUT("%i", &mapx);
		INPUT("%i", &mapy);
		INPUT("%i", &trig);

		_triggerLoc[mapx][mapy] = trig;
	}
	file.close();


	for (int y = 0; y <= 23; y++) {
		for (int x = 0; x <= 39; x++)
			_rampData[x][y] = tempmap[3 * 40 + x][y + 40];
	}

	for (int y = 0; y <= 23; y++) {
		for (int x = 0; x <= 39; x++) {
			for (int l = 0; l <= 2; l++) {
				for (int a = 0; a <= 2; a++)
					_tileinfo[l][x][y][a] = 0;
			}
		}
	}

	if (_scriptFlag[kScriptFindShield][0] == 1 && _curMap == 4) {
		_triggerLoc[9][7] = 5004;
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

					int alpha = 255;
					if (l == 2 && curtilel == 1) {
						for (int ff = 0; ff <= 5; ff++) {
							int ffa = 20 * 5 - 1 + ff * 20;
							int ffb = 20 * 5 + 4 + ff * 20;
							if (curtile > ffa && curtile < ffb) {
								alpha = 128;
							}
						}
					}
					if (l == 1 && curtilel == 2) {
						for (int ff = 0; ff <= 4; ff++) {
							int ffa = 20 * (5 + ff) + 3;
							if (curtile == ffa) {
								alpha = 192;
							}
						}
					}

					_tiles[curtilel]->blit(*_mapBg, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc, TS_ARGB(alpha, 255, 255, 255));

					rcDest.left = x * 8;
					rcDest.top = y * 8;
					rcDest.setWidth(8);
					rcDest.setHeight(8);

					_clipBg->fillRect(rcDest, 0);
				}
			}
		}
	}

	for (int x = 0; x <= 39; x++) {
		for (int y = 0; y <= 23; y++) {
			int d = tempmap[3 * 40 + x][y];

			if (_scriptFlag[kScriptFindShield][0] == 1 && x == 9 && y == 7)
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
							drawLine(_clipBg, x1, y1 + i, x1 + 7 - i, y1 + i, ccc);
						}
					} else if (d == 2) {
						drawLine(_clipBg, x1, y1, x1 + 7, y1, ccc);
						drawLine(_clipBg, x1, y1 + 1, x1 + 7, y1 + 1, ccc);
					} else if (d == 3) {
						for (int i = 0; i <= 7; i++) {
							drawLine(_clipBg, x1 + i, y1 + i, x1 + 7, y1 + i, ccc);
						}
					} else if (d == 4) {
						drawLine(_clipBg, x1, y1, x1, y1 + 7, ccc);
						drawLine(_clipBg, x1 + 1, y1, x1 + 1, y1 + 7, ccc);
					} else if (d == 5) {
						rcDest.left = x1;
						rcDest.top = y1;
						rcDest.setWidth(8);
						rcDest.setHeight(8);
						_clipBg->fillRect(rcDest, ccc);
					} else if (d == 6) {
						drawLine(_clipBg, x1 + 7, y1, x1 + 7, y1 + 7, ccc);
						drawLine(_clipBg, x1 + 6, y1, x1 + 6, y1 + 7, ccc);
					} else if (d == 7) {
						for (int i = 0; i <= 7; i++) {
							drawLine(_clipBg, x1, y1 + i, x1 + i, y1 + i, ccc);
						}
					} else if (d == 8) {
						drawLine(_clipBg, x1, y1 + 7, x1 + 7, y1 + 7, ccc);
						drawLine(_clipBg, x1, y1 + 7, x1 + 6, y1 + 6, ccc);
					} else if (d == 9) {
						for (int i = 0; i <= 7; i++) {
							drawLine(_clipBg, x1 + 7 - i, y1 + i, x1 + 7, y1 + i, ccc);
						}
					}
				}
			}
		}
	}

	_lastObj = 0;
	_lastNpc = 0;

	for (int i = 0; i < kMaxNPC; i++)
		_npcInfo[i].onMap = false;

	for (int x = 0; x <= 20; x++) {
		for (int y = 0; y <= 14; y++) {
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

				if (_objectMapFull[_curMap][x][y] == 0) {
					_objectMap[x][y] = o;

					if (_objectInfo[o].nFrames > 1) {
						if (o > _lastObj)
							_lastObj = o;
					}

					int x1 = x * 8;
					int y1 = y * 8;

					rcDest.left = x1;
					rcDest.top = y1;
					rcDest.setWidth(8);
					rcDest.setHeight(8);

					if (_objectInfo[o].type == 1)
						_clipBg->fillRect(rcDest, ccc);
					if (_objectInfo[o].type == 3)
						_clipBg->fillRect(rcDest, ccc);
				}
			}
			if (npc == 1) {
				int o = tempmap[4 * 40 + x][y];

				if (o > _lastNpc)
					_lastNpc = o;

				_npcInfo[o].x = x * 16 - 4;
				_npcInfo[o].y = y * 16 - 5;

				_npcInfo[o].walkdir = 1;
				_npcInfo[o].onMap = true;
			}
		}
	}


	if (_curMap == 62 && _scriptFlag[kScriptGardenMasterKey][0] > 0)
		_lastNpc = 0;
	if (_curMap == 73 && _scriptFlag[kScriptArmourChest][0] > 0)
		_lastNpc = 0;
	if (_curMap == 81 && _scriptFlag[kScriptCitadelMasterKey][0] > 0)
		_lastNpc = 0;

	if (_curMap == 73 && _scriptFlag[kScriptArmourChest][0] == 0)
		_roomLock = true;
	if (_curMap == 81 && _scriptFlag[kScriptCitadelMasterKey][0] == 0)
		_roomLock = true;
	if (_curMap == 83 && _scriptFlag[kScriptGetSword3][0] == 0)
		_roomLock = true;
	if (_curMap == 82)
		_roomLock = true;

	// read *.npc file
	sprintf(name, "mapdb/%04i.npc", mapnum);
	debug(1, "Reading %s", name);
	file.open(name);

	if (!file.isOpen())
		error("Cannot open file %s", name);

	for (int i = 0; i < kMaxNPC; i++) {
		INPUT("%i", &_npcInfo[i].spriteset);
		INPUT("%i", &_npcInfo[i].x1);
		INPUT("%i", &_npcInfo[i].y1);
		INPUT("%i", &_npcInfo[i].x2);
		INPUT("%i", &_npcInfo[i].y2);
		INPUT("%i", &_npcInfo[i].movementmode);
		INPUT("%i", &_npcInfo[i].hp);
		INPUT("%i", &_npcInfo[i].item1);
		INPUT("%i", &_npcInfo[i].item2);
		INPUT("%i", &_npcInfo[i].item3);
		INPUT("%i", &_npcInfo[i].script);

		_npcInfo[i].cframe = 0;
		_npcInfo[i].frame = 0;
		_npcInfo[i].frame2 = 0;
		_npcInfo[i].attackattempt = 0;
		_npcInfo[i].ticks = 0;

		// baby dragon
		if (_npcInfo[i].spriteset == kMonsterBabyDragon) {
			_npcInfo[i].hp = 12;
			_npcInfo[i].attackdelay = 2000;

			_npcInfo[i].attackDamage = 2;
			_npcInfo[i].spellDamage = 0;

			_npcInfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				_npcInfo[i].hp = 0;
		}

		// onewing
		if (_npcInfo[i].spriteset == kMonsterOneWing) {
			_npcInfo[i].hp = 200;
			_npcInfo[i].attackdelay = 2000;
			_npcInfo[i].swaySpeed = 1;

			_npcInfo[i].attackDamage = 24;
			_npcInfo[i].spellDamage = 30;

			_npcInfo[i].walkspd = 1.4f;
			_npcInfo[i].castPause = _ticks;
		}

		// boss1
		if (_npcInfo[i].spriteset == kMonsterBoss1) {
			_npcInfo[i].hp = 300;
			_npcInfo[i].attackdelay = 2200;

			_npcInfo[i].attackDamage = 0;
			_npcInfo[i].spellDamage = 30;

			_npcInfo[i].walkspd = 1.2f;
		}

		// black knights
		if (_npcInfo[i].spriteset == kMonsterBlackKnight) {
			_npcInfo[i].hp = 200;
			_npcInfo[i].attackdelay = 2800;

			_npcInfo[i].attackDamage = 0;
			_npcInfo[i].spellDamage = 30;

			_npcInfo[i].walkspd = 1;
		}

		// boss2 firehydra
		if (_npcInfo[i].spriteset == kMonsterFireHydra) {
			_npcInfo[i].hp = 600;
			_npcInfo[i].attackdelay = 2200;

			_npcInfo[i].attackDamage = 50;
			_npcInfo[i].spellDamage = 30;

			_npcInfo[i].walkspd = 1.3f;

			_npcInfo[i].swayAngle = 0;
		}

		// baby fire dragon
		if (_npcInfo[i].spriteset == kMonsterRedDragon) {
			_npcInfo[i].hp = 20;
			_npcInfo[i].attackdelay = 1500;

			_npcInfo[i].attackDamage = 0;
			_npcInfo[i].spellDamage = 12;

			_npcInfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				_npcInfo[i].hp = 0;
		}

		// priest1
		if (_npcInfo[i].spriteset == kMonsterPriest) {
			_npcInfo[i].hp = 40;
			_npcInfo[i].attackdelay = 5000;

			_npcInfo[i].attackDamage = 0;
			_npcInfo[i].spellDamage = 8;

			_npcInfo[i].walkspd = 1;

			if (RND() * 8 == 0)
				_npcInfo[i].hp = 0;
		}

		// yellow fire dragon
		if (_npcInfo[i].spriteset == kMonsterYellowDragon) {
			_npcInfo[i].hp = 100;
			_npcInfo[i].attackdelay = 1500;

			_npcInfo[i].attackDamage = 0;
			_npcInfo[i].spellDamage = 24;

			_npcInfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				_npcInfo[i].hp = 0;
		}

		// twowing
		if (_npcInfo[i].spriteset == kMonsterTwoWing) {
			_npcInfo[i].hp = 140;
			_npcInfo[i].attackdelay = 2000;
			_npcInfo[i].swaySpeed = 1;

			_npcInfo[i].attackDamage = 30;
			_npcInfo[i].spellDamage = 0;

			_npcInfo[i].walkspd = 1;

			_npcInfo[i].castPause = 0;
		}

		// dragon2
		if (_npcInfo[i].spriteset == kMonsterDragon2) {
			_npcInfo[i].hp = 80;
			_npcInfo[i].attackdelay = 1500;

			_npcInfo[i].attackDamage = 24;
			_npcInfo[i].spellDamage = 0;

			_npcInfo[i].walkspd = 1;

			_npcInfo[i].floating = RND() * 16;
		}

		// end boss
		if (_npcInfo[i].spriteset == kMonsterFinalBoss) {
			_npcInfo[i].hp = 1200;
			_npcInfo[i].attackdelay = 2000;

			_npcInfo[i].attackDamage = 100;
			_npcInfo[i].spellDamage = 60;

			_npcInfo[i].walkspd = 1;

			_npcInfo[i].floating = RND() * 16;
		}

		// bat kitty
		if (_npcInfo[i].spriteset == kMonsterBatKitty) {
			_npcInfo[i].hp = 800;
			_npcInfo[i].attackdelay = 2000;

			_npcInfo[i].attackDamage = 100;
			_npcInfo[i].spellDamage = 50;

			_npcInfo[i].walkspd = 1;

			_npcInfo[i].floating = RND() * 16;
		}

		if (!_npcInfo[i].onMap)
			_npcInfo[i].hp = 0;

		_npcInfo[i].maxhp = _npcInfo[i].hp;

		_npcInfo[i].attacking = false;
		_npcInfo[i].attackframe = 0;
		_npcInfo[i].cattackframe = 0;
		_npcInfo[i].attackspd = 1.5;
		_npcInfo[i].attacknext = _ticks + _npcInfo[i].attackdelay * (1 + RND() * 2);

		if (_npcInfo[i].spriteset == kMonsterOneWing || _npcInfo[i].spriteset == kMonsterTwoWing) {
			_npcInfo[i].bodysection[0].sprite = 0;
			_npcInfo[i].bodysection[1].sprite = 1;
			_npcInfo[i].bodysection[2].sprite = 2;
			_npcInfo[i].bodysection[3].sprite = 3;
			_npcInfo[i].bodysection[4].sprite = 4;
			_npcInfo[i].bodysection[5].sprite = 3;
			_npcInfo[i].bodysection[6].sprite = 3;
			_npcInfo[i].bodysection[7].sprite = 5;

			_npcInfo[i].bodysection[0].bonelength = 8;
			_npcInfo[i].bodysection[1].bonelength = 7;
			_npcInfo[i].bodysection[2].bonelength = 6;
			_npcInfo[i].bodysection[3].bonelength = 4;
			_npcInfo[i].bodysection[4].bonelength = 4;
			_npcInfo[i].bodysection[5].bonelength = 4;
			_npcInfo[i].bodysection[6].bonelength = 4;

			for (int f = 0; f <= 7; f++) {
				_npcInfo[i].bodysection[f].x = _npcInfo[i].x + 12;
				_npcInfo[i].bodysection[f].y = _npcInfo[i].y + 14;
			}

			_npcInfo[i].headTargetX[0] = _npcInfo[i].x + 12;
			_npcInfo[i].headTargetY[0] = _npcInfo[i].y + 14;

		}

		if (_npcInfo[i].spriteset == kMonsterFireHydra) {
			for (int f = 0; f <= 29; f++) {
				_npcInfo[i].bodysection[f].x = _npcInfo[i].x + 12;
				_npcInfo[i].bodysection[f].y = _npcInfo[i].y + 14;
			}

			for (int f = 0; f <= 2; f++) {
				_npcInfo[i].headTargetX[f] = _npcInfo[i].x + 12;
				_npcInfo[i].headTargetY[f] = _npcInfo[i].y + 14;

				_npcInfo[i].attacking2[f] = false;
				_npcInfo[i].attackframe2[f] = 0;
			}
		}

		if (_npcInfo[i].script == kScriptMasterKey) {
			_roomLock = true;
			if (_scriptFlag[kScriptMasterKey][0] > 0) {
				_roomLock = false;
				_npcInfo[i].hp = 0;
			}
		}

		if (_npcInfo[i].script == kScriptFindCrystal) {
			_roomLock = true;
			if (_scriptFlag[kScriptFindCrystal][0] > 0) {
				_roomLock = false;
				_npcInfo[i].hp = 0;
			}
		}

		if (_npcInfo[i].script == kScriptFindSword) {
			_roomLock = true;
			if (_scriptFlag[kScriptFindSword][0] > 0) {
				_roomLock = false;
				_npcInfo[i].hp = 0;
			}
		}

		if (_npcInfo[i].script == kScriptGetSword3) {
			_roomLock = true;
			if (_scriptFlag[kScriptGetSword3][0] > 0) {
				_roomLock = false;
				_npcInfo[i].hp = 0;
			}
		}

		_npcInfo[i].pause = _ticks;
	}

	file.close();


	int cx, cy, npx, npy, lx, ly;

	// academy master key
	if (_curMap == 34 && _scriptFlag[kScriptMasterKey][0] == 1) {
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

		_clipBg->fillRect(rcDest, ccc);
	}

	// academy crystal
	if (_curMap == 24 && !_player.foundSpell[0] && _scriptFlag[kScriptFindCrystal][0] == 1) {
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

		_clipBg->fillRect(rcDest, ccc);
	}

	// gardens master key
	if (_curMap == 62 && _scriptFlag[kScriptGardenMasterKey][0] == 1) {
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

		_clipBg->fillRect(rcDest, ccc);
	}

	// gardens fidelis sword
	if (_curMap == 66 && _scriptFlag[kScriptFindSword][0] == 1 && _player.sword == 1) {
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

		_clipBg->fillRect(rcDest, ccc);
	}

	// citadel armour
	if (_curMap == 73 && _scriptFlag[kScriptArmourChest][0] == 1 && _player.armour == 1) {
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

		_clipBg->fillRect(rcDest, ccc);
	}

	// citadel master key
	if (_curMap == 81 && _scriptFlag[kScriptCitadelMasterKey][0] == 1) {
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

		_clipBg->fillRect(rcDest, ccc);
	}


	// max ups
	if (_curMap == 83 && _scriptFlag[kScriptGetSword3][0] == 1 && _player.sword < 3) {
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

		_clipBg->fillRect(rcDest, ccc);
	}

	if (_curMap == 83 && _scriptFlag[kScriptShield3][0] == 1 && _player.shield < 3) {
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

		_clipBg->fillRect(rcDest, ccc);
	}

	if (_curMap == 83 && _scriptFlag[kScriptArmour3][0] == 1 && _player.armour < 3) {
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

		_clipBg->fillRect(rcDest, ccc);
	}

	_clipBg2->copyRectToSurface(_clipBg->getPixels(), _clipBg->pitch, 0, 0, _clipBg->w, _clipBg->h);
}

void GriffonEngine::loadAnims() {
	_spellImg = loadImage("art/spells.bmp", true);
	_anims[0] = loadImage("art/anims0.bmp", true);
	_animsAttack[0] = loadImage("art/anims0a.bmp", true);
	_anims[13] = loadImage("art/anims0x.bmp", true);
	_animsAttack[13] = loadImage("art/anims0xa.bmp", true);
	_anims[1] = loadImage("art/anims1.bmp", true);
	_animsAttack[1] = loadImage("art/anims1a.bmp", true);
	_anims[2] = loadImage("art/anims2.bmp", true);

	// huge
	_animSet2[0].xofs = 8;
	_animSet2[0].yofs = 7;
	_animSet2[0].x = 123;
	_animSet2[0].y = 0;
	_animSet2[0].w = 18;
	_animSet2[0].h = 16;
	// big
	_animSet2[1].xofs = 7;
	_animSet2[1].yofs = 7;
	_animSet2[1].x = 107;
	_animSet2[1].y = 0;
	_animSet2[1].w = 16;
	_animSet2[1].h = 14;
	// med
	_animSet2[2].xofs = 6;
	_animSet2[2].yofs = 6;
	_animSet2[2].x = 93;
	_animSet2[2].y = 0;
	_animSet2[2].w = 14;
	_animSet2[2].h = 13;
	// small
	_animSet2[3].xofs = 4;
	_animSet2[3].yofs = 4;
	_animSet2[3].x = 83;
	_animSet2[3].y = 0;
	_animSet2[3].w = 10;
	_animSet2[3].h = 10;
	// wing
	_animSet2[4].xofs = 4;
	_animSet2[4].yofs = 20;
	_animSet2[4].x = 42;
	_animSet2[4].y = 0;
	_animSet2[4].w = 41;
	_animSet2[4].h = 33;
	// head
	_animSet2[5].xofs = 20;
	_animSet2[5].yofs = 18;
	_animSet2[5].x = 0;
	_animSet2[5].y = 0;
	_animSet2[5].w = 42;
	_animSet2[5].h = 36;

	_anims[9] = loadImage("art/anims9.bmp", true);

	// huge
	_animSet9[0].xofs = 8;
	_animSet9[0].yofs = 7;
	_animSet9[0].x = 154;
	_animSet9[0].y = 0;
	_animSet9[0].w = 18;
	_animSet9[0].h = 16;
	// big
	_animSet9[1].xofs = 7;
	_animSet9[1].yofs = 7;
	_animSet9[1].x = 138;
	_animSet9[1].y = 0;
	_animSet9[1].w = 16;
	_animSet9[1].h = 14;
	// med
	_animSet9[2].xofs = 6;
	_animSet9[2].yofs = 6;
	_animSet9[2].x = 93 + 31;
	_animSet9[2].y = 0;
	_animSet9[2].w = 14;
	_animSet9[2].h = 13;
	// small
	_animSet9[3].xofs = 4;
	_animSet9[3].yofs = 4;
	_animSet9[3].x = 83 + 31;
	_animSet9[3].y = 0;
	_animSet9[3].w = 10;
	_animSet9[3].h = 10;
	// wing
	_animSet9[4].xofs = 36;
	_animSet9[4].yofs = 20;
	_animSet9[4].x = 42;
	_animSet9[4].y = 0;
	_animSet9[4].w = 72;
	_animSet9[4].h = 33;
	// head
	_animSet9[5].xofs = 20;
	_animSet9[5].yofs = 18;
	_animSet9[5].x = 0;
	_animSet9[5].y = 0;
	_animSet9[5].w = 42;
	_animSet9[5].h = 36;

	_anims[3] = loadImage("art/anims3.bmp", true);
	_anims[4] = loadImage("art/anims4.bmp", true);
	_anims[5] = loadImage("art/anims5.bmp", true);
	_anims[6] = loadImage("art/anims6.bmp", true);
	_anims[7] = loadImage("art/anims7.bmp", true);
	_anims[8] = loadImage("art/anims8.bmp", true);
	_anims[10] = loadImage("art/anims10.bmp", true);
	_animsAttack[10] = loadImage("art/anims10a.bmp", true);
	_anims[11] = loadImage("art/anims11.bmp", true);
	_animsAttack[11] = loadImage("art/anims11a.bmp", true);
	_anims[12] = loadImage("art/anims12.bmp", true);
}

void GriffonEngine::loadItemImgs() {
	Graphics::TransparentSurface *temp = loadImage("art/icons.bmp", true);

	for (int i = 0; i <= 20; i++) {
		_itemImg[i] = new Graphics::TransparentSurface;
		_itemImg[i]->create(16, 16, g_system->getScreenFormat());

		rcSrc.left = i * 16;
		rcSrc.top = 0;
		rcSrc.setWidth(16);
		rcSrc.setHeight(16);

		temp->blit(*_itemImg[i], 0, 0, Graphics::FLIP_NONE, &rcSrc);
	}

	temp->free();
}

void GriffonEngine::loadFont() {
	Graphics::TransparentSurface *font = loadImage("art/font.bmp", true);

	for (int i = 32; i <= 255; i++)
		for (int f = 0; f <= 4; f++) {
			int i2 = i - 32;

			_fontChr[i2][f] = new Graphics::TransparentSurface;
			_fontChr[i2][f]->create(8, 8, g_system->getScreenFormat());

			int col = i2 % 40;

			int row = (i2 - col) / 40;

			rcSrc.left = col * 8;
			rcSrc.top = row * 8 + f * 48;
			rcSrc.setWidth(8);
			rcSrc.setHeight(8);

			rcDest.left = 0;
			rcDest.top = 0;
			font->blit(*_fontChr[i2][f], rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
		}

	font->free();
}

void GriffonEngine::loadTiles() {
	_tiles[0] = loadImage("art/tx.bmp", true);
	_tiles[1] = loadImage("art/tx1.bmp", true);
	_tiles[2] = loadImage("art/tx2.bmp", true);
	_tiles[3] = loadImage("art/tx3.bmp", true);

	_windowImg = loadImage("art/window.bmp", true);
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
		INPUT("%i", &_objectInfo[a].nFrames);
		INPUT("%i", &_objectInfo[a].xTiles);
		INPUT("%i", &_objectInfo[a].yTiles);
		INPUT("%i", &_objectInfo[a].speed);
		INPUT("%i", &_objectInfo[a].type);
		INPUT("%i", &_objectInfo[a].script);

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


} // end of namespace Griffon
