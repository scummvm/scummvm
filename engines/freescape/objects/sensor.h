#ifndef FREESCAPE_SENSOR_H
#define FREESCAPE_SENSOR_H

#include "freescape/objects/object.h"

namespace Freescape {

class Sensor : public Object {
public:

	Sensor(
		uint16 objectID,
		const Math::Vector3d &origin,
		const Math::Vector3d &rotation);
	virtual ~Sensor();

	bool isDrawable();
	bool isPlanar();
	Type getType() override { return Type::Sensor; };
	Math::Vector3d getRotation() { return _rotation; }

	void draw(Freescape::Renderer *gfx) override { error("cannot render sensor"); };
};

} // End of namespace Freescape

#endif
