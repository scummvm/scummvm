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
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/fonts/font_manager.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/nuvie/screen/game_palette.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

// MsgText Class
MsgText::MsgText() {
	font = NULL;
	color = 0;
}

MsgText::MsgText(Std::string new_string, Font *f) {
	s.assign(new_string);
	font = f;
	color = 0;
	if (font) {
		color = font->getDefaultColor();
	}
}

MsgText::~MsgText() {
}

void MsgText::append(Std::string new_string) {
	s.append(new_string);
}

void MsgText::copy(MsgText *msg_text) {
	s.assign(msg_text->s);
	font = msg_text->font;
	color = msg_text->color;
}

uint32 MsgText::length() {
	return (uint32)s.length();
}

uint16 MsgText::getDisplayWidth() {
	return font->getStringWidth(s.c_str());
}

MsgLine::~MsgLine() {
	Std::list<MsgText *>::iterator iter;

	for (iter = text.begin(); iter != text.end(); iter++) {
		delete *iter;
	}
}

void MsgLine::append(MsgText *new_text) {
	MsgText *msg_text = NULL;

	if (text.size() > 0)
		msg_text = text.back();

	if (msg_text && msg_text->font == new_text->font && msg_text->color == new_text->color && new_text->s.length() == 1 && new_text->s[0] != ' ')
		msg_text->s.append(new_text->s);
	else {
		msg_text = new MsgText();
		msg_text->copy(new_text);
		text.push_back(msg_text);
	}

	total_length += new_text->s.length();

	return;
}

// remove the last char in the line.
void MsgLine::remove_char() {
	MsgText *msg_text;

	if (total_length == 0)
		return;

	msg_text = text.back();
	msg_text->s.erase(msg_text->s.length() - 1, 1);

	if (msg_text->s.length() == 0) {
		text.pop_back();
		delete msg_text;
	}

	total_length--;

	return;
}

uint32 MsgLine::length() {
	return total_length;
}

// gets the MsgText object that contains the text at line position pos
MsgText *MsgLine::get_text_at_pos(uint16 pos) {
	uint16 i;
	Std::list<MsgText *>::iterator iter;

	if (pos > total_length)
		return NULL;

	for (i = 0, iter = text.begin(); iter != text.end(); iter++) {
		if (i + (*iter)->s.length() >= pos)
			return (*iter);

		i += (*iter)->s.length();
	}

	return NULL;
}

uint16 MsgLine::get_display_width() {
	uint16 len = 0;
	Std::list<MsgText *>::iterator iter;
	for (iter = text.begin(); iter != text.end() ; iter++) {
		MsgText *token = *iter;

		len += token->font->getStringWidth(token->s.c_str());
	}
	return len;
}

// MsgScroll Class

void MsgScroll::init(Configuration *cfg, Font *f) {
	font = f;

	config = cfg;
	config->value("config/GameType", game_type);

	input_char = 0;
	yes_no_only = false;
	aye_nay_only = false;
	numbers_only = false;
	scroll_updated = false;
	discard_whitespace = true;
	page_break = false;
	show_cursor = true;
	talking = false;
	autobreak = false;
	just_finished_page_break = false;
	using_target_cursor = false;

	callback_target = NULL;
	callback_user_data = NULL;

	scrollback_height = MSGSCROLL_SCROLLBACK_HEIGHT;
	capitalise_next_letter = false;

	font_color = FONT_COLOR_U6_NORMAL;
	font_highlight_color = FONT_COLOR_U6_HIGHLIGHT;
	if (game_type != NUVIE_GAME_U6) {
		font_color = FONT_COLOR_WOU_NORMAL;
		font_highlight_color = FONT_COLOR_WOU_HIGHLIGHT;
	}
}

