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

#include "igor/igor.h"

namespace Igor {

struct CreditsLayout {
	int16 x;
	int16 y;
	uint8 count;
};

struct CreditsPage {
	uint16 imgId;
	uint16 palId;
	uint16 textId;
	CreditsLayout cd[2];
	CreditsLayout floppy;
};

static const CreditsPage kCreditsPages[7] = {
	{ IMG_Credits1, PAL_Credits1, TXT_Credits1, { { 232, 90, 5 }, { 210, 5, 2 } }, { 232, 90, 5 } },
	{ IMG_Credits2, PAL_Credits2, TXT_Credits2, { { 74, 105, 4 }, { 74, 20, 8 } }, { 74, 105, 4 } },
	{ IMG_Credits3, PAL_Credits3, TXT_Credits3, { { 244, 30, 2 }, { 244, 30, 8 } }, { 244, 30, 2 } },
	{ IMG_Credits4, PAL_Credits4, TXT_Credits4, { { 74, 23, 2 }, { 74, 23, 2 } }, { 74, 23, 2 } },
	{ IMG_Credits5, PAL_Credits5, TXT_Credits5, { { 210, 20, 2 }, { 240, 20, 2 } }, { 210, 20, 2 } },
	{ IMG_Credits6, PAL_Credits6, TXT_Credits6, { { 90, 5, 2 }, { 74, 72, 6 } }, { 74, 142, 2 } },
	{ IMG_Credits7, PAL_Credits7, TXT_Credits7, { { 240, 142, 2 }, { 238, 140, 2 } }, { 238, 90, 5 } }
};

// Number of timer units a page stays on the screen
static const int kCreditsPageTicks = 4000;
// Color of the names, set to white in the palette of every page
static const int kCreditsNameColor = 0xC3;

/**
 * Centers the string horizontally
 */
void IgorEngine::partCreditsDrawText(int x, int y, uint8 color, bool underline, const char *str) {
	const int width = _font.getStringWidth(str);
	_font.drawString(_screenVGA, str, x - (width >> 1), y, color, 0, 0);
	if (underline) { // headings have an underline of the same color as text
		const int barX = x - (width >> 1) - 10;
		const int barWidth = width + 21;
		memset(_screenVGA + (y + 18) * 320 + barX, 0, barWidth);
		memset(_screenVGA + (y + 19) * 320 + barX, color, barWidth);
		memset(_screenVGA + (y + 20) * 320 + barX, 0, barWidth);
		_screenVGA[(y + 19) * 320 + barX] = 0;
		_screenVGA[(y + 19) * 320 + barX + barWidth - 1] = 0;
	}
}

/**
 * States 910-970: seven pages with the credits, shown twice (the second time has other texts over the same
 * pictures), then the logos again.
 */
void IgorEngine::partCredits() {
	const bool savingWasAllowed = _canSaveGame;
	_canSaveGame = false;
	_optionsMenuReachable = true;
	const CreditsPage &page = kCreditsPages[(_currentPart - 910) / 10];

	if (isCDVersion())
		playMusic(9);
	else
		playMusic(12);

	fillScreenRows(0, 200, 0);
	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);

	loadData(page.imgId, _screenVGA);
	loadData(page.palId, _paletteBuffer);
	setVerbPanelColors();
	// The Spanish floppy pictures already contain the credits text
	const bool bakedText = _game.version == kIdSpaFloppy;
	if (!bakedText) {
		memset(_paletteBuffer + kCreditsNameColor * 3, 0x3F, 3);

		const bool englishFloppy = _game.version == kIdEngFloppy;
		const int pass = (_creditsPass == 0) ? 0 : 1;
		const CreditsLayout &layout = englishFloppy ? page.floppy : page.cd[pass];
		const int skip = !englishFloppy && pass == 1 ? page.cd[0].count : 0;
		int size;
		uint8 *texts = loadData(page.textId, nullptr, &size);
		int offset = 0;
		for (int i = 0; i < skip + layout.count; ++i) {
			assert(offset < size);
			const int length = texts[offset++];
			assert(offset + length <= size);
			if (i >= skip) {
				const int line = i - skip;
				const Common::String text(reinterpret_cast<const char *>(texts + offset), length);
				const int y = layout.y + (line == 0 ? 0 : 28 + (line - 1) * 15);
				const uint8 color = line == 0 ? 0xC2 : (englishFloppy ? 0xFF : kCreditsNameColor);
				partCreditsDrawText(layout.x, y, color, line == 0, text.c_str());
			}
			offset += length;
		}
		free(texts);
	}

	if (_gameStateLoaded) {
		fadeIn(768);
		_gameStateLoaded = false;
	} else {
		part79FadeColors(0, 255, true);
	}

	const int pageTicks = isFullFloppyVersion() ? 3928 : kCreditsPageTicks;
	for (int ticks = 0; ticks < pageTicks; ticks += kTimerTicksCount) {
		if (_inputVars[kInputOptions]) {
			_inputVars[kInputOptions] = 0;
			handleOptionsMenu();
		}
		// the original does not look at escape on these pages
		if (_inputVars[kInputEscape] && !_eventQuitGame) {
			_inputVars[kInputEscape] = 0;
		}
		waitForTimer();
		if (_currentPart == kInvalidPart || _gameStateLoaded) {
			break;
		}
	}

	if (_gameStateLoaded || _currentPart == kInvalidPart) {
		// a game was loaded or the game is quit: the state is not changed
		fadeOut(768);
		setInventoryPanelColors();
		_canSaveGame = savingWasAllowed;
	_optionsMenuReachable = false;
		return;
	}

	part79FadeColors(0, 255, false);
	if (_currentPart == 970) {
		_currentPart = (_creditsPass == 1) ? 900 : 910;
		++_creditsPass;
	} else {
		_currentPart += 10;
	}
	_canSaveGame = savingWasAllowed;
	_optionsMenuReachable = false;
}

} // End of namespace Igor
