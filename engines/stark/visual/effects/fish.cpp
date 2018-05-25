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

#include "engines/stark/visual/effects/fish.h"

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

VisualEffectFish::VisualEffectFish(Gfx::Driver *gfx, const Common::Point &size) :
		VisualEffect(TYPE, size, gfx),
		_fishCount(10),
		_currentFrame(0),
		_numFrames(20),
		_maxRadius(100),
		_fishPathWidthRatio(20),
		_fishPathHeightRatio(30),
		_fishPathWidth(0),
		_fishPathHeight(0),
		_masterPathWidth(0),
		_masterPathHeight(0),
		_mainColorR(44),
		_mainColorG(97),
		_mainColorB(133),
		_mainColor(0),
		_otherColor(0) {
}

VisualEffectFish::~VisualEffectFish() {
}

void VisualEffectFish::render(const Common::Point &position) {
	// Stop rendering if special effect is off
	if (!StarkSettings->getBoolSetting(Settings::kSpecialFX)) return;

	_timeRemainingUntilNextUpdate -= StarkGlobal->getMillisecondsPerGameloop();
	if (_timeRemainingUntilNextUpdate <= 0) {
		update();
		_timeRemainingUntilNextUpdate = _timeBetweenTwoUpdates;
	}

	// Fill with transparent color
	_surface->fillRect(Common::Rect(_surface->w, _surface->h), 0);

	for (uint i = 0; i < _fishList.size(); i++) {
		drawFish(_fishList[i]);
	}

	_texture->update(_surface);
	_surfaceRenderer->render(_texture, position);
}

void VisualEffectFish::setParams(const Common::String &params) {
	// Example input: GFX_Fish( 2, 30, 20, 30, (20, 50, 70), 20, 100 )
	Common::StringTokenizer tokenizer(params, "(), ");

	int index = 0;
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		switch (index) {
		case 0:
			if (token != "GFX_Fish") {
				error("Unexpected effect type '%s'", token.c_str());
			}
			break;
		case 1:
			_timeBetweenTwoUpdates = CLIP<uint>(strtol(token.c_str(), nullptr, 10), 0, 1000) * 33;
			break;
		case 2:
			_fishCount = CLIP<uint>(strtol(token.c_str(), nullptr, 10), 1, 300);
			break;
		case 3:
			_fishPathWidthRatio = CLIP<int>(strtol(token.c_str(), nullptr, 10), 0, 100);
			break;
		case 4:
			_fishPathHeightRatio = CLIP<int>(strtol(token.c_str(), nullptr, 10), 0, 100);
			break;
		case 5:
			_mainColorR = strtol(token.c_str(), nullptr, 10);
			break;
		case 6:
			_mainColorG = strtol(token.c_str(), nullptr, 10);
			break;
		case 7:
			_mainColorB = strtol(token.c_str(), nullptr, 10);
			break;
		case 8:
			_numFrames = CLIP<uint>(strtol(token.c_str(), nullptr, 10), 10, 100);
			break;
		case 9:
			_maxRadius = CLIP<uint>(strtol(token.c_str(), nullptr, 10), 30, 200);
			break;
		default:
			warning("Unexpected parameter %d: %s", index, token.c_str());
		}

		index++;
	}

	_mainColor = _surface->format.RGBToColor(_mainColorR, _mainColorG, _mainColorB);
	_otherColor = _surface->format.RGBToColor(
			_mainColorR + (255 - _mainColorR) / 4,
			_mainColorG + (255 - _mainColorG) / 4,
			_mainColorB + (255 - _mainColorB) / 3
	);

	_fishPathWidth =  _size.x * _fishPathWidthRatio  / 100;
	_fishPathHeight = _size.y * _fishPathHeightRatio / 100;
	_masterPathWidth =  _size.x - _fishPathWidth;
	_masterPathHeight = _size.y - _fishPathHeight;

	_numFrames = 20; // The original also ignores the parameter
	_frames.resize(_numFrames);
	for (uint i = 0; i < _frames.size(); i++) {
		Frame &frame = _frames[i];

		// Barycentric coordinates
		float t = (i + 1) / (float)_frames.size();
		frame.weight1 = (1.0f - t) * (1.0f - t) * (1.0f - t) / 6.0f;
		frame.weight2 = (t * t * t * 3.0f - t * t * 6.0f + 4.0f) / 6.0f;
		frame.weight3 = (((3.0f - t * 3.0f) * t + 3.0f) * t + 1.0f) / 6.0f;
		frame.weight4 = t * t * t / 6.0f;
	}

	_fishList.resize(_fishCount);
	for (uint i = 0; i < _fishList.size(); i++) {
		Fish &fish = _fishList[i];

		fish.point1.x = StarkRandomSource->getRandomNumber(_fishPathWidth - 1);
		fish.point1.y = StarkRandomSource->getRandomNumber(_fishPathHeight - 1);
		fish.point2.x = StarkRandomSource->getRandomNumber(_fishPathWidth - 1);
		fish.point2.y = StarkRandomSource->getRandomNumber(_fishPathHeight - 1);
		fish.point3.x = StarkRandomSource->getRandomNumber(_fishPathWidth - 1);
		fish.point3.y = StarkRandomSource->getRandomNumber(_fishPathHeight - 1);
		fish.point4.x = StarkRandomSource->getRandomNumber(_fishPathWidth - 1);
		fish.point4.y = StarkRandomSource->getRandomNumber(_fishPathHeight - 1);
	}

	_masterPath.point1.x = StarkRandomSource->getRandomNumber(_masterPathWidth - 1);
	_masterPath.point1.y = StarkRandomSource->getRandomNumber(_masterPathHeight - 1);
	_masterPath.point2 = _masterPath.point1;
	_masterPath.point3 = _masterPath.point1;
	_masterPath.point4 = _masterPath.point1;
}

