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

#include "got/game/back.h"
#include "common/file.h"
#include "got/events.h"
#include "got/game/boss1.h"
#include "got/game/boss2.h"
#include "got/game/boss3.h"
#include "got/game/move.h"
#include "got/game/object.h"
#include "got/game/script.h"
#include "got/gfx/image.h"
#include "got/vars.h"

namespace Got {

const char *OBJECT_NAMES[] = {
	"Shrub", "Child's Doll", "UNUSED", "FUTURE",
	"FUTURE", "FUTURE", "FUTURE", "FUTURE", "FUTURE",
	"FUTURE", "FUTURE", "FUTURE", "FUTURE", "FUTURE",
	"FUTURE"};
const char *ITEM_NAMES[] = {
	"Enchanted Apple", "Lightning Power",
	"Winged Boots", "Wind Power",
	"Amulet of Protection", "Thunder Power"};

static const char *odinEndMessage;

void show_level(int new_level) {
	_G(boss_active) = false;
	if (!_G(shield_on))
		_G(actor[2]).used = 0;
	_G(bomb_flag) = false;
	_G(slipping) = false;

	if (_G(scrn).icon[_G(thor)->center_y][_G(thor)->center_x] == 154)
		_G(thor)->dir = 0;

	// The original copied 130 bytes from _G(scrn).static_object onwards into sd_data.
	// This doesn't make sense, because that would put the ending in the middle of static_y.
	// Plus, it follows with an entire copy of scrn into sd_data anyway, so the first
	// move seems entirely redundant.
	_G(scrn).save(_G(current_level));
	_G(scrn).load(_G(new_level));

	_G(level_type) = _G(scrn).type;

	_G(thor)->next = 0;

	show_objects();
	show_enemies();

	// The original was probably shortly displaying Thor in direction 0 before switching back to its prior position.
	// This behavior wasn't noticed during initial playthrough by Dreammaster - Warning has been added so it can be checked eventually.
	if (_G(scrn).icon[_G(thor)->center_y][_G(thor)->center_x] == 154)
		warning("show_level - Potential short move missing");

	if (_G(warp_flag))
		_G(current_level) = new_level - 5; // Force phase
	_G(warp_flag) = false;

	if (_G(warp_scroll)) {
		_G(warp_scroll) = false;
		if (_G(thor)->dir == 0)
			_G(current_level) = new_level + 10;
		else if (_G(thor)->dir == 1)
			_G(current_level) = new_level - 10;
		else if (_G(thor)->dir == 2)
			_G(current_level) = new_level + 1;
		else if (_G(thor)->dir == 3)
			_G(current_level) = new_level - 1;
	}

	if (!_G(setup).scroll_flag)
		_G(current_level) = new_level; // Force no scroll

	if (_G(music_current) != _G(level_type))
		_G(sound).music_pause();

	switch (_G(new_level) - _G(current_level)) {
	case 0:
		// Nothing to do
		show_level_done();
		break;
	case -1:
		_G(gameMode) = MODE_AREA_CHANGE;
		_G(transitionDir) = DIR_LEFT;
		break;
	case 1:
		_G(gameMode) = MODE_AREA_CHANGE;
		_G(transitionDir) = DIR_RIGHT;
		break;
	case -10:
		_G(gameMode) = MODE_AREA_CHANGE;
		_G(transitionDir) = DIR_UP;
		break;
	case 10:
		_G(gameMode) = MODE_AREA_CHANGE;
		_G(transitionDir) = DIR_DOWN;
		break;
	default:
		_G(gameMode) = MODE_AREA_CHANGE;
		_G(transitionDir) = DIR_PHASED;
		break;
	}
}

void show_level_done() {
	_G(current_level) = _G(new_level);

	_G(thor_info).last_health = _G(thor)->health;
	_G(thor_info).last_magic = _G(thor_info).magic;
	_G(thor_info).last_jewels = _G(thor_info).jewels;
	_G(thor_info).last_keys = _G(thor_info).keys;
	_G(thor_info).last_score = _G(thor_info).score;
	_G(thor_info).last_item = _G(thor_info).item;
	_G(thor_info).last_screen = _G(current_level);
	_G(thor_info).last_icon = ((_G(thor)->x + 8) / 16) + (((_G(thor)->y + 14) / 16) * 20);
	_G(thor_info).last_dir = _G(thor)->dir;
	_G(thor_info).last_inventory = _G(thor_info).inventory;
	_G(thor_info).last_object = _G(thor_info).object;
	_G(thor_info).last_object_name = _G(thor_info).object_name;

	_G(last_setup) = _G(setup);

	bool f = true;
	if (GAME1 && _G(new_level) == BOSS_LEVEL1) {
		if (!_G(setup).boss_dead[0]) {
			if (!_G(auto_load))
				boss_level1();
			f = false;
		}
	}
	if (GAME2 && _G(new_level) == BOSS_LEVEL2) {
		if (!_G(setup).boss_dead[1]) {
			if (!_G(auto_load))
				boss_level2();
			f = false;
		}
	}
	if (GAME3) {
		if (_G(new_level) == BOSS_LEVEL3) {
			if (!_G(setup).boss_dead[2]) {
				if (!_G(auto_load))
					boss_level3();
				f = false;
			}
		}
		if (_G(current_level) == ENDING_SCREEN) {
			ending_screen();
			f = false;
		}
	}

	if (_G(startup))
		f = false;
	if (f)
		music_play(_G(level_type), false);
}

static void odin_speaks_end() {
	// In case Thor is now dead, flag as such
	if (!_G(thor)->health) {
		_G(thor)->show = 0;
		_G(exit_flag) = 2;
	}

	// If there's an end message, pass it on to the view hierarchy.
	// This is used in cases like the game end where multiple
	// odin_speaks are done in sequence
	if (odinEndMessage)
		g_events->send(GameMessage(odinEndMessage));
}

void odin_speaks(int index, int item, const char *endMessage) {
	odinEndMessage = endMessage;

	execute_script((long)index, _G(odin), odin_speaks_end);
}

int switch_icons() {
	play_sound(WOOP, false);

	for (int y = 0; y < 12; y++) {
		for (int x = 0; x < 20; x++) {
			int ix = x * 16;
			int iy = y * 16;
			if (_G(scrn).icon[y][x] == 93) {
				place_tile(x, y, 144);
			} else if (_G(scrn).icon[y][x] == 144) {
				place_tile(x, y, 93);
				kill_enemies(iy, ix);
			}
			if (_G(scrn).icon[y][x] == 94) {
				place_tile(x, y, 146);
			} else if (_G(scrn).icon[y][x] == 146) {
				place_tile(x, y, 94);
				kill_enemies(iy, ix);
			}
		}
	}

	return 0;
}

int rotate_arrows() {
	play_sound(WOOP, false);

	for (int y = 0; y < 12; y++) {
		for (int x = 0; x < 20; x++) {
			if (_G(scrn).icon[y][x] == 205)
				place_tile(x, y, 208);
			else if (_G(scrn).icon[y][x] == 206)
				place_tile(x, y, 207);
			else if (_G(scrn).icon[y][x] == 207)
				place_tile(x, y, 205);
			else if (_G(scrn).icon[y][x] == 208)
				place_tile(x, y, 206);
		}
	}

	return 0;
}

void kill_enemies(int iy, int ix) {
	int x1, y1, x2, y2;

	for (int i = 3; i < MAX_ACTORS; i++) {
		if (_G(actor[i]).used) {
			x1 = _G(actor[i]).x;
			y1 = _G(actor[i]).y + _G(actor[i]).size_y - 2;
			x2 = (_G(actor[i]).x + _G(actor[i]).size_x);
			y2 = _G(actor[i]).y + _G(actor[i]).size_y - 1;

			if (point_within(x1, y1, ix, iy, ix + 15, iy + 15))
				actor_destroyed(&_G(actor[i]));
			else if (point_within(x2, y1, ix, iy, ix + 15, iy + 15))
				actor_destroyed(&_G(actor[i]));
			else if (point_within(x1, y2, ix, iy, ix + 15, iy + 15))
				actor_destroyed(&_G(actor[i]));
			else if (point_within(x2, y2, ix, iy, ix + 15, iy + 15))
				actor_destroyed(&_G(actor[i]));
		}
	}

	x1 = _G(thor)->x;
	y1 = _G(thor)->y + 11;
	x2 = x1 + 13;
	y2 = y1 + 5;

	if (point_within(x1, y1, ix, iy, ix + 15, iy + 15) || point_within(x2, y1, ix, iy, ix + 15, iy + 15) || point_within(x1, y2, ix, iy, ix + 15, iy + 15) || point_within(x2, y2, ix, iy, ix + 15, iy + 15)) {
		if (!_G(cheats).freezeHealth) {
			_G(thor)->health = 0;
			g_events->send(GameMessage("THOR_DIES"));
		}
	}
}

void remove_objects(int y, int x) {
	int p = (y * 20) + x;

	if (_G(object_map[p]) > 0) {
		_G(object_map[p]) = 0;
		_G(object_index[p]) = 0;
	}
}

void place_tile(int x, int y, int tile) {
	_G(scrn).icon[y][x] = tile;
	remove_objects(y, x);
}

int bgtile(int x, int y) {
	if (x < 0 || x >= 319 || y < 0 || y >= 191)
		return 0;

	x = (x + 1) >> 4;
	y = (y + 1) >> 4;

	return _G(scrn).icon[y][x];
}

void select_item() {
	// Only allow opening the dialog if something isn't currently going on
	if (g_engine->canSaveAutosaveCurrently()) {
		g_events->addView("SelectItem");
	}
}

int actor_speaks(ACTOR *actr, int index, int item) {
	if (actr->type != 4)
		return 0;

	int v = atoi(actr->name);
	if (v < 1 || v > 20)
		return 0;

	long lind = (long)_G(current_level);
	lind = lind * 1000;
	lind += (long)actr->actor_num;

	Common::String str = Common::String::format("FACE%d", v);
	if (Common::File::exists(Common::Path(str))) {
		Gfx::Pics pics(str, 262);
		execute_script(lind, pics);
	} else {
		execute_script(lind, _G(odin));
	}

	if (!_G(thor)->health) {
		_G(thor)->show = 0;
		_G(exit_flag) = 2;
	}

	return 1;
}

} // namespace Got
