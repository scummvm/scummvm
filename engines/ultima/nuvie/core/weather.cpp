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
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/call_back.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/save/obj_list.h"
#include "ultima/nuvie/core/timed_event.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/script/script.h"

namespace Ultima {
namespace Nuvie {

//the longest we will go before having a change in wind direction
#define WEATHER_MAX_WIND 30

Weather::Weather(Configuration *cfg, GameClock *c, nuvie_game_t type) {
	config = cfg;
	clock = c;
	gametype = type;

	wind_dir = NUVIE_DIR_NONE;
	wind_timer = NULL;
	string s;
	config->value(config_get_game_key(config) + "/displayed_wind_dir", s, "from");
	if (s == "to")
		display_from_wind_dir = false;
	else
		display_from_wind_dir = true;
}

Weather::~Weather() {
}

bool Weather::load(NuvieIO *objlist) {
	clear_wind();

	if (gametype == NUVIE_GAME_U6) {
		wind_dir = load_wind(objlist);
		set_wind_change_callback(); //set a timer to change the wind direction in the future.
		send_wind_change_notification_callback();
	}

	return true;
}

MapCoord Weather::get_moonstone(uint8 moonstone) {
	if (moonstone < 8) // FIXME: hardcoded constant
		return Game::get_game()->get_script()->call_moonstone_get_loc(moonstone + 1);

	DEBUG(0, LEVEL_ERROR, "get_moonstone(%d): Moonstone out of range\n", moonstone);
	return MapCoord(0, 0, 0);
}
bool Weather::set_moonstone(uint8 moonstone, MapCoord where) {
	if (moonstone < 8) { // FIXME: hardcoded constant
		Game::get_game()->get_script()->call_moonstone_set_loc(moonstone + 1, where); //phase starts at 1 in script.
		return true;
	}
	DEBUG(0, LEVEL_ERROR, "set_moonstone(%d): Moonstone out of range\n", moonstone);
	return false;
}

void Weather::update_moongates() {
	Game::get_game()->get_script()->call_update_moongates(is_moon_visible());
}

uint8 Weather::load_wind(NuvieIO *objlist) {
	const uint8 wind_tbl[8] = {
		NUVIE_DIR_N,
		NUVIE_DIR_NE,
		NUVIE_DIR_E,
		NUVIE_DIR_SE,
		NUVIE_DIR_S,
		NUVIE_DIR_SW,
		NUVIE_DIR_W,
		NUVIE_DIR_NW
	};

	uint8 objlist_wind;

	objlist->seek(OBJLIST_OFFSET_U6_WIND_DIR);
	objlist_wind = objlist->read1();

	if (objlist_wind > 7) //objlist 0xff = Calm 'C'
		return NUVIE_DIR_NONE;

	return wind_tbl[objlist_wind];
}

void Weather::clear_wind() {
	if (wind_timer) {
		wind_timer->stop_timer();
		wind_timer = NULL;
	}


	wind_dir = NUVIE_DIR_NONE;

	return;
}

bool Weather::save(NuvieIO *objlist) {
	if (gametype == NUVIE_GAME_U6) {
		save_wind(objlist);
	}

	return true;
}

bool Weather::save_wind(NuvieIO *objlist) {
	const uint8 wind_tbl[] = {
		OBJLIST_U6_WIND_DIR_N,
		OBJLIST_U6_WIND_DIR_S,
		OBJLIST_U6_WIND_DIR_E,
		OBJLIST_U6_WIND_DIR_W,
		OBJLIST_U6_WIND_DIR_NE,
		OBJLIST_U6_WIND_DIR_SE,
		OBJLIST_U6_WIND_DIR_SW,
		OBJLIST_U6_WIND_DIR_NW,
		OBJLIST_U6_WIND_DIR_C
	};

	objlist->seek(OBJLIST_OFFSET_U6_WIND_DIR);
	objlist->write1(wind_tbl[wind_dir]);

	return true;
}

bool Weather::is_eclipse() {
	if (gametype != NUVIE_GAME_U6 || clock->get_timer(GAMECLOCK_TIMER_U6_ECLIPSE) == 0)
		return false;

	return true;
}

bool Weather::is_moon_visible() {
	//FIXME this is duplicated logic. Maybe we should look at how the original works out moon locations

	uint8 day = clock->get_day();
	uint8 hour = clock->get_hour();
	uint8 phase = 0;
	// trammel (starts 1 hour ahead of sun)
	phase = uint8(nearbyint((day - 1) / TRAMMEL_PHASE)) % 8;
	uint8 posA = ((hour + 1) + 3 * phase) % 24; // advance 3 positions each phase-change
	if (posA >= 5 && posA <= 19)
		return true;

	// felucca (starts 1 hour behind sun)
	// ...my FELUCCA_PHASE may be wrong but this method works with it...
	sint8 phaseb = (day - 1) % uint8(nearbyint(FELUCCA_PHASE * 8)) - 1;
	phase = (phaseb >= 0) ? phaseb : 0;
	uint8 posB = ((hour - 1) + 3 * phase) % 24; // advance 3 positions per phase-change

	if (posB >= 5 && posB <= 19)
		return true;

	return false;
}

string Weather::get_wind_dir_str() {
	string s;
	if (display_from_wind_dir) {
		const char from_names[9][3] = {"N", "E", "S", "W", "NE", "SE", "SW", "NW", "C"};
		s = from_names[wind_dir];
	} else {
		const char to_names[9][3] = {"S", "W", "N", "E", "SW", "NW", "NE", "SE", "C"};
		s = to_names[wind_dir];
	}

	return s;
}

void Weather::change_wind_dir() {
	uint8 new_wind_dir;

	new_wind_dir = NUVIE_RAND() % 9;

	set_wind_dir(new_wind_dir);
	return;
}

bool Weather::set_wind_dir(uint8 new_wind_dir) {
	uint8 old_wind_dir = wind_dir;

	if (new_wind_dir >= 9)
		return false;

	clear_wind();
	if (Game::get_game()->get_map_window()->in_dungeon_level())
		wind_dir = NUVIE_DIR_NONE;
	else
		wind_dir = new_wind_dir;

	if (wind_dir != old_wind_dir)
		send_wind_change_notification_callback();

	set_wind_change_callback();

	return true;
}

inline void Weather::set_wind_change_callback() {
	uint16 length = (NUVIE_RAND() % WEATHER_MAX_WIND) + 1;
	uint8 *cb_msgid = new uint8;
	*cb_msgid = WEATHER_CB_CHANGE_WIND_DIR;
	wind_timer = new GameTimedCallback((CallBack *)this, cb_msgid, length);
	DEBUG(0, LEVEL_DEBUGGING, "Adding wind change timer. Length = %d\n", length);
}

inline void Weather::send_wind_change_notification_callback() {
	Std::list<CallBack *>::iterator cb_iter;
	for (cb_iter = wind_change_notification_list.begin(); cb_iter != wind_change_notification_list.end(); cb_iter++)
		(*cb_iter)->callback(WEATHER_CB_CHANGE_WIND_DIR, (CallBack *)this, NULL);
}

bool Weather::add_wind_change_notification_callback(CallBack *caller) {
	wind_change_notification_list.push_back(caller);

	return true;
}


uint16 Weather::callback(uint16 msg, CallBack *caller, void *data) {
	uint8 *cb_msgid = (uint8 *)callback_user_data;

	switch (*cb_msgid) {
	case WEATHER_CB_CHANGE_WIND_DIR :
		wind_timer = NULL;
		change_wind_dir();
		break;
	default :
		DEBUG(0, LEVEL_ERROR, "Weather: Unknown callback!\n");
		break;
	}

	delete cb_msgid;

	return 1;
}

} // End of namespace Nuvie
} // End of namespace Ultima
