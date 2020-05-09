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

#include "backends/cloud/cloudicon.h"
#include "common/memstream.h"
#include "common/system.h"
#include "image/png.h"

namespace Cloud {

const float CloudIcon::ALPHA_SPEED = 0.0005f;
const float CloudIcon::ALPHA_MAX = 1.f;
const float CloudIcon::ALPHA_MIN = 0.6f;

CloudIcon::CloudIcon() {
	initIcons();
	hide();
	_lastUpdateTime = g_system->getMillis();
}

CloudIcon::~CloudIcon() {
	_icon.free();
	_disabledIcon.free();
	_alphaIcon.free();
}

void CloudIcon::show(CloudIcon::Type icon, int duration) {
	if (_type == icon) {
		return; // Nothing to do
	}

	if (icon != kNone) {
		_state = kShown;
		_type = icon;

		if (duration) {
			_hideTime = g_system->getMillis() + duration;
		} else {
			_hideTime = 0;
		}
	} else {
		_state = kGoingToHide;
	}
}

void CloudIcon::hide() {
	_state = kHidden;
	_type = kNone;
	_hideTime = 0;
	_currentAlpha = 0;
	_alphaRising = true;
}

CloudIcon::Type CloudIcon::getShownType() const {
	return _type;
}

bool CloudIcon::needsUpdate() const {
	uint32 delaySinceLastUpdate = g_system->getMillis() - _lastUpdateTime;
	return delaySinceLastUpdate >= UPDATE_DELAY_MIN_MILLIS;
}

void CloudIcon::update() {
	uint32 currentTime = g_system->getMillis();
	uint32 delaySinceLastUpdate = currentTime - _lastUpdateTime;
	_lastUpdateTime = currentTime;

	switch (_state) {
	default:
		// fallthrough intended
	case kHidden:
		return; // Nothing to do
	case kShown:
		if (_alphaRising) {
			if (_currentAlpha < ALPHA_MIN)
				_currentAlpha += 5 * ALPHA_SPEED * delaySinceLastUpdate;
			else
				_currentAlpha += ALPHA_SPEED * delaySinceLastUpdate;
			if (_currentAlpha > ALPHA_MAX) {
				_currentAlpha = ALPHA_MAX;
				_alphaRising = false;
			}
		} else {
			_currentAlpha -= ALPHA_SPEED * delaySinceLastUpdate;
			if (_currentAlpha < ALPHA_MIN) {
				_currentAlpha = ALPHA_MIN;
				_alphaRising = true;
			}
		}

		if (_hideTime != 0 && _hideTime <= currentTime) {
			_hideTime = 0;
			_state = kGoingToHide;
		}
		break;
	case kGoingToHide:
		_currentAlpha -= 5 * ALPHA_SPEED * delaySinceLastUpdate;
		if (_currentAlpha <= 0) {
			hide();
		}
		break;
	}

	if (!_icon.getPixels() || !_disabledIcon.getPixels()) {
		// Loading the icons failed. Don't try to draw them.
		return;
	}

	if (_state != kHidden) {
		makeAlphaIcon((_type == kDisabled ? _disabledIcon : _icon), _currentAlpha);
		g_system->displayActivityIconOnOSD(&_alphaIcon);
	} else {
		g_system->displayActivityIconOnOSD(nullptr);
	}
}

#include "backends/cloud/cloudicon_data.h"
#include "backends/cloud/cloudicon_disabled_data.h"

void CloudIcon::initIcons() {
	loadIcon(_icon, cloudicon_data, ARRAYSIZE(cloudicon_data));
	loadIcon(_disabledIcon, cloudicon_disabled_data, ARRAYSIZE(cloudicon_disabled_data));
}

void CloudIcon::loadIcon(Graphics::Surface &icon, byte *data, uint32 size) {
	Image::PNGDecoder decoder;
	Common::MemoryReadStream stream(data, size);
	if (!decoder.loadStream(stream)) {
		warning("CloudIcon::loadIcon: error decoding PNG");
		return;
	}

	const Graphics::Surface *s = decoder.getSurface();
	icon.copyFrom(*s);
}

void CloudIcon::makeAlphaIcon(const Graphics::Surface &icon, float alpha) {
	_alphaIcon.copyFrom(icon);

	byte *pixels = (byte *)_alphaIcon.getPixels();
	for (int y = 0; y < _alphaIcon.h; y++) {
		byte *row = pixels + y * _alphaIcon.pitch;
		for (int x = 0; x < _alphaIcon.w; x++) {
			uint32 srcColor;
			if (_alphaIcon.format.bytesPerPixel == 2)
				srcColor = READ_UINT16(row);
			else if (_alphaIcon.format.bytesPerPixel == 3)
				srcColor = READ_UINT24(row);
			else
				srcColor = READ_UINT32(row);

			// Update color's alpha
			byte r, g, b, a;
			_alphaIcon.format.colorToARGB(srcColor, a, r, g, b);
			a = (byte)(a * alpha);
			uint32 color = _alphaIcon.format.ARGBToColor(a, r, g, b);

			if (_alphaIcon.format.bytesPerPixel == 2)
				*((uint16 *)row) = color;
			else
				*((uint32 *)row) = color;

			row += _alphaIcon.format.bytesPerPixel;
		}
	}
}

} // End of namespace Cloud
