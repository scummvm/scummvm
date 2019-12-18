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

#ifndef ULTIMA6_GUI_GUI_CONSOLE_H
#define ULTIMA6_GUI_GUI_CONSOLE_H



#include "ultima/ultima6/gui/gui_widget.h"
#include "ultima/ultima6/gui/gui_types.h"
#include "ultima/ultima6/gui/gui_font.h"
#include "ultima/ultima6/screen/screen.h"

namespace Ultima {
namespace Ultima6 {

class GUI_Console : public GUI_Widget {

	GUI_Color *bg_color;
	GUI_Font *font;
	uint16 num_cols;
	uint16 num_rows;
	std::list<std::string> data;

public:
	GUI_Console(uint16 x, uint16 y, uint16 w, uint16 h);
	~GUI_Console();

	/* Map the color to the display */
	virtual void SetDisplay(Screen *s);

	/* Show the widget  */
	virtual void Display(bool full_redraw);

	/* events, used for dragging the area. */
	GUI_status MouseDown(int x, int y, int button);
	GUI_status MouseUp(int x, int y, int button);
	GUI_status MouseMotion(int x, int y, uint8 state);

	virtual void AddLine(std::string line);

protected:

};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
