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
#include "got/game/move.h"
#include "got/game/object.h"
#include "got/gfx/image.h"
#include "got/vars.h"

namespace Got {
namespace Views {

void GameContent::draw() {
	GfxSurface s;
	if (_shakeDelta.x != 0 || _shakeDelta.y != 0) {
		s.create(320, 192);
	} else {
		s = getSurface();
	}
	s.clear();

	drawBackground(s);
	drawObjects(s);
	drawEnemies(s, &_G(actor)[MAX_ACTORS - 1]);

	// If we're shaking the screen, render the content with the shake X/Y
	if (_shakeDelta.x != 0 || _shakeDelta.y != 0) {
		GfxSurface win = getSurface();
		win.blitFrom(s, _shakeDelta);
	}
}

bool GameContent::msgGame(const GameMessage &msg) {
	if (msg._name == "SHOW_LEVEL") {
		show_level(msg._value);
		return true;
	} else if (msg._name == "HIDE_ACTORS") {
		// Hide all actors and immediately redraw the screen
		for (int i = 0; i < MAX_ACTORS; i++)
			_G(actor)[i].show = 0;
		draw();
		return true;
	}

	return false;
}

bool GameContent::tick() {
	checkThunderShake();
	checkSwitchFlag();
	checkForItem();

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

void GameContent::checkThunderShake() {
	if (_G(thunder_flag)) {
		// Introduce a random screen shake by rendering screen 1 pixel offset randomly
		static const int8 DELTA_X[4] = { -1, 1, 0, 0 };
		static const int8 DELTA_Y[4] = { 0, 0, -1, 1 };
		int delta = g_events->getRandomNumber(3);

		_shakeDelta.x = DELTA_X[delta];
		_shakeDelta.y = DELTA_Y[delta];

		_G(thunder_flag)--;
		if ((_G(thunder_flag) < MAX_ACTORS) && _G(thunder_flag) > 2) {
			if (_G(actor)[_G(thunder_flag)].used) {
				_G(actor)[_G(thunder_flag)].vunerable = 0;
				actor_damaged(&_G(actor)[_G(thunder_flag)], 20);
			}
		}

		redraw();

	} else if (_shakeDelta.x != 0 || _shakeDelta.y != 0) {
		_shakeDelta = Common::Point(0, 0);
		redraw();
	}
}

void GameContent::checkSwitchFlag() {
	if (_G(switch_flag)) {
		switch (_G(switch_flag)) {
		case 1:
			switch_icons();
			break;
		case 2:
			rotate_arrows();
			break;
		default:
			break;
		}

		_G(switch_flag) = 0;
	}
}

void GameContent::checkForItem() {
	int thor_pos = ((_G(thor)->x + 7) / 16) + (((_G(thor)->y + 8) / 16) * 20);
	if (_G(object_map)[thor_pos])
		pick_up_object(thor_pos);
}

} // namespace Views
} // namespace Got
