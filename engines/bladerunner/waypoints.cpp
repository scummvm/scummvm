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

#include "bladerunner/waypoints.h"

#include "bladerunner/savefile.h"

namespace BladeRunner {

Waypoints::Waypoints(BladeRunnerEngine *vm, int count) {
	_vm = vm;
	_count = count;
	_waypoints.resize(count);
}

void Waypoints::getXYZ(int waypointId, float *x, float *y, float *z) const {
	*x = 0;
	*y = 0;
	*z = 0;

	if (waypointId < 0 || waypointId >= _count || !_waypoints[waypointId].present) {
		return;
	}

	*x = _waypoints[waypointId].position.x;
	*y = _waypoints[waypointId].position.y;
	*z = _waypoints[waypointId].position.z;
}

int Waypoints::getSetId(int waypointId) const {
	if (waypointId < 0 || waypointId >= _count || !_waypoints[waypointId].present) {
		return -1;
	}
	return _waypoints[waypointId].setId;
}

bool Waypoints::set(int waypointId, int setId, Vector3 position) {
	if (waypointId < 0 || waypointId >= _count) {
		return false;
	}

	_waypoints[waypointId].setId = setId;
	_waypoints[waypointId].position = position;
	_waypoints[waypointId].present = true;

	return true;
}

bool Waypoints::reset(int waypointId) {
	if (waypointId < 0 || waypointId >= _count) {
		return false;
	}

	_waypoints[waypointId].setId = -1;
	_waypoints[waypointId].position.x = 0;
	_waypoints[waypointId].position.y = 0;
	_waypoints[waypointId].position.z = 0;
	_waypoints[waypointId].present = false;

	return true;
}

float Waypoints::getX(int waypointId) const {
	return _waypoints[waypointId].position.x;
}

float Waypoints::getY(int waypointId) const {
	return _waypoints[waypointId].position.y;
}

float Waypoints::getZ(int waypointId) const {
	return _waypoints[waypointId].position.z;
}

void Waypoints::save(SaveFileWriteStream &f) {
	f.writeInt(_count);
	for (int i = 0; i < _count; ++i) {
		Waypoint &w = _waypoints[i];
		f.writeInt(w.setId);
		f.writeVector3(w.position);
		f.writeInt(w.present);
	}
}

void Waypoints::load(SaveFileReadStream &f) {
	_count = f.readInt();
	for (int i = 0; i < _count; ++i) {
		Waypoint &w = _waypoints[i];
		w.setId = f.readInt();
		w.position = f.readVector3();
		w.present = f.readInt();
	}
}

} // End of namespace BladeRunner