MsgScroll::MsgScroll(Configuration *cfg, Font *f) : GUI_Widget(NULL, 0, 0, 0, 0) {
	uint16 x, y;

	init(cfg, f);

	switch (game_type) {
	case NUVIE_GAME_MD :
		scroll_width = MSGSCROLL_MD_WIDTH;
		scroll_height = MSGSCROLL_MD_HEIGHT;
		x = 184;
		y = 128;
		break;

	case NUVIE_GAME_SE :
		scroll_width = MSGSCROLL_SE_WIDTH;
		scroll_height = MSGSCROLL_SE_HEIGHT;
		x = 184;
		y = 128;
		break;
	case NUVIE_GAME_U6 :
	default :
		scroll_width = MSGSCROLL_U6_WIDTH;
		scroll_height = MSGSCROLL_U6_HEIGHT;
		x = 176;
		y = 112;
		break;
	}

	if (Game::get_game()->is_original_plus())
		x += Game::get_game()->get_game_width() - 320;

	uint16 x_off = Game::get_game()->get_game_x_offset();
	uint16 y_off = Game::get_game()->get_game_y_offset();

	GUI_Widget::Init(NULL, x + x_off, y + y_off, scroll_width * 8, scroll_height * 8);

	cursor_char = 0;
	cursor_x = 0;
	cursor_y = scroll_height - 1;
	line_count = 0;

	cursor_wait = 0;

	display_pos = 0;

	bg_color = Game::get_game()->get_palette()->get_bg_color();

	capitalise_next_letter = false;

	left_margin = 0;

	add_new_line();
}

MsgScroll::~MsgScroll() {
	Std::list<MsgLine *>::iterator msg_line;
	Std::list<MsgText *>::iterator msg_text;

// delete the scroll buffer
	for (msg_line = msg_buf.begin(); msg_line != msg_buf.end(); msg_line++)
		delete *msg_line;

// delete the holding buffer
	for (msg_text = holding_buffer.begin(); msg_text != holding_buffer.end(); msg_text++)
		delete *msg_text;

}

bool MsgScroll::init(const char *player_name) {
	Std::string prompt_string;

	prompt_string.append(player_name);
	if (game_type == NUVIE_GAME_U6) {
		prompt_string.append(":\n");
	}

	prompt_string.append(">");

	if (set_prompt(prompt_string.c_str(), font) == false)
		return false;

	set_input_mode(false);

	return true;
}

void MsgScroll::set_scroll_dimensions(uint16 w, uint16 h) {
	scroll_width = w;
	scroll_height = h;

	cursor_char = 0;
	cursor_x = 0;
	cursor_y = scroll_height - 1;
	line_count = 0;

	cursor_wait = 0;

	display_pos = 0;
}

int MsgScroll::print(Std::string format, ...) {

	va_list ap;
	int printed = 0;
	static size_t bufsize = 128; // will be resized if needed
	static char *buffer = (char *) malloc(bufsize); // static so we don't have to reallocate all the time.

	while (1) {
		if (buffer == NULL) {
			DEBUG(0, LEVEL_ALERT, "MsgScroll::printf: Couldn't allocate %d bytes for buffer\n", bufsize);
			/* try to shrink the buffer to at least have a change next time,
			 * but if we're low on memory probably have worse issues...
			 */
			bufsize >>= 1;
			buffer = (char *) malloc(bufsize); // no need to check now.
			/*
			 * We don't retry, or if we need e.g. 4 bytes for the format and
			 * there's only 3 available, we'd grow and shrink forever.
			 */
			return printed;
		}

		/* try formatting */
		va_start(ap, format);
		printed = vsnprintf(buffer, bufsize, format.c_str(), ap);
		va_end(ap);

		if (printed < 0) {
			DEBUG(0, LEVEL_ERROR, "MsgScroll::printf: vsnprintf returned < 0: either output error or glibc < 2.1\n");
			free(buffer);
			bufsize *= 2; // In case of an output error, we'll just keep doubling until the malloc fails.
			buffer = (char *) malloc(bufsize); // if this fails, will be caught later on
			/* try again */
			continue;
		} else if ((size_t)printed >= bufsize) {
			DEBUG(0, LEVEL_DEBUGGING, "MsgScroll::printf: needed buffer of %d bytes, only had %d bytes.\n", printed + 1, bufsize);
			bufsize = printed + 1; //this should be enough
			free(buffer);
			buffer = (char *) malloc(bufsize); // if this fails, will be caught later on
			/* try again */
			continue;
		}
		/* if we're here, formatting probably worked. We can stop looping */
		break;
	}
	/* use the string */
	display_string(buffer);

	return printed;
}

