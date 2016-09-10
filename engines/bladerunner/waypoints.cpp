#include "bladerunner/waypoints.h"
namespace BladeRunner {

Waypoints::Waypoints(BladeRunnerEngine* vm, int count) {
	_count = count;
	_waypoints = new Waypoint[count];
}

Waypoints::~Waypoints() {
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
