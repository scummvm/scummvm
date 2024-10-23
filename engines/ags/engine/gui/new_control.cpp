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

#include "ags/engine/gui/new_control.h"
#include "ags/engine/gui/gui_dialog.h"
#include "ags/engine/gui/gui_dialog_defines.h"
#include "ags/globals.h"

namespace AGS3 {

NewControl::NewControl(int xx, int yy, int wi, int hi) {
	x = xx;
	y = yy;
	wid = wi;
	hit = hi;
	state = 0;
	typeandflags = 0;
	wlevel = 0;
	visible = 1;
	enabled = 1;
	needredraw = 1;
}

NewControl::NewControl() {
	x = y = wid = hit = 0;
	state = 0;
	typeandflags = 0;
	wlevel = 0;
	visible = 1;
	enabled = 1;
	needredraw = 1;
}

int NewControl::mouseisinarea(int mx, int my) {
	if (_G(topwindowhandle) != wlevel)
		return 0;

	if ((mx > x) &(mx < x + wid) &(my > y) &(my < y + hit))
		return 1;

	return 0;
}

void NewControl::drawifneeded() {
	if (_G(topwindowhandle) != wlevel)
		return;
	if (needredraw) {
		needredraw = 0;
		draw(get_gui_screen());
	}
}
void NewControl::drawandmouse() {
	draw(get_gui_screen());
}

} // namespace AGS3