void MsgScroll::display_fmt_string(const char *format, ...) {
	char buf[1024];
	memset(buf, 0, 1024);
	va_list args;
	va_start(args, format);
	vsnprintf(buf, 1024, format, args);
	va_end(args);

	display_string(buf);
}

void MsgScroll::display_string(Std::string s, uint16 length, uint8 lang_num) {

}

void MsgScroll::display_string(Std::string s, bool include_on_map_window) {
	display_string(s, font, include_on_map_window);
}

void MsgScroll::display_string(Std::string s, uint8 color, bool include_on_map_window) {
	display_string(s, font, color, include_on_map_window);
}

void MsgScroll::display_string(Std::string s, Font *f, bool include_on_map_window) {
	display_string(s, f, font_color, include_on_map_window);
}

void MsgScroll::display_string(Std::string s, Font *f, uint8 color, bool include_on_map_window) {
	MsgText *msg_text;

	if (s.empty())
		return;

	if (f == NULL)
		f = font;

	msg_text = new MsgText(s, f);
	msg_text->color = color;
	//::debug(1, "%s", msg_text->s.c_str());

	holding_buffer.push_back(msg_text);

	process_holding_buffer();

}

// process text tokens till we either run out or hit a page break.

void MsgScroll::process_holding_buffer() {
	MsgText *token;

	if (!page_break) {
		token = holding_buffer_get_token();

		for (; token != NULL && !page_break;) {
			parse_token(token);
			delete token;
			scroll_updated = true;

			if (!page_break)
				token = holding_buffer_get_token();
		}
	}
}

MsgText *MsgScroll::holding_buffer_get_token() {
	MsgText *input, *token;
	int i;

	if (holding_buffer.empty())
		return NULL;

	input = holding_buffer.front();

	if (input->font == NULL) {
		line_count = 0;
		holding_buffer.pop_front();
		delete input;
		return NULL;
	}

	i = input->s.findFirstOf(" \t\n*<>`", 0);
	if (i == 0) i++;

	if (i == -1)
		i = input->s.length();

	if (i > 0) {
		token = new MsgText(input->s.substr(0, i), font); // FIX maybe a format flag. // input->font);
		token->color = input->color;
		input->s.erase(0, i);
		if (input->s.length() == 0) {
			holding_buffer.pop_front();
			delete input;
		}
		return token;
	}

	return NULL;
}

bool MsgScroll::can_fit_token_on_msgline(MsgLine *msg_line, MsgText *token) {
	if (msg_line->total_length + token->length() > scroll_width) {
		return false; //token doesn't fit on the current line.
	}

	return true;
}

bool MsgScroll::parse_token(MsgText *token) {
	MsgLine *msg_line = NULL;

	if (!msg_buf.empty())
		msg_line = msg_buf.back(); // retrieve the last line from the scroll buffer.

	switch (token->s[0]) {
	case '\n' :
		add_new_line();
		break;

	case '\t' :
		set_autobreak(false);
		break;

	case '`'  :
		capitalise_next_letter = true;
		break;

	case '<'  :
		set_font(NUVIE_FONT_GARG); // runic / gargoyle font
		break;

	case '>'  :
		if (is_garg_font()) {
			set_font(NUVIE_FONT_NORMAL); // english font
			break;
		}
	// Note fall through. ;-) We fall through if we haven't already seen a '<' char

	default   :
		if (msg_line) {
			if (can_fit_token_on_msgline(msg_line, token) == false) { // the token is to big for the current line
				msg_line = add_new_line();
			}
			// This adds extra newlines. (SB-X)
			//                 if(msg_line->total_length + token->length() == scroll_width) //we add a new line but write to the old line.
			//                    add_new_line();

			if (msg_line->total_length == 0 && token->s[0] == ' ' && discard_whitespace) // discard whitespace at the start of a line.
				return true;
		}
		if (token->s[0] == '*') {
			if (just_finished_page_break == false) //we don't want to add another break if we've only just completed an autobreak
				set_page_break();
		} else {
			if (capitalise_next_letter) {
				token->s[0] = toupper(token->s[0]);
				capitalise_next_letter = false;
			}

			if (msg_line == NULL) {
				msg_line = add_new_line();
			}

			add_token(token);
			if (msg_line->total_length == scroll_width // add another line for cursor.
			        && (talking || Game::get_game()->get_event()->get_mode() == INPUT_MODE
			            || Game::get_game()->get_event()->get_mode() == KEYINPUT_MODE)) {
				msg_line = add_new_line();
			}
		}
		break;
	}

	if (msg_buf.size() > scroll_height)
		display_pos = msg_buf.size() - scroll_height;
	just_finished_page_break = false;
	just_displayed_prompt = false;
	return true;
}

