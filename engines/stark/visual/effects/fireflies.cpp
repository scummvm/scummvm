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

#include "engines/stark/visual/effects/fireflies.h"

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

VisualEffectFireFlies::VisualEffectFireFlies(Gfx::Driver *gfx, const Common::Point &size) :
		VisualEffect(TYPE, size, gfx),
		_fireFlyCount(10),
		_mainColorR(44),
		_mainColorG(20),
		_mainColorB(33) {
}

VisualEffectFireFlies::~VisualEffectFireFlies() {
}

void VisualEffectFireFlies::render(const Common::Point &position) {
	// Stop rendering if special effect is off
	if (!StarkSettings->getBoolSetting(Settings::kSpecialFX)) return;

	_timeRemainingUntilNextUpdate -= StarkGlobal->getMillisecondsPerGameloop();
	if (_timeRemainingUntilNextUpdate <= 0) {
		update();
		_timeRemainingUntilNextUpdate = _timeBetweenTwoUpdates;
	}

	// Fill with transparent color
	_surface->fillRect(Common::Rect(_surface->w, _surface->h), 0);

	for (uint i = 0; i < _fireFlies.size(); i++) {
		drawFireFly(_fireFlies[i]);
	}

	_texture->update(_surface);
	_surfaceRenderer->render(_texture, position);
}

void VisualEffectFireFlies::setParams(const Common::String &params) {
	// Example input: GFX_FireFlies( 2, 70 )
	Common::StringTokenizer tokenizer(params, "(), ");

	int index = 0;
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		switch (index) {
		case 0:
			if (token != "GFX_FireFlies") {
				error("Unexpected effect type '%s'", token.c_str());
			}
			break;
		case 1:
			_timeBetweenTwoUpdates = CLIP<uint>(strtol(token.c_str(), nullptr, 10), 0, 1000) * 33;
			break;
		case 2:
			_fireFlyCount = CLIP<uint>(strtol(token.c_str(), nullptr, 10), 1, 300);
			break;
		case 3:
			_mainColorR = strtol(token.c_str(), nullptr, 10);
			break;
		case 4:
			_mainColorG = strtol(token.c_str(), nullptr, 10);
			break;
		case 5:
			_mainColorB = strtol(token.c_str(), nullptr, 10);
			break;
		default:
			warning("Unexpected parameter %d: %s", index, token.c_str());
		}

		index++;
	}

	_frames.resize(40);
	for (uint i = 0; i < _frames.size(); i++) {
		Frame &frame = _frames[i];

		// Barycentric coordinates
		float t = (cos((_frames.size() - i) / (float)_frames.size() * 2.1415f + 0.5f) + 1.0f) * 0.5f;
		frame.weight1 = (1.0f - t) * (1.0f - t) * (1.0f - t) / 6.0f;
		frame.weight2 = (t * t * t * 3.0f - t * t * 6.0f + 4.0f) / 6.0f;
		frame.weight3 = (((3.0f - t * 3.0f) * t + 3.0f) * t + 1.0f) / 6.0f;
		frame.weight4 = t * t * t / 6.0f;

		int green;
		if (i < 5) {
			green = _mainColorG + (255 - _mainColorG) * (5 - i) / 5;
		} else if (i + 4 >= _frames.size()) {
			green = _mainColorG + (255 - _mainColorG) * (i + 4 - _frames.size()) / 5;
		} else {
			green = _mainColorG;
		}
		frame.color = _surface->format.RGBToColor(_mainColorR, green, _mainColorB);
	}

	_fireFlies.resize(_fireFlyCount);
	for (uint i = 0; i < _fireFlies.size(); i++) {
		FireFly &fireFly = _fireFlies[i];

		fireFly.point1.x = StarkRandomSource->getRandomNumber(_size.x - 1);
		fireFly.point1.y = StarkRandomSource->getRandomNumber(_size.y - 1);
		fireFly.point2.x = StarkRandomSource->getRandomNumber(_size.x - 1);
		fireFly.point2.y = StarkRandomSource->getRandomNumber(_size.y - 1);
		fireFly.point3.x = StarkRandomSource->getRandomNumber(_size.x - 1);
		fireFly.point3.y = StarkRandomSource->getRandomNumber(_size.y - 1);
		fireFly.point4.x = StarkRandomSource->getRandomNumber(_size.x - 1);
		fireFly.point4.y = StarkRandomSource->getRandomNumber(_size.y - 1);
		fireFly.currentFrame = StarkRandomSource->getRandomNumber(_frames.size() - 1);
	}
}

void VisualEffectFireFlies::update() {
	for (uint i = 0; i < _fireFlies.size(); i++) {
		FireFly &fireFly = _fireFlies[i];
		fireFly.currentFrame++;

		if (fireFly.currentFrame >= _frames.size()) {
			fireFly.currentFrame %= _frames.size();

			fireFly.point1 = fireFly.point2;
			fireFly.point2 = fireFly.point3;
			fireFly.point3 = fireFly.point4;
			fireFly.point4.x = StarkRandomSource->getRandomNumber(_size.x - 1);
			fireFly.point4.y = StarkRandomSource->getRandomNumber(_size.y - 1);
		}
	}

	for (uint i = 0; i < _fireFlies.size(); i++) {
		FireFly &fireFly = _fireFlies[i];
		const Frame &frame = _frames[fireFly.currentFrame];

		fireFly.currentPosition.x = fireFly.point1.x * frame.weight1 + fireFly.point2.x * frame.weight2
		                            + fireFly.point3.x * frame.weight3 + fireFly.point4.x * frame.weight4;
		fireFly.currentPosition.y = fireFly.point1.y * frame.weight1 + fireFly.point2.y * frame.weight2
		                            + fireFly.point3.y * frame.weight3 + fireFly.point4.y * frame.weight4;
	}
}

void VisualEffectFireFlies::drawFireFly(const FireFly &fly) {
	if (fly.currentPosition.x < 0 || fly.currentPosition.x >= _surface->w
	    || fly.currentPosition.y < 0 || fly.currentPosition.y >= _surface->h) {
		return;
	}

	uint32 *pixel = static_cast<uint32 *>(_surface->getBasePtr(fly.currentPosition.x, fly.currentPosition.y));
	*pixel = _frames[fly.currentFrame].color;
}

} // End of namespace Stark
