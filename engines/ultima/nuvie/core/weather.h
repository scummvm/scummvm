/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NUVIE_CORE_WEATHER_H
#define NUVIE_CORE_WEATHER_H

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/call_back.h"
#include "ultima/nuvie/core/map.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class NuvieIO;
class CallBack;
class GameClock;
class GameTimedCallback;

using Std::list;
using Std::string;

//our callbacks

#define WEATHER_CB_CHANGE_WIND_DIR 1
#define WEATHER_CB_END_ECLIPSE     2
#define WEATHER_WIND_CALM 8

class Weather: public CallBack {
	const Configuration *config;
	GameClock *_clock;
	nuvie_game_t gametype; // what game is being played?

	NuvieDir wind_dir;
	Std::list<CallBack *>wind_change_notification_list;

	GameTimedCallback *wind_timer;

public:

	Weather(const Configuration *cfg, GameClock *c, nuvie_game_t type);
	~Weather() override;

	bool load(NuvieIO *objlist);
	bool save(NuvieIO *objlist);

	Std::string get_wind_dir_str() const;
	NuvieDir get_wind_dir() const {
		return wind_dir;
	}
	bool is_displaying_from_wind_dir() const {
		return display_from_wind_dir;
	}
	bool set_wind_dir(NuvieDir new_wind_dir);
	bool add_wind_change_notification_callback(CallBack *caller);
	bool set_moonstone(uint8 moonstone, MapCoord where);
	MapCoord get_moonstone(uint8 moonstone);
	void update_moongates();

	bool is_eclipse() const;
	bool is_moon_visible() const;

	uint16 callback(uint16 msg, CallBack *caller, void *data = nullptr) override;

protected:

	NuvieDir load_wind(NuvieIO *objlist);
	bool save_wind(NuvieIO *objlist);
	void change_wind_dir();
	inline void set_wind_change_callback();
	inline void send_wind_change_notification_callback();
	void clear_wind();
	bool display_from_wind_dir;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
