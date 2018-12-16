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

#include "bladerunner/ui/ui_check_box.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/game_info.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/kia_shapes.h"

namespace BladeRunner {

UICheckBox::UICheckBox(BladeRunnerEngine *vm, UIComponentCallback *valueChangedCallback, void *callbackData, Common::Rect rect, int style, bool isChecked)
	: UIComponent(vm) {

	_valueChangedCallback = valueChangedCallback;
	_callbackData = callbackData;

	_isEnabled = true;
	_hasFocus = false;
	_isPressed = false;
	_style = style;

	if (isChecked) {
		_frame = 5;
	} else {
		_frame = 0;
	}

	_timeLast = _vm->getTotalPlayTime(); // Original game is using system timer
	_rect = rect;
	_isChecked = isChecked;
}


void UICheckBox::draw(Graphics::Surface &surface) {
	int shapeId;
	if (_rect.right > _rect.left && _rect.bottom > _rect.top) {
		uint timeNow = _vm->getTotalPlayTime(); // Original game is using system timer
		if (timeNow - _timeLast > 67) {
			int frameDelta = (timeNow - _timeLast) / 67u;
			_timeLast = timeNow;

			if (_isChecked) {
				_frame = MIN(_frame + frameDelta, 5);
			} else {
				_frame = MAX(_frame - frameDelta, 0);
			}
		}

		if (_style) {
			if (_frame || (_hasFocus && !_isPressed && _isEnabled)) {
				if (_frame != 5 || (_hasFocus && !_isPressed && _isEnabled)) {
					shapeId = _frame + 54;
				} else {
					shapeId = 53;
				}
			} else {
				shapeId = 52;
			}
		} else {
			if (_frame || (_hasFocus && !_isPressed && _isEnabled)) {
				if (_frame != 5 || (_hasFocus && !_isPressed && _isEnabled)) {
					shapeId = _frame + 62;
				} else {
					shapeId = 61;
				}
			} else {
				shapeId = 60;
			}
		}

		_vm->_kia->_shapes->get(shapeId)->draw(surface, _rect.left, _rect.top + 1);
	}
}

void UICheckBox::enable() {
	_isEnabled = true;
	_isPressed = false;
	_hasFocus = false;
}

void UICheckBox::disable() {
	_isEnabled = false;
}

void UICheckBox::setChecked(bool isChecked) {
	_isChecked = isChecked;
}

void UICheckBox::handleMouseMove(int mouseX, int mouseY) {
	if (_rect.contains(mouseX, mouseY)) {
		if (!_hasFocus && _isEnabled && !_isPressed ) {
			_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(508), 100, 0, 0, 50, 0);
		}
		_hasFocus = true;
	} else {
		_hasFocus = false;
	}
}

void UICheckBox::handleMouseDown(bool alternateButton) {
	if (!alternateButton) {
		if (_isEnabled && _hasFocus) {
			_isChecked = !_isChecked;
			if (_valueChangedCallback) {
				_valueChangedCallback(_callbackData, this);
			}
			_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(509), 100, 0, 0, 50, 0);
		} else {
			_isPressed = true;
		}
	}
}

void UICheckBox::handleMouseUp(bool alternateButton) {
	_isPressed = false;
}

} // End of namespace BladeRunner
