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
	if (kbd.ascii != 0) {
		// The above check for kbd.ascii > 0 gets rid of the tentative warning:
		// "Adding \0 to String. This is permitted, but can have unwanted consequences."
		// which was triggered by the .encode(Common::kDos850) operation below.
		//
		// The values that the KeyState::ascii field receives from the SDL backend are actually ISO 8859-1 encoded. They need to be
		// reencoded to DOS so as to match the game font encoding (although we currently use UIInputBox::charIsValid() to block most
		// extra characters, so it might not make much of a difference).
		char kc = Common::U32String(Common::String::format("%c", kbd.ascii), Common::kISO8859_1).encode(Common::kDos850).firstChar();
		if (_isVisible) {
			if (charIsValid(kc) && _text.size() < _maxLength) {
				_text += kc;
			} else if (kbd.keycode == Common::KEYCODE_BACKSPACE) {
				_text.deleteLastChar();
			} else if (kbd.keycode == Common::KEYCODE_RETURN && !_text.empty()) {
				if (_valueChangedCallback) {
					_valueChangedCallback(_callbackData, this);
				}
			}
		}
	}
}

bool UIInputBox::charIsValid(char kc) {
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
		&& kc <= '~';// || kc == '¡' || kc == 'ß');
}

} // End of namespace BladeRunner
