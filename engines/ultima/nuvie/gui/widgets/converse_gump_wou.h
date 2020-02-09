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

#ifndef NUVIE_CORE_CONVERSE_GUMP_WOU_H
#define NUVIE_CORE_CONVERSE_GUMP_WOU_H

#include "ultima/nuvie/misc/call_back.h"
#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Nuvie {

using Std::list;

class Configuration;
class Font;
class MsgScroll;
class Actor;

class ConverseGumpWOU: public MsgScroll {

	uint8 converse_bg_color;

	uint8 frame_w;
	uint8 frame_h;
	uint16 min_w;

	nuvie_game_t game_type;
	bool found_break_char;
public:

	ConverseGumpWOU(Configuration *cfg, Font *f, Screen *s);
	~ConverseGumpWOU() override;

	void set_talking(bool state, Actor *actor = NULL) override;
	void set_font(uint8 font_type) override {}
	void display_converse_prompt() override;

	void Display(bool full_redraw) override;

	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}

	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}
	GUI_status MouseMotion(int x, int y, uint8 state) override {
		return GUI_YUM;
	}
	GUI_status MouseEnter(uint8 state) override {
		return GUI_YUM;
	}
	GUI_status MouseLeave(uint8 state) override {
		return GUI_YUM;
	}
	GUI_status MouseClick(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}
	GUI_status MouseDouble(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}
	GUI_status MouseDelayed(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}
	GUI_status MouseHeld(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}

	bool is_converse_finished() override {
		return (is_holding_buffer_empty() && !page_break);
	}

protected:

	void input_add_string(Std::string token_str);
	void process_page_break() override;
	uint8 get_input_font_color() override {
		return FONT_COLOR_WOU_CONVERSE_INPUT;
	}
	void display_bg();

private:

	Graphics::ManagedSurface *bg_image;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
