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

#ifndef AGS_ENGINE_GUI_MY_TEXTBOX_H
#define AGS_ENGINE_GUI_MY_TEXTBOX_H

#include "ags/engine/gui/new_control.h"

namespace AGS3 {

#define TEXTBOX_MAXLEN 49
struct MyTextBox : public NewControl {
	char text[TEXTBOX_MAXLEN + 1];
	MyTextBox(int xx, int yy, int wii, const char *tee);
	void draw(Shared::Bitmap *ds) override;
	int pressedon(int mousex, int mousey) override;
	int processmessage(int mcode, int wParam, NumberPtr lParam) override;
};

} // namespace AGS3

#endif
