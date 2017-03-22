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

#include "bladerunner/movement_track.h"

namespace BladeRunner {

MovementTrack::MovementTrack() {
	reset();
}

MovementTrack::~MovementTrack() {
	reset();
}

void MovementTrack::reset() {
	_currentIndex = -1;
	_lastIndex = 0;
	_hasNext = false;
	_paused = false;
	for (int i = 0; i < 100; i++) {
		_entries[i].waypointId = -1;
		_entries[i].delay = -1;
		_entries[i].angle = -1;
		_entries[i].running = 0;
	}
}

int MovementTrack::append(int waypointId, int delay, int running) {
	return append(waypointId, delay, -1, running);
}

int MovementTrack::append(int waypointId, int delay, int angle, int running) {	
	if (_lastIndex >= 100) {
		return 0;
	}

	_entries[_lastIndex].waypointId = waypointId;
	_entries[_lastIndex].delay = delay;
	_entries[_lastIndex].angle = angle;
	_entries[_lastIndex].running = running;

	_lastIndex++;
	_hasNext = true;
	_currentIndex = 0;
	return 1;
}

void MovementTrack::flush() {
	reset();
}

void MovementTrack::repeat() {
	_currentIndex = 0;
	_hasNext = true;
}

void MovementTrack::pause() {
	_paused = true;
}

void MovementTrack::unpause() {
	_paused = false;
}

bool MovementTrack::isPaused() {
	return _paused;
}

bool MovementTrack::hasNext() {
	return _hasNext;
}

bool MovementTrack::next(int *waypointId, int *delay, int *angle, int *running) {
	if (_currentIndex < _lastIndex && _hasNext) {
		*waypointId = _entries[_currentIndex].waypointId;
		*delay = _entries[_currentIndex].delay;
		*angle = _entries[_currentIndex].angle;
		*running = _entries[_currentIndex++].running;
		return true;
	} else {
		*waypointId = -1;
		*delay = -1;
		*angle = -1;
		*running = 0;
		_hasNext = false;
		return false;
	}
}

} // End of namespace BladeRunner
