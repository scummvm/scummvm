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

#ifndef AGS_ENGINE_GUI_PUSHBUTTON_H
#define AGS_ENGINE_GUI_PUSHBUTTON_H

#include "ags/engine/gui/newcontrol.h"

namespace AGS3 {

struct MyPushButton : public NewControl {
	char text[50];
	MyPushButton(int xx, int yy, int wi, int hi, const char *tex);
	void draw(Shared::Bitmap *ds) override;
	int pressedon(int mousex, int mousey) override;
	int processmessage(int mcode, int wParam, NumberPtr lParam) override;
};

} // namespace AGS3

#endif
