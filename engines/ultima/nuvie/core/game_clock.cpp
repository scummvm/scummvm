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
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/save/obj_list.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/core/game_clock.h"

namespace Ultima {
namespace Nuvie {

GameClock::GameClock(Configuration *cfg, nuvie_game_t type) {
	config = cfg;
	game_type = type;

	day_of_week = 0;
	date_string[10] = '\0';
	time_string[10] = '\0';

	init();
}

GameClock::~GameClock() {

}

void GameClock::init() {
	move_counter = 0;
	time_counter = 0;
// tick_counter = 0;

	minute = 0;
	hour = 0;
	day = 0;
	month = 0;
	year = 0;
	rest_counter = 0;
//active = true;
	num_timers = 0;
}

bool GameClock::load(NuvieIO *objlist) {
	init();

	if (game_type == NUVIE_GAME_U6) {
		objlist->seek(OBJLIST_OFFSET_U6_GAMETIME); // start of time data
	} else {
		objlist->seek(OBJLIST_OFFSET_WOU_GAMETIME); // start of time data
	}
	minute = objlist->read1();
	hour = objlist->read1();
	day = objlist->read1();
	month = objlist->read1();
	year = objlist->read2();

	update_day_of_week();

	if (game_type == NUVIE_GAME_U6) {
		load_U6_timers(objlist);
	} else if (game_type == NUVIE_GAME_MD) {
		load_MD_timers(objlist);
	}

	DEBUG(0, LEVEL_INFORMATIONAL, "Loaded game clock: %s %s\n", get_date_string(), get_time_string());

	return true;
}

void GameClock::load_U6_timers(NuvieIO *objlist) {
	num_timers = GAMECLOCK_NUM_TIMERS;
	timers.reserve(num_timers);
	timers.clear();
	objlist->seek(OBJLIST_OFFSET_U6_TIMERS);

	for (uint8 i = 0; i < GAMECLOCK_NUM_TIMERS; i++) {
		timers.push_back(objlist->read1());
	}

	objlist->seek(OBJLIST_OFFSET_U6_REST_COUNTER);
	rest_counter = objlist->read1();
}

void GameClock::load_MD_timers(NuvieIO *objlist) {
	num_timers = GAMECLOCK_NUM_TIMERS * 3 + 1; //three berries per party member. 16 slots + 1 for the blue berry counter.
	timers.reserve(num_timers);
	timers.clear();
	objlist->seek(OBJLIST_OFFSET_MD_BERRY_TIMERS);

	for (uint8 i = 0; i < GAMECLOCK_NUM_TIMERS; i++) {
		uint8 byte = objlist->read1();

		timers.push_back((uint8)(byte & 0xf));   //purple
		timers.push_back((uint8)(byte >> 4));    //green
		timers.push_back((uint8)(objlist->read1() & 0xf));    //brown
	}

	objlist->seek(OBJLIST_OFFSET_MD_BLUE_BERRY_COUNTER);
	timers.push_back(objlist->read1()); //blue berry counter
}

bool GameClock::save(NuvieIO *objlist) {
	objlist->seek(OBJLIST_OFFSET_U6_GAMETIME); // start of time data

	objlist->write1(minute);
	objlist->write1(hour);
	objlist->write1(day);
	objlist->write1(month);
	objlist->write2(year);

	if (game_type == NUVIE_GAME_U6) {
		save_U6_timers(objlist);
	} else if (game_type == NUVIE_GAME_MD) {
		save_MD_timers(objlist);
	}

	return true;
}

void GameClock::save_U6_timers(NuvieIO *objlist) {
	objlist->seek(OBJLIST_OFFSET_U6_TIMERS);

	for (uint8 i = 0; i < num_timers; i++) {
		objlist->write1(timers[i]);
	}

	objlist->seek(OBJLIST_OFFSET_U6_REST_COUNTER);
	objlist->write1(rest_counter);
}

void GameClock::save_MD_timers(NuvieIO *objlist) {
	objlist->seek(OBJLIST_OFFSET_MD_BERRY_TIMERS);

	for (uint8 i = 0; i < num_timers - 1; i += 3) {
		objlist->write1((uint8)(timers[i + 1] << 4) + timers[i]);
		objlist->write1(timers[i + 2]);
	}

	objlist->seek(OBJLIST_OFFSET_MD_BLUE_BERRY_COUNTER);
	objlist->write1(timers[num_timers - 1]);
}

void GameClock::inc_move_counter() {
	move_counter++;

	/* if((move_counter % GAMECLOCK_TICKS_PER_MINUTE) == 0)
	   inc_minute();
	 else
	   tick_counter++;*/ // commented out because time is updated independently

	return;
}

// move_counter by a minute.

void GameClock::inc_move_counter_by_a_minute() {
	move_counter += GAMECLOCK_TICKS_PER_MINUTE;

	/* inc_minute();*/ // commented out because time is updated independently
}

// advance game time to the start of the next hour.

void GameClock::advance_to_next_hour() {
	minute = 0;
	inc_hour();
}

void GameClock::inc_minute(uint16 amount) {
	minute += amount;

	if (minute >= 60) {
		for (; minute >= 60; minute -= 60) {
			inc_hour();
		}
		time_counter += minute;
		DEBUG(0, LEVEL_INFORMATIONAL, "%s\n", get_time_string());
	} else {
		time_counter += amount;
	}

//update_timers(1);
	return;
}

void GameClock::inc_hour() {
	if (rest_counter > 0)
		rest_counter--;

	if (hour == 23) {
		hour = 0;
		inc_day();
	} else {
		hour++;
		time_counter += 60;
	}


	if (game_type == NUVIE_GAME_U6)
		Game::get_game()->get_weather()->update_moongates();

	return;
}

void GameClock::inc_day() {
	if (day == 28) {
		day = 1;
		inc_month();
	} else {
		day++;
		time_counter += 1440;
	}
	update_day_of_week();

	DEBUG(0, LEVEL_INFORMATIONAL, "%s\n", get_date_string());


	return;
}

void GameClock::inc_month() {
	if (month == 12) {
		month = 1;
		inc_year();
	} else {
		month++;
		time_counter += 40320;
	}
	return;
}

void GameClock::inc_year() {
	year++;
	time_counter += 483840;
	return;
}

uint32 GameClock::get_move_count() {
	return move_counter;
}

const char *GameClock::get_time_of_day_string() {
	if (hour < 12)
		return "morning";

	if (hour >= 12 && hour <= 18)
		return "afternoon";

	return "evening";
}

uint8 GameClock::get_hour() {
	return hour;
}

uint8 GameClock::get_minute() {
	return minute;
}

uint8 GameClock::get_day() {
	return day;
}

uint8 GameClock::get_month() {
	return month;
}

uint16 GameClock::get_year() {
	return year;
}

uint8 GameClock::get_day_of_week() {
	return day_of_week;
}

char *GameClock::get_date_string() {

	sprintf(date_string, "%2u-%02u-%04u", month, day, year);

	return date_string;
}

char *GameClock::get_time_string() {
	char c;
	uint8 tmp_hour;

	if (hour < 12)
		c = 'A';
	else
		c = 'P';

	if (hour > 12)
		tmp_hour = hour - 12;
	else {
		if (hour == 0)
			tmp_hour = 12;
		else
			tmp_hour = hour;
	}

	sprintf(time_string, "%0u:%02u %c.M.", tmp_hour, minute, c);

	return time_string;
}

uint8 GameClock::get_rest_counter() {
	return rest_counter;
}

inline void GameClock::update_day_of_week() {
	day_of_week = day % 7;
	if (day_of_week == 0)
		day_of_week = 7;
}

void GameClock::set_timer(uint8 timer_num, uint8 val) {
	if (timer_num < num_timers) {
		timers[timer_num] = val;
	}
}

uint8 GameClock::get_timer(uint8 timer_num) {
	if (timer_num < num_timers) {
		return timers[timer_num];
	}

	return 0;
}

void GameClock::update_timers(uint8 amount) {
	for (uint8 i = 0; i < num_timers; i++) {
		if (timers[i] > amount) {
			timers[i] -= amount;
		} else
			timers[i] = 0;
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
