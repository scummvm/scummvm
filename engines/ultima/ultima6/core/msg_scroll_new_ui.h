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

#ifndef ULTIMA6_CORE_MSG_SCROLL_NEW_UI_H
#define ULTIMA6_CORE_MSG_SCROLL_NEW_UI_H

#include "ultima/ultima6/misc/call_back.h"
#include "ultima/ultima6/gui/gui_widget.h"
//#include <stdarg.h>

//#include <list>
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima6 {

using std::list;

class Configuration;
class Font;
class MsgScroll;
class Actor;
class CallBack;

typedef enum {
	SCROLL_UP,
	SCROLL_DOWN,
	SCROLL_ESCAPE
} ScrollEventType;

class MsgScrollNewUI: public MsgScroll {

	CallBack *timer;

	Font *font_normal;
	Font *font_garg;

	bool solid_bg;
	uint8 bg_color;
	uint8 border_color;
	uint16 position;

	std::string trailing_whitespace;

public:

	MsgScrollNewUI(Configuration *cfg, Screen *s);
	~MsgScrollNewUI();

	uint16 callback(uint16 msg, CallBack *caller, void *data);
	bool can_display_prompt() {
		return false;
	}

	bool can_fit_token_on_msgline(MsgLine *msg_line, MsgText *token);

	void Display(bool full_redraw);

	void display_prompt() {}

	void display_string(std::string s, Font *f, bool include_on_map_window);

	void set_font(uint8 font_type);
	bool is_garg_font();

	GUI_status KeyDown(Common::KeyState key);
	GUI_status MouseDown(int x, int y, int button);

	void move_scroll_down() {
		scroll_movement_event(SCROLL_DOWN);
	}
	void move_scroll_up() {
		scroll_movement_event(SCROLL_UP);
	}

protected:


	MsgLine *add_new_line();

private:
	GUI_status scroll_movement_event(ScrollEventType event);
	uint16 count_empty_lines(std::string s);

};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
