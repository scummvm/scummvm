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
#include "ultima/nuvie/fonts/conv_font.h"
#include "ultima/nuvie/views/scroll_widget_gump.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/core/timed_event.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

// ScrollWidgetGump Class

ScrollWidgetGump::ScrollWidgetGump(Configuration *cfg, Screen *s) :
		arrow_up_rect(SCROLLWIDGETGUMP_W - 8 - 1, 4 + 1,
			SCROLLWIDGETGUMP_W - 8 - 1 + 7, 4 + 1 + 5),
		arrow_down_rect(SCROLLWIDGETGUMP_W - 8 - 1, SCROLLWIDGETGUMP_H - 8 + 3,
			SCROLLWIDGETGUMP_W - 8 - 1 + 7, SCROLLWIDGETGUMP_H - 8 + 3 + 5) {
	drop_target = false; //we don't participate in drag and drop.

	font_normal = Game::get_game()->get_font_manager()->get_conv_font();

	font_garg = Game::get_game()->get_font_manager()->get_conv_garg_font();

	init(cfg, font_normal);

	font_color = 0; // black
	font_highlight = FONT_COLOR_U6_HIGHLIGHT;

	scroll_width = 40;
	scroll_height = 10;

	show_up_arrow = false;
	show_down_arrow = false;

	GUI_Widget::Init(NULL, 0, 0, SCROLLWIDGETGUMP_W, SCROLLWIDGETGUMP_H);

	add_new_line(); //MsgScroll requires a line to start.

	position = 0;
// ignore_page_breaks = true;
}

ScrollWidgetGump::~ScrollWidgetGump() {
// ignore_page_breaks = false;
}

void ScrollWidgetGump::set_font(uint8 font_type) {
	if (font_type == NUVIE_FONT_NORMAL) {
		font = font_normal;
	} else {
		font = font_garg;
	}
}

bool ScrollWidgetGump::is_garg_font() {
	return (font == font_garg);
}

bool ScrollWidgetGump::can_fit_token_on_msgline(MsgLine *msg_line, MsgText *token) {
	if (msg_line->get_display_width() + token->getDisplayWidth() > SCROLLWIDGETGUMP_W - 8 - 8) {
		return false; //token doesn't fit on the current line.
	}

	return true;
}

bool ScrollWidgetGump::parse_token(MsgText *token) {
	if (token->s[0] == '*') // We don't want page breaks in the scroll widget.
		return true;

	return MsgScroll::parse_token(token);
}

void ScrollWidgetGump::display_string(Std::string s) {
	MsgScroll::display_string(s);
	update_arrows();
}

void ScrollWidgetGump::Display(bool full_redraw) {
	MsgText *token;

	uint16 y = area.top + 4;
	Std::list<MsgLine *>::iterator iter;

	if (show_up_arrow) {
		font_normal->drawChar(screen, FONT_UP_ARROW_CHAR, area.left + SCROLLWIDGETGUMP_W - 8, area.top + 4);
	}

	if (show_down_arrow) {
		font_normal->drawChar(screen, FONT_DOWN_ARROW_CHAR, area.left + SCROLLWIDGETGUMP_W - 8, area.top + SCROLLWIDGETGUMP_H - 8);
	}

	iter = msg_buf.begin();
	for (uint16 i = 0; i < position && iter != msg_buf.end(); i++)
		iter++;

	for (uint16 i = 0; i < scroll_height && iter != msg_buf.end(); i++, iter++) {
		MsgLine *msg_line = *iter;
		Std::list<MsgText *>::iterator iter1;

		iter1 = msg_line->text.begin();

		//if not last record or if last record is not an empty line.
		if (i + position < ((int)msg_buf.size() - 1) || (iter1 != msg_line->text.end() && ((*iter)->total_length != 0))) {
			//screen->fill(26, area.left, y + (i==0?-4:4), scroll_width * 7 + 8, (i==0?18:10));


			for (uint16 total_length = 0; iter1 != msg_line->text.end() ; iter1++) {
				token = *iter1;

				total_length += token->font->drawString(screen, token->s.c_str(), area.left + 4 + 4 + total_length, y + 4, font_color, font_highlight); //FIX for hardcoded font height
			}
			y += 10;
		}

	}

	screen->update(area.left, area.top, area.width(), area.height());
}

