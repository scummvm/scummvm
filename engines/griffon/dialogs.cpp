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

namespace Griffon {

#define MINCURSEL 7
#define MAXCURSEL 14
#define SY 22

void GriffonEngine::title(int mode) {
	float xofs = 0;

	rcSrc.left = 0;
	rcSrc.top = 0;
	rcSrc.setWidth(320);
	rcSrc.setHeight(240);

	_videobuffer2->fillRect(rcSrc, 0);
	_videobuffer3->fillRect(rcSrc, 0);

	_ticks = g_system->getMillis();

	_videobuffer->blit(*_videobuffer3);
	_videobuffer->blit(*_videobuffer2);

	int cursel = 0;
	int keypause = _ticks + 220;
	int _ticks1 = _ticks;

	if (config.music) {
		setChannelVolume(_musicchannel, 0);
		pauseSoundChannel(_musicchannel);

		_menuchannel = playSound(_mmenu, true);
		setChannelVolume(_menuchannel, config.musicvol);
		_pmenu = true;
	}

	bool ldstop = false;

	float ld = 0;
	do {
		Common::Rect rc;

		ld += 4.0 * _fpsr;
		if (ld > config.musicvol)
			ld = config.musicvol;
		if (!ldstop) {
			setChannelVolume(_menuchannel, (int)ld);
			if ((int)ld == config.musicvol)
				ldstop = true;
		}

		rc.left = -xofs;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		rc.left = -xofs + 320.0;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		rc.left = 0;
		rc.top = 0;

		_titleimg2->blit(*_videobuffer, rc.left, rc.top);

		int y = 172;
		int x = 160 - 14 * 4;

		drawString(_videobuffer, "new game/save/load", x, y, 4);
		drawString(_videobuffer, "options", x, y + 16, 4);
		drawString(_videobuffer, "quit game", x, y + 32, 4);

		if (mode == 1)
			drawString(_videobuffer, "return", x, y + 48, 4);
		else
			drawString(_videobuffer, "(c) 2005 by Daniel 'Syn9' Kennedy", 28, 224, 4);

		rc.left = (float)(x - 16 - 4 * cos(3.14159 * 2 * _itemyloc / 16));
		rc.top = (float)(y - 4 + 16 * cursel);

		_itemimg[15]->blit(*_videobuffer, rc.left, rc.top);

		float yf = 255.0;
		if (_ticks < _ticks1 + 1000) {
			yf = 255.0 * ((float)(_ticks - _ticks1) / 1000.0);
			yf = CLIP<float>(yf, 0.0, 255.0);
		}

		_videobuffer->setAlpha((int)yf);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

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

		float add = 0.5 * _fpsr;
		if (add > 1)
			add = 1;

		xofs = xofs + add;
		if (xofs >= 320.0)
			xofs -= 320.0;

		_itemyloc += 0.75 * _fpsr;

		while (_itemyloc >= 16)
			_itemyloc = _itemyloc - 16;

		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_QUIT)
			_shouldQuit = true;

		if (keypause < _ticks) {
			if (_event.type == Common::EVENT_KEYDOWN) {
				keypause = _ticks + 150;

				if (_event.kbd.keycode == Common::KEYCODE_ESCAPE && mode == 1)
					break;
				else if (_event.kbd.keycode == Common::KEYCODE_UP) {
					cursel--;
					if (cursel < 0)
						cursel = (mode == 1 ? 3 : 2);
				} else if (_event.kbd.keycode == Common::KEYCODE_DOWN) {
					cursel++;
					if (cursel >= (mode == 1 ? 4 : 3))
						cursel = 0;
				} else if (_event.kbd.keycode == Common::KEYCODE_RETURN) {
					if (cursel == 0) {
						saveLoadNew();
						_ticks = g_system->getMillis();
						keypause = _ticks + 150;
						_ticks1 = _ticks;
					} else if (cursel == 1) {
						configMenu();
						_ticks = g_system->getMillis();
						keypause = _ticks + 150;
						_ticks1 = _ticks;
					} else if (cursel == 2) {
						_shouldQuit = true;
					} else if (cursel == 3) {
						break;
					}
				}
			}
		}

		g_system->delayMillis(10);
	} while (!_shouldQuit);

