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

#include "ags/engine/gui/my_label.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/font/fonts.h"
#include "ags/engine/gui/gui_dialog_defines.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace Shared;

MyLabel::MyLabel(int xx, int yy, int wii, const char *tee) {
	snprintf(text, sizeof(text), "%s", tee);
	x = xx;
	y = yy;
	wid = wii;
	hit = TEXT_HT;
}

void MyLabel::draw(Bitmap *ds) {
	int cyp = y;
	char *teptr = &text[0];
	color_t text_color = ds->GetCompatibleColor(0);

	if (break_up_text_into_lines(teptr, _GP(Lines), wid, _G(acdialog_font)) == 0)
		return;
	for (size_t ee = 0; ee < _GP(Lines).Count(); ee++) {
		wouttext_outline(ds, x, cyp, _G(acdialog_font), text_color, _GP(Lines)[ee].GetCStr());
		cyp += TEXT_HT;
	}
}

int MyLabel::pressedon(int mousex, int mousey) {
	return 0;
}

int MyLabel::processmessage(int mcode, int wParam, NumberPtr lParam) {
	return -1;                  // doesn't support messages
}

} // namespace AGS3
