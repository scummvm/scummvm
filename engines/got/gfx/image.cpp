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

#include "got/gfx/image.h"
#include "common/memstream.h"
#include "got/events.h"
#include "got/got.h"
#include "got/utils/file.h"
#include "got/vars.h"

namespace Got {

static void createSurface(Graphics::ManagedSurface &s, const byte *src) {
	s.create(16, 16);

	// Both 0 and 15 are transparent colors, so as we load the
	// surface, standard on a single color
	byte *dest = (byte *)s.getPixels();
	for (int i = 0; i < 16 * 16; ++i, ++src, ++dest)
		*dest = (*src == 15) ? 0 : *src;

	s.setTransparentColor(0);
}

void setup_actor(ACTOR *actr, char num, char dir, int x, int y) {
	actr->next = 0; // Next frame to be shown
	actr->frame_count = actr->frame_speed;
	actr->dir = dir;      // Direction of travel
	actr->last_dir = dir; // Last direction of travel
	if (actr->directions == 1)
		actr->dir = 0;
	if (actr->directions == 2)
		actr->dir &= 1;
	if (actr->directions == 4 && actr->frames == 1) {
		actr->dir = 0;
		actr->next = dir;
	}

	actr->x = x;         // Actual X coor
	actr->y = y;         // Actual Y coor
	actr->width = 16;    // Actual X coor
	actr->height = 16;   // Actual Y coor
	actr->center = 0;    // Center of object
	actr->last_x[0] = x; // Last X coor on each page
	actr->last_x[1] = x;
	actr->last_y[0] = y; // Last Y coor on each page
	actr->last_y[1] = y;
	actr->used = 1;            // 1=active, 0=not active
	actr->speed_count = 8;     // Count down to movement
	actr->vunerable = STAMINA; // Count down to vulnerability
	actr->shot_cnt = 20;       // Count down to another shot
	actr->num_shots = 0;       // # of shots currently on screen
	actr->creator = 0;         // which actor # created this actor
	actr->pause = 0;           // Pause must be 0 to move
	actr->show = 0;
	actr->actor_num = num;
	actr->counter = 0;
	actr->move_counter = 0;
	actr->edge_counter = 20;
	actr->hit_thor = 0;
	actr->rand = g_engine->getRandomNumber(99);
	actr->temp1 = 0;
	actr->init_health = actr->health;
}

void make_actor_surface(ACTOR *actr) {
	assert(actr->directions <= 4 && actr->frames <= 4);
	for (int d = 0; d < actr->directions; d++) {
		for (int f = 0; f < actr->frames; f++) {
			Graphics::ManagedSurface &s = actr->pic[d][f];
			const byte *src = &_G(tmp_buff[256 * ((d * 4) + f)]);
			createSurface(s, src);
		}
	}
}

int load_standard_actors() {
	load_actor(0, 100 + _G(thor_info).armor); // Load Thor
	_G(actor[0]).loadFixed(_G(tmp_buff) + 5120);
	setup_actor(&_G(actor[0]), 0, 0, 100, 100);
	_G(thor) = &_G(actor[0]);

	make_actor_surface(&_G(actor[0]));

	_G(thor_x1) = _G(thor)->x + 2;
	_G(thor_y1) = _G(thor)->y + 2;
	_G(thor_x2) = _G(thor)->x + 14;
	_G(thor_y2) = _G(thor)->y + 14;

	load_actor(0, 103 + _G(thor_info).armor); // Load hammer
	_G(actor[1]).loadFixed(_G(tmp_buff) + 5120);
	setup_actor(&_G(actor[1]), 1, 0, 100, 100);
	_G(actor[1]).used = 0;
	_G(hammer) = &_G(actor[1]);

	make_actor_surface(&_G(actor[1]));

	// Load sparkle
	load_actor(0, 106);
	_G(sparkle).loadFixed(_G(tmp_buff) + 5120);
	setup_actor(&_G(sparkle), 20, 0, 100, 100);
	_G(sparkle).used = 0;
	make_actor_surface(&_G(sparkle));

	// Load explosion
	load_actor(0, 107);
	_G(explosion).loadFixed(_G(tmp_buff) + 5120);
	setup_actor(&_G(explosion), 21, 0, 100, 100);
	_G(explosion).used = 0;
	make_actor_surface(&_G(explosion));

	// Load tornado
	load_actor(0, 108);
	_G(magic_item[0]).loadFixed((const byte *)_G(tmp_buff) + 5120);
	Common::copy(_G(tmp_buff), _G(tmp_buff) + 1024, _G(magic_pic[0]));

	setup_actor(&_G(magic_item[0]), 20, 0, 0, 0);
	_G(magic_item[0]).used = 0;

	// Load shield
	load_actor(0, 109);
	_G(magic_item[1]).loadFixed((const byte *)_G(tmp_buff) + 5120);
	Common::copy(_G(tmp_buff), _G(tmp_buff) + 1024, _G(magic_pic[1]));

	setup_actor(&_G(magic_item[1]), 20, 0, 0, 0);
	_G(magic_item[1]).used = 0;

	_G(actor[2]).used = 0;

	make_actor_surface(&_G(magic_item[0]));

	return 1;
}

void show_enemies() {
	int i, d, r;

	for (i = 3; i < MAX_ACTORS; i++)
		_G(actor[i]).used = 0;
	for (i = 0; i < MAX_ENEMIES; i++)
		_G(enemy_type[i]) = 0;

	for (i = 0; i < MAX_ENEMIES; i++) {
		if (_G(scrn).actor_type[i] > 0) {
			r = load_enemy(_G(scrn).actor_type[i]);
			if (r >= 0) {
				_G(actor[i + 3]) = _G(enemy[r]);

				d = _G(scrn).actor_dir[i];

				setup_actor(&_G(actor[i + 3]), i + 3, d, (_G(scrn).actor_loc[i] % 20) * 16,
							(_G(scrn).actor_loc[i] / 20) * 16);
				_G(actor[i + 3]).init_dir = _G(scrn).actor_dir[i];
				_G(actor[i + 3]).pass_value = _G(scrn).actor_value[i];

				if (_G(actor[i + 3]).move == 23) {
					// Spinball
					if (_G(actor[i + 3]).pass_value & 1)
						_G(actor[i + 3]).move = 24;
				}

				if (_G(scrn).actor_invis[i])
					_G(actor[i + 3]).used = 0;
			}

			_G(etype[i]) = r;
		}
	}
}

int load_enemy(int type) {
	int i, e;

	for (i = 0; i < MAX_ENEMIES; i++) {
		if (_G(enemy_type[i]) == type)
			return i;
	}

	if (!load_actor(1, type)) {
		return -1;
	}

	e = -1;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (!_G(enemy_type[i])) {
			e = i;
			break;
		}
	}
	if (e == -1)
		return -1;

