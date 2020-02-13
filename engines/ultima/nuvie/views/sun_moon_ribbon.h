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

#ifndef NUVIE_VIEWS_SUN_MOON_RIBBON_H
#define NUVIE_VIEWS_SUN_MOON_RIBBON_H

#include "ultima/nuvie/views/sun_moon_strip_widget.h"

namespace Ultima {
namespace Nuvie {

class SunMoonRibbon : public SunMoonStripWidget {

private:
	Graphics::ManagedSurface *bg_data;
	Weather *weather;
	bool retracted;
	uint16 current_time;

public:
	SunMoonRibbon(Player *p, Weather *w, TileManager *tm);
	~SunMoonRibbon() override;

	void init(Screen *screen);
	void Display(bool full_redraw) override;

	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;


	void extend() {
		retracted = false;
	}
	void retract() {
		retracted = true;
	}

protected:
	void display_sun_moon(Tile *tile, uint8 pos) override;

private:
	void loadBgImage(uint8 num);
	void display_surface_strip();
	void display_dungeon_strip();
	void update_hour(uint16 time);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
