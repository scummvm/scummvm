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
	Type _type,
	uint16 _objectID,
	uint16 _flags,
	const Vector3d &_origin,
	const Vector3d &_size,
	Common::Array<uint8> *_colours,
	Common::Array<uint16> *_ordinates,
	FCLInstructionVector _condition) {
	type = _type;
	flags = _flags;
	objectID = _objectID;
	origin = _origin;
	size = _size;

	if (_colours)
		colours = new Common::Array<uint8>(*_colours);
	if (_ordinates)
		ordinates = new Common::Array<uint16>(*_ordinates);
	condition = _condition;
}

GeometricObject::~GeometricObject() {
}

bool GeometricObject::isDrawable() { return true; }
bool GeometricObject::isPlanar() {
	Type type = this->getType();
	return (type >= Object::Line) || !size.x() || !size.y() || !size.z();
}

void GeometricObject::draw(Freescape::Renderer *gfx) {
	if (this->getType() == Cube) {
		gfx->renderCube(origin, size, colours);
	}
		
};