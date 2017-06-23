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

#ifndef BLADERUNNER_WAYPOINTS_H
#define BLADERUNNER_WAYPOINTS_H

#include "bladerunner/bladerunner.h"
#include "bladerunner/vector.h"

#include "common/array.h"

namespace BladeRunner {

struct Waypoint {
	int _setId;
	Vector3 _position;
	bool _present;
};

class Waypoints {
	BladeRunnerEngine *_vm;

public:
	int       _count;
	Waypoint *_waypoints;

public:
	Waypoints(BladeRunnerEngine *vm, int count);
	~Waypoints();

	void getXYZ(int waypointId, float *x, float *y, float *z);
	float getX(int waypointId);
	float getY(int waypointId);
	float getZ(int waypointId);
	int getSetId(int waypointId);

	bool set(int waypointId, int setId, Vector3 position);
	bool reset(int waypointId);
};

} // End of namespace BladeRunner

#endif
