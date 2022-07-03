//
//  GeometricObject.cpp
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "common/str.h"
#include "freescape/objects/geometricobject.h"

#pragma mark -
#pragma mark Static Getters

namespace Freescape {

int GeometricObject::numberOfColoursForObjectOfType(Type type) {
	switch (type) {
	default:
	case Entrance:
	case Group:
	case Sensor:
		return 0;

	case Line:
		return 2;

	case Rectangle:
	case Triangle:
	case Quadrilateral:
	case Pentagon:
	case Hexagon:
		return 2;

	case Cube:
	case EastPyramid:
	case WestPyramid:
	case UpPyramid:
	case DownPyramid:
	case NorthPyramid:
	case SouthPyramid:
		return 6;
	}
}

int GeometricObject::numberOfOrdinatesForType(Type type) {
	switch (type) {
	default:
	case Entrance:
	case Group:
	case Rectangle:
	case Sensor:
		return 0;

	case EastPyramid:
	case WestPyramid:
	case UpPyramid:
	case DownPyramid:
	case NorthPyramid:
	case SouthPyramid:
		return 4;

	case Line:
	case Triangle:
	case Quadrilateral:
	case Pentagon:
	case Hexagon:
		return 3 * (2 + type - Line);
	}
}

bool GeometricObject::isPyramid(Type type) {
	switch (type) {
	default:
		return false;

	case EastPyramid:
	case WestPyramid:
	case UpPyramid:
	case DownPyramid:
	case NorthPyramid:
	case SouthPyramid:
		return true;
	}

}

bool GeometricObject::isPolygon(Type type) {
	switch (type) {
	default:
		return false;

	case Line:
	case Triangle:
	case Quadrilateral:
	case Pentagon:
	case Hexagon:
		return true;
	}
}

#pragma mark -
#pragma mark Construction/Destruction

GeometricObject::GeometricObject(
	Type type,
	uint16 objectID,
	uint16 flags,
	const Math::Vector3d &_origin,
	const Math::Vector3d &_size,
	Common::Array<uint8> *_colours,
	Common::Array<uint16> *_ordinates,
	FCLInstructionVector _conditionInstructions,
	Common::String *_conditionSource) {
	_type = type;
	_flags = flags;
	_objectID = objectID;
	origin = _origin;
	size = _size;

	if (_colours)
		colours = new Common::Array<uint8>(*_colours);
	if (_ordinates)
		ordinates = new Common::Array<uint16>(*_ordinates);
	condition = _conditionInstructions;
	conditionSource = _conditionSource;
	createBoundingBox();
}

void GeometricObject::createBoundingBox() {
	Math::Vector3d v;
	switch (_type) {
	default:
	break;
	case Cube:
		_boundingBox.expand(origin);
		for (int i = 0; i < 3; i++) {
			v = origin;
			v.setValue(i, v.getValue(i) + size.getValue(i));
			_boundingBox.expand(v);
		}

		for (int i = 0; i < 3; i++) {
			v = origin + size;
			v.setValue(i, v.getValue(i) - size.getValue(i));
			_boundingBox.expand(v);
		}
		_boundingBox.expand(origin + size);
		assert(_boundingBox.isValid());
		break;
	case Rectangle:
		_boundingBox.expand(origin);

		v = origin + size;
		for (int i = 0; i < 3; i++) {
			if (size.getValue(i) == 0)
				v.setValue(i, v.getValue(i) + 10);
		}
		_boundingBox.expand(v);
		break;
	case EastPyramid:
		_boundingBox.expand(origin + Math::Vector3d(0, 0, size.z()));
		_boundingBox.expand(origin + Math::Vector3d(0, size.y(), size.z()));
		_boundingBox.expand(origin + Math::Vector3d(0, size.y(), 0));

		_boundingBox.expand(origin + Math::Vector3d(size.x(), (*ordinates)[0], (*ordinates)[3]));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), (*ordinates)[2], (*ordinates)[3]));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), (*ordinates)[2], (*ordinates)[1]));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), (*ordinates)[0], (*ordinates)[1]));
		break;
	case WestPyramid:
		_boundingBox.expand(origin + Math::Vector3d(size.x(), 0, 0));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), 0));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), size.z()));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), 0, size.z()));

		_boundingBox.expand(origin + Math::Vector3d(0, (*ordinates)[0], (*ordinates)[1]));
		_boundingBox.expand(origin + Math::Vector3d(0, (*ordinates)[2], (*ordinates)[1]));
		_boundingBox.expand(origin + Math::Vector3d(0, (*ordinates)[2], (*ordinates)[3]));
		_boundingBox.expand(origin + Math::Vector3d(0, (*ordinates)[0], (*ordinates)[3]));
		break;
	case UpPyramid:
		_boundingBox.expand(origin + Math::Vector3d(size.x(), 0, 0));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), 0, size.z()));
		_boundingBox.expand(origin + Math::Vector3d(0, 0, size.z()));

		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], size.y(), (*ordinates)[1]));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], size.y(), (*ordinates)[1]));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], size.y(), (*ordinates)[3]));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], size.y(), (*ordinates)[3]));
		break;
	case DownPyramid:
		_boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), 0));
		_boundingBox.expand(origin + Math::Vector3d(0, size.y(), 0));
		_boundingBox.expand(origin + Math::Vector3d(0, size.y(), size.z()));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), size.z()));

		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], 0, (*ordinates)[1]));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], 0, (*ordinates)[1]));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], 0, (*ordinates)[3]));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], 0, (*ordinates)[3]));
		break;
	case NorthPyramid:
		_boundingBox.expand(origin + Math::Vector3d(0, size.y(), 0));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), 0));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), 0, 0));

		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], (*ordinates)[3], size.z()));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], (*ordinates)[3], size.z()));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], (*ordinates)[1], size.z()));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], (*ordinates)[1], size.z()));
		break;
	case SouthPyramid:
		_boundingBox.expand(origin + Math::Vector3d(0, 0, size.z()));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), 0, size.z()));
		_boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), size.z()));

		_boundingBox.expand(origin + Math::Vector3d(0, size.y(), size.z()));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], (*ordinates)[1], 0));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], (*ordinates)[1], 0));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], (*ordinates)[3], 0));
		_boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], (*ordinates)[3], 0));
		break;
	}
}

