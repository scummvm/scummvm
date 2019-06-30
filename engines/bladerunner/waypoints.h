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

class SaveFileReadStream;
class SaveFileWriteStream;

class Waypoints {
	friend class Debugger;

	struct Waypoint {
		int     setId;
		Vector3 position;
		bool    present;
	};

	BladeRunnerEngine *_vm;

	int                     _count;
	Common::Array<Waypoint> _waypoints;

public:
	Waypoints(BladeRunnerEngine *vm, int count);

	void getXYZ(int waypointId, float *x, float *y, float *z) const;
	float getX(int waypointId) const;
	float getY(int waypointId) const;
	float getZ(int waypointId) const;
	int getSetId(int waypointId) const;

	bool set(int waypointId, int setId, Vector3 position);
	bool reset(int waypointId);

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);
};

} // End of namespace BladeRunner

#endif
