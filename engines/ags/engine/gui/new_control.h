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

#ifndef AGS_ENGINE_GUI_NEW_CONTROL_H
#define AGS_ENGINE_GUI_NEW_CONTROL_H

#include "ags/shared/gfx/bitmap.h"

namespace AGS3 {

using namespace AGS; // FIXME later

struct NewControl {
	int x, y, wid, hit, state, typeandflags, wlevel;
	int8 visible, enabled;        // not implemented
	int8 needredraw;
	virtual void draw(Shared::Bitmap *ds) = 0;
	virtual int pressedon(int mousex, int mousey) = 0;
	virtual int processmessage(int, int, NumberPtr) = 0;

	NewControl(int xx, int yy, int wi, int hi);
	NewControl();
	virtual ~NewControl() {}
	int mouseisinarea(int mousex, int mousey);
	void drawifneeded();
	void drawandmouse();
};

} // namespace AGS3

#endif
