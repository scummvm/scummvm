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

#include "agos/agos.h"
#include "agos/intern.h"

#include "graphics/fonts/amigafont.h"
#include "graphics/surface.h"

namespace AGOS {

void AGOSEngine_PN::clearInputLine() {
	_inputting = false;
	_inputReady = false;
	clearWindow(_windowArray[2]);
}

void AGOSEngine_PN::handleKeyboard() {
	if (!_inputReady)
		return;

	if (_keymapEnabled) {
		getEventManager()->getKeymapper()->getKeymap("game-shortcuts")->setEnabled(false);
		_keymapEnabled = false;
	}

	if (_hitCalled != 0) {
		mouseHit();
	}

	int16 chr = -1;
	if (_mouseString) {
		const char *strPtr = _mouseString;
		while (*strPtr != 0 && *strPtr != 13)
			addChar(*strPtr++);
		_mouseString = nullptr;

		chr = *strPtr;
		if (chr == 13) {
			addChar(13);
		}
	}
	if (_mouseString1 && chr != 13) {
		const char *strPtr = _mouseString1;
		while (*strPtr != 13)
			addChar(*strPtr++);
		_mouseString1 = nullptr;

		chr = *strPtr;
		if (chr == 13) {
			addChar(13);
		}
	}
	if (chr == -1) {
		if (_keyPressed.keycode == Common::KEYCODE_BACKSPACE || _keyPressed.keycode == Common::KEYCODE_RETURN) {
			chr = _keyPressed.keycode;
			addChar(chr);
		} else if (!(_videoLockOut & 0x10) &&
				   !_keyPressed.hasFlags(Common::KBD_CTRL) &&
				   !_keyPressed.hasFlags(Common::KBD_ALT)) {
			chr = _keyPressed.ascii;
			if (chr >= 32)
				addChar(chr);
		}
	}

	if (chr == 13) {
		_mouseString = nullptr;
		_mouseString1 = nullptr;
		_mousePrintFG = 0;
		_inputReady = false;
		if (!_keymapEnabled) {
			getEventManager()->getKeymapper()->getKeymap("game-shortcuts")->setEnabled(true);
			_keymapEnabled = true;
		}
	}

	_keyPressed.reset();
}

void AGOSEngine_PN::interact(char *buffer, uint8 size) {
	if (!_inputting) {
		memset(_keyboardBuffer, 0, sizeof(_keyboardBuffer));
		_intputCounter = 0;
		_inputMax = size;
		_inputWindow = _windowArray[_curWindow];
		if (isPnAmigaInputWindow(_inputWindow)) {
			drawPnAmigaTopazChar(_inputWindow, 128);
			compositePnAmigaTextPlane(_inputWindow);
		} else {
			windowPutChar(_inputWindow, 128);
			windowPutChar(_inputWindow, 8);
		}
		_inputting = true;
		_inputReady = true;
	}

	while (!shouldQuit() && _inputReady) {
		if (!_noScanFlag && _scanFlag) {
			buffer[0] = 1;
			buffer[1] = 0;
			_scanFlag = 0;
			break;
		}
		delay(1);
	}

	if (!_inputReady) {
		memcpy(buffer, _keyboardBuffer, size);
		_inputting = false;
	}
}

void AGOSEngine_PN::addChar(uint8 chr) {
	if (chr == 13) {
		_keyboardBuffer[_intputCounter++] = chr;
		windowPutChar(_inputWindow, 13);
	} else if (chr == 8 && _intputCounter) {
		clearCursor(_inputWindow);
		if (isPnAmigaInputWindow(_inputWindow)) {
			const byte deletedChar = _keyboardBuffer[_intputCounter - 1];
			const Graphics::AmigaFont *font = getPnAmigaFont();
			const uint16 advance = getPnAmigaGlyphAdvance(deletedChar);
			byte metricChar = deletedChar;
			int16 drawOffset = 0;
			uint16 inkWidth = 0;

			if (metricChar == 128)
				metricChar = '_';
			if (metricChar == 128 || metricChar == 129) {
				inkWidth = getPnAmigaGlyphAdvance(' ');
			} else if (font != nullptr && metricChar >= font->getLoChar() && metricChar <= font->getHiChar()) {
				drawOffset = font->getCharDrawOffset(metricChar);
				inkWidth = font->getCharInkWidth(metricChar);
			}

			_inputWindow->textColumn = MAX<int16>(kPnAmigaTextStartX, _inputWindow->textColumn - (int16)advance);

			PnAmigaTextPlane *plane = getPnAmigaTextPlane(_inputWindow);
			if (plane != nullptr && plane->pixels != nullptr) {
				Graphics::Surface surface;
				surface.init(plane->width, plane->height, plane->width, plane->pixels, Graphics::PixelFormat::createFormatCLUT8());
				const int16 clearLeft = CLIP<int16>(_inputWindow->textColumn + drawOffset, 0, plane->width);
				const int16 clearRight = CLIP<int16>(clearLeft + (int16)inkWidth, 0, plane->width);
				surface.fillRect(Common::Rect(clearLeft, _inputWindow->textRow,
					clearRight, _inputWindow->textRow + getPnAmigaGlyphHeight()), _inputWindow->fillColor);
				drawPnAmigaTopazChar(_inputWindow, 128);
				compositePnAmigaTextPlane(_inputWindow);
			}
		} else {
			windowPutChar(_inputWindow, 8);
			windowPutChar(_inputWindow, 128);
			windowPutChar(_inputWindow, 8);
		}

		_keyboardBuffer[--_intputCounter] = 0;
	} else if (chr >= 32 && _intputCounter < _inputMax) {
		_keyboardBuffer[_intputCounter++] = chr;

		clearCursor(_inputWindow);
		windowPutChar(_inputWindow, chr);
		if (isPnAmigaInputWindow(_inputWindow)) {
			drawPnAmigaTopazChar(_inputWindow, 128);
			compositePnAmigaTextPlane(_inputWindow);
		} else {
			windowPutChar(_inputWindow, 128);
			windowPutChar(_inputWindow, 8);
		}
	}
}

void AGOSEngine_PN::clearCursor(WindowBlock *window) {
	if (isPnAmigaInputWindow(window)) {
		const Graphics::AmigaFont *font = getPnAmigaFont();
		PnAmigaTextPlane *plane = getPnAmigaTextPlane(window);
		if (plane != nullptr && plane->pixels != nullptr) {
			Graphics::Surface surface;
			surface.init(plane->width, plane->height, plane->width, plane->pixels, Graphics::PixelFormat::createFormatCLUT8());
			int16 drawOffset = 0;
			uint16 inkWidth = getPnAmigaGlyphAdvance(' ');
			if (font != nullptr && '_' >= font->getLoChar() && '_' <= font->getHiChar()) {
				drawOffset = font->getCharDrawOffset('_');
				inkWidth = font->getCharInkWidth('_');
			}
			const int16 cursorLeft = CLIP<int16>(window->textColumn + drawOffset, 0, plane->width);
			const int16 cursorRight = CLIP<int16>(cursorLeft + (int16)inkWidth, 0, plane->width);
			surface.fillRect(Common::Rect(cursorLeft, window->textRow,
				cursorRight, window->textRow + getPnAmigaGlyphHeight()), window->fillColor);
			compositePnAmigaTextPlane(window);
		}
		return;
	}

	byte oldTextColor = window->textColor;

	window->textColor = window->fillColor;
	windowPutChar(window, 128);
	window->textColor = oldTextColor;

	windowPutChar(window, 8);
}

bool AGOSEngine_PN::processSpecialKeys() {
	if (shouldQuit())
		_exitCutscene = true;

	switch (_action) {
	case kActionExitCutscene:
		_exitCutscene = true;
		break;
	case kActionPause:
		pause();
		break;
	default:
		break;
	}

	_keyPressed.reset();
	return false;
}

} // End of namespace AGOS
