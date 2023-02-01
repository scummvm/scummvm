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

#ifndef TETRAEDGE_TE_TE_REAL_TIMER_H
#define TETRAEDGE_TE_TE_REAL_TIMER_H

namespace Tetraedge {

/* All times in microseconds. */
class TeRealTimer {
public:
	TeRealTimer();

	uint64 getTimeFromStart();
	void pause();
	void start();
	void stop();
	uint64 systemTime();
	uint64 monotonicSystemTime();
	uint64 time_();
	uint64 timeElapsed();
	uint64 timeFromLastTimeElapsed();

	bool isPaused() const { return _paused; }

private:
	bool _paused;
	uint64 _startTime;
	uint64 _startTime2;
	uint64 _pausedTime;
	uint64 _maxTimeSeen;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_REAL_TIMER_H