	Common::MemoryReadStream inf(_G(tmp_buff) + 5120, 40);
	_G(enemy[e]).loadFixed(&inf);

	make_actor_surface(&_G(enemy[e]));
	_G(enemy_type[e]) = type;
	_G(enemy[e]).shot_type = 0;

	if (_G(enemy[e]).shots_allowed) {
		_G(enemy[e]).shot_type = e + 1;

		// Set up shot info
		_G(shot[e]).loadFixed(_G(tmp_buff) + 5160);

		// Loop to set up graphics
		for (int d = 0; d < _G(shot[e]).directions; d++) {
			for (int f = 0; f < _G(shot[e]).frames; f++) {
				if (_G(shot[e]).directions < _G(shot[e]).frames) {
					Graphics::ManagedSurface &s = _G(shot[e]).pic[d][f];
					const byte *src = &_G(tmp_buff[4096 + (256 * ((d * 4) + f))]);
					createSurface(s, src);
				} else {
					Graphics::ManagedSurface &s = _G(shot[e]).pic[f][d];
					const byte *src = &_G(tmp_buff[4096 + (256 * ((f * 4) + d))]);
					createSurface(s, src);
				}
			}
		}
	}

	return e;
}

int actor_visible(int invis_num) {
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (_G(scrn).actor_invis[i] == invis_num) {
			int etype = _G(etype[i]);
			if (etype >= 0 && !_G(actor[i + 3]).used) {
				_G(actor[i + 3]) = _G(enemy[etype]);

				int d = _G(scrn).actor_dir[i];
				setup_actor(&_G(actor[i + 3]), i + 3, d, (_G(scrn).actor_loc[i] % 20) * 16,
							(_G(scrn).actor_loc[i] / 20) * 16);
				_G(actor[i + 3]).init_dir = _G(scrn).actor_dir[i];
				_G(actor[i + 3]).pass_value = _G(scrn).actor_value[i];
				return i;
			}

			return -1;
		}
	}
	return -1;
}

void setup_magic_item(int item) {
	for (int i = 0; i < 4; i++) {
		createSurface(_G(magic_item[item]).pic[i / 4][i % 4], &_G(magic_pic[item][256 * i]));
	}
}

void load_new_thor() {
	load_actor(0, 100 + _G(thor_info).armor); // Load Thor

	make_actor_surface(&_G(actor[0]));
}

} // namespace Got
