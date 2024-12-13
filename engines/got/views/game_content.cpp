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

#include "got/views/game_content.h"
#include "got/vars.h"

namespace Got {
namespace Views {

void GameContent::draw() {
	GfxSurface s = getSurface();
	s.clear();

	int save_d;

	_G(boss_active) = 0;
	if (!_G(shield_on))
		_G(actor)[2].used = 0;
	_G(bomb_flag) = 0;

	save_d = _G(thor)->dir;
	if (_G(scrn).icon[_G(thor)->center_y][_G(thor)->center_x] == 154)
		_G(thor)->dir = 0;

	drawBackground(s);
	drawObjects(s);
}

void GameContent::drawBackground(GfxSurface &s) {
	for (int y = 0; y < TILES_Y; y++) {
		for (int x = 0; x < TILES_X; x++) {
			if (_G(scrn).icon[y][x] != 0) {
				const Common::Point pt(x * TILE_SIZE, y * TILE_SIZE);
				s.blitFrom(_G(bgPics)[_G(scrn).bg_color], pt);
				s.blitFrom(_G(bgPics)[_G(scrn).icon[y][x]], pt);
			}
		}
	}
}

void GameContent::drawObjects(GfxSurface &s) {
	int i, p;

	Common::fill(_G(object_map), _G(object_map) + TILES_COUNT, 0);
	Common::fill(_G(object_index), _G(object_index) + TILES_COUNT, 0);

	for (i = 0; i < OBJECTS_COUNT; i++) {
		if (_G(scrn).static_obj[i]) {
			s.blitFrom(_G(objects)[_G(scrn).static_obj[i] - 1],
				Common::Point(_G(scrn).static_x[i] * TILE_SIZE,
					_G(scrn).static_y[i] * TILE_SIZE));

			p = _G(scrn).static_x[i] + (_G(scrn).static_y[i] * TILES_X);
			_G(object_index)[p] = i;
			_G(object_map)[p] = _G(scrn).static_obj[i];
		}
	}
}

} // namespace Views
} // namespace Got
