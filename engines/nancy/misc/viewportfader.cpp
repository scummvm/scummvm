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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/misc/viewportfader.h"

namespace Nancy {
namespace Misc {

// How quickly the color moves toward the target; a lower rate is faster
static const float kBlendRate = 0.9f;

void ViewportFader::init() {
	Common::Rect bounds = NancySceneState.getViewport().getScreenPosition();
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
	_drawSurface.clear(0);
	moveTo(bounds);
	setVisible(false);
}

void ViewportFader::arm(uint32 color, uint32 holdTime) {
	_target = color | 0xFF000000;
	_holdTime = holdTime;

	_state = kArmed;
}

void ViewportFader::onSceneChange() {
	if (_state != kArmed) {
		return;
	}

	_state = kFadingIn;
	_lastUpdateTime = g_nancy->getTotalPlayTime();
	g_nancy->_cursor->showCursor(false);
}

// Returns true once the color has reached the target
bool ViewportFader::blendTowards(uint32 target, float seconds) {
	blendColor(_current, target, getBlendAmount(seconds, kBlendRate));
	return channelsToArgb(_current) == target;
}

void ViewportFader::updateGraphics() {
	if (_state == kIdle || _state == kArmed) {
		return;
	}

	uint32 now = g_nancy->getTotalPlayTime();
	float seconds = (now - _lastUpdateTime) / 1000.0f;
	_lastUpdateTime = now;

	switch (_state) {
	case kFadingIn:
		if (blendTowards(_target, seconds)) {
			_state = kHolding;
			_holdEndTime = now + _holdTime;
		}

		break;
	case kHolding:
		if (now > _holdEndTime) {
			_state = kFadingOut;
			g_nancy->_cursor->showCursor(true);
		}

		break;
	case kFadingOut:
		// The fade goes back to transparent black
		if (blendTowards(0, seconds)) {
			_state = kIdle;
		}

		break;
	default:
		break;
	}

	redraw();
}

void ViewportFader::redraw() {
	uint32 color = channelsToArgb(_current);
	bool isVisible = (color >> 24) != 0;
	setVisible(isVisible);
	if (isVisible) {
		_drawSurface.clear(argbToColor(_drawSurface.format, color));
	}

	_needsRedraw = true;
}

} // End of namespace Misc
} // End of namespace Nancy
