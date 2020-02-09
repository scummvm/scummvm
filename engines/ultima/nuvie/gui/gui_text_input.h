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

#ifndef NUVIE_GUI_GUI_TEXT_INPUT_H
#define NUVIE_GUI_GUI_TEXT_INPUT_H

#include "ultima/nuvie/gui/gui_text.h"

namespace Ultima {
namespace Nuvie {

class GUI_Font;


#define TEXTINPUT_CB_TEXT_READY 0x1

class GUI_TextInput : public GUI_Text {
protected:
	uint16 max_height;
	uint16 pos;
	uint16 length;

	GUI_CallBack *callback_object;

	uint32 cursor_color;
	uint32 selected_bgcolor;

public:

	GUI_TextInput(int x, int y, uint8 r, uint8 g, uint8 b,
	              const char *str, GUI_Font *gui_font, uint16 width, uint16 height, GUI_CallBack *callback);
	~GUI_TextInput() override;

	void release_focus() override;

	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status KeyDown(const Common::KeyState &key) override;

	void add_char(char c);
	void remove_char();
	void set_text(const char *new_text);
	char *get_text() {
		return text;
	}
	void SetDisplay(Screen *s) override;
	void display_cursor();

	/* Show the widget  */
	void Display(bool full_redraw) override;

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
