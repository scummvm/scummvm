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
#include "ultima/nuvie/portraits/portrait.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/gui/widgets/converse_gump.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

#define CURSOR_COLOR 248

// ConverseGump Class

ConverseGump::ConverseGump(Configuration *cfg, Font *f, Screen *s) {
// uint16 x, y;

	init(cfg, f);
	Game *game = Game::get_game();
	game_type = game->get_game_type();

	scroll_width = 30;
	scroll_height = 8;

// x = 8;
// y = 8;
	int gump_h;
	uint8 min_h, default_c;
	Std::string height_str;
	min_w = game->get_min_converse_gump_width();
	uint16 x_off = game->get_game_x_offset();
	uint16 y_off = game->get_game_y_offset();
	int game_h = game->get_game_height();

	if (game_type == NUVIE_GAME_SE) {
		default_c = 216;
		min_h = 185;
	} else if (game_type == NUVIE_GAME_MD) {
		default_c = 136;
		min_h = 181;
	} else {// U6
		default_c = 218;
		min_h = 152;
	}
	cfg->value(config_get_game_key(cfg) + "/converse_height", height_str, "default");

	if (game->is_orig_style()) {
		gump_h = game_h;
	} else {
		if (height_str == "default") {
			if (game_h > min_h * 1.5) // big enough that we probably don't want to take up the whole screen
				gump_h = min_h;
			else
				gump_h = game_h;
		} else {
			cfg->value(config_get_game_key(cfg) + "/converse_height", gump_h, game_h);
			if (gump_h < min_h)
				gump_h = min_h;
			else if (gump_h > game_h)
				gump_h = game_h;
		}
	}

	GUI_Widget::Init(NULL, x_off, y_off, game->get_converse_gump_width(), (uint16)gump_h);
	npc_portrait = NULL;
	avatar_portrait = NULL;
	keyword_list = NULL;

	font = game->get_font_manager()->get_conv_font();

	found_break_char = false;
	cursor_wait = 0;

	if (game->is_forcing_solid_converse_bg()) {
		solid_bg = true;
		force_solid_bg = true;
	} else {
		force_solid_bg = false;
		cfg->value(config_get_game_key(config) + "/converse_solid_bg", solid_bg, false);
	}

	int c;
	cfg->value(config_get_game_key(config) + "/converse_bg_color", c, default_c);
	if (c < 256)
		converse_bg_color = (uint8)c;

	cursor_position = 0;

	portrait_width = frame_w = game->get_portrait()->get_portrait_width();
	portrait_height = frame_h = game->get_portrait()->get_portrait_height();
	if (game_type == NUVIE_GAME_U6) {
		frame_w = portrait_width + 8;
		frame_h = portrait_height + 9;
	}
//DEBUG(0, LEVEL_DEBUGGING, "\nMin w = %d\n", frame_w + 12 + 210);
}

ConverseGump::~ConverseGump() {
	if (npc_portrait)
		free(npc_portrait);
	if (avatar_portrait)
		free(avatar_portrait);
	conv_keywords.clear();
	permitted_input_keywords.clear();
}

void ConverseGump::set_talking(bool state, Actor *actor) {
	if (state == true) {
		Game::get_game()->get_keybinder()->set_enable_joy_repeat(false);
		found_break_char = true;
		conv_keywords.clear();
		permitted_input_keywords.clear();
		Show();
		set_input_mode(false);
		clear_scroll();
		set_found_break_char(true);
		bool altar = (game_type == NUVIE_GAME_U6 // Singularity is excluded on purpose
		              && actor->get_actor_num() >= 192 && actor->get_actor_num() <= 199);
		if (!altar) {
			add_keyword("name");
			add_keyword("job");
			add_keyword("bye");
		}
		bool cant_join = (game_type == NUVIE_GAME_U6 // statues and altars
		                  && actor->get_actor_num() >= 189 && actor->get_actor_num() <= 200);
		if (actor->is_in_party())
			add_keyword("leave");
		else if (!cant_join)
			add_keyword("join");
		if (game_type == NUVIE_GAME_U6 && !altar) {
			add_keyword("rune");
			add_keyword("mantra");
		}
		keyword_list = &conv_keywords;

		if (avatar_portrait) {
			free(avatar_portrait);
			avatar_portrait = NULL;
		}

		cursor_position = 0;
	} else {
		Game::get_game()->get_keybinder()->set_enable_joy_repeat(true);
	}

	MsgScroll::set_talking(state);
}

