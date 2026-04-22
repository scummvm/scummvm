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

#include "common/debug.h"
#include "common/textconsole.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/mouse.h"

namespace MADS {
namespace MADSV2 {

int screen_normal_color;
int screen_hilite_color;
int screen_video_mode;
int screen_max_x = 80;
int screen_max_y = 25;

int screen_center_x = 0;
int screen_center_y = 0;

Window screen_active = { 0,0,79,24,NULL };  /* The currently active window */

byte *screen = nullptr; // color_text_video;

int *screen_bound_x = &screen_max_x;

void screen_set_size(short numlines) {
	mouse_hide();

	if (screen_video_mode == text_mode) {
		// No implementation in ScummVM
	}
	if (screen_max_y == 50) {
		mouse_vert_bound(0, 399);
	} else {
		mouse_vert_bound(0, 199);
	}
	screen_active.lr_y = screen_max_y - 1;

	mouse_show();
}

void screen_wipe_line(short ul_x, short ul_y, short len, short wipe_color, byte wipe_char) {
	warning("TODO: screen_wipe_line");
}

short screen_out(const char *outstring, short strcolor, short selcolor, short str_x, short str_y) {
	debug("TODO: screen_out %s", outstring);
	return 0;
}

short screen_put(const char *outstring, short strcolor, short selcolor, short str_x, short str_y) {
	warning("TODO: screen_put");
	return 0;
}

void screen_set_colors(int normal_color, int hilite_color) {
	warning("TODO: screen_set_colors");
}

void screen_set_line_width(int line_width) {
	warning("TODO: screen_set_line_width");
}

short screen_show_line(const char *outstring, short locx, short locy) {
	warning("TODO: screen_show_line");
	return 0;
}

short screen_write(const char *outstring) {
	warning("TODO: screen_write");
	return 0;
}

short screen_write_line(const char *outstring) {
	warning("TODO: screen_write_line");
	return 0;
}

void screen_clear(int clear_color) {
	warning("TODO: screen_clear");
}

void screen_dominant_mode(int dominant_mode) {
	// No implementation in ScummVM
}

void screen_init(int video_mode) {
	// No implementation in ScummVM
}

void screen_init_dual(int mono_left) {
	error("TODO: screen_init_dual");
}

void screen_shutdown_dual(int clear_flag) {
	error("TODO: screen_shutdown_dual");
}

void screen_init_graphics(int which_mode) {
	warning("TODO: screen_init_graphics");
}

void screen_shutdown_graphics(int clear_flag) {
	warning("TODO: screen_shutdown_graphics");
}

void screen_show_spot(const char *message, int wx, int wy, int class_, int num) {
	warning("TODO: screen_show_spot");
}

void screen_init_text(int which_mode) {
	warning("TODO: screen_init_text");
}

void screen_shutdown_text(int clear_flag) {
	warning("TODO: screen_shutdown_text");
}

void screen_save(void) {
	warning("TODO: screen_save");
}

void screen_restore(void) {
	warning("TODO: screen_restore");
}

short screen_show_wide(const char *outstring, short locx, short locy, short width) {
	warning("TODO: screen_show_wide");
	return 0;
}

int screen_show(const char *outstring, int locx, int locy) {
	locx = screen_put(outstring, screen_normal_color, screen_hilite_color, locx, locy);
	return (locx);
}

int screen_printf(int x, int y, const char *string, ...) {
	va_list va;
	va_start(va, string);
	Common::String msg = Common::String::vformat(string, va);
	va_end(va);

	screen_show(msg.c_str(), x, y);
	return 0;
}

int screen_print(const char *string, ...) {
	va_list va;
	va_start(va, string);
	Common::String msg = Common::String::vformat(string, va);
	va_end(va);

	screen_write(msg.c_str());
	return 0;
}

} // namespace MADSV2
} // namespace MADS