	_itemticks = _ticks + 210;

	if (config.music) {
		haltSoundChannel(_menuchannel);
		resumeSoundChannel(_musicchannel);
		setChannelVolume(_musicchannel, config.musicvol);
		_pmenu = false;
	}
}

void GriffonEngine::configMenu() {
	int cursel = MINCURSEL;

	int tickwait = 1000 / 60;

	_ticks = g_system->getMillis();
	int keypause = _ticks + tickwait;

	Graphics::TransparentSurface *configwindow = loadImage("art/configwindow.bmp", true);
	configwindow->setAlpha(160, true);

	int _ticks1 = _ticks;
	do {
		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);

		rcDest.left = 256 + 256 * cos(3.141592 / 180 * clouddeg * 40);
		rcDest.top = 192 + 192 * sin(3.141592 / 180 * clouddeg * 40);
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		cloudimg->setAlpha(128, true);
		cloudimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, &rcDest);
		cloudimg->setAlpha(64, true);

		rcDest.left = 256;
		rcDest.top = 192;
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		cloudimg->setAlpha(128, true);
		cloudimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, &rcDest);
		cloudimg->setAlpha(64, true);

		_videobuffer->copyRectToSurface(configwindow->getPixels(), configwindow->pitch, 0, 0, configwindow->w, configwindow->h);

		int sy = SY;

		for (int i = 0; i <= 21; i++) {
			static const char *vr[22] = {
				"", "",
				"", "", "", "",
				"", "", "",
				"Music:", "", "",
				"Sound Effects:", "", "",
				"Music Volume:", "",
				"Effects Volume:", "", "", "", ""
			};
			static const char *vl[22] = {
				"", "",
				"", "", "", "",
				"", "", "",
				"On", "Off", "",
				"On", "Off", "",
				"[----------]", "",
				"[----------]", "",
				"Exit + Save", "",
				"Exit"
			};
			static char line[24];

			if (i == 15 || i == 17) {
				int vol = (i == 15 ? config.musicvol : config.effectsvol) * 9 / 255;
				vol = CLIP(vol, 0, 9);

				strcpy(line, "[----------]");
				line[vol + 1] = 'X';
				vl[i] = line;
			}

			int cl = 3;

			if (i == 9 && config.music)
				cl = 0;
			else if (i == 10 && !config.music)
				cl = 0;
			else if (i == 12 && config.effects)
				cl = 0;
			else if (i == 13 && !config.effects)
				cl = 0;
			else if (i > 18)
				cl = 0;

			drawString(_videobuffer, vr[i], 156 - 8 * strlen(vr[i]), sy + i * 8, 0);
			drawString(_videobuffer, vl[i], 164, sy + i * 8, cl);
		}

		int curselt = cursel + 2;
		if (cursel > 8)
			curselt += 1;
		if (cursel > 10)
			curselt += 1;
		if (cursel > 11)
			curselt += 1;
		if (cursel > 12)
			curselt += 1;
		if (cursel > 13)
			curselt += 1;

		Common::Rect rc;
		rc.left = 148 + 3 * cos(3.14159 * 2 * _itemyloc / 16.0);
		rc.top = sy + 8 * curselt - 4;

		_itemimg[15]->blit(*_videobuffer, rc.left, rc.top);

		float yy = 255.0;
		if (_ticks < _ticks1 + 1000) {
			yy = 255.0 * ((float)(_ticks - _ticks1) / 1000.0);
			yy = CLIP<float>(yy, 0.0, 255.0);
		}

		_videobuffer->setAlpha((int)yy);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		_itemyloc += 0.75 * _fpsr;
		while (_itemyloc >= 16)
			_itemyloc -= 16;

		if (keypause < _ticks) {
			g_system->getEventManager()->pollEvent(_event);

			if (_event.type == Common::EVENT_QUIT)
				_shouldQuit = true;

			if (_event.type == Common::EVENT_KEYDOWN) {
				keypause = _ticks + tickwait;

				if (_event.kbd.keycode == Common::KEYCODE_ESCAPE)
					break;

				if (_event.kbd.keycode == Common::KEYCODE_LEFT) {
					if (cursel == 11) {
						config.musicvol = config.musicvol - 25;
						if (config.musicvol < 0)
							config.musicvol = 0;

						setChannelVolume(_musicchannel, config.musicvol);
						setChannelVolume(_menuchannel, config.musicvol);
					} else if (cursel == 12) {
						config.effectsvol = config.effectsvol - 25;
						if (config.effectsvol < 0)
							config.effectsvol = 0;

						setChannelVolume(-1, config.effectsvol);
						setChannelVolume(_musicchannel, config.musicvol);
						setChannelVolume(_menuchannel, config.musicvol);

						if (config.effects) {
							int snd = playSound(_sfx[kSndDoor]);
							setChannelVolume(snd, config.effectsvol);
						}
					}
				}
				if (_event.kbd.keycode == Common::KEYCODE_RIGHT) {
					if (cursel == 11) {
						config.musicvol = config.musicvol + 25;
						if (config.musicvol > 255)
							config.musicvol = 255;

						setChannelVolume(_musicchannel, config.musicvol);
						setChannelVolume(_menuchannel, config.musicvol);
					} else if (cursel == 12) {
						config.effectsvol = config.effectsvol + 25;
						if (config.effectsvol > 255)
							config.effectsvol = 255;

						setChannelVolume(-1, config.effectsvol);
						setChannelVolume(_musicchannel, config.musicvol);
						setChannelVolume(_menuchannel, config.musicvol);

						if (config.effects) {
							int snd = playSound(_sfx[kSndDoor]);
							setChannelVolume(snd, config.effectsvol);
						}
					}
				}

				if (_event.kbd.keycode == Common::KEYCODE_UP) {
					cursel--;
					if (cursel < MINCURSEL)
						cursel = 14;
				}
				if (_event.kbd.keycode == Common::KEYCODE_DOWN) {
					cursel++;
					if (cursel > 14)
						cursel = MINCURSEL;
				}

				if (_event.kbd.keycode == Common::KEYCODE_RETURN) {
					if (cursel == 7 && !config.music) {
						config.music = true;
						_menuchannel = playSound(_mmenu, true);
						setChannelVolume(_menuchannel, config.musicvol);
					}
					if (cursel == 8 && config.music) {
						config.music = false;
						haltSoundChannel(_musicchannel);
						haltSoundChannel(_menuchannel);
					}
					if (cursel == 9 && !config.effects) {
						config.effects = true;
						int snd = playSound(_sfx[kSndDoor]);
						setChannelVolume(snd, config.effectsvol);
					}

					if (cursel == 10 && config.effects)
						config.effects = false;

					if (cursel == 13) {
						config_save(&config);
						break;
					}

					if (cursel == 14) {
						// reset keys to avoid returning
						// keys[SDLK_SPACE] = keys[SDLK_RETURN] = 0; // FIXME
						break;
					}
				}
			}
		}

		clouddeg += 0.01 * _fpsr;
		while (clouddeg >= 360)
			clouddeg -= 360;

		g_system->delayMillis(10);
	} while (!_shouldQuit);

	configwindow->free();
	_itemticks = _ticks + 210;

	cloudimg->setAlpha(64, true);
}

