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

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/gui_button.h"
#include "ultima/nuvie/gui/gui_scroll_bar.h"

namespace Ultima {
namespace Nuvie {

// Colours

#define SLIDER_HIGHLIGHT_R 245
#define SLIDER_HIGHLIGHT_G 247
#define SLIDER_HIGHLIGHT_B 201

#define SLIDER_SHADOW_R 115
#define SLIDER_SHADOW_G 116
#define SLIDER_SHADOW_B 94

#define SLIDER_BASE_R 183
#define SLIDER_BASE_G 185
#define SLIDER_BASE_B 150

#define TRACK_BORDER_R 76
#define TRACK_BORDER_G 75
#define TRACK_BORDER_B 71

#define TRACK_BASE_R 137
#define TRACK_BASE_G 139
#define TRACK_BASE_B 113


GUI_ScrollBar::GUI_ScrollBar(int x, int y, int h, GUI_CallBack *callback)
	: GUI_Widget(NULL, x, y, SCROLLBAR_WIDTH, h) {
	callback_object = callback;
	drag = false;

	loadButtons();

	track_length = area.height() - 2 * button_height;
	slider_length = track_length / 2;
	slider_y = 5;

}


void GUI_ScrollBar::loadButtons() {
	Std::string datadir = GUI::get_gui()->get_data_dir();
	Std::string imagefile;
	Graphics::ManagedSurface *image, *image1;

	build_path(datadir, "ScrollBarUp_1.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	build_path(datadir, "ScrollBarUp_2.bmp", imagefile);
	image1 = SDL_LoadBMP(imagefile.c_str());

	up_button = new GUI_Button(NULL, 0, 0, image, image1, this);
	this->AddWidget(up_button);

	build_path(datadir, "ScrollBarDown_1.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	build_path(datadir, "ScrollBarDown_2.bmp", imagefile);
	image1 = SDL_LoadBMP(imagefile.c_str());

	button_height = image->h;

	down_button = new GUI_Button(NULL, 0, area.height() - button_height, image, image1, this);
	this->AddWidget(down_button);

	return;
}

/* Map the color to the display */
void GUI_ScrollBar::SetDisplay(Screen *s) {
	GUI_Widget::SetDisplay(s);

	slider_highlight_c = SDL_MapRGB(surface->format, SLIDER_HIGHLIGHT_R, SLIDER_HIGHLIGHT_G, SLIDER_HIGHLIGHT_B);
	slider_shadow_c = SDL_MapRGB(surface->format, SLIDER_SHADOW_R, SLIDER_SHADOW_G, SLIDER_SHADOW_B);
	slider_base_c = SDL_MapRGB(surface->format, SLIDER_BASE_R, SLIDER_BASE_G, SLIDER_BASE_B);

	track_border_c = SDL_MapRGB(surface->format, TRACK_BORDER_R, TRACK_BORDER_G, TRACK_BORDER_B);
	track_base_c = SDL_MapRGB(surface->format, TRACK_BASE_R, TRACK_BASE_G, TRACK_BASE_B);
}

void GUI_ScrollBar::set_slider_length(float percentage) {
	slider_length = (int)((float)track_length * percentage);
}

void GUI_ScrollBar::set_slider_position(float percentage) {
	move_slider((int)((float)track_length * percentage));
}


/* Show the widget  */
void GUI_ScrollBar::Display(bool full_redraw) {
	Common::Rect framerect;
// Common::Rect src, dst;

	if (slider_y > 0) {
		framerect.left = area.left;
		framerect.top = area.top + button_height;
		framerect.setWidth(SCROLLBAR_WIDTH);
		framerect.setHeight(slider_y);
		SDL_FillRect(surface, &framerect, track_base_c);

		// Draw Border
		framerect.left = area.left;
		framerect.top = area.top + button_height;
		framerect.setHeight(SCROLLBAR_WIDTH);
		framerect.setHeight(1);
		SDL_FillRect(surface, &framerect, track_border_c);

		framerect.left = area.left;
		framerect.top = area.top + button_height;
		framerect.setWidth(1);
		framerect.setHeight(slider_y);
		SDL_FillRect(surface, &framerect, track_border_c);

		framerect.left = area.left + SCROLLBAR_WIDTH - 1;
		framerect.top = area.top + button_height;
		framerect.setWidth(1);
		framerect.setHeight(slider_y);
		SDL_FillRect(surface, &framerect, track_border_c);

	}

	DisplaySlider();

	if (slider_y + slider_length < track_length) {
		framerect.left = area.left;
		framerect.top = area.top + button_height + slider_y + slider_length;
		framerect.setWidth(SCROLLBAR_WIDTH);
		framerect.setHeight(track_length - (slider_y + slider_length));
		SDL_FillRect(surface, &framerect, track_base_c);

		// Draw Border
		framerect.left = area.left;
		framerect.top = area.top + area.height() - button_height - 1;
		framerect.setWidth(SCROLLBAR_WIDTH);
		framerect.setHeight(1);
		SDL_FillRect(surface, &framerect, track_border_c);

		framerect.left = area.left;
		framerect.top = area.top + button_height + slider_y + slider_length;
		framerect.setWidth(1);
		framerect.setHeight(track_length - slider_y - slider_length);
		SDL_FillRect(surface, &framerect, track_border_c);

		framerect.left = area.left + SCROLLBAR_WIDTH - 1;
		framerect.top = area.top + button_height + slider_y + slider_length;
		framerect.setWidth(1);
		framerect.setHeight(track_length - slider_y - slider_length);
		SDL_FillRect(surface, &framerect, track_border_c);

	}

	DisplayChildren();

	screen->update(area.left, area.top, area.width(), area.height());

	return;
}

inline void GUI_ScrollBar::DisplaySlider() {
	Common::Rect rect;

	rect.left = area.left;
	rect.top = area.top + button_height + slider_y;
	rect.setWidth(SCROLLBAR_WIDTH);
	rect.setHeight(slider_length);
	SDL_FillRect(surface, &rect, slider_base_c);

	rect.left = area.left;
	rect.top = area.top + button_height + slider_y;
	rect.setWidth(1);
	rect.setHeight(slider_length - 1);
	SDL_FillRect(surface, &rect, slider_highlight_c);

	rect.left = area.left + 1;
	rect.top = area.top + button_height + slider_y;
	rect.setWidth(SCROLLBAR_WIDTH - 1);
	rect.setHeight(1);
	SDL_FillRect(surface, &rect, slider_highlight_c);

	rect.left = area.left + SCROLLBAR_WIDTH - 1;
	rect.top = area.top + button_height + slider_y;
	rect.setWidth(1);
	rect.setHeight(slider_length);
	SDL_FillRect(surface, &rect, slider_shadow_c);

	rect.left = area.left;
	rect.top = area.top + button_height + slider_y + slider_length - 1;
	rect.setWidth(SCROLLBAR_WIDTH - 1);
	rect.setHeight(1);
	SDL_FillRect(surface, &rect, slider_shadow_c);

}
GUI_status GUI_ScrollBar::MouseWheel(sint32 x, sint32 y) {
	if (y > 0) {
		send_up_button_msg();
	} else if (y < 0) {
		send_down_button_msg();
	}

	return GUI_YUM;
}

GUI_status GUI_ScrollBar::MouseDown(int x, int y, Shared::MouseButton button) {
	if (y >= area.top + button_height + slider_y && y <= area.top + button_height + slider_y + slider_length) {
		drag = true;
		slider_click_offset = y - area.top - button_height - slider_y;
		grab_focus();
	} else if (y < area.top + button_height + slider_y)
		callback_object->callback(SCROLLBAR_CB_PAGE_UP, this, NULL);
	else
		callback_object->callback(SCROLLBAR_CB_PAGE_DOWN, this, NULL);

	return GUI_YUM;
}

GUI_status GUI_ScrollBar::MouseUp(int x, int y, Shared::MouseButton button) {
	drag = false;

	release_focus();

	return GUI_YUM;
}

GUI_status GUI_ScrollBar::MouseMotion(int x, int y, uint8 state) {
	int new_slider_y;

	if (!drag)
		return GUI_PASS;

	new_slider_y = y - slider_click_offset - (area.top + button_height);

	if (move_slider(new_slider_y)) {
		send_slider_moved_msg();
	}
// Redraw();

	return (GUI_YUM);
}

inline bool GUI_ScrollBar::move_slider(int new_slider_y) {
	if (new_slider_y < 0 || new_slider_y + slider_length > track_length) { //new slider position is out of bounds.
		if (new_slider_y < 0)
			new_slider_y = 0;
		else
			new_slider_y = track_length - slider_length;
	}

	if (slider_y == new_slider_y)
		return false;

	slider_y = new_slider_y;

	return true;
}

void GUI_ScrollBar::send_slider_moved_msg() {
	float slider_percentage;

	slider_percentage = slider_y / (float)track_length;

	callback_object->callback(SCROLLBAR_CB_SLIDER_MOVED, this, &slider_percentage);

	return;
}

void GUI_ScrollBar::send_up_button_msg() {
	callback_object->callback(SCROLLBAR_CB_UP_BUTTON, this, NULL);
}

void GUI_ScrollBar::send_down_button_msg() {
	callback_object->callback(SCROLLBAR_CB_DOWN_BUTTON, this, NULL);
}

GUI_status GUI_ScrollBar::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	if (msg == BUTTON_CB) {
		if (caller == up_button)
			send_up_button_msg();

		if (caller == down_button)
			send_down_button_msg();
	}

	return GUI_YUM;
}

} // End of namespace Nuvie
} // End of namespace Ultima

