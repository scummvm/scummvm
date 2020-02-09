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

#ifndef NUVIE_CORE_MSG_SCROLL_H
#define NUVIE_CORE_MSG_SCROLL_H

#include "ultima/nuvie/misc/call_back.h"
#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/shared/std/containers.h"

#define MSGSCROLL_U6_WIDTH 17
#define MSGSCROLL_U6_HEIGHT 10

#define MSGSCROLL_MD_WIDTH 16
#define MSGSCROLL_MD_HEIGHT 8

#define MSGSCROLL_SE_WIDTH 16
#define MSGSCROLL_SE_HEIGHT 8

#define MSGSCROLL_CURSOR_DELAY 6 // used to slow down the animated cursor

#define MSGSCROLL_SCROLLBACK_HEIGHT 100

#define MSGSCROLL_NO_MAP_DISPLAY false

namespace Ultima {
namespace Nuvie {

using Std::list;


class Configuration;
class Font;
class Actor;

class MsgText {
public:

	Font *font;
	Std::string s;
	uint8 color;

	MsgText();
	MsgText(Std::string new_string, Font *f);
	~MsgText();

	void append(Std::string new_string);
	void copy(MsgText *msg_text);
	uint32 length();

	uint16 getDisplayWidth();
	bool operator<(const MsgText &rhs) const {
		return (s < rhs.s);
	}
};

class MsgLine {
public:

	Std::list<MsgText *> text;
	uint32 total_length;

	MsgLine() {
		total_length = 0;
	};
	~MsgLine();

	void append(MsgText *new_text);
	void remove_char();
	uint32 length();
	MsgText *get_text_at_pos(uint16 pos);
	uint16 get_display_width();
};

class MsgScroll: public GUI_Widget, public CallBack {
protected:
	Configuration *config;
	int game_type;
	Font *font;
	uint8 font_color;
	uint8 font_highlight_color;
	uint16 scroll_height;
	uint16 scroll_width;
	uint8 left_margin; // margin width in pixels

// set by request_input()
	CallBack *callback_target;
	char *callback_user_data;
	uint8 input_char;
	bool input_mode;
	const char *permit_input; // character list, or 0 = any string
	bool yes_no_only, aye_nay_only, numbers_only; // limited input selection

	bool page_break;
	bool just_finished_page_break;
	bool just_displayed_prompt;
	virtual void process_page_break();
	Std::list<MsgLine *> msg_buf;

	Std::string input_buf;
	bool permit_inputescape; // can RETURN or ESCAPE be used to escape input entry

	uint16 cursor_wait;

	uint16 scrollback_height;
	bool discard_whitespace;
	bool using_target_cursor;

	uint8 bg_color;
	bool talking;

private:
	uint16 screen_x; //x offset to top left corner of MsgScroll
	uint16 screen_y; //y offset to top left corner of MsgScroll



	bool keyword_highlight;




	MsgText prompt;
	Std::list<MsgText *> holding_buffer;


	bool show_cursor;
	bool autobreak; // if true, a page break will be added when the scroll is full






	bool scroll_updated;
	uint8 cursor_char;
	uint16 cursor_x, cursor_y;




	uint16 line_count; // count the number of lines since last page break.

	uint16 display_pos;


	bool capitalise_next_letter;




public:

	MsgScroll(Configuration *cfg, Font *f);
	MsgScroll() : GUI_Widget(NULL, 0, 0, 0, 0) {
		config = NULL;
		game_type = 0;
		font = NULL;
		scroll_height = 0;
		scroll_width = 0;
		callback_target = NULL;
		callback_user_data = NULL;
		input_mode = false;
		permit_input = NULL;
		page_break = false;
		just_finished_page_break = false;
		permit_inputescape = false;
		cursor_wait = 0;
		screen_x = 0;
		screen_y = 0;
		bg_color = 0;
		keyword_highlight = true;
		talking = false;
		show_cursor = false;
		autobreak = false;
		scroll_updated = false;
		cursor_char = 0;
		cursor_x = 0;
		cursor_y = 0;
		line_count = 0;
		display_pos = 0;
		capitalise_next_letter = false;
		just_displayed_prompt = false;
		scrollback_height = MSGSCROLL_SCROLLBACK_HEIGHT;
		discard_whitespace = false;
		left_margin = 0;
	}
	~MsgScroll() override;

