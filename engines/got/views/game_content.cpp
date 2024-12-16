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
#include "got/game/back.h"
#include "got/gfx/image.h"
#include "got/vars.h"

namespace Got {
namespace Views {

void GameContent::draw() {
	GfxSurface s = getSurface();
	s.clear();

	drawBackground(s);
	drawObjects(s);
	drawEnemies(s, &_G(actor)[MAX_ACTORS - 1]);
}

bool GameContent::msgGame(const GameMessage &msg) {
	if (msg._name == "SHOW_LEVEL") {
		show_level(msg._value);
		return true;
	}

	return false;
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

void GameContent::drawEnemies(GfxSurface &s, ACTOR *lastActor) {
	ACTOR *actor_ptr = lastActor;
	ACTOR *actor2_storage = nullptr;

	for (int actor_num = 0; actor_num <= MAX_ACTORS; ) {
		// Check for blinking flag
		if (!(actor_ptr->show & 2)) {
			actor_ptr->last_x[0] = actor_ptr->x;
			actor_ptr->last_y[0] = actor_ptr->y;

			const Graphics::ManagedSurface &frame = actor_ptr->pic[actor_ptr->dir]
				[actor_ptr->frame_sequence[actor_ptr->next]];
			s.blitFrom(frame, Common::Point(actor_ptr->x, actor_ptr->y));
		}

		// Move to the next actor
		do {
			--actor_ptr;
			++actor_num;

			if (actor_num == MAX_ACTORS)
				actor_ptr = actor2_storage;
			else if (actor_num == (MAX_ACTORS - 3))
				actor2_storage = actor_ptr;
		} while (actor_num == (MAX_ACTORS - 3));
	}
}

} // namespace Views
} // namespace Got
