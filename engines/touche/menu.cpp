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
 */


#include "common/events.h"
#include "common/system.h"
#include "common/savefile.h"

#include "touche/graphics.h"
#include "touche/midi.h"
#include "touche/touche.h"

namespace Touche {

static void drawArrow(uint8 *dst, int dstPitch, int x, int y, int delta, uint8 color) {
	static const int8 arrowCoordsTable[7][4] = {
		{  5,  0,  9,  0 },
		{  5,  0,  5,  4 },
		{ -5,  4,  5,  4 },
		{ -5,  0, -5,  4 },
		{ -9,  0, -5,  0 },
		{ -9,  0,  0, -9 },
		{  0, -9,  9,  0 }
	};
	for (uint i = 0; i < 7; ++i) {
		const int x1 = x + arrowCoordsTable[i][0];
		const int y1 = y + arrowCoordsTable[i][1] * delta;
		const int x2 = x + arrowCoordsTable[i][2];
		const int y2 = y + arrowCoordsTable[i][3] * delta;
		Graphics::drawLine(dst, dstPitch, x1, y1, x2, y2, color);
	}
}

void ToucheEngine::drawButton(Button *button) {
	if (button->flags & kButtonBorder) {
		Graphics::drawRect(_offscreenBuffer, kScreenWidth, button->x, button->y, button->w, button->h, 0xF7, 0xF9);
	}
	if (button->flags & kButtonText) {
		if (button->data != 0) {
			const char *str = getString(button->data);
			const int w = getStringWidth(button->data);
			const int h = kTextHeight;
			const int x = button->x + (button->w - w) / 2;
			const int y = button->y + (button->h - h) / 2;
			Graphics::drawString16(_offscreenBuffer, kScreenWidth, 0xFF, x, y, str);
		}
	}
	if (button->flags & kButtonArrow) {
		int dx = 0;
		int dy = 0;
		switch (button->data) {
		case 2000: // up arrow
			dx = 1;
			dy = 2;
			break;
		case 2001: // down arrow
			dx = -1;
			dy = -2;
			break;
		default:
			break;
		}
		const int x = button->x + button->w / 2;
		const int y = button->y + button->h / 2;
		drawArrow(_offscreenBuffer, kScreenWidth, x, y + dy + 1, dx, 0xD2);
		drawArrow(_offscreenBuffer, kScreenWidth, x, y + dy,     dx, 0xFF);
	}
}

static void drawVolumeSlideBar(uint8 *dst, int dstPitch, int volume) {
	const int w = volume * 232 / 255;
	if (w > 0) {
		Graphics::fillRect(dst, dstPitch, 157, 259, w, 6, 0xF0);
	}
	if (w < 232) {
		Graphics::fillRect(dst, dstPitch, 157 + w, 259, 232 - w, 6, 0xD2);
	}
}

static void drawSaveGameStateDescriptions(uint8 *dst, int dstPitch, MenuData *menuData, int currentPage, int currentSlot) {
	for (int i = 0, slot = currentPage * 10; i < 10; ++i, ++slot) {
		const Button *b = &menuData->buttonsTable[i];
		const uint8 color = (slot == currentSlot) ? 0xCB : 0xD9;
		Common::String savegameNameStr = Common::String::format("%d.", slot);
		Graphics::drawString16(dst, dstPitch, color, b->x, b->y, savegameNameStr.c_str());
		savegameNameStr = menuData->saveLoadDescriptionsTable[slot];
		if (slot == currentSlot && menuData->mode == kMenuSaveStateMode) {
			savegameNameStr += "_";
		}
		Graphics::drawString16(dst, dstPitch, color, b->x + 30, b->y, savegameNameStr.c_str());
	}
}

static void setupMenu(MenuMode mode, MenuData *menuData) {
	static Button settingsButtonsTable[] = {
		{ 452, 120,  94, 24, kActionLoadMenu,     -52, kButtonBorder | kButtonText },
		{ 452, 152,  94, 24, kActionSaveMenu,     -53, kButtonBorder | kButtonText },
		{ 452, 184,  94, 24, kActionRestartGame,  -90, kButtonBorder | kButtonText },
		{ 452, 216,  94, 24, kActionPlayGame,     -54, kButtonBorder | kButtonText },
		{ 452, 248,  94, 24, kActionQuitGame,     -55, kButtonBorder | kButtonText },
		{ 396, 130,  24, 24, kActionTextOnly,       0, kButtonBorder | kButtonText },
		{ 396, 160,  24, 24, kActionVoiceOnly,      0, kButtonBorder | kButtonText },
		{ 396, 190,  24, 24, kActionTextAndVoice,   0, kButtonBorder | kButtonText },
		{ 126, 130, 254, 24, kActionNone,         -92, kButtonBorder | kButtonText },
		{ 126, 160, 254, 24, kActionNone,         -93, kButtonBorder | kButtonText },
		{ 126, 190, 254, 24, kActionNone,         -94, kButtonBorder | kButtonText },
		{ 126, 222, 294, 20, kActionNone,         -91, kButtonBorder | kButtonText },
		{ 126, 250,  24, 24, kActionLowerVolume,  -87, kButtonBorder | kButtonText },
		{ 396, 250,  24, 24, kActionUpperVolume,  -88, kButtonBorder | kButtonText },
		{ 154, 256, 238, 12, kActionNone,           0, kButtonBorder }
	};
	static Button saveLoadButtonsTable[] = {
		{ 108, 120, 336, 15, kActionGameState1,         0, 0 },
		{ 108, 136, 336, 15, kActionGameState2,         0, 0 },
		{ 108, 152, 336, 15, kActionGameState3,         0, 0 },
		{ 108, 168, 336, 15, kActionGameState4,         0, 0 },
		{ 108, 184, 336, 15, kActionGameState5,         0, 0 },
		{ 108, 200, 336, 15, kActionGameState6,         0, 0 },
		{ 108, 216, 336, 15, kActionGameState7,         0, 0 },
		{ 108, 232, 336, 15, kActionGameState8,         0, 0 },
		{ 108, 248, 336, 15, kActionGameState9,         0, 0 },
		{ 108, 264, 336, 15, kActionGameState10,        0, 0 },
		{ 452, 120,  94, 24, kActionScrollUpSaves,   2000, kButtonBorder | kButtonArrow },
		{ 452, 152,  94, 24, kActionCancelSaveLoad,   -56, kButtonBorder | kButtonText  },
		{ 452, 216,  94, 24, kActionPerformSaveLoad,    0, kButtonBorder | kButtonText  },
		{ 452, 248,  94, 24, kActionScrollDownSaves, 2001, kButtonBorder | kButtonArrow }
	};

	switch (mode) {
	case kMenuSettingsMode:
		menuData->buttonsTable = settingsButtonsTable;
		menuData->buttonsCount = ARRAYSIZE(settingsButtonsTable);
		break;
	case kMenuLoadStateMode:
		saveLoadButtonsTable[12].data = -52;
		menuData->buttonsTable = saveLoadButtonsTable;
		menuData->buttonsCount = ARRAYSIZE(saveLoadButtonsTable);
		break;
	case kMenuSaveStateMode:
		saveLoadButtonsTable[12].data = -53;
		menuData->buttonsTable = saveLoadButtonsTable;
		menuData->buttonsCount = ARRAYSIZE(saveLoadButtonsTable);
		break;
	default:
		break;
	}
}

void ToucheEngine::redrawMenu(MenuData *menu) {
	Graphics::fillRect(_offscreenBuffer, kScreenWidth, 90, 102, 460, 196, 0xF8);
	Graphics::drawRect(_offscreenBuffer, kScreenWidth, 90, 102, 460, 196, 0xF7, 0xF9);
	Graphics::drawRect(_offscreenBuffer, kScreenWidth, 106, 118, 340, 164, 0xF9, 0xF7);
	switch (menu->mode) {
	case kMenuSettingsMode:
		drawVolumeSlideBar(_offscreenBuffer, kScreenWidth, getMusicVolume());
		menu->buttonsTable[5].data = 0;
		menu->buttonsTable[6].data = 0;
		menu->buttonsTable[7].data = 0;
		menu->buttonsTable[5 + _talkTextMode].data = -86;
		break;
	case kMenuLoadStateMode:
	case kMenuSaveStateMode:
		drawSaveGameStateDescriptions(_offscreenBuffer, kScreenWidth, menu, _saveLoadCurrentPage, _saveLoadCurrentSlot);
		break;
	default:
		break;
	}
	for (uint i = 0; i < menu->buttonsCount; ++i) {
		drawButton(&menu->buttonsTable[i]);
	}
}

void ToucheEngine::handleMenuAction(MenuData *menu, int actionId) {
	switch (actionId) {
	case kActionLoadMenu:
		menu->mode = kMenuLoadStateMode;
		break;
	case kActionSaveMenu:
		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
		menu->mode = kMenuSaveStateMode;
		break;
	case kActionRestartGame:
		restart();
		menu->quit = true;
		break;
	case kActionPlayGame:
		menu->quit = true;
		break;
	case kActionQuitGame:
		quitGame();
		menu->quit = true;
		break;
	case kActionTextOnly:
		_talkTextMode = kTalkModeTextOnly;
		break;
	case kActionVoiceOnly:
		_talkTextMode = kTalkModeVoiceOnly;
		break;
	case kActionTextAndVoice:
		_talkTextMode = kTalkModeVoiceAndText;
		break;
	case kActionLowerVolume:
		adjustMusicVolume(-16);
		break;
	case kActionUpperVolume:
		adjustMusicVolume(+16);
		break;
	case kActionScrollUpSaves:
		--_saveLoadCurrentPage;
		if (_saveLoadCurrentPage < 0) {
			_saveLoadCurrentPage = 9;
		}
		_saveLoadCurrentSlot = _saveLoadCurrentPage * 10 + (_saveLoadCurrentSlot % 10);
		break;
	case kActionScrollDownSaves:
		++_saveLoadCurrentPage;
		if (_saveLoadCurrentPage > 9) {
			_saveLoadCurrentPage = 0;
		}
		_saveLoadCurrentSlot = _saveLoadCurrentPage * 10 + (_saveLoadCurrentSlot % 10);
		break;
	case kActionPerformSaveLoad:
		if (menu->mode == kMenuLoadStateMode) {
			if (loadGameState(_saveLoadCurrentSlot).getCode() == Common::kNoError) {
				menu->quit = true;
			}
		} else if (menu->mode == kMenuSaveStateMode) {
			_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
			const char *description = menu->saveLoadDescriptionsTable[_saveLoadCurrentSlot];
			if (strlen(description) > 0) {
				if (saveGameState(_saveLoadCurrentSlot, description).getCode() == Common::kNoError) {
					menu->quit = true;
				}
			}
		}
		break;
	case kActionCancelSaveLoad:
		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
		menu->mode = kMenuSettingsMode;
		break;
	default:
		if (actionId >= kActionGameState1 && actionId <= kActionGameState10) {
			_saveLoadCurrentSlot = _saveLoadCurrentPage * 10 + (actionId - kActionGameState1);
		}
		break;
	}
}

void ToucheEngine::handleOptions(int forceDisplay) {
	if (_disabledInputCounter == 0 || forceDisplay != 0) {
		setDefaultCursor(_currentKeyCharNum);
		_gameState = kGameStateOptionsDialog;
		MenuData menuData;
		memset(&menuData, 0, sizeof(MenuData));
		menuData.quit = false;
		menuData.exit = false;
		menuData.mode = kMenuSettingsMode;
		int curMode = -1;
		bool doRedraw = false;
		while (!menuData.quit) {
			if (menuData.mode != curMode) {
				doRedraw = true;
				setupMenu(menuData.mode, &menuData);
				curMode = menuData.mode;
				if (menuData.mode == kMenuLoadStateMode || menuData.mode == kMenuSaveStateMode) {
					for (int i = 0; i < kMaxSaveStates; ++i) {
						menuData.saveLoadDescriptionsTable[i][0] = 0;
					}
					Common::String gameStateFileName = generateGameStateFileName(_targetName.c_str(), 0, true);
					Common::StringArray filenames = _saveFileMan->listSavefiles(gameStateFileName);
					for (Common::StringArray::const_iterator it = filenames.begin(); it != filenames.end(); ++it) {
						int i = getGameStateFileSlot(it->c_str());
						if (i >= 0 && i < kMaxSaveStates) {
							Common::InSaveFile *f = _saveFileMan->openForLoading(*it);
							if (f) {
								readGameStateDescription(f, menuData.saveLoadDescriptionsTable[i], 32);
								delete f;
							}
						}
					}
				}
			}
			if (doRedraw) {
				redrawMenu(&menuData);
				updateScreenArea(90, 102, 460, 196);
				doRedraw = false;
			}
			Common::Event event;
			while (_eventMan->pollEvent(event)) {
				const Button *button = 0;
				switch (event.type) {
				case Common::EVENT_RETURN_TO_LAUNCHER:
				case Common::EVENT_QUIT:
					menuData.quit = true;
					menuData.exit = true;
					break;
				case Common::EVENT_LBUTTONDOWN:
					button = menuData.findButtonUnderCursor(event.mouse.x, event.mouse.y);
					if (button) {
						handleMenuAction(&menuData, button->action);
						doRedraw = true;
					}
					break;
				case Common::EVENT_KEYDOWN:
					if (menuData.mode == kMenuSaveStateMode) {
						if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
							menuData.removeLastCharFromDescription(_saveLoadCurrentSlot);
						} else {
							menuData.addCharToDescription(_saveLoadCurrentSlot, (char)event.kbd.ascii);
						}
						doRedraw = true;
					}
					break;
				case Common::EVENT_WHEELUP:
					handleMenuAction(&menuData, kActionScrollUpSaves);
					doRedraw = true;
					break;
				case Common::EVENT_WHEELDOWN:
					handleMenuAction(&menuData, kActionScrollDownSaves);
					doRedraw = true;
					break;
				default:
					break;
				}
			}
			_system->updateScreen();
			_system->delayMillis(10);
		}
		_fullRedrawCounter = 2;
		if (!menuData.exit && shouldQuit()) {
			if (displayQuitDialog())
				quitGame();
		}
		_gameState = kGameStateGameLoop;
	}
}

