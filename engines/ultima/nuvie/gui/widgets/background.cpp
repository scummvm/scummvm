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
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/files/u6_bmp.h"
#include "ultima/nuvie/screen/dither.h"
#include "ultima/nuvie/gui/widgets/background.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/gui/gui.h"

namespace Ultima {
namespace Nuvie {

Background::Background(Configuration *cfg) : GUI_Widget(NULL) {
	config = cfg;
	config->value("config/GameType", game_type);

	bg_w = 0;
	bg_h = 0;
	border_width = 0;
	background = NULL;
	x_off = Game::get_game()->get_game_x_offset();
	y_off = Game::get_game()->get_game_y_offset();


	Init(NULL, 0, 0, Game::get_game()->get_screen()->get_width(), Game::get_game()->get_screen()->get_height());
}

Background::~Background() {
	if (background)
		delete background;
}

bool Background::init() {
	Std::string filename;

	if (!Game::get_game()->is_new_style()) {
		switch (game_type) {
		case NUVIE_GAME_U6 :
			config_get_path(config, "paper.bmp", filename);
			background = (U6Shape *) new U6Bmp();
			if (background->load(filename) == false)
				return false;
			if (Game::get_game()->is_original_plus()) {
				border_width = 158;
				right_bg_x_off = x_off + Game::get_game()->get_game_width() - 152;
				left_bg_x_off = x_off + Game::get_game()->get_game_width() - border_width;
			}
			break;

		case NUVIE_GAME_MD :
			background = new U6Shape();
			background->load_WoU_background(config, game_type);
			if (Game::get_game()->is_original_plus()) {
				border_width = 144;
				left_bg_x_off = x_off + Game::get_game()->get_game_width() - border_width;
			}
			break;

		case NUVIE_GAME_SE :
			background = new U6Shape();
			background->load_WoU_background(config, game_type);
			if (Game::get_game()->is_original_plus()) {
				border_width = 142;
				left_bg_x_off = x_off + Game::get_game()->get_game_width() - border_width;
			}
			break;
		}

		background->get_size(&bg_w, &bg_h);

		Game::get_game()->get_dither()->dither_bitmap(background->get_data(), bg_w, bg_h, DITHER_NO_TRANSPARENCY);
	}
	return true;
}

void Background::Display(bool full_redraw) {
	if (full_redraw || update_display || Game::get_game()->is_original_plus_full_map()) {
		if (Game::get_game()->is_original_plus()) {
			if (Game::get_game()->is_original_plus_cutoff_map())
				screen->clear(area.left, area.top, area.width(), area.height(), NULL);
			else if (full_redraw || update_display) { // need to clear null background when we have a game size smaller than the screen
				uint16 game_width = Game::get_game()->get_game_width();
				uint16 game_height = Game::get_game()->get_game_height();
				if (x_off > 0) { // centered
					screen->clear(area.left, area.top, x_off, area.height(), NULL); // left side
					screen->clear(x_off + game_width, area.top, x_off, area.height(), NULL); // right side
				} else if (area.width() > game_width) { // upper_left position
					screen->clear(game_width, area.top, area.width() - game_width, area.height(), NULL); // right side
				}
				if (y_off > 0) { // centered
					screen->clear(area.left, area.top, area.width(), y_off, NULL); // top
					screen->clear(area.left, y_off + game_height, area.width(), y_off, NULL); // bottom
				} else if (area.height() > game_height) { // upper_left position
					screen->clear(area.left, game_height, area.width(), area.height() - game_height, NULL); // bottom
				}
			}
			unsigned char *ptr = background->get_data();
			if (game_type == NUVIE_GAME_U6) {
				ptr += (bg_w - 152);
				screen->blit(right_bg_x_off, y_off, ptr, 8, 152, bg_h, bg_w, true);
				screen->blit(left_bg_x_off, y_off, background->get_data(), 8, 6, bg_h, bg_w, true);
			} else {
				if (game_type == NUVIE_GAME_MD)
					screen->fill(0, left_bg_x_off, y_off, border_width, bg_h); // background has transparent parts that should be black
				ptr += (bg_w - border_width);
				screen->blit(left_bg_x_off, y_off, ptr, 8, border_width, bg_h, bg_w, true);
			}
		} else {
			screen->clear(area.left, area.top, area.width(), area.height(), NULL);
			if (Game::get_game()->is_orig_style())
				screen->blit(x_off, y_off, background->get_data(), 8,  bg_w, bg_h, bg_w, true);
		}
		update_display = false;
		screen->update(0, 0, area.width(), area.height());
	}

	return;
}

bool Background::drag_accept_drop(int x, int y, int message, void *data) {
	GUI::get_gui()->force_full_redraw();
	DEBUG(0, LEVEL_DEBUGGING, "Background::drag_accept_drop()\n");
	if (Game::get_game()->is_original_plus_full_map() && message == GUI_DRAG_OBJ) { // added to gui before the map window so we need to redirect
		MapWindow *map_window = Game::get_game()->get_map_window();
		if (!map_window) // should be initialized before drops occur but we will play it safe
			return false;
		if (Game::get_game()->get_game_width() > x - x_off && x >= x_off // make sure we are on the map window
		        && Game::get_game()->get_game_height() > y - y_off && y >= y_off) {
			if (x >= left_bg_x_off && y <= 200 + y_off) // over background image
				return false;
			return map_window->drag_accept_drop(x, y, message, data);
		}
	}
	return false;
}

void Background::drag_perform_drop(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "Background::drag_perform_drop()\n");
	if (message == GUI_DRAG_OBJ) // should only happen with original_plus_full_map
		Game::get_game()->get_map_window()->drag_perform_drop(x, y, message, data);
}

} // End of namespace Nuvie
} // End of namespace Ultima
