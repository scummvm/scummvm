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

#include "nuvieDefs.h"
#include "U6misc.h"
#include "GUI.h"
#include "GUI_button.h"
#include "Configuration.h"
#include "ViewManager.h"
#include "MsgScroll.h"
#include "DraggableView.h"

namespace Ultima {
namespace Ultima6 {

DraggableView::DraggableView(Configuration *config)
	: View(config) {
	drag = false;
	button_x = 0;
	button_y = 0;
	bg_image = NULL;
	bg_color_key = 0;
	Game *game = Game::get_game();
	if (game->is_orig_style() || game->is_original_plus_cutoff_map()) {
		need_full_redraw_when_moved = true;
		always_need_full_redraw_when_moved = true;
	} else if (game->get_game_width() < game->get_screen()->get_width()
	           || game->get_game_height() < game->get_screen()->get_height()) {
		need_full_redraw_when_moved = true;
		always_need_full_redraw_when_moved = false;
	} else // no need to set always_need_full_redraw_when_moved
		need_full_redraw_when_moved = false;
}

DraggableView::~DraggableView() {
	if (bg_image) {
		SDL_FreeSurface(bg_image);
		bg_image = NULL;
	}
}

void DraggableView::set_bg_color_key(Uint8 r, Uint8 g, Uint8 b) {
	if (bg_image) {
		bg_color_key = SDL_MapRGB(bg_image->format, 0, 0x70, 0xfc);
		SDL_SetColorKey(bg_image, SDL_TRUE, bg_color_key);
	}
}

GUI_status DraggableView::MouseDown(int x, int y, int button) {
	if (bg_image && HitRect(x, y)) {
		Uint32 pixel = sdl_getpixel(bg_image, x - area.x, y - area.y);
		if (pixel == bg_color_key) {
			return GUI_PASS;
		}
	}
	drag = true;
	button_x = x;
	button_y = y;

	moveToFront();
	if (Game::get_game()->is_new_style()) {
		Game::get_game()->get_scroll()->moveToFront();
	}
	grab_focus();

	return GUI_YUM;
}

GUI_status DraggableView::MouseUp(int x, int y, int button) {
	drag = false;

	release_focus();
	if (SDL_BUTTON(button) & SDL_BUTTON_RMASK) {
		Game::get_game()->get_view_manager()->close_gump(this);
	}
	return GUI_YUM;
}

GUI_status DraggableView::MouseMotion(int x, int y, Uint8 state) {
	int dx, dy;

	if (!drag || state == 0) //state is 0 if no button pressed
		return GUI_PASS;

	dx = x - button_x;
	dy = y - button_y;

	button_x = x;
	button_y = y;

	GUI::get_gui()->moveWidget(this, dx, dy);
// Redraw();

	return (GUI_YUM);
}

void DraggableView::force_full_redraw_if_needed() {
	if (need_full_redraw_when_moved) {
		if (always_need_full_redraw_when_moved // or over background
		        || (area.x + area.w > Game::get_game()->get_game_width() + Game::get_game()->get_game_x_offset()
		            || area.x < Game::get_game()->get_game_x_offset() || area.y < Game::get_game()->get_game_y_offset()
		            || area.y + area.h > Game::get_game()->get_game_height() + Game::get_game()->get_game_y_offset()))
			GUI::get_gui()->force_full_redraw();
	}
}

void DraggableView::MoveRelative(int dx, int dy) {
	int new_x = area.x + dx;

	if (new_x < 0) {
		dx = -area.x;
	} else if (new_x + area.w > screen->get_width()) {
		dx = screen->get_width() - (area.x + area.w);
	}

	int new_y = area.y + dy;

	if (new_y < 0) {
		dy = -area.y;
	} else if (new_y + area.h > screen->get_height()) {
		dy = screen->get_height() - (area.y + area.h);
	}

	force_full_redraw_if_needed(); // needs to happen before the move
	GUI_Widget::MoveRelative(dx, dy);

	return;
}

} // End of namespace Ultima6
} // End of namespace Ultima
