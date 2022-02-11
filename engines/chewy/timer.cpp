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

#include "common/system.h"
#include "common/textconsole.h"
#include "chewy/chewy.h"
#include "chewy/timer.h"
#include "chewy/defines.h"

namespace Chewy {

bool timer_int;
int timer_count;
bool timer_suspend;

timer::timer(int16 max_t, TimerBlk *t) {
	_timerBlk = t;
	_timerMax = max_t;
	timer_int = false;
	timer_suspend = false;
	set_all_status(TIMER_STOP);
}

timer::~timer() {
}

float timer_freq[6] = {
	0.0182f,
	0.182f,
	1.82f,
	18.2f,
	1092.0f,
	65520.0f
};

void timer::calc_timer() {
	int16 i;
	int16 count;
	int ak_time;
	float freq;
	ak_time = timer_count;
	for (i = 0; i < _timerMax; i++) {
		if (_timerBlk[i].TimeStatus != TIMER_STOP) {
			freq = timer_freq[_timerBlk[i].TimeMode];

			_timerBlk[i].TimeLast = ((float)ak_time - (float)_timerBlk[i].TimeLast);

			count = (int16)((float)_timerBlk[i].TimeLast / (float)freq);
			if (_timerBlk[i].TimeStatus == TIMER_START)
				_timerBlk[i].TimeCount += count;

			_timerBlk[i].TimeLast = (float)ak_time - (float)(_timerBlk[i].TimeLast -
			                        (float)((float)count * (float)freq));

			if (_timerBlk[i].TimeCount >= _timerBlk[i].TimeEnd) {
				++_timerBlk[i].TimeFlag;
				_timerBlk[i].TimeCount = 0;
			}
		}
	}
}

int16 timer::set_new_timer(int16 timer_nr, int16 timer_end_wert, int16 timer_mode) {
	int16 ret;
	if (timer_nr < _timerMax) {
		ret = true;
		_timerBlk[timer_nr].TimeCount = 0;
		_timerBlk[timer_nr].TimeEnd = timer_end_wert;
		_timerBlk[timer_nr].TimeFlag = 0;
		_timerBlk[timer_nr].TimeLast = timer_count;
		_timerBlk[timer_nr].TimeMode = timer_mode;
		_timerBlk[timer_nr].TimeStatus = true;
	} else
		ret = -1;
	return ret;
}

void timer::reset_timer(int16 timer_nr, int16 timer_wert) {
	if (timer_nr < _timerMax) {
		_timerBlk[timer_nr].TimeCount = 0;
		_timerBlk[timer_nr].TimeFlag = 0;
		_timerBlk[timer_nr].TimeLast = timer_count;
		if (timer_wert)
			_timerBlk[timer_nr].TimeEnd = timer_wert;
	}
}

void timer::reset_all_timer() {
	int16 i;
	for (i = 0; i < _timerMax; i++) {
		_timerBlk[i].TimeCount = 0;
		_timerBlk[i].TimeFlag = 0;
		_timerBlk[i].TimeLast = timer_count;
	}
}

void timer::set_status(int16 timer_nr, int16 status) {
	if (timer_nr < _timerMax) {
		_timerBlk[timer_nr].TimeStatus = status;
	}
}

void timer::set_all_status(int16 status) {
	int16 i;
	if (status == TIMER_FREEZE) {
		for (i = 0; i < _timerMax; i++) {
			if (_timerBlk[i].TimeStatus != TIMER_STOP)
				_timerBlk[i].TimeStatus = TIMER_FREEZE;
		}
	}
	else if (status == TIMER_UNFREEZE) {
		for (i = 0; i < _timerMax; i++) {
			if (_timerBlk[i].TimeStatus != TIMER_STOP)
				_timerBlk[i].TimeStatus = TIMER_START;
		}
	} else {
		for (i = 0; i < _timerMax; i++)
			_timerBlk[i].TimeStatus = status;
	}
}

void timer::disable_timer() {
	timer_suspend = true;
}

void timer::enable_timer() {
	timer_suspend = false;
}

} // namespace Chewy
