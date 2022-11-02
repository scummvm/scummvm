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

#define SEC_10_MODE 2
#define SEC_MODE 3

#define TIMER_STOP 0
#define TIMER_START 1
#define TIMER_FREEZE 2

#define TIMER_UNFREEZE 3

struct TimerBlk {
	int16 _timeCount;

	int16 _timeEnd;
	float _timeLast;
	int16 _timeFlag;

	int16 _timeMode;

	int16 _timeStatus;
};

class Timer {
public:
	Timer(int16 maxTimer, TimerBlk *t);
	~Timer();

	void calcTimer();
	int16 setNewTimer(int16 timerNr, int16 timerEndValue, int16 timerMode);
	void resetTimer(int16 timerNr, int16 timerValue);
	void resetAllTimer();
	void setStatus(int16 timerNr, int16 status);
	void setAllStatus(int16 status);
	void disableTimer();
	void enableTimer();

private:
	TimerBlk *_timerBlk;
	int16 _timerMax;
};

} // namespace Chewy

#endif
