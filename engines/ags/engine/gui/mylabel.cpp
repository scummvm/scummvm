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

#include <string.h>
#include "ac/display.h"
#include "ac/gamesetup.h"
#include "ac/string.h"
#include "font/fonts.h"
#include "gui/guidefines.h"
#include "gui/mylabel.h"
#include "gui/guidialoginternaldefs.h"

namespace AGS3 {

using namespace Shared;

extern GameSetup usetup;

extern int acdialog_font;

MyLabel::MyLabel(int xx, int yy, int wii, const char *tee) {
	strncpy(text, tee, 150);
	text[149] = 0;
	x = xx;
	y = yy;
	wid = wii;
	hit = TEXT_HT;
}

void MyLabel::draw(Bitmap *ds) {
	int cyp = y;
	char *teptr = &text[0];
	color_t text_color = ds->GetCompatibleColor(0);

	if (break_up_text_into_lines(teptr, Lines, wid, acdialog_font) == 0)
		return;
	for (size_t ee = 0; ee < Lines.Count(); ee++) {
		wouttext_outline(ds, x, cyp, acdialog_font, text_color, Lines[ee]);
		cyp += TEXT_HT;
	}
}

int MyLabel::pressedon(int mousex, int mousey) {
	return 0;
}

int MyLabel::processmessage(int mcode, int wParam, long lParam) {
	return -1;                  // doesn't support messages
}

} // namespace AGS3
