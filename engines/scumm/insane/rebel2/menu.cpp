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
 */

#include "common/config-manager.h"
#include "common/system.h"
#include "common/events.h"
#include "common/util.h"
#include "graphics/paletteman.h"

#include "audio/mixer.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "scumm/scumm_v7.h"

#include "scumm/smush/smush_player.h"
#include "scumm/smush/rebel/font_rebel2.h"
#include "scumm/smush/rebel/smush_multi_font.h"

#include "scumm/insane/rebel2/rebel.h"

namespace Scumm {

static void setRebel2MixerVolume(ScummEngine_v7 *vm, int volumeLevel) {
	const int mixerVolume = CLIP<int>(volumeLevel * 2, 0, (int)Audio::Mixer::kMaxMixerVolume);
	vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, mixerVolume);
	vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, mixerVolume);
	vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, mixerVolume);
}

void InsaneRebel2::resetMenu() {
	_menuSelection = 0;
	_menuInactivityTimer = 0;
	_menuInactivityTimedOut = false;
	_menuRepeatDelay = 0;
	resetMenuGamepadAxis();
	_menuSelectionConfirmed = false;
	setVirtualKeyboardVisible(false);
}

bool InsaneRebel2::isMenuTextInputActive() const {
	if (!_menuInputActive)
		return false;

	if (_gameState == kStatePilotSelect && _pilotMenuMode == kPilotModeNameInput)
		return true;

	return _gameState == kStateChapterSelect &&
	       _chapterSelection >= 0 &&
	       _chapterSelection < 16 &&
	       !_chapterUnlocked[_chapterSelection];
}

void InsaneRebel2::setVirtualKeyboardVisible(bool visible) {
	if (!_vm->_system->hasFeature(OSystem::kFeatureVirtualKeyboard))
		return;

	if (_virtualKeyboardActive == visible)
		return;

	_vm->_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, visible);
	_virtualKeyboardActive = visible;
}

void InsaneRebel2::updateMenuVirtualKeyboard() {
	setVirtualKeyboardVisible(isMenuTextInputActive());
}

void InsaneRebel2::unlockAllChapters() {
	debugC(DEBUG_INSANE, "Unlocking all chapters for testing");
	for (int i = 0; i < 16; i++) {
		_chapterUnlocked[i] = true;
		_levelUnlocked[i] = true;
	}
}

Common::String InsaneRebel2::getRandomMenuVideo() {
	int variant;
	do {
		variant = _vm->_rnd.getRandomNumber(14);
	} while (variant == _lastMenuVariant && _lastMenuVariant >= 0);
	_lastMenuVariant = variant;

	char letter = 'A' + variant;
	debugC(DEBUG_INSANE, "Selected menu variant %c", letter);
	return Common::String::format("OPEN/O_MENU_%c.SAN", letter);
}

int InsaneRebel2::processMenuInput() {

	int result = -1;

	const bool highRes = isHiRes();
	const int baseY = highRes ? (_menuItemCount * -5 + 0x5a) * 2 + 0x1c : _menuItemCount * -5 + 0x68;
	const int itemSpacing = highRes ? 20 : 10;
	const int itemHitTop = highRes ? 2 : 1;
	const int itemHitHeight = highRes ? 18 : 10;

	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.pop();
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_menuInactivityTimer = 0;

			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				_menuSelection--;
				if (_menuSelection < 0) {
					_menuSelection = _menuItemCount - 1;
				}
				_menuRepeatDelay = 3;
				debugC(DEBUG_INSANE, "Menu: Selection changed to %d (UP)", _menuSelection);
				break;

			case Common::KEYCODE_DOWN:
				_menuSelection++;
				if (_menuSelection >= _menuItemCount) {
					_menuSelection = 0;
				}
				_menuRepeatDelay = 3;
				debugC(DEBUG_INSANE, "Menu: Selection changed to %d (DOWN)", _menuSelection);
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				if (_menuSelection >= 0 && _menuSelection < _menuItemCount) {
					result = _menuSelection;
					debugC(DEBUG_INSANE, "Menu: Item %d selected (ENTER)", _menuSelection);
				}
				break;

			case Common::KEYCODE_ESCAPE:
				result = _menuItemCount - 1;
				debugC(DEBUG_INSANE, "Menu: Back action - selecting quit (item %d)", result);
				break;

			default:
				break;
			}
			break;

		case Common::EVENT_LBUTTONDOWN:
			_menuInactivityTimer = 0;
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			for (int i = 0; i < _menuItemCount; i++) {
				int itemY = baseY + i * itemSpacing;
				if (event.mouse.y >= itemY - itemHitTop && event.mouse.y < itemY - itemHitTop + itemHitHeight) {
					_menuSelection = i;
					result = _menuSelection;
					debugC(DEBUG_INSANE, "Menu: Item %d selected (mouse)", _menuSelection);
					break;
				}
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			{
				int mouseY = event.mouse.y;
				for (int i = 0; i < _menuItemCount; i++) {
					int itemY = baseY + i * itemSpacing;
					if (mouseY >= itemY - itemHitTop && mouseY < itemY - itemHitTop + itemHitHeight) {
						if (i != _menuSelection) {
							_menuSelection = i;
							debugC(DEBUG_INSANE, "Menu: Hover selection changed to %d (mouseY=%d)", _menuSelection, mouseY);
						}
						break;
					}
				}
			}
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			result = _menuItemCount - 1;
			break;

		default:
			break;
		}
	}

	if (_menuRepeatDelay > 0) {
		_menuRepeatDelay--;
	}

	return result;
}

void InsaneRebel2::drawMenuItems(byte *renderBitmap, int pitch, int width, int height,
                                  const char **items, int numItems, int selection,
                                  bool leftAligned) {
	const bool highRes = isHiRes();
	const int centerX = highRes ? 0x140 : width / 2;
	const int titleY = highRes ?
		(numItems * -5 + 0x5a) * 2 + (leftAligned ? -8 : -0x12) :
		numItems * -5 + (leftAligned ? 0x56 : 0x51);
	const int itemBaseY = highRes ? (numItems * -5 + 0x5a) * 2 + 0x1c : numItems * -5 + 0x68;
	const int itemSpacing = highRes ? 20 : 10;

	NutRenderer *fonts[3] = { _smush_talkfontNut, _smush_smalfontNut, _smush_titlefontNut };
	NutRenderer *defaultFont = fonts[0] ? fonts[0] : _smush_smalfontNut;
	if (!defaultFont)
		return;

	Common::Rect clipRect(0, 0, _vm->_screenWidth, _vm->_screenHeight);
	int actualPitch = _vm->_screenWidth;

	auto getStringWidth = [&](const char *str) -> int {
		return getMenuStringWidth(str);
	};

	auto drawString = [&](const char *str, int x, int y) {
		drawMenuString(renderBitmap, str, x, y, 1);
	};

	{
		int titleWidth = getStringWidth(items[0]);
		int titleX = leftAligned ? (highRes ? 0x50 : 0x28) : (centerX - titleWidth / 2);
		drawString(items[0], titleX, titleY);
	}

	for (int i = 0; i < numItems; i++) {
		int itemY = itemBaseY + i * itemSpacing;
		const char *text = items[i + 1];

		int textWidth = getStringWidth(text);
		int textX = leftAligned ? (highRes ? 0x2e : 0x17) : (centerX - textWidth / 2);
		drawString(text, textX, itemY);

		if (i == selection) {
			int bracketWidth = textWidth + (highRes ? 12 : 6);
			int bracketHeight = highRes ? 20 : 10;

			byte highlightColor = ((_vm->_system->getMillis() / 133) & 1) ? 248 : 240;

			int leftX = leftAligned ? (highRes ? 0x28 : 0x14) : (centerX - bracketWidth / 2);
			int rightX = leftX + bracketWidth;
			int topY = highRes ? itemY - 2 : itemY - 1;
			int bottomY = topY + bracketHeight - 1;

			int screenW = _vm->_screenWidth;
			int screenH = _vm->_screenHeight;
			if (leftX < 0)
				leftX = 0;
			if (rightX >= screenW)
				rightX = screenW - 1;
			if (topY < 0)
				topY = 0;
			if (bottomY >= screenH)
				bottomY = screenH - 1;

			for (int x = leftX; x <= rightX && x < screenW; x++) {
				if (topY >= 0 && topY < screenH)
					renderBitmap[topY * actualPitch + x] = highlightColor;
				if (bottomY >= 0 && bottomY < screenH)
					renderBitmap[bottomY * actualPitch + x] = highlightColor;
			}
			for (int py = topY; py <= bottomY && py < screenH; py++) {
				if (leftX >= 0 && leftX < screenW)
					renderBitmap[py * actualPitch + leftX] = highlightColor;
				if (rightX >= 0 && rightX < screenW)
					renderBitmap[py * actualPitch + rightX] = highlightColor;
			}
		}
	}
}