void ConverseGump::set_actor_portrait(Actor *a) {
	if (npc_portrait)
		free(npc_portrait);

	if (Game::get_game()->get_portrait()->has_portrait(a))
		npc_portrait = get_portrait_data(a);
	else
		npc_portrait = NULL;

	if (avatar_portrait == NULL) {
		Actor *p = Game::get_game()->get_player()->get_actor();
		Actor *p1 = Game::get_game()->get_actor_manager()->get_actor(1);
		avatar_portrait = get_portrait_data(p->get_actor_num() != 0 ? p : p1); // don't use portrait 0 when in a vehicle
	}
}

unsigned char *ConverseGump::get_portrait_data(Actor *a) {
	if (game_type == NUVIE_GAME_U6) {
		return create_framed_portrait(a);
	}

	Portrait *p = Game::get_game()->get_portrait();
	return p->get_portrait_data(a);
}

unsigned char *ConverseGump::create_framed_portrait(Actor *a) { //FIXME U6 specific.
	//uint8 FRAME_W = portrait_width + 8;
	uint16 i;
	Portrait *p = Game::get_game()->get_portrait();
	unsigned char *portrait_data = p->get_portrait_data(a);
	unsigned char *framed_data = (unsigned char *)malloc(frame_w * frame_h);

	memset(framed_data, 255, frame_w * frame_h);

	memset(framed_data, 0, frame_w);
	memset(framed_data + (frame_h - 1)*frame_w, 0, frame_w);
	memset(framed_data + 1 * frame_w + 2, 53, 57);
	memset(framed_data + 2 * frame_w + 2, 57, 59);

	memset(framed_data + 3 * frame_w + 4, 0, 57);

	//top left corner
	framed_data[1 * frame_w] = 0;
	framed_data[1 * frame_w + 1] = 138;
	framed_data[2 * frame_w] = 0;
	framed_data[2 * frame_w + 1] = 139;
	framed_data[3 * frame_w] = 0;
	framed_data[3 * frame_w + 1] = 139;
	framed_data[3 * frame_w + 2] = 57;
	framed_data[3 * frame_w + 3] = 143;

	for (i = 0; i < portrait_height; i++) {
		framed_data[(i + 4)*frame_w] = 0;
		framed_data[(i + 4)*frame_w + 1] = 139;
		framed_data[(i + 4)*frame_w + 2] = 57;
		framed_data[(i + 4)*frame_w + 3] = 142;

		memcpy(&framed_data[(i + 4)*frame_w + 4], &portrait_data[i * p->get_portrait_width()], portrait_width);

		framed_data[(i + 4)*frame_w + 4 + portrait_width] = 0;
		framed_data[(i + 4)*frame_w + 4 + portrait_width + 1] = 57;
		framed_data[(i + 4)*frame_w + 4 + portrait_width + 2] = 53;
		framed_data[(i + 4)*frame_w + 4 + portrait_width + 3] = 0;
	}

	memset(framed_data + (frame_h - 5)*frame_w + 3, 142, 57);
	memset(framed_data + (frame_h - 4)*frame_w + 2, 57, 60);
	memset(framed_data + (frame_h - 3)*frame_w + 1, 139, 61);
	memset(framed_data + (frame_h - 2)*frame_w + 1, 142, 62);

	//bottom left
	framed_data[(frame_h - 5)*frame_w] = 0;
	framed_data[(frame_h - 5)*frame_w + 1] = 139;
	framed_data[(frame_h - 5)*frame_w + 2] = 57;
	framed_data[(frame_h - 4)*frame_w] = 0;
	framed_data[(frame_h - 4)*frame_w + 1] = 139;
	framed_data[(frame_h - 3)*frame_w] = 0;
	framed_data[(frame_h - 2)*frame_w] = 0;

	//top right
	framed_data[1 * frame_w + 59] = 50;
	framed_data[1 * frame_w + 59 + 1] = 49;
	framed_data[1 * frame_w + 59 + 2] = 49;
	framed_data[1 * frame_w + 59 + 3] = 15;
	framed_data[1 * frame_w + 59 + 4] = 0;
	framed_data[2 * frame_w + 59 + 2] = 15;
	framed_data[2 * frame_w + 59 + 3] = 49;
	framed_data[2 * frame_w + 59 + 4] = 0;
	framed_data[3 * frame_w + 59 + 2] = 57;
	framed_data[3 * frame_w + 59 + 3] = 49;
	framed_data[3 * frame_w + 59 + 4] = 0;
	framed_data[4 * frame_w + 59 + 3] = 50;

	//bottom right
	framed_data[(frame_h - 5)*frame_w + 60] = 143;
	framed_data[(frame_h - 5)*frame_w + 61] = 57;
	framed_data[(frame_h - 5)*frame_w + 62] = 53;
	framed_data[(frame_h - 5)*frame_w + 63] = 0;
	framed_data[(frame_h - 4)*frame_w + 62] = 53;
	framed_data[(frame_h - 4)*frame_w + 63] = 0;
	framed_data[(frame_h - 3)*frame_w + 62] = 173;
	framed_data[(frame_h - 3)*frame_w + 63] = 0;
	framed_data[(frame_h - 2)*frame_w + 63] = 0;

	free(portrait_data);

	return framed_data;
}

