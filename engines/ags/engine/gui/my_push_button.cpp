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

#include "ags/engine/gui/my_push_button.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/timer.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gui/gui_dialog.h"
#include "ags/engine/gui/gui_dialog_defines.h"
#include "ags/engine/main/game_run.h"

#include "ags/engine/platform/base/ags_platform_driver.h"

namespace AGS3 {

using AGS::Shared::Bitmap;

MyPushButton::MyPushButton(int xx, int yy, int wi, int hi, const char *tex) {                             //wlevel=2;
	x = xx;
	y = yy;
	wid = wi;
	hit = hi + 1;               //hit=hi;
	state = 0;
	snprintf(text, sizeof(text), "%s", tex);
}

void MyPushButton::draw(Bitmap *ds) {
	color_t text_color = ds->GetCompatibleColor(0);
	color_t draw_color = ds->GetCompatibleColor(COL254);
	ds->FillRect(Rect(x, y, x + wid, y + hit), draw_color);
	if (state == 0)
		draw_color = ds->GetCompatibleColor(_G(pushbuttondarkcolor));
	else
		draw_color = ds->GetCompatibleColor(_G(pushbuttonlightcolor));

	ds->DrawRect(Rect(x, y, x + wid, y + hit), draw_color);
	if (state == 0)
		draw_color = ds->GetCompatibleColor(_G(pushbuttonlightcolor));
	else
		draw_color = ds->GetCompatibleColor(_G(pushbuttondarkcolor));

	ds->DrawLine(Line(x, y, x + wid - 1, y), draw_color);
	ds->DrawLine(Line(x, y, x, y + hit - 1), draw_color);
	wouttextxy(ds, x + (wid / 2 - get_text_width(text, _G(cbuttfont)) / 2), y + 2, _G(cbuttfont), text_color, text);
	if (typeandflags & CNF_DEFAULT)
		draw_color = ds->GetCompatibleColor(0);
	else
		draw_color = ds->GetCompatibleColor(_G(windowbackgroundcolor));

	ds->DrawRect(Rect(x - 1, y - 1, x + wid + 1, y + hit + 1), draw_color);
}

int MyPushButton::pressedon(int mx, int my) {
	int wasstat;
	while (!ags_misbuttondown(kMouseLeft) == 0) {

		wasstat = state;
		state = mouseisinarea(mx, my);
		update_polled_stuff();
		if (wasstat != state) {
			draw(get_gui_screen());
		}

		refresh_gui_screen();

		WaitForNextFrame();
	}
	wasstat = state;
	state = 0;
	draw(get_gui_screen());
	return wasstat;
}

int MyPushButton::processmessage(int /*mcode*/, int /*wParam*/, NumberPtr /*lParam*/) {
	return -1;                  // doesn't support messages
}

} // namespace AGS3