int InsaneRebel2::parseFormatCode(const char *&str, int &outColor) {
	if (*str != '^')
		return -1;
	const char *p = str + 1;
	if (*p == '^') { str = p; return -1; }
	if (*p == 'f') {
		p++;
		int idx = 0;
		while (*p >= '0' && *p <= '9') { idx = idx * 10 + (*p - '0'); p++; }
		str = p;
		return (idx >= 0 && idx < 3) ? idx : 0;
	}
	if (*p == 'c') {
		p++;
		int color = 0;
		while (*p >= '0' && *p <= '9') { color = color * 10 + (*p - '0'); p++; }
		str = p;
		outColor = color;
		return -2;
	}
	if (*p == 'l') { str = p + 1; return -2; }
	return -1;
}

int InsaneRebel2::getMenuStringWidth(const char *str) const {
	NutRenderer *fonts[3] = { _smush_talkfontNut, _smush_smalfontNut, _smush_titlefontNut };
	NutRenderer *defaultFont = fonts[0] ? fonts[0] : _smush_smalfontNut;
	if (!defaultFont)
		return 0;

	int w = 0;
	NutRenderer *curFont = defaultFont;
	int dummyColor = 0;
	while (*str) {
		int fc = parseFormatCode(str, dummyColor);
		if (fc >= 0) { curFont = (fonts[fc] ? fonts[fc] : defaultFont); continue; }
		if (fc == -2) continue;
		byte c = (byte)*str++;
		if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
		if (curFont && c < curFont->getNumChars())
			w += curFont->getCharWidth(c);
	}
	return w;
}

void InsaneRebel2::drawMenuString(byte *renderBitmap, const char *str, int x, int y, int defaultColor) {
	NutRenderer *fonts[3] = { _smush_talkfontNut, _smush_smalfontNut, _smush_titlefontNut };
	NutRenderer *defaultFont = fonts[0] ? fonts[0] : _smush_smalfontNut;
	if (!defaultFont)
		return;

	Common::Rect clipRect(0, 0, _vm->_screenWidth, _vm->_screenHeight);
	int pitch = _vm->_screenWidth;

	NutRenderer *curFont = defaultFont;
	int curColor = defaultColor;
	while (*str) {
		int fc = parseFormatCode(str, curColor);
		if (fc >= 0) { curFont = (fonts[fc] ? fonts[fc] : defaultFont); continue; }
		if (fc == -2) continue;
		byte c = (byte)*str++;
		if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
		if (!curFont || c >= curFont->getNumChars()) continue;
		int charW = curFont->getCharWidth(c);
		if (x >= 0 && y >= 0 && charW > 0)
			drawRebel2Char(curFont, renderBitmap, clipRect, x, y, pitch, curColor, c);
		x += charW;
	}
}

void InsaneRebel2::drawMenuStringCentered(byte *renderBitmap, const char *str, int cx, int y, int defaultColor) {
	int w = getMenuStringWidth(str);
	drawMenuString(renderBitmap, str, cx - w / 2, y, defaultColor);
}

void InsaneRebel2::drawMenuStringRight(byte *renderBitmap, const char *str, int rx, int y, int defaultColor) {
	int w = getMenuStringWidth(str);
	drawMenuString(renderBitmap, str, rx - w, y, defaultColor);
}

void InsaneRebel2::drawMenuOverlay(byte *renderBitmap, int pitch, int width, int height) {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer) {
		debugC(DEBUG_INSANE, "drawMenuOverlay: SmushPlayer not available for TRS strings!");
		return;
	}

	const char *menuItems[8];
	for (int i = 0; i < 8; i++) {
		menuItems[i] = splayer->getString(10 + i);
		if (!menuItems[i] || !menuItems[i][0]) {
			debugC(DEBUG_INSANE, "drawMenuOverlay: TRS string %d not found!", 10 + i);
			menuItems[i] = "";
		}
	}

	drawMenuItems(renderBitmap, pitch, width, height, menuItems, 7, _menuSelection);
}

void pauseFillRect(byte *buf, int bufW, int bufH, int x, int y, int w, int h, byte color) {
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > bufW) w = bufW - x;
	if (y + h > bufH) h = bufH - y;
	if (w <= 0 || h <= 0) return;
	for (int row = y; row < y + h; row++)
		memset(buf + row * bufW + x, color, w);
}

void InsaneRebel2::showPauseOverlay() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer)
		return;

	byte *frameBuffer = splayer->_dst;
	byte *palette = splayer->_pal;
	int width = splayer->_width;
	int height = splayer->_height;

	if (!frameBuffer || !palette || width <= 0 || height <= 0)
		return;

	int screenW = MIN(width, (int)_vm->_screenWidth);
	int screenH = MIN(height, (int)_vm->_screenHeight);

	memcpy(_savedPausePalette, palette, 768);
	_pauseOverlayActive = true;

	byte dimPal[768];
	memcpy(dimPal, palette, 768);
	for (int i = 0; i < 768; i++)
		dimPal[i] >>= 2;

	dimPal[0x50 * 3 + 0] = 80; dimPal[0x50 * 3 + 1] = 80; dimPal[0x50 * 3 + 2] = 80;
	dimPal[0x51 * 3 + 0] = 110; dimPal[0x51 * 3 + 1] = 110; dimPal[0x51 * 3 + 2] = 110;
	dimPal[1 * 3 + 0] = 255; dimPal[1 * 3 + 1] = 255; dimPal[1 * 3 + 2] = 255;
	dimPal[2 * 3 + 0] = 188; dimPal[2 * 3 + 1] = 188; dimPal[2 * 3 + 2] = 188;
	dimPal[3 * 3 + 0] = 128; dimPal[3 * 3 + 1] = 128; dimPal[3 * 3 + 2] = 128;
	dimPal[4 * 3 + 0] = 0;   dimPal[4 * 3 + 1] = 0;   dimPal[4 * 3 + 2] = 0;
	dimPal[5 * 3 + 0] = 252; dimPal[5 * 3 + 1] = 252; dimPal[5 * 3 + 2] = 252;

	_vm->_system->getPaletteManager()->setPalette(dimPal, 0, 256);

	pauseFillRect(frameBuffer, width, height, 0, 0x17, 0x140, 2, 0x50);
	pauseFillRect(frameBuffer, width, height, 0, 0xAF, 0x140, 2, 0x50);

	pauseFillRect(frameBuffer, width, height, 0,     0, 0x28, 200, 0x50);
	pauseFillRect(frameBuffer, width, height, 0x118, 0, 0x28, 200, 0x50);

	for (int i = 0; i < 6; i++) {
		int yOff = i * 0x24;  // i * 36
		pauseFillRect(frameBuffer, width, height, 0x0C, yOff,     0x19, 0x11, 0x51);
		pauseFillRect(frameBuffer, width, height, 0x0B, yOff + 1, 0x1B, 0x0F, 0x51);
		pauseFillRect(frameBuffer, width, height, 0x0D, yOff,     0x17, 0x11, 4);
		pauseFillRect(frameBuffer, width, height, 0x0B, yOff + 2, 0x1B, 0x0D, 4);
		pauseFillRect(frameBuffer, width, height, 0x0C, yOff + 1, 0x19, 0x0F, 4);
	}

	for (int i = 0; i < 6; i++) {
		int yOff = i * 0x24;
		int xBase = 0x11A;
		pauseFillRect(frameBuffer, width, height, xBase,     yOff,     0x19, 0x11, 0x51);
		pauseFillRect(frameBuffer, width, height, xBase - 1, yOff + 1, 0x1B, 0x0F, 0x51);
		pauseFillRect(frameBuffer, width, height, xBase + 1, yOff,     0x17, 0x11, 4);
		pauseFillRect(frameBuffer, width, height, xBase - 1, yOff + 2, 0x1B, 0x0D, 4);
		pauseFillRect(frameBuffer, width, height, xBase,     yOff + 1, 0x19, 0x0F, 4);
	}

	const char *pauseText = splayer->getString(0x78);
	if (!pauseText || !pauseText[0])
		pauseText = "Game Paused";

	SmushMultiFont *multiFont = splayer->getMultiFont();
	if (!multiFont) {
		splayer->ensureMultiFont();
		multiFont = splayer->getMultiFont();
	}
	if (multiFont) {
		Common::Rect clipRect(0, 0, screenW, screenH);
		multiFont->drawString(pauseText, frameBuffer, clipRect, 10, 10, width, 4, kStyleAlignLeft);
	}

	_vm->_system->copyRectToScreen(frameBuffer, width, 0, 0, screenW, screenH);
	_vm->_system->updateScreen();
}

