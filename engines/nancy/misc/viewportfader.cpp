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

// Each 1/30 of a second, the color keeps this much of its distance to the target
static const float kBlendRate = 0.9f;

static byte toByte(float channel) {
	return (byte)(CLIP(channel, 0.0f, 1.0f) * 255 + 0.5f);
}

void ViewportFader::init() {
	Common::Rect bounds = NancySceneState.getViewport().getScreenPosition();
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
	_drawSurface.clear(0);
	moveTo(bounds);
	setVisible(false);
}

void ViewportFader::arm(uint32 color, uint32 holdTime) {
	_target.a = 1.0f;
	_target.r = ((color >> 16) & 0xFF) / 255.0f;
	_target.g = ((color >> 8) & 0xFF) / 255.0f;
	_target.b = (color & 0xFF) / 255.0f;
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
bool ViewportFader::blendTowards(const Color &target, float seconds) {
	float amount = 1.0f - powf(kBlendRate, 30.0f * seconds);
	_current.a += (target.a - _current.a) * amount;
	_current.r += (target.r - _current.r) * amount;
	_current.g += (target.g - _current.g) * amount;
	_current.b += (target.b - _current.b) * amount;

	return toByte(_current.a) == toByte(target.a) && toByte(_current.r) == toByte(target.r) &&
		toByte(_current.g) == toByte(target.g) && toByte(_current.b) == toByte(target.b);
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
		if (blendTowards(Color(), seconds)) {
			_state = kIdle;
		}

		break;
	default:
		break;
	}

	redraw();
}

void ViewportFader::redraw() {
	byte alpha = toByte(_current.a);
	setVisible(alpha != 0);
	if (alpha != 0) {
		_drawSurface.clear(_drawSurface.format.ARGBToColor(alpha, toByte(_current.r), toByte(_current.g), toByte(_current.b)));
	}

	_needsRedraw = true;
}

} // End of namespace Misc
} // End of namespace Nancy
