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

#ifndef NUVIE_GUI_GUI_SCROLLER_H
#define NUVIE_GUI_GUI_SCROLLER_H



#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/screen/screen.h"

namespace Ultima {
namespace Nuvie {

class GUI_ScrollBar;

class GUI_Scroller : public GUI_Widget {

	uint8 R, G, B;
	uint32 bg_color;
	uint16 row_height;
	uint16 rows_per_page;
	uint16 num_rows;
	GUI_ScrollBar *scroll_bar;
	uint16 disp_offset;

public:
	/* Passed the area, color and shape */
	GUI_Scroller(int x, int y, int w, int h, uint8 r, uint8 g, uint8 b, uint16 r_height);

	/* Map the color to the display */
	void SetDisplay(Screen *s) override;
	void PlaceOnScreen(Screen *s, GUI_DragManager *dm, int x, int y) override;
	void move_up();
	void move_down();
	void page_up(bool all = false);
	void page_down(bool all = false);

	int AddWidget(GUI_Widget *widget);

	/* Show the widget  */
	void Display(bool full_redraw) override;

	/* events, used for dragging the area. */
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseMotion(int x, int y, uint8 state) override;
	GUI_status MouseWheel(sint32 x, sint32 y) override;

protected:
	void update_viewport(bool update_slider);
	void move_percentage(float offset_percentage);
	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
