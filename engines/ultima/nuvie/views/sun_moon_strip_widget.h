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

#ifndef NUVIE_VIEWS_SUN_MOON_STRIP_WIDGET_H
#define NUVIE_VIEWS_SUN_MOON_STRIP_WIDGET_H

#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/core/tile_manager.h"

namespace Ultima {
namespace Nuvie {

class Player;

class SunMoonStripWidget : public GUI_Widget {

protected:
	TileManager *tile_manager;
	Player *player;

public:
	SunMoonStripWidget(Player *p, TileManager *tm);
	~SunMoonStripWidget() override;

	void init(sint16 x, sint16 y);
	void Display(bool full_redraw) override;

protected:
	virtual void display_sun_moon(Tile *tile, uint8 pos);
	void display_sun(uint8 hour, uint8 minute, bool eclipse);
	void display_moons(uint8 day, uint8 hour, uint8 minute = 0);

private:
	void display_surface_strip();
	void display_dungeon_strip();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
