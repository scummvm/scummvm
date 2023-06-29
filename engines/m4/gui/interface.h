
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

#ifndef M4_GUI_INTERFACE_H
#define M4_GUI_INTERFACE_H

#include "m4/m4_types.h"
#include "m4/graphics/graphics.h"
#include "m4/graphics/gr_buff.h"

namespace M4 {

struct Interface_Globals {
	int arrow = 0;
	int wait = 0;
	int look = 0;
	int grab = 0;
	int use = 0;
	bool visible = false;
	bool shown = false;
	int x1 = 0;
	int y1 = 374;
	int x2 = SCREEN_WIDTH;
	int y2 = SCREEN_HEIGHT;
	GrBuff *gameInterfaceBuff = nullptr;

	void show() {
		assert(0);
	}
};

extern void interface_init(int arrow, int wait, int look, int grab, int use);
extern void interface_shutdown();

} // End of namespace M4

#endif
