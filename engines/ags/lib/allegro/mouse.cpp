/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/lib/allegro/mouse.h"
#include "ags/events.h"
#include "ags/globals.h"
#include "common/textconsole.h"

namespace AGS3 {

static bool isMouseButtonDown(Common::EventType type) {
	return type == Common::EVENT_LBUTTONDOWN || type == Common::EVENT_MBUTTONDOWN ||
		type == Common::EVENT_RBUTTONDOWN;
}

static bool isMouseButtonUp(Common::EventType type) {
	return type == Common::EVENT_LBUTTONUP || type == Common::EVENT_MBUTTONUP ||
		type == Common::EVENT_RBUTTONUP;
}

static bool isMouseWheel(Common::EventType type) {
	return type == Common::EVENT_WHEELDOWN || type == Common::EVENT_WHEELUP;
}

static bool isMouseEvent(Common::EventType type) {
	return type == Common::EVENT_MOUSEMOVE || isMouseButtonDown(type) ||
		isMouseButtonUp(type) || isMouseWheel(type);
}


int install_mouse() {
	_G(mouse_x) = _G(mouse_y) = _G(mouse_z) = 0;
	_G(mouse_b) = 0;
	_G(mouse_pos) = 0;

	return 0;
}

void remove_mouse() {
}

int poll_mouse() {
	::AGS::g_events->pollEvents();

	Common::Event e;
	while ((e = ::AGS::g_events->readEvent()).type != Common::EVENT_INVALID) {
		if (isMouseEvent(e.type)) {
			_G(mouse_x) = e.mouse.x;
			_G(mouse_y) = e.mouse.y;
			_G(mouse_pos) = (e.mouse.x << 16) | e.mouse.y;
		}

		switch (e.type) {
		case Common::EVENT_LBUTTONDOWN:
			_G(mouse_b) |= 1;
			break;
		case Common::EVENT_LBUTTONUP:
			_G(mouse_b) &= ~1;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_G(mouse_b) |= 2;
			break;
		case Common::EVENT_RBUTTONUP:
			_G(mouse_b) &= ~2;
			break;
		case Common::EVENT_MBUTTONDOWN:
			_G(mouse_b) |= 4;
			break;
		case Common::EVENT_MBUTTONUP:
			_G(mouse_b) &= ~4;
			break;
		case Common::EVENT_WHEELDOWN:
			++_G(mouse_z);
			break;
		case Common::EVENT_WHEELUP:
			--_G(mouse_z);
			break;
		default:
			break;
		}
	}

	return 0;
}

int mouse_needs_poll() {
	// We can always poll mouse in ScummVM
	return true;
}

void enable_hardware_cursor() {
}

void disable_hardware_cursor() {
}

void show_mouse(BITMAP *bmp) {
	warning("TODO: show_mouse");
}

void scare_mouse() {
}

void scare_mouse_area(int x, int y, int w, int h) {
}

void unscare_mouse() {
}

void position_mouse(int x, int y) {
	_G(mouse_x) = x;
	_G(mouse_y) = y;
	_G(mouse_pos) = (x << 16) | y;
	::AGS::g_events->warpMouse(Common::Point(x, y));
}

void position_mouse_z(int z) {
	_G(mouse_z) = z;
}

void set_mouse_range(int x1, int y_1, int x2, int y2) {
}

void set_mouse_speed(int xspeed, int yspeed) {
}

void select_mouse_cursor(int cursor) {
}

void set_mouse_cursor_bitmap(int cursor, BITMAP *bmp) {
}

void set_mouse_sprite_focus(int x, int y) {
}

void get_mouse_mickeys(int *mickeyx, int *mickeyy) {
}

void set_mouse_sprite(BITMAP *sprite) {
}

int show_os_cursor(int cursor) {
	return 0;
}

int mouse_on_screen() {
	return 0;
}

} // namespace AGS3
