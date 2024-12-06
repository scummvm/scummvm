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

#include "common/file.h"
#include "got/utils/file.h"
#include "got/prototypes.h"
#include "got/vars.h"

namespace Got {

static const char *gotres = "GOTRES.00";

long file_size(char *path) {
	Common::File f;
	return f.open(Common::Path(path)) ? f.size() : -1;
}

bool load_sd_data() {
	Common::String fname = Common::String::format("SDAT%d", _G(area));

	if (!_G(sd_data))
		_G(sd_data) = new byte[61440];
	if (!_G(sd_data))
		return false;

	return res_read(fname, _G(sd_data)) > 0;
}

bool load_objects() {
	return res_read("OBJECTS", _G(objects)) > 0;
}

bool load_actor(int file, int num) {
	Common::String fname = Common::String::format("ACTOR%d", num);

	if (res_read(fname, _G(tmp_buff)) < 0)
		return false;

	//file = file;
	return true;
}

bool load_speech(int index) {
	int  cnt;
	char *p;
	char *pm;
	char *sp;
	char tmps[5];

	Common::String fname = Common::String::format("SPEAK%d", _G(area));

	sp = new char[30000];
	if (!sp)
		return false;

	if (res_read(fname, sp) < 0) {
		delete[] sp;
		return 0;
	}

	p = sp;

	cnt = 0;
	for (;;) {
		if (*p == ':') {
			p++;
			cnt++;
			strncpy(tmps, p, 4);
			tmps[4] = '\0';

			if (atoi(tmps) == index) {
				break;
			}
		}

		p++;
		cnt++;
	}

	while (*p != 10)
		p++;
	p++;

	pm = p;
	cnt = 0;

	for (;;) {
		if (*p == 13)
			*p = 32;
		if (*p == ':') {
			if ((*(p + 1) == 'E') && (*(p + 2) == 'N') && (*(p + 3) == 'D')) break;
		}

		p++;
		cnt++;

		if (cnt > 5799) {
			delete[] sp;
			return false;
		}
	}

	if (*(p - 1) == 10)
		*(p - 1) = 0;
	*p = 0;

	Common::copy(pm, pm + cnt, _G(tmp_buff));
	_G(tmp_buff)[cnt] = 0;

	delete[] sp;
	return true;
}

void setup_filenames(int level) {
}

void help(void) {
#ifdef TODO
	odin_speaks(2008, -1);
#endif
}

void save_game() {
#if TODO
	int handle;
	unsigned int total;
	char buff[32];

	if (game_over)
		return;

	setup.area = _G(area);
	setup.game_over = game_over;

	if (select_option(options_yesno, "Save Game?", 0) != 1) {
		d_restore();
		return;
	}

	d_restore();


	if (_dos_open(save_filename, O_RDONLY, &handle) != 0) return;
	_dos_read(handle, buff, 32, &total);
	_dos_close(handle);

	if (_dos_open(save_filename, O_WRONLY, &handle) != 0) return;
	_dos_write(handle, buff, 32, &total);
	_dos_write(handle, &setup, sizeof(SETUP), &total);
	_dos_write(handle, &thor_info, sizeof(THOR_INFO), &total);
	_dos_write(handle, _G(sd_data), 61440u, &total);
	_dos_close(handle);
	odin_speaks(2009, 0);
#endif
}

bool load_game(int flag) {
#ifdef TODO
	int handle;
	unsigned int total;
	char buff[32];

	if (flag) {
		if (select_option(options_yesno, "Load Game?", 0) != 1) {
			d_restore();
			return 0;
		}
		d_restore();
	}
	if (_dos_open(save_filename, O_RDONLY, &handle) != 0) return 0;
	_dos_read(handle, buff, 32, &total);
	_dos_read(handle, &setup, sizeof(SETUP), &total);
	_dos_read(handle, &thor_info, sizeof(THOR_INFO), &total);
	_dos_read(handle, _G(sd_data), 61440u, &total);
	_dos_close(handle);

	current_area = thor_info.last_screen;
	area = setup.area;
	if (area == 0) area = 1;

	thor->x = (thor_info.last_icon % 20) * 16;
	thor->y = ((thor_info.last_icon / 20) * 16) - 1;
	if (thor->x < 1) thor->x = 1;
	if (thor->y < 0) thor->y = 0;
	thor->dir = thor_info.last_dir;
	thor->last_dir = thor_info.last_dir;
	thor->health = thor_info.last_health;
	thor->num_moves = 1;
	thor->vunerable = 60;
	thor->show = 60;
	thor->speed_count = 6;
	load_new_thor();
	display_health();
	display_magic();
	display_jewels();
	display_keys();
	display_item();
	if (!music_flag) setup.music = 0;
	if (!sound_flag) setup.dig_sound = 0;
	if (setup.music == 1) {
		if (GAME1 == 1 && current_area == 59) {
			if (flag) music_play(5, 1);
		} else if (flag) music_play(level_type, 1);
	} else {
		setup.music = 1;
		music_pause();
		setup.music = 0;
	}
	game_over = setup.game_over;
	slow_mode = setup.speed;
#endif
	return true;
}

long res_read(const Common::String &name, void *buff) {
	Common::File f;
	if (f.open(Common::Path(name))) {
		return f.read(buff, f.size());
	} else {
		error("Could not load - %s", name.c_str());
		return -1;
	}
}

bool load_music(int num) {
	switch (num) {
	case 0:
		res_read("SONG1", _G(song));
		break;
	case 1:
		res_read("SONG2", _G(song));
		break;
	case 2:
		res_read("SONG3", _G(song));
		break;
	case 3:
		res_read("SONG4", _G(song));
		break;
	case 4:
		res_read("WINSONG", _G(song));
		break;
	case 5:
		res_read("BOSSSONG", _G(song));
		break;
	}

	return _G(song) != nullptr;
}

} // End of namespace Got