void VisualEffectFish::update() {
	_currentFrame++;
	if (_currentFrame >= _frames.size()) {
		_currentFrame %= _frames.size();

		for (uint i = 0; i < _fishList.size(); i++) {
			Fish &fish = _fishList[i];
			fish.point1 = fish.point2;
			fish.point2 = fish.point3;
			fish.point3 = fish.point4;
			fish.point4.x = StarkRandomSource->getRandomNumber(_fishPathWidth - 1);
			fish.point4.y = StarkRandomSource->getRandomNumber(_fishPathHeight - 1);
		}

		_masterPath.point1 = _masterPath.point2;
		_masterPath.point2 = _masterPath.point3;
		_masterPath.point3 = _masterPath.point4;

		uint radius;
		do {
			_masterPath.point4.x = StarkRandomSource->getRandomNumber(_masterPathWidth - 1);
			_masterPath.point4.y = StarkRandomSource->getRandomNumber(_masterPathHeight - 1);

			int deltaX = _masterPath.point4.x - _masterPath.point3.x;
			int deltaY = _masterPath.point4.y - _masterPath.point3.y;
			radius = sqrt(deltaX * deltaX + deltaY * deltaY);
		} while (radius > _maxRadius);
	}

	const Frame &frame = _frames[_currentFrame];

	_masterPath.currentPosition.x = _masterPath.point1.x * frame.weight1 + _masterPath.point2.x * frame.weight2
	                              + _masterPath.point3.x * frame.weight3 + _masterPath.point4.x * frame.weight4;
	_masterPath.currentPosition.y = _masterPath.point1.y * frame.weight1 + _masterPath.point2.y * frame.weight2
	                              + _masterPath.point3.y * frame.weight3 + _masterPath.point4.y * frame.weight4;

	for (uint i = 0; i < _fishList.size(); i++) {
		Fish &fish = _fishList[i];

		fish.previousPosition = fish.currentPosition;
		fish.currentPosition.x = fish.point1.x * frame.weight1 + fish.point2.x * frame.weight2
		                       + fish.point3.x * frame.weight3 + fish.point4.x * frame.weight4;
		fish.currentPosition.y = fish.point1.y * frame.weight1 + fish.point2.y * frame.weight2
		                       + fish.point3.y * frame.weight3 + fish.point4.y * frame.weight4;

		fish.currentPosition += _masterPath.currentPosition;
	}
}

void VisualEffectFish::drawFish(const Fish &fish) {
	if (fish.currentPosition.x < 0 || fish.currentPosition.x >= _surface->w
	    || fish.currentPosition.y < 0 || fish.currentPosition.y >= _surface->h) {
		return;
	}

	if (fish.previousPosition.x < 0 || fish.previousPosition.x >= _surface->w
	    || fish.previousPosition.y < 0 || fish.previousPosition.y >= _surface->h) {
		return;
	}

	int dot = (fish.previousPosition.x - fish.currentPosition.x)
	        * (fish.previousPosition.x - fish.currentPosition.x)
	        + (fish.previousPosition.y - fish.currentPosition.y)
	        * (fish.previousPosition.y - fish.currentPosition.y);

	uint color;
	if (dot >= 4) {
		color = _mainColor;
	} else {
		color = _otherColor;
	}
	_surface->drawLine(fish.previousPosition.x, fish.previousPosition.y,
	                   fish.currentPosition.x,  fish.currentPosition.y, color);
}

} // End of namespace Stark
