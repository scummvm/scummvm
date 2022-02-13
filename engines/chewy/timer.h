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

#ifndef CHEWY_TIMER_H
#define CHEWY_TIMER_H

namespace Chewy {

#define SEC_1000_MODE 0
#define SEC_100_MODE 1
#define SEC_10_MODE 2
#define SEC_MODE 3
#define MIN_MODE 4
#define STD_MODE 5

#define TIMER_STOP 0
#define TIMER_START 1
#define TIMER_FREEZE 2

#define TIMER_UNFREEZE 3

struct TimerBlk {
	int16 TimeCount;

	int16 TimeEnd;
	float TimeLast;
	int16 TimeFlag;

	int16 TimeMode;

	int16 TimeStatus;
};

class Timer {
public:
	Timer(int16 max_timer, TimerBlk *t);
	~Timer();

	void calc_timer();
	int16 set_new_timer(int16 timer_nr, int16 timer_end_wert,
	                    int16 timer_mode);
	void reset_timer(int16 timer_nr, int16 timer_wert);
	void reset_all_timer();
	void set_status(int16 timer_nr, int16 status);
	void set_all_status(int16 status);
	void disable_timer();
	void enable_timer();

private:
	TimerBlk *_timerBlk;
	int16 _timerMax;
};

} // namespace Chewy

#endif
