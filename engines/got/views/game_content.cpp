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
#include "got/game/move_patterns.h"
#include "got/game/object.h"
#include "got/gfx/image.h"
#include "got/vars.h"

namespace Got {
namespace Views {

#define SPIN_INTERVAL 4
#define SPIN_COUNT 20
#define DEATH_THRESHOLD (SPIN_COUNT * SPIN_INTERVAL)

GameContent::GameContent() : View("GameContent") {
	_surface.create(320, 192);
}

void GameContent::draw() {
	GfxSurface s;
	if (_G(gameMode) == MODE_THUNDER || _G(gameMode) == MODE_AREA_CHANGE) {
		s.create(320, 192);
	} else {
		s = getSurface();
	}
	s.clear();

	drawBackground(s);
	drawObjects(s);
	drawActors(s);

	// If we're shaking the screen, render the content with the shake X/Y
	if (_G(gameMode) == MODE_THUNDER) {
		GfxSurface win = getSurface();
		win.clear();
		win.blitFrom(s, _moveDelta);
	} else if (_G(gameMode) == MODE_AREA_CHANGE) {
		// Draw parts of the new scene along with parts of the old one
		// as it's scrolled off-screen
		GfxSurface win = getSurface();

		switch (_G(transitionDir)) {
		case DIR_LEFT:
			win.blitFrom(s, Common::Rect(320 - _transitionPos, 0, 320, 192),
				Common::Point(0, 0));
			win.blitFrom(_surface, Common::Rect(0, 0, 320 - _transitionPos, 192),
				Common::Point(_transitionPos, 0));
			break;
		case DIR_RIGHT:
			win.blitFrom(_surface, Common::Rect(_transitionPos, 0, 320, 192),
				Common::Point(0, 0));
			win.blitFrom(s, Common::Rect(0, 0, _transitionPos, 192),
				Common::Point(320 - _transitionPos, 0));
			break;
		case DIR_UP:
			win.blitFrom(s, Common::Rect(0, 192 - _transitionPos, 320, 192),
				Common::Point(0, 0));
			win.blitFrom(_surface, Common::Rect(0, 0, 320, 192 - _transitionPos),
				Common::Point(0, _transitionPos));
			break;
		case DIR_DOWN:
			win.blitFrom(_surface, Common::Rect(0, _transitionPos, 320, 192),
				Common::Point(0, 0));
			win.blitFrom(s, Common::Rect(0, 0, 320, _transitionPos),
				Common::Point(0, 192 - _transitionPos));
			break;
		case DIR_PHASED:
			win.blitFrom(_surface);		// Copy old surface

			// Copy the randomly chosen blocks over from new scene
			for (int i = 0; i < 240; ++i) {
				int x = (i * 16) % 320;
				int y = ((i * 16) / 320) * 16;
				if (_phased[i])
					win.blitFrom(s, Common::Rect(x, y, x + 16, y + 16), Common::Point(x, y));
			}
			break;
		default:
			break;
		}
	}
}

bool GameContent::msgGame(const GameMessage &msg) {
	if (msg._name == "THOR_DIES") {
		thorDies();
		return true;
	}

	return false;
}

bool GameContent::tick() {
	checkThunderShake();

	switch (_G(gameMode)) {
	case MODE_NORMAL:
		checkSwitchFlag();
		checkForItem();
		moveActors();
		use_item();
		updateActors();
		checkForCheats();
		break;

	case MODE_THOR_DIES:
		if (_deathCtr < DEATH_THRESHOLD) {
			spinThor();
		} else if (_deathCtr < DEATH_THRESHOLD + 60) {
			_G(thor)->used = 0;
			++_deathCtr;
		} else {
			thorDead();
		}
		break;

	default:
		break;
	}

	checkForAreaChange();

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

void GameContent::drawActors(GfxSurface &s) {
	ACTOR *actor_ptr = &_G(actor)[MAX_ACTORS - 1];
	ACTOR *actor2_storage = nullptr;

	for (int actor_num = 0; actor_num <= MAX_ACTORS; ) {
		// Check for blinking flag
		if (actor_ptr->used && !(actor_ptr->show & 2)) {
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

	if (_G(gameMode) == MODE_THOR_DIES && _deathCtr >= DEATH_THRESHOLD)
		s.blitFrom(_G(objects)[10], Common::Point(_G(thor)->x, _G(thor)->y));
}

void GameContent::checkThunderShake() {
	if (_G(thunder_flag)) {
		_G(gameMode) = MODE_THUNDER;

		// Introduce a random screen shake by rendering screen 1 pixel offset randomly
		static const int8 DELTA_X[4] = { -1, 1, 0, 0 };
		static const int8 DELTA_Y[4] = { 0, 0, -1, 1 };
		int delta = g_events->getRandomNumber(3);

		_moveDelta.x = DELTA_X[delta];
		_moveDelta.y = DELTA_Y[delta];

		_G(thunder_flag)--;
		if ((_G(thunder_flag) < MAX_ACTORS) && _G(thunder_flag) > 2) {
			if (_G(actor)[_G(thunder_flag)].used) {
				_G(actor)[_G(thunder_flag)].vunerable = 0;
				actor_damaged(&_G(actor)[_G(thunder_flag)], 20);
			}
		}

		if (!_G(thunder_flag)) {
			_G(gameMode) = MODE_NORMAL;
			_moveDelta = Common::Point(0, 0);
		}

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
	int thor_pos = _G(thor)->getPos();
	if (_G(object_map)[thor_pos])
		pick_up_object(thor_pos);
}

void GameContent::moveActors() {
	for (int i = 0; i < MAX_ACTORS; i++) {
		if (_G(actor)[i].used) {
			if (_G(hourglass_flag))
				if ((i > 2) && (!_G(pge)) && (!(_G(actor)[i].magic_hurts & HOURGLASS_MAGIC)))
					continue;

			_G(actor)[i].move_count = _G(actor)[i].num_moves;
			while (_G(actor)[i].move_count--)
				move_actor(&_G(actor)[i]);

			if (i == 0)
				set_thor_vars();

			if (_G(new_level) != _G(current_level))
				return;
		}
	}

	int thor_pos = _G(thor)->getPos();
	_G(thor)->center_x = thor_pos % 20;
	_G(thor)->center_y = thor_pos / 20;
}

void GameContent::updateActors() {
	for (int i = 0; i < MAX_ACTORS; ++i) {
		ACTOR *actor = &_G(actor)[i];

		if (!actor->used && actor->dead > 0)
			actor->dead--;
	}
}

void GameContent::checkForAreaChange() {
	if (_G(gameMode) == MODE_AREA_CHANGE) {
		// Area transition is already in progress
		switch (_G(transitionDir)) {
		case DIR_LEFT:
		case DIR_RIGHT:
			_transitionPos += 32;
			if (_transitionPos == 320)
				_G(gameMode) = MODE_NORMAL;
			break;
		case DIR_UP:
		case DIR_DOWN:
			_transitionPos += 16;
			if (_transitionPos == 192)
				_G(gameMode) = MODE_NORMAL;
			break;
		case DIR_PHASED:
			_transitionPos += 10;
			if (_transitionPos == 240) {
				_G(gameMode) = MODE_NORMAL;
				Common::fill(_phased, _phased + 240, false);
			} else {
				// The screen is subdivided into 240 16x16 blocks. Picks ones
				// randomly to copy over from the new screen
				for (int i = 0; i < 10; ++i) {
					for (;;) {
						int idx = g_events->getRandomNumber(239);
						if (!_phased[idx]) {
							_phased[idx] = true;
							break;
						}
					}
				}
			}
			break;

		default:
			break;
		}

		if (_G(gameMode) == MODE_NORMAL) {
			_transitionPos = 0;
			show_level_done();
		}

	} else if (_G(new_level) != _G(current_level)) {
		// Area transition beginning
		_G(thor)->show = 0;
		_G(thor)->used = 0;
		_G(hammer)->used = 0;
		_G(tornado_used) = 0;

		// Draws the old area without Thor, and then save a copy of it.
		// This will be used to scroll old area off-screen as new area scrolls in
		draw();
		_surface.copyFrom(getSurface());

		// Set up new level
		_G(thor)->used = 1;
		show_level(_G(new_level));
	} 
}

void GameContent::thorDies() {
	// Stop any actors on-screen from moving
	for (int li = 0; li < MAX_ACTORS; li++)
		_G(actor)[li].show = 0;
	_G(actor)[2].used = 0;

	// Set the state for showing death animation
	_G(gameMode) = MODE_THOR_DIES;
	_deathCtr = 0;
	_G(shield_on) = false;

	play_sound(DEAD, 1);
}

void GameContent::spinThor() {
	static const byte DIRS[] = { 0,2,1,3 };

	_G(thor)->dir = DIRS[(_deathCtr / SPIN_INTERVAL) % 4];
	_G(thor)->last_dir = DIRS[(_deathCtr / SPIN_INTERVAL) % 4];

	++_deathCtr;
}

void GameContent::thorDead() {
	int li = _G(thor_info).item;
	int ln = _G(thor_info).inventory;

	_G(new_level) = _G(thor_info).last_screen;
	_G(thor)->x = (_G(thor_info).last_icon % 20) * 16;
	_G(thor)->y = ((_G(thor_info).last_icon / 20) * 16) - 1;
	if (_G(thor)->x < 1) _G(thor)->x = 1;
	if (_G(thor)->y < 0) _G(thor)->y = 0;
	_G(thor)->last_x[0] = _G(thor)->x;
	_G(thor)->last_x[1] = _G(thor)->x;
	_G(thor)->last_y[0] = _G(thor)->y;
	_G(thor)->last_y[1] = _G(thor)->y;
	_G(thor)->dir = _G(thor_info).last_dir;
	_G(thor)->last_dir = _G(thor_info).last_dir;
	_G(thor)->health = _G(thor_info).last_health;
	_G(thor_info).magic = _G(thor_info).last_magic;
	_G(thor_info).jewels = _G(thor_info).last_jewels;
	_G(thor_info).keys = _G(thor_info).last_keys;
	_G(thor_info).score = _G(thor_info).last_score;
	_G(thor_info).object = _G(thor_info).last_object;
	_G(thor_info).object_name = _G(thor_info).last_object_name;

	if (ln == _G(thor_info).last_inventory) {
		_G(thor_info).item = li;
	} else {
		_G(thor_info).item = _G(thor_info).last_item;
		_G(thor_info).inventory = _G(thor_info).last_inventory;
	}

	_G(setup) = _G(last_setup);

	_G(thor)->num_moves = 1;
	_G(thor)->vunerable = 60;
	_G(thor)->show = 60;
	_G(hourglass_flag) = 0;
	_G(apple_flag) = 0;
	_G(bomb_flag) = 0;
	_G(thunder_flag) = 0;
	_G(lightning_used) = 0;
	_G(tornado_used) = 0;
	_G(shield_on) = 0;
	music_resume();
	_G(actor)[1].used = 0;
	_G(actor)[2].used = 0;
	_G(thor)->speed_count = 6;
	_G(thor)->used = 1;

	// Load saved data for new level back into scrn
	_G(scrn).load(_G(sd_data) + (_G(new_level) * 512));

	_G(gameMode) = MODE_NORMAL;
	_deathCtr = 0;

	show_level(_G(new_level));
	set_thor_vars();
}

void GameContent::checkForCheats() {
	if (_G(cheats).freezeHealth)
		_G(thor)->health = 150;
	if (_G(cheats).freezeMagic)
		_G(thor_info).magic = 150;
	if (_G(cheats).freezeJewels)
		_G(thor_info).jewels = 999;
}

} // namespace Views
} // namespace Got
