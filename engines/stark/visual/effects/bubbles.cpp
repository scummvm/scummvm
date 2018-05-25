/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/visual/effects/bubbles.h"

#include "common/random.h"
#include "common/tokenizer.h"

#include "graphics/surface.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/surfacerenderer.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/settings.h"

namespace Stark {

VisualEffectBubbles::VisualEffectBubbles(Gfx::Driver *gfx, const Common::Point &size) :
		VisualEffect(TYPE, size, gfx),
		_bubbleCount(50),
		_kind(kSmall),
		_sourcePositionRatioX(50),
		_maxVerticalSpeed(3),
		_maxHorizontalSpeed(1),
		_mainColorR(128),
		_mainColorG(128),
		_mainColorB(180),
		_mainColor(0),
		_darkColor(0) {
}

VisualEffectBubbles::~VisualEffectBubbles() {
}

void VisualEffectBubbles::render(const Common::Point &position) {
	// Stop rendering if special effect is off
	if (!StarkSettings->getBoolSetting(Settings::kSpecialFX)) return;

	_timeRemainingUntilNextUpdate -= StarkGlobal->getMillisecondsPerGameloop();
	if (_timeRemainingUntilNextUpdate <= 0) {
		update();
		_timeRemainingUntilNextUpdate = _timeBetweenTwoUpdates;
	}

	// Fill with transparent color
	_surface->fillRect(Common::Rect(_surface->w, _surface->h), 0);

	for (uint i = 0; i < _bubbles.size(); i++) {
		drawBubble(_bubbles[i]);
	}

	_texture->update(_surface);
	_surfaceRenderer->render(_texture, position);
}

void VisualEffectBubbles::setParams(const Common::String &params) {
	Common::StringTokenizer tokenizer(params, "(), ");

	// Example input: GFX_Bubbles( 1, 25, 3, 50, 3, 2, (50, 50, 75))

	int index = 0;
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		switch (index) {
		case 0:
			if (token != "GFX_Bubbles") {
				error("Unexpected effect type '%s'", token.c_str());
			}
			break;
		case 1:
			_timeBetweenTwoUpdates = CLIP<uint>(strtol(token.c_str(), nullptr, 10), 0, 1000) * 33;
			break;
		case 2:
			_bubbleCount = CLIP<uint>(strtol(token.c_str(), nullptr, 10), 1, 300);
			break;
		case 3:
			_kind = static_cast<Kind>(CLIP<int>(strtol(token.c_str(), nullptr, 10), 1, 3));
			break;
		case 4:
			_sourcePositionRatioX = CLIP<int>(strtol(token.c_str(), nullptr, 10), 0, 100);
			break;
		case 5:
			_maxVerticalSpeed = CLIP<int>(strtol(token.c_str(), nullptr, 10), 1, 5);
			break;
		case 6:
			_maxHorizontalSpeed = CLIP<int>(strtol(token.c_str(), nullptr, 10), 1, 10);
			break;
		case 7:
			_mainColorR = strtol(token.c_str(), nullptr, 10);
			break;
		case 8:
			_mainColorG = strtol(token.c_str(), nullptr, 10);
			break;
		case 9:
			_mainColorB = strtol(token.c_str(), nullptr, 10);
			break;
		default:
			warning("Unexpected parameter %d: %s", index, token.c_str());
		}

		index++;
	}

	_sourcePosition.x = _size.x * _sourcePositionRatioX / 100;
	_sourcePosition.y = _size.y;

	_mainColor = _surface->format.RGBToColor(_mainColorR, _mainColorG, _mainColorB);

	byte darkColorR =  3 * (_mainColorR >> 2);
	byte darkColorG =  3 * (_mainColorG >> 2);
	byte darkColorB =  3 * (_mainColorB >> 2);
	_darkColor = _surface->format.RGBToColor(darkColorR, darkColorG, darkColorB);

	_bubbles.resize(_bubbleCount);
	for (uint i = 0; i < _bubbles.size(); i++) {
		Bubble &bubble = _bubbles[i];
		bubble.position.x = -1;
		bubble.position.y = -1;

		if (_kind == kRandom) {
			bubble.kind = (StarkRandomSource->getRandomNumber(255) & 3) ? kLarge : kSmall;
		} else {
			bubble.kind = _kind;
		}
	}
}

void VisualEffectBubbles::update() {
	for (uint i = 0; i < _bubbles.size(); i++) {
		Bubble &bubble = _bubbles[i];

		if (bubble.position.x == -1 && bubble.position.y == -1) {
			bubble.position = _sourcePosition;
			break;
		}

		if (bubble.position.y <= 1) {
			bubble.position = _sourcePosition;
		} else {
			uint deltaUp = StarkRandomSource->getRandomNumberRng(1, _maxVerticalSpeed);
			int deltaLeft =  (bubble.position.x > _maxHorizontalSpeed)           * StarkRandomSource->getRandomNumberRng(0, _maxHorizontalSpeed);
			int deltaRight = (bubble.position.x < _size.x - _maxHorizontalSpeed) * StarkRandomSource->getRandomNumberRng(0, _maxHorizontalSpeed);

			bubble.position.x += deltaRight - deltaLeft;
			bubble.position.y -= deltaUp;
		}
	}
}

void VisualEffectBubbles::drawBubble(const Bubble &bubble) const {
	if (bubble.position.x == -1 && bubble.position.y == -1) {
		return;
	}

	if (bubble.kind == kSmall) {
		drawSmallBubble(bubble);
	} else {
		drawLargeBubble(bubble);
	}
}

void VisualEffectBubbles::drawSmallBubble(const Bubble &bubble) const {
	if (bubble.position.x < 0 || bubble.position.x >= _surface->w
	    || bubble.position.y < 0 || bubble.position.y >= _surface->h) {
			return;
	}

	uint32 *pixel = static_cast<uint32 *>(_surface->getBasePtr(bubble.position.x, bubble.position.y));
	*pixel = _mainColor;
}

void VisualEffectBubbles::drawLargeBubble(const Bubble &bubble) const {
	if (bubble.position.x < 1 || bubble.position.x >= _surface->w - 1
	    || bubble.position.y < 1 || bubble.position.y >= _surface->h - 1) {
		return;
	}

	uint32 *pixel = static_cast<uint32 *>(_surface->getBasePtr(bubble.position.x, bubble.position.y - 1));
	*pixel = _darkColor;

	pixel = static_cast<uint32 *>(_surface->getBasePtr(bubble.position.x - 1, bubble.position.y));
	*pixel++ = _darkColor;
	*pixel++ = _mainColor;
	*pixel = _darkColor;

	pixel = static_cast<uint32 *>(_surface->getBasePtr(bubble.position.x, bubble.position.y + 1));
	*pixel = _darkColor;
}

} // End of namespace Stark
