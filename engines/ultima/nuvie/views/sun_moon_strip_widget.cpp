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
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/views/sun_moon_strip_widget.h"

namespace Ultima {
namespace Nuvie {

SunMoonStripWidget::SunMoonStripWidget(Player *p, TileManager *tm): GUI_Widget(NULL, 0, 0, 0, 0) {
	player = p;
	tile_manager = tm;
}

SunMoonStripWidget::~SunMoonStripWidget() {

}


void SunMoonStripWidget::init(sint16 x, sint16 y) {
	GUI_Widget::Init(NULL, x, y, 100, 20);
}

void SunMoonStripWidget::Display(bool full_redraw) {
//if(full_redraw || update_display)
// {
	update_display = false;
	uint8 level = player->get_location_level();

	if (level == 0 || level == 5)
		display_surface_strip();
	else
		display_dungeon_strip();

	screen->update(area.left, area.top, area.width(), area.height());
//  }

}


void SunMoonStripWidget::display_surface_strip() {
	uint8 i;
	Tile *tile;
	GameClock *clock = Game::get_game()->get_clock();
	Weather *weather = Game::get_game()->get_weather();
	bool eclipse = weather->is_eclipse();

	display_sun(clock->get_hour(), 0/*minutes*/, eclipse);

	if (!eclipse)
		display_moons(clock->get_day(), clock->get_hour());

	for (i = 0; i < 9; i++) {
		tile = tile_manager->get_tile(352 + i);
		screen->blit(area.left + 8 + i * 16, area.top, tile->data, 8, 16, 16, 16, true);
	}

	return;
}

void SunMoonStripWidget::display_dungeon_strip() {
	uint8 i;
	Tile *tile;

	tile = tile_manager->get_tile(372);
	screen->blit(area.left + 8, area.top, tile->data, 8, 16, 16, 16, true);

	tile = tile_manager->get_tile(373);

	for (i = 1; i < 8; i++) {
		screen->blit(area.left + 8 + i * 16, area.top, tile->data, 8, 16, 16, 16, true);
	}

	tile = tile_manager->get_tile(374);
	screen->blit(area.left + 8 + 7 * 16 + 8, area.top, tile->data, 8, 16, 16, 16, true);

	return;
}
// <SB-X>
void SunMoonStripWidget::display_sun_moon(Tile *tile, uint8 pos) {
	struct {
		sint16 x, y;
	} skypos[15] = { // sky positions relative to area
		{ 8 + 7 * 16 - 0 * 8, 6 }, // 7*16 is the first position on the right side
		{ 8 + 7 * 16 - 1 * 8, 3 },
		{ 8 + 7 * 16 - 2 * 8, 1 },
		{ 8 + 7 * 16 - 3 * 8, -1 },
		{ 8 + 7 * 16 - 4 * 8, -2 },
		{ 8 + 7 * 16 - 5 * 8, -3 },
		{ 8 + 7 * 16 - 6 * 8, -4 },
		{ 8 + 7 * 16 - 7 * 8, -4 },
		{ 8 + 7 * 16 - 8 * 8, -4 },
		{ 8 + 7 * 16 - 9 * 8, -3 },
		{ 8 + 7 * 16 - 10 * 8, -2 },
		{ 8 + 7 * 16 - 11 * 8, -1 },
		{ 8 + 7 * 16 - 12 * 8, 1 },
		{ 8 + 7 * 16 - 13 * 8, 3 },
		{ 8 + 7 * 16 - 14 * 8, 6 }
	};

	int height = 16;
	uint16 x = area.left + skypos[pos].x, y = area.top + skypos[pos].y;
	if (skypos[pos].y == 6) // goes through the bottom if not reduced
		height = 10;
	screen->blit(x, y, tile->data, 8 , 16, height, 16, true);
}

void SunMoonStripWidget::display_sun(uint8 hour, uint8 minute, bool eclipse) {
	uint16 sun_tile = 0;
	if (eclipse)
		sun_tile = 363; //eclipsed sun
	else if (hour == 5 || hour == 19)
		sun_tile = 361; //orange sun
	else if (hour > 5 && hour < 19)
		sun_tile = 362; //yellow sun
	else return; //no sun
	display_sun_moon(tile_manager->get_tile(sun_tile), hour - 5);
}

void SunMoonStripWidget::display_moons(uint8 day, uint8 hour, uint8 minute) {
	uint8 phase = 0;
	// trammel (starts 1 hour ahead of sun)
	phase = uint8(nearbyint((day - 1) / TRAMMEL_PHASE)) % 8;
	Tile *tileA = tile_manager->get_tile((phase == 0) ? 584 : 584 + (8 - phase)); // reverse order in tilelist
	uint8 posA = ((hour + 1) + 3 * phase) % 24; // advance 3 positions each phase-change

	// felucca (starts 1 hour behind sun)
	// ...my FELUCCA_PHASE may be wrong but this method works with it...
	sint8 phaseb = (day - 1) % uint8(nearbyint(FELUCCA_PHASE * 8)) - 1;
	phase = (phaseb >= 0) ? phaseb : 0;
	Tile *tileB = tile_manager->get_tile((phase == 0) ? 584 : 584 + (8 - phase)); // reverse order in tilelist
	uint8 posB = ((hour - 1) + 3 * phase) % 24; // advance 3 positions per phase-change

	if (posA >= 5 && posA <= 19)
		display_sun_moon(tileA, posA - 5);
	if (posB >= 5 && posB <= 19)
		display_sun_moon(tileB, posB - 5);
}

} // End of namespace Nuvie
} // End of namespace Ultima
