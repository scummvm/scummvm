/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "common/config-manager.h"

#include "griffon/griffon.h"

#include "common/text-to-speech.h"

namespace Griffon {

#define MINCURSEL 7
#define MAXCURSEL 16
#define SY 25
#define PI 3.141593

void GriffonEngine::title(int mode) {
	const char *optionTitles[4] = {
		"new game/save/load",
		"options",
		"quit game",
		"return"
	};

	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();

	float xofs = 0;
	_itemyloc = 0;
	bool exitTitle = false;

	rcSrc.left = 0;
	rcSrc.top = 0;
	rcSrc.setWidth(320);
	rcSrc.setHeight(240);

	_videoBuffer2->fillRect(rcSrc, 0);
	_videoBuffer3->fillRect(rcSrc, 0);

	_ticks = g_system->getMillis();

	_videoBuffer->blit(*_videoBuffer3);
	_videoBuffer->blit(*_videoBuffer2);

	int cursel = 0;
	int ticks1 = _ticks;

	if (config.music) {
		setChannelVolume(_musicChannel, 0);
		pauseSoundChannel(_musicChannel);

		_menuChannel = playSound(_musicMenu, true);
		setChannelVolume(_menuChannel, config.musicVol);
		_pmenu = true;
	}

	bool ldstop = false;

	float ld = 0;
	do {
		Common::Rect rc;

		ld += 4.0 * _fpsr;
		if (ld > config.musicVol)
			ld = config.musicVol;
		if (!ldstop) {
			setChannelVolume(_menuChannel, (int)ld);
			if ((int)ld == config.musicVol)
				ldstop = true;
		}

		rc.left = -xofs;
		rc.top = 0;

		_titleImg->blit(*_videoBuffer, rc.left, rc.top);

		rc.left = -xofs + 320.0;
		rc.top = 0;

		_titleImg->blit(*_videoBuffer, rc.left, rc.top);

		rc.left = 0;
		rc.top = 0;

		_titleImg2->blit(*_videoBuffer, rc.left, rc.top);

		int y = 172;
		int x = 160 - 14 * 4;

		drawString(_videoBuffer, optionTitles[0], x, y, 4);
		drawString(_videoBuffer, optionTitles[1], x, y + 16, 4);
		drawString(_videoBuffer, optionTitles[2], x, y + 32, 4);

		if (mode == 1)
			drawString(_videoBuffer, optionTitles[3], x, y + 48, 4);
		else
			drawString(_videoBuffer, "(c) 2005 by Daniel 'Syn9' Kennedy", 28, 224, 4);

		rc.left = (int16)(x - 16 - 4 * cos(2 * PI * _itemyloc / 16));
		rc.top = (int16)(y - 4 + 16 * cursel);

		_itemImg[15]->blit(*_videoBuffer, rc.left, rc.top);

		float yf = 255.0;
		if (_ticks < ticks1 + 1000) {
			yf = 255.0 * ((float)(_ticks - ticks1) / 1000.0);
			yf = CLIP<float>(yf, 0.0, 255.0);
		}

		_videoBuffer->setAlpha((int)yf);
		g_system->copyRectToScreen(_videoBuffer->getPixels(), _videoBuffer->pitch, 0, 0, _videoBuffer->w, _videoBuffer->h);
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
		}

		float add = 0.5 * _fpsr;
		if (add > 1)
			add = 1;

		xofs += add;
		if (xofs >= 320.0)
			xofs -= 320.0;

		_itemyloc += 0.75 * _fpsr;

		while (_itemyloc >= 16)
			_itemyloc = _itemyloc - 16;

		if (g_system->getEventManager()->pollEvent(_event)) {
			if (_event.type == Common::EVENT_QUIT || _event.type == Common::EVENT_RETURN_TO_LAUNCHER)
				_shouldQuit = true;

			if (_event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {

				switch(_event.customType) {
				case kGriffonMenu:
					if (mode == 1)
						exitTitle = true;
					break;
				case kGriffonUp:
					cursel--;
					if (cursel < 0)
						cursel = (mode == 1 ? 3 : 2);
					if (ttsMan != nullptr && ConfMan.getBool("tts_enabled"))
						ttsMan->say(optionTitles[cursel]);
					break;
				case kGriffonDown:
					cursel++;
					if (cursel >= (mode == 1 ? 4 : 3))
						cursel = 0;
					if (ttsMan != nullptr && ConfMan.getBool("tts_enabled"))
						ttsMan->say(optionTitles[cursel]);
					break;
				case kGriffonConfirm:
					if (ttsMan != nullptr)
							ttsMan->stop();
					switch(cursel) {
					case 0:
						_ticks = g_system->getMillis();
						ticks1 = _ticks;

						saveLoadNew();

						if (_gameMode == kGameModeNewGame || _gameMode == kGameModeLoadGame) {
							exitTitle = true;
						}
						break;
					case 1:
						configMenu();
						_ticks = g_system->getMillis();
						ticks1 = _ticks;
						break;
					case 2:
						_shouldQuit = true;
						break;
					case 3:
						exitTitle = true;
					default:
						break;
					}
					break;
				default:
					break;
				}
			}
		}

		g_system->delayMillis(10);
	} while (!_shouldQuit && !exitTitle && _gameMode != kGameModeNewGame && _gameMode != kGameModeLoadGame);

	_itemTicks = _ticks + 210;

	if (config.music) {
		haltSoundChannel(_menuChannel);
		resumeSoundChannel(_musicChannel);
		setChannelVolume(_musicChannel, config.musicVol);
		_pmenu = false;
	}
}

static Common::String formatPercent(int val) {
	val = val / 2.55;
	return Common::String::format("%d percent", val - val % 10);
}

static void speakMenuItem(int mapTitles, int mapValues, const char *mapTitleText[], const char *mapValueText[]) {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr && ConfMan.getBool("tts_enabled")) {
		Common::TextToSpeechManager::Action valueAction = Common::TextToSpeechManager::INTERRUPT;
		if (mapTitles != 0) {
			ttsMan->say(mapTitleText[mapTitles]);
			valueAction = Common::TextToSpeechManager::QUEUE;
		}
		if (mapValues != 0)
			ttsMan->say(mapValueText[mapValues], valueAction);
	}
}

