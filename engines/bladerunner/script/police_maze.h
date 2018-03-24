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

#ifndef BLADERUNNER_SCRIPT_POLICE_MAZE_H
#define BLADERUNNER_SCRIPT_POLICE_MAZE_H

#include "bladerunner/script/script.h"
#include "bladerunner/vector.h"

namespace BladeRunner {

enum {
	kNumMazeTracks = 64,
	kNumTrackPoints = 100
};

class BladeRunnerEngine;

class PoliceMazeTargetTrack : ScriptBase {
	BladeRunnerEngine *_vm;

	uint32 _time;
	bool _isPresent;
	int _itemId;
	int _count;
	Vector3 _points[kNumTrackPoints];
	int *_data;
	int _dataIndex;
	int32 _updateDelay;
	int32 _waitTime;
	bool _haveToWait;
	int _pmt_var4;
	int _pointIndex;
	int _pmt_var5;
	bool _rotating;
	int _maxAngle;
	int _angleChange;
	bool _visible;

public:
	PoliceMazeTargetTrack(BladeRunnerEngine *vm);
	~PoliceMazeTargetTrack();

	void reset();
	void clear(bool isLoadingGame);
	void add(int trackId, float startX, float startY, float startZ, float endX, float endY, float endZ, int count, void *list, bool a11);

	bool tick();
	bool isPresent() { return _isPresent; }
	void setVisible() { _visible = true; }
	void resetVisible() { _visible = false; }
	bool isVisible() { return _visible; }
	void setTime(uint32 t) { _time = t; }

	void readdObject(int itemId);
};

class PoliceMaze : ScriptBase {
	BladeRunnerEngine *_vm;

	bool _isPaused;
	bool _needAnnouncement;
	bool _announcementRead;
	int _pm_var1;
	int _pm_var2;

public:
	PoliceMazeTargetTrack *_tracks[kNumMazeTracks];

public:
	PoliceMaze(BladeRunnerEngine *vm);
	~PoliceMaze();

	void tick();
	void reset();
	void clear(bool isLoadingGame);
	void setPauseState(bool state);
	void activate();
};

} // End of namespace BladeRunner

#endif