void ConverseGump::set_permitted_input(const char *allowed) {
	permitted_input_keywords.clear();
	keyword_list = &permitted_input_keywords;
	MsgScroll::set_permitted_input(allowed);

	if (yes_no_only) {
		add_keyword("yes");
		add_keyword("no");
	} else if (aye_nay_only) {
		add_keyword("aye");
		add_keyword("nay");
	} else if (numbers_only) {
		add_keyword("0");
		add_keyword("1");
		add_keyword("2");
		add_keyword("3");
		add_keyword("4");
		add_keyword("5");
		add_keyword("6");
		add_keyword("7");
		add_keyword("8");
		add_keyword("9");
	}

	cursor_position = 0;
}

void ConverseGump::clear_permitted_input() {
	keyword_list = &conv_keywords;
	MsgScroll::clear_permitted_input();
}

/*
void ConverseGump::add_token(MsgText *token)
{
    DEBUG(0,LEVEL_ALERT, "TOKEN: %s\n", token->s.c_str());

    display_text.push_back(*token);
}
*/

void ConverseGump::display_string(Std::string s, Font *f,  bool include_on_map_window) {
	if (s.empty())
		return;

	MsgScroll::display_string(strip_whitespace_after_break(s), f, include_on_map_window);//, MSGSCROLL_NO_MAP_DISPLAY);
}

Std::string ConverseGump::strip_whitespace_after_break(Std::string s) {
	Std::string::iterator iter;
	for (iter = s.begin(); iter != s.end();) {
		if (found_break_char == true) {
			char c = *iter;
			if (c == ' ' || c == '\t' || c == '\n' || c == '*') {
				iter = s.erase(iter);
			} else {
				found_break_char = false;
				iter++;
			}
		} else {
			char c = *iter;
			if (c == '*') {
				found_break_char = true;
			}
			iter++;
		}
	}

	return s;
}

bool ConverseGump::parse_token(MsgText *token) {
	int at_idx = token->s.findFirstOf('@', 0);
	int i = 0;
	int len = (int)token->s.length();
	while (at_idx != -1 && i < len) {
		Std::string keyword = "";
		for (i = at_idx + 1; i < len; i++) {
			char c = token->s[i];
			if (Common::isAlpha(c)) {
				keyword.push_back(c);
			}

			if (!Common::isAlpha(c) || i == len - 1) {
				token->s.erase(at_idx, 1);
				i--;
				at_idx = token->s.findFirstOf('@', i);
				break;
			}
		}
		DEBUG(0, LEVEL_WARNING, "%s", keyword.c_str());
		add_keyword(keyword);
	}

	parse_fm_towns_token(token);
	return MsgScroll::parse_token(token);
}