int InsaneRebel2::runMainMenu() {

	debugC(DEBUG_INSANE, "Entering main menu");

	resetMenu();
	_gameState = kStateMainMenu;

	_menuInputActive = true;
	while (!_menuEventQueue.empty())
		_menuEventQueue.pop();

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	while (!_vm->shouldQuit()) {
		_vm->_smushVideoShouldFinish = false;

		Common::String menuVideo = getRandomMenuVideo();
		debugC(DEBUG_INSANE, "Playing menu video: %s", menuVideo.c_str());

		splayer->setCurVideoFlags(0x20);

		splayer->play(menuVideo.c_str(), 15);

		if (_vm->shouldQuit()) {
			_menuInputActive = false;
			return kMenuQuit;
		}

		if (_menuInactivityTimedOut) {
			debugC(DEBUG_INSANE, "Main menu inactivity - resuming intro/demo loop");
			_menuInactivityTimedOut = false;
			_menuInputActive = false;
			return kMenuResumeDemo;
		}

		if (!_menuSelectionConfirmed) {
			continue;
		}

		_vm->_smushVideoShouldFinish = false;
		_menuSelectionConfirmed = false;

		debugC(DEBUG_INSANE, "Menu video ended with selection=%d", _menuSelection);

		switch (_menuSelection) {
		case 0:
			debugC(DEBUG_INSANE, "Start Game selected - going to pilot selection");
			_gameState = kStatePilotSelect;
			_menuInputActive = false;
			return kMenuNewGame;

		case 1:
			debugC(DEBUG_INSANE, "Options selected");
			showOptionsMenu();
			break;

		case 2:
			debugC(DEBUG_INSANE, "Calibrate Joystick selected - no-op for modern joystick support");
			break;

		case 3:
			debugC(DEBUG_INSANE, "Continue Intro selected - resuming intro/demo loop");
			_menuInputActive = false;
			return kMenuResumeDemo;

		case 4:
			debugC(DEBUG_INSANE, "Show Top Pilots selected");
			showTopPilots();
			break;

		case 5:
			debugC(DEBUG_INSANE, "Show Credits selected - playing O_CREDIT.SAN");
			_gameState = kStateCredits;
			_menuInputActive = false;
			splayer->setCurVideoFlags(0x20);
			splayer->play("OPEN/O_CREDIT.SAN", 15);
			_gameState = kStateMainMenu;
			_menuInputActive = true;
			resetMenuGamepadAxis();
			break;

		case 6:
			debugC(DEBUG_INSANE, "Return to Launcher selected");
			_menuInputActive = false;
			return kMenuQuit;

		default:
			debugC(DEBUG_INSANE, "Unknown menu selection %d", _menuSelection);
			break;
		}
	}

	_menuInputActive = false;
	return kMenuQuit;
}

int InsaneRebel2::runChapterSelect() {

	debugC(DEBUG_INSANE, "Entering chapter selection (FUN_00415CF8)");

	_menuInputActive = true;
	while (!_menuEventQueue.empty())
		_menuEventQueue.pop();
	resetMenuGamepadAxis();

	_chapterSelection = 15;
	while (_chapterSelection > 0 && !_chapterUnlocked[_chapterSelection]) {
		_chapterSelection--;
	}
	_chapterItemCount = 17;
	_selectedChapter = 0;
	_passwordInput = "";
	_menuRepeatDelay = 0;
	_gameState = kStateChapterSelect;
	updateMenuVirtualKeyboard();

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	_previewOffsetX = -90;
	_previewOffsetY = _chapterSelection * -50 + 75;

	// O_LEVEL.SAN SKIP chunks use bits 16..1 for chapter lock state.
	for (int i = 0; i < 16; i++) {
		if (_chapterUnlocked[i])
			setBit(16 - i);
		else
			clearBit(16 - i);
	}

	while (!_vm->shouldQuit()) {
		_vm->_smushVideoShouldFinish = false;

		debugC(DEBUG_INSANE, "Playing chapter select background: OPEN/O_LEVEL.SAN");

		splayer->setCurVideoFlags(0x28);

		splayer->play("OPEN/O_LEVEL.SAN", 15);

		if (_vm->shouldQuit()) {
			setVirtualKeyboardVisible(false);
			_menuInputActive = false;
			return kChapterSelectQuit;
		}

		if (!_menuSelectionConfirmed) {
			continue;
		}

		_vm->_smushVideoShouldFinish = false;
		_menuSelectionConfirmed = false;

		debugC(DEBUG_INSANE, "Chapter selection made: %d", _chapterSelection);

		if (_chapterSelection == 16) {
			debugC(DEBUG_INSANE, "BACK to main menu selected");
			setVirtualKeyboardVisible(false);
			_menuInputActive = false;
			return kChapterSelectBack;
		}

		if (_chapterSelection >= 0 && _chapterSelection < 16) {
			if (_chapterUnlocked[_chapterSelection]) {
				_selectedChapter = _chapterSelection;
				debugC(DEBUG_INSANE, "Chapter %d selected (unlocked)", _selectedChapter + 1);
				setVirtualKeyboardVisible(false);
				_menuInputActive = false;
				return kChapterSelectPlay;
			}

			if (_activePilot >= 0 && _activePilot < _numPilots &&
			    _pilots[_activePilot].difficulty < 6 && _chapterSelection > 0) {
				Common::String expected = getChapterPassword(
					_chapterSelection, _pilots[_activePilot].difficulty);
				if (!expected.empty() && _passwordInput.equalsIgnoreCase(expected)) {
					PilotData &pilot = _pilots[_activePilot];
					pilot.score[_chapterSelection] = 0;
					pilot.damage[_chapterSelection] = 0;
					pilot.lives[_chapterSelection] = 3;
					pilot.rating[_chapterSelection] = 0;
					savePilots();
					_chapterUnlocked[_chapterSelection] = true;
					setBit(16 - _chapterSelection);
					_passwordInput.clear();
					updateMenuVirtualKeyboard();
					debugC(DEBUG_INSANE, "Chapter %d unlocked via password", _chapterSelection + 1);
					continue;
				}
			}
			_passwordInput.clear();
			playSfx(3, 127, 0);
			debugC(DEBUG_INSANE, "Password rejected for chapter %d", _chapterSelection + 1);
		}
	}

	setVirtualKeyboardVisible(false);
	_menuInputActive = false;
	return kChapterSelectQuit;
}

