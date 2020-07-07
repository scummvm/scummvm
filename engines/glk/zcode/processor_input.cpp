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

#include "glk/zcode/processor.h"

namespace Glk {
namespace ZCode {

#define INPUT_BUFFER_SIZE 200

bool Processor::read_yes_or_no(const char *s) {
	zchar key;

	print_string(s);
	print_string("? (y/n) >");

	key = stream_read_key(0, 0, false);

	if (key == 'y' || key == 'Y') {
		print_string("y\n");
		return true;
	} else {
		print_string("n\n");
		return false;
	}
}

void Processor::read_string(int max, zchar *buffer) {
	zchar key;

	buffer[0] = 0;

	do {
		key = stream_read_input(max, buffer, 0, 0, false, false);
	} while (key != ZC_RETURN);
}

bool Processor::is_terminator(zchar key) {
	if (key == ZC_TIME_OUT)
		return true;
	if (key == ZC_RETURN)
		return true;
	if (key >= ZC_HKEY_MIN && key <= ZC_HKEY_MAX)
		return true;

	if (h_terminating_keys != 0) {
		if (key >= ZC_ARROW_MIN && key <= ZC_MENU_CLICK) {

			zword addr = h_terminating_keys;
			zbyte c;

			do {
				LOW_BYTE(addr, c);
				if (c == 255 || key == translate_from_zscii(c))
					return true;
				addr++;
			} while (c != 0);
		}
	}

	return false;
}

void Processor::z_make_menu() {
	// This opcode was only used for the Macintosh version of Journey.
	// It controls menus with numbers greater than 2 (menus 0, 1 and 2
	// are system menus).
	branch(false);
}

int Processor::read_number() {
	zchar buffer[6];
	int value = 0;
	int i;

	read_string(5, buffer);

	for (i = 0; buffer[i] != 0; i++)
	if (buffer[i] >= '0' && buffer[i] <= '9')
		value = 10 * value + buffer[i] - '0';

	return value;
}

void Processor::z_read() {
	zchar buffer[INPUT_BUFFER_SIZE];
	zword addr;
	zchar key;
	zbyte max, size;
	zbyte c;
	int i;

	// Supply default arguments
	if (zargc < 3)
		zargs[2] = 0;

	// Get maximum input size
	addr = zargs[0];

	LOW_BYTE(addr, max);

	if (h_version <= V4)
		max--;

	if (max >= INPUT_BUFFER_SIZE)
		max = INPUT_BUFFER_SIZE - 1;

	// Get initial input size
	if (h_version >= V5) {
		addr++;
		LOW_BYTE(addr, size);
	} else {
		size = 0;
	}

	// Copy initial input to local buffer
	for (i = 0; i < size; i++) {
		addr++;
		LOW_BYTE(addr, c);
		buffer[i] = translate_from_zscii(c);
	}
	buffer[i] = 0;

	// Draw status line for V1 to V3 games
	if (h_version <= V3)
		z_show_status();

	// Read input from current input stream
	key = stream_read_input(
		max, buffer,		// buffer and size
		zargs[2],			// timeout value
		zargs[3],			// timeout routine
		false,				// enable hot keys
		h_version == V6		// no script in V6
	);

	if (key == ZC_BAD)
		return;

	// Perform save_undo for V1 to V4 games
	if (h_version <= V4)
		save_undo();

	// Copy local buffer back to dynamic memory
	for (i = 0; buffer[i] != 0; i++) {
		if (key == ZC_RETURN) {
			buffer[i] = unicode_tolower (buffer[i]);
		}

		storeb((zword)(zargs[0] + ((h_version <= V4) ? 1 : 2) + i), translate_to_zscii(buffer[i]));
	}

	// Add null character (V1-V4) or write input length into 2nd byte
	if (h_version <= V4)
		storeb((zword)(zargs[0] + 1 + i), 0);
	else
		storeb((zword)(zargs[0] + 1), i);

	// Tokenise line if a token buffer is present
	if (key == ZC_RETURN && zargs[1] != 0)
		tokenise_line (zargs[0], zargs[1], 0, false);

	// Store key
	if (h_version >= V5)
		store(translate_to_zscii(key));
}

void Processor::z_read_char() {
	zchar key;

	// Supply default arguments
	if (zargc < 2)
		zargs[1] = 0;

	// Read input from the current input stream
	key = stream_read_key(
		zargs[1],	// timeout value
		zargs[2],	// timeout routine
		false  		// enable hot keys
	);

	if (key == ZC_BAD)
		return;

	// Store key
	store(translate_to_zscii(key));
}

void Processor::z_read_mouse() {
	zword btn;

	// Read the mouse position, the last menu click and which buttons are down
	btn = os_read_mouse();
	hx_mouse_y = mouse_y;
	hx_mouse_x = mouse_x;

	storew((zword)(zargs[0] + 0), hx_mouse_y);
	storew((zword)(zargs[0] + 2), hx_mouse_x);
	storew((zword)(zargs[0] + 4), btn);				// mouse button bits
	storew((zword)(zargs[0] + 6), menu_selected);	// menu selection
}

} // End of namespace ZCode
} // End of namespace Glk
