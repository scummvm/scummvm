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

#include "common/memstream.h"
#include "got/game/back.h"
#include "got/gfx/image.h"
#include "got/vars.h"

namespace Got {

void show_level(int new_level) {
	int save_d;

	_G(boss_active) = 0;
	if (!_G(shield_on))
		_G(actor)[2].used = 0;
	_G(bomb_flag) = 0;

	save_d = _G(thor)->dir;
	if (_G(scrn).icon[_G(thor)->center_y][_G(thor)->center_x] == 154)
		_G(thor)->dir = 0;

	// The original copied 130 bytes from scrn.static_object onwards into sd_data.
	// This doesn't make sense, because that would put the ending in the middle of static_y.
	// Plus, it follows with an entire copy of scrn into sd_data anyway, so the first
	// move seems entirely redundant.
	Common::MemoryWriteStream outStream(_G(sd_data) + (_G(current_level) * 512), 512);
	_G(scrn).save(&outStream);
	Common::MemoryReadStream inStream(_G(sd_data) + (_G(current_level) * 512), 512);
	_G(scrn).load(&inStream);

	_G(scrnp) = (byte *)&_G(scrn);
	_G(level_type) = _G(scrn).type;

	_G(thor)->next = 0;
	//xdisplay_actors(&_G(actor)[MAX_ACTORS - 1], PAGE2);

	//build_screen(draw_page);
	//show_objects(new_level, draw_page);
	show_enemies();

	if (_G(scrn).icon[_G(thor)->center_y][_G(thor)->center_x] == 154) _G(thor)->dir = 0;
	//xdisplay_actors(&_G(actor)[MAX_ACTORS - 1], draw_page);
	_G(thor)->dir = save_d;

	if (_G(warp_flag))
		_G(current_level) = new_level - 5;   //force phase

	_G(warp_flag) = 0;
	if (_G(warp_scroll)) {
		_G(warp_scroll) = 0;
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
		_G(current_level) = new_level; //force no scroll

	if (_G(music_current) != _G(level_type))
		_G(sound).music_pause();

#ifdef TODO
	switch (new_level - _G(current_level)) {
	case 0:
		xshowpage(draw_page);
		xcopyd2d(0, 0, 320, 192, 0, 0, draw_page, display_page, 320, 320);
		break;
	case 1:
	{
		scroll_level_right(); break;
	}
	case 10:
	{
		scroll_level_down(); break;
	}
	case -1:
	{
		scroll_level_left(); break;
	}
	case -10:
	{
		scroll_level_up(); break;
	}
	default:
		phase_level();
	}
	build_screen(PAGE2);
	show_objects(new_level, PAGE2);

	_G(current_level) = new_level;
	thor_info.last_health = _G(thor)->health;
	thor_info.last_magic = thor_info.magic;
	thor_info.last_jewels = thor_info.jewels;
	thor_info.last_keys = thor_info.keys;
	thor_info.last_score = thor_info.score;
	thor_info.last_item = thor_info.item;
	thor_info.last_screen = _G(current_level);
	thor_info.last_icon = ((_G(thor)->x + 8) / 16) + (((_G(thor)->y + 14) / 16) * 20);
	thor_info.last_dir = _G(thor)->dir;
	thor_info.last_inventory = thor_info.inventory;
	thor_info.last_object = thor_info.object;
	thor_info.last_object_name = thor_info.object_name;

	_G(last_setup) = _G(setup);

	f = 1;
	if (GAME1 && new_level == BOSS_LEVEL1) {
		if (!_G(setup).boss_dead[0]) {
			if (!auto_load) boss_level1();
			f = 0;
		}
	}
	if (startup) f = 0;
	if (f) music_play(_G(level_type), 0);
#endif
}

} // namespace Got
