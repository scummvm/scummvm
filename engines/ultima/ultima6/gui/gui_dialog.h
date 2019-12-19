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

#ifndef ULTIMA6_GUI_GUI_DIALOG_H
#define ULTIMA6_GUI_GUI_DIALOG_H



#include "ultima/ultima6/gui/gui_widget.h"
#include "ultima/ultima6/screen/screen.h"

namespace Ultima {
namespace Ultima6 {

#define GUI_DIALOG_MOVABLE true
#define GUI_DIALOG_UNMOVABLE false

class GUI_Dialog : public GUI_Widget {

	int old_x, old_y;
	int button_x, button_y;
	uint8 R, G, B;
	uint32 bg_color;

	bool drag;
	Graphics::ManagedSurface *border[8];

	unsigned char *backingstore;

	Common::Rect backingstore_rect;
public:
	/* Passed the area, color and shape */
	GUI_Dialog(int x, int y, int w, int h, uint8 r, uint8 g, uint8 b, bool is_moveable);
	~GUI_Dialog();
	/* Map the color to the display */
	virtual void SetDisplay(Screen *s);

	/* Show the widget  */
	virtual void Display(bool full_redraw);

	/* events, used for dragging the area. */
	GUI_status MouseDown(int x, int y, MouseButton button);
	GUI_status MouseUp(int x, int y, MouseButton button);
	GUI_status MouseMotion(int x, int y, uint8 state);
	void MoveRelative(int dx, int dy);
protected:
	bool can_drag;
	void loadBorderImages();
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