void GriffonEngine::configMenu() {
	static const char *optionTitles[25] = {
		"", "",
		"", "", "", "",
		"", "", "",
		"Music:", "", "",
		"Sound Effects:", "", "",
		"Text to Speech:", "", "",
		"Music Volume:", "",
		"Effects Volume:", "", "", "", ""
	};
	static const char *optionValues[25] = {
		"", "",
		"", "", "", "",
		"", "", "",
		"On", "Off", "",
		"On", "Off", "",
		"On", "Off", "",
		"[----------]", "",
		"[----------]", "",
		"Exit + Save", "",
		"Exit"
	};
	static const int curselMapTitles[MAXCURSEL+1] = {
		0, 0, 0, 0, 0,
		0, 0, 9, 9, 12,
		12, 15, 15, 18, 20,
		0, 0
	};
	static const int curselMapValues[MAXCURSEL+1] = {
		0, 0, 0, 0, 0,
		0, 0, 9, 10, 12,
		13, 15, 16, 0, 0,
		22, 24
	};

	int cursel = MINCURSEL;
	bool exitMenu = false;

	_ticks = g_system->getMillis();

	Graphics::TransparentSurface *configwindow = loadImage("art/configwindow.bmp", true);
	configwindow->setAlpha(160, true);

	int ticks1 = _ticks;

	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();

	do {
		_videoBuffer->fillRect(Common::Rect(0, 0, _videoBuffer->w, _videoBuffer->h), 0);
		_videoBuffer2->fillRect(Common::Rect(0, 0, _videoBuffer2->w, _videoBuffer2->h), 0);

		rcDest.left = 256 + 256 * cos(PI / 180 * _cloudAngle * 40);
		rcDest.top = 192 + 192 * sin(PI / 180 * _cloudAngle * 40);
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		_cloudImg->blit(*_videoBuffer, 0, 0, Graphics::FLIP_NONE, &rcDest, MS_ARGB(128, 255, 255, 255));

		rcDest.left = 256;
		rcDest.top = 192;
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		_cloudImg->blit(*_videoBuffer, 0, 0, Graphics::FLIP_NONE, &rcDest, MS_ARGB(128, 255, 255, 255));

		configwindow->blit(*_videoBuffer);

		int sy = SY;

		for (int i = 0; i <= 24; i++) {
			static char line[24];

			int destColumn = 3;
			if (i == 9 && config.music)
				destColumn = 0;
			else if (i == 10 && !config.music)
				destColumn = 0;
			else if (i == 12 && config.effects)
				destColumn = 0;
			else if (i == 13 && !config.effects)
				destColumn = 0;
			else if (i == 15 && ConfMan.getBool("tts_enabled"))
				destColumn = 0;
			else if (i == 16 && !ConfMan.getBool("tts_enabled"))
				destColumn = 0;
			else if (i == 18|| i == 20) {
				int vol = (i ==18 ? config.musicVol : config.effectsVol) * 9 / 255;
				vol = CLIP(vol, 0, 9);

				Common::strcpy_s(line, "[----------]");
				line[vol + 1] = 'X';
				optionValues[i] = line;
			} else if (i > 21)
				destColumn = 0;

			drawString(_videoBuffer, optionTitles[i], 156 - 8 * strlen(optionTitles[i]), sy + i * 8, 0);
			drawString(_videoBuffer, optionValues[i], 164, sy + i * 8, destColumn);
		}

		int curselt = cursel + 2;
		if (cursel > 8)
			curselt += 1;
		if (cursel > 10)
			curselt += 1;
		if (cursel > 12)
			curselt += 1;
		if (cursel > 13)
			curselt += 1;
		if (cursel > 14)
			curselt += 1;
		if (cursel > 15)
			curselt += 1;

		Common::Rect rc;
		rc.left = 148 + 3 * cos(2 * PI * _itemyloc / 16.0);
		rc.top = sy + 8 * curselt - 4;

		_itemImg[15]->blit(*_videoBuffer, rc.left, rc.top);

		if (_ticks < ticks1 + 1000) {
			float yy = 255.0 * ((float)(_ticks - ticks1) / 1000.0);
			yy = CLIP<float>(yy, 0.0, 255.0);

			_videoBuffer->setAlpha((uint8)yy);
		}

		_videoBuffer->blit(*_videoBuffer2);
		g_system->copyRectToScreen(_videoBuffer2->getPixels(), _videoBuffer2->pitch, 0, 0, _videoBuffer2->w, _videoBuffer2->h);

		_ticksPassed = _ticks;
		_ticks = g_system->getMillis();

		_ticksPassed = _ticks - _ticksPassed;
		_fpsr = (float)_ticksPassed / 24;

		_fp++;
		if (_ticks > _nextTicks) {
			_nextTicks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		_itemyloc += 0.75 * _fpsr;
		while (_itemyloc >= 16)
			_itemyloc -= 16;

		while (g_system->getEventManager()->pollEvent(_event)) {
			switch (_event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				_shouldQuit = true;
				break;

			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				switch (_event.customType) {
				case kGriffonMenu:
					exitMenu = true;
					break;

				case kGriffonLeft:
					if (cursel == 13) {
						config.musicVol = CLIP(config.musicVol - 25, 0, 255);
						setChannelVolume(_musicChannel, config.musicVol);
						setChannelVolume(_menuChannel, config.musicVol);
						if (ttsMan != nullptr && ConfMan.getBool("tts_enabled"))
							ttsMan->say(formatPercent(config.musicVol));
					} else if (cursel == 14) {
						config.effectsVol = CLIP(config.effectsVol - 25, 0, 255);

						setChannelVolume(-1, config.effectsVol);
						setChannelVolume(_musicChannel, config.musicVol);
						setChannelVolume(_menuChannel, config.musicVol);
						if (ttsMan != nullptr && ConfMan.getBool("tts_enabled"))
							ttsMan->say(formatPercent(config.effectsVol));

						if (config.effects) {
							int snd = playSound(_sfx[kSndDoor]);
							setChannelVolume(snd, config.effectsVol);
						}
					}
					break;

				case kGriffonRight:
					if (cursel == 13) {
						config.musicVol = CLIP(config.musicVol + 25, 0, 255);
						setChannelVolume(_musicChannel, config.musicVol);
						setChannelVolume(_menuChannel, config.musicVol);
						if (ttsMan != nullptr && ConfMan.getBool("tts_enabled"))
							ttsMan->say(formatPercent(config.musicVol));
					} else if (cursel == 14) {
						config.effectsVol = CLIP(config.effectsVol + 25, 0, 255);
						setChannelVolume(-1, config.effectsVol);
						setChannelVolume(_musicChannel, config.musicVol);
						setChannelVolume(_menuChannel, config.musicVol);
						if (ttsMan != nullptr && ConfMan.getBool("tts_enabled"))
							ttsMan->say(formatPercent(config.effectsVol));

						if (config.effects) {
							int snd = playSound(_sfx[kSndDoor]);
							setChannelVolume(snd, config.effectsVol);
						}
					}
					break;

				case kGriffonUp:
					cursel--;
					if (cursel < MINCURSEL)
						cursel = MAXCURSEL;
					speakMenuItem(curselMapTitles[cursel], curselMapValues[cursel], optionTitles, optionValues);
					break;

				case kGriffonDown:
					++cursel;
					if (cursel > MAXCURSEL)
						cursel = MINCURSEL;
					speakMenuItem(curselMapTitles[cursel], curselMapValues[cursel], optionTitles, optionValues);
					break;

				case kGriffonConfirm:
					switch (cursel) {
					case 7:
						if (!config.music) {
							config.music = true;
							_menuChannel = playSound(_musicMenu, true);
							setChannelVolume(_menuChannel, config.musicVol);
						}
						break;
					case 8:
						if (config.music) {
							config.music = false;
							haltSoundChannel(_musicChannel);
							haltSoundChannel(_menuChannel);
						}
						break;
					case 9:
						if (!config.effects) {
							config.effects = true;
							int snd = playSound(_sfx[kSndDoor]);
							setChannelVolume(snd, config.effectsVol);
						}
						break;
					case 10:
						if (config.effects)
							config.effects = false;
						break;
					case 11:
						if (!ConfMan.getBool("tts_enabled")) {
							ConfMan.setBool("tts_enabled", true);
						}
						break;
					case 12:
						if (ConfMan.getBool("tts_enabled")) {
							ConfMan.setBool("tts_enabled", false);
						}
						break;
					case 15:
						saveConfig();
						// fall through
					case 16:
						if (ttsMan != nullptr)
							ttsMan->stop();
						exitMenu = true;
						break;
					default:
						break;
					}
					break;

				default:
					break;
				}

			default:
				break;
			}
		}

		_cloudAngle += 0.01 * _fpsr;
		while (_cloudAngle >= 360)
			_cloudAngle -= 360;

		g_system->updateScreen();
		g_system->delayMillis(10);
	} while (!_shouldQuit && !exitMenu && _gameMode != kGameModeNewGame && _gameMode != kGameModeLoadGame);


	configwindow->free();
	_itemTicks = _ticks + 210;
}

void GriffonEngine::renderSaveStates() {
	_videoBuffer2->fillRect(Common::Rect(0, 0, _videoBuffer2->w, _videoBuffer2->h), 0);

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
			Common::sprintf_s(line, "Game Time: %02i:%02i:%02i", h, m, s);
			drawString(_videoBuffer2, line, 160 - strlen(line) * 4, sy, 0);

			sx = 12;
			sy += 11;
			int cc = 0;

			Common::sprintf_s(line, "Health: %i/%i", _playera.hp, _playera.maxHp);
			drawString(_videoBuffer2, line, sx, sy, cc);

			if (_playera.level == 22)
				Common::strcpy_s(line, "Level: MAX");
			else
				Common::sprintf_s(line, "Level: %i", _playera.level);

			drawString(_videoBuffer2, line, sx, sy + 11, 0);

			rcSrc.left = sx + 15 * 8 + 24;
			rcSrc.top = sy + 1;

			int ss = (_playera.sword - 1) * 3;
			if (_playera.sword == 3)
				ss = 18;
			_itemImg[ss]->blit(*_videoBuffer2, rcSrc.left, rcSrc.top);

			rcSrc.left += 16;
			ss = (_playera.shield - 1) * 3 + 1;
			if (_playera.shield == 3)
				ss = 19;
			_itemImg[ss]->blit(*_videoBuffer2, rcSrc.left, rcSrc.top);

			rcSrc.left += 16;
			ss = (_playera.armour - 1) * 3 + 2;
			if (_playera.armour == 3)
				ss = 20;
			_itemImg[ss]->blit(*_videoBuffer2, rcSrc.left, rcSrc.top);

			int nx = rcSrc.left + 13 + 3 * 8;
			rcSrc.left = nx - 17;

			if (_playera.foundSpell[0]) {
				for (int i = 0; i < 5; i++) {
					rcSrc.left += 17;
					if (_playera.foundSpell[i])
						_itemImg[7 + i]->blit(*_videoBuffer2, rcSrc.left, rcSrc.top);
				}
			}
		} else {
			int sy = 57 + ff * 48;
			drawString(_videoBuffer2, "Empty", 160 - 5 * 4, sy, 0);
		}
	}
}

