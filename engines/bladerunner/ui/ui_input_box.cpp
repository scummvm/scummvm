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

#include "bladerunner/ui/ui_input_box.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/font.h"
#include "bladerunner/time.h"

#include "common/keyboard.h"
#include "common/system.h"

#include "graphics/surface.h"

namespace BladeRunner {

UIInputBox::UIInputBox(BladeRunnerEngine *vm, UIComponentCallback *valueChangedCallback, void *callbackData, Common::Rect rect, int maxLength, const Common::String &text)
	: UIComponent(vm) {
	_valueChangedCallback = valueChangedCallback;
	_callbackData = callbackData;

	_isVisible = true;
	_rect = rect;
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	_maxLength = maxLength;
	setText(text);

	_cursorIsVisible = false;
	_timeLast = _vm->_time->currentSystem();
}

void UIInputBox::draw(Graphics::Surface &surface) {
	if (!_isVisible) {
		return;
	}

	int rectHalfWidth = (_rect.right + _rect.left) / 2;
	int textHalfWidth = _vm->_mainFont->getStringWidth(_text) / 2;

	_vm->_mainFont->drawString(&surface, _text, rectHalfWidth - textHalfWidth, _rect.top, surface.w, surface.format.RGBToColor(152, 112, 56));

	if (_cursorIsVisible) {
		surface.vLine(textHalfWidth + rectHalfWidth + 2, _rect.top, _rect.bottom - 1, surface.format.RGBToColor(248, 240, 232));
	}

	if (_vm->_time->currentSystem() - _timeLast > 500) {
		_timeLast = _vm->_time->currentSystem();
		_cursorIsVisible = !_cursorIsVisible;
	}
}

void UIInputBox::setText(const Common::String &text) {
	_text = text;
}

const Common::String &UIInputBox::getText() {
	return _text;
}

void UIInputBox::show() {
	_isVisible = true;
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
}

void UIInputBox::hide() {
	_isVisible = false;
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void UIInputBox::handleKeyDown(const Common::KeyState &kbd) {
	if (_isVisible) {
		uint8 kc = 0;
		if (getValidChar(kbd.ascii, kc) && _text.size() < _maxLength) {
			_text += kc;
		} else if (kbd.keycode == Common::KEYCODE_BACKSPACE) {
			_text.deleteLastChar();
		}
	}
}

void UIInputBox::handleCustomEventStart(const Common::Event &evt) {
	if (_isVisible
	    && evt.customType == BladeRunnerEngine::BladeRunnerEngineMappableAction::kMpConfirmDlg
	    && !_text.empty()
	    && _valueChangedCallback) {
		_valueChangedCallback(_callbackData, this);
	}
}

bool UIInputBox::getValidChar(const uint16 &kAscii16bit, uint8 &targetAscii) {
	if (kAscii16bit != 0) {
		// The above check for kAscii16bit > 0 gets rid of the tentative warning:
		// "Adding \0 to String. This is permitted, but can have unwanted consequences."
		// which was triggered by the .encode(Common::kDos850) operation below.
		//
		// The values that the KeyState::ascii field receives from the SDL backend are actually ISO 8859-1 encoded. They need to be
		// reencoded to DOS so as to match the game font encoding (although we currently use UIInputBox::charIsValid() to block most
		// extra characters, so it might not make much of a difference).
		targetAscii = (uint8)(Common::U32String(Common::String::format("%c", kAscii16bit), Common::kISO8859_1).encode(Common::kDos850).firstChar());
		return charIsValid(targetAscii);
	}
	return false;
}

bool UIInputBox::charIsValid(const uint8 &kc) {
	// The in-game font for text input is KIA6PT which follows IBM PC Code page 437 (CCSID 437)
	// This code page is identical to Code page 850 for the first 128 codes.
	// This method is:
	// 1) Filtering out characters not allowed in a DOS filename.
	//    Note, however, that it does allow ',', '.', ';', '=', '[' and ']'
	//    TODO Is that a bug?
	// 2) Allowing codes for glyphs that exist in KIA6PT up to code 0xA8 (glyph '¿')
	//    and also the extra codes for 0xAD (glyph '¡') and 0xE1 (glyph 'ß')
	//    (in order for these extra extended ASCII codes to be included,
	//     the comparisons in the return clause should be between uint values).
	// 3) Additionally disallows the '\x7F' character which caused a glyph '⊐' to be printed
	//    when the Delete key was pressed with no saved game selected,
	//    ie. the highlighted line on the KIA save screen is "<< NEW SLOT >>".
	//    The original does not show this glyph either but seems to filter the key earlier (not in this method).
	//    It's more effective to completely block the glyph in this method, though.
	return kc >= ' '
		&& kc != '<'
		&& kc != '>'
		&& kc != ':'
		&& kc != '"'
		&& kc != '/'
		&& kc != '\\'
		&& kc != '|'
		&& kc != '?'
		&& kc != '*'
		&& kc != (uint8)'\x7F'
		&& (kc <= (uint8)'\xA8' || kc == (uint8)'\xAD' || kc == (uint8)'\xE1');
}

} // End of namespace BladeRunner