void ToucheEngine::drawActionsPanel(int dstX, int dstY, int deltaX, int deltaY) {
	Graphics::copyRect(_offscreenBuffer, kScreenWidth, dstX, dstY,
	  _menuKitData, 42, 0, 0,
	  14, 24,
	  Graphics::kTransparent);
	Graphics::copyRect(_offscreenBuffer, kScreenWidth, deltaX - 14 + dstX, dstY,
	  _menuKitData, 42, 0, 40,
	  14, 24,
	  Graphics::kTransparent);
	Graphics::copyRect(_offscreenBuffer, kScreenWidth, dstX, deltaY - 16 + dstY,
	  _menuKitData, 42, 0, 24,
	  14, 16,
	  Graphics::kTransparent);
	Graphics::copyRect(_offscreenBuffer, kScreenWidth, deltaX - 14 + dstX, deltaY - 16 + dstY,
	  _menuKitData, 42, 0, 64,
	  14, 16,
	  Graphics::kTransparent);
	int x1 = deltaX - 28;
	int x2 = dstX + 14;
	while (x1 > 0) {
		int w = (x1 > 14) ? 14 : x1;
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, x2, dstY,
		  _menuKitData, 42, 0, 80,
		  w, 24,
		  Graphics::kTransparent);
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, x2, deltaY - 16 + dstY,
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
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, dstX, x2,
		  _menuKitData, 42, 14, 0,
		  14, w,
		  Graphics::kTransparent);
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, deltaX - 14 + dstX, x2,
		  _menuKitData, 42, 28, 0,
		  14, w,
		  Graphics::kTransparent);
		x1 -= 120;
		x2 += 120;
	}
}

