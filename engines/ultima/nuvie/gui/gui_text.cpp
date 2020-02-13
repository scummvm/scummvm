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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/gui/gui_text.h"
#include "ultima/nuvie/gui/gui_font.h"

namespace Ultima {
namespace Nuvie {

GUI_Text:: GUI_Text(int x, int y, uint8 r, uint8 g, uint8 b, GUI_Font *gui_font, uint16 line_length)
	: GUI_Widget(NULL, x, y, 0, 0) {
	R = r;
	G = g;
	B = b;
	text = NULL;
	max_width = line_length;

	font = gui_font;
}


GUI_Text:: GUI_Text(int x, int y, uint8 r, uint8 g, uint8 b, const char *str, GUI_Font *gui_font, uint16 line_length)
	: GUI_Widget(NULL, x, y, 0, 0) {
	int w, h;

	R = r;
	G = g;
	B = b;
	text = NULL;
	max_width = line_length;

	font = gui_font;

	text = scumm_strdup(str);

	if (text == NULL) {
		DEBUG(0, LEVEL_ERROR, "GUI_Text: failed to allocate memory for text\n");
		return;
	}

	font->textExtent(text, &w, &h, max_width);

	area.setWidth(w);
	area.setHeight(h);
}

GUI_Text::~GUI_Text() {
	delete[] text;
}


/* Show the widget  */
void GUI_Text:: Display(bool full_redraw) {
	font->setTransparency(true);
	font->setColoring(R, G, B);
	font->textOut(surface, area.left, area.top, text, max_width);

	DisplayChildren();
}

} // End of namespace Nuvie
} // End of namespace Ultima