GeometricObject::~GeometricObject() {
}

bool GeometricObject::isDrawable() { return true; }
bool GeometricObject::isPlanar() {
	Type type = this->getType();
	return (type >= Object::Line) || !size.x() || !size.y() || !size.z();
}

bool GeometricObject::collides(const Math::AABB &boundingBox) {
	if (isInvisible() || !_boundingBox.isValid() || !boundingBox.isValid())
		return false;

	return(	_boundingBox.getMax().x() > boundingBox.getMin().x() &&
			_boundingBox.getMin().x() < boundingBox.getMax().x() &&
			_boundingBox.getMax().y() > boundingBox.getMin().y() &&
			_boundingBox.getMin().y() < boundingBox.getMax().y() &&
			_boundingBox.getMax().z() > boundingBox.getMin().z() &&
			_boundingBox.getMin().z() < boundingBox.getMax().z());
}

void GeometricObject::draw(Freescape::Renderer *gfx) {
	//debug("Drawing %d of type %d", this->getObjectID(), this->getType());
	if (this->getType() == Cube) {
		gfx->renderCube(origin, size, colours);
	} else if (this->getType() == Rectangle) {
		gfx->renderRectangle(origin, size, colours);
	} else if (isPyramid(this->getType())) {
		gfx->renderPyramid(origin, size, ordinates, colours, this->getType());
	} else if (this->isPlanar() && _type <= 14) {
		//debug("Drawing %d of type %d", this->getObjectID(), this->getType());
		gfx->renderPolygon(origin, size, ordinates, colours);
	}
}

} // End of namespace Freescape