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
#include "ultima/nuvie/files/nuvie_bmp_file.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/views/sun_moon_ribbon.h"
#include "ultima/nuvie/gui/gui.h"

namespace Ultima {
namespace Nuvie {

#define SUNMOON_RIBBON_END_WIDTH 5
#define SUNMOON_RIBBON_WIDTH 48
#define SUNMOON_RIBBON_HEIGHT 14
#define SUNMOON_RIBBON_DIR_WIDTH 14
#define SUNMOON_RIBBON_TOTAL_WIDTH (SUNMOON_RIBBON_WIDTH + SUNMOON_RIBBON_DIR_WIDTH)

SunMoonRibbon::SunMoonRibbon(Player *p, Weather *w, TileManager *tm): SunMoonStripWidget(p, tm) {
	bg_data = NULL;
	weather = w;
	retracted = true;
	current_time = 0;
}

SunMoonRibbon::~SunMoonRibbon() {

}


void SunMoonRibbon::init(Screen *) {
	GUI_Widget::Init(NULL, Game::get_game()->get_game_x_offset()
		+ Game::get_game()->get_game_width() - SUNMOON_RIBBON_TOTAL_WIDTH,
		Game::get_game()->get_game_y_offset(),
		SUNMOON_RIBBON_TOTAL_WIDTH,SUNMOON_RIBBON_HEIGHT);

	loadBgImage(0);
}

void SunMoonRibbon::loadBgImage(uint8 num) {
	char filename[6]; // "n.bmp\0"
	Std::string datadir = GUI::get_gui()->get_data_dir();
	Std::string imagefile;
	Std::string path;
	NuvieBmpFile bmp;

	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "gumps", path);
	datadir = path;
	build_path(datadir, "celestial", path);
	datadir = path;

	sprintf(filename, "%d.bmp", num);
	build_path(datadir, filename, imagefile);

	if (bg_data)
		SDL_FreeSurface(bg_data);
	bg_data = bmp.getSdlSurface32(imagefile);

