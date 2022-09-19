/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Based on Phantasma code by Thomas Harte (2013)

#include "freescape/objects/geometricobject.h"

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

GeometricObject::GeometricObject(
	Type _type,
	uint16 _objectID,
	uint16 _flags,
	const Math::Vector3d &_origin,
	const Math::Vector3d &_size,
	Common::Array<uint8> *_colours,
	Common::Array<uint16> *_ordinates,
	FCLInstructionVector _conditionInstructions,
	Common::String *_conditionSource) {
	type = _type;
	flags = _flags;

	if (isDestroyed()) // If the object is destroyed, restore it
		flags = flags & ~0x20;

	objectID = _objectID;
	origin = _origin;
	size = _size;

	colours = nullptr;

	if (_colours)
		colours = new Common::Array<uint8>(*_colours);

	ordinates = nullptr;

	if (_ordinates)
		ordinates = new Common::Array<uint16>(*_ordinates);
	condition = _conditionInstructions;
	conditionSource = _conditionSource;

	if (type == Type::Rectangle) {
		if ((size.x() == 0 && size.y() == 0) ||
			(size.y() == 0 && size.z() == 0) ||
			(size.x() == 0 && size.z() == 0)) {

			type = Type::Line;
			assert(!ordinates);
			ordinates = new Common::Array<uint16>();
			ordinates->push_back(origin.x());
			ordinates->push_back(origin.y());
			ordinates->push_back(origin.z());

			ordinates->push_back(origin.x() + size.x());
			ordinates->push_back(origin.y() + size.y());
			ordinates->push_back(origin.z() + size.z());
		   }
	}

	computeBoundingBox();
}

void GeometricObject::setOrigin(Math::Vector3d _origin) {
	origin = _origin;
	computeBoundingBox();
};

GeometricObject *GeometricObject::duplicate() {
	return new GeometricObject(
		type,
		objectID,
		flags,
		origin,
		size,
		colours,
		ordinates,
		condition,
		conditionSource);
}