int InsaneRebel2::processChapterSelectInput() {
	int result = -1;

	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.front();
		_menuEventQueue.pop();

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				_chapterSelection--;
				if (_chapterSelection < 0) {
					_chapterSelection = _chapterItemCount - 1;
				}
				_passwordInput.clear();
				_previewOffsetY = _chapterSelection * -50 + 75;
				updateMenuVirtualKeyboard();
				debugC(DEBUG_INSANE, "ChapterSelect: Selection changed to %d (UP) offsetY=%d", _chapterSelection, _previewOffsetY);
				break;

			case Common::KEYCODE_DOWN:
				_chapterSelection++;
				if (_chapterSelection >= _chapterItemCount) {
					_chapterSelection = 0;
				}
				_passwordInput.clear();
				_previewOffsetY = _chapterSelection * -50 + 75;
				updateMenuVirtualKeyboard();
				debugC(DEBUG_INSANE, "ChapterSelect: Selection changed to %d (DOWN) offsetY=%d", _chapterSelection, _previewOffsetY);
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				if (_chapterSelection >= 0 && _chapterSelection < _chapterItemCount) {
					result = _chapterSelection;
					debugC(DEBUG_INSANE, "ChapterSelect: Item %d selected (ENTER)", _chapterSelection);
				}
				break;

			case Common::KEYCODE_ESCAPE:
				setVirtualKeyboardVisible(false);
				result = 16;
				debugC(DEBUG_INSANE, "ChapterSelect: Back action - back to menu");
				break;

			case Common::KEYCODE_BACKSPACE:
				if (!_passwordInput.empty()) {
					_passwordInput.deleteLastChar();
					debugC(DEBUG_INSANE, "ChapterSelect: Password backspace, now: %s", _passwordInput.c_str());
				}
				break;

			default:
				if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7E) {
					if (_passwordInput.size() < 8) {
						_passwordInput += (char)event.kbd.ascii;
						debugC(DEBUG_INSANE, "ChapterSelect: Password input: %s", _passwordInput.c_str());
					}
				}
				break;
			}
			break;

		case Common::EVENT_LBUTTONDOWN:
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			{
				const bool highRes = isHiRes();
				const int baseY = highRes ? (_chapterItemCount * -5 + 0x5a) * 2 + 0x1c : _chapterItemCount * -5 + 0x68;
				const int itemSpacing = highRes ? 20 : 10;
				const int itemHitTop = highRes ? 2 : 1;
				const int itemHitHeight = highRes ? 18 : 10;
				for (int i = 0; i < _chapterItemCount; i++) {
					int itemY = baseY + i * itemSpacing;
					if (event.mouse.y >= itemY - itemHitTop && event.mouse.y < itemY - itemHitTop + itemHitHeight) {
						_chapterSelection = i;
						_previewOffsetY = _chapterSelection * -50 + 75;
						updateMenuVirtualKeyboard();
						result = _chapterSelection;
						debugC(DEBUG_INSANE, "ChapterSelect: Item %d selected (mouse)", _chapterSelection);
						break;
					}
				}
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			{
				const bool highRes = isHiRes();
				const int baseY = highRes ? (_chapterItemCount * -5 + 0x5a) * 2 + 0x1c : _chapterItemCount * -5 + 0x68;
				const int itemSpacing = highRes ? 20 : 10;
				const int itemHitTop = highRes ? 2 : 1;
				const int itemHitHeight = highRes ? 18 : 10;
				int mouseY = event.mouse.y;

				for (int i = 0; i < _chapterItemCount; i++) {
					int itemY = baseY + i * itemSpacing;
					if (mouseY >= itemY - itemHitTop && mouseY < itemY - itemHitTop + itemHitHeight) {
						if (i != _chapterSelection) {
							_chapterSelection = i;
							_previewOffsetY = _chapterSelection * -50 + 75;
							updateMenuVirtualKeyboard();
							debugC(DEBUG_INSANE, "ChapterSelect: Hover changed to %d", _chapterSelection);
						}
						break;
					}
				}
			}
			break;

		default:
			break;
		}
	}

	return result;
}

void InsaneRebel2::drawPreviewBox(byte *renderBitmap, int pitch, int width, int height) {
	const int scale = isHiRes() ? 2 : 1;

	int outerX = 228 * scale, outerY = 73 * scale, outerW = 84 * scale, outerH = 54 * scale;
	byte outerColor = 0xF8;

	for (int px = outerX; px < outerX + outerW && px < width; px++) {
		if (outerY >= 0 && outerY < height && px >= 0)
			renderBitmap[outerY * pitch + px] = outerColor;
	}
	int bottomY = outerY + outerH - 1;
	if (bottomY < height) {
		for (int px = outerX; px < outerX + outerW && px < width; px++) {
			if (px >= 0)
				renderBitmap[bottomY * pitch + px] = outerColor;
		}
	}
	for (int py = outerY; py < outerY + outerH && py < height; py++) {
		if (py >= 0 && outerX >= 0 && outerX < width)
			renderBitmap[py * pitch + outerX] = outerColor;
	}
	int rightX = outerX + outerW - 1;
	if (rightX < width) {
		for (int py = outerY; py < outerY + outerH && py < height; py++) {
			if (py >= 0)
				renderBitmap[py * pitch + rightX] = outerColor;
		}
	}

	int innerX = 229 * scale, innerY = 74 * scale, innerW = 82 * scale, innerH = 52 * scale;
	byte innerColor = 4;

	for (int px = innerX; px < innerX + innerW && px < width; px++) {
		if (innerY >= 0 && innerY < height && px >= 0)
			renderBitmap[innerY * pitch + px] = innerColor;
	}
	bottomY = innerY + innerH - 1;
	if (bottomY < height) {
		for (int px = innerX; px < innerX + innerW && px < width; px++) {
			if (px >= 0)
				renderBitmap[bottomY * pitch + px] = innerColor;
		}
	}
	for (int py = innerY; py < innerY + innerH && py < height; py++) {
		if (py >= 0 && innerX >= 0 && innerX < width)
			renderBitmap[py * pitch + innerX] = innerColor;
	}
	rightX = innerX + innerW - 1;
	if (rightX < width) {
		for (int py = innerY; py < innerY + innerH && py < height; py++) {
			if (py >= 0)
				renderBitmap[py * pitch + rightX] = innerColor;
		}
	}
}

Common::String InsaneRebel2::getRankString(int rating) {
	if (rating > 50)
		rating = 50;
	Common::String result;
	while (rating >= 9) { result += (char)3; rating -= 9; }
	while (rating >= 3) { result += (char)2; rating -= 3; }
	while (rating >= 1) { result += (char)1; rating -= 1; }
	return result;
}

