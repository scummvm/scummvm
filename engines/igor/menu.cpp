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
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"
#include "common/system.h"

#include "igor/igor.h"

namespace Igor {

static const struct {
	int y;
	int h;
} buttonsBarCoords[] = {
	{ 3, 1 },
	{ 1, 1 },
	{ 2, 2 },
	{ 4, 1 },
	{ 5, 10 },
	{ 6, 1 },
	{ 2, 2 },
	{ 7, 1 },
	{ 8, 93 },
	{ 9, 1 },
	{ 2, 2 },
	{ 3, 1 }
};

static void drawButtonsBar(uint8 *dst, const uint8 *src) {
	int y0 = 15;
	for (int i = 1; i <= 11; ++i) {
		int y2 = y0 + buttonsBarCoords[i].h - 1;
		if (y0 > y2) {
			continue;
		}
		for (int y1 = y0; y1 <= y2; ++y1) {
			memcpy(dst + (y1 - 15) * 320 + 43, src + (buttonsBarCoords[i].y - 1) * 234, 234);
			++y0;
		}
	}
}

static void redrawButton(uint8 *dst, int button, bool highlight) {
	const int colorDiff = highlight ? 4 : -4;
	for (int y = 0; y <= 11; ++y) {
		uint8 *p = dst + (y + 3) * 320 + button * 46 + 43 + 3;
		for (int x = 0; x <= 43; ++x) {
			p[x] += colorDiff;
		}
	}
}

enum {
	kPageSave = 0,
	kPageLoad,
	kPageQuit,
	kPageCtrl,
	kPagePlay
};

struct Page {
	const char *captions[2];
	int xPos[2];
	void (IgorEngine::*paintProc)();
	bool (IgorEngine::*handleKeyDownProc)(int key);
};

static const Page pages[] = {
	{
		{ "SAVE", "Choose a position to SAVE to" },
		{ 53, 75 },
		&IgorEngine::handleOptionsMenu_paintSave,
		&IgorEngine::handleOptionsMenu_handleKeyDownSave
	},
	{
		{ "LOAD", "Choose a game to load" },
		{ 98, 93 },
		&IgorEngine::handleOptionsMenu_paintLoad,
		&IgorEngine::handleOptionsMenu_handleKeyDownLoad
	},
	{
		{ "QUIT", "QUIT" },
		{ 146, 146 },
		&IgorEngine::handleOptionsMenu_paintQuit,
		&IgorEngine::handleOptionsMenu_handleKeyDownQuit
	},
	{
		{ "CTRL", "Game CONTROLS" },
		{ 190, 112 },
		&IgorEngine::handleOptionsMenu_paintCtrl,
		&IgorEngine::handleOptionsMenu_handleKeyDownCtrl
	},
	{
		{ "PLAY", 0 },
		{ 237, 0 },
		0,
		0
	}
};

void IgorEngine::handleOptionsMenu_paintSave() {
	drawString(_screenTextLayer, "Not available in shareware version", 60, 70 - 16, 0xF6, -1, 0);
}

bool IgorEngine::handleOptionsMenu_handleKeyDownSave(int key) {
	return true;
}

void IgorEngine::handleOptionsMenu_paintLoad() {
	drawString(_screenTextLayer, "Not available in shareware version", 60, 70 - 16, 0xF6, -1, 0);
}

bool IgorEngine::handleOptionsMenu_handleKeyDownLoad(int key) {
	return true;
}

void IgorEngine::handleOptionsMenu_paintQuit() {
	drawString(_screenTextLayer, "Are you sure?", 120, 64 - 15, 0xF6, -1, 0);
	drawString(_screenTextLayer, "(Y/N)", 143, 76 - 15, 0xF6, -1, 0);
}

bool IgorEngine::handleOptionsMenu_handleKeyDownQuit(int key) {
	if (key == Common::KEYCODE_y) {
		_currentPart = kInvalidPart;
	}
	return true;
}

static void redrawRect(uint8 *dst, int num, bool border) {
	const int x = 43 + 6;
	const int y = 22 * num;
	for (int i = 0; i < 21; ++i) {
		memset(dst + (y + i) * 320 + x, 0xF8, 222);
	}
	if (border) {
		memset(dst +  y       * 320 + x, 0xF7, 222);
		memset(dst + (y + 20) * 320 + x, 0xF9, 222);
		for (int i = 1; i < 20; ++i) {
			dst[(y + i) * 320 + x      ] = 0xF7;
			dst[(y + i) * 320 + x + 221] = 0xF9;
		}
	}
}

void IgorEngine::handleOptionsMenu_paintCtrl() {
	static const char *textsStrTable1[] = {
		"Sound effects OFF",
		"Sound effects ON",
		"Sound effects UNAVAILABLE"
	};
	static int textsPosTable1[] = { 106, 110, 77 };
	int i = _gameState.configSoundEnabled;
	redrawRect(_screenTextLayer, 1, false);
	drawString(_screenTextLayer, textsStrTable1[i], textsPosTable1[i], 41 - 15, 0xF7, -1, 0);

	redrawRect(_screenTextLayer, 2, false);
	drawString(_screenTextLayer, "Only TEXT AVAILABLE", 93, 63 - 15, 0xF7, -1, 0);

	static const char *textsStrTable2[] = {
		"Music OFF",
		"Music ON",
		"Music UNAVAILABLE"
	};
	static int textsPosTable2[] = { 129, 133, 100 };
	i = 1;
	redrawRect(_screenTextLayer, 3, false);
	drawString(_screenTextLayer, textsStrTable2[i], textsPosTable2[i], 85 - 15, 0xF7, -1, 0);

	static const char *textsStrTable3[] = {
		"Text speed VERY SLOW",
		"Text speed SLOW",
		"Text speed NORMAL",
		"Text speed FAST",
		"Text speed VERY FAST"
	};
	static int textsPosTable3[] = { 93, 110, 102, 111, 94 };
	i = _gameState.talkSpeed - 1;
	redrawRect(_screenTextLayer, 4, true);
	drawString(_screenTextLayer, textsStrTable3[i], textsPosTable3[i], 107 - 15, 0xF6, -1, 0);
}

bool IgorEngine::handleOptionsMenu_handleKeyDownCtrl(int key) {
	switch (key) {
	case Common::KEYCODE_ESCAPE:
		return true;
	case Common::KEYCODE_RETURN:
		if (_gameState.talkSpeed == 5) {
			_gameState.talkSpeed = 1;
		} else {
			++_gameState.talkSpeed;
		}
		handleOptionsMenu_paintCtrl();
		break;
	}
	return false;
}

void IgorEngine::handleOptionsMenu() {
	hideCursor();
	memcpy(_paletteBuffer, _currentPalette, 768);
	memset(_screenVGA + 144 * 320, 0, 11 * 320);
	_system->copyRectToScreen(_screenVGA, 320, 0, 0, 320, 200);
	for (int m = 3; m < 22; m += 3) {
		for (int i = 1 * 3; i <= 245 * 3; ++i) {
			if (_paletteBuffer[i] >= m) {
				_currentPalette[i] = MAX<int>(_paletteBuffer[i] - m, _currentPalette[i] - 3);
			}
		}
		setPaletteRange(1, 245);
		_system->updateScreen();
		_system->delayMillis(1000 / 60);
	}
	setPaletteColor(255, 35, 35, 23);

	memcpy(_screenTextLayer, _screenVGA + 4800, 36800);

	uint8 *optionsMenu = loadData(IMG_OptionsMenu);
	for (int i = 0; i < 36800; ++i) {
		if (optionsMenu[i]) {
			_screenTextLayer[i] = optionsMenu[i];
		}
	}
	free(optionsMenu);
	uint8 *optionsButton = loadData(IMG_OptionsButton);
	drawButtonsBar(_screenTextLayer, optionsButton);
	free(optionsButton);

	const int yPosCaption = 18 - 15;
	for (int i = 0; i < 5; ++i) {
		drawString(_screenTextLayer, pages[i].captions[0], pages[i].xPos[0], yPosCaption, 0xF6, -1, 0);
	}
	redrawButton(_screenTextLayer, 0, true);

	uint8 buttonsBarBackup[320 * 12];
	int currentPage = 0;
	bool menuLoop = true;
	bool focusOnPage = false;
	while (menuLoop && !_eventQuitGame && _currentPart != kInvalidPart) {
		int previousPage = currentPage;
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_QUIT:
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
				break;
			case Common::EVENT_KEYDOWN:
				if (focusOnPage) {
					if ((this->*pages[currentPage].handleKeyDownProc)(ev.kbd.keycode)) {
						memcpy(_screenTextLayer + 3 * 320, buttonsBarBackup, 320 * 12);
						for (int y = 22; y <= 110; ++y) {
							memset(_screenTextLayer + y * 320 + 43 + 4, 0xF8, 226);
						}
						focusOnPage = false;
					}
				} else {
					switch (ev.kbd.keycode) {
					case Common::KEYCODE_ESCAPE:
						menuLoop = false;
						break;
					case Common::KEYCODE_RETURN:
						if (currentPage == kPagePlay) {
							menuLoop = false;
						} else {
							memcpy(buttonsBarBackup, _screenTextLayer + 3 * 320, 320 * 12);
							for (int y = 0; y <= 11; ++y) {
								memset(_screenTextLayer + (y + 3) * 320 + 44, 0xF8, 232);
							}
							drawString(_screenTextLayer, pages[currentPage].captions[1], pages[currentPage].xPos[1], yPosCaption, 0xFB, -1, 0);
							(this->*pages[currentPage].paintProc)();
							focusOnPage = true;
						}
						break;
					case Common::KEYCODE_LEFT:
						--currentPage;
						if (currentPage < 0) {
							currentPage = 0;
						}
						break;
					case Common::KEYCODE_RIGHT:
						++currentPage;
						if (currentPage > 4) {
							currentPage = 4;
						}
						break;
					default:
						break;
					}
				}
				break;
			default:
				break;
			}
		}
		if (previousPage != currentPage) {
			redrawButton(_screenTextLayer, previousPage, false);
			redrawButton(_screenTextLayer, currentPage, true);
		}
		_system->delayMillis(10);
		_system->copyRectToScreen(_screenTextLayer, 320, 0, 15, 320, 115);
		_system->updateScreen();
	}

	for (int m = 21; m >= 3; m -= 3) {
		for (int i = 1 * 3; i <= 245 * 3; ++i) {
			if (_paletteBuffer[i] >= m) {
				_currentPalette[i] = MIN<int>(_paletteBuffer[i], _currentPalette[i] + 3);
			}
		}
		setPaletteRange(1, 245);
		_system->updateScreen();
		_system->delayMillis(1000 / 60);
	}
	showCursor();
}

void IgorEngine::handlePause() {
	drawActionSentence(getString(STR_GamePaused), 0xFB);
	do {
		waitForTimer();
	} while (!_inputVars[kInputPause]);
	memset(_inputVars, 0, sizeof(_inputVars));
}

} // namespace Igor