void MsgScroll::add_token(MsgText *token) {
	msg_buf.back()->append(token);
}

bool MsgScroll::remove_char() {
	MsgLine *msg_line;

	msg_line = msg_buf.back(); // retrieve the last line from the scroll buffer.
	msg_line->remove_char();

	if (msg_line->total_length == 0) { // remove empty line from scroll buffer
		msg_buf.pop_back();
		delete msg_line;
	}

	return true;
}

void MsgScroll::set_font(uint8 font_type) {
	font = Game::get_game()->get_font_manager()->get_font(font_type); // 0 = normal english; 1 = runic / gargoyle font
}

bool MsgScroll::is_garg_font() {
	return (font == Game::get_game()->get_font_manager()->get_font(NUVIE_FONT_GARG));
}

void MsgScroll::clear_scroll() {
	Std::list<MsgLine *>::iterator iter;

	for (iter = msg_buf.begin(); iter != msg_buf.end(); iter++) {
		MsgLine *line = *iter;
		delete line;
	}

	msg_buf.clear();
	line_count = 0;
	display_pos = 0;
	scroll_updated = true;
	add_new_line();
}

void MsgScroll::delete_front_line() {
	MsgLine *msg_line_front = msg_buf.front();
	msg_buf.pop_front();
	delete msg_line_front;
}

MsgLine *MsgScroll::add_new_line() {
	MsgLine *msg_line = new MsgLine();
	msg_buf.push_back(msg_line);
	line_count++;

	if (msg_buf.size() > scrollback_height) {
		delete_front_line();
	}

	if (autobreak && line_count > scroll_height - 1)
		set_page_break();

	return msg_line;
}

bool MsgScroll::set_prompt(const char *new_prompt, Font *f) {

	prompt.s.assign(new_prompt);
	prompt.font = f;

	return true;
}

void MsgScroll::display_prompt() {
	if (!talking && !just_displayed_prompt) {
//line_count = 0;
		display_string(prompt.s, prompt.font, MSGSCROLL_NO_MAP_DISPLAY);
//line_count = 0;

		clear_page_break();
		just_displayed_prompt = true;
	}
}

void MsgScroll::display_converse_prompt() {
	display_string("\nyou say:", MSGSCROLL_NO_MAP_DISPLAY);
}

void MsgScroll::set_keyword_highlight(bool state) {
	keyword_highlight = state;
}

void MsgScroll::set_permitted_input(const char *allowed) {
	permit_input = allowed;
	if (permit_input) {
		if (strcmp(permit_input, "yn") == 0)
			yes_no_only = true;
		else if (strncmp(permit_input, "0123456789", strlen(permit_input)) == 0)
			numbers_only = true;
		else if (game_type == NUVIE_GAME_U6 && strcmp(permit_input, "ayn") == 0) // Heftimus npc 47
			aye_nay_only = true;
	}
}

void MsgScroll::clear_permitted_input() {
	permit_input = NULL;
	yes_no_only = false;
	numbers_only = false;
	aye_nay_only = false;
}

