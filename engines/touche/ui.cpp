/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $Id: $
 *
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "common/savefile.h"

#include "touche/graphics.h"
#include "touche/touche.h"

namespace Touche {

static const Common::Rect buttonsRectTable1[15] = {
	Common::Rect(108, 120, 444, 135),
	Common::Rect(108, 136, 444, 151),
	Common::Rect(108, 152, 444, 167),
	Common::Rect(108, 168, 444, 183),
	Common::Rect(108, 184, 444, 199),
	Common::Rect(108, 200, 444, 215),
	Common::Rect(108, 216, 444, 231),
	Common::Rect(108, 232, 444, 247),
	Common::Rect(108, 248, 444, 263),
	Common::Rect(108, 264, 444, 279),
	Common::Rect(452, 120, 546, 144),
	Common::Rect(452, 152, 546, 176),
	Common::Rect(452, 216, 546, 240),
	Common::Rect(452, 248, 546, 272),
	Common::Rect(452, 184, 546, 208)
};

static const Common::Rect buttonsRectTable2[10] = {
	Common::Rect(396, 130, 420, 154),
	Common::Rect(396, 160, 420, 184),
	Common::Rect(396, 190, 420, 214),
	Common::Rect(126, 130, 380, 154),
	Common::Rect(126, 160, 380, 184),
	Common::Rect(126, 190, 380, 214),
	Common::Rect(126, 250, 150, 274),
	Common::Rect(396, 250, 420, 274),
	Common::Rect(154, 256, 392, 268),
	Common::Rect(126, 222, 420, 242)
};

static int16 settingsMenuTextsTable[] = { 0, 0, 0, -92, -93, -94, -87, -88, 0, -91 };

static const int16 optionsMenuTextsTable[] = { -52, -53, -54, -55, -90 };

static const int16 saveMenuTextsTable[] = { 2000, -56, -52, 2001, 0 };

static const int16 loadMenuTextsTable[] = { 2000, -56, -53, 2001, 0 };

bool ToucheEngine::ui_processEvents() {
	bool quit = false;
	OSystem::Event event;
	while (_system->pollEvent(event)) {
		switch (event.type) {
		case OSystem::EVENT_QUIT:
			quit = true;
			break;
		case OSystem::EVENT_KEYDOWN:
			if (_saveLoadCurrentDescriptionLen != -1) {
				if (event.kbd.keycode == 8) {
					if (_saveLoadCurrentDescriptionLen > 0) {
						--_saveLoadCurrentDescriptionLen;
						_saveLoadCurrentDescription[_saveLoadCurrentDescriptionLen] = 0;
					}
				} else if (isprint((char)event.kbd.ascii)) {
					if (_saveLoadCurrentDescriptionLen < 32) {
						_saveLoadCurrentDescription[_saveLoadCurrentDescriptionLen] = (char)event.kbd.ascii;
						++_saveLoadCurrentDescriptionLen;
						_saveLoadCurrentDescription[_saveLoadCurrentDescriptionLen] = 0;
					}
				}
			}
			break;
		case OSystem::EVENT_MOUSEMOVE:
			_inp_mousePos.x = event.mouse.x;
			_inp_mousePos.y = event.mouse.y;
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			_inp_mousePos.x = event.mouse.x;
			_inp_mousePos.y = event.mouse.y;
			_inp_mouseButtonClicked = true;
			break;
		case OSystem::EVENT_LBUTTONUP:
			_inp_mousePos.x = event.mouse.x;
			_inp_mousePos.y = event.mouse.y;
			break;
		default:
			break;
		}
	}
	_system->updateScreen();
	_system->delayMillis(50);
	return quit;
}

void ToucheEngine::ui_drawButtonBorders(const Common::Rect *r, int count) {
	while (count--) {
		Graphics::drawRect(_offscreenBuffer, 640, r->left, r->top, r->width(), r->height(), 0xF7, 0xF9);
		++r;
	}
}

void ToucheEngine::ui_drawMusicVolumeBar() {
	int volume = _snd_midiContext.volume * 232 / 256;
	if (volume != 0) {
		Graphics::fillRect(_offscreenBuffer, 640, 157, 259, volume, 6, 0xF0);
	}
	if (volume <= 232) {
		Graphics::fillRect(_offscreenBuffer, 640, 157 + volume, 259, 232 - volume, 6, 0xD2);
	}
}

void ToucheEngine::ui_drawTalkMode() {
	settingsMenuTextsTable[0] = 0;
	settingsMenuTextsTable[1] = 0;
	settingsMenuTextsTable[2] = 0;
	settingsMenuTextsTable[_talkTextMode] = -86;
}

void ToucheEngine::ui_drawAllBorders() {
	Graphics::fillRect(_offscreenBuffer, 640, 90, 102, 460, 196, 248);
	Graphics::drawRect(_offscreenBuffer, 640, 90, 102, 460, 196, 0xF7, 0xF9);
	Graphics::drawRect(_offscreenBuffer, 640, 106, 118, 340, 164, 0xF9, 0xF7);
	ui_drawButtonBorders(&buttonsRectTable1[10], 5);
}

void ToucheEngine::ui_drawSaveGamesList(int page) {
	ui_drawAllBorders();
	for (int i = 0; i < 10; ++i) {
		const Common::Rect *r = &buttonsRectTable1[i];
		uint8 color = (_saveLoadCurrentSlot == i) ? 0xCB : 0xD9;
		char num[10];
		sprintf(num, "%d.", page + i);
		Graphics::drawString16(_offscreenBuffer, 640, color, r->left, r->top, num);
		Graphics::drawString16(_offscreenBuffer, 640, color, r->left + 30, r->top, _saveLoadDescriptionsTable[i]);
	}
}

void ToucheEngine::ui_drawCurrentGameStateDescription() {
	const Common::Rect *r = &buttonsRectTable1[_saveLoadCurrentSlot];
	Graphics::fillRect(_offscreenBuffer, 640, r->left, r->top, r->width(), r->height(), 0xF8);

	int y = r->top;
	int x = r->left;
	char num[10];
	sprintf(num, "%d.", _saveLoadCurrentSlot);
	Graphics::drawString16(_offscreenBuffer, 640, 0xCB, x, y, num);
	x += 30;
	Graphics::drawString16(_offscreenBuffer, 640, 0xCB, x, y, _saveLoadCurrentDescription);
	x += Graphics::getStringWidth16(_saveLoadCurrentDescription);
	Graphics::drawString16(_offscreenBuffer, 640, 0xCB, x, y, "_");

	updateScreenArea(_offscreenBuffer, 640, r->left, r->top, r->left, r->top, r->width(), r->height());
}

void ToucheEngine::ui_drawSaveLoadMenu(int page, int saveOrLoad) {
	for (int i = 0; i < 10; ++i) {
		_saveLoadDescriptionsTable[i][0] = 0;
		const int gameState = page + i;
		if (_saveLoadMarks[gameState]) {
			readGameStateDescription(gameState, _saveLoadDescriptionsTable[i], 32);
		}
	}
	ui_drawSaveGamesList(page);
	if (saveOrLoad == 0) {
		ui_drawButtonText(loadMenuTextsTable, &buttonsRectTable1[10], 5, true);
	} else {
		ui_drawButtonText(saveMenuTextsTable, &buttonsRectTable1[10], 5, true);
	}
	updateScreenArea(_offscreenBuffer, 640, 90, 102, 90, 102, 460, 196);
}

int ToucheEngine::ui_getButtonPressed(const Common::Rect *r, int count) const {
	for (int i = 0; i < count; ++i) {
		if (r[i].contains(_inp_mousePos)) {
			return i;
		}
	}
	return -1;
}

void ToucheEngine::ui_drawButtonText(const int16 *texts, const Common::Rect *r, int count, bool centerTexts) {
	for (int i = 0; i < count; ++i, ++texts, ++r) {
		int x, y;
		if (*texts < 2000) {
			const char *str = getString(*texts);
			x = r->left;
			y = r->top;
			if (centerTexts) {
				const int w = getStringWidth(16, *texts);
				x += (r->width() - w) / 2;
				y += (r->height() - 16) / 2;
			}
			Graphics::drawString16(_offscreenBuffer, 640, 0xFF, x, y, str);
		} else {
			x = r->left + r->width() / 2;
			y = r->top + r->height() / 2;
			int dx, dy;
			switch (*texts) {
			case 2000: // up arrow
				dx = 1;
				dy = 2;
				break;
			case 2001: // down arrow
				dx = -1;
				dy = -2;
				break;
			}
			ui_drawArrow(x, y + dy + 1, dx, 0xD2);
			ui_drawArrow(x, y + dy, dx, 0xFF);
		}
	}
}

void ToucheEngine::ui_drawArrow(int x, int y, int dx, uint8 color) {
	static const int16 arrowCoordsTable[] = {
		 5,  0,  9,  0,
		 5,  0,  5,  4,
		-5,  4,  5,  4,
		-5,  0, -5,  4,
		-9,  0, -5,  0,
		-9,  0,  0, -9,
		 0, -9,  9,  0
	};
	for (uint i = 0; i < ARRAYSIZE(arrowCoordsTable) / 4; ++i) {
		const int x1 = x + arrowCoordsTable[i * 4 + 0];
		const int y1 = y + arrowCoordsTable[i * 4 + 1] * dx;
		const int x2 = x + arrowCoordsTable[i * 4 + 2];
		const int y2 = y + arrowCoordsTable[i * 4 + 3] * dx;
		Graphics::drawLine(_offscreenBuffer, 640, x1, y1, x2, y2, color);
	}
}

void ToucheEngine::ui_drawOptionsMenu() {
	ui_drawTalkMode();
	ui_drawAllBorders();
	ui_drawButtonText(optionsMenuTextsTable, &buttonsRectTable1[10], 5, true);
	ui_drawButtonBorders(buttonsRectTable2, 10);
	ui_drawButtonText(settingsMenuTextsTable, buttonsRectTable2, 10, true);
	ui_drawMusicVolumeBar();
	updateScreenArea(_offscreenBuffer, 640, 90, 102, 90, 102, 460, 196);
}

int ToucheEngine::ui_handleSaveLoad(int saveOrLoad) {
	char gameStateFileName[16];
	generateGameStateFileName(999, gameStateFileName, 15, true);
	_saveFileMan->listSavefiles(gameStateFileName, _saveLoadMarks, NUM_GAMESTATE_FILES);
	int ret = 0;
	bool quitMenu = false;
	while (!quitMenu) {
		_saveLoadCurrentDescription[0] = 0;
		_saveLoadCurrentDescriptionLen = 0;
		ui_drawSaveLoadMenu(_saveLoadCurrentPage, saveOrLoad);
		int descriptionLen = 0;
		int button = -1;
		while (button == -1 && !quitMenu) {
			button = ui_getButtonPressed(buttonsRectTable1, 15);
			if (!_inp_mouseButtonClicked) {
				button = -1;
			}
			if (saveOrLoad == 0) {
				_saveLoadCurrentPage = (_saveLoadCurrentSlot / 10) * 10;
				if (_saveLoadCurrentDescriptionLen != descriptionLen) {
					descriptionLen = _saveLoadCurrentDescriptionLen;
					ui_drawCurrentGameStateDescription();
					strcpy(_saveLoadDescriptionsTable[_saveLoadCurrentSlot % 10], _saveLoadCurrentDescription);
				}
			}
			quitMenu = ui_processEvents();
		}
		_inp_mouseButtonClicked = false;
		switch (button) {
		case 10:
			_saveLoadCurrentPage -= 10;
			if (_saveLoadCurrentPage < 0) {
				_saveLoadCurrentPage = 90;
			}
			break;
		case 11:
			quitMenu = true;
			ret = 0;
			break;
		case 12:
			quitMenu = true;
			ret = 1;
			if (saveOrLoad == 0) {
				if (saveGameState(_saveLoadCurrentSlot, _saveLoadDescriptionsTable[_saveLoadCurrentSlot % 10])) {
					ret = 2;
				}
			} else {
				if (loadGameState(_saveLoadCurrentSlot, _saveLoadDescriptionsTable[_saveLoadCurrentSlot % 10])) {
					ret = 2;
				}
			}
			break;
		case 13:
			_saveLoadCurrentPage += 10;
			if (_saveLoadCurrentPage > 90) {
				_saveLoadCurrentPage = 0;
			}
			break;
		default:
			if (button >= 0 && button <= 9) {
				_saveLoadCurrentSlot = _saveLoadCurrentPage + button;
			}
			break;
		}
	}
	return ret;
}

void ToucheEngine::ui_handleOptions(int forceDisplay) {
	if (_disabledInputCounter == 0 || forceDisplay != 0) {
		_saveLoadCurrentDescriptionLen = -1;
		updateCursor(_currentKeyCharNum);
		int16 mode = _flagsTable[618];
		_flagsTable[618] = 0;
		updateEntireScreen();
		bool quitMenu = false;
		while (!quitMenu) {
			ui_drawOptionsMenu();
			int button = -1;
			while (button == -1 && !quitMenu) {
				if (_inp_mouseButtonClicked) {
					button = ui_getButtonPressed(buttonsRectTable1, 15);
					if (button < 10) {
						button = ui_getButtonPressed(buttonsRectTable2, 10) + 20;
					}
				}
				quitMenu = ui_processEvents();
			}
			_inp_mouseButtonClicked = false;
			switch (button) {
			case 10:
				if (ui_handleSaveLoad(1) == 2) {
					quitMenu = true;
				}
				break;
			case 11:
				if (ui_handleSaveLoad(0) == 2) {
					quitMenu = true;
				}
				break;
			case 12:
				quitMenu = true;
				break;
			case 13:
				quitMenu = true;
				_flagsTable[611] = 1;
				break;
			case 14:
				restart();
				quitMenu = true;
				break;
			case 20:
				_talkTextMode = kTalkModeTextOnly;
				break;
			case 21:
				_talkTextMode = kTalkModeVoiceOnly;
				break;
			case 22:
				_talkTextMode = kTalkModeVoiceAndText;
				break;
			case 26:
				if (_snd_midiContext.volume > 0) {
					_snd_midiContext.volume -= 16;
				}
				break;
			case 27:
				if (_snd_midiContext.volume < 256) {
					_snd_midiContext.volume += 16;
				}
				break;
			}
		}
		_fullRedrawCounter = 2;
		_flagsTable[618] = mode;
		if (_flagsTable[611] != 0) {
			_flagsTable[611] = ui_displayQuitDialog();
		}
		_snd_midiContext.currentVolume = _snd_midiContext.volume;
	}
}

void ToucheEngine::ui_drawActionsPanel(int dstX, int dstY, int deltaX, int deltaY) {
	Graphics::copyRect(_offscreenBuffer, 640, dstX, dstY,
	  _menuKitData, 42, 0, 0,
	  14, 24,
	  Graphics::kTransparent);
	Graphics::copyRect(_offscreenBuffer, 640, deltaX - 14 + dstX, dstY,
	  _menuKitData, 42, 0, 40,
	  14, 24,
	  Graphics::kTransparent);
	Graphics::copyRect(_offscreenBuffer, 640, dstX, deltaY - 16 + dstY, 
	  _menuKitData, 42, 0, 24, 
	  14, 16,
	  Graphics::kTransparent);
	Graphics::copyRect(_offscreenBuffer, 640, deltaX - 14 + dstX, deltaY - 16 + dstY,
	  _menuKitData, 42, 0, 64, 
	  14, 16,
	  Graphics::kTransparent);
	int x1 = deltaX - 28;
	int x2 = dstX + 14;
	while (x1 > 0) {
		int w = (x1 > 14) ? 14 : x1;
		Graphics::copyRect(_offscreenBuffer, 640, x2, dstY, 
		  _menuKitData, 42, 0, 80, 
		  w, 24, 
		  Graphics::kTransparent);
		Graphics::copyRect(_offscreenBuffer, 640, x2, deltaY - 16 + dstY, 
		  _menuKitData, 42, 0, 104, 
		  w, 16,
		  Graphics::kTransparent);
		x1 -= 14;
		x2 += 14;
	}
	x1 = deltaY - 40;
	x2 = dstY + 24;
	while (x1 > 0) {
		int w = (x1 > 120) ? 120 : x1;
		Graphics::copyRect(_offscreenBuffer, 640, dstX, x2, 
		  _menuKitData, 42, 14, 0, 
		  14, w, 
		  Graphics::kTransparent);
		Graphics::copyRect(_offscreenBuffer, 640, deltaX - 14 + dstX, x2, 
		  _menuKitData, 42, 28, 0, 
		  14, w,
		  Graphics::kTransparent);
		x1 -= 120;
		x2 += 120;
	}
}

void ToucheEngine::ui_drawConversationPanelBorder(int dstY, int srcX, int srcY) {
	int dstX = 24;
	int w = 48;
	for (int i = 0; i < 13; ++i) {
		if (i == 12) {
			w = 34;
		}
		Graphics::copyRect(_offscreenBuffer, 640, dstX, dstY, _convKitData, 152, srcX, srcY, w, 6);
		dstX += w;
	}
}

void ToucheEngine::ui_drawConversationPanel() {
	Graphics::copyRect(_offscreenBuffer, 640, 0, 320, _convKitData, 152, 0, 0, 72, 80);
	int dstX = 54;
	int dstY = 326;
	int w = 96;
	for (int i = 0; i < 7; ++i) {
		if (i == 5) {
			w = 50;
		}
		Graphics::copyRect(_offscreenBuffer, 640, dstX, dstY, _convKitData, 152, 24, 6, w, 68);
		dstX += w;
	}
	--dstX;
	Graphics::copyRect(_offscreenBuffer, 640, dstX, 320, _convKitData, 152, 120, 0, 7, 80);
	dstX -= 3;
	if (_drawCharacterConversionRepeatCounter != 0) {
		ui_drawConversationPanelBorder(320, 72, 0);
		Graphics::copyRect(_offscreenBuffer, 640, 0, 320, _convKitData, 152, 128, 0, 24, 21);
		Graphics::copyRect(_offscreenBuffer, 640, dstX, 320, _convKitData, 152, 128, 34, 10, 10);
	} else {
		ui_drawConversationPanelBorder(320, 24, 0);
	}
	if (_conversationChoicesTable[_drawCharacterConversionRepeatCounter + 4].msg != 0) {
		ui_drawConversationPanelBorder(394, 72, 74);
		Graphics::copyRect(_offscreenBuffer, 640, 0, 379, _convKitData, 152, 128, 59, 24, 21);
		Graphics::copyRect(_offscreenBuffer, 640, dstX, 394, _convKitData, 152, 128, 46, 10, 6);
	} else {
		ui_drawConversationPanelBorder(394, 24, 74);
	}
}

void ToucheEngine::ui_printStatusString(const char *str) {
	Graphics::fillRect(_offscreenBuffer, 640, 0, 0, 640, 16, 0xD7);
	Graphics::drawRect(_offscreenBuffer, 640, 0, 0, 640, 16, 0xD6, 0xD8);
	Graphics::drawString16(_offscreenBuffer, 640, 0xFF, 0, 0, str);
	updateScreenArea(_offscreenBuffer, 640, 0, 0, 0, 0, 640, 16);
}

void ToucheEngine::ui_clearStatusString() {
	Graphics::copyRect(_offscreenBuffer, 640, 0, 0, 
	  _backdropBuffer, _currentBitmapWidth, _flagsTable[614], _flagsTable[615],
	  640, 16);
	updateScreenArea(_offscreenBuffer, 640, 0, 0, 0, 0, 640, 16);
}

int ToucheEngine::ui_displayQuitDialog() {
	debug(kDebugUserIntf, "ui_displayQuitDialog()");
	ui_printStatusString(getString(-85));
	int ret = 0;
	bool quitLoop = false;
	while (!quitLoop) {
		OSystem::Event event;
		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_QUIT:
				quitLoop = true;
				ret = 1;
				break;
			case OSystem::EVENT_KEYDOWN:
				quitLoop = true;
				switch (_language) {
				case Common::FR_FRA:
					if (event.kbd.ascii == 'o' || event.kbd.ascii == 'O') {
						ret = 1;
					}
					break;
				default:
					if (event.kbd.ascii == 'y' || event.kbd.ascii == 'Y') {
						ret = 1;
					}
					break;
				}
				break;
			default:
				break;
			}
		}
		_system->delayMillis(50);
	}
	ui_clearStatusString();
	return ret;
}

void ToucheEngine::ui_displayTextMode(int str) {
	debug(kDebugUserIntf, "ui_displayTextMode(%d)", str);
	ui_printStatusString(getString(str));
	_system->delayMillis(1000);
	ui_clearStatusString();
}

} // namespace Touche