void ToucheEngine::drawConversationPanelBorder(int dstY, int srcX, int srcY) {
	int dstX = 24;
	int w = 48;
	for (int i = 0; i < 13; ++i) {
		if (i == 12) {
			w = 34;
		}
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, dstX, dstY, _convKitData, 152, srcX, srcY, w, 6);
		dstX += w;
	}
}

void ToucheEngine::drawConversationPanel() {
	Graphics::copyRect(_offscreenBuffer, kScreenWidth, 0, 320, _convKitData, 152, 0, 0, 72, 80);
	int dstX = 54;
	int dstY = 326;
	int w = 96;
	for (int i = 0; i < 7; ++i) {
		if (i == 5) {
			w = 50;
		}
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, dstX, dstY, _convKitData, 152, 24, 6, w, 68);
		dstX += w;
	}
	--dstX;
	Graphics::copyRect(_offscreenBuffer, kScreenWidth, dstX, 320, _convKitData, 152, 120, 0, 7, 80);
	dstX -= 3;
	if (_scrollConversationChoiceOffset != 0) {
		drawConversationPanelBorder(320, 72, 0);
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, 0, 320, _convKitData, 152, 128, 0, 24, 21);
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, dstX, 320, _convKitData, 152, 128, 34, 10, 10);
	} else {
		drawConversationPanelBorder(320, 24, 0);
	}
	if (_conversationChoicesTable[_scrollConversationChoiceOffset + 4].msg != 0) {
		drawConversationPanelBorder(394, 72, 74);
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, 0, 379, _convKitData, 152, 128, 59, 24, 21);
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, dstX, 394, _convKitData, 152, 128, 46, 10, 6);
	} else {
		drawConversationPanelBorder(394, 24, 74);
	}
}

