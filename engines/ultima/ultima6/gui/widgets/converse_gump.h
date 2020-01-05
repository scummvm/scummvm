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

#ifndef ULTIMA6_CORE_CONVERSE_GUMP_H
#define ULTIMA6_CORE_CONVERSE_GUMP_H

#include "ultima/ultima6/misc/call_back.h"
#include "ultima/ultima6/gui/widgets/gui_widget.h"
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

class ConverseGump: public MsgScroll {
	std::list<MsgText> conv_keywords;
	std::list<MsgText> permitted_input_keywords;

	std::list<MsgText> *keyword_list;

	unsigned char *npc_portrait;
	unsigned char *avatar_portrait;

	bool found_break_char;
	bool solid_bg;
	bool force_solid_bg;
	uint8 converse_bg_color;

	uint16 cursor_position;

	uint8 portrait_width;
	uint8 portrait_height;
	uint8 frame_w;
	uint8 frame_h;
	uint16 min_w;

	nuvie_game_t game_type;

public:

	ConverseGump(Configuration *cfg, Font *f, Screen *s);
	~ConverseGump();

	void set_actor_portrait(Actor *a);
	unsigned char *create_framed_portrait(Actor *a);
	virtual bool parse_token(MsgText *token);
	virtual std::string get_token_string_at_pos(uint16 x, uint16 y);
	virtual void display_string(std::string s, Font *f, bool include_on_map_window);
	virtual void set_talking(bool state, Actor *actor = NULL);
	virtual void set_font(uint8 font_type) {}
//bool get_solid_bg() { return solid_bg; }
	void set_solid_bg(bool val) {
		solid_bg = val;
	}

	void Display(bool full_redraw);

	GUI_status KeyDown(const Common::KeyState &key);
	GUI_status MouseUp(int x, int y, Shared::MouseButton button);

	GUI_status MouseDown(int x, int y, Shared::MouseButton button) {
		return GUI_YUM;
	}
	GUI_status MouseMotion(int x, int y, uint8 state) {
		return GUI_YUM;
	}
	GUI_status MouseEnter(uint8 state) {
		return GUI_YUM;
	}
	GUI_status MouseLeave(uint8 state) {
		return GUI_YUM;
	}
	GUI_status MouseClick(int x, int y, Shared::MouseButton button) {
		return GUI_YUM;
	}
	GUI_status MouseDouble(int x, int y, Shared::MouseButton button) {
		return GUI_YUM;
	}
	GUI_status MouseDelayed(int x, int y, Shared::MouseButton button) {
		return GUI_YUM;
	}
	GUI_status MouseHeld(int x, int y, Shared::MouseButton button) {
		return GUI_YUM;
	}

	void set_found_break_char(bool val) {
		found_break_char = val;
	}

	virtual bool input_buf_add_char(char c);
	virtual bool input_buf_remove_char();

	virtual bool is_converse_finished() {
		return (is_holding_buffer_empty() && msg_buf.size() == 1 && msg_buf.back()->total_length == 0);
	}

	virtual void drawCursor(uint16 x, uint16 y);

protected:
	std::string strip_whitespace_after_break(std::string s);
	void add_keyword(std::string keyword);

	virtual void set_permitted_input(const char *allowed);
	virtual void clear_permitted_input();

	bool cursor_at_input_section() {
		return (keyword_list && cursor_position == keyword_list->size());
	}
	void cursor_reset() {
		cursor_position = 0;
	}
	void cursor_move_to_input() {
		cursor_position = keyword_list ? keyword_list->size() : 0;
	}

	void input_add_string(std::string token_str);

	std::string get_token_at_cursor();

	bool is_permanent_keyword(std::string keyword);
	void parse_fm_towns_token(MsgText *token);

private:
	unsigned char *get_portrait_data(Actor *a);
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