const char *const kPasswordTable[90] = {
	"JABBA",    "EWOKS",    "BANTHA",   "ANAKIN",   "WOOKIEE",  "WOOKIEE",
	"ENDOR",    "CHEWIE",   "KATANA",   "KENOBI",   "DROID",    "DROID",
	"LACHTON",  "DANKIN",   "DENGAR",   "FORTUNA",  "RODIAN",   "RODIAN",
	"BORSK",    "NOGHRI",   "PELLAEON", "MODON",    "BPFASSH",  "BPFASSH",
	"KROYIES",  "CHAMMA",   "ITHULL",   "OMMIN",    "KSHYY",    "KSHYY",
	"AURIL",    "BOGGA",    "STENNESS", "REKKON",   "TORVE",    "TORVE",
	"KAMPL",    "INCOM",    "MYRKR",    "SHAZEEN",  "SLUISSI",  "SLUISSI",
	"FERRIER",  "KOTHLIS",  "CHURBA",   "KIIRIUM",  "PALANHI",  "PALANHI",
	"GALIA",    "KRATH",    "ARTOO",    "GUNDARK",  "DROKKO",   "DROKKO",
	"DENARII",  "SIOSK",    "SATAL",    "DIANOGA",  "NATTH",    "NATTH",
	"SADOW",    "ADEGAN",   "LOBUE",    "ATUARRE",  "SABACC",   "SABACC",
	"ONDERON",  "AMANOA",   "DENEBA",   "ESSADA",   "ANDUR",    "ANDUR",
	"ALEEMA",   "AMBRIA",   "STURM",    "PAPLOO",   "ARKANIA",  "ARKANIA",
	"CATHAR",   "SYLVAR",   "CRADO",    "NASHTAH",  "DIATH",    "DIATH",
	"DOMINIS",  "MIRALUKA", "CARRACK",  "PESTAGE",  "DREEBO",   "DREEBO",
};

Common::String InsaneRebel2::getChapterPassword(int level, int difficulty) {
	if (level < 1 || level > 15 || difficulty < 0 || difficulty > 5)
		return "";
	int idx = difficulty + (level * 3 - 3) * 2;
	return kPasswordTable[idx];
}

void InsaneRebel2::drawChapterInfoLine(byte *renderBitmap, int pitch, int width, int height) {
	if (_chapterSelection < 0 || _chapterSelection >= 16)
		return;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer)
		return;

	const int scale = isHiRes() ? 2 : 1;

	if (_chapterUnlocked[_chapterSelection]) {
		const char *fmtStr = splayer->getString(80);
		if (!fmtStr || !fmtStr[0])
			return;

		int32 pilotLives = 0;
		int32 pilotScore = 0;
		int16 pilotRating = 0;
		if (_activePilot >= 0 && _activePilot < _numPilots) {
			pilotLives = _pilots[_activePilot].lives[_chapterSelection];
			pilotScore = _pilots[_activePilot].score[_chapterSelection];
			pilotRating = _pilots[_activePilot].rating[_chapterSelection];
		}
		Common::String rankStr = getRankString(pilotRating);

		Common::String displayStr = Common::String::format(fmtStr,
			(short)pilotLives, (long)pilotScore, rankStr.c_str());

		drawMenuString(renderBitmap, displayStr.c_str(), 25 * scale, 190 * scale);
	} else {
		const char *lockStr = splayer->getString(81);
		if (!lockStr || !lockStr[0])
			lockStr = "^f01^c248UNREGISTERED - PASSCODE REQUIRED";

		char cursor = ((_vm->_system->getMillis() / 133) & 1) ? '_' : ' ';

		Common::String displayStr = Common::String::format("%s ^c005%s%c",
			lockStr, _passwordInput.c_str(), cursor);

		drawMenuString(renderBitmap, displayStr.c_str(), 30 * scale, 190 * scale);
	}
}

void InsaneRebel2::drawChapterSelectOverlay(byte *renderBitmap, int pitch, int width, int height) {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer)
		return;

	const char *items[18];

	items[0] = splayer->getString(60);
	if (!items[0] || !items[0][0])
		items[0] = "^f02Chapters";

	for (int i = 1; i <= 16; i++) {
		bool unlocked = _chapterUnlocked[i - 1];
		int trsIdx = unlocked ? (40 + i) : (60 + i);
		items[i] = splayer->getString(trsIdx);
		if (!items[i] || !items[i][0])
			items[i] = "";
	}

	items[17] = splayer->getString(77);
	if (!items[17] || !items[17][0])
		items[17] = "^f01^c240RETURN TO PILOTS";

	drawMenuItems(renderBitmap, pitch, width, height, items, 17, _chapterSelection, true);

	drawPreviewBox(renderBitmap, pitch, width, height);

	drawChapterInfoLine(renderBitmap, pitch, width, height);
}

int InsaneRebel2::runLevelSelect() {

	debugC(DEBUG_INSANE, "Entering pilot selection (FUN_00414A41), %d pilots loaded", _numPilots);

	_menuInputActive = true;
	while (!_menuEventQueue.empty())
		_menuEventQueue.pop();
	resetMenuGamepadAxis();

	_levelSelection = 0;
	_levelItemCount = _numPilots + 4;
	_selectedLevel = 1;
	_menuRepeatDelay = 0;
	_gameState = kStatePilotSelect;
	_pilotMenuMode = kPilotModeSelect;
	_pilotNameInput = "";
	_pilotEditIndex = -1;
	updateMenuVirtualKeyboard();

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	while (!_vm->shouldQuit()) {
		_vm->_smushVideoShouldFinish = false;

		Common::String menuVideo = getRandomMenuVideo();
		splayer->setCurVideoFlags(0x20);
		splayer->play(menuVideo.c_str(), 15);

		if (_vm->shouldQuit()) {
			setVirtualKeyboardVisible(false);
			_menuInputActive = false;
			return kLevelSelectQuit;
		}

		if (!_menuSelectionConfirmed)
			continue;

		_vm->_smushVideoShouldFinish = false;
		_menuSelectionConfirmed = false;

		if (_pilotMenuMode == kPilotModeDifficulty) {
			if (_pilotEditIndex >= 0 && _pilotEditIndex < _numPilots) {
				_pilots[_pilotEditIndex].difficulty = _difficultySelection;
				_difficulty = _difficultySelection;
				savePilots();
				_activePilot = _pilotEditIndex;

				for (int i = 0; i < 16; i++) {
					_chapterUnlocked[i] = _debugUnlockAll || (_pilots[_activePilot].damage[i] < 0xFF);
				}
			}
			_pilotMenuMode = kPilotModeSelect;
			_levelItemCount = _numPilots + 4;
			_gameState = kStatePilotSelect;
			resetMenuGamepadAxis();
			_menuInputActive = false;
			return kLevelSelectPlay;
		}

		if (_pilotMenuMode == kPilotModeNameInput) {
			if (_pilotEditIndex >= 0 && _pilotEditIndex < _numPilots) {
				Common::strlcpy(_pilots[_pilotEditIndex].name, _pilotNameInput.c_str(),
				                sizeof(_pilots[_pilotEditIndex].name));
			}
			setVirtualKeyboardVisible(false);
			_pilotMenuMode = kPilotModeDifficulty;
			_gameState = kStateDifficultySelect;
			_difficultySelection = 2;
			resetMenuGamepadAxis();
			continue;
		}

		if (_pilotMenuMode == kPilotModeCopySelect || _pilotMenuMode == kPilotModeDeleteSelect) {
			int pilotIndex = _levelSelection;
			if (pilotIndex >= 0 && pilotIndex < _numPilots) {
				if (_pilotMenuMode == kPilotModeCopySelect) {
					copyPilot(pilotIndex);
					savePilots();
					_levelSelection = pilotIndex;
					debugC(DEBUG_INSANE, "Copied pilot %d, now %d pilots", pilotIndex, _numPilots);
				} else {
					deletePilot(pilotIndex);
					savePilots();
					if (_numPilots > 0) {
						_levelSelection = (pilotIndex != 0) ? pilotIndex - 1 : 0;
					} else {
						_levelSelection = 0;
					}
					debugC(DEBUG_INSANE, "Deleted pilot %d, %d remaining", pilotIndex, _numPilots);
				}
			}
			_pilotMenuMode = kPilotModeSelect;
			_levelItemCount = _numPilots + 4;
			_gameState = kStatePilotSelect;
			resetMenuGamepadAxis();
			continue;
		}

		debugC(DEBUG_INSANE, "Pilot selection: %d (numPilots=%d)", _levelSelection, _numPilots);

		if (_levelSelection < _numPilots) {
			_activePilot = _levelSelection;
			_difficulty = _pilots[_activePilot].difficulty;

			for (int i = 0; i < 16; i++) {
				_chapterUnlocked[i] = _debugUnlockAll || (_pilots[_activePilot].damage[i] < 0xFF);
			}

			debugC(DEBUG_INSANE, "Pilot '%s' selected (slot %d, difficulty %d)",
			      _pilots[_activePilot].name, _activePilot, _difficulty);
			setVirtualKeyboardVisible(false);
			_menuInputActive = false;
			return kLevelSelectPlay;

		} else if (_levelSelection == _numPilots) {
			int newIdx = createNewPilot();
			if (newIdx >= 0) {
				_pilotEditIndex = newIdx;
				_pilotNameInput = "";
				_pilotMenuMode = kPilotModeNameInput;
				_levelItemCount = _numPilots + 4;
				resetMenuGamepadAxis();
				updateMenuVirtualKeyboard();
				debugC(DEBUG_INSANE, "NEW PILOT - entering name for slot %d", newIdx);
			}
			continue;

		} else if (_levelSelection == _numPilots + 1) {
			if (_numPilots > 0 && _numPilots < kMaxPilots) {
				_pilotMenuMode = kPilotModeCopySelect;
				_levelSelection = 0;
				_levelItemCount = _numPilots;
				resetMenuGamepadAxis();
				debugC(DEBUG_INSANE, "COPY PILOT - selecting source");
			}
			continue;

		} else if (_levelSelection == _numPilots + 2) {
			if (_numPilots > 0) {
				_pilotMenuMode = kPilotModeDeleteSelect;
				_levelSelection = 0;
				_levelItemCount = _numPilots;
				resetMenuGamepadAxis();
				debugC(DEBUG_INSANE, "DELETE PILOT - selecting target");
			}
			continue;

		} else if (_levelSelection == _numPilots + 3) {
			setVirtualKeyboardVisible(false);
			_menuInputActive = false;
			return kLevelSelectBack;
		}
	}

	setVirtualKeyboardVisible(false);
	_menuInputActive = false;
	return kLevelSelectQuit;
}

