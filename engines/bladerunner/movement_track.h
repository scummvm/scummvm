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

#ifndef BLADERUNNER_MOVEMENT_TRACK_H
#define BLADERUNNER_MOVEMENT_TRACK_H

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

class BladeRunnerEngine;
class BoundingBox;

struct MovementTrackEntry {
	int waypointId;
	int delay;
	int angle;
	int running;
};

class MovementTrack {
//	BladeRunnerEngine *_vm;

private:
	int _currentIndex;
	int _lastIndex;
	bool _hasNext;
	bool _paused;
	MovementTrackEntry _entries[100];
	void reset();

public:
	MovementTrack();
	~MovementTrack();
	int append(int waypointId, int delay, int running);
	int append(int waypointId, int delay, int angle, int running);
	void flush();
	void repeat();
	void pause();
	void unpause();
	bool isPaused();
	bool hasNext();
	bool next(int *waypointId, int *delay, int *angle, int *running);

	//int saveGame();
};

} // End of namespace BladeRunner

#endif
