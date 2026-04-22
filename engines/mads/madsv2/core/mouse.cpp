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

#include "common/system.h"
#include "graphics/cursorman.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

int mouse_button = -1;
int mouse_status = 0;
int mouse_x = 0, mouse_y = 0;
bool mouse_start_stroke = false;
bool mouse_stroke_going = false;
bool mouse_changed = false;
bool mouse_latched = false;
bool mouse_stop_stroke = false;
bool mouse_any_stroke = false;
int mouse_old_x = 0;
int mouse_old_y = 0;
long mouse_clock = 0;
byte mouse_showing = 0;
int mouse_video_mode = 0;


int mouse_init(int driverflag, int videomode) {
	// No implementation in ScummVM
	return 0;
}

void mouse_show() {
	if (!CursorMan.isVisible())
		CursorMan.showMouse(true);
}

void mouse_hide() {
	CursorMan.showMouse(false);
}

void mouse_force(int x, int y) {
	g_system->warpMouse(x, y);
}

int mouse_in_box(int ul_x, int ul_y, int lr_x, int lr_y) {
	return ((mouse_x >= ul_x) && (mouse_x <= lr_x)) &&
		((mouse_y >= ul_y) && (mouse_y <= lr_y));
}

void mouse_init_cycle() {
	mouse_old_x = -1;
	mouse_old_y = -1;

	mouse_latched = false;
	mouse_stroke_going = mouse_status;
	mouse_start_stroke = false;
}

void mouse_begin_cycle(int double_flag) {
	if (double_flag) mouse_check_double();

	mouse_old_x = mouse_x;
	mouse_old_y = mouse_y;

	mouse_status = mouse_get_status(&mouse_x, &mouse_y);
	mouse_clock = timer_read();
	// mouse_video_mode = mouse_get_video_mode();
	mouse_stop_stroke = (mouse_latched && (!mouse_status));
	mouse_start_stroke = (mouse_status && !mouse_stroke_going);
	mouse_stroke_going = mouse_status;

	if (!mouse_stroke_going) mouse_latched = false;

	mouse_changed = ((mouse_x != mouse_old_x) || (mouse_y != mouse_old_y) || mouse_start_stroke || mouse_stop_stroke);

	if (mouse_start_stroke) {
		mouse_latched = true;
		if (mouse_status & 1) {
			mouse_button = 0;
		} else {
			mouse_button = 1;
		}
	}

	mouse_any_stroke = (mouse_stroke_going || mouse_stop_stroke);
}

void mouse_end_cycle(int double_flag, int timing_flag) {
	if (double_flag) {
		mouse_double_freedom(true);
	}
	if (timing_flag) {
		while (mouse_clock == timer_read());
	}
}

void mouse_cursor_sprite(SeriesPtr series, int id) {
	byte work_area[17][17];
	Buffer load_buffer = { 17, 17 };
	int hot_x, hot_y, count;

	load_buffer.data = &work_area[0][0];

	hot_x = 0;
	hot_y = 0;

	buffer_fill(load_buffer, 255);
	sprite_draw(series, id, &load_buffer, 0, 0);

	for (count = 0; count < 16; count++) {
		if (work_area[count][16] != 255) hot_y = count;
		if (work_area[16][count] != 255) hot_x = count;
	}

	// Form a 16x16 surface around the data, ignoring the last column/row,
	// which are only used for designating the hotspot position
	Graphics::Surface s;
	s.format = Graphics::PixelFormat::createFormatCLUT8();
	s.setPixels(work_area);
	s.w = 16;
	s.pitch = 17;
	s.h = 16;

	// Set the cursor
	CursorMan.replaceCursor(s, hot_x, hot_y, 0xff);
	CursorMan.disableCursorPalette(true);
}

void mouse_video_init() {
	mouse_set_work_buffer(scr_work.data, scr_work.x);
	mouse_set_view_port_loc(0, 0, scr_work.x - 1, scr_work.y - 1);
	mouse_set_view_port(0, 0);
}

void mouse_video_update(int from_x, int from_y, int unto_x, int unto_y,
	int size_x, int size_y) {
	int refresh_flag;

	mouse_freeze();
	refresh_flag = mouse_refresh_view_port();

	video_update(&scr_work, from_x, from_y, unto_x, unto_y, size_x, size_y);

	if (refresh_flag) mouse_refresh_done();
	mouse_thaw();
}

int mouse_set_hotspot(int spot_x, int spot_y) {
	return 0;
}

void mouse_change_cursor_begin() {
}

void mouse_change_cursor_end() {
}

void mouse_screen_swap(int mode) {
	mouse_video_mode = mode;
}

int mouse_get_video_mode() {
	return mouse_video_mode;
}

void mouse_begin_double(int first_video_mode, int second_video_mode,
	int mono_to_right, int auto_freedom) {
}

void mouse_check_double() {
}

void mouse_end_double() {
}

void mouse_double_freedom(int freedom_flag) {
}

int mouse_get_status(int *x, int *y) {
	return g_engine->getMouseState(*x, *y);
}

void mouse_timing() {
}

void mouse_freeze() {
}

void mouse_thaw() {
}

void mouse_horiz_bound(int min_x, int max_x) {
}

void mouse_vert_bound(int min_y, int max_y) {
}

void mouse_set_work_buffer(byte *work_buffer, int wrap_value) {
}

void mouse_set_view_port_loc(int x1, int y1, int x2, int y2) {
}

void mouse_set_view_port(int dx, int dy) {
}

int mouse_refresh_view_port() {
	return 0;
}

void mouse_refresh_done() {
}

void mouse_disable_scale() {
}

void mouse_hard_cursor_mode(int mode, Palette *mypal) {
}

const byte *mouse_get_stack() {
	error("TODO: mouse_get_stack");
}

} // namespace MADSV2
} // namespace MADS



