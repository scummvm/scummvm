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
#include "common/textconsole.h"

namespace AGS3 {

MOUSE_DRIVER mousedrv_none;
MOUSE_DRIVER *mouse_driver;
_DRIVER_INFO _mouse_driver_list[] = {
	{ 0, nullptr, 0 }
};

BITMAP *mouse_sprite;
int mouse_x_focus;
int mouse_y_focus;

volatile int mouse_x;
volatile int mouse_y;
volatile int mouse_z;
volatile int mouse_w;
volatile int mouse_b;
volatile int mouse_pos;

volatile int freeze_mouse_flag;

int install_mouse() {
	return 0;
}

void remove_mouse() {
}

int poll_mouse() {
	return 0;
}

int mouse_needs_poll() {
	return 0;
}

void enable_hardware_cursor() {
}

void disable_hardware_cursor() {
}

void show_mouse(BITMAP *bmp) {
}

void scare_mouse() {
}

void scare_mouse_area(int x, int y, int w, int h) {
}

void unscare_mouse() {
}

void position_mouse(int x, int y) {
}

void position_mouse_z(int z) {
}

void position_mouse_w(int w) {
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