// Add FM-Towns keywords which take the form. +actor_numKeyword+ eg. +5runes+
// Only add keyword if the player has met the actor given by the actor_num
void ConverseGump::parse_fm_towns_token(MsgText *token) {
	int at_idx = token->s.findFirstOf('+', 0);
	int i = 0;
	int len = (int)token->s.length();
	bool has_met = false;
	while (at_idx != -1 && i < len) {
		i = at_idx + 1;
		char c = token->s[i];
		if (i < len && Common::isDigit(c)) {
			const char *c_str = token->s.c_str();
			uint16 actor_num = (int)strtol(&c_str[i], NULL, 10);
			if (actor_num < 256) {
				Actor *actor = Game::get_game()->get_actor_manager()->get_actor(actor_num);
				if (actor) {
					has_met = actor->is_met();
				}
			}
			for (; Common::isDigit(c_str[i]);)
				i++;
		}

		Std::string keyword = "";
		for (; i < len; i++) {
			char ch = token->s[i];

			if (Common::isAlpha(ch)) {
				keyword.push_back(ch);
			}

			if (!Common::isAlpha(ch) || i == len - 1) {
				token->s.erase(at_idx, (i - at_idx) + 1);
				i -= i - at_idx;
				at_idx = token->s.findFirstOf('+', i);
				break;
			}
		}
		DEBUG(0, LEVEL_WARNING, "%s", keyword.c_str());
		if (has_met) { //only add keyword if the player has met the actor in question.
			add_keyword(keyword);
			has_met = false;
		}
	}

	return;
}

void ConverseGump::add_keyword(Std::string keyword) {
	keyword = " *" + keyword;

	Std::list<MsgText>::iterator iter;
	for (iter = keyword_list->begin(); iter != keyword_list->end(); iter++) {
		if (string_i_compare((*iter).s, keyword)) {
			return;
		}
	}

	MsgText m_text;
	m_text.s = keyword;
	m_text.font = font;
	keyword_list->push_back(m_text);
}

Std::string ConverseGump::get_token_string_at_pos(uint16 x, uint16 y) {
	uint16 total_length = 0;
	uint16 tmp_y = area.top + portrait_height + 8 + 3 + 4;
	Std::list<MsgText>::iterator iter;
	for (iter = keyword_list->begin(); iter != keyword_list->end(); iter++) {
		MsgText t = *iter;
		uint16 token_len = font->getStringWidth(t.s.c_str());

//		if(token_len + total_length >= (26 * 8))
		if (portrait_width / 2 + portrait_width + token_len + total_length + 8 >= min_w - 4) {
			total_length = 0;
			tmp_y += 10;
		}
		//t.font->drawString(screen, t.s.c_str(), area.left + portrait_width / 2 + portrait_width + 8 + total_length * 8, y + portrait_height + 8, 0);
		if (x > area.left + portrait_width / 2 + portrait_width + 8 + total_length && x < area.left + portrait_width / 2 + portrait_width + 8 + total_length + token_len) {
			if (y > tmp_y && y < tmp_y + 8) {
				if (!is_permanent_keyword(t.s))
					keyword_list->erase(iter);
				return t.s;
			}
		}
		total_length += token_len;
	}
	return "";
}

Std::string ConverseGump::get_token_at_cursor() {
	uint16 i = 0;
	Std::list<MsgText>::iterator iter;
	for (iter = keyword_list->begin(); iter != keyword_list->end(); i++, iter++) {
		if (i == cursor_position) {
			Std::string keyword = (*iter).s;
			if (!is_permanent_keyword(keyword)) {
				keyword_list->erase(iter);
				if (permit_input)
					keyword = keyword.at(2); // only return first char after " *"
			}
			return keyword;
		}
	}

	return "";
}

bool ConverseGump::input_buf_add_char(char c) {
	input_char = 0;
	if (permit_input != NULL)
		input_buf_remove_char();
	input_buf.push_back(c);
	return true;
}

bool ConverseGump::input_buf_remove_char() {
	if (input_buf.length()) {
		input_buf.erase(input_buf.length() - 1, 1);
		return true;
	}

	return false;
}

