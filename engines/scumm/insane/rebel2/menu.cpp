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

// ---------------------------------------------------------------------------
// Menu System Implementation
// ---------------------------------------------------------------------------
// Emulates original menu system from FUN_004147B2 and FUN_0041FDC8.

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

// unlockAllChapters -- Debug mode unlock (FUN_00415CF8 lines 60-71, DAT_0047ab34=='d').
void InsaneRebel2::unlockAllChapters() {
	debugC(DEBUG_INSANE, "Unlocking all chapters for testing");
	for (int i = 0; i < 16; i++) {
		_chapterUnlocked[i] = true;
		_levelUnlocked[i] = true;
	}
}

// getRandomMenuVideo -- Select random menu video variant (FUN_0041FDC8).
// Original plays O_MENU.SAN when no progress flags are set, but that file
// contains ONLY audio (no FOBJ frames) resulting in a black background.
// We always use O_MENU_X.SAN (A-O) which have 320x200 background images.
Common::String InsaneRebel2::getRandomMenuVideo() {

	// Select random variant (0-14 maps to A-O), ensuring different from last
	int variant;
	do {
		variant = _vm->_rnd.getRandomNumber(14);  // 0-14
	} while (variant == _lastMenuVariant && _lastMenuVariant >= 0);
	_lastMenuVariant = variant;

	// Map 0-14 to A-O (case 0/default = A, 1 = B, etc.)
	char letter = 'A' + variant;
	debugC(DEBUG_INSANE, "Selected menu variant %c", letter);
	return Common::String::format("OPEN/O_MENU_%c.SAN", letter);
}

//
// processMenuInput -- Menu input handling (FUN_0041f5ae)
//
// Returns -1 (no action) or a 0-based selected menu item.
// Events captured by notifyEvent() before ScummEngine consumes them.
// Keyboard: Up=0x148, Down=0x150, Enter=0x0d.
// Physical ESC is handled by notifyEvent() and opens the global menu.
// Mouse mode (DAT_0047a806 == 1): Y position maps to selection.
//
int InsaneRebel2::processMenuInput() {

	int result = -1;

	const bool highRes = isHiRes();
	const int baseY = highRes ? (_menuItemCount * -5 + 0x5a) * 2 + 0x1c : _menuItemCount * -5 + 0x68;
	const int itemSpacing = highRes ? 20 : 10;
	const int itemHitTop = highRes ? 2 : 1;
	const int itemHitHeight = highRes ? 18 : 10;

	// Process events from the queue (populated by notifyEvent)
	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.pop();
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_menuInactivityTimer = 0;  // Reset inactivity timer on any input

			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				// Navigate up (wrap around) - emulates key code 0x148
				_menuSelection--;
				if (_menuSelection < 0) {
					_menuSelection = _menuItemCount - 1;
				}
				// Reset repeat delay counter (DAT_00459ce0)
				_menuRepeatDelay = 3;
				debugC(DEBUG_INSANE, "Menu: Selection changed to %d (UP)", _menuSelection);
				break;

			case Common::KEYCODE_DOWN:
				// Navigate down (wrap around) - emulates key code 0x150
				_menuSelection++;
				if (_menuSelection >= _menuItemCount) {
					_menuSelection = 0;
				}
				_menuRepeatDelay = 3;
				debugC(DEBUG_INSANE, "Menu: Selection changed to %d (DOWN)", _menuSelection);
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				// Confirm selection - emulates key code 0x0d
				if (_menuSelection >= 0 && _menuSelection < _menuItemCount) {
					result = _menuSelection;
					debugC(DEBUG_INSANE, "Menu: Item %d selected (ENTER)", _menuSelection);
				}
				break;

			case Common::KEYCODE_ESCAPE:
				// Synthetic custom back action - quit/back (last item)
				result = _menuItemCount - 1;  // Select quit option
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
					if (mouseY >= itemY - itemHitTop - 3 * (highRes ? 2 : 1) &&
							mouseY < itemY - itemHitTop + itemHitHeight - 3 * (highRes ? 2 : 1)) {
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
			// Handle quit request - select quit option
			result = _menuItemCount - 1;
			break;

		default:
			break;
		}
	}

	// Decrement repeat delay counter (for smooth keyboard navigation)
	if (_menuRepeatDelay > 0) {
		_menuRepeatDelay--;
	}

	return result;
}

//
// drawMenuItems -- Shared menu item renderer (FUN_0041f5ae)
//
void InsaneRebel2::drawMenuItems(byte *renderBitmap, int pitch, int width, int height,
                                  const char **items, int numItems, int selection,
                                  bool leftAligned) {
	//
	// items[0] = title string, items[1..numItems] = selectable items
	// numItems = number of selectable items (FUN_0041f5ae param_3)
	// selection = currently highlighted item (0-based, maps to DAT_00459988)
	// leftAligned = false: param_4==0 (centered), true: param_4==1 (left-aligned)
	//
	// Coordinate formulas from FUN_0041f5ae (low-res, DAT_0047a808 < 2):
	// Centered (param_4=0):
	//   Title X:     center - titleWidth/2  (centerX = 160)
	//   Title Y:     param_3 * -5 + 0x51
	//   Item X:      center - textWidth/2
	//   Box X:       center - bracketWidth/2
	// Left-aligned (param_4=1):
	//   Title X:     0x28 = 40
	//   Title Y:     param_3 * -5 + 0x56
	//   Item X:      0x17 = 23
	//   Box X:       0x14 = 20
	// Both modes:
	//   Item base Y: param_3 * -5 + 0x68
	//   Item Y:      param_3 * -5 + i * 10 + 0x68
	//   Box Y:       param_3 * -5 + i * 10 + 0x67  (1px above text)

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

	// -------------------------------------------------------------------
	// Draw title - items[0]
	// Centered: X = center - titleWidth/2
	// Left-aligned: X = 40 (0x28)
	// -------------------------------------------------------------------
	{
		int titleWidth = getStringWidth(items[0]);
		int titleX = leftAligned ? (highRes ? 0x50 : 0x28) : (centerX - titleWidth / 2);
		drawString(items[0], titleX, titleY);
	}

	// -------------------------------------------------------------------
	// Draw selectable items with selection highlight box
	// Centered: item X = center - textWidth/2, box X = center - bracketWidth/2
	// Left-aligned: item X = 23 (0x17), box X = 20 (0x14)
	// -------------------------------------------------------------------
	for (int i = 0; i < numItems; i++) {
		int itemY = itemBaseY + i * itemSpacing;
		const char *text = items[i + 1];

		int textWidth = getStringWidth(text);
		int textX = leftAligned ? (highRes ? 0x2e : 0x17) : (centerX - textWidth / 2);
		drawString(text, textX, itemY);

		// Selection highlight box - FUN_004292d0
		if (i == selection) {
			// Width: textWidth + ((DAT_0047a808 < 2) - 1 & 6) + 6 = textWidth + 6
			int bracketWidth = textWidth + (highRes ? 12 : 6);
			// Height: ((DAT_0047a808 < 2) - 1 & 10) + 10 = 10
			int bracketHeight = highRes ? 20 : 10;

			// Flash color: (-((DAT_0047a7e4 & 1) == 0) & 8U) - 0x10
			// bit0==0: 8-16=248(0xF8), bit0==1: 0-16=240(0xF0)
			byte highlightColor = ((_vm->_system->getMillis() / 133) & 1) ? 248 : 240;

			// Box position: Y = itemY - 1 (0x67 vs 0x68)
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

			// FUN_004292d0 - Draw rectangle border (4 lines)
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

// parseFormatCode -- Shared ^fNN/^cNNN/^^/^l parser.
// Returns: fontIdx (>=0) on font change, -2 on color/newline, -1 on no match.
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

// getMenuStringWidth -- Format-code-aware string width.
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

// Format-code-aware string rendering at (x, y).
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
	// Main menu renderer - calls shared drawMenuItems()
	// Emulates FUN_004147b2 -> FUN_0041f5ae with param_3=7, param_4=0
	// -------------------------------------------------------------------
	//
	// Menu strings loaded from GAME.TRS (keyboard mode indices 10-17):
	//   TRS index 10: "^f02Game Main Menu"           -> Title (uses TITLFONT)
	//   TRS index 11: "^f01^c005Start Game"          -> Item 0 (uses SMALFONT, color 5)
	//   TRS index 12: "^f01^c009Options"             -> Item 1
	//   TRS index 13: "^f01^c009Calibrate Joystick"  -> Item 2
	//   TRS index 14: "^f01^c009Continue Intro"      -> Item 3
	//   TRS index 15: "^f01^c009Show Top Pilots"     -> Item 4
	//   TRS index 16: "^f01^c009Show Credits"        -> Item 5
	//   TRS index 17: "^f01^c240Return to Launcher"  -> Item 6 (color 240)

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer) {
		debugC(DEBUG_INSANE, "drawMenuOverlay: SmushPlayer not available for TRS strings!");
		return;
	}

	// Load TRS strings 10-17 (title + 7 selectable items)
	const char *menuItems[8];
	for (int i = 0; i < 8; i++) {
		menuItems[i] = splayer->getString(10 + i);
		if (!menuItems[i] || !menuItems[i][0]) {
			debugC(DEBUG_INSANE, "drawMenuOverlay: TRS string %d not found!", 10 + i);
			menuItems[i] = "";
		}
	}

	// FUN_004147b2 line 25: param_3 = (DAT_0047a806 == 0) + 6 = 7 (keyboard mode)
	drawMenuItems(renderBitmap, pitch, width, height, menuItems, 7, _menuSelection);
}

// ---------------------------------------------------------------------------
// Pause Overlay
// ---------------------------------------------------------------------------

// pauseFillRect -- Helper to fill a rectangle in the frame buffer with bounds checking.
void pauseFillRect(byte *buf, int bufW, int bufH, int x, int y, int w, int h, byte color) {
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > bufW) w = bufW - x;
	if (y + h > bufH) h = bufH - y;
	if (w <= 0 || h <= 0) return;
	for (int row = y; row < y + h; row++)
		memset(buf + row * bufW + x, color, w);
}