void MsgScroll::set_input_mode(bool state, const char *allowed, bool can_escape, bool use_target_cursor, bool set_numbers_only_to_true) {
	bool do_callback = false;

	input_mode = state;
	clear_permitted_input();
	permit_inputescape = can_escape;
	using_target_cursor = use_target_cursor;
	if (set_numbers_only_to_true)
		numbers_only = true;

	line_count = 0;

	clear_page_break();

	if (input_mode == true) {
		if (allowed && strlen(allowed))
			set_permitted_input(allowed);
		//FIXME SDL2 SDL_EnableUNICODE(1); // allow character translation
		input_buf.erase(0, input_buf.length());
	} else {
		//FIXME SDL2 SDL_EnableUNICODE(0); // reduce translation overhead when not needed
		if (callback_target)
			do_callback = true; // **DELAY until end-of-method so callback can set_input_mode() again**
	}
	Game::get_game()->get_gui()->lock_input((input_mode && !using_target_cursor) ? this : NULL);

// send whatever input was collected to target that requested it
	if (do_callback) {
		CallBack *requestor = callback_target; // copy to temp
		char *user_data = callback_user_data;
		cancel_input_request(); // clear originals (callback may request again)

		Std::string input_str = input_buf;
		requestor->set_user_data(user_data); // use temp requestor/user_data
		requestor->callback(MSGSCROLL_CB_TEXT_READY, this, &input_str);
	}
}

void MsgScroll::move_scroll_down() {
	if (msg_buf.size() > scroll_height && display_pos < msg_buf.size() - scroll_height) {
		display_pos++;
		scroll_updated = true;
	}
}

void MsgScroll::move_scroll_up() {
	if (display_pos > 0) {
		display_pos--;
		scroll_updated = true;
	}
}

void MsgScroll::page_up() {
	uint8 i = 0;
	for (; display_pos > 0 && i < scroll_height; i++)
		display_pos--;
	if (i > 0)
		scroll_updated = true;
}

void MsgScroll::page_down() {
	uint8 i = 0;
	for (; msg_buf.size() > scroll_height && i < scroll_height
	        && display_pos < msg_buf.size() - scroll_height ; i++)
		display_pos++;
	if (i > 0)
		scroll_updated = true;
}

void MsgScroll::process_page_break() {
	page_break = false;
	just_finished_page_break = true;
	if (!input_mode)
		Game::get_game()->get_gui()->unlock_input();
	process_holding_buffer(); // Process any text in the holding buffer.
	just_displayed_prompt = true;
}

/* Take input from the main event handler and do something with it
 * if necessary.
 */
