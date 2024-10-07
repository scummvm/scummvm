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

#include "darkseed/morph.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

Morph::Morph(const Common::Rect &area) : _area(area) {}
void Morph::loadSrcFromScreen() {
	_src.copyFrom(*(Graphics::ManagedSurface *)g_engine->_screen);
}

void Morph::loadDestFromScreen() {
	_dest.copyFrom(*(Graphics::ManagedSurface *)g_engine->_screen);
}

void Morph::start(MorphDirection direction) {
	_stepCount = 0;
	_direction = direction;
}

bool Morph::morphStep() {
	if (_stepCount > 16) {
		return false;
	}
	draw(_direction == MorphDirection::Forward ? _stepCount : 16 - _stepCount);
	_stepCount += 1;
	return _stepCount < 17;
}

void Morph::draw(int16 drawIdx) {
	uint8 *screen = (uint8 *)g_engine->_screen->getBasePtr(_area.left, _area.top);
	uint8 *src = (uint8 *)_src.getBasePtr(_area.left, _area.top);
	uint8 *dest = (uint8 *)_dest.getBasePtr(_area.left, _area.top);

	for (int y = 0; y < _area.height(); y++) {
		for (int x = 0; x < _area.width(); x++) {
			if (src[x] != dest[x]) {
				screen[x] = (src[x] * (16 - drawIdx) + dest[x] * drawIdx) >> 4;
			}
		}
		screen += g_engine->_screen->pitch;
		src += _src.pitch;
		dest += _dest.pitch;
	}
	g_engine->_screen->addDirtyRect(_area);
}

} // End of namespace Darkseed
