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

#include <cassert>
#include <string.h>
#include "SDL.h"

#include "Configuration.h"

#include "nuvieDefs.h"
#include "U6misc.h"
#include "Game.h"
#include "Screen.h"
#include "FontManager.h"
#include "Font.h"
#include "FpsCounter.h"

namespace Ultima {
namespace Ultima6 {

using std::string;


FpsCounter::FpsCounter(Game *g) : GUI_Widget(NULL) {
	game = g;
	font = game->get_font_manager()->get_conv_font();

	uint16 x_off = game->get_game_x_offset();
	uint16 y_off = game->get_game_y_offset();

	Init(NULL, x_off + 280, y_off, 40, 10);

	strcpy(fps_string, "000.00");
}

FpsCounter::~FpsCounter() {
}

void FpsCounter::setFps(float fps) {
	snprintf(fps_string, sizeof(fps_string), "%3.02f", fps);
}

void FpsCounter::Display(bool full_redraw) {
	Screen *screen = game->get_screen();

//    if(full_redraw || update_display || game->is_new_style())
	{
//        update_display = false;
		screen->fill(0, area.x, area.y, area.w, area.h);
		font->drawString(screen, fps_string, area.x, area.y);

		screen->update(area.x, area.y, area.w, area.h);
	}
}

} // End of namespace Ultima6
} // End of namespace Ultima
