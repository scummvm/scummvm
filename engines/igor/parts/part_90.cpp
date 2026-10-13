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

void IgorEngine::part90() {
	static const uint16 holdTicks[] = { 2000, 500, 500, 2500, 2000 };
	const uint16 duration = holdTicks[_currentPart - 900];
	const char *str;

	if (_currentPart == 900) {
		playMusic(14);
	}

	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);
	switch (_currentPart) {
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
		memcpy(_paletteBuffer, _currentPalette, 768);
		_inputVars[kInputSkipDialogue] = 0;
		_inputVars[kInputEscape] = 0;
		_inputVars[kInputOptions] = 0;
		_inputVars[kInputClick] = 0;
		const uint32 timerStart = _system->getMillis();
		uint8 observedTimer = 0;
		for (uint16 elapsed = 0; elapsed < duration; ++elapsed) {
			if (_inputVars[kInputOptions]) {
				handleOptionsMenu();
				_inputVars[kInputOptions] = 0;
				waitForTimer(0);
			}
			while (!_gameStateLoaded && !_eventQuitGame && _currentPart != kInvalidPart) {
				waitForTimer(0, false);
				const uint32 timerMillis = _system->getMillis() - timerStart;
				const uint64 timerRate = (uint64)kTickDelay * _fastMode;
				const uint64 timerTicks = (uint64)timerMillis * timerRate / 1000;
				const uint8 timerByte = (uint8)timerTicks;
				if (timerByte != observedTimer) {
					observedTimer = timerByte;
					break;
				}
				const uint32 deadline = timerStart + ((timerTicks + 1) * 1000 + timerRate - 1) / timerRate;
				waitForTimer(0, false, deadline);
			}
			_gameTicks = (_gameTicks + 1) & 63;
			if (_gameStateLoaded || _eventQuitGame || _currentPart == kInvalidPart) {
				break;
			}
		}
	}
	if (_gameStateLoaded || _eventQuitGame || _currentPart == kInvalidPart) {
		if (!_gameStateLoaded)
			fadeOut(768);
		SET_PAL_208_96_1();
		SET_PAL_240_48_1();
		return;
	}
	part90FadePalette(false);
	if (_gameStateLoaded || _eventQuitGame || _currentPart == kInvalidPart) {
		return;
	}
	++_currentPart;
	if ((_game.version == kIdSpaCD && _currentPart == 904) || _currentPart == 905) {
		_currentPart = 850;
	}
}

} // End of namespace Igor

