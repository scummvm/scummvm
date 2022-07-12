#include "freescape/objects/sensor.h"

namespace Freescape {

Sensor::Sensor(
	uint16 objectID,
	const Math::Vector3d &_origin,
	const Math::Vector3d &rotation) {
	_objectID = objectID;
	origin = _origin;
	_rotation = rotation;
}

Sensor::~Sensor() {}

bool Sensor::isDrawable() { return false; }
bool Sensor::isPlanar() { return true; }

} // End of namespace Freescape
