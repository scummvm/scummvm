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
namespace BladeRunner {

Waypoints::Waypoints(BladeRunnerEngine *vm, int count) {
	_vm = vm;
	_count = count;
	_waypoints = new Waypoint[count];
}

Waypoints::~Waypoints() {
	delete[] _waypoints;
}

void Waypoints::getXYZ(int waypointId, float *x, float *y, float *z) {
	*x = 0;
	*y = 0;
	*z = 0;

	if (waypointId < 0 || waypointId >= _count || !_waypoints[waypointId]._present)
		return;

	*x = _waypoints[waypointId]._position.x;
	*y = _waypoints[waypointId]._position.y;
	*z = _waypoints[waypointId]._position.z;
}

int Waypoints::getSetId(int waypointId) {
	if (waypointId < 0 || waypointId >= _count || !_waypoints[waypointId]._present)
		return -1;
	return _waypoints[waypointId]._setId;
}

bool Waypoints::set(int waypointId, int setId, Vector3 position) {
	if (waypointId < 0 || waypointId >= _count)
		return false;

	_waypoints[waypointId]._setId = setId;
	_waypoints[waypointId]._position = position;
	_waypoints[waypointId]._present = true;

	return true;
}

bool Waypoints::reset(int waypointId) {
	if (waypointId < 0 || waypointId >= _count)
		return false;

	_waypoints[waypointId]._setId = -1;
	_waypoints[waypointId]._position.x = 0;
	_waypoints[waypointId]._position.y = 0;
	_waypoints[waypointId]._position.z = 0;
	_waypoints[waypointId]._present = false;

	return true;
}

float Waypoints::getX(int waypointId) {
	return _waypoints[waypointId]._position.x;
}

float Waypoints::getY(int waypointId) {
	return _waypoints[waypointId]._position.y;
}

float Waypoints::getZ(int waypointId) {
	return _waypoints[waypointId]._position.z;
}

} // End of namespace BladeRunner
