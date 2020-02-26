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

#include "bladerunner/savefile.h"

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
	for (int i = 0; i < kSize; ++i) {
		_entries[i].waypointId = -1;
		_entries[i].delay = -1;
		_entries[i].angle = -1;
		_entries[i].run = false;
	}
}

int MovementTrack::append(int waypointId, int32 delay, bool run) {
	return append(waypointId, delay, -1, run);
}

int MovementTrack::append(int waypointId, int32 delay, int angle, bool run) {
	if (_lastIndex >= kSize) {
		return 0;
	}

	_entries[_lastIndex].waypointId = waypointId;
	_entries[_lastIndex].delay = delay;
	_entries[_lastIndex].angle = angle;
	_entries[_lastIndex].run = run;

	++_lastIndex;
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

bool MovementTrack::isPaused() const {
	return _paused;
}

bool MovementTrack::hasNext() const {
	return _hasNext;
}

bool MovementTrack::next(int *waypointId, int32 *delay, int *angle, bool *run) {
	if (_currentIndex < _lastIndex && _hasNext) {
		*waypointId = _entries[_currentIndex].waypointId;
		*delay = _entries[_currentIndex].delay;
		*angle = _entries[_currentIndex].angle;
		*run = _entries[_currentIndex++].run;
		return true;
	} else {
		*waypointId = -1;
		*delay = -1;
		*angle = -1;
		*run = false;
		_hasNext = false;
		return false;
	}
}

void MovementTrack::save(SaveFileWriteStream &f) {
	f.writeInt(_currentIndex);
	f.writeInt(_lastIndex);
	f.writeBool(_hasNext);
	f.writeBool(_paused);
	for (int i = 0; i < kSize; ++i) {
		Entry &e = _entries[i];
		f.writeInt(e.waypointId);
		f.writeInt(e.delay);
		f.writeInt(e.angle);
		f.writeBool(e.run);
	}
}

void MovementTrack::load(SaveFileReadStream &f) {
	_currentIndex = f.readInt();
	_lastIndex = f.readInt();
	_hasNext = f.readBool();
	_paused = f.readBool();
	for (int i = 0; i < kSize; ++i) {
		Entry &e = _entries[i];
		e.waypointId = f.readInt();
		e.delay = f.readInt();
		e.angle = f.readInt();
		e.run = f.readBool();
	}
}

} // End of namespace BladeRunner