int InsaneRebel2::processLevelSelectInput() {
	int result = -1;

	if (_pilotMenuMode == kPilotModeNameInput) {
		while (!_menuEventQueue.empty()) {
			Common::Event event = _menuEventQueue.pop();
			if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
				    event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					if (_pilotNameInput.size() > 0) {
						setVirtualKeyboardVisible(false);
						_menuSelectionConfirmed = true;
						_vm->_smushVideoShouldFinish = true;
						debugC(DEBUG_INSANE, "PilotName: confirmed '%s'", _pilotNameInput.c_str());
					}
				} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					if (_pilotEditIndex >= 0 && _pilotEditIndex < _numPilots) {
						deletePilot(_pilotEditIndex);
					}
					_pilotMenuMode = kPilotModeSelect;
					_levelItemCount = _numPilots + 4;
					resetMenuGamepadAxis();
					updateMenuVirtualKeyboard();
					debugC(DEBUG_INSANE, "PilotName: cancelled");
				} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
					if (_pilotNameInput.size() > 0) {
						_pilotNameInput.deleteLastChar();
					}
				} else {
					char c = (char)event.kbd.ascii;
					if (c >= 0x20 && c <= 0x7E &&
					    (int)_pilotNameInput.size() < kMaxPilotNameLen) {
						_pilotNameInput += c;
					}
				}
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				_vm->_mouse.x = event.mouse.x;
				_vm->_mouse.y = event.mouse.y;
			} else if (event.type == Common::EVENT_QUIT ||
			           event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				if (_pilotEditIndex >= 0 && _pilotEditIndex < _numPilots) {
					deletePilot(_pilotEditIndex);
				}
				_pilotMenuMode = kPilotModeSelect;
				_levelItemCount = _numPilots + 4;
				resetMenuGamepadAxis();
				updateMenuVirtualKeyboard();
			}
		}
		return -1;
	}

	bool isDifficultyMode = (_gameState == kStateDifficultySelect);
	bool isPilotOperationMode =
		(_pilotMenuMode == kPilotModeCopySelect || _pilotMenuMode == kPilotModeDeleteSelect);
	int &selection = isDifficultyMode ? _difficultySelection : _levelSelection;
	int itemCount = isDifficultyMode ? 6 : (isPilotOperationMode ? _numPilots : _levelItemCount);
	if (itemCount <= 0)
		return -1;

	const bool highRes = isHiRes();
	const int itemBaseY = highRes ? (itemCount * -5 + 0x5a) * 2 + 0x1c : itemCount * -5 + 0x68;
	const int itemSpacing = highRes ? 20 : 10;
	const int itemHitTop = highRes ? 2 : 1;
	const int itemHitHeight = highRes ? 18 : 10;

	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.pop();
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				selection--;
				if (selection < 0) {
					selection = itemCount - 1;
				}
				break;

			case Common::KEYCODE_DOWN:
				selection++;
				if (selection >= itemCount) {
					selection = 0;
				}
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				if (selection >= 0 && selection < itemCount) {
					result = selection;
				}
				break;

			case Common::KEYCODE_ESCAPE:
				if (isDifficultyMode) {
					_gameState = kStatePilotSelect;
					resetMenuGamepadAxis();
				} else if (isPilotOperationMode) {
					bool wasCopyMode = (_pilotMenuMode == kPilotModeCopySelect);
					_pilotMenuMode = kPilotModeSelect;
					_levelItemCount = _numPilots + 4;
					_levelSelection = _numPilots + (wasCopyMode ? 1 : 2);
					resetMenuGamepadAxis();
				} else {
					result = _levelItemCount - 1;
				}
				break;

			default:
				break;
			}
			break;

		case Common::EVENT_LBUTTONDOWN:
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			for (int i = 0; i < itemCount; i++) {
				int itemY = itemBaseY + i * itemSpacing;
				if (event.mouse.y >= itemY - itemHitTop && event.mouse.y < itemY - itemHitTop + itemHitHeight) {
					selection = i;
					result = selection;
					break;
				}
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			for (int i = 0; i < itemCount; i++) {
				int itemY = itemBaseY + i * itemSpacing;
				if (event.mouse.y >= itemY - itemHitTop && event.mouse.y < itemY - itemHitTop + itemHitHeight) {
					selection = i;
					break;
				}
			}
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			if (isDifficultyMode) {
				_gameState = kStatePilotSelect;
				resetMenuGamepadAxis();
			} else if (isPilotOperationMode) {
				_pilotMenuMode = kPilotModeSelect;
				_levelItemCount = _numPilots + 4;
				_levelSelection = _levelItemCount - 1;
				resetMenuGamepadAxis();
				result = _levelSelection;
			} else {
				result = _levelItemCount - 1;
			}
			break;

		default:
			break;
		}
	}

	return result;
}

