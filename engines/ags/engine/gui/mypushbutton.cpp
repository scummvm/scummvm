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
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/mouse.h"
#include "ags/shared/font/fonts.h"
#include "ags/engine/gui/mypushbutton.h"
#include "ags/engine/gui/guidialog.h"
#include "ags/engine/gui/guidialoginternaldefs.h"
#include "ags/engine/main/game_run.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/globals.h"

namespace AGS3 {

using AGS::Shared::Bitmap;

extern int windowbackgroundcolor, pushbuttondarkcolor;
extern int pushbuttonlightcolor;
extern int cbuttfont;

MyPushButton::MyPushButton(int xx, int yy, int wi, int hi, const char *tex) {
	//wlevel=2;
	x = xx;
	y = yy;
	wid = wi;
	hit = hi + 1;               //hit=hi;
	state = 0;
	strncpy(text, tex, 50);
	text[49] = 0;
}

void MyPushButton::draw(Bitmap *ds) {
	color_t text_color = ds->GetCompatibleColor(0);
	color_t draw_color = ds->GetCompatibleColor(COL254);
	ds->FillRect(Rect(x, y, x + wid, y + hit), draw_color);
	if (state == 0)
		draw_color = ds->GetCompatibleColor(pushbuttondarkcolor);
	else
		draw_color = ds->GetCompatibleColor(pushbuttonlightcolor);

	ds->DrawRect(Rect(x, y, x + wid, y + hit), draw_color);
	if (state == 0)
		draw_color = ds->GetCompatibleColor(pushbuttonlightcolor);
	else
		draw_color = ds->GetCompatibleColor(pushbuttondarkcolor);

	ds->DrawLine(Line(x, y, x + wid - 1, y), draw_color);
	ds->DrawLine(Line(x, y, x, y + hit - 1), draw_color);
	wouttextxy(ds, x + (wid / 2 - wgettextwidth(text, cbuttfont) / 2), y + 2, cbuttfont, text_color, text);
	if (typeandflags & CNF_DEFAULT)
		draw_color = ds->GetCompatibleColor(0);
	else
		draw_color = ds->GetCompatibleColor(windowbackgroundcolor);

	ds->DrawRect(Rect(x - 1, y - 1, x + wid + 1, y + hit + 1), draw_color);
}

//extern const int LEFT;  // in mousew32

int MyPushButton::pressedon(int mousex, int mousey) {
	int wasstat;
	while (mbutrelease(LEFT) == 0) {

		wasstat = state;
		state = mouseisinarea(_G(mousex), _G(mousey));
		// stop mp3 skipping if button held down
		update_polled_stuff_if_runtime();
		if (wasstat != state) {
			//        ags_domouse(DOMOUSE_DISABLE);
			draw(get_gui_screen());
			//ags_domouse(DOMOUSE_ENABLE);
		}

		//      ags_domouse(DOMOUSE_UPDATE);

		refresh_gui_screen();

		WaitForNextFrame();
	}
	wasstat = state;
	state = 0;
	//    ags_domouse(DOMOUSE_DISABLE);
	draw(get_gui_screen());
	//  ags_domouse(DOMOUSE_ENABLE);
	return wasstat;
}

int MyPushButton::processmessage(int mcode, int wParam, long lParam) {
	return -1;                  // doesn't support messages
}

} // namespace AGS3
