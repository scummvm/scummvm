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

#ifndef TETRAEDGE_TE_TE_TIMER_H
#define TETRAEDGE_TE_TE_TIMER_H

#include "common/array.h"

#include "tetraedge/te/te_signal.h"
#include "tetraedge/te/te_real_timer.h"

namespace Tetraedge {

class TeTimer {
public:
	TeTimer();
	~TeTimer();

	void stop();
	void start();
	void pause();
	void update();
	unsigned long getTimeFromStart();
	void setAlarmIn(unsigned long offset);
	unsigned long timeElapsed();
	unsigned long timeFromLastTimeElapsed();
	unsigned long time_();
	void setTime(unsigned long time);

	void pausable(bool ispausable);

	TeSignal0Param &alarmSignal() { return _alarmSignal; }

	static void pauseAll();
	static void resumeAll();
	static void updateAll();

	static void cleanup();

	bool running() const { return !_stopped; }

private:
	static TeRealTimer *realTimer();
	static Common::Array<TeTimer *> *timers();
	static Common::Array<TeTimer *> *pausedTimers();

	unsigned long _startTime;
	unsigned long _startTimeOffset;
	unsigned long _alarmTime;
	unsigned long _lastTimeElapsed;
	bool _pausable;
	bool _alarmSet;
	bool _updated;
	bool _stopped;

	TeSignal0Param _alarmSignal;

	static bool _pausedAll;
	static unsigned long _realTime;
	static Common::Array<TeTimer *> *_timers;
	static Common::Array<TeTimer *> *_pausedTimers;
	static TeRealTimer *_realTimer;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_TIMER_H
