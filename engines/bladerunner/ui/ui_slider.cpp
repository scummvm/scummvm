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

#include "bladerunner/ui/ui_slider.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/game_info.h"
#include "bladerunner/game_constants.h"

namespace BladeRunner {

const Color256 UISlider::kColors[] = {
	{   0,   0,   0 }, // Black - unpressed (framing rectange)
	{  16,   8,   8 },
	{  32,  24,   8 },
	{  56,  32,  16 },
	{  72,  48,  16 },
	{  88,  56,  24 }, // Mouse-over (framing rectange)
	{ 104,  72,  32 },
	{ 128,  80,  40 },
	{ 136,  96,  48 },
	{ 152, 112,  56 },
	{ 168, 128,  72 }, // Pressed (framing rectange)
	{ 184, 144,  88 },
	{ 200, 160,  96 },
	{ 216, 184, 112 },
	{ 232, 200, 128 },
	{ 240, 224, 144 }
};

UISlider::UISlider(BladeRunnerEngine *vm, UIComponentCallback *valueChangedCallback, void *callbackData, Common::Rect rect, int maxValue, int value)
	: UIComponent(vm) {

	_valueChangedCallback = valueChangedCallback;
	_callbackData = callbackData;

	_maxValue = MAX(0, maxValue);
	_value = CLIP(value, 0, _maxValue - 1);

	_rect = rect;
	_isEnabled = true;
	_currentFrameColor = 0;
	_hasFocus = false;
	_pressedStatus = 0;
	_mouseX = 0;
}

void UISlider::draw(Graphics::Surface &surface) {
	if (_rect.isEmpty()) {
		return;
	}

	int frameColor;
	if (_pressedStatus == 1) {
		frameColor = 10;
	} else if (_hasFocus && _pressedStatus != 2 && _isEnabled) {
		frameColor = 5;
	} else {
		frameColor = 0;
	}

	// Ensures animated transition of the frame's (outlining rectangle's) color to the new one
	if (_currentFrameColor < frameColor) {
		++_currentFrameColor;
	}

	// Ensures animated transition of the frame's (outlining rectangle's) color to the new one
	if (_currentFrameColor > frameColor) {
		--_currentFrameColor;
	}

	surface.frameRect(_rect, surface.format.RGBToColor(kColors[_currentFrameColor].r, kColors[_currentFrameColor].g, kColors[_currentFrameColor].b));

	int sliderX = 0;
	if (_maxValue <= 1) {
		sliderX = _rect.left;
	} else {
		sliderX = _rect.left + ((_value * _rect.width()) / (_maxValue - 1));
	}

	if (_pressedStatus == 1) {
		int sliderValue = ((_maxValue - 1) * (_mouseX - _rect.left)) / _rect.width();
		sliderX = _rect.left + ((sliderValue * _rect.width()) / (_maxValue - 1));
		sliderX = CLIP(sliderX, (int)_rect.left, (int)_rect.right);
	}

	if (_rect.left + 1 < _rect.right - 1) {
		int striding = _rect.left + sliderX;
		for (int x = _rect.left + 1; x < _rect.right - 1; ++x) {
			int colorIndex =  15 - (abs(sliderX - x) >> 2);

			if (!_isEnabled) {
				colorIndex /= 2;
			}

			if (colorIndex < 3) {
				colorIndex = 3;
			}

			uint32 color = surface.format.RGBToColor(kColors[colorIndex].r, kColors[colorIndex].g, kColors[colorIndex].b);
			if ((striding + x) & 1 || x == sliderX) {
				color = surface.format.RGBToColor(0, 0, 0);
			}

			surface.vLine(x, _rect.top + 1, _rect.bottom - 2, color);
		}
	}
}

void UISlider::handleMouseMove(int mouseX, int mouseY) {
	_mouseX = mouseX;
	if (_rect.contains(mouseX, mouseY)) {
		if (!_hasFocus && _isEnabled && _pressedStatus == 0) {
			_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxTEXT3), 100, 0, 0, 50, 0);
		}
		_hasFocus = true;
	} else {
		_hasFocus = false;
	}
}

void UISlider::handleMouseDown(bool alternateButton) {
	if (_isEnabled && !alternateButton) {
		if (_hasFocus) {
			_pressedStatus = 1;
		} else {
			_pressedStatus = 2;
		}
	}
}

void UISlider::handleMouseUp(bool alternateButton) {
	if (!alternateButton) {
		if (_pressedStatus == 1) {
			if (_rect.width() == 0) {
				_value = 0;
			} else {
				_value = ((_maxValue - 1) * (_mouseX - _rect.left)) / _rect.width();
			}
			_value = CLIP(_value, 0, _maxValue - 1);

			if (_valueChangedCallback) {
				_valueChangedCallback(_callbackData, this);
			}
		}
		_pressedStatus = 0;
	}
}

void UISlider::setValue(int value) {
	_value = CLIP(value, 0, _maxValue - 1);
}

} // End of namespace BladeRunner
