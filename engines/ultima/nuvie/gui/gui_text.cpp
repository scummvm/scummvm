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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/gui/gui_text.h"
#include "ultima/nuvie/gui/gui_font.h"

namespace Ultima {
namespace Nuvie {

GUI_Text::GUI_Text(int x, int y, uint8 r, uint8 g, uint8 b, GUI_Font *gui_font, uint16 line_length)
	: GUI_Widget(nullptr, x, y, 0, 0) {
	R = r;
	G = g;
	B = b;
	text = nullptr;
	max_width = line_length;

	font = gui_font;
}


GUI_Text::GUI_Text(int x, int y, uint8 r, uint8 g, uint8 b, const char *str, GUI_Font *gui_font, uint16 line_length)
	: GUI_Widget(nullptr, x, y, 0, 0), R(r), G(g), B(b), max_width(line_length),
	  font(gui_font), text(nullptr) {
	setText(str);
}

GUI_Text::~GUI_Text() {
	free(text);
}


/* Show the widget  */
void GUI_Text::Display(bool full_redraw) {
	font->setTransparency(true);
	font->setColoring(R, G, B);
	font->textOut(surface, area.left, area.top, text, max_width);

	DisplayChildren();
}

void GUI_Text::setText(const char *txt) {
	if (text)
		free(text);
	text = scumm_strdup(txt);
	if (text == nullptr)
		error("GUI_Text: failed to allocate memory for text");

	int w, h;
	font->textExtent(text, &w, &h, max_width);

	area.setWidth(w);
	area.setHeight(h);
}

} // End of namespace Nuvie
} // End of namespace Ultima
