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
class SaveFileReadStream;
class SaveFileWriteStream;

class PoliceMazeTargetTrack : ScriptBase {
	friend class PoliceMaze;
	uint32     _time;
	bool       _isPresent;
	int        _itemId;
	int        _pointCount;
	Vector3    _points[kNumTrackPoints];
	const int *_data;
	int        _dataIndex;
	int32      _timeLeftUpdate;
	int32      _timeLeftWait;
	bool       _isWaiting;
	int        _isMoving;
	int        _pointIndex;
	int        _pointTarget;
	bool       _isRotating;
	int        _angleTarget;
	int        _angleDelta;
	bool       _isPaused;

public:
	PoliceMazeTargetTrack(BladeRunnerEngine *vm);
	~PoliceMazeTargetTrack() override;

	void reset();
	void clear(bool isLoadingGame);
	void add(int trackId, float startX, float startY, float startZ, float endX, float endY, float endZ, int steps, const int *instructions, bool isActive);

	bool tick();
	bool isPresent() const { return _isPresent; }
	void setPaused() { _isPaused = true; }
	void resetPaused() { _isPaused = false; }
	bool isPaused() const { return _isPaused; }
	void setTime(uint32 t) { _time = t; }

	void readdObject(int itemId);

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);
};

class PoliceMaze : ScriptBase {
	bool _isPaused;
	bool _isActive;
	bool _isEnding;
	int  _pm_var1;
	int  _pm_var2;

public:
	PoliceMazeTargetTrack *_tracks[kNumMazeTracks];

public:
	PoliceMaze(BladeRunnerEngine *vm);
	~PoliceMaze() override;

	void tick();
	void clear(bool isLoadingGame);
	void setPauseState(bool state);
	void activate();

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);
};

} // End of namespace BladeRunner

#endif
