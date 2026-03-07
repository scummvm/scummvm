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

#include "common/system.h"
#include "common/events.h"

#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

bool InsaneRebel1::notifyEvent(const Common::Event &event) {
	if (_menuActive && _optionsActive && event.type == Common::EVENT_KEYDOWN) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
		case Common::KEYCODE_w:
			_optionsSel = (_optionsSel + 2) % 3;
			return true;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_s:
			_optionsSel = (_optionsSel + 1) % 3;
			return true;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_SPACE:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_ESCAPE:
			_optionsSel = 2;  // Back
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			break;
		}
	}

	if (_menuActive && !_optionsActive && event.type == Common::EVENT_KEYDOWN) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
		case Common::KEYCODE_w:
			_menuSelection = (_menuSelection + 4) % 5;
			return true;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_s:
			_menuSelection = (_menuSelection + 1) % 5;
			return true;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_SPACE:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_1:
		case Common::KEYCODE_2:
		case Common::KEYCODE_3:
		case Common::KEYCODE_4:
		case Common::KEYCODE_5:
			_menuSelection = event.kbd.keycode - Common::KEYCODE_1;
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_ESCAPE:
			_menuSelection = 4;
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			break;
		}
	}

	if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
		if (_player) {
			debug("Rebel1: ESC pressed - skipping video");
			_vm->_smushVideoShouldFinish = true;
			return true;
		}
	}

	return false;
}

void InsaneRebel1::renderMainMenuOverlay(byte *dst, int pitch, int width, int height) {
	_menuFrameCounter++;

	if (_optionsActive) {
		// --- Options submenu ---
		static const char *kDiffNames[3] = { "EASY", "NORMAL", "HARD" };

		const int titleW = getFontBankStringWidth("GAME OPTIONS");
		drawFontBankString(dst, pitch, width, height, (width - titleW) / 2, 36, "GAME OPTIONS");

		// Build dynamic option strings
		char diffLine[64];
		snprintf(diffLine, sizeof(diffLine), "DIFFICULTY: %s", kDiffNames[CLIP(_difficulty, 0, 2)]);
		const char *turbLine = _turbulenceEnabled ? "TURBULENCE: ON" : "TURBULENCE: OFF";
		const char *kOptionsItems[3] = { diffLine, turbLine, "BACK" };

		const int menuY = 60;
		const int rowH = 16;

		for (int i = 0; i < 3; i++) {
			const int textW = getFontBankStringWidth(kOptionsItems[i]);
			const int textX = (width - textW) / 2;
			const int y = menuY + i * rowH;
			drawFontBankString(dst, pitch, width, height, textX, y + 1, kOptionsItems[i]);

			if (i == _optionsSel) {
				byte highlightColor = ((_menuFrameCounter / 8) & 1) ? 248 : 240;
				int bracketWidth = textW + 12;
				int leftX = CLIP(textX - 6, 0, width - 1);
				int rightX = CLIP(leftX + bracketWidth, 0, width - 1);
				int topY = CLIP(y - 1, 0, height - 1);
				int bottomY = CLIP(y + rowH - 2, 0, height - 1);
				for (int x = leftX; x <= rightX; x++) {
					dst[topY * pitch + x] = highlightColor;
					dst[bottomY * pitch + x] = highlightColor;
				}
				for (int py = topY; py <= bottomY; py++) {
					dst[py * pitch + leftX] = highlightColor;
					dst[py * pitch + rightX] = highlightColor;
				}
			}
		}
		return;
	}

	// --- Main menu ---
	static const char *kMenuItems[5] = {
		"START NEW GAME",
		"GAME OPTIONS",
		"ENTER PASSCODE",
		"CONTINUE DEMO",
		"EXIT TO DOS"
	};

	// Center title
	const int titleW = getFontBankStringWidth("MAIN MENU");
	const int titleX = (width - titleW) / 2;
	drawFontBankString(dst, pitch, width, height, titleX, 36, "MAIN MENU");

	// Draw menu items centered horizontally
	const int menuY = 60;
	const int rowH = 16;

	for (int i = 0; i < 5; i++) {
		const int textW = getFontBankStringWidth(kMenuItems[i]);
		const int textX = (width - textW) / 2;
		const int y = menuY + i * rowH;

		drawFontBankString(dst, pitch, width, height, textX, y + 1, kMenuItems[i]);

		// Selection highlight box — flashing border (FUN_004292d0 pattern from RA2)
		if (i == _menuSelection) {
			// Flash between two palette colors every 8 frames
			byte highlightColor = ((_menuFrameCounter / 8) & 1) ? 248 : 240;

			int bracketWidth = textW + 12;
			int bracketHeight = rowH;
			int leftX = textX - 6;
			int rightX = leftX + bracketWidth;
			int topY = y - 1;
			int bottomY = y + bracketHeight - 2;

			// Clamp
			if (leftX < 0) leftX = 0;
			if (rightX >= width) rightX = width - 1;
			if (topY < 0) topY = 0;
			if (bottomY >= height) bottomY = height - 1;

			// Draw rectangle border (4 lines)
			for (int x = leftX; x <= rightX && x < width; x++) {
				if (topY >= 0 && topY < height)
					dst[topY * pitch + x] = highlightColor;
				if (bottomY >= 0 && bottomY < height)
					dst[bottomY * pitch + x] = highlightColor;
			}
			for (int py = topY; py <= bottomY && py < height; py++) {
				if (leftX >= 0 && leftX < width)
					dst[py * pitch + leftX] = highlightColor;
				if (rightX >= 0 && rightX < width)
					dst[py * pitch + rightX] = highlightColor;
			}
		}
	}
}

int InsaneRebel1::runMainMenu() {
	debug(1, "InsaneRebel1: Main menu");

	_menuSelection = 0;
	while (!_vm->shouldQuit()) {
		_menuActive = true;
		_menuConfirmed = false;
		_menuFrameCounter = 0;
		clearVideoBuffer();
		playCinematic("OPEN/O1OPTION.ANM");
		_menuActive = false;

		if (_vm->shouldQuit())
			return 5;

		if (_menuConfirmed)
			return _menuSelection + 1;
	}

	return 5;
}

void InsaneRebel1::runOptionsMenu() {
	_optionsSel = 0;
	_optionsActive = true;

	while (!_vm->shouldQuit()) {
		_menuActive = true;
		_menuConfirmed = false;
		_menuFrameCounter = 0;
		clearVideoBuffer();
		playCinematic("OPEN/O1OPTION.ANM");
		_menuActive = false;

		if (_vm->shouldQuit())
			break;

		if (_menuConfirmed) {
			switch (_optionsSel) {
			case 0:
				// Cycle difficulty
				_difficulty = (_difficulty + 1) % 3;
				loadTuningForLevel(0);
				break;
			case 1:
				// Toggle turbulence
				_turbulenceEnabled = !_turbulenceEnabled;
				break;
			case 2:
				// Back to main menu
				_optionsActive = false;
				return;
			}
		}
	}
	_optionsActive = false;
}

} // End of namespace Scumm
