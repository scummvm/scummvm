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
		if (_npcinfo[i].spriteset == kMonsterBabyDragon) {
			_npcinfo[i].hp = 12;
			_npcinfo[i].attackdelay = 2000;

			_npcinfo[i].attackdamage = 2;
			_npcinfo[i].spelldamage = 0;

			_npcinfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				_npcinfo[i].hp = 0;
		}

		// onewing
		if (_npcinfo[i].spriteset == kMonsterOneWing) {
			_npcinfo[i].hp = 200;
			_npcinfo[i].attackdelay = 2000;
			_npcinfo[i].swayspd = 1;

			_npcinfo[i].attackdamage = 24;
			_npcinfo[i].spelldamage = 30;

			_npcinfo[i].walkspd = 1.4f;
			_npcinfo[i].castpause = _ticks;
		}

		// boss1
		if (_npcinfo[i].spriteset == kMonsterBoss1) {
			_npcinfo[i].hp = 300;
			_npcinfo[i].attackdelay = 2200;

			_npcinfo[i].attackdamage = 0;
			_npcinfo[i].spelldamage = 30;

			_npcinfo[i].walkspd = 1.2f;
		}

		// black knights
		if (_npcinfo[i].spriteset == kMonsterBlackKnight) {
			_npcinfo[i].hp = 200;
			_npcinfo[i].attackdelay = 2800;

			_npcinfo[i].attackdamage = 0;
			_npcinfo[i].spelldamage = 30;

			_npcinfo[i].walkspd = 1;
		}

		// boss2 firehydra
		if (_npcinfo[i].spriteset == kMonsterFireHydra) {
			_npcinfo[i].hp = 600;
			_npcinfo[i].attackdelay = 2200;

			_npcinfo[i].attackdamage = 50;
			_npcinfo[i].spelldamage = 30;

			_npcinfo[i].walkspd = 1.3f;

			_npcinfo[i].swayangle = 0;
		}

		// baby fire dragon
		if (_npcinfo[i].spriteset == kMonsterRedDragon) {
			_npcinfo[i].hp = 20;
			_npcinfo[i].attackdelay = 1500;

			_npcinfo[i].attackdamage = 0;
			_npcinfo[i].spelldamage = 12;

			_npcinfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				_npcinfo[i].hp = 0;
		}

		// priest1
		if (_npcinfo[i].spriteset == kMonsterPriest) {
			_npcinfo[i].hp = 40;
			_npcinfo[i].attackdelay = 5000;

			_npcinfo[i].attackdamage = 0;
			_npcinfo[i].spelldamage = 8;

			_npcinfo[i].walkspd = 1;

			if (RND() * 8 == 0)
				_npcinfo[i].hp = 0;
		}

		// yellow fire dragon
		if (_npcinfo[i].spriteset == kMonsterYellowDragon) {
			_npcinfo[i].hp = 100;
			_npcinfo[i].attackdelay = 1500;

			_npcinfo[i].attackdamage = 0;
			_npcinfo[i].spelldamage = 24;

			_npcinfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				_npcinfo[i].hp = 0;
		}

		// twowing
		if (_npcinfo[i].spriteset == kMonsterTwoWing) {
			_npcinfo[i].hp = 140;
			_npcinfo[i].attackdelay = 2000;
			_npcinfo[i].swayspd = 1;

			_npcinfo[i].attackdamage = 30;
			_npcinfo[i].spelldamage = 0;

			_npcinfo[i].walkspd = 1;

			_npcinfo[i].castpause = 0;
		}

		// dragon2
		if (_npcinfo[i].spriteset == kMonsterDragon2) {
			_npcinfo[i].hp = 80;
			_npcinfo[i].attackdelay = 1500;

			_npcinfo[i].attackdamage = 24;
			_npcinfo[i].spelldamage = 0;

			_npcinfo[i].walkspd = 1;

			_npcinfo[i].floating = RND() * 16;
		}

		// end boss
		if (_npcinfo[i].spriteset == kMonsterFinalBoss) {
			_npcinfo[i].hp = 1200;
			_npcinfo[i].attackdelay = 2000;

			_npcinfo[i].attackdamage = 100;
			_npcinfo[i].spelldamage = 60;

			_npcinfo[i].walkspd = 1;

			_npcinfo[i].floating = RND() * 16;
		}

		// bat kitty
		if (_npcinfo[i].spriteset == kMonsterBatKitty) {
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

		if (_npcinfo[i].spriteset == kMonsterOneWing || _npcinfo[i].spriteset == kMonsterTwoWing) {
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

		if (_npcinfo[i].spriteset == kMonsterFireHydra) {
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

		if (_npcinfo[i].script == kScriptFindCrystal) {
			_roomlock = true;
			if (_scriptflag[kScriptFindCrystal][0] > 0) {
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
	if (_curmap == 24 && _player.foundSpell[0] == 0 && _scriptflag[kScriptFindCrystal][0] == 1) {
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


} // end of namespace Griffon
