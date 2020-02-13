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
#include "bladerunner/game_constants.h"
#include "bladerunner/game_info.h"
#include "bladerunner/shape.h"
#include "bladerunner/time.h"
#include "bladerunner/ui/kia.h"

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
		_frame = 5u;
	} else {
		_frame = 0u;
	}

	_timeLast = _vm->_time->currentSystem();
	_rect = rect;
	_isChecked = isChecked;
}


void UICheckBox::draw(Graphics::Surface &surface) {
	if (_rect.right > _rect.left && _rect.bottom > _rect.top) {
		uint32 shapeId;

		uint32 timeNow = _vm->_time->currentSystem();
		// unsigned difference is intentional
		if (timeNow - _timeLast > 67u) {
			// unsigned difference is intentional
			uint32 frameDelta = (timeNow - _timeLast) / 67u;
			_timeLast = timeNow;

			if (_isChecked) {
				_frame = MIN<uint32>(_frame + frameDelta, 5u);
			} else {
				_frame = (_frame < frameDelta) ? 0 : MAX<uint32>(_frame - frameDelta, 0u);
			}
		}

		if (_style) {
			if (_frame || (_hasFocus && !_isPressed && _isEnabled)) {
				if (_frame != 5u || (_hasFocus && !_isPressed && _isEnabled)) {
					shapeId = _frame + 54u;
				} else {
					shapeId = 53u;
				}
			} else {
				shapeId = 52u;
			}
		} else {
			if (_frame || (_hasFocus && !_isPressed && _isEnabled)) {
				if (_frame != 5u || (_hasFocus && !_isPressed && _isEnabled)) {
					shapeId = _frame + 62u;
				} else {
					shapeId = 61u;
				}
			} else {
				shapeId = 60u;
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
			_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxTEXT3), 100, 0, 0, 50, 0);
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
			_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBEEP10), 100, 0, 0, 50, 0);
		} else {
			_isPressed = true;
		}
	}
}

void UICheckBox::handleMouseUp(bool alternateButton) {
	_isPressed = false;
}

} // End of namespace BladeRunner