void InsaneRebel2::drawLevelSelectOverlay(byte *renderBitmap, int pitch, int width, int height) {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer) {
		debugC(DEBUG_INSANE, "drawLevelSelectOverlay: SmushPlayer not available for TRS strings!");
		return;
	}

	if (_gameState == kStateDifficultySelect) {
		const char *diffItems[7];
		for (int i = 0; i < 7; i++) {
			diffItems[i] = splayer->getString(110 + i);
			if (!diffItems[i] || !diffItems[i][0]) {
				diffItems[i] = "";
			}
		}
		drawMenuItems(renderBitmap, pitch, width, height, diffItems, 6, _difficultySelection);
		return;
	}

	if (_pilotMenuMode == kPilotModeCopySelect || _pilotMenuMode == kPilotModeDeleteSelect) {
		Common::String pilotNameStrs[kMaxPilots];
		for (int i = 0; i < _numPilots; i++) {
			pilotNameStrs[i] = Common::String::format("^f01^c005%s^f00", _pilots[i].name);
		}

		const char *pilotItems[kMaxPilots + 1];
		int idx = 0;
		pilotItems[idx++] = splayer->getString(_pilotMenuMode == kPilotModeCopySelect ? 28 : 27);

		for (int i = 0; i < _numPilots; i++) {
			pilotItems[idx++] = pilotNameStrs[i].c_str();
		}

		for (int i = 0; i < idx; i++) {
			if (!pilotItems[i] || !pilotItems[i][0]) {
				pilotItems[i] = "";
			}
		}

		drawMenuItems(renderBitmap, pitch, width, height, pilotItems, _numPilots, _levelSelection);
		return;
	}

	Common::String pilotNameStrs[kMaxPilots];
	for (int i = 0; i < _numPilots; i++) {
		if (_pilotMenuMode == kPilotModeNameInput && i == _pilotEditIndex) {
			pilotNameStrs[i] = Common::String::format("^f01^c005%s_^f00", _pilotNameInput.c_str());
		} else {
			pilotNameStrs[i] = Common::String::format("^f01^c005%s^f00", _pilots[i].name);
		}
	}

	const char *pilotItems[15];
	int idx = 0;

	pilotItems[idx++] = splayer->getString(20);

	for (int i = 0; i < _numPilots; i++) {
		pilotItems[idx++] = pilotNameStrs[i].c_str();
	}

	for (int i = 0; i < 4; i++) {
		pilotItems[idx++] = splayer->getString(21 + i);
	}

	for (int i = 0; i < idx; i++) {
		if (!pilotItems[i] || !pilotItems[i][0]) {
			pilotItems[i] = "";
		}
	}

	drawMenuItems(renderBitmap, pitch, width, height, pilotItems, _numPilots + 4, _levelSelection);

	drawPilotInfoLines(renderBitmap);
}

// Difficulty and current chapter of the highlighted pilot, at the bottom left.
void InsaneRebel2::drawPilotInfoLines(byte *renderBitmap) {
	if (_pilotMenuMode != kPilotModeSelect || _levelSelection < 0 || _levelSelection >= _numPilots)
		return;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer)
		return;

	const PilotData &pilot = _pilots[_levelSelection];
	const int scale = isHiRes() ? 2 : 1;

	const char *diffFmt = splayer->getString(25);
	if (!diffFmt || !diffFmt[0])
		diffFmt = "^f01^c248Difficulty: %s";
	const char *diffName = splayer->getString(111 + CLIP<int>(pilot.difficulty, 0, 5));
	if (diffName && strlen(diffName) > 9) {
		// Skip the ^f01^c005 prefix so the name inherits the line's color.
		Common::String line = Common::String::format(diffFmt, diffName + 9);
		drawMenuString(renderBitmap, line.c_str(), 30 * scale, 180 * scale);
	}

	int chapterIdx = 15;
	while (chapterIdx > 0 && pilot.damage[chapterIdx] >= 0xFF)
		chapterIdx--;
	const char *chapFmt = splayer->getString(26);
	if (!chapFmt || !chapFmt[0])
		chapFmt = "^f01^c248Chapter:    %hd";
	Common::String line = Common::String::format(chapFmt, (short)(chapterIdx + 1));
	drawMenuString(renderBitmap, line.c_str(), 30 * scale, 190 * scale);
}

void InsaneRebel2::initDefaultRankings() {
	_numRankings = 0;
	memset(_rankings, 0, sizeof(_rankings));
	for (int i = 0; i < kMaxRankings; i++) {
		int k = kMaxRankings - i;
		RankingEntry &r = _rankings[_numRankings];
		Common::strlcpy(r.name, "-----", sizeof(r.name));
		r.score = k * 1500;
		r.rating = k * 2;
		r.difficulty = (int16)((k * 3 + 14) / 15);
		r.chapter = (int16)((k * 15 + 14) / 15);
		_numRankings++;
	}
}

void InsaneRebel2::insertRanking(const char *name, int32 score, int32 rating,
                                  int16 difficulty, int16 chapter) {
	if (score == 0)
		return;

	int insertPos = 0;
	while (insertPos < _numRankings && score <= _rankings[insertPos].score) {
		insertPos++;
	}
	if (insertPos > kMaxRankings - 1)
		return;

	for (int i = 0; i < _numRankings; i++) {
		if (strcmp(_rankings[i].name, name) == 0) {
			if (score <= _rankings[i].score)
				return;
			for (int j = i; j < _numRankings - 1; j++)
				_rankings[j] = _rankings[j + 1];
			_numRankings--;
			if (insertPos > i)
				insertPos--;
			break;
		}
	}

	int lastIdx = MIN(_numRankings, kMaxRankings - 1);
	for (int i = lastIdx; i > insertPos; i--)
		_rankings[i] = _rankings[i - 1];

	RankingEntry &r = _rankings[insertPos];
	Common::strlcpy(r.name, name, sizeof(r.name));
	r.score = score;
	r.rating = rating;
	r.difficulty = difficulty;
	r.chapter = chapter;

	if (_numRankings < kMaxRankings)
		_numRankings++;
}

void InsaneRebel2::showTopPilots() {
	debugC(DEBUG_INSANE, "Showing Top Pilots screen (FUN_00420116)");

	_menuInputActive = true;
	while (!_menuEventQueue.empty())
		_menuEventQueue.pop();
	resetMenuGamepadAxis();

	_topPilotsMaxFrames = 120;
	_topPilotsFrameCount = 0;

	_gameState = kStateTopPilots;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	_vm->_smushVideoShouldFinish = false;

	Common::String menuVideo = getRandomMenuVideo();
	splayer->setCurVideoFlags(0x20);
	splayer->play(menuVideo.c_str(), 15);

	_gameState = kStateMainMenu;
	_menuInputActive = true;
	resetMenuGamepadAxis();

	debugC(DEBUG_INSANE, "Top Pilots screen finished");
}

void InsaneRebel2::drawTopPilotsOverlay(byte *renderBitmap, int pitch, int width, int height) {
	if (_topPilotsFrameCount >= _topPilotsMaxFrames) {
		_vm->_smushVideoShouldFinish = true;
		return;
	}

	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.pop();
		if (event.type == Common::EVENT_KEYDOWN ||
		    event.type == Common::EVENT_LBUTTONDOWN) {
			_vm->_smushVideoShouldFinish = true;
			return;
		}
	}

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer)
		return;

	const int scale = isHiRes() ? 2 : 1;

	drawMenuStringCentered(renderBitmap, "^f02Top Pilots", 152 * scale, 10 * scale);

	int headerY = 30 * scale;
	int headerColor = 5;
	drawMenuStringCentered(renderBitmap, "^f01Rank", 43 * scale, headerY, headerColor);
	drawMenuString(renderBitmap, "^f01Name", 88 * scale, headerY, headerColor);
	drawMenuStringCentered(renderBitmap, "^f01Difficulty", 195 * scale, headerY, headerColor);
	drawMenuStringCentered(renderBitmap, "^f01Chapter", 245 * scale, headerY, headerColor);
	drawMenuStringRight(renderBitmap, "^f01Score", 295 * scale, headerY, headerColor);

	int showCount = MIN(_topPilotsFrameCount, _numRankings);

	for (int row = 0; row < showCount; row++) {
		const RankingEntry &r = _rankings[row];
		int rowY = (row * 10 + 42) * scale;
		int color = 244;  // 0xF4

		Common::String rankStr = getRankString(r.rating);
		if (!rankStr.empty()) {
			Common::String rankFmt = Common::String::format("^f00%s", rankStr.c_str());
			drawMenuStringCentered(renderBitmap, rankFmt.c_str(), 43 * scale, rowY, color);
		}

		Common::String nameFmt = Common::String::format("^f01%s", r.name);
		drawMenuString(renderBitmap, nameFmt.c_str(), 88 * scale, rowY, color);

		int trsIdx = CLIP((int)r.difficulty, 0, 5) + 155;
		const char *diffStr = splayer->getString(trsIdx);
		if (diffStr && diffStr[0]) {
			Common::String diffFmt = Common::String::format("^f01%s", diffStr);
			drawMenuStringCentered(renderBitmap, diffFmt.c_str(), 195 * scale, rowY, color);
		}

		Common::String chFmt = Common::String::format("^f01%d", (int)r.chapter);
		drawMenuStringCentered(renderBitmap, chFmt.c_str(), 245 * scale, rowY, color);

		Common::String scoreFmt = Common::String::format("^f01%ld", (long)r.score);
		drawMenuStringRight(renderBitmap, scoreFmt.c_str(), 295 * scale, rowY, color);
	}

	_topPilotsFrameCount++;
}