GUI_status ScrollWidgetGump::KeyDown(const Common::KeyState &key) {
	ScrollEventType event = SCROLL_ESCAPE;

	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case MSGSCROLL_DOWN_KEY:
		event = SCROLL_PAGE_DOWN;
		break;
	case SOUTH_KEY:
		event = SCROLL_DOWN;
		break;
	case MSGSCROLL_UP_KEY:
		event = SCROLL_PAGE_UP;
		break;
	case NORTH_KEY:
		event = SCROLL_UP;
		break;
	case HOME_KEY:
		event = SCROLL_TO_BEGINNING;
		break;
	case END_KEY:
		event = SCROLL_TO_END;
		break;
	default :
		break;
	}

	if (scroll_movement_event(event) == GUI_YUM)
		return GUI_YUM;

	return MsgScroll::KeyDown(key);
}

GUI_status ScrollWidgetGump::MouseWheel(sint32 x, sint32 y) {
	ScrollEventType event = SCROLL_ESCAPE;

	if (y > 0)
		event = SCROLL_UP;
	if (y < 0)
		event = SCROLL_DOWN;

	return scroll_movement_event(event);
}

GUI_status ScrollWidgetGump::MouseDown(int x, int y, Shared::MouseButton button) {
	ScrollEventType event = SCROLL_ESCAPE;

	switch (button) {
	case Shared::BUTTON_LEFT : {
		x -= area.left;
		y -= area.top;
		if (HitRect(x, y, arrow_up_rect))
			event = SCROLL_UP;
		else if (HitRect(x, y, arrow_down_rect))
			event = SCROLL_DOWN;
// FIXME - uncomment when we get a checkmark
//	                       else if(show_down_arrow || show_up_arrow) // don't close if scrollable
//	                           return GUI_YUM;
		break;
	}
	default :
		break;
	}

	return scroll_movement_event(event);
}

GUI_status ScrollWidgetGump::scroll_movement_event(ScrollEventType event) {
	switch (event) {
	case SCROLL_UP :
		if (position > 0) {
			//timer = new TimedCallback(this, NULL, 2000);
			position--;
			update_arrows();
			//grab_focus();
		}
		return GUI_YUM;

	case SCROLL_DOWN :
		//timer = new TimedCallback(this, NULL, 2000);
		if (page_break && position + scroll_height >= (int)msg_buf.size()) {
			if (position + scroll_height == (int)msg_buf.size()) // break was just off the page so advance text
				position++;
			process_page_break();
			update_arrows();
		} else if (position + scroll_height < (int)msg_buf.size()) {
			position++;
			update_arrows();
		}
		return (GUI_YUM);
	case SCROLL_PAGE_UP:
		if (position > 0) {
			position = position > scroll_height ? position - scroll_height : 0;
			update_arrows();
		}
		return GUI_YUM;
	case SCROLL_PAGE_DOWN:
		if (position + scroll_height < (int)msg_buf.size() || page_break) {
			if (position + scroll_height >= (int)msg_buf.size())
				position = (int)msg_buf.size();
			else {
				position += scroll_height;
				update_arrows();
				return GUI_YUM;
			}
			if (page_break) { // we didn't have enough text showing to fill out the current page.
				position = msg_buf.size();
				process_page_break();
			}
			update_arrows();
		}
		return GUI_YUM;
	case SCROLL_TO_BEGINNING :
		if (position > 0) {
			position = 0;
			update_arrows();
		}
		return GUI_YUM;
	case SCROLL_TO_END :
		if (position + scroll_height < (int)msg_buf.size() || page_break) {
			while (position + scroll_height < (int)msg_buf.size() || page_break) {
				if (page_break)
					process_page_break();
				else // added else just in case noting is added to the buffer
					position++;
			}
			update_arrows();
		}
		return GUI_YUM;
	default :
		//release_focus();
		//new TimedCallback(this, NULL, 50);
		break;
	}

	return GUI_PASS;
}

void ScrollWidgetGump::update_arrows() {
	if (position == 0) {
		show_up_arrow = false;
	} else {
		show_up_arrow = true;
	}

	if (position + scroll_height < (int)msg_buf.size() || page_break) {
		show_down_arrow = true;
	} else {
		show_down_arrow = false;
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