void GriffonEngine::saveLoadNew() {
	float y = 0.0;

	clouddeg = 0;

	_videobuffer->setAlpha(255);
	saveloadimg->setAlpha(192, true);

	int currow = 0;
	int curcol = 0;
	int lowerlock = 0;

	_ticks = g_system->getMillis();
	int _ticks1 = _ticks;
	int tickpause = _ticks + 150;

	do {
		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);

		y = y + 1 * _fpsr;

		rcDest.left = 256 + 256 * cos(3.141592 / 180 * clouddeg * 40);
		rcDest.top = 192 + 192 * sin(3.141592 / 180 * clouddeg * 40);
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		cloudimg->setAlpha(128, true);
		cloudimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, &rcDest);
		cloudimg->setAlpha(64, true);

		rcDest.left = 256;
		rcDest.top = 192;
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		cloudimg->setAlpha(128, true);
		cloudimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, &rcDest);
		cloudimg->setAlpha(64, true);

		saveloadimg->blit(*_videobuffer);

		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_QUIT) {
			_shouldQuit = true;
			return;
		}

		if (tickpause < _ticks) {
			if (_event.type == Common::EVENT_KEYDOWN) {
				_itemticks = _ticks + 220;

				if (_event.kbd.keycode == Common::KEYCODE_RETURN) {
					// QUIT - non existent :)
					if (currow == 0 && curcol == 4) {
						_shouldQuit = true;
						return;
					}
					// RETURN
					if (currow == 0 && curcol == 3) {
						// reset keys to avoid returning
						// keys[SDLK_RETURN] = keys[SDLK_SPACE] = 0; // FIXME
						return;
					}
					// NEW GAME
					if (currow == 0 && curcol == 0)
						newGame();

					// LOAD GAME
					if (currow == 0 && curcol == 1) {
						lowerlock = 1;
						currow = 1 + _saveslot;
						tickpause = _ticks + 125;
					}
					// SAVE GAME
					if (currow == 0 && curcol == 2) {
						lowerlock = 1;
						currow = 1;
						tickpause = _ticks + 125;
					}


					if (lowerlock == 1 && curcol == 1 && tickpause < _ticks) {
						if (saveState(currow - 1)) {
							_secstart = _secstart + _secsingame;
							_secsingame = 0;
							lowerlock = 0;
							_saveslot = currow - 1;
							currow = 0;
						}
					}

					if (lowerlock == 1 && curcol == 2 && tickpause < _ticks) {
						if (loadState(currow - 1)) {
							_player.walkspd = 1.1f;
							_animspd = 0.5f;
							attacking = false;
							_player.attackspd = 1.5f;

							_pgardens = false;
							_ptown = false;
							_pboss = false;
							_pacademy = false;
							_pcitadel = false;

							haltSoundChannel(-1);

							_secsingame = 0;
							_saveslot = currow - 1;
							loadMap(_curmap);
							mainLoop();
						}
					}

					tickpause = _ticks + 125;
				}

				switch (_event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					if (lowerlock == 0)
						return;
					lowerlock = 0;
					currow = 0;
					tickpause = _ticks + 125;
					break;
				case Common::KEYCODE_DOWN:
					if (lowerlock == 1) {
						currow = currow + 1;
						if (currow == 5)
							currow = 1;
						tickpause = _ticks + 125;
					}
					break;

				case Common::KEYCODE_UP:
					if (lowerlock == 1) {
						currow = currow - 1;
						if (currow == 0)
							currow = 4;
						tickpause = _ticks + 125;
					}
					break;

				case Common::KEYCODE_LEFT:
					if (lowerlock == 0) {
						curcol = curcol - 1;
						if (curcol == -1)
							curcol = 3;
						tickpause = _ticks + 125;
					}
					break;

				case Common::KEYCODE_RIGHT:
					if (lowerlock == 0) {
						curcol = curcol + 1;
						if (curcol == 4)
							curcol = 0;
						tickpause = _ticks + 125;
					}
					break;
				default:
					;
				}
			}
		}


		// savestates---------------------------------
		// read it only when needed!

		for (int ff = 0; ff <= 3; ff++) {
			loadPlayer(ff);

			if (_playera.level > 0) {
				int sx = 8;
				int sy = 57 + ff * 48;

				// time
				int ase = _asecstart;
				int h = ((ase - (ase % 3600)) / 3600);
				ase = (ase - h * 3600);
				int m = ((ase - (ase % 60)) / 60);
				int s = (ase - m * 60);

				char line[256];
				sprintf(line, "Game Time: %02i:%02i:%02i", h, m, s);
				drawString(_videobuffer, line, 160 - strlen(line) * 4, sy, 0);

				sx  = 12;
				sy = sy + 11;
				int cc = 0;

				sprintf(line, "Health: %i/%i", _playera.hp, _playera.maxhp);
				drawString(_videobuffer, line, sx, sy, cc);

				if (_playera.level == 22)
					strcpy(line, "Level: MAX");
				else
					sprintf(line, "Level: %i", _playera.level);

				drawString(_videobuffer, line, sx, sy + 11, 0);

				rcSrc.left = sx + 15 * 8 + 24;
				rcSrc.top = sy + 1;

				int ss = (_playera.sword - 1) * 3;
				if (_playera.sword == 3)
					ss = 18;
				_itemimg[ss]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

				rcSrc.left = rcSrc.left + 16;
				ss = (_playera.shield - 1) * 3 + 1;
				if (_playera.shield == 3)
					ss = 19;
				_itemimg[ss]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

				rcSrc.left = rcSrc.left + 16;
				ss = (_playera.armour - 1) * 3 + 2;
				if (_playera.armour == 3)
					ss = 20;
				_itemimg[ss]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

				int nx = rcSrc.left + 13 + 3 * 8;
				rcSrc.left = nx - 17;

				if (_playera.foundspell[0] == 1) {
					for (int i = 0; i < 5; i++) {
						rcSrc.left = rcSrc.left + 17;
						if (_playera.foundspell[i] == 1)
							_itemimg[7 + i]->blit(*_videobuffer, rcSrc.left, rcSrc.top);
					}
				}
			} else {
				int sy = 57 + ff * 48;
				drawString(_videobuffer, "Empty", 160 - 5 * 4, sy, 0);
			}
		}
		// ------------------------------------------


		if (currow == 0) {
			rcDest.top = 18;
			if (curcol == 0)
				rcDest.left = 10;
			if (curcol == 1)
				rcDest.left = 108;
			if (curcol == 2)
				rcDest.left = 170;
			if (curcol == 3)
				rcDest.left = 230;
			rcDest.left = (float)(rcDest.left + 2 + 2 * sin(3.14159 * 2 * _itemyloc / 16));
		}

		if (currow > 0) {
			rcDest.left = (float)(0 + 2 * sin(3.14159 * 2 * _itemyloc / 16));
			rcDest.top = (float)(53 + (currow - 1) * 48);
		}

		_itemimg[15]->blit(*_videobuffer, rcDest.left, rcDest.top);


		if (lowerlock == 1) {
			rcDest.top = 18;
			if (curcol == 1)
				rcDest.left = 108;
			if (curcol == 2)
				rcDest.left = 170;

			// CHECKME: Useless code? or temporary commented?
			// rcDest.left = rcDest.left; // + 2 + 2 * sin(-3.14159 * 2 * _itemyloc / 16)

			_itemimg[15]->blit(*_videobuffer, rcDest.left, rcDest.top);
		}

		int yy = 255;
		if (_ticks < _ticks1 + 1000) {
			yy = 255 * (_ticks - _ticks1) / 1000;
			yy = CLIP(yy, 0, 255);
		}

		_videobuffer->setAlpha((int)yy);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		clouddeg += 0.01 * _fpsr;
		while (clouddeg >= 360)
			clouddeg -= 360;

		_itemyloc += 0.6 * _fpsr;
		while (_itemyloc >= 16)
			_itemyloc -= 16;

		g_system->delayMillis(10);
	} while (!_shouldQuit);

	cloudimg->setAlpha(64, true);
}


} // end of namespace Griffon
