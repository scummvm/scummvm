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
#include "ultima/nuvie/misc/sdl_compat.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/portraits/portrait.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/gui/widgets/converse_gump_wou.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/keybinding/keys.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/gui/widgets/background.h"
#include "ultima/nuvie/files/u6_shape.h"
#include "ultima/nuvie/files/nuvie_bmp_file.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/views/md_sky_strip_widget.h"

namespace Ultima {
namespace Nuvie {

#define INPUT_FONT_COLOR 1

// ConverseGumpWOU Class

ConverseGumpWOU::ConverseGumpWOU(Configuration *cfg, Font *f, Screen *s) {
// uint16 x, y;

	init(cfg, f);
	Game *game = Game::get_game();
	game_type = game->get_game_type();

//scroll_width = 20;
//scroll_height = 18;

	set_scroll_dimensions(18, 18);

	Std::string height_str;
	uint16 x_off = game->get_game_x_offset();
	uint16 y_off = game->get_game_y_offset();

	if (game_type == NUVIE_GAME_U6) {
		GUI_Widget::Init(NULL, x_off + 8, y_off + 8, 160, 160);
		bg_color = converse_bg_color = 0x31; //17;
		if (game->get_game_width() >= 335) {
			Std::string imagefile;
			Std::string datadir = GUI::get_gui()->get_data_dir();
			build_path(datadir, "U6_WOU_Scroll_bg.bmp", imagefile);
			NuvieBmpFile bmp;
			bg_image = bmp.getSdlSurface32(imagefile);
		} else
			bg_image = NULL;
	} else { //MD and SE
		bg_image = NULL;
		GUI_Widget::Init(NULL, x_off + 8, y_off + 16, 160, 144);
		bg_color = converse_bg_color = Game::get_game()->get_palette()->get_bg_color();
	}

	found_break_char = false;
	left_margin = 8;
	add_new_line();
//DEBUG(0, LEVEL_DEBUGGING, "\nMin w = %d\n", frame_w + 12 + 210);
}

ConverseGumpWOU::~ConverseGumpWOU() {
	if (bg_image)
		SDL_FreeSurface(bg_image);
}

void ConverseGumpWOU::set_talking(bool state, Actor *actor) {
	if (state) {
		found_break_char = true;
		clear_scroll();
		Show();
		if (Game::get_game()->get_view_manager()->get_mdSkyWidget() && Game::get_game()->is_original_plus())
			Game::get_game()->get_view_manager()->get_mdSkyWidget()->Show();
	} else {
		if (talking) {
			MsgScroll::display_string("\nPress any key...*", MSGSCROLL_NO_MAP_DISPLAY);
		} else {
			if (Game::get_game()->get_view_manager()->get_mdSkyWidget() && Game::get_game()->is_original_plus())
				Game::get_game()->get_view_manager()->get_mdSkyWidget()->Hide();
		}
	}
	MsgScroll::set_talking(state);
}

void ConverseGumpWOU::process_page_break() {
	page_break = false;
	just_finished_page_break = true;
	if (!input_mode)
		Game::get_game()->get_gui()->unlock_input();
	if (!input_mode) {
		//clear_scroll();

		process_holding_buffer(); // Process any text in the holding buffer.
	}
}

void ConverseGumpWOU::display_converse_prompt() {
	MsgScroll::display_string("\nyou say:", INPUT_FONT_COLOR, MSGSCROLL_NO_MAP_DISPLAY);
}

void ConverseGumpWOU::display_bg() {
	Game *game = Game::get_game();
	U6Shape *background = game->get_background()->get_bg_shape();
	unsigned char *ptr = background->get_data();
	uint16 bg_w = game->get_background()->get_bg_w();
	uint16 x_off = game->get_game_x_offset();
	uint16 y_off = game->get_game_y_offset();
	uint16 game_w = game->get_game_width();

	if (game_type == NUVIE_GAME_U6) {
		if (game_w >= 335) { // get right size
			Common::Rect dst;
			dst.left = x_off;
			dst.top = y_off;
			dst.setWidth(176);
			dst.setHeight(176);
			SDL_BlitSurface(bg_image, NULL, game->get_screen()->get_sdl_surface(), &dst);
			screen->update(x_off, y_off, 176, 176);
		} else {
			screen->blit(x_off, y_off, ptr, 8, 171, 200, bg_w, true); // main bg
			if (game_w > 323) { // add filler to the right
				ptr += 5;
				screen->blit(x_off + 171, y_off, ptr, 8, game_w - 323, 200, bg_w, true);
				screen->fill(game->get_palette()->get_bg_color(), x_off + 171, y_off + 8, game_w - 323, 184); // ovewrite center
				screen->update(x_off, y_off, 171 + game_w - 323, 200);
			} else
				screen->update(x_off, y_off, 171, 200);
		}
	} else if (game_type == NUVIE_GAME_SE) {
		ptr += bg_w + 1;
		screen->fill(0, x_off + 172, y_off, 4, 1); // upper right corner
		screen->blit(x_off + 1, y_off + 1, ptr, 8, 176, 173, bg_w, true); // main bg
		// FIXME: will need to add time strip to top once it is coded
		screen->update(x_off + 1, y_off, 177, 174);
	} else { // MD
		screen->fill(0, x_off, y_off, 176, 164); // background has transparent parts that should be black - also adding border to bottom
		if (game_w == 321)
			screen->fill(4, x_off + 176, y_off, 1, 1); // overwrite black pixel on top that looks bad
		else if (game_w > 321) // add right border
			screen->fill(0, x_off + 176, y_off, 1, 164);
		screen->blit(x_off, y_off, ptr, 8, 176, 163, bg_w, true); // main bg
		if (game->get_view_manager()->get_mdSkyWidget())
			game->get_view_manager()->get_mdSkyWidget()->Display(true);
		screen->update(x_off, y_off, 177, 164);
	}
}

void ConverseGumpWOU::Display(bool full_redraw) {
	if (Game::get_game()->is_original_plus())
		display_bg(); // do first so that the text will be displayed on top
	MsgScroll::Display(true);
	if (game_type == NUVIE_GAME_U6) {
		Game::get_game()->get_map_window()->drawBorder();
		screen->update(area.left, area.top, area.width(), area.height());
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
