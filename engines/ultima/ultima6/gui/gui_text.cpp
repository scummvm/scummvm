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

#include "nuvieDefs.h"

#include "GUI_text.h"
#include "GUI_font.h"
#include <stdlib.h>

namespace Ultima {
namespace Ultima6 {

GUI_Text:: GUI_Text(int x, int y, Uint8 r, Uint8 g, Uint8 b, GUI_Font *gui_font, uint16 line_length)
	: GUI_Widget(NULL, x, y, 0, 0) {
	R = r;
	G = g;
	B = b;
	text = NULL;
	max_width = line_length;

	font = gui_font;
}


GUI_Text:: GUI_Text(int x, int y, Uint8 r, Uint8 g, Uint8 b, const char *str, GUI_Font *gui_font, uint16 line_length)
	: GUI_Widget(NULL, x, y, 0, 0) {
	int w, h;

	R = r;
	G = g;
	B = b;
	text = NULL;
	max_width = line_length;

	font = gui_font;

	text = (char *)strdup(str);

	if (text == NULL) {
		DEBUG(0, LEVEL_ERROR, "GUI_Text: failed to allocate memory for text\n");
		return;
	}

	font->TextExtent(text, &w, &h, max_width);

	area.w = w;
	area.h = h;
}

GUI_Text::~GUI_Text() {
	if (text)
		free(text);

	return;
}


/* Show the widget  */
void
GUI_Text:: Display(bool full_redraw) {
	font->SetTransparency(1);
	font->SetColoring(R, G, B);
	font->TextOut(surface, area.x, area.y, text, max_width);

	DisplayChildren();
}

} // End of namespace Ultima6
} // End of namespace Ultima
