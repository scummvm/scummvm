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

#ifndef BLADERUNNER_POLICE_MAZE_TRACK_H
#define BLADERUNNER_POLICE_MAZE_TRACK_H

#include "bladerunner/vector.h"

namespace BladeRunner {

class BladeRunnerEngine;
class SaveFile;

class PoliceMazeTrack {
	BladeRunnerEngine *_vm;

	int     _time;
	bool    _isPresent;
	int     _itemId;
	int     _count;
	Vector2 _points[100];
	int     _data;
	int     _dataIndex;
	int     _a4;
	int     _a5;
	int     _a6;
	int     _a7;
	int     _pointIndex;
	int     _a9;
	int     _rotating;
	int     _maxAngle;
	int     _angleChange;
	int     _a13;

public:
	PoliceMazeTrack(BladeRunnerEngine *vm);
	~PoliceMazeTrack();

	void save(SaveFile &f);

	void reset();
};

} // End of namespace BladeRunner

#endif
