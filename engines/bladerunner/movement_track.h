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

#ifndef BLADERUNNER_MOVEMENT_TRACK_H
#define BLADERUNNER_MOVEMENT_TRACK_H

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

class BladeRunnerEngine;
class BoundingBox;
class SaveFileReadStream;
class SaveFileWriteStream;

class MovementTrack {
	static const int kSize = 100;

	struct Entry {
		int     waypointId;
		// delay specifies how long (in milliseconds) to stay at the target waypoint (when reached)
		int32   delay;
		int     angle;
		bool    run;
	};

	int   _currentIndex;
	int   _lastIndex;
	bool  _hasNext;
	bool  _paused;
	Entry _entries[kSize];

public:
	MovementTrack();
	~MovementTrack();
	int append(int waypointId, int32 delayMillis, bool run);
	int append(int waypointId, int32 delayMillis, int angle, bool run);
	void flush();
	void repeat();
	void pause();
	void unpause();
	bool isPaused() const;
	bool hasNext() const;
	bool next(int *waypointId, int32 *delayMillis, int *angle, bool *run);

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

private:
	void reset();
};

} // End of namespace BladeRunner

#endif