void GriffonEngine::saveLoadNew() {
	const char *optionTitles[4] = {
		"new game", "", "load", "return",
	};

	_cloudAngle = 0;

	int curRow = 0;
	int curCol = 0;

	bool lowerLock = false;

	_ticks = g_system->getMillis();
	int ticks1 = _ticks;
	int tickPause = _ticks + 150;


	renderSaveStates();

	delete _saveLoadImg;

	_saveLoadImg = loadImage("art/saveloadnew.bmp", true);
	if (_gameMode == kGameModeIntro) {
		uint32 color = *(uint32 *)_saveLoadImg->getBasePtr(120, 10);
		_saveLoadImg->fillRect(Common::Rect(125, 15, 160, 33), color);
	}
	_saveLoadImg->setAlpha(192, true);

	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();

	// Main menu loop
	do {
		_videoBuffer->fillRect(Common::Rect(0, 0, _videoBuffer->w, _videoBuffer->h), 0);

		rcDest.left = 256 + 256 * cos(PI / 180 * _cloudAngle * 40);
		rcDest.top = 192 + 192 * sin(PI / 180 * _cloudAngle * 40);
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		_cloudImg->blit(*_videoBuffer, 0, 0, Graphics::FLIP_NONE, &rcDest, MS_ARGB(128, 255, 255, 255));

		rcDest.left = 256;
		rcDest.top = 192;
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		_cloudImg->blit(*_videoBuffer, 0, 0, Graphics::FLIP_NONE, &rcDest, MS_ARGB(128, 255, 255, 255));

		_saveLoadImg->blit(*_videoBuffer);

		if (g_system->getEventManager()->pollEvent(_event)) {
			if (_event.type == Common::EVENT_QUIT || _event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_shouldQuit = true;
				return;
			}

			if (tickPause < _ticks && _event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				_itemTicks = _ticks + 220;

				if (_event.customType == kGriffonConfirm) {
					if (curRow == 0) {
						if (curCol == 0) {
							// NEW GAME
							if (ttsMan != nullptr)
								ttsMan->stop();
							_gameMode = kGameModeNewGame;

							return;
						} else if (curCol == 1) {
							// SAVE GAME
							lowerLock = true;
							curRow = 1 + _saveSlot;
							tickPause = _ticks + 125;
						} else if (curCol == 2) {
							// LOAD GAME
							lowerLock = true;
							curRow = 1;
							tickPause = _ticks + 125;
						} else if (curCol == 3) {
							if (ttsMan != nullptr)
								ttsMan->stop();
							// RETURN
							return;
						} else if (curCol == 4) {
							// QUIT - non existent :)
							_shouldQuit = true;
							return;
						}
					}
					if (lowerLock && tickPause < _ticks) {
						if ((curCol == 1) && saveGameState(curRow - 1, "", false).getCode() == Common::kNoError) {
							_secStart += _secsInGame;
							_secsInGame = 0;
							lowerLock = false;
							_saveSlot = curRow - 1;
							curRow = 0;

							renderSaveStates();
						} else if ((curCol == 2) && loadGameState(curRow - 1).getCode() == Common::kNoError) {

							return;
						}
						tickPause = _ticks + 125;
					}
				}

				switch (_event.customType) {
				case kGriffonMenu:
					if (curRow == 0)
						return;
					lowerLock = false;
					curRow = 0;
					tickPause = _ticks + 125;
					break;
				case kGriffonDown:
					if (lowerLock) {
						++curRow;
						if (curRow == 5)
							curRow = 1;
						tickPause = _ticks + 125;
					}
					break;

				case kGriffonUp:
					if (lowerLock) {
						--curRow;
						if (curRow == 0)
							curRow = 4;
						tickPause = _ticks + 125;
					}
					break;

				case kGriffonLeft:
					if (!lowerLock) {
						--curCol;
						if (curCol == -1)
							curCol = 3;

						if (curCol == 1 && _gameMode == kGameModeIntro)
							curCol = 0;

						tickPause = _ticks + 125;
					}
					if (ttsMan != nullptr && ConfMan.getBool("tts_enabled"))
						ttsMan -> say(optionTitles[curCol]);
					break;

				case kGriffonRight:
					if (!lowerLock) {
						++curCol;
						if (curCol == 4)
							curCol = 0;

						if (curCol == 1 && _gameMode == kGameModeIntro)
							curCol = 2;

						tickPause = _ticks + 125;
					}
					if (ttsMan != nullptr && ConfMan.getBool("tts_enabled"))
						ttsMan -> say(optionTitles[curCol]);
					break;
				default:
					;
				}
			}
		}

		// Render savestates
		_videoBuffer2->blit(*_videoBuffer);

		// ------------------------------------------

		if (curRow == 0) {
			rcDest.top = 18;
			switch(curCol) {
				case 0:
					rcDest.left = 10;
					break;
				case 1:
					rcDest.left = 108;
					break;
				case 2:
					rcDest.left = 170;
					break;
				case 3:
					rcDest.left = 230;
				default:
					break;
			}

			rcDest.left += (int16)(2 + 2 * sin(2 * PI * _itemyloc / 16));
		}

		if (curRow > 0) {
			rcDest.left = (int16)(0 + 2 * sin(2 * PI * _itemyloc / 16));
			rcDest.top = (int16)(53 + (curRow - 1) * 48);
		}

		_itemImg[15]->blit(*_videoBuffer, rcDest.left, rcDest.top);

		if (curRow != 0) {
			rcDest.top = 18;
			if (curCol == 1)
				rcDest.left = 108;
			else if (curCol == 2)
				rcDest.left = 170;

			_itemImg[15]->blit(*_videoBuffer, rcDest.left, rcDest.top);
		}

		if (_ticks < ticks1 + 1000) {
			int yy = 255 * (_ticks - ticks1) / 1000;
			yy = CLIP(yy, 0, 255);

			_videoBuffer->setAlpha((uint8)yy);
		}

		_videoBuffer3->fillRect(Common::Rect(0, 0, _videoBuffer3->w, _videoBuffer3->h), 0);
		_videoBuffer->blit(*_videoBuffer3);

		g_system->copyRectToScreen(_videoBuffer3->getPixels(), _videoBuffer3->pitch, 0, 0, _videoBuffer3->w, _videoBuffer3->h);
		g_system->updateScreen();

		_ticksPassed = _ticks;
		_ticks = g_system->getMillis();

		_ticksPassed = _ticks - _ticksPassed;
		_fpsr = (float)_ticksPassed / 24;

		_fp++;
		if (_ticks > _nextTicks) {
			_nextTicks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		_cloudAngle += 0.01 * _fpsr;
		while (_cloudAngle >= 360)
			_cloudAngle -= 360;

		_itemyloc += 0.6 * _fpsr;
		while (_itemyloc >= 16)
			_itemyloc -= 16;

		g_system->delayMillis(10);
	} while (!_shouldQuit);

}


} // end of namespace Griffon