	void init(Configuration *cfg, Font *f);

	bool init(const char *player_name);
	void page_up();
	void page_down();
	virtual void move_scroll_down();
	virtual void move_scroll_up();
	void set_using_target_cursor(bool val) {
		using_target_cursor = val;
	}

	void process_holding_buffer();

	MsgText *holding_buffer_get_token();
	bool is_holding_buffer_empty() {
		return holding_buffer.empty();
	}
	virtual bool can_display_prompt() {
		return (!just_displayed_prompt);
	}

	virtual bool parse_token(MsgText *token);
	void add_token(MsgText *token);
	bool remove_char();

	virtual void set_font(uint8 font_type);
	virtual bool is_garg_font();

	int print(const Std::string format, ...);

	virtual void display_string(Std::string s, Font *f, bool include_on_map_window);
	void display_string(Std::string s, Font *f, uint8 color, bool include_on_map_window);
	void display_string(Std::string s, uint16 length, uint8 lang_num);
	void display_string(Std::string s, bool include_on_map_window = true);
	void display_string(Std::string s, uint8 color, bool include_on_map_window);
	void display_fmt_string(const char *format, ...);
	void message(const char *string) {
		display_string(string);
		display_prompt();
	}

	bool set_prompt(const char *new_prompt, Font *f = NULL);
	virtual void display_prompt();
	virtual void display_converse_prompt();

	void set_keyword_highlight(bool state);

	void set_input_mode(bool state, const char *allowed = NULL,
	                    bool can_escape = true, bool use_target_cursor = false,
	                    bool set_numbers_only_to_true = false);
	virtual void set_talking(bool state,  Actor *actor = NULL) {
		talking = state;
		input_char = 0;
	}
	bool is_talking() {
		return talking;
	}
	void set_show_cursor(bool state) {
		show_cursor = state;
	}

	void set_autobreak(bool state) {
		autobreak = state;
	}
	void set_discard_whitespace(bool discard) {
		discard_whitespace = discard;
	}

	bool get_page_break() {
		return (page_break);
	}

	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseWheel(sint32 x, sint32 y) override;
	virtual Std::string get_token_string_at_pos(uint16 x, uint16 y);
//void updateScroll();
	void Display(bool full_redraw) override;

	void clearCursor(uint16 x, uint16 y);
	virtual void drawCursor(uint16 x, uint16 y);

	void set_page_break();

	virtual bool input_buf_add_char(char c);
	virtual bool input_buf_remove_char();

	/* Converse uses this to tell if the scroll has finished displaying the converse dialog */
	virtual bool is_converse_finished() {
		return true;
	}

	bool has_input();
	Std::string get_input();
	const char *peek_at_input();
	void request_input(CallBack *caller, void *user_data);
	void cancel_input_request() {
		request_input(NULL, NULL);
	}
	void clear_scroll();

protected:

	void set_scroll_dimensions(uint16 w, uint16 h);
	void delete_front_line();
	virtual MsgLine *add_new_line();
	void drawLine(Screen *screen, MsgLine *msg_line, uint16 line_y);
	inline void clear_page_break();

	virtual void set_permitted_input(const char *allowed);
	virtual void clear_permitted_input();
	virtual bool can_fit_token_on_msgline(MsgLine *msg_line, MsgText *token);
	void increase_input_char();
	void decrease_input_char();
	uint8 get_char_from_input_char();
	virtual uint8 get_input_font_color() {
		return font_color;
	}


};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