GUI_status MsgScroll::KeyDown(const Common::KeyState &keyState) {
	Common::KeyState key = keyState;
	char ascii = get_ascii_char_from_keysym(key);

	if (page_break == false && input_mode == false)
		return (GUI_PASS);

	bool is_printable = Common::isPrint(ascii);
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);
	ActionKeyType action_key_type = keybinder->GetActionKeyType(a);

	if (using_target_cursor && !is_printable && action_key_type <= DO_ACTION_KEY) // directional keys, toggle_cursor, and do_action
		return GUI_PASS;

	if (!input_mode || !is_printable) {
		switch (action_key_type) {
		case WEST_KEY:
			key.keycode = Common::KEYCODE_LEFT;
			break;
		case EAST_KEY:
			key.keycode = Common::KEYCODE_RIGHT;
			break;
		case SOUTH_KEY:
			key.keycode = Common::KEYCODE_DOWN;
			break;
		case NORTH_KEY:
			key.keycode = Common::KEYCODE_UP;
			break;
		case CANCEL_ACTION_KEY:
			key.keycode = Common::KEYCODE_ESCAPE;
			break;
		case DO_ACTION_KEY:
			key.keycode = Common::KEYCODE_RETURN;
			break;
		case MSGSCROLL_UP_KEY:
			key.keycode = Common::KEYCODE_PAGEUP;
			break;
		case MSGSCROLL_DOWN_KEY:
			key.keycode = Common::KEYCODE_PAGEDOWN;
			break;
		default:
			if (keybinder->handle_always_available_keys(a)) return GUI_YUM;
			break;
		}
	}
	switch (key.keycode) {
	case Common::KEYCODE_UP :
		if (input_mode) break; //will select printable ascii
		move_scroll_up();
		return (GUI_YUM);

	case Common::KEYCODE_DOWN:
		if (input_mode) break; //will select printable ascii
		move_scroll_down();
		return (GUI_YUM);
	case Common::KEYCODE_PAGEUP:
		if (Game::get_game()->is_new_style())
			move_scroll_up();
		else page_up();
		return (GUI_YUM);
	case Common::KEYCODE_PAGEDOWN:
		if (Game::get_game()->is_new_style())
			move_scroll_down();
		else page_down();
		return (GUI_YUM);
	default :
		break;
	}

	if (page_break) {
		process_page_break();
		return (GUI_YUM);
	}

	switch (key.keycode) {
	case Common::KEYCODE_ESCAPE:
		if (permit_inputescape) {
			// reset input buffer
			permit_input = NULL;
			if (input_mode)
				set_input_mode(false);
		}
		return (GUI_YUM);
	case Common::KEYCODE_KP_ENTER:
	case Common::KEYCODE_RETURN:
		if (permit_inputescape || input_char != 0) { // input_char should only be permit_input
			if (input_char != 0)
				input_buf_add_char(get_char_from_input_char());
			if (input_mode)
				set_input_mode(false);
		}
		return (GUI_YUM);
	case Common::KEYCODE_RIGHT:
		if (input_char != 0 && permit_input == NULL)
			input_buf_add_char(get_char_from_input_char());
		break;
	case Common::KEYCODE_DOWN:
		increase_input_char();
		break;
	case Common::KEYCODE_UP:
		decrease_input_char();
		break;
	case Common::KEYCODE_LEFT :
	case Common::KEYCODE_BACKSPACE :
		if (input_mode) {
			if (input_char != 0)
				input_char = 0;
			else
				input_buf_remove_char();
		}
		break;
	case Common::KEYCODE_LSHIFT :
		return (GUI_YUM);
	case Common::KEYCODE_RSHIFT :
		return (GUI_YUM);
	default: // alphanumeric characters
		if (input_mode && is_printable) {
			if (permit_input == NULL) {
				if (!numbers_only || Common::isDigit(ascii)) {
					if (input_char != 0)
						input_buf_add_char(get_char_from_input_char());
					input_buf_add_char(ascii);
				}
			} else if (strchr(permit_input, ascii) || strchr(permit_input, tolower(ascii))) {
				input_buf_add_char(toupper(ascii));
				set_input_mode(false);
			}
		}
		break;
	}

	return (GUI_YUM);
}


GUI_status MsgScroll::MouseWheel(sint32 x, sint32 y) {
	if (page_break) { // any click == scroll-to-end
		process_page_break();
		return (GUI_YUM);
	}

	Game *game = Game::get_game();

	if (game->is_new_style()) {
		if (!input_mode)
			return GUI_PASS;
		if (y > 0)
			move_scroll_up();
		if (y < 0)
			move_scroll_down();
	} else {
		if (y > 0)
			page_up();
		if (y < 0)
			page_down();
	}
	return GUI_YUM;
}

GUI_status MsgScroll::MouseUp(int x, int y, Shared::MouseButton button) {
	uint16 i;
	Std::string token_str;

	if (page_break) { // any click == scroll-to-end
		process_page_break();
		return (GUI_YUM);
	}

	if (button == 1) { // left click == select word
		if (input_mode) {
			token_str = get_token_string_at_pos(x, y);
			if (permit_input != NULL) {
				if (strchr(permit_input, token_str[0])
				        || strchr(permit_input, tolower(token_str[0]))) {
					input_buf_add_char(token_str[0]);
					set_input_mode(false);
				}
				return (GUI_YUM);
			}

			for (i = 0; i < token_str.length(); i++) {
				if (Common::isAlnum(token_str[i]))
					input_buf_add_char(token_str[i]);
			}

		} else if (!Game::get_game()->is_new_style())
			Game::get_game()->get_event()->cancelAction();
	} else if (button == 3) { // right click == send input
		if (input_mode) {
			if (permit_inputescape) {
				set_input_mode(false);
				return (GUI_YUM);
			}
		} else if (!Game::get_game()->is_new_style())
			Game::get_game()->get_event()->cancelAction();
	}
	return (GUI_PASS);
}

