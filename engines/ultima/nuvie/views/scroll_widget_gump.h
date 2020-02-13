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

#ifndef NUVIE_VIEWS_SCROLL_WIDGET_GUMP_H
#define NUVIE_VIEWS_SCROLL_WIDGET_GUMP_H

#include "ultima/nuvie/misc/call_back.h"
#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Nuvie {

using Std::list;


class Configuration;
class Actor;
class CallBack;

typedef enum {
	SCROLL_UP,
	SCROLL_DOWN,
	SCROLL_PAGE_UP,
	SCROLL_PAGE_DOWN,
	SCROLL_ESCAPE,
	SCROLL_TO_BEGINNING,
	SCROLL_TO_END
} ScrollEventType;

#define SCROLLWIDGETGUMP_W 200
#define SCROLLWIDGETGUMP_H 100

class ScrollWidgetGump: public MsgScroll {

	Font *font_normal;
	Font *font_garg;

	uint8 font_color;
	uint8 font_highlight;
	uint16 position;

	Std::string trailing_whitespace;

	bool show_up_arrow;
	bool show_down_arrow;

public:

	ScrollWidgetGump(Configuration *cfg, Screen *s);
	~ScrollWidgetGump() override;

	bool parse_token(MsgText *token) override;

	bool can_display_prompt() override {
		return false;
	}

	void Display(bool full_redraw) override;

	void display_prompt() override {}
	void display_string(Std::string s);
	void display_string(Std::string s, Font *f, bool include_on_map_window) override {
		return MsgScroll::display_string(s, f, include_on_map_window);
	}

	void set_font(uint8 font_type) override;
	bool is_garg_font() override;

	bool can_fit_token_on_msgline(MsgLine *msg_line, MsgText *token) override;

	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;    // otherwise we do Msgscroll::MouseUp
	}
	GUI_status MouseWheel(sint32 x, sint32 y) override;

	void move_scroll_down() override {
		scroll_movement_event(SCROLL_DOWN);
	}
	void move_scroll_up() override {
		scroll_movement_event(SCROLL_UP);
	}

protected:




private:
	const Common::Rect arrow_up_rect;
	const Common::Rect arrow_down_rect;

	GUI_status scroll_movement_event(ScrollEventType event);
	void update_arrows();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
