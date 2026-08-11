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

#include "ags/engine/gui/my_textbox.h"
#include "ags/shared/ac/keycode.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gui/gui_dialog_defines.h"

#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

MyTextBox::MyTextBox(int xx, int yy, int wii, const char *tee) {
	x = xx;
	y = yy;
	wid = wii;
	if (tee != nullptr)
		Common::strcpy_s(text, tee);
	else
		text[0] = 0;

	hit = TEXT_HT + 1;
}

void MyTextBox::draw(Bitmap *ds) {
	ds->SetClip(RectWH(x, y, wid + 1, hit + 1));
	color_t draw_color = ds->GetCompatibleColor(_G(windowbackgroundcolor));
	ds->FillRect(Rect(x, y, x + wid, y + hit), draw_color);
	draw_color = ds->GetCompatibleColor(0);
	ds->DrawRect(Rect(x, y, x + wid, y + hit), draw_color);
	color_t text_color = ds->GetCompatibleColor(0);
	wouttextxy(ds, x + 2, y + 1, _G(cbuttfont), text_color, text);

	char tbu[2] = "_";
	wouttextxy(ds, x + 2 + get_text_width(text, _G(cbuttfont)), y + 1, _G(cbuttfont), text_color, tbu);
	ds->ResetClip();
}

int MyTextBox::pressedon(int /*mx*/, int /*my*/) {
	return 0;
}

int MyTextBox::processmessage(int mcode, int wParam, NumberPtr lParam) {

	if (mcode == CTB_SETTEXT) {
		snprintf(text, sizeof(text), "%s", (const char *)lParam.ptr());
		needredraw = 1;
	} else if (mcode == CTB_GETTEXT)
		Common::strcpy_s((char *)lParam.ptr(), 260, text); // FIXME! dangerous
	else if (mcode == CTB_KEYPRESS) {
		// NOTE: this deprecated control does not support UTF-8
		int key = wParam;
		int uchar = lParam;
		size_t len = strlen(text);
		if (key == eAGSKeyCodeBackspace) {
			if (len > 0)
				text[len - 1] = 0;
			drawandmouse();
			return 0;
		}

		if (len >= TEXTBOX_MAXLEN - 1)
			return 0; // buffer full;
		if (uchar == 0)
			return 0; // not a textual event
		if ((uchar >= 128) && (!font_supports_extended_characters(_G(cbuttfont))))
			return 0; // unsupported letter
		if (get_text_width(text, _G(cbuttfont)) >= wid - 5)
			return 0; // not enough control space
		text[len] = uchar;
		text[len + 1] = 0;
		drawandmouse();
	} else
		return -1;

	return 0;
}

} // namespace AGS3
