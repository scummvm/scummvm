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

#include "mads/madsv2/core/hspot.h"
#include "mads/madsv2/core/cursor.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/screen.h"

namespace MADS {
namespace MADSV2 {

Spot spot[max_hot_spots + 1];                    /* The list    */
int  numspots = 0;

int hs_stack[HS_STACK_SIZE + 1];  /* For push/popping the list */
int hs_stack_pointer;

static int hspot_special_num;
int hotkeys[max_hot_spots + 1];


int hspot_add(int ul_x, int ul_y, int lr_x, int lr_y,
	int class_, int num, int video_mode) {
	if (numspots == max_hot_spots) {
		error_report(ERROR_TOO_MANY_DAMN_HOTSPOTS, ERROR, MODULE_HOTSPOT, class_, num);
		return(no);
	}

	numspots++;
	spot[numspots].ul_x = ul_x;
	spot[numspots].ul_y = ul_y;
	spot[numspots].lr_x = lr_x;
	spot[numspots].lr_y = lr_y;
	spot[numspots]._class = class_;
	spot[numspots].num = num;
	spot[numspots].active = yes;
	spot[numspots].video_mode = video_mode;

	return yes;
}

int hspot_push() {
	int a;

	if (hs_stack_pointer >= HS_STACK_SIZE)
		return(no);
	for (a = 1; a <= numspots; a++)
		spot[a].active = no;
	hs_stack[++hs_stack_pointer] = numspots;

	return yes;
}

int hspot_pop() {
	int a;

	if (hs_stack_pointer <= 0)
		return(no);
	numspots = hs_stack[hs_stack_pointer--];
	for (a = 1; a <= numspots; a++)
		spot[a].active = yes;
	return(yes);
}

void hspot_pack() {
	int from, unto;

	unto = 0;

	for (from = 1; from <= numspots; from++) {
		// If it's still alive
		if (spot[from].num != HS_DEATH) {
			unto++;
			if (from != unto)  // If it's actually going to move
				spot[unto] = spot[from];  // Shift spot down
		}
	}

	numspots = unto;
}


int hspot_remove(int class_, int num) {
	int a, cleared;

	// Clear ALL hotspots
	if (class_ == HS_ALL) {
		a = numspots;
		numspots = 0;
		return a;
	}

	cleared = 0;
	for (a = 1; a <= numspots; a++) {
		if ((spot[a]._class == class_) &&
			(num == HS_ALL || spot[a].num == num)) {
			spot[a].num = HS_DEATH;
			if (num != HS_ALL) {
				hspot_pack();
				return(1);
			}

			cleared++;
		}
	}

	hspot_pack();
	return cleared;
}

void hspot_toggle(int class_, int num, int active) {
	int count;

	for (count = 1; count <= numspots; count++) {
		if ((spot[count]._class == class_) && (spot[count].num == num)) {
			spot[count].active = active;
		}
	}
}

void hspot_wipe() {
	int count;

	numspots = 0;
	for (count = 0; count < max_hot_spots; count++) {
		hotkeys[count] = -1;
	}

	hotkeys[max_hot_spots] = -1;
}


int hspot_key(int key) {
	int result = -1;
	int count;

	for (count = 0; count < max_hot_spots; count++) {
		if (hotkeys[count] == key) {
			result = count;
		}
	}

	return result;
}


int hspot_begin(int x, int y, int class_, int num, int hotkey) {
	cursor_set_follow(true);
	cursor_set_pos(x, y);

	hspot_add(x, y, x, y, class_, num, screen_video_mode);
	hspot_special_num = numspots;
	hotkeys[num] = hotkey;
	return (x);
}


int hspot_end() {
	int x, y;

	cursor_get_pos(&x, &y);

	spot[hspot_special_num].lr_x = x;
	spot[hspot_special_num].lr_y = y;

	cursor_set_follow(false);
	return (x);
}


void hspot_dummy() {

}

int hspot_which(int coord_x, int coord_y, int video_mode) {
	int a;

	for (a = 1; a <= numspots; a++) {
		if (spot[a].active &&
				coord_x >= spot[a].ul_x &&
				coord_x <= spot[a].lr_x &&
				coord_y >= spot[a].ul_y &&
				coord_y <= spot[a].lr_y &&
				video_mode == spot[a].video_mode) {
			return a;
		}
	}

	return 0;
}

int hspot_which_reverse(int coord_x, int coord_y, int video_mode) {
	int a;

	for (a = numspots; a >= 1; a--) {
		if (spot[a].active &&
				coord_x >= spot[a].ul_x &&
				coord_x <= spot[a].lr_x &&
				coord_y >= spot[a].ul_y &&
				coord_y <= spot[a].lr_y &&
				video_mode == spot[a].video_mode) {
			return a;
		}
	}

	return 0;
}

} // namespace MADSV2
} // namespace MADS