// showPauseOverlay -- Dimmed overlay with metallic frame and "PAUSED" text.
// Reproduces FUN_405A21 from the original executable.
//
// The original dims pixels using a green-channel formula that produces palette
// indices 16-79, relying on a built-in grayscale ramp at those positions.
// We instead dim the system palette (25% brightness) which achieves the same
// visual effect regardless of palette layout. The pixel buffer is only modified
// for the frame decorations and text — the game frame pixels stay unchanged.
// The original palette is saved in _savedPausePalette and restored on unpause.
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

	// Step 1: Save current palette and create a dimmed version.
	memcpy(_savedPausePalette, palette, 768);
	_pauseOverlayActive = true;

	byte dimPal[768];
	memcpy(dimPal, palette, 768);
	for (int i = 0; i < 768; i++)
		dimPal[i] >>= 2;  // 25% brightness

	// Override specific palette entries for UI elements.
	// Frame bars (0x50): medium gray
	dimPal[0x50 * 3 + 0] = 80; dimPal[0x50 * 3 + 1] = 80; dimPal[0x50 * 3 + 2] = 80;
	// Rivet outline (0x51): slightly lighter gray
	dimPal[0x51 * 3 + 0] = 110; dimPal[0x51 * 3 + 1] = 110; dimPal[0x51 * 3 + 2] = 110;
	// RA2 NUT fonts use hardcoded palette indices 1-4 for rendering:
	//   1=body color (remapped to col param), 2=highlight, 3=anti-alias, 4=outline
	// Index 5 is used by TRS ^c005 escape code as the text body color.
	dimPal[1 * 3 + 0] = 255; dimPal[1 * 3 + 1] = 255; dimPal[1 * 3 + 2] = 255;
	dimPal[2 * 3 + 0] = 188; dimPal[2 * 3 + 1] = 188; dimPal[2 * 3 + 2] = 188;
	dimPal[3 * 3 + 0] = 128; dimPal[3 * 3 + 1] = 128; dimPal[3 * 3 + 2] = 128;
	dimPal[4 * 3 + 0] = 0;   dimPal[4 * 3 + 1] = 0;   dimPal[4 * 3 + 2] = 0;
	dimPal[5 * 3 + 0] = 252; dimPal[5 * 3 + 1] = 252; dimPal[5 * 3 + 2] = 252;

	_vm->_system->getPaletteManager()->setPalette(dimPal, 0, 256);

	// Step 2: Draw the metallic frame (FUN_405A21 lines 261-283).
	// Horizontal border lines: 2px thick at y=23 and y=175
	pauseFillRect(frameBuffer, width, height, 0, 0x17, 0x140, 2, 0x50);
	pauseFillRect(frameBuffer, width, height, 0, 0xAF, 0x140, 2, 0x50);

	// Thick side bars: 40px wide on left and right
	pauseFillRect(frameBuffer, width, height, 0,     0, 0x28, 200, 0x50);
	pauseFillRect(frameBuffer, width, height, 0x118, 0, 0x28, 200, 0x50);

	// Rivet decorations along left side bar.
	// Layered rectangles: outer ring (0x51), inner fill (4).
	for (int i = 0; i < 6; i++) {
		int yOff = i * 0x24;  // i * 36
		pauseFillRect(frameBuffer, width, height, 0x0C, yOff,     0x19, 0x11, 0x51);
		pauseFillRect(frameBuffer, width, height, 0x0B, yOff + 1, 0x1B, 0x0F, 0x51);
		pauseFillRect(frameBuffer, width, height, 0x0D, yOff,     0x17, 0x11, 4);
		pauseFillRect(frameBuffer, width, height, 0x0B, yOff + 2, 0x1B, 0x0D, 4);
		pauseFillRect(frameBuffer, width, height, 0x0C, yOff + 1, 0x19, 0x0F, 4);
	}

	// Right side bar rivets: same pattern at x=282 (0x11A).
	for (int i = 0; i < 6; i++) {
		int yOff = i * 0x24;
		int xBase = 0x11A;
		pauseFillRect(frameBuffer, width, height, xBase,     yOff,     0x19, 0x11, 0x51);
		pauseFillRect(frameBuffer, width, height, xBase - 1, yOff + 1, 0x1B, 0x0F, 0x51);
		pauseFillRect(frameBuffer, width, height, xBase + 1, yOff,     0x17, 0x11, 4);
		pauseFillRect(frameBuffer, width, height, xBase - 1, yOff + 2, 0x1B, 0x0D, 4);
		pauseFillRect(frameBuffer, width, height, xBase,     yOff + 1, 0x19, 0x0F, 4);
	}

	// Step 3: Draw "Game Paused" text using TRS string 0x78 (120).
	// Original: FUN_00434cb0(-1, buf, NULL, x=10, y=10, align=1, fg=4, bg=0x10, text)
	// fg=4 is the foreground color (used by ^cNNN in the TRS string itself).
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
		// Original uses x=10, y=10 (single-buffer mode) or y=20 (double-buffer mode).
		// We use single-buffer mode (y=10).
		multiFont->drawString(pauseText, frameBuffer, clipRect, 10, 10, width, 4, kStyleAlignLeft);
	}

	// Step 4: Push to screen.
	_vm->_system->copyRectToScreen(frameBuffer, width, 0, 0, screenW, screenH);
	_vm->_system->updateScreen();
}