void GeometricObject::computeBoundingBox() {
	boundingBox = Math::AABB();
	Math::Vector3d v;
	switch (type) {
	default:
	break;
	case Cube:
		boundingBox.expand(origin);
		for (int i = 0; i < 3; i++) {
			v = origin;
			v.setValue(i, v.getValue(i) + size.getValue(i));
			boundingBox.expand(v);
		}

		for (int i = 0; i < 3; i++) {
			v = origin + size;
			v.setValue(i, v.getValue(i) - size.getValue(i));
			boundingBox.expand(v);
		}
		boundingBox.expand(origin + size);
		assert(boundingBox.isValid());
		break;
	case Rectangle:
		boundingBox.expand(origin);
		boundingBox.expand(origin + size);
		break;
	case Line:
		for (int i = 0; i < int(ordinates->size()); i = i + 3) {
			boundingBox.expand(Math::Vector3d((*ordinates)[i], (*ordinates)[i + 1], (*ordinates)[i + 2]));
		}
		int dx, dy, dz;
		dx = dy = dz = 0;
		if (size.x() == 0 && size.y() == 0) {
			dx = 2;
			dy = 2;
		} else if (size.x() == 0 && size.z() == 0) {
			dx = 2;
			dz = 2;
		} else if (size.y() == 0 && size.z() == 0) {
			dy = 2;
			dz = 2;
		}

		for (int i = 0; i < int(ordinates->size()); i = i + 3) {
			boundingBox.expand(Math::Vector3d((*ordinates)[i] + dx, (*ordinates)[i + 1] + dy, (*ordinates)[i + 2] + dz));
		}

		break;
	case Triangle:
	case Quadrilateral:
	case Pentagon:
	case Hexagon:
		for (int i = 0; i < int(ordinates->size()); i = i + 3) {
			boundingBox.expand(Math::Vector3d((*ordinates)[i], (*ordinates)[i + 1], (*ordinates)[i + 2]));
		}
		break;

	case EastPyramid:
		boundingBox.expand(origin + Math::Vector3d(0, 0, size.z()));
		boundingBox.expand(origin + Math::Vector3d(0, size.y(), size.z()));
		boundingBox.expand(origin + Math::Vector3d(0, size.y(), 0));

		boundingBox.expand(origin + Math::Vector3d(size.x(), (*ordinates)[0], (*ordinates)[3]));
		boundingBox.expand(origin + Math::Vector3d(size.x(), (*ordinates)[2], (*ordinates)[3]));
		boundingBox.expand(origin + Math::Vector3d(size.x(), (*ordinates)[2], (*ordinates)[1]));
		boundingBox.expand(origin + Math::Vector3d(size.x(), (*ordinates)[0], (*ordinates)[1]));
		break;
	case WestPyramid:
		boundingBox.expand(origin + Math::Vector3d(size.x(), 0, 0));
		boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), 0));
		boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), size.z()));
		boundingBox.expand(origin + Math::Vector3d(size.x(), 0, size.z()));

		boundingBox.expand(origin + Math::Vector3d(0, (*ordinates)[0], (*ordinates)[1]));
		boundingBox.expand(origin + Math::Vector3d(0, (*ordinates)[2], (*ordinates)[1]));
		boundingBox.expand(origin + Math::Vector3d(0, (*ordinates)[2], (*ordinates)[3]));
		boundingBox.expand(origin + Math::Vector3d(0, (*ordinates)[0], (*ordinates)[3]));
		break;
	case UpPyramid:
		boundingBox.expand(origin + Math::Vector3d(size.x(), 0, 0));
		boundingBox.expand(origin + Math::Vector3d(size.x(), 0, size.z()));
		boundingBox.expand(origin + Math::Vector3d(0, 0, size.z()));

		boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], size.y(), (*ordinates)[1]));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], size.y(), (*ordinates)[1]));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], size.y(), (*ordinates)[3]));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], size.y(), (*ordinates)[3]));
		break;
	case DownPyramid:
		boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), 0));
		boundingBox.expand(origin + Math::Vector3d(0, size.y(), 0));
		boundingBox.expand(origin + Math::Vector3d(0, size.y(), size.z()));
		boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), size.z()));

		boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], 0, (*ordinates)[1]));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], 0, (*ordinates)[1]));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], 0, (*ordinates)[3]));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], 0, (*ordinates)[3]));
		break;
	case NorthPyramid:
		boundingBox.expand(origin + Math::Vector3d(0, size.y(), 0));
		boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), 0));
		boundingBox.expand(origin + Math::Vector3d(size.x(), 0, 0));

		boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], (*ordinates)[3], size.z()));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], (*ordinates)[3], size.z()));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], (*ordinates)[1], size.z()));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], (*ordinates)[1], size.z()));
		break;
	case SouthPyramid:
		boundingBox.expand(origin + Math::Vector3d(0, 0, size.z()));
		boundingBox.expand(origin + Math::Vector3d(size.x(), 0, size.z()));
		boundingBox.expand(origin + Math::Vector3d(size.x(), size.y(), size.z()));

		boundingBox.expand(origin + Math::Vector3d(0, size.y(), size.z()));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], (*ordinates)[1], 0));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], (*ordinates)[1], 0));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[2], (*ordinates)[3], 0));
		boundingBox.expand(origin + Math::Vector3d((*ordinates)[0], (*ordinates)[3], 0));
		break;
	}
}

GeometricObject::~GeometricObject() {
}

bool GeometricObject::isDrawable() { return true; }
bool GeometricObject::isPlanar() {
	Type t = this->getType();
	return (t >= Object::Line) || t == Object::Rectangle || !size.x() || !size.y() || !size.z();
}

bool GeometricObject::collides(const Math::AABB &_boundingBox) {
	if (isDestroyed() || isInvisible() || !boundingBox.isValid() || !_boundingBox.isValid())
		return false;

	return(	boundingBox.getMax().x() > _boundingBox.getMin().x() &&
			boundingBox.getMin().x() < _boundingBox.getMax().x() &&
			boundingBox.getMax().y() > _boundingBox.getMin().y() &&
			boundingBox.getMin().y() < _boundingBox.getMax().y() &&
			boundingBox.getMax().z() > _boundingBox.getMin().z() &&
			boundingBox.getMin().z() < _boundingBox.getMax().z());
}

void GeometricObject::draw(Freescape::Renderer *gfx) {
	//debug("Drawing %d of type %d", this->getObjectID(), this->getType());
	if (this->getType() == Cube) {
		gfx->renderCube(origin, size, colours);
	} else if (this->getType() == Rectangle) {
		gfx->renderRectangle(origin, size, colours);
	} else if (isPyramid(this->getType())) {
		gfx->renderPyramid(origin, size, ordinates, colours, this->getType());
	} else if (this->isPlanar() && type <= 14) {
		if (this->getType() == Triangle)
			assert(ordinates->size() == 9);

		//debug("Drawing %d of type %d", this->getObjectID(), this->getType());
		gfx->renderPolygon(origin, size, ordinates, colours);
	}
}

} // End of namespace Freescape