void ConverseGump::Display(bool full_redraw) {
	MsgText *token;
	//Std::list<MsgText>::iterator iter;
	uint16 total_length = 0;
	uint16 y = area.top + portrait_height + 8 + 3;

	if (converse_bg_color != 255 || force_solid_bg) {
		if (solid_bg)
			screen->fill(converse_bg_color, area.left, area.top, area.width(), area.height());
		else
			screen->stipple_8bit(converse_bg_color, area.left, area.top, area.width(), area.height());
	}

	bool use_transparency = (game_type == NUVIE_GAME_U6) ? false : true;

	if (npc_portrait) {
		screen->blit(area.left + 4, area.top + 4, npc_portrait, 8, frame_w, frame_h, frame_w, use_transparency);
	}

	if (!page_break && input_mode && avatar_portrait && is_talking()) {
		screen->blit(area.left + portrait_width / 2 + 4, y, avatar_portrait, 8, frame_w, frame_h, frame_w, use_transparency);
		Std::list<MsgText>::iterator iter;
		sint16 i = 0;
		for (iter = keyword_list->begin(); iter != keyword_list->end(); i++, iter++) {
			MsgText t = *iter;
			uint16 token_len = font->getStringWidth(t.s.c_str());
//			 if(token_len + total_length >= (26 * 8))
			if (portrait_width / 2 + portrait_width + token_len + total_length + 8 >= min_w - 4) {
				total_length = 0;
				y += 10;
			}
			t.font->drawString(screen, t.s.c_str(), area.left + portrait_width / 2 + portrait_width + 8 + total_length, y + 4, 0, 0);
			if (cursor_position == i) {
				screen->fill(CURSOR_COLOR, area.left + portrait_width / 2 + portrait_width + 16 + total_length, y + 4 + 8, token_len - 8, 1);
			}
			total_length += token_len;
			//total_length += t.s.length();
		}
		y += 16;
		font->drawString(screen, " *", area.left + portrait_width / 2 + portrait_width + 8, y, 0, 0);
		font->drawString(screen, input_buf.c_str(), area.left + portrait_width / 2 + portrait_width + 8 + font->getStringWidth(" *"), y, 0, 0);
		drawCursor(area.left + portrait_width / 2 + portrait_width + 8 + font->getStringWidth(" *") + font->getStringWidth(input_buf.c_str()), y);
		if (cursor_position == keyword_list->size()) {
			screen->fill(CURSOR_COLOR, area.left + portrait_width / 2 + portrait_width + 16, y + 8, font->getStringWidth(input_buf.c_str()) + 8, 1);
		}
	}

	y = area.top + 4;
	total_length = 0;
	Std::list<MsgLine *>::iterator iter;
	for (iter = msg_buf.begin(); iter != msg_buf.end(); iter++) {
		MsgLine *msg_line = *iter;
		Std::list<MsgText *>::iterator iter1;

		for (iter1 = msg_line->text.begin(); iter1 != msg_line->text.end() ; iter1++) {
			token = *iter1;

			total_length += token->font->drawString(screen, token->s.c_str(), area.left + 4 + frame_w + 4 + total_length, y + 4, 0, 0); //FIX for hardcoded font height

			//token->s.length();
			//token->font->drawChar(screen, ' ', area.left + portrait_width + 8 + total_length * 8, y, 0);
			//total_length += 1;

		}
		y += 10;
		total_length = 0;
	}

	//font->drawString(screen, conv_str.c_str(), area.left, area.top);
	screen->update(area.left, area.top, area.width(), area.height());
}