// runMainMenu -- Main menu loop (FUN_004147B2).
// Returns kMenuNewGame, kMenuResumeDemo, or kMenuQuit.
int InsaneRebel2::runMainMenu() {

	debugC(DEBUG_INSANE, "Entering main menu");

	resetMenu();
	_gameState = kStateMainMenu;

	// Enable menu input capture via EventObserver
	_menuInputActive = true;
	// Clear any stale events
	while (!_menuEventQueue.empty())
		_menuEventQueue.pop();

	// Get the SmushPlayer from ScummEngine_v7
	// Note: _player isn't set until SmushPlayer::initAudio() is called during playback
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Main menu loop
	while (!_vm->shouldQuit()) {
		// Reset video finish flag before playing menu
		_vm->_smushVideoShouldFinish = false;

		// Select and play a random menu video
		Common::String menuVideo = getRandomMenuVideo();
		debugC(DEBUG_INSANE, "Playing menu video: %s", menuVideo.c_str());

		// Set video flags for menu (0x20 = intro/menu flag)
		// This tells procPostRendering we're in menu mode
		splayer->setCurVideoFlags(0x20);

		// Play the menu video
		// Input is processed in procPostRendering during playback
		// When user confirms selection, _vm->_smushVideoShouldFinish is set
		splayer->play(menuVideo.c_str(), 15);

		// Check for quit
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

		// Only process selection if user explicitly confirmed (ENTER/ESC),
		// not when video ended naturally (EOF sets _smushVideoShouldFinish too)
		if (!_menuSelectionConfirmed) {
			continue;
		}

		// Clear the flags
		_vm->_smushVideoShouldFinish = false;
		_menuSelectionConfirmed = false;

		// A selection was made - process it
		debugC(DEBUG_INSANE, "Menu video ended with selection=%d", _menuSelection);

		// Process the menu result based on current selection
		// Menu items matching GAME.TRS indices 11-17 (FUN_004147B2):
		//   case 0 (TRS 11): Start Game -> pilot selection, returns 2
		//   case 1 (TRS 12): Options -> FUN_00416787 options screen
		//   case 2 (TRS 13): Calibrate Joystick -> FUN_00425820
		//   case 3 (TRS 14): Continue Intro -> return to intro/demo loop
		//   case 4 (TRS 15): Show Top Pilots -> FUN_00420116(-1)
		//   case 5 (TRS 16): Show Credits -> play O_CREDIT.SAN, returns 1
		//   case 6 (TRS 17): Return to Launcher -> quit, returns 0
		switch (_menuSelection) {
		case 0:  // Start Game -> go to pilot selection
			debugC(DEBUG_INSANE, "Start Game selected - going to pilot selection");
			_gameState = kStatePilotSelect;
			_menuInputActive = false;
			return kMenuNewGame;  // Return 2 (kMenuNewGame)

		case 1:  // Options -> show options menu (FUN_00416787)
			debugC(DEBUG_INSANE, "Options selected");
			showOptionsMenu();
			break;

		case 2:  // Calibrate Joystick
			debugC(DEBUG_INSANE, "Calibrate Joystick selected - no-op for modern joystick support");
			// Modern controller support uses live keymapper actions; no explicit
			// joystick calibration flow is required here.
			break;

		case 3:  // Continue Intro -> return to intro/demo loop
			debugC(DEBUG_INSANE, "Continue Intro selected - resuming intro/demo loop");
			_menuInputActive = false;
			return kMenuResumeDemo;

		case 4:  // Show Top Pilots -> high score display (FUN_00420116(-1))
			debugC(DEBUG_INSANE, "Show Top Pilots selected");
			showTopPilots();
			break;

		case 5:  // Show Credits -> play credits video
			debugC(DEBUG_INSANE, "Show Credits selected - playing O_CREDIT.SAN");
			_gameState = kStateCredits;
			_menuInputActive = false;
			splayer->setCurVideoFlags(0x20);
			splayer->play("OPEN/O_CREDIT.SAN", 15);
			_gameState = kStateMainMenu;
			_menuInputActive = true;
			// Returns 1 in original -> stays at stage 1 (main menu)
			break;

		case 6:  // Return to Launcher -> quit game
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

// ---------------------------------------------------------------------------
// Chapter Selection Screen
// ---------------------------------------------------------------------------

// runChapterSelect -- Chapter selection with preview (FUN_00415CF8).
// Returns kChapterSelectPlay, kChapterSelectBack, or kChapterSelectQuit.
int InsaneRebel2::runChapterSelect() {

	debugC(DEBUG_INSANE, "Entering chapter selection (FUN_00415CF8)");

	// Enable menu input capture
	_menuInputActive = true;
	while (!_menuEventQueue.empty())
		_menuEventQueue.pop();

	// Initialize chapter selection state
	// Original (lines 51-54): local_10 = 0xf; while (local_10 > 0 && locked) local_10--;
	// Finds highest unlocked chapter. With debug unlock all = 15 (FINALE).
	_chapterSelection = 15;
	while (_chapterSelection > 0 && !_chapterUnlocked[_chapterSelection]) {
		_chapterSelection--;
	}
	_chapterItemCount = 17;  // 16 chapters + RETURN TO PILOTS
	_selectedChapter = 0;
	_passwordInput = "";
	_menuRepeatDelay = 0;
	_gameState = kStateChapterSelect;
	updateMenuVirtualKeyboard();

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Initialize preview offset for initial selection
	_previewOffsetX = -90;
	_previewOffsetY = _chapterSelection * -50 + 75;

	// Set iactBits for chapter unlock state (FUN_00415CF8 lines 79-86)
	// Bits 16..1 correspond to chapters 0..15: set if unlocked, clear if locked.
	// These control SKIP chunks in O_LEVEL.SAN for locked/unlocked preview variants.
	for (int i = 0; i < 16; i++) {
		if (_chapterUnlocked[i])
			setBit(16 - i);
		else
			clearBit(16 - i);
	}

	// Chapter selection background - FUN_00415CF8 line 57:
	// FUN_0041f4d0(s_OPEN_O_LEVEL_SAN, 8, 0xffff, 0xffff, 0)
	// O_LEVEL.SAN contains chapter preview thumbnails at specific FOBJ positions.
	// The FOBJ offset system scrolls the correct preview into the preview box area.
	while (!_vm->shouldQuit()) {
		_vm->_smushVideoShouldFinish = false;

		debugC(DEBUG_INSANE, "Playing chapter select background: OPEN/O_LEVEL.SAN");

		// Flags: 0x20 (overlay/menu rendering) | 0x08 (preserve buffer, suppress
		// AHDR speed override). Matches original FUN_0041f4d0 parameter 8.
		// O_LEVEL.SAN AHDR specifies 15fps; flag 0x08 suppresses this override
		// so we use our intended 12fps. The preview screen is cleared each frame
		// by procPreRendering's memset, so buffer preservation is harmless.
		splayer->setCurVideoFlags(0x28);

		// Play O_LEVEL.SAN — preview thumbnails are rendered by FOBJ offset
		splayer->play("OPEN/O_LEVEL.SAN", 15);

		if (_vm->shouldQuit()) {
			setVirtualKeyboardVisible(false);
			_menuInputActive = false;
			return kChapterSelectQuit;
		}

		// Only process selection if user explicitly confirmed
		if (!_menuSelectionConfirmed) {
			continue;
		}

		_vm->_smushVideoShouldFinish = false;
		_menuSelectionConfirmed = false;

		debugC(DEBUG_INSANE, "Chapter selection made: %d", _chapterSelection);

		// Process chapter selection (lines 134-236 of FUN_00415CF8)
		if (_chapterSelection == 16) {
			// BACK selected (index 16 = 17th item)
			debugC(DEBUG_INSANE, "BACK to main menu selected");
			setVirtualKeyboardVisible(false);
			_menuInputActive = false;
			return kChapterSelectBack;
		}

		if (_chapterSelection >= 0 && _chapterSelection < 16) {
			if (_chapterUnlocked[_chapterSelection]) {
				// Unlocked chapter — play it
				_selectedChapter = _chapterSelection;
				debugC(DEBUG_INSANE, "Chapter %d selected (unlocked)", _selectedChapter + 1);
				setVirtualKeyboardVisible(false);
				_menuInputActive = false;
				return kChapterSelectPlay;
			}

			// Locked chapter — validate password (FUN_00415CF8 lines 239-257)
			if (_activePilot >= 0 && _activePilot < _numPilots &&
			    _pilots[_activePilot].difficulty < 6 && _chapterSelection > 0) {
				Common::String expected = getChapterPassword(
					_chapterSelection, _pilots[_activePilot].difficulty);
				if (!expected.empty() && _passwordInput.equalsIgnoreCase(expected)) {
					// Password accepted — unlock chapter (FUN_00415CF8 lines 253-257)
					PilotData &pilot = _pilots[_activePilot];
					pilot.score[_chapterSelection] = 0;
					pilot.damage[_chapterSelection] = 0;
					pilot.lives[_chapterSelection] = 3;
					pilot.rating[_chapterSelection] = 0;
					savePilots();
					_chapterUnlocked[_chapterSelection] = true;
					// Update iactBit for video preview (original jumps to LAB_00415d88)
					setBit(16 - _chapterSelection);
					_passwordInput.clear();
					updateMenuVirtualKeyboard();
					debugC(DEBUG_INSANE, "Chapter %d unlocked via password", _chapterSelection + 1);
					continue;  // Re-render with updated unlock state
				}
			}
			// Wrong password or no password entered
			_passwordInput.clear();
			debugC(DEBUG_INSANE, "Password rejected for chapter %d", _chapterSelection + 1);
		}
	}

	setVirtualKeyboardVisible(false);
	_menuInputActive = false;
	return kChapterSelectQuit;
}

int InsaneRebel2::processChapterSelectInput() {
	// Process input for chapter selection screen
	// Emulates input handling in FUN_00415CF8 (lines 95-133)
	// Returns: -1 = no action, 0+ = item selected

	int result = -1;

	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.front();
		_menuEventQueue.pop();

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				// Move selection up, wrap to bottom
				_chapterSelection--;
				if (_chapterSelection < 0) {
					_chapterSelection = _chapterItemCount - 1;
				}
				_passwordInput.clear();
				// Update preview offset (FUN_00425170: Y = selected * -50 + 75)
				_previewOffsetY = _chapterSelection * -50 + 75;
				updateMenuVirtualKeyboard();
				debugC(DEBUG_INSANE, "ChapterSelect: Selection changed to %d (UP) offsetY=%d", _chapterSelection, _previewOffsetY);
				break;

			case Common::KEYCODE_DOWN:
				// Move selection down, wrap to top
				_chapterSelection++;
				if (_chapterSelection >= _chapterItemCount) {
					_chapterSelection = 0;
				}
				_passwordInput.clear();
				// Update preview offset (FUN_00425170: Y = selected * -50 + 75)
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
				// Synthetic custom back action (same as selecting BACK)
				setVirtualKeyboardVisible(false);
				result = 16;  // BACK index
				debugC(DEBUG_INSANE, "ChapterSelect: Back action - back to menu");
				break;

			case Common::KEYCODE_BACKSPACE:
				// Backspace for password input (line 107-112 of FUN_00415CF8)
				if (!_passwordInput.empty()) {
					_passwordInput.deleteLastChar();
					debugC(DEBUG_INSANE, "ChapterSelect: Password backspace, now: %s", _passwordInput.c_str());
				}
				break;

			default:
				// Printable character for password input (lines 114-121 of FUN_00415CF8)
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
				// Mouse hover changes highlight (original FUN_0041f5ae mouse mode).
				// Item Y = numItems * -5 + i * 10 + 0x68
				const bool highRes = isHiRes();
				const int baseY = highRes ? (_chapterItemCount * -5 + 0x5a) * 2 + 0x1c : _chapterItemCount * -5 + 0x68;
				const int itemSpacing = highRes ? 20 : 10;
				const int itemHitTop = highRes ? 2 : 1;
				const int itemHitHeight = highRes ? 18 : 10;
				int mouseY = event.mouse.y;

				for (int i = 0; i < _chapterItemCount; i++) {
					int itemY = baseY + i * itemSpacing;
					if (mouseY >= itemY - itemHitTop - 3 * (highRes ? 2 : 1) &&
							mouseY < itemY - itemHitTop + itemHitHeight - 3 * (highRes ? 2 : 1)) {
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

// Draw preview box border - emulates FUN_004292D0 calls at lines 128-133 of FUN_00415CF8
void InsaneRebel2::drawPreviewBox(byte *renderBitmap, int pitch, int width, int height) {
	// Low-res (320x200) coordinates from FUN_00415CF8:
	// Outer box: X=0xe4 (228), Y=0x49 (73), W=0x54 (84), H=0x36 (54), color=0xF8
	// Inner box: X=0xe5 (229), Y=0x4a (74), W=0x52 (82), H=0x34 (52), color=4
	// High-res uses the original DAT_0047a808 >= 2 formulas, doubling the
	// low-res anchors and dimensions.
	const int scale = isHiRes() ? 2 : 1;

	// Outer border (bright)
	int outerX = 228 * scale, outerY = 73 * scale, outerW = 84 * scale, outerH = 54 * scale;
	byte outerColor = 0xF8;

	// Draw outer box edges
	// Top edge
	for (int px = outerX; px < outerX + outerW && px < width; px++) {
		if (outerY >= 0 && outerY < height && px >= 0)
			renderBitmap[outerY * pitch + px] = outerColor;
	}
	// Bottom edge
	int bottomY = outerY + outerH - 1;
	if (bottomY < height) {
		for (int px = outerX; px < outerX + outerW && px < width; px++) {
			if (px >= 0)
				renderBitmap[bottomY * pitch + px] = outerColor;
		}
	}
	// Left edge
	for (int py = outerY; py < outerY + outerH && py < height; py++) {
		if (py >= 0 && outerX >= 0 && outerX < width)
			renderBitmap[py * pitch + outerX] = outerColor;
	}
	// Right edge
	int rightX = outerX + outerW - 1;
	if (rightX < width) {
		for (int py = outerY; py < outerY + outerH && py < height; py++) {
			if (py >= 0)
				renderBitmap[py * pitch + rightX] = outerColor;
		}
	}

	// Inner border (dark)
	int innerX = 229 * scale, innerY = 74 * scale, innerW = 82 * scale, innerH = 52 * scale;
	byte innerColor = 4;

	// Top edge
	for (int px = innerX; px < innerX + innerW && px < width; px++) {
		if (innerY >= 0 && innerY < height && px >= 0)
			renderBitmap[innerY * pitch + px] = innerColor;
	}
	// Bottom edge
	bottomY = innerY + innerH - 1;
	if (bottomY < height) {
		for (int px = innerX; px < innerX + innerW && px < width; px++) {
			if (px >= 0)
				renderBitmap[bottomY * pitch + px] = innerColor;
		}
	}
	// Left edge
	for (int py = innerY; py < innerY + innerH && py < height; py++) {
		if (py >= 0 && innerX >= 0 && innerX < width)
			renderBitmap[py * pitch + innerX] = innerColor;
	}
	// Right edge
	rightX = innerX + innerW - 1;
	if (rightX < width) {
		for (int py = innerY; py < innerY + innerH && py < height; py++) {
			if (py >= 0)
				renderBitmap[py * pitch + rightX] = innerColor;
		}
	}
}

// Rating to medal string (FUN_0042001f): TALKFONT glyphs 3=big, 2=medium, 1=small
Common::String InsaneRebel2::getRankString(int rating) {
	if (rating > 50)
		rating = 50;
	Common::String result;
	while (rating >= 9) { result += (char)3; rating -= 9; }
	while (rating >= 3) { result += (char)2; rating -= 3; }
	while (rating >= 1) { result += (char)1; rating -= 1; }
	return result;
}

// Password table lookup - emulates FUN_0041BCE0
// 90 entries: 15 levels × 6 difficulty slots, extracted from RA2WIN95.EXE at 0x481AF0
// Index formula: difficulty + (level * 3 - 3) * 2, level is 1-based (1-15), difficulty 0-5
const char *const kPasswordTable[90] = {
	// Level 1:  diff 0-5
	"JABBA",    "EWOKS",    "BANTHA",   "ANAKIN",   "WOOKIEE",  "WOOKIEE",
	// Level 2:  diff 0-5
	"ENDOR",    "CHEWIE",   "KATANA",   "KENOBI",   "DROID",    "DROID",
	// Level 3:  diff 0-5
	"LACHTON",  "DANKIN",   "DENGAR",   "FORTUNA",  "RODIAN",   "RODIAN",
	// Level 4:  diff 0-5
	"BORSK",    "NOGHRI",   "PELLAEON", "MODON",    "BPFASSH",  "BPFASSH",
	// Level 5:  diff 0-5
	"KROYIES",  "CHAMMA",   "ITHULL",   "OMMIN",    "KSHYY",    "KSHYY",
	// Level 6:  diff 0-5
	"AURIL",    "BOGGA",    "STENNESS", "REKKON",   "TORVE",    "TORVE",
	// Level 7:  diff 0-5
	"KAMPL",    "INCOM",    "MYRKR",    "SHAZEEN",  "SLUISSI",  "SLUISSI",
	// Level 8:  diff 0-5
	"FERRIER",  "KOTHLIS",  "CHURBA",   "KIIRIUM",  "PALANHI",  "PALANHI",
	// Level 9:  diff 0-5
	"GALIA",    "KRATH",    "ARTOO",    "GUNDARK",  "DROKKO",   "DROKKO",
	// Level 10: diff 0-5
	"DENARII",  "SIOSK",    "SATAL",    "DIANOGA",  "NATTH",    "NATTH",
	// Level 11: diff 0-5
	"SADOW",    "ADEGAN",   "LOBUE",    "ATUARRE",  "SABACC",   "SABACC",
	// Level 12: diff 0-5
	"ONDERON",  "AMANOA",   "DENEBA",   "ESSADA",   "ANDUR",    "ANDUR",
	// Level 13: diff 0-5
	"ALEEMA",   "AMBRIA",   "STURM",    "PAPLOO",   "ARKANIA",  "ARKANIA",
	// Level 14: diff 0-5
	"CATHAR",   "SYLVAR",   "CRADO",    "NASHTAH",  "DIATH",    "DIATH",
	// Level 15: diff 0-5
	"DOMINIS",  "MIRALUKA", "CARRACK",  "PESTAGE",  "DREEBO",   "DREEBO",
};

Common::String InsaneRebel2::getChapterPassword(int level, int difficulty) {
	if (level < 1 || level > 15 || difficulty < 0 || difficulty > 5)
		return "";
	int idx = difficulty + (level * 3 - 3) * 2;
	return kPasswordTable[idx];
}

// Draw score/info line at bottom of chapter select
void InsaneRebel2::drawChapterInfoLine(byte *renderBitmap, int pitch, int width, int height) {
	if (_chapterSelection < 0 || _chapterSelection >= 16)
		return;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer)
		return;

	const int scale = isHiRes() ? 2 : 1;

	if (_chapterUnlocked[_chapterSelection]) {
		// Unlocked: show score info using TRS 80 at X=25 (0x19), Y=190 (0xbe)
		// TRS 80 = "^f01^c248Pilots: %hd  Score: %ld  Rank: ^f00%s"
		const char *fmtStr = splayer->getString(80);
		if (!fmtStr || !fmtStr[0])
			return;

		// Get pilot data for this chapter
		int32 pilotLives = 0;
		int32 pilotScore = 0;
		int16 pilotRating = 0;
		if (_activePilot >= 0 && _activePilot < _numPilots) {
			pilotLives = _pilots[_activePilot].lives[_chapterSelection];
			pilotScore = _pilots[_activePilot].score[_chapterSelection];
			pilotRating = _pilots[_activePilot].rating[_chapterSelection];
		}
		Common::String rankStr = getRankString(pilotRating);

		// sprintf substitution: %hd → lives, %ld → score, %s → rank
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

// Draw chapter selection overlay - called during O_LEVEL.SAN playback
// FUN_00415CF8 - Chapter selection screen with preview thumbnail
void InsaneRebel2::drawChapterSelectOverlay(byte *renderBitmap, int pitch, int width, int height) {
	// Emulates FUN_00415CF8 rendering via shared drawMenuItems(leftAligned=true)
	//
	// GAME.TRS chapter selection strings:
	//   TRS 40     = "^f02Chapters" (title)
	//   TRS 41-56  = unlocked chapter names (e.g. "^f01^c244Chapter 1 - The Dreighton Triangle")
	//   TRS 57     = "^f01^c240RETURN TO PILOTS"
	//   TRS 60     = "^f02Chapters" (title, locked section duplicate)
	//   TRS 61-76  = locked chapter names (e.g. "^f01^c244Chapter 1 -")
	//   TRS 77     = "^f01^c240RETURN TO PILOTS" (locked section duplicate)
	//
	// Menu array: items[0]=title, items[1..16]=chapters, items[17]=RETURN TO PILOTS
	// FUN_0041f5ae(0, &DAT_004577a8, 0x11, 1): param_3=17, param_4=1 (left-aligned)

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer)
		return;

	// Build items array matching original DAT_004577a8 layout
	const char *items[18];

	// Original (FUN_00415CF8 lines 55-77): starts with ALL locked strings,
	// then overrides unlocked chapters individually.
	// items[0] = title = TRS 60 ("^f02Chapters")
	items[0] = splayer->getString(60);
	if (!items[0] || !items[0][0])
		items[0] = "^f02Chapters";

	// items[1..16] = chapters, using unlocked (TRS 41-56) or locked (TRS 61-76) strings
	for (int i = 1; i <= 16; i++) {
		bool unlocked = _chapterUnlocked[i - 1];
		int trsIdx = unlocked ? (40 + i) : (60 + i);
		items[i] = splayer->getString(trsIdx);
		if (!items[i] || !items[i][0])
			items[i] = "";
	}

	// items[17] = "RETURN TO PILOTS" = TRS 77
	items[17] = splayer->getString(77);
	if (!items[17] || !items[17][0])
		items[17] = "^f01^c240RETURN TO PILOTS";

	// Render menu using shared renderer with left-aligned mode
	drawMenuItems(renderBitmap, pitch, width, height, items, 17, _chapterSelection, true);

	// Draw preview box border on the right side (FUN_004292d0 calls at lines 128-133)
	// The preview thumbnail is rendered by O_LEVEL.SAN via FOBJ offset + STOR/FTCH.
	// SKIP chunks in O_LEVEL.SAN use iactBits to show locked/unlocked preview variants.
	drawPreviewBox(renderBitmap, pitch, width, height);

	// Draw score/info line at bottom
	drawChapterInfoLine(renderBitmap, pitch, width, height);
}

// ---------------------------------------------------------------------------
// Pilot Selection Menu (FUN_00414A41)
// ---------------------------------------------------------------------------
// Pilot/save selection before chapter selection.

// runLevelSelect -- Pilot selection menu (FUN_00414A41).
// Returns kLevelSelectPlay, kLevelSelectBack, or kLevelSelectQuit.
int InsaneRebel2::runLevelSelect() {

	debugC(DEBUG_INSANE, "Entering pilot selection (FUN_00414A41), %d pilots loaded", _numPilots);

	_menuInputActive = true;
	while (!_menuEventQueue.empty())
		_menuEventQueue.pop();

	_levelSelection = 0;
	_levelItemCount = _numPilots + 4;  // N pilots + NEW/COPY/DELETE/MAIN MENU
	_selectedLevel = 1;
	_menuRepeatDelay = 0;
	_gameState = kStatePilotSelect;
	_pilotMenuMode = kPilotModeSelect;
	_pilotNameInput = "";
	_pilotEditIndex = -1;
	updateMenuVirtualKeyboard();

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Pilot selection uses menu video as background (320x200 mode)
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

		// --- Difficulty submenu completed ---
		if (_pilotMenuMode == kPilotModeDifficulty) {
			// Store difficulty in the new pilot and save
			if (_pilotEditIndex >= 0 && _pilotEditIndex < _numPilots) {
				_pilots[_pilotEditIndex].difficulty = _difficultySelection;
				_difficulty = _difficultySelection;
				savePilots();
				_activePilot = _pilotEditIndex;

				// Update chapter unlock state from pilot data
				for (int i = 0; i < 16; i++) {
					_chapterUnlocked[i] = _debugUnlockAll || (_pilots[_activePilot].damage[i] < 0xFF);
				}
			}
			_pilotMenuMode = kPilotModeSelect;
			_levelItemCount = _numPilots + 4;
			_gameState = kStatePilotSelect;
			_menuInputActive = false;
			return kLevelSelectPlay;
		}

		// --- Name input completed ---
		if (_pilotMenuMode == kPilotModeNameInput) {
			// Name was confirmed — now show difficulty submenu
			if (_pilotEditIndex >= 0 && _pilotEditIndex < _numPilots) {
				Common::strlcpy(_pilots[_pilotEditIndex].name, _pilotNameInput.c_str(),
				                sizeof(_pilots[_pilotEditIndex].name));
			}
			setVirtualKeyboardVisible(false);
			_pilotMenuMode = kPilotModeDifficulty;
			_gameState = kStateDifficultySelect;
			_difficultySelection = 2;
			continue;
		}

		// --- Pilot operation submenu completed ---
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
			continue;
		}

		// --- Normal pilot menu selection ---
		debugC(DEBUG_INSANE, "Pilot selection: %d (numPilots=%d)", _levelSelection, _numPilots);

		if (_levelSelection < _numPilots) {
			// Existing pilot selected — activate and go to chapter select
			_activePilot = _levelSelection;
			_difficulty = _pilots[_activePilot].difficulty;

			// Update chapter unlock state from pilot data
			for (int i = 0; i < 16; i++) {
				_chapterUnlocked[i] = _debugUnlockAll || (_pilots[_activePilot].damage[i] < 0xFF);
			}

			debugC(DEBUG_INSANE, "Pilot '%s' selected (slot %d, difficulty %d)",
			      _pilots[_activePilot].name, _activePilot, _difficulty);
			setVirtualKeyboardVisible(false);
			_menuInputActive = false;
			return kLevelSelectPlay;

		} else if (_levelSelection == _numPilots) {
			// ADD NEW PILOT — create slot, enter name input mode
			int newIdx = createNewPilot();
			if (newIdx >= 0) {
				_pilotEditIndex = newIdx;
				_pilotNameInput = "";
				_pilotMenuMode = kPilotModeNameInput;
				_levelItemCount = _numPilots + 4;
				updateMenuVirtualKeyboard();
				debugC(DEBUG_INSANE, "NEW PILOT - entering name for slot %d", newIdx);
			}
			continue;

		} else if (_levelSelection == _numPilots + 1) {
			// COPY PILOT - original opens a second menu to select the source pilot.
			if (_numPilots > 0 && _numPilots < kMaxPilots) {
				_pilotMenuMode = kPilotModeCopySelect;
				_levelSelection = 0;
				_levelItemCount = _numPilots;
				debugC(DEBUG_INSANE, "COPY PILOT - selecting source");
			}
			continue;

		} else if (_levelSelection == _numPilots + 2) {
			// DELETE PILOT - original opens a second menu to select the target pilot.
			if (_numPilots > 0) {
				_pilotMenuMode = kPilotModeDeleteSelect;
				_levelSelection = 0;
				_levelItemCount = _numPilots;
				debugC(DEBUG_INSANE, "DELETE PILOT - selecting target");
			}
			continue;

		} else if (_levelSelection == _numPilots + 3) {
			// RETURN TO MAIN MENU
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
	// Process input for pilot selection and difficulty submenu.
	// Handles pilot list, pilot operation submenus, name input, and difficulty.
	// Returns: -1 = no action, 0+ = item selected

	int result = -1;

	// Name input mode — keyboard goes to _pilotNameInput instead of menu nav
	// Original: FUN_00414A41 lines 87-116
	if (_pilotMenuMode == kPilotModeNameInput) {
		while (!_menuEventQueue.empty()) {
			Common::Event event = _menuEventQueue.pop();
			if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
				    event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					// Confirm name — signal back to runLevelSelect()
					if (_pilotNameInput.size() > 0) {
						setVirtualKeyboardVisible(false);
						_menuSelectionConfirmed = true;
						_vm->_smushVideoShouldFinish = true;
						debugC(DEBUG_INSANE, "PilotName: confirmed '%s'", _pilotNameInput.c_str());
					}
				} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					// Synthetic custom back action - cancel name entry
					if (_pilotEditIndex >= 0 && _pilotEditIndex < _numPilots) {
						deletePilot(_pilotEditIndex);
					}
					_pilotMenuMode = kPilotModeSelect;
					_levelItemCount = _numPilots + 4;
					updateMenuVirtualKeyboard();
					debugC(DEBUG_INSANE, "PilotName: cancelled");
				} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
					// Backspace — remove last character
					if (_pilotNameInput.size() > 0) {
						_pilotNameInput.deleteLastChar();
					}
				} else {
					// Printable ASCII (0x20-0x7E), max 15 chars
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
				updateMenuVirtualKeyboard();
			}
		}
		return -1;
	}

	// Normal menu navigation (pilot select or difficulty submenu)
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
				} else if (isPilotOperationMode) {
					bool wasCopyMode = (_pilotMenuMode == kPilotModeCopySelect);
					_pilotMenuMode = kPilotModeSelect;
					_levelItemCount = _numPilots + 4;
					_levelSelection = _numPilots + (wasCopyMode ? 1 : 2);
				} else {
					result = _levelItemCount - 1;  // Last item = MAIN MENU
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
				if (event.mouse.y >= itemY - itemHitTop - 3 * (highRes ? 2 : 1) &&
						event.mouse.y < itemY - itemHitTop + itemHitHeight - 3 * (highRes ? 2 : 1)) {
					selection = i;
					break;
				}
			}
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			if (isDifficultyMode) {
				_gameState = kStatePilotSelect;
			} else if (isPilotOperationMode) {
				_pilotMenuMode = kPilotModeSelect;
				_levelItemCount = _numPilots + 4;
				_levelSelection = _levelItemCount - 1;
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
	// -------------------------------------------------------------------
	// Pilot selection / difficulty submenu renderer
	// Emulates FUN_00414A41 → FUN_0041f5ae
	// -------------------------------------------------------------------

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer) {
		debugC(DEBUG_INSANE, "drawLevelSelectOverlay: SmushPlayer not available for TRS strings!");
		return;
	}

	if (_gameState == kStateDifficultySelect) {
		// -------------------------------------------------------------------
		// Difficulty submenu - LAB_00414ff6
		// FUN_0041f5ae(0, &DAT_00457458, 6, 0)
		// DAT_00457458 = 7 entries loaded from TRS 110-116 (FUN_00414073 lines 47-50)
		// param_3 = 6 → items[0]=title(TRS 110), items[1..6]=selectable(TRS 111-116)
		// -------------------------------------------------------------------
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

	// -------------------------------------------------------------------
	// Pilot menu - FUN_0041f5ae(0, &DAT_00457768, N+4, 0)
	// -------------------------------------------------------------------
	// items[0]    = title (TRS 20)
	// items[1..N] = saved pilots (formatted with ^f01^c005<name>^f00)
	// items[N+1]  = TRS 21 (ADD NEW PILOT)
	// items[N+2]  = TRS 22 (COPY PILOT)
	// items[N+3]  = TRS 23 (DELETE PILOT)
	// items[N+4]  = TRS 24 (RETURN TO MAIN MENU)

	// Build pilot name strings with font/color formatting
	// Original uses "^f01^c005<name>^f00" for pilot entries
	Common::String pilotNameStrs[kMaxPilots];
	for (int i = 0; i < _numPilots; i++) {
		if (_pilotMenuMode == kPilotModeNameInput && i == _pilotEditIndex) {
			// Show name being typed with cursor (underscore)
			pilotNameStrs[i] = Common::String::format("^f01^c005%s_^f00", _pilotNameInput.c_str());
		} else {
			pilotNameStrs[i] = Common::String::format("^f01^c005%s^f00", _pilots[i].name);
		}
	}

	// Max items: 1 title + 10 pilots + 4 options = 15
	const char *pilotItems[15];
	int idx = 0;

	// Title: TRS 20
	pilotItems[idx++] = splayer->getString(20);

	// Saved pilot slots
	for (int i = 0; i < _numPilots; i++) {
		pilotItems[idx++] = pilotNameStrs[i].c_str();
	}

	// Fixed options: TRS 21-24
	for (int i = 0; i < 4; i++) {
		pilotItems[idx++] = splayer->getString(21 + i);
	}

	for (int i = 0; i < idx; i++) {
		if (!pilotItems[i] || !pilotItems[i][0]) {
			pilotItems[i] = "";
		}
	}

	drawMenuItems(renderBitmap, pitch, width, height, pilotItems, _numPilots + 4, _levelSelection);
}

// ---------------------------------------------------------------------------
// Top Pilots Screen (FUN_00420116)
// ---------------------------------------------------------------------------
// Ranked pilot scores with animated reveal over a menu background video.
// 0x4a (74) byte records (max 15, from FUN_00410271):
//   +0x00 (4): timestamp, +0x04 (40): name, +0x36 (4): score,
//   +0x3a (4): rating, +0x3e (2): difficulty tier (1-3, TRS=value+0x9b),
//   +0x40 (2): highest chapter (1-15).
// Column X positions (low-res): medals=43, name=88, diff=195, ch=245, score=295.
// Row Y: sVar1 * 10 + 42.

// initDefaultRankings -- Fill ranking table with defaults (FUN_0040FF00).
// Generates 15 placeholder entries: score=(15-i)*1500, rating=(15-i)*2,
// difficulty=((15-i)*3+14)/15, chapter=((15-i)*15+14)/15.
void InsaneRebel2::initDefaultRankings() {
	_numRankings = 0;
	memset(_rankings, 0, sizeof(_rankings));
	for (int i = 0; i < kMaxRankings; i++) {
		int k = kMaxRankings - i;  // 15 down to 1
		RankingEntry &r = _rankings[_numRankings];
		Common::strlcpy(r.name, "-----", sizeof(r.name));
		r.score = k * 1500;
		r.rating = k * 2;
		r.difficulty = (int16)((k * 3 + 14) / 15);
		r.chapter = (int16)((k * 15 + 14) / 15);
		_numRankings++;
	}
}

// insertRanking -- Insert into sorted ranking table (FUN_00410271).
void InsaneRebel2::insertRanking(const char *name, int32 score, int32 rating,
                                  int16 difficulty, int16 chapter) {
	if (score == 0)
		return;

	// Find insertion point (first entry with score < new score)
	int insertPos = 0;
	while (insertPos < _numRankings && score <= _rankings[insertPos].score) {
		insertPos++;
	}
	if (insertPos > kMaxRankings - 1)
		return;

	// Remove any existing entry with same name
	for (int i = 0; i < _numRankings; i++) {
		if (strcmp(_rankings[i].name, name) == 0) {
			if (score <= _rankings[i].score)
				return;  // Existing entry has higher score
			// Remove old entry by shifting
			for (int j = i; j < _numRankings - 1; j++)
				_rankings[j] = _rankings[j + 1];
			_numRankings--;
			if (insertPos > i)
				insertPos--;
			break;
		}
	}

	// Shift entries down to make room
	int lastIdx = MIN(_numRankings, kMaxRankings - 1);
	for (int i = lastIdx; i > insertPos; i--)
		_rankings[i] = _rankings[i - 1];

	// Insert new entry
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

	// param_1 = -1 from main menu: maxFrames = 120 (0x78)
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

	// Title centered at X=152, Y=10 (TITLFONT)
	drawMenuStringCentered(renderBitmap, "^f02Top Pilots", 152 * scale, 10 * scale);

	// Column headers at Y=30 (SMALFONT), positioned to match data columns
	int headerY = 30 * scale;
	int headerColor = 5;
	drawMenuStringCentered(renderBitmap, "^f01Rank", 43 * scale, headerY, headerColor);
	drawMenuString(renderBitmap, "^f01Name", 88 * scale, headerY, headerColor);
	drawMenuStringCentered(renderBitmap, "^f01Difficulty", 195 * scale, headerY, headerColor);
	drawMenuStringCentered(renderBitmap, "^f01Chapter", 245 * scale, headerY, headerColor);
	drawMenuStringRight(renderBitmap, "^f01Score", 295 * scale, headerY, headerColor);

	// Animated reveal: show up to _topPilotsFrameCount entries
	int showCount = MIN(_topPilotsFrameCount, _numRankings);

	for (int row = 0; row < showCount; row++) {
		const RankingEntry &r = _rankings[row];
		int rowY = (row * 10 + 42) * scale;
		int color = 244;  // 0xF4

		// Column 1: Rank medals at X=43, centered (font 0 = TALKFONT)
		Common::String rankStr = getRankString(r.rating);
		if (!rankStr.empty()) {
			Common::String rankFmt = Common::String::format("^f00%s", rankStr.c_str());
			drawMenuStringCentered(renderBitmap, rankFmt.c_str(), 43 * scale, rowY, color);
		}

		// Column 2: Pilot name at X=88, left-aligned (font 1 = SMALFONT)
		Common::String nameFmt = Common::String::format("^f01%s", r.name);
		drawMenuString(renderBitmap, nameFmt.c_str(), 88 * scale, rowY, color);

		// Column 3: Difficulty at X=195, centered - TRS (difficulty + 155)
		int trsIdx = CLIP((int)r.difficulty, 0, 5) + 155;
		const char *diffStr = splayer->getString(trsIdx);
		if (diffStr && diffStr[0]) {
			Common::String diffFmt = Common::String::format("^f01%s", diffStr);
			drawMenuStringCentered(renderBitmap, diffFmt.c_str(), 195 * scale, rowY, color);
		}

		// Column 4: Highest chapter at X=245, centered
		Common::String chFmt = Common::String::format("^f01%d", (int)r.chapter);
		drawMenuStringCentered(renderBitmap, chFmt.c_str(), 245 * scale, rowY, color);

		// Column 5: Total score at X=295, right-aligned
		Common::String scoreFmt = Common::String::format("^f01%ld", (long)r.score);
		drawMenuStringRight(renderBitmap, scoreFmt.c_str(), 295 * scale, rowY, color);
	}

	_topPilotsFrameCount++;
}

// ---------------------------------------------------------------------------
// Options Menu (FUN_004167A6)
// ---------------------------------------------------------------------------
// Toggle labels and slider items. Settings at DAT_00482e20[0..3].
// TRS IDs: Title=89, Music=90/91, SFX=92/93, Voices=94/95, Text=96/97,
// Controls=98/99, Rapid Fire=100/101, Volume=103 "%hd%%", Back=107.

// showOptionsMenu -- Options menu loop (FUN_00416787).
void InsaneRebel2::showOptionsMenu() {
	debugC(DEBUG_INSANE, "Showing Options menu (FUN_00416787)");

	_menuInputActive = true;
	while (!_menuEventQueue.empty())
		_menuEventQueue.pop();

	_optionsSelection = 0;
	_optionsItemCount = 8;
	_optionsExitRequested = false;

	_gameState = kStateOptions;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Loop videos until user exits options (same pattern as runMainMenu)
	while (!_vm->shouldQuit() && !_optionsExitRequested) {
		_vm->_smushVideoShouldFinish = false;

		Common::String menuVideo = getRandomMenuVideo();
		splayer->setCurVideoFlags(0x20);
		splayer->play(menuVideo.c_str(), 15);
	}

	_gameState = kStateMainMenu;
	_menuInputActive = true;

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
				// Volume slider: decrease by 4 (original step size)
				if (_optionsSelection == 6) {
					_optVolumeLevel = MAX(0, _optVolumeLevel - 4);
					setRebel2MixerVolume(_vm, _optVolumeLevel);
				}
				return -1;

			case Common::KEYCODE_RIGHT:
				// Volume slider: increase by 4
				if (_optionsSelection == 6) {
					_optVolumeLevel = MIN(127, _optVolumeLevel + 4);
					setRebel2MixerVolume(_vm, _optVolumeLevel);
				}
				return -1;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				// Toggle items 0-5, back item 7
				switch (_optionsSelection) {
				case 0:  // Music toggle
					_optMusicEnabled = !_optMusicEnabled;
					_vm->_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, !_optMusicEnabled);
					break;
				case 1:  // SFX toggle
					_optSfxEnabled = !_optSfxEnabled;
					_vm->_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, !_optSfxEnabled);
					break;
				case 2:  // Voices toggle
					_optVoicesEnabled = !_optVoicesEnabled;
					_vm->_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, !_optVoicesEnabled);
					break;
				case 3:  // Text toggle
					_optTextEnabled = !ConfMan.getBool("subtitles");
					ConfMan.setBool("subtitles", _optTextEnabled);
					break;
				case 4:  // Controls toggle
					_optControlsFlipped = !_optControlsFlipped;
					break;
				case 5:  // Rapid fire toggle
					_optRapidFire = !_optRapidFire;
					break;
				case 6:  // Volume (handled by left/right)
					break;
				case 7:  // Back
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
			// Mouse click on items — match drawMenuItems Y positions
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
					// Simulate enter for this item
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

	// Build items array from TRS strings based on current toggle states
	// TRS 89: title, 90/91: music, 92/93: sfx, 94/95: voices,
	// 96/97: text, 98/99: controls, 100/101: rapid fire, 103: volume, 107: back
	const char *items[10];  // title + up to 9 selectable

	// [0] Title
	items[0] = splayer->getString(89);
	if (!items[0] || !items[0][0])
		items[0] = "^f02Game Options";

	// [1] Music On/Off
	items[1] = splayer->getString(_optMusicEnabled ? 90 : 91);
	if (!items[1] || !items[1][0])
		items[1] = _optMusicEnabled ? "^f01^c005Music is On" : "^f01^c005Music is Off";

	// [2] SFX On/Off
	items[2] = splayer->getString(_optSfxEnabled ? 92 : 93);
	if (!items[2] || !items[2][0])
		items[2] = _optSfxEnabled ? "^f01^c005SFX are On" : "^f01^c005SFX are Off";

	// [3] Voices On/Off
	items[3] = splayer->getString(_optVoicesEnabled ? 94 : 95);
	if (!items[3] || !items[3][0])
		items[3] = _optVoicesEnabled ? "^f01^c005Voices are On" : "^f01^c005Voices are Off";

	// [4] Text On/Off
	items[4] = splayer->getString(_optTextEnabled ? 96 : 97);
	if (!items[4] || !items[4][0])
		items[4] = _optTextEnabled ? "^f01^c005Text is On" : "^f01^c005Text is Off";

	// [5] Controls Normal/Flipped
	items[5] = splayer->getString(_optControlsFlipped ? 99 : 98);
	if (!items[5] || !items[5][0])
		items[5] = _optControlsFlipped ? "^f01^c005Controls are Flipped" : "^f01^c005Controls are Normal";

	// [6] Rapid Fire On/Off
	items[6] = splayer->getString(_optRapidFire ? 100 : 101);
	if (!items[6] || !items[6][0])
		items[6] = _optRapidFire ? "^f01^c005Rapid Fire On" : "^f01^c005Rapid Fire Off";

	// [7] Volume Level (slider) — TRS 103 = "^f01^c005Volume Level: %hd%%"
	char volumeBuf[64];
	const char *volFmt = splayer->getString(103);
	if (volFmt && volFmt[0])
		Common::sprintf_s(volumeBuf, volFmt, (short)(_optVolumeLevel * 100 / 127));
	else
		Common::sprintf_s(volumeBuf, "^f01^c005Volume Level: %hd%%", (short)(_optVolumeLevel * 100 / 127));
	items[7] = volumeBuf;

	// [8] Back — TRS 107
	items[8] = splayer->getString(107);
	if (!items[8] || !items[8][0])
		items[8] = "^f01^c240Return to Main Menu";

	drawMenuItems(renderBitmap, pitch, width, height, items, _optionsItemCount, _optionsSelection);
}

} // End of namespace Scumm
