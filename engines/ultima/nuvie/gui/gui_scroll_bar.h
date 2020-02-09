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

#ifndef NUVIE_GUI_GUI_SCROLLBAR_H
#define NUVIE_GUI_GUI_SCROLLBAR_H



#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/screen/screen.h"

namespace Ultima {
namespace Nuvie {

#define GUI_DIALOG_MOVABLE true

#define SCROLLBAR_WIDTH 14

// Callback message types

#define SCROLLBAR_CB_DOWN_BUTTON  0x1
#define SCROLLBAR_CB_UP_BUTTON    0x2
#define SCROLLBAR_CB_SLIDER_MOVED 0x3
#define SCROLLBAR_CB_PAGE_DOWN    0x4
#define SCROLLBAR_CB_PAGE_UP      0x5

class GUI_Button;

class GUI_ScrollBar : public GUI_Widget {
	GUI_CallBack *callback_object;
	GUI_Button *up_button, *down_button;

	bool drag;
	uint16 button_height;

	// Various colours.

	uint32 slider_highlight_c;
	uint32 slider_shadow_c;
	uint32 slider_base_c;
	uint32 track_border_c;
	uint32 track_base_c;

	uint16 track_length;
	uint16 slider_length;
	uint16 slider_y;
	uint16 slider_click_offset; // where on the slider were we clicked?

public:
	/* Passed the area, color and shape */
	GUI_ScrollBar(int x, int y, int h, GUI_CallBack *callback);

	void set_slider_length(float percentage);
	void set_slider_position(float percentage);

	/* Map the color to the display */
	void SetDisplay(Screen *s) override;

	/* Show the widget  */
	void Display(bool full_redraw) override;

	/* events, used for dragging the area. */
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseMotion(int x, int y, uint8 state) override;
	GUI_status MouseWheel(sint32 x, sint32 y) override;

protected:
	void loadButtons();
	void DisplaySlider();

	void send_slider_moved_msg();
	bool move_slider(int new_slider_y);
	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;

	void send_up_button_msg();
	void send_down_button_msg();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
