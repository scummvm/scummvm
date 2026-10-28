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

#include "igor/igor.h"

namespace Igor {

static const int kSoundSpanishFloppyTitle = 64;

void IgorEngine::part90FadePalette(bool fadeInColors) {
	for (int pass = 1; pass <= 63; ++pass) {
		const int threshold = fadeInColors ? 64 - pass : pass;
		for (int i = 0; i < 768; ++i) {
			if (_paletteBuffer[i] >= threshold) {
				if (fadeInColors) {
					++_currentPalette[i];
				} else {
					--_currentPalette[i];
				}
			}
		}
		setPaletteRange(0, 255);
		// Present each palette pass and poll input without a timer delay.
		waitForTimer(0);
		if (_gameStateLoaded || _eventQuitGame || _currentPart == kInvalidPart) {
			return;
		}
	}
}

enum {
	kHoldNoSkip,
	kHoldSkipConfirm, // Enter ends the hold
	kHoldSkipAnyKey // Escape, Enter, Space or a mouse button ends the hold
};

/**
 * Holds the current screen for [duration] timer ticks, one iteration per change of
 * the timer counter. With [optionsMenu] the options menu can be opened meanwhile.
 * Returns false if the game was loaded or quit meanwhile.
 */
bool IgorEngine::part90Hold(uint16 duration, int skipMode, bool optionsMenu) {
	const bool menuWasReachable = _optionsMenuReachable;
	_optionsMenuReachable = optionsMenu;
	_inputVars[kInputSkipDialogue] = 0;
	_inputVars[kInputEscape] = 0;
	_inputVars[kInputOptions] = 0;
	_inputVars[kInputClick] = 0;
	if (!isCDVersion())
		_inputVars[kInputConfirm] = 0;
	if (skipMode == kHoldSkipAnyKey)
		_inputVars[kInputRightClick] = 0;

	auto skipRequested = [&]() -> bool {
		switch (skipMode) {
		case kHoldSkipConfirm:
			return _inputVars[kInputConfirm] != 0;
		case kHoldSkipAnyKey:
			return _inputVars[kInputEscape] || _inputVars[kInputConfirm] || _inputVars[kInputClick] ||
				_inputVars[kInputRightClick] || (!optionsMenu && _inputVars[kInputOptions]);
		default:
			return false;
		}
	};

	const uint32 timerStart = _system->getMillis();
	uint8 observedTimer = 0;
	for (uint16 elapsed = 0; elapsed < duration; ++elapsed) {
		if (optionsMenu && _inputVars[kInputOptions]) {
			// The title screen asks for the protection before opening the menu for the first time.
			if (_currentPart == 900 && !checkCopyProtection())
				break;
			handleOptionsMenu();
			_inputVars[kInputOptions] = 0;
			waitForTimer(0);
		}
		while (!_gameStateLoaded && !_eventQuitGame && _currentPart != kInvalidPart && !skipRequested()) {
			waitForTimer(0, false);
			const uint64 timerRate = (uint64)kTickDelay * _fastMode;
			const uint64 timerTicks = ticksSince(timerStart);
			const uint8 timerByte = (uint8)timerTicks;
			if (timerByte != observedTimer) {
				observedTimer = timerByte;
				break;
			}
			const uint32 deadline = timerStart + ((timerTicks + 1) * 1000 + timerRate - 1) / timerRate;
			waitForTimer(0, false, deadline);
		}
		_gameTicks = (_gameTicks + 1) & 63;
		if (_gameStateLoaded || _eventQuitGame || _currentPart == kInvalidPart || skipRequested()) {
			break;
		}
	}
	if (skipMode == kHoldSkipConfirm) {
		_inputVars[kInputConfirm] = 0;
	} else if (skipMode == kHoldSkipAnyKey) {
		_inputVars[kInputEscape] = 0;
		_inputVars[kInputConfirm] = 0;
		_inputVars[kInputClick] = 0;
		_inputVars[kInputRightClick] = 0;
		_inputVars[kInputOptions] = 0;
	}
	_optionsMenuReachable = menuWasReachable;
	return !_gameStateLoaded && !_eventQuitGame && _currentPart != kInvalidPart;
}

/**
 * The first screen of the Spanish floppy raises the colors 121 to 239 of its
 * palette towards white in steps (the highlight on the logo), 5 timer ticks
 * apart.
 */
void IgorEngine::part90Highlight(bool brighten) {
	for (int step = 0; step <= 55; ++step) {
		const int level = brighten ? 55 - step : step;
		for (int i = 121 * 3; i < 240 * 3; ++i) {
			if (_paletteBuffer[i] <= level) {
				if (brighten) {
					++_currentPalette[i];
				} else {
					--_currentPalette[i];
				}
			}
		}
		setPaletteRange(121, 239);
		waitForTimer(brighten ? 5 : 0);
		if (_gameStateLoaded || _eventQuitGame || _currentPart == kInvalidPart) {
			return;
		}
	}
}


void IgorEngine::part90SetUserInterfacePalettes() {
	loadData(PAL_InventoryPanel, _paletteBuffer + 208 * 3);
	loadData(PAL_VerbsPanel, _paletteBuffer + 240 * 3);
}

void IgorEngine::part90() {
	const bool savingWasAllowed = _canSaveGame;
	_canSaveGame = false;

	const bool spanishFloppy = (_game.version == kIdSpaFloppy);
	static const uint16 holdTicks[] = { 2000, 500, 500, 2500, 2000 };
	const uint16 duration = spanishFloppy ? 0 : holdTicks[_currentPart - 900];
	const char *str;
	Common::Array<uint8> titleImage;

	if (_currentPart == 900 && isCDVersion()) {
		playMusic(14);
	}
	if (spanishFloppy && _currentPart == 980) {
		// The title screen selection repeats until the intro starts.
		playMusic(2);
	}

	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);
	// Spanish floppy has only two screens
	if (spanishFloppy) {
		if (_currentPart == 900) {
			loadData(PAL_PenduloStudios, _paletteBuffer);
			loadData(IMG_PenduloStudios, _screenVGA);
			loadData(PAL_VerbsPanel, _paletteBuffer + 240 * 3);
		} else {
			// the title is drawn over the image shown first, with the same palette
			loadData(PAL_TitleScreen, _paletteBuffer);
			loadData(IMG_FloppyTitleBackdrop, _screenVGA);
			titleImage.resize(320 * 200);
			loadData(IMG_TitleScreen, titleImage.data());
		}
	} else switch (_currentPart) {
	case 900:
		loadData(PAL_OptikSoftware, _paletteBuffer);
		loadData(IMG_OptikSoftware, _screenVGA);
		break;
	case 901:
		loadData(PAL_Presents, _paletteBuffer);
		loadData(IMG_Presents, _screenVGA);
		break;
	case 902:
		loadData(PAL_GraphicAdventure, _paletteBuffer);
		loadData(IMG_GraphicAdventure, _screenVGA);
		break;
	case 903:
		loadData(PAL_PenduloStudios, _paletteBuffer);
		loadData(IMG_PenduloStudios, _screenVGA);
		break;
	case 904:
		loadData(PAL_TitleScreen, _paletteBuffer);
		loadData(IMG_TitleScreen, _screenVGA);
		str = (_game.version == kIdEngDemo110) ? getString(STR_Copyright1994) : getString(STR_Copyright1995);
		_font.drawString(_screenVGA, str, 2, 187, 0xF5, 0, 0);
		break;
	}
	if (_gameStateLoaded) {
		_gameStateLoaded = false;
		fadeIn(768);
	} else {
		part90FadePalette(true);
	}
	if (!_gameStateLoaded && !_eventQuitGame && _currentPart != kInvalidPart) {
		if (spanishFloppy && _currentPart == 900) {
			// no input is checked during the first hold
			playMusic(1);
			bool running = part90Hold(800, kHoldNoSkip, false);
			if (running) {
				part90Highlight(true);
				playMusic(0);
				memcpy(_paletteBuffer, _currentPalette, 768);
				running = part90Hold(2000, kHoldSkipAnyKey, true);
			}
			if (running) {
				part90Highlight(false);
			}
		} else if (spanishFloppy) {
			if (part90Hold(1000, kHoldSkipAnyKey, false)) {
				memcpy(_screenVGA, titleImage.data(), 320 * 200);
				playSound(kSoundSpanishFloppyTitle, 1);
				waitForTimer(0);
				part90Hold(1000, kHoldSkipAnyKey, false);
			}
		} else {
			memcpy(_paletteBuffer, _currentPalette, 768);
			part90Hold(duration, isCDVersion() ? kHoldNoSkip : kHoldSkipConfirm, true);
		}
	}
	if (_gameStateLoaded || _eventQuitGame || _currentPart == kInvalidPart) {
		if (!_gameStateLoaded)
			fadeOut(768);
		if (spanishFloppy) {
			part90SetUserInterfacePalettes();
		} else {
			SET_PAL_208_96_1();
			SET_PAL_240_48_1();
		}
		_canSaveGame = savingWasAllowed;
		return;
	}
	part90FadePalette(false);
	if (_gameStateLoaded || _eventQuitGame || _currentPart == kInvalidPart) {
		_canSaveGame = savingWasAllowed;
		return;
	}
	if (spanishFloppy) {
		_currentPart = (_currentPart == 900) ? 980 : 850;
	} else {
		++_currentPart;
		if ((_game.version == kIdSpaCD && _currentPart == 904) || _currentPart == 905) {
			_currentPart = 850;
		}
	}
	_canSaveGame = savingWasAllowed;
}

} // End of namespace Igor

