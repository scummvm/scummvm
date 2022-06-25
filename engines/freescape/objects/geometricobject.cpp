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

#pragma mark -
#pragma mark Construction/Destruction

GeometricObject::GeometricObject(
	Type type,
	uint16 objectID,
	uint16 flags,
	const Math::Vector3d &origin,
	const Math::Vector3d &size,
	Common::Array<uint8> *_colours,
	Common::Array<uint16> *_ordinates,
	FCLInstructionVector _condition) {
	_type = type;
	_flags = flags;
	_objectID = objectID;
	_origin = origin;
	_size = size;

	if (_colours)
		colours = new Common::Array<uint8>(*_colours);
	if (_ordinates)
		ordinates = new Common::Array<uint16>(*_ordinates);
	condition = _condition;

	if (_type == Cube) {
		Math::Vector3d v;
		v = origin;
		_boundingBox.expand(v);

		for (int i = 0; i < 3; i++) {
			v = origin;
			v.setValue(i, origin.getValue(i) + size.getValue(i));
			_boundingBox.expand(v);
		}

		v = size;
		_boundingBox.expand(v);
	}
}

GeometricObject::~GeometricObject() {
}

bool GeometricObject::isDrawable() { return true; }
bool GeometricObject::isPlanar() {
	Type type = this->getType();
	return (type >= Object::Line) || !_size.x() || !_size.y() || !_size.z();
}

void GeometricObject::draw(Freescape::Renderer *gfx) {
	//debug("Drawing %d of type %d", this->getObjectID(), this->getType());
	if (this->getType() == Cube) {
		//debug("Drawing cube!");
		gfx->renderCube(_origin, _size, colours);
	} else if (this->getType() == Rectangle) {
		gfx->renderRectangle(_origin, _size, colours);
	} else if (this->isPlanar()) {
		//debug("Drawing %d of type %d", this->getObjectID(), this->getType());
		gfx->renderPolygon(_origin, _size, ordinates, colours);
	}
}