void ToucheEngine::printStatusString(const char *str) {
	Graphics::fillRect(_offscreenBuffer, kScreenWidth, 0, 0, kScreenWidth, kTextHeight, 0xD7);
	Graphics::drawRect(_offscreenBuffer, kScreenWidth, 0, 0, kScreenWidth, kTextHeight, 0xD6, 0xD8);
	Graphics::drawString16(_offscreenBuffer, kScreenWidth, 0xFF, 0, 0, str);
	updateScreenArea(0, 0, kScreenWidth, kTextHeight);
	_system->updateScreen();
}

void ToucheEngine::clearStatusString() {
	Graphics::copyRect(_offscreenBuffer, kScreenWidth, 0, 0,
	  _backdropBuffer, _currentBitmapWidth, _flagsTable[614], _flagsTable[615],
	  kScreenWidth, kTextHeight);
	updateScreenArea(0, 0, kScreenWidth, kTextHeight);
}

int ToucheEngine::displayQuitDialog() {
	debug(kDebugMenu, "ToucheEngine::displayQuitDialog()");
	_gameState = kGameStateQuitDialog;
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	printStatusString(getString(-85));
	int ret = 0;
	bool quitLoop = false;
	while (!quitLoop) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RETURN_TO_LAUNCHER:
			case Common::EVENT_QUIT:
				quitLoop = true;
				ret = 1;
				break;
			case Common::EVENT_KEYDOWN:
				quitLoop = true;
				switch (_language) {
				case Common::FR_FRA:
					if (event.kbd.keycode == Common::KEYCODE_o) {
						ret = 1;
					}
					break;
				case Common::DE_DEU:
					if (event.kbd.keycode == Common::KEYCODE_j) {
						ret = 1;
					}
					break;
				case Common::ES_ESP:
					if (event.kbd.keycode == Common::KEYCODE_s) {
						ret = 1;
					}
					break;
				case Common::PL_POL:
					if (event.kbd.keycode == Common::KEYCODE_s || event.kbd.keycode == Common::KEYCODE_t) {
						ret = 1;
					}
					break;
				default:
					// According to cyx, the Italian version uses the same
					// keys as the English one.
					if (event.kbd.keycode == Common::KEYCODE_y) {
						ret = 1;
					}
					break;
				}
				break;
			default:
				break;
			}
		}
		_system->delayMillis(10);
		_system->updateScreen();
	}
	clearStatusString();
	_gameState = kGameStateGameLoop;
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	return ret;
}

void ToucheEngine::displayTextMode(int str) {
	debug(kDebugMenu, "ToucheEngine::displayTextMode(%d)", str);
	printStatusString(getString(str));
	_system->delayMillis(1000);
	clearStatusString();
}

} // namespace Touche