void InsaneRebel2::showOptionsMenu() {
	debugC(DEBUG_INSANE, "Showing Options menu (FUN_00416787)");

	_menuInputActive = true;
	while (!_menuEventQueue.empty())
		_menuEventQueue.pop();
	resetMenuGamepadAxis();

	_optionsSelection = 0;
	_optionsItemCount = 8;
	_optionsExitRequested = false;

	_gameState = kStateOptions;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	while (!_vm->shouldQuit() && !_optionsExitRequested) {
		_vm->_smushVideoShouldFinish = false;

		Common::String menuVideo = getRandomMenuVideo();
		splayer->setCurVideoFlags(0x20);
		splayer->play(menuVideo.c_str(), 15);
	}

	_gameState = kStateMainMenu;
	_menuInputActive = true;
	resetMenuGamepadAxis();

	debugC(DEBUG_INSANE, "Options menu finished");
}

int InsaneRebel2::processOptionsInput() {
	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.pop();

		if (event.type == Common::EVENT_KEYDOWN) {
			_menuInactivityTimer = 0;

			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				_optionsSelection--;
				if (_optionsSelection < 0)
					_optionsSelection = _optionsItemCount - 1;
				return -1;

			case Common::KEYCODE_DOWN:
				_optionsSelection++;
				if (_optionsSelection >= _optionsItemCount)
					_optionsSelection = 0;
				return -1;

			case Common::KEYCODE_LEFT:
				if (_optionsSelection == 6) {
					_optVolumeLevel = MAX(0, _optVolumeLevel - 4);
					setRebel2MixerVolume(_vm, _optVolumeLevel);
				}
				return -1;

			case Common::KEYCODE_RIGHT:
				if (_optionsSelection == 6) {
					_optVolumeLevel = MIN(127, _optVolumeLevel + 4);
					setRebel2MixerVolume(_vm, _optVolumeLevel);
				}
				return -1;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				switch (_optionsSelection) {
				case 0:
					_optMusicEnabled = !_optMusicEnabled;
					_vm->_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, !_optMusicEnabled);
					break;
				case 1:
					_optSfxEnabled = !_optSfxEnabled;
					_vm->_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, !_optSfxEnabled);
					break;
				case 2:
					_optVoicesEnabled = !_optVoicesEnabled;
					_vm->_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, !_optVoicesEnabled);
					break;
				case 3:
					_optTextEnabled = !ConfMan.getBool("subtitles");
					ConfMan.setBool("subtitles", _optTextEnabled);
					break;
				case 4:
					_optControlsFlipped = !_optControlsFlipped;
					break;
				case 5:
					_optRapidFire = !_optRapidFire;
					ConfMan.setBool("rebel2_rapid_fire", _optRapidFire);
					ConfMan.flushToDisk();
					break;
				case 6:
					break;
				case 7:
					_optionsExitRequested = true;
					_vm->_smushVideoShouldFinish = true;
					return 7;
				}
				return _optionsSelection;

			case Common::KEYCODE_ESCAPE:
				_optionsExitRequested = true;
				_vm->_smushVideoShouldFinish = true;
				return -2;

			default:
				break;
			}
		}

		if (event.type == Common::EVENT_LBUTTONDOWN) {
			int mouseY = event.mouse.y;
			const bool highRes = isHiRes();
			const int baseY = highRes ? (_optionsItemCount * -5 + 0x5a) * 2 + 0x1c : _optionsItemCount * -5 + 0x68;
			const int itemSpacing = highRes ? 20 : 10;
			const int itemHitTop = highRes ? 2 : 1;
			const int itemHitHeight = highRes ? 18 : 10;
			for (int i = 0; i < _optionsItemCount; i++) {
				int itemY = baseY + i * itemSpacing;
				if (mouseY >= itemY - itemHitTop && mouseY < itemY - itemHitTop + itemHitHeight) {
					_optionsSelection = i;
					Common::Event enterEvent;
					enterEvent.type = Common::EVENT_KEYDOWN;
					enterEvent.kbd.keycode = Common::KEYCODE_RETURN;
					_menuEventQueue.push(enterEvent);
					return -1;
				}
			}
		}
	}
	return -1;
}

void InsaneRebel2::drawOptionsOverlay(byte *renderBitmap, int pitch, int width, int height) {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer)
		return;

	_optTextEnabled = ConfMan.getBool("subtitles");

	const char *items[10];

	items[0] = splayer->getString(89);
	if (!items[0] || !items[0][0])
		items[0] = "^f02Game Options";

	items[1] = splayer->getString(_optMusicEnabled ? 90 : 91);
	if (!items[1] || !items[1][0])
		items[1] = _optMusicEnabled ? "^f01^c005Music is On" : "^f01^c005Music is Off";

	items[2] = splayer->getString(_optSfxEnabled ? 92 : 93);
	if (!items[2] || !items[2][0])
		items[2] = _optSfxEnabled ? "^f01^c005SFX are On" : "^f01^c005SFX are Off";

	items[3] = splayer->getString(_optVoicesEnabled ? 94 : 95);
	if (!items[3] || !items[3][0])
		items[3] = _optVoicesEnabled ? "^f01^c005Voices are On" : "^f01^c005Voices are Off";

	items[4] = splayer->getString(_optTextEnabled ? 96 : 97);
	if (!items[4] || !items[4][0])
		items[4] = _optTextEnabled ? "^f01^c005Text is On" : "^f01^c005Text is Off";

	items[5] = splayer->getString(_optControlsFlipped ? 99 : 98);
	if (!items[5] || !items[5][0])
		items[5] = _optControlsFlipped ? "^f01^c005Controls are Flipped" : "^f01^c005Controls are Normal";

	items[6] = splayer->getString(_optRapidFire ? 100 : 101);
	if (!items[6] || !items[6][0])
		items[6] = _optRapidFire ? "^f01^c005Rapid Fire On" : "^f01^c005Rapid Fire Off";

	char volumeBuf[64];
	const char *volFmt = splayer->getString(103);
	if (volFmt && volFmt[0])
		Common::sprintf_s(volumeBuf, volFmt, (short)(_optVolumeLevel * 100 / 127));
	else
		Common::sprintf_s(volumeBuf, "^f01^c005Volume Level: %hd%%", (short)(_optVolumeLevel * 100 / 127));
	items[7] = volumeBuf;

	items[8] = splayer->getString(107);
	if (!items[8] || !items[8][0])
		items[8] = "^f01^c240Return to Main Menu";

	drawMenuItems(renderBitmap, pitch, width, height, items, _optionsItemCount, _optionsSelection);
}

} // End of namespace Scumm
