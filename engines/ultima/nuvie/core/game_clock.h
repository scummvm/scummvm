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

#ifndef NUVIE_CORE_GAME_CLOCK_H
#define NUVIE_CORE_GAME_CLOCK_H

#include "ultima/shared/std/containers.h"

#include "ultima/nuvie/core/nuvie_defs.h"

namespace Ultima {
namespace Nuvie {

using Std::vector;

#define GAMECLOCK_TICKS_PER_MINUTE   4

#define GAMECLOCK_NUM_TIMERS 16

#define GAMECLOCK_TIMER_U6_LIGHT 0
#define GAMECLOCK_TIMER_U6_INFRAVISION 1
#define GAMECLOCK_TIMER_U6_STORM 13
#define GAMECLOCK_TIMER_U6_TIME_STOP 14
#define GAMECLOCK_TIMER_U6_ECLIPSE 15

#define GAMECLOCK_TIMER_MD_BLUE_BERRY 16*3

class Configuration;
class NuvieIO;

class GameClock {
	Configuration *config;
	nuvie_game_t game_type;

	uint16 minute;
	uint8 hour;
	uint8 day;
	uint8 month;
	uint16 year;
	uint8 day_of_week;

	uint32 move_counter; // player steps taken since start
	uint32 time_counter; // game minutes
// uint32 tick_counter; // moves/turns since last minute

	char date_string[11];
	char time_string[11];

//bool active; // clock is active and running (false = paused)

	vector<uint8> timers;
	uint8 num_timers;

	uint8 rest_counter; //hours until the party will heal again while resting.

public:

	GameClock(Configuration *cfg, nuvie_game_t type);
	~GameClock();

	bool load(NuvieIO *objlist);
	bool save(NuvieIO *objlist);

//void set_active(bool state) { active = state; }
//bool get_active()           { return(active); }

	void inc_move_counter();
	void inc_move_counter_by_a_minute();

	void advance_to_next_hour();

	void inc_minute(uint16 amount = 1);
	void inc_hour();
	void inc_day();
	void inc_month();
	void inc_year();

	uint32 get_move_count();

	const char *get_time_of_day_string();

	uint8 get_hour();
	uint8 get_minute();

	uint8 get_day();
	uint8 get_month();
	uint16 get_year();
	uint8 get_day_of_week();

	char *get_date_string();
	char *get_time_string();

	uint8 get_rest_counter();
	void set_rest_counter(uint8 value) {
		rest_counter = value;
	}

	uint32 get_ticks() {
		return (SDL_GetTicks());    // milliseconds since start
	}
	uint32 get_game_ticks() {
		return (time_counter/**GAMECLOCK_TICKS_PER_MINUTE+tick_counter*/);
	}
// uint32 get_time()  { return(time_counter); } // get_game_ticks() is preferred
	uint32 get_turn()  {
		return (move_counter);
	}

	void set_timer(uint8 timer_num, uint8 val);
	uint8 get_timer(uint8 timer_num);
	void update_timers(uint8 amount);

//MD berry counters
	uint8 get_purple_berry_counter(uint8 actor_num) {
		return get_timer(actor_num * 3);
	}
	uint8 get_green_berry_counter(uint8 actor_num) {
		return get_timer(actor_num * 3 + 1);
	}
	uint8 get_brown_berry_counter(uint8 actor_num) {
		return get_timer(actor_num * 3 + 2);
	}

protected:

	void init();
	inline void update_day_of_week();

private:
	void load_U6_timers(NuvieIO *objlist);
	void load_MD_timers(NuvieIO *objlist);
	void save_U6_timers(NuvieIO *objlist);
	void save_MD_timers(NuvieIO *objlist);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
