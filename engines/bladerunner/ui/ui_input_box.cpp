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

#include "bladerunner/ui/ui_input_box.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/font.h"

#include "common/keyboard.h"

#include "graphics/surface.h"

namespace BladeRunner {

UIInputBox::UIInputBox(BladeRunnerEngine *vm, UIComponentCallback *valueChangedCallback, void *callbackData, Common::Rect rect, int maxLength, const Common::String &text)
	: UIComponent(vm) {
	_valueChangedCallback = valueChangedCallback;
	_callbackData = callbackData;

	_isVisible = true;
	_rect = rect;

	_maxLength = maxLength;
	setText(text);

	_cursorIsVisible = false;
	_timeLast = _vm->getTotalPlayTime(); // Original game is using system timer
}

void UIInputBox::draw(Graphics::Surface &surface) {
	if (!_isVisible) {
		return;
	}

	int rectHalfWidth = (_rect.right + _rect.left) / 2;
	int textHalfWidth = _vm->_mainFont->getTextWidth(_text) / 2;

	_vm->_mainFont->drawColor(_text, surface, rectHalfWidth - textHalfWidth, _rect.top, 0x4DC7); // 10011 01110 00111

	if (_cursorIsVisible) {
		surface.vLine(textHalfWidth + rectHalfWidth + 2, _rect.top, _rect.bottom - 1, 0x7FDD); // 11111 11110 11101
	}

	if (_vm->getTotalPlayTime() - _timeLast > 500) { // Original game is using system timer
		_timeLast = _vm->getTotalPlayTime();
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
}

void UIInputBox::hide() {
	_isVisible = false;
}

void UIInputBox::handleKeyUp(const Common::KeyState &kbd) {
	if (_isVisible) {
		// Check for "Enter" in keyUp instead of in keyDown as keyDown is repeating characters
		// and that can screw up UX (which is not great in the original game either).
		if (kbd.keycode == Common::KEYCODE_RETURN && !_text.empty()) {
			if (_valueChangedCallback) {
				_valueChangedCallback(_callbackData, this);
			}
		}
	}
}

void UIInputBox::handleKeyDown(const Common::KeyState &kbd) {
	if (_isVisible) {
		if (charIsValid(kbd) && _text.size() < _maxLength) {
			_text += kbd.ascii;
		} else if (kbd.keycode == Common::KEYCODE_BACKSPACE) {
			_text.deleteLastChar();
		}
	}
}

bool UIInputBox::charIsValid(const Common::KeyState &kbd) {
	return kbd.ascii >= ' '
		&& kbd.ascii != '<'
		&& kbd.ascii != '>'
		&& kbd.ascii != ':'
		&& kbd.ascii != '"'
		&& kbd.ascii != '/'
		&& kbd.ascii != '\\'
		&& kbd.ascii != '|'
		&& kbd.ascii != '?'
		&& kbd.ascii != '*'
		&& kbd.ascii <= '~';// || kbd.ascii == '¡' || kbd.ascii == 'ß');
}

} // End of namespace BladeRunner
