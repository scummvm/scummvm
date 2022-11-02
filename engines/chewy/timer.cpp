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
#include "chewy/chewy.h"
#include "chewy/defines.h"
#include "chewy/globals.h"
#include "chewy/timer.h"

namespace Chewy {

Timer::Timer(int16 max_t, TimerBlk *t) {
	_timerBlk = t;
	_timerMax = max_t;
	_G(timer_int) = false;
	_G(timer_suspend) = false;
	setAllStatus(TIMER_STOP);
}

Timer::~Timer() {
}

float timer_freq[6] = {
	0.0182f,
	0.182f,
	1.82f,
	18.2f,
	1092.0f,
	65520.0f
};

void Timer::calcTimer() {
	int ak_time = _G(timer_count);
	for (int16 i = 0; i < _timerMax; i++) {
		if (_timerBlk[i]._timeStatus != TIMER_STOP) {
			float freq = timer_freq[_timerBlk[i]._timeMode];

			_timerBlk[i]._timeLast = ((float)ak_time - (float)_timerBlk[i]._timeLast);

			int16 count = (int16)((float)_timerBlk[i]._timeLast / (float)freq);
			if (_timerBlk[i]._timeStatus == TIMER_START)
				_timerBlk[i]._timeCount += count;

			_timerBlk[i]._timeLast = (float)ak_time - (float)(_timerBlk[i]._timeLast -
			                        (float)((float)count * (float)freq));

			if (_timerBlk[i]._timeCount >= _timerBlk[i]._timeEnd) {
				++_timerBlk[i]._timeFlag;
				_timerBlk[i]._timeCount = 0;
			}
		}
	}
}

int16 Timer::setNewTimer(int16 timerNr, int16 timerEndValue, int16 timerMode) {
	int16 ret = -1;

	if (timerNr < _timerMax) {
		ret = 1;
		_timerBlk[timerNr]._timeCount = 0;
		_timerBlk[timerNr]._timeEnd = timerEndValue;
		_timerBlk[timerNr]._timeFlag = 0;
		_timerBlk[timerNr]._timeLast = _G(timer_count);
		_timerBlk[timerNr]._timeMode = timerMode;
		_timerBlk[timerNr]._timeStatus = true;
	}

	return ret;
}

void Timer::resetTimer(int16 timerNr, int16 timerValue) {
	if (timerNr < _timerMax) {
		_timerBlk[timerNr]._timeCount = 0;
		_timerBlk[timerNr]._timeFlag = 0;
		_timerBlk[timerNr]._timeLast = _G(timer_count);
		if (timerValue)
			_timerBlk[timerNr]._timeEnd = timerValue;
	}
}

void Timer::resetAllTimer() {
	for (int16 i = 0; i < _timerMax; i++) {
		_timerBlk[i]._timeCount = 0;
		_timerBlk[i]._timeFlag = 0;
		_timerBlk[i]._timeLast = _G(timer_count);
	}
}

void Timer::setStatus(int16 timerNr, int16 status) {
	if (timerNr < _timerMax) {
		_timerBlk[timerNr]._timeStatus = status;
	}
}

void Timer::setAllStatus(int16 status) {
	if (status == TIMER_FREEZE) {
		for (int16 i = 0; i < _timerMax; i++) {
			if (_timerBlk[i]._timeStatus != TIMER_STOP)
				_timerBlk[i]._timeStatus = TIMER_FREEZE;
		}
	} else if (status == TIMER_UNFREEZE) {
		for (int16 i = 0; i < _timerMax; i++) {
			if (_timerBlk[i]._timeStatus != TIMER_STOP)
				_timerBlk[i]._timeStatus = TIMER_START;
		}
	} else {
		for (int16 i = 0; i < _timerMax; i++)
			_timerBlk[i]._timeStatus = status;
	}
}

void Timer::disableTimer() {
	_G(timer_suspend) = true;
}

void Timer::enableTimer() {
	_G(timer_suspend) = false;
}

} // namespace Chewy