GUI_status ConverseGump::KeyDown(const Common::KeyState &keyState) {
	Common::KeyState key = keyState;
	char ascii = get_ascii_char_from_keysym(key);

	if (page_break || !is_talking()) {
		page_break = false;
		just_finished_page_break = true;
		if (!input_mode)
			Game::get_game()->get_gui()->unlock_input();
		if (!is_holding_buffer_empty() || !input_mode || !is_talking()) {
			clear_scroll();
			process_holding_buffer(); // Process any text in the holding buffer.
		}
		return (GUI_YUM);
	}

	if (!input_mode || !Common::isPrint(ascii)) {
		KeyBinder *keybinder = Game::get_game()->get_keybinder();
		ActionType a = keybinder->get_ActionType(key);
		switch (keybinder->GetActionKeyType(a)) {
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
		default:
			if (keybinder->handle_always_available_keys(a)) return GUI_YUM;
			break;
		}
	}

	switch (key.keycode) {
	case Common::KEYCODE_LEFT:
		if (cursor_at_input_section() && input_char != 0)
			input_char = 0;
		else {
			if (!cursor_at_input_section() || !input_buf_remove_char()) {
				if (cursor_position == 0) {
					cursor_position = keyword_list->size();
				} else {
					cursor_position--;
				}
			}
		}
		break;
	case Common::KEYCODE_RIGHT:
		if (cursor_at_input_section() && input_char != 0 && permit_input == NULL)
			input_buf_add_char(get_char_from_input_char());
		else
			cursor_position = (cursor_position + 1) % (keyword_list->size() + 1);
		break;
	case Common::KEYCODE_DOWN:
		cursor_move_to_input();
		increase_input_char();
		break;
	case Common::KEYCODE_UP:
		cursor_move_to_input();
		decrease_input_char();
		break;
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
		if (permit_inputescape || !cursor_at_input_section()
		        || input_char != 0) { // input_char should only be permit_input
			if (!cursor_at_input_section())
				input_add_string(get_token_at_cursor());
			else {
				if (input_char != 0)
					input_buf_add_char(get_char_from_input_char());
			}
			//if(input_mode)
			set_input_mode(false);
			clear_scroll();
			found_break_char = true; //strip leading whitespace.
			cursor_reset();
		}

		return (GUI_YUM);
	case Common::KEYCODE_BACKSPACE :
		if (input_mode)
			input_buf_remove_char();
		break;
	default: // alphanumeric characters
		if (input_mode && Common::isPrint(ascii)) {
			cursor_move_to_input();
			if (permit_input == NULL) {
				if (!numbers_only || Common::isDigit(ascii))
					if (input_char != 0)
						input_buf_add_char(get_char_from_input_char());
				input_buf_add_char(ascii);
			} else if (strchr(permit_input, ascii) || strchr(permit_input, tolower(ascii))) {
				input_buf_add_char(toupper(ascii));
				set_input_mode(false);
				clear_scroll();
				found_break_char = true;
			}
		}
		break;
	}
	return GUI_YUM;
}

GUI_status ConverseGump::MouseUp(int x, int y, Shared::MouseButton button) {
	Std::string token_str;

	if (page_break || !is_talking()) { // any click == scroll-to-end
		page_break = false;
		just_finished_page_break = true;
		if (!input_mode)
			Game::get_game()->get_gui()->unlock_input();

		if (!is_holding_buffer_empty() || !input_mode || !is_talking()) {
			clear_scroll();
			process_holding_buffer(); // Process any text in the holding buffer.
		}
		return (GUI_YUM);
	} else if (button == 1) { // left click == select word
		if (input_mode) {
			token_str = get_token_string_at_pos(x, y);
			if (token_str.length() > 0) {
				input_add_string(token_str);
				set_input_mode(false);
				clear_scroll();
				found_break_char = true; //strip leading whitespace.
			}
		}
	}
	/*
	else if(button == 3) // right click == send input
	    if(permit_inputescape && input_mode)
	    {
	        set_input_mode(false);
	        return(GUI_YUM);
	    }
	 */
	return (GUI_YUM);
}

void ConverseGump::input_add_string(Std::string token_str) {
	input_buf.clear();
	for (uint16 i = 0; i < token_str.length(); i++) {
		if (Common::isAlnum(token_str[i]) && (!permit_input || strchr(permit_input, token_str[i])
		                              || strchr(permit_input, tolower(token_str[i]))))
			input_buf_add_char(token_str[i]);
	}
}

bool ConverseGump::is_permanent_keyword(Std::string keyword) {
	return (string_i_compare(keyword, " *buy") || string_i_compare(keyword, " *sell")
	        || string_i_compare(keyword, " *bye") || string_i_compare(keyword, " *spells")
	        || string_i_compare(keyword, " *reagents"));
}

void ConverseGump::drawCursor(uint16 x, uint16 y) {
	if (input_char != 0) {
		font->drawChar(screen, get_char_from_input_char(), x, y);
	} else {
		MsgScroll::drawCursor(x, y);
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