	uint32 bg_color_key = SDL_MapRGB(bg_data->format, 0xb3, 0x94, 0x78);
	SDL_SetColorKey(bg_data, SDL_TRUE, bg_color_key);
}

void SunMoonRibbon::Display(bool full_redraw) {
	update_display = false;
	uint8 level = player->get_location_level();

	GameClock *clock = Game::get_game()->get_clock();
	uint16 time = 324; //not completely dark - used when in dungeons

	if (level == 0 || level == 5)
		time = clock->get_hour() * 60 + clock->get_minute();

	update_hour(time);

	if (!retracted && (level == 0 || level == 5))
		display_surface_strip();
	else
		display_dungeon_strip();


}

void SunMoonRibbon::update_hour(uint16 time) {
	uint8 dawn_tbl[] = {8, 7, 7, 6, 5, 4, 3, 2, 1, 0};
	uint8 dusk_tbl[] = {1, 2, 3, 4, 5, 6, 7, 7, 7, 8};

	time = time / 6;

	if (current_time != time || bg_data == NULL) {
		current_time = time;
		uint8 bg_num = 8; //night
		if (current_time >= 50 && current_time < 60) {
			bg_num = dawn_tbl[current_time - 50]; //dawn
		} else if (current_time >= 60 && current_time < 190) {
			bg_num = 0; //day time
		} else if (current_time >= 190 && current_time < 200) { //dusk
			bg_num = dusk_tbl[current_time - 190]; //dusk
		}
		loadBgImage(bg_num);
	}
}

void SunMoonRibbon::display_sun_moon(Tile *tile, uint8 pos) {
	struct {
		sint16 x, y;
	} skypos[15] = { // sky positions relative to area
		{ SUNMOON_RIBBON_WIDTH - 0 * 3, 7 },
		{ SUNMOON_RIBBON_WIDTH - 1 * 3, 6 },
		{ SUNMOON_RIBBON_WIDTH - 2 * 3, 5 },
		{ SUNMOON_RIBBON_WIDTH - 3 * 3, 4 },
		{ SUNMOON_RIBBON_WIDTH - 4 * 3, 3 },
		{ SUNMOON_RIBBON_WIDTH - 5 * 3, 2 },
		{ SUNMOON_RIBBON_WIDTH - 6 * 3, 1 },
		{ SUNMOON_RIBBON_WIDTH - 7 * 3, 0 },
		{ SUNMOON_RIBBON_WIDTH - 8 * 3, 1 },
		{ SUNMOON_RIBBON_WIDTH - 9 * 3, 2 },
		{ SUNMOON_RIBBON_WIDTH - 10 * 3, 3 },
		{ SUNMOON_RIBBON_WIDTH - 11 * 3, 4 },
		{ SUNMOON_RIBBON_WIDTH - 12 * 3, 5 },
		{ SUNMOON_RIBBON_WIDTH - 13 * 3, 6 },
		{ SUNMOON_RIBBON_WIDTH - 14 * 3, 7 }
	};

	uint16 x = area.left + skypos[pos].x - 10, y = area.top + skypos[pos].y;

	screen->blit(x, y, tile->data, 8 , 16, area.height() - skypos[pos].y > 16 ? 16 : area.height() - skypos[pos].y, 16, true);
}

void SunMoonRibbon::display_surface_strip() {
	Common::Rect src;
	Common::Rect dest;

	src = Common::Rect(SUNMOON_RIBBON_WIDTH, SUNMOON_RIBBON_HEIGHT);

	dest = area;
	dest.setWidth(SUNMOON_RIBBON_WIDTH);
	dest.setHeight(SUNMOON_RIBBON_HEIGHT);

	SDL_BlitSurface(bg_data, &src, surface, &dest);

	GameClock *clock = Game::get_game()->get_clock();
	bool eclipse = weather->is_eclipse();

	display_sun(clock->get_hour(), 0/*minutes*/, eclipse);

	if (!eclipse)
		display_moons(clock->get_day(), clock->get_hour());

	src.left = SUNMOON_RIBBON_WIDTH + weather->get_wind_dir() * SUNMOON_RIBBON_DIR_WIDTH;
	uint8 dir;
	if (weather->is_displaying_from_wind_dir()) { // points dir wind is coming from
		const uint8 from_wind_pos[] = { 1, 3, 5, 7, 2, 4, 6, 8, 0 };
		dir = from_wind_pos[weather->get_wind_dir()];
	} else { // points dir wind is blowing to
		const uint8 to_wind_pos[] = { 5, 7, 1, 3, 6, 8, 2, 4, 0 };
		dir = to_wind_pos[weather->get_wind_dir()];
	}
	src.left = SUNMOON_RIBBON_WIDTH + dir * SUNMOON_RIBBON_DIR_WIDTH;
	src.top = 0;
	src.setWidth(SUNMOON_RIBBON_DIR_WIDTH);
	src.setHeight(SUNMOON_RIBBON_HEIGHT);

	dest = area;
	dest.left = area.left + SUNMOON_RIBBON_WIDTH;
	dest.setWidth(SUNMOON_RIBBON_DIR_WIDTH);
	dest.setHeight(SUNMOON_RIBBON_HEIGHT);

	SDL_BlitSurface(bg_data, &src, surface, &dest);



	screen->update(area.left, area.top, area.width(), area.height());
}

void SunMoonRibbon::display_dungeon_strip() {
	Common::Rect src;
	Common::Rect dest;

	src = Common::Rect(SUNMOON_RIBBON_END_WIDTH, SUNMOON_RIBBON_HEIGHT);

	dest.left = area.left + SUNMOON_RIBBON_TOTAL_WIDTH - SUNMOON_RIBBON_END_WIDTH;
	dest.top = area.top;
	dest.setWidth(SUNMOON_RIBBON_END_WIDTH);
	dest.setHeight(SUNMOON_RIBBON_HEIGHT);

	SDL_BlitSurface(bg_data, &src, surface, &dest);
	screen->update(dest.left, dest.top, dest.width(), dest.height());
}

GUI_status SunMoonRibbon::MouseDown(int x, int y, Shared::MouseButton button) {
	Common::Rect rect = area;

	if (retracted) {
		rect.left = area.left + SUNMOON_RIBBON_TOTAL_WIDTH - SUNMOON_RIBBON_END_WIDTH;
		rect.setWidth(SUNMOON_RIBBON_END_WIDTH);
		if (HitRect(x, y, rect)) {
			retracted = false;
			return GUI_YUM;
		}
	} else {
		if (HitRect(x, y, rect)) {
			retracted = true;
			return GUI_YUM;
		}
	}

	return GUI_PASS;
}

} // End of namespace Nuvie
} // End of namespace Ultima