Std::string MsgScroll::get_token_string_at_pos(uint16 x, uint16 y) {
	uint16 i;
	sint32 buf_x, buf_y;
	MsgText *token = NULL;
	Std::list<MsgLine *>::iterator iter;

	buf_x = (x - area.left) / 8;
	buf_y = (y - area.top) / 8;

	if (buf_x < 0 || buf_x >= scroll_width || // click not in MsgScroll area.
	        buf_y < 0 || buf_y >= scroll_height)
		return "";

	if (msg_buf.size() <= scroll_height) {
		if ((sint32)msg_buf.size() < buf_y + 1)
			return "";
	} else {
		buf_y = display_pos + buf_y;
	}

	for (i = 0, iter = msg_buf.begin(); i < buf_y && iter != msg_buf.end();) {
		iter++;
		i++;
	}

	if (iter != msg_buf.end()) {
		token = (*iter)->get_text_at_pos(buf_x);
		if (token) {
			DEBUG(0, LEVEL_DEBUGGING, "Token at (%d,%d) = %s\n", buf_x, buf_y, token->s.c_str());
			return token->s;
		}
	}

	return "";
}

void MsgScroll::Display(bool full_redraw) {
	uint16 i;
	Std::list<MsgLine *>::iterator iter;
	MsgLine *msg_line = NULL;



	if (scroll_updated || full_redraw || Game::get_game()->is_original_plus_full_map()) {
		screen->fill(bg_color, area.left, area.top, area.width(), area.height()); //clear whole scroll

		iter = msg_buf.begin();
		for (i = 0; i < display_pos; i++)
			iter++;

		for (i = 0; i < scroll_height && iter != msg_buf.end(); i++, iter++) {
			msg_line = *iter;
			drawLine(screen, msg_line, i);
		}
		scroll_updated = false;

		screen->update(area.left, area.top, area.width(), area.height());

		cursor_y = i - 1;
		if (msg_line) {
			cursor_x = msg_line->total_length;
			if (cursor_x == scroll_width) { // don't draw the cursor outside the scroll (SB-X)
				if (cursor_y + 1 < scroll_height)
					cursor_y++;
				cursor_x = 0;
			}
		} else
			cursor_x = area.left;
	} else {
		clearCursor(area.left + 8 * cursor_x, area.top + cursor_y * 8);
	}

	if (show_cursor && (msg_buf.size() <= scroll_height || display_pos == msg_buf.size() - scroll_height)) {
		drawCursor(area.left + left_margin + 8 * cursor_x, area.top + cursor_y * 8);
	}

}

inline void MsgScroll::drawLine(Screen *theScreen, MsgLine *msg_line, uint16 line_y) {
	MsgText *token;
	Std::list<MsgText *>::iterator iter;
	uint16 total_length = 0;

	for (iter = msg_line->text.begin(); iter != msg_line->text.end() ; iter++) {
		token = *iter;
		token->font->drawString(theScreen, token->s.c_str(), area.left + left_margin + total_length * 8, area.top + line_y * 8, token->color, font_highlight_color); //FIX for hardcoded font height
		total_length += token->s.length();
	}
}

void MsgScroll::clearCursor(uint16 x, uint16 y) {
	screen->fill(bg_color, x, y, 8, 8);
}

