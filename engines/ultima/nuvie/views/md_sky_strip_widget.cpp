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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/files/u6_shape.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/views/md_sky_strip_widget.h"

namespace Ultima {
namespace Nuvie {

MDSkyStripWidget::MDSkyStripWidget(Configuration *cfg, GameClock *c, Player *p): GUI_Widget(NULL, 0, 0, 0, 0) {
	config = cfg;
	clock = c;
	player = p;
}

MDSkyStripWidget::~MDSkyStripWidget() {

}


void MDSkyStripWidget::init(sint16 x, sint16 y) {
	Std::string filename;

	GUI_Widget::Init(NULL, x, y, 112, 11);
	config_get_path(config, "mdscreen.lzc", filename);

	strip1.load_from_lzc(filename, 2, 2);
	strip2.load_from_lzc(filename, 2, 3);
}

void MDSkyStripWidget::Display(bool full_redraw) {
	if (full_redraw || update_display) {
		update_display = false;

		uint8 z = player->get_actor() ? player->get_actor()->get_z() : 0;
		if (z == 0) {
			display_surface();
		} else if (z == 1) {
			screen->fill(0, area.left, area.top, area.width(), area.height());
		} else {
			screen->fill(7, area.left, area.top, area.width(), area.height());
		}

		screen->update(area.left, area.top, area.width(), area.height());
	}

}

void MDSkyStripWidget::display_surface() {
	uint16 w, h;
	strip1.get_size(&w, &h);
	uint8 hour = clock->get_hour();
	uint8 minute = clock->get_minute();
	unsigned char *shp_data = hour < 12 ? strip1.get_data() : strip2.get_data();

	if (hour >= 12) {
		hour -= 12;
	}

	shp_data += hour * 16 + (minute / 15) * 4;

	screen->blit(area.left, area.top, shp_data, 8, area.width(), area.height(), w, false);
}

} // End of namespace Nuvie
} // End of namespace Ultima
