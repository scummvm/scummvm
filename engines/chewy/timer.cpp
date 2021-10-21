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

#include "common/textconsole.h"
#include "chewy/chewy.h"
#include "chewy/timer.h"
#include "chewy/defines.h"

namespace Chewy {

bool timer_int;
int timer_count;
bool timer_suspend;

timer::timer(int16 max_t, TimerBlk *t) {
	timer_blk = t;
	timer_max = max_t;
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
	for (i = 0; i < timer_max; i++) {
		if (timer_blk[i].TimeStatus != TIMER_STOP) {
			freq = timer_freq[timer_blk[i].TimeMode];

			timer_blk[i].TimeLast = ((float)ak_time - (float)timer_blk[i].TimeLast);

			count = (int16)((float)timer_blk[i].TimeLast / (float)freq);
			if (timer_blk[i].TimeStatus == TIMER_START)
				timer_blk[i].TimeCount += count;

			timer_blk[i].TimeLast = (float)ak_time - (float)(timer_blk[i].TimeLast -
			                        (float)((float)count * (float)freq));

			if (timer_blk[i].TimeCount >= timer_blk[i].TimeEnd) {
				++timer_blk[i].TimeFlag;
				timer_blk[i].TimeCount = 0;
			}
		}
	}
}

int16 timer::set_new_timer(int16 timer_nr, int16 timer_end_wert, int16 timer_mode) {
	int16 ret;
	if (timer_nr < timer_max) {
		ret = true;
		timer_blk[timer_nr].TimeCount = 0;
		timer_blk[timer_nr].TimeEnd = timer_end_wert;
		timer_blk[timer_nr].TimeFlag = 0;
		timer_blk[timer_nr].TimeLast = timer_count;
		timer_blk[timer_nr].TimeMode = timer_mode;
		timer_blk[timer_nr].TimeStatus = true;
	} else
		ret = -1;
	return (ret);
}

void timer::reset_timer(int16 timer_nr, int16 timer_wert) {
	if (timer_nr < timer_max) {
		timer_blk[timer_nr].TimeCount = 0;
		timer_blk[timer_nr].TimeFlag = 0;
		timer_blk[timer_nr].TimeLast = timer_count;
		if (timer_wert)
			timer_blk[timer_nr].TimeEnd = timer_wert;
	}
}

void timer::reset_all_timer() {
	int16 i;
	for (i = 0; i < timer_max; i++) {
		timer_blk[i].TimeCount = 0;
		timer_blk[i].TimeFlag = 0;
		timer_blk[i].TimeLast = timer_count;
	}
}

void timer::set_status(int16 timer_nr, int16 status) {
	if (timer_nr < timer_max) {
		timer_blk[timer_nr].TimeStatus = status;
	}
}

void timer::set_all_status(int16 status) {
	int16 i;
	if (status == TIMER_FREEZE) {
		for (i = 0; i < timer_max; i++) {
			if (timer_blk[i].TimeStatus != TIMER_STOP)
				timer_blk[i].TimeStatus = TIMER_FREEZE;
		}
	}
	else if (status == TIMER_UNFREEZE) {
		for (i = 0; i < timer_max; i++) {
			if (timer_blk[i].TimeStatus != TIMER_STOP)
				timer_blk[i].TimeStatus = TIMER_START;
		}
	} else {
		for (i = 0; i < timer_max; i++)
			timer_blk[i].TimeStatus = status;
	}
}

void timer::disable_timer() {
	timer_suspend = true;
}

void timer::enable_timer() {
	timer_suspend = false;
}

} // namespace Chewy