void MsgScroll::drawCursor(uint16 x, uint16 y) {
	uint8 cursor_color = input_mode ? get_input_font_color() : font_color;

	if (input_char != 0) { // show letter selected by arrow keys
		font->drawChar(screen, get_char_from_input_char(), x, y, cursor_color);
		screen->update(x, y, 8, 8);
		return;
	}
	if (page_break) {
		if (cursor_wait <= 2) // flash arrow
			font->drawChar(screen, 1, x, y, cursor_color); // down arrow
	} else
		font->drawChar(screen, cursor_char + 5, x, y, cursor_color); //spinning ankh

	screen->update(x, y, 8, 8);
	if (cursor_wait == MSGSCROLL_CURSOR_DELAY) {
		cursor_char = (cursor_char + 1) % 4;
		cursor_wait = 0;
	} else
		cursor_wait++;
}


void MsgScroll::set_page_break() {
	line_count = 1;
	page_break = true;

	if (!input_mode) {
		Game::get_game()->get_gui()->lock_input(this);
	}

	return;
}

bool MsgScroll::input_buf_add_char(char c) {
	MsgText token;
	input_char = 0;
	if (permit_input != NULL)
		input_buf_remove_char();
	input_buf.push_back(c);
	scroll_updated = true;

// Add char to scroll buffer

	token.s.assign(&c, 1);
	token.font = font;
	token.color = get_input_font_color();

	parse_token(&token);

	return true;
}

bool MsgScroll::input_buf_remove_char() {
	if (input_buf.length()) {
		input_buf.erase(input_buf.length() - 1, 1);
		scroll_updated = true;
		remove_char();

		return true;
	}

	return false;
}

bool MsgScroll::has_input() {
	if (input_mode == false) //we only have input ready after the user presses enter.
		return true;

	return false;
}

Std::string MsgScroll::get_input() {
// MsgScroll sets input_mode to false when it receives Common::KEYCODE_ENTER
	Std::string s;

	if (input_mode == false) {
		s.assign(input_buf);
	}
	//::debug(1, "%s", s.c_str());

	return s;
}

void MsgScroll::clear_page_break() {
	MsgText *msg_text = new MsgText("", NULL);
	holding_buffer.push_back(msg_text);

	process_holding_buffer();
}

/* Set callback & callback_user_data so that a message will be sent to the
 * caller when input has been gathered. */
void MsgScroll::request_input(CallBack *caller, void *user_data) {
	callback_target = caller;
	callback_user_data = (char *)user_data;
}

// 0 is no char, 1 - 26 is alpha, 27 is space, 28 - 37 is numbers
void MsgScroll::increase_input_char() {
	if (permit_input != NULL && strcmp(permit_input, "\n") == 0) // blame hacky PauseEffect
		return;
	if (yes_no_only)
		input_char = input_char == 25 ? 14 : 25;
	else if (aye_nay_only)
		input_char = input_char == 1 ? 14 : 1;
	else if (numbers_only)
		input_char = (input_char == 0 || input_char == 37) ? 28 : input_char + 1;
	else
		input_char = (input_char + 1) % 38;
	if (permit_input != NULL && !strchr(permit_input, get_char_from_input_char())) // might only be needed for the teleport cheat menu
		increase_input_char();
}

void MsgScroll::decrease_input_char() {
	if (permit_input != NULL && strcmp(permit_input, "\n") == 0) // blame hacky PauseEffect
		return;
	if (yes_no_only)
		input_char = input_char == 25 ? 14 : 25;
	else if (numbers_only)
		input_char = (input_char == 0 || input_char == 28) ? 37 : input_char - 1;
	else if (aye_nay_only)
		input_char = input_char == 1 ? 14 : 1;
	else
		input_char = input_char == 0 ? 37 : input_char - 1;
	if (permit_input != NULL && !strchr(permit_input, get_char_from_input_char())) // might only be needed for the teleport cheat menu
		decrease_input_char();
}

uint8 MsgScroll::get_char_from_input_char() {

	if (input_char > 27)
		return (input_char - 28 + Common::KEYCODE_0);
	else if (input_char == 27)
		return Common::KEYCODE_SPACE;
	else
		return (input_char + Common::KEYCODE_a - 1);
}

} // End of namespace Nuvie
} // End of namespace Ultima
