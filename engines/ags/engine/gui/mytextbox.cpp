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

//include <string.h>
#include "ags/shared/util/wgt2allg.h"
#include "ags/shared/font/fonts.h"
#include "ags/engine/gui/mytextbox.h"
#include "ags/engine/gui/guidialoginternaldefs.h"
#include "ags/shared/gfx/bitmap.h"

namespace AGS3 {

using AGS::Shared::Bitmap;

extern GameSetup usetup;

extern int windowbackgroundcolor;
extern int cbuttfont;

MyTextBox::MyTextBox(int xx, int yy, int wii, const char *tee) {
	x = xx;
	y = yy;
	wid = wii;
	if (tee != nullptr)
		strcpy(text, tee);
	else
		text[0] = 0;

	hit = TEXT_HT + 1;
}

void MyTextBox::draw(Bitmap *ds) {
	color_t draw_color = ds->GetCompatibleColor(windowbackgroundcolor);
	ds->FillRect(Rect(x, y, x + wid, y + hit), draw_color);
	draw_color = ds->GetCompatibleColor(0);
	ds->DrawRect(Rect(x, y, x + wid, y + hit), draw_color);
	color_t text_color = ds->GetCompatibleColor(0);
	wouttextxy(ds, x + 2, y + 1, cbuttfont, text_color, text);

	char tbu[2] = "_";
	wouttextxy(ds, x + 2 + wgettextwidth(text, cbuttfont), y + 1, cbuttfont, text_color, tbu);
}

int MyTextBox::pressedon(int mousex, int mousey) {
	return 0;
}

int MyTextBox::processmessage(int mcode, int wParam, NumberPtr lParam) {
	if (mcode == CTB_SETTEXT) {
		strcpy(text, (char *)lParam._ptr);
		needredraw = 1;
	} else if (mcode == CTB_GETTEXT)
		strcpy((char *)lParam._ptr, text);
	else if (mcode == CTB_KEYPRESS) {
		if (wParam == 8) {
			if (text[0] != 0)
				text[strlen(text) - 1] = 0;

			drawandmouse();
		} else if (strlen(text) >= TEXTBOX_MAXLEN - 1)
			;
		else if (wgettextwidth(text, cbuttfont) >= wid - 5)
			;
		else if (wParam > 127)
			;  // font only has 128 chars
		else {
			text[strlen(text) + 1] = 0;
			text[strlen(text)] = wParam;
			drawandmouse();
		}
	} else
		return -1;

	return 0;
}

} // namespace AGS3
