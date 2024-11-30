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

bool load_bg_data() {
	_G(bg_pics) = new byte[60460];
	if (!_G(bg_pics))
		return false;

	if (GAME1) {
		if (res_read("BPICS1", _G(bg_pics)) < 0)
			return false;
	}

	return true;
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
	odin_speaks(2008, -1);
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

/*
long res_read(char *name,char *buff){
int num,bytes;
size_t len;
size_t total;
char bf[256];
char *p;
unsigned int clen;
unsigned int *up;

if(!res_active) return RES_NOT_ACTIVE;
if(!res_fp) return RES_NOT_OPEN;

num=res_find_name(name);
if(num<0) return RES_CANT_FIND;

if(fseek(res_fp,res_header[num].offset,SEEK_SET)) return RES_CANT_SEEK;
len=(size_t) res_header[num].length;

total=0;
if(res_header[num].key) p=buff;
else p=lzss_buff;
while(total<len){
	 if(((len-total) >255) && (len > 255)) bytes=fread(bf,1,256,res_fp);
	 else bytes=fread(bf,1,len-total,res_fp);
	 if(!bytes) break;
	 total+=bytes;
	 movedata(FP_SEG(bf),FP_OFF(bf),FP_SEG(p),FP_OFF(p),bytes);
	 p+=bytes;
}
if(res_header[num].key) res_decode(buff,len,res_header[num].key);
else{
  p=lzss_buff;
  up=(unsigned int *) p;
  clen=*up;
  p+=4;
  UnLZSS(p,buff,clen);
}
return res_header[num].length;
}
*/

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
