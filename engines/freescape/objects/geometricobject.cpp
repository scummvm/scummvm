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

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#include "common/system.h"

#include "freescape/objects/geometricobject.h"

namespace Freescape {

extern FCLInstructionVector *duplicateCondition(FCLInstructionVector *condition);

int GeometricObject::numberOfColoursForObjectOfType(ObjectType type) {
	switch (type) {
	default:
	case kEntranceType:
	case kGroupType:
	case kSensorType:
		return 0;

	case kLineType:
		return 2;

	case kRectangleType:
	case kTriangleType:
	case kQuadrilateralType:
	case kPentagonType:
	case kHexagonType:
		return 2;

	case kCubeType:
	case kEastPyramidType:
	case kWestPyramidType:
	case kUpPyramidType:
	case kDownPyramidType:
	case kNorthPyramidType:
	case kSouthPyramidType:
		return 6;
	}
}

int GeometricObject::numberOfOrdinatesForType(ObjectType type) {
	switch (type) {
	default:
	case kEntranceType:
	case kGroupType:
	case kRectangleType:
	case kSensorType:
		return 0;

	case kEastPyramidType:
	case kWestPyramidType:
	case kUpPyramidType:
	case kDownPyramidType:
	case kNorthPyramidType:
	case kSouthPyramidType:
		return 4;

	case kLineType:
	case kTriangleType:
	case kQuadrilateralType:
	case kPentagonType:
	case kHexagonType:
		return 3 * (2 + type - kLineType);
	}
}

bool GeometricObject::isPyramid(ObjectType type) {
	switch (type) {
	default:
		return false;

	case kEastPyramidType:
	case kWestPyramidType:
	case kUpPyramidType:
	case kDownPyramidType:
	case kNorthPyramidType:
	case kSouthPyramidType:
		return true;
	}
}

bool GeometricObject::isPolygon(ObjectType type) {
	switch (type) {
	default:
		return false;

	case kLineType:
	case kTriangleType:
	case kQuadrilateralType:
	case kPentagonType:
	case kHexagonType:
		return true;
	}
}

GeometricObject::GeometricObject(
	ObjectType type_,
	uint16 objectID_,
	uint16 flags_,
	const Math::Vector3d &origin_,
	const Math::Vector3d &size_,
	Common::Array<uint8> *colours_,
	Common::Array<uint8> *ecolours_,
	Common::Array<float> *ordinates_,
	FCLInstructionVector conditionInstructions_,
	Common::String conditionSource_) {
	_type = type_;
	assert(_type != kGroupType);
	_flags = flags_;

	if (isDestroyed()) // If the object is destroyed, restore it
		restore();

	if (isInitiallyInvisible())
		makeInvisible();
	else
		makeVisible();

	_objectID = objectID_;
	_origin = origin_;
	_size = size_;

	_colours = nullptr;

	if (colours_)
		_colours = colours_;

	_ecolours = nullptr;

	if (ecolours_)
		_ecolours = ecolours_;

	_cyclingColors = false; // This needs to be set manually
	_ordinates = nullptr;
	_initialOrdinates = nullptr;

	if (ordinates_) {
		_ordinates = ordinates_;
		_initialOrdinates = new Common::Array<float>(*_ordinates);
	}
	_condition = conditionInstructions_;
	_conditionSource = conditionSource_;

	if (_type == kRectangleType) {
		if ((_size.x() == 0 && _size.y() == 0) ||
			(_size.y() == 0 && _size.z() == 0) ||
			(_size.x() == 0 && _size.z() == 0)) {

			_type = kLineType;
			assert(!_ordinates);
			_ordinates = new Common::Array<float>();
			_ordinates->push_back(_origin.x());
			_ordinates->push_back(_origin.y());
			_ordinates->push_back(_origin.z());

			_ordinates->push_back(_origin.x() + _size.x());
			_ordinates->push_back(_origin.y() + _size.y());
			_ordinates->push_back(_origin.z() + _size.z());
		}
	} else if (isPyramid(_type))
		assert(_size.x() > 0 && _size.y() > 0 && _size.z() > 0);

	computeBoundingBox();
}

void GeometricObject::setOrigin(Math::Vector3d origin_) {
	_origin = origin_;
	computeBoundingBox();
}

void GeometricObject::offsetOrigin(Math::Vector3d origin_) {
	if (isPolygon(_type)) {
		Math::Vector3d offset = origin_ - _origin;
		for (int i = 0; i < int(_ordinates->size()); i = i + 3) {
			float ordinate = 0;
			ordinate = (*_ordinates)[i];
			ordinate +=  offset.x();
			assert(ordinate >= 0);
			(*_ordinates)[i] = ordinate;

			ordinate = (*_ordinates)[i + 1];
			ordinate +=  offset.y();
			assert(ordinate >= 0);
			(*_ordinates)[i + 1] = ordinate;

			ordinate = (*_ordinates)[i + 2];
			ordinate +=  offset.z();
			assert(ordinate >= 0);
			(*_ordinates)[i + 2] = ordinate;
		}
	}
	setOrigin(origin_);
}

void GeometricObject::scale(int factor) {
	_origin = _origin / factor;
	_size = _size / factor;
	if (_ordinates) {
		for (uint i = 0; i < _ordinates->size(); i++) {
			(*_ordinates)[i] = (*_ordinates)[i] / factor;
			if (_initialOrdinates)
				(*_initialOrdinates)[i] = (*_initialOrdinates)[i] / factor;
		}
	}
	computeBoundingBox();
}

void GeometricObject::restoreOrdinates() {
	if (!isPolygon(_type))
		return;

	for (uint i = 0; i < _ordinates->size(); i++)
		(*_ordinates)[i] = (*_initialOrdinates)[i];

	computeBoundingBox();
}

Object *GeometricObject::duplicate() {
	Common::Array<uint8> *coloursCopy = nullptr;
	Common::Array<uint8> *ecoloursCopy = nullptr;
	Common::Array<float> *ordinatesCopy = nullptr;
	FCLInstructionVector *conditionCopy = nullptr;

	if (_colours)
		coloursCopy = new Common::Array<uint8>(*_colours);

	if (_ecolours)
		ecoloursCopy = new Common::Array<uint8>(*_ecolours);

	if (_ordinates)
		ordinatesCopy = new Common::Array<float>(*_ordinates);

	conditionCopy = duplicateCondition(&_condition);
	assert(conditionCopy);

	GeometricObject *copy = new GeometricObject(
		_type,
		_objectID,
		_flags,
		_origin,
		_size,
		coloursCopy,
		ecoloursCopy,
		ordinatesCopy,
		*conditionCopy,
		_conditionSource
	);

	copy->_cyclingColors = _cyclingColors;
	return copy;
}

void GeometricObject::computeBoundingBox() {
	_boundingBox = Math::AABB();
	Math::Vector3d v;
	switch (_type) {
	default:
		break;
	case kCubeType:
		_boundingBox.expand(_origin);
		for (int i = 0; i < 3; i++) {
			v = _origin;
			v.setValue(i, v.getValue(i) + _size.getValue(i));
			_boundingBox.expand(v);
		}

		for (int i = 0; i < 3; i++) {
			v = _origin + _size;
			v.setValue(i, v.getValue(i) - _size.getValue(i));
			_boundingBox.expand(v);
		}
		_boundingBox.expand(_origin + _size);
		assert(_boundingBox.isValid());
		break;
	case kRectangleType:
		_boundingBox.expand(_origin);
		_boundingBox.expand(_origin + _size);
		break;
	case kLineType:
		if (!_ordinates)
			error("Ordinates needed to compute bounding box!");
		for (uint i = 0; i < _ordinates->size(); i = i + 3) {
			_boundingBox.expand(Math::Vector3d((*_ordinates)[i], (*_ordinates)[i + 1], (*_ordinates)[i + 2]));
		}
		int dx, dy, dz;
		dx = dy = dz = 0;
		if (_size.x() == 0 && _size.y() == 0) {
			dx = 2;
			dy = 2;
		} else if (_size.x() == 0 && _size.z() == 0) {
			dx = 2;
			dz = 2;
		} else if (_size.y() == 0 && _size.z() == 0) {
			dy = 2;
			dz = 2;
		}

		for (uint i = 0; i < _ordinates->size(); i = i + 3) {
			_boundingBox.expand(Math::Vector3d((*_ordinates)[i] + dx, (*_ordinates)[i + 1] + dy, (*_ordinates)[i + 2] + dz));
		}

		break;
	case kTriangleType:
	case kQuadrilateralType:
	case kPentagonType:
	case kHexagonType:
		if (!_ordinates)
			error("Ordinates needed to compute bounding box!");
		for (uint i = 0; i < _ordinates->size(); i = i + 3) {
			_boundingBox.expand(Math::Vector3d((*_ordinates)[i], (*_ordinates)[i + 1], (*_ordinates)[i + 2]));
		}
		break;

	case kEastPyramidType:
		if (!_ordinates)
			error("Ordinates needed to compute bounding box!");
		_boundingBox.expand(_origin + Math::Vector3d(0, 0, _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d(0, _size.y(), _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d(0, _size.y(), 0));

		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), (*_ordinates)[0], (*_ordinates)[3]));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), (*_ordinates)[2], (*_ordinates)[3]));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), (*_ordinates)[2], (*_ordinates)[1]));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), (*_ordinates)[0], (*_ordinates)[1]));
		break;
	case kWestPyramidType:
		if (!_ordinates)
			error("Ordinates needed to compute bounding box!");
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), 0, 0));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), _size.y(), 0));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), _size.y(), _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), 0, _size.z()));

		_boundingBox.expand(_origin + Math::Vector3d(0, (*_ordinates)[0], (*_ordinates)[1]));
		_boundingBox.expand(_origin + Math::Vector3d(0, (*_ordinates)[2], (*_ordinates)[1]));
		_boundingBox.expand(_origin + Math::Vector3d(0, (*_ordinates)[2], (*_ordinates)[3]));
		_boundingBox.expand(_origin + Math::Vector3d(0, (*_ordinates)[0], (*_ordinates)[3]));
		break;
	case kUpPyramidType:
		if (!_ordinates)
			error("Ordinates needed to compute bounding box!");
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), 0, 0));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), 0, _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d(0, 0, _size.z()));

		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[0], _size.y(), (*_ordinates)[1]));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[2], _size.y(), (*_ordinates)[1]));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[2], _size.y(), (*_ordinates)[3]));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[0], _size.y(), (*_ordinates)[3]));
		break;
	case kDownPyramidType:
		if (!_ordinates)
			error("Ordinates needed to compute bounding box!");
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), _size.y(), 0));
		_boundingBox.expand(_origin + Math::Vector3d(0, _size.y(), 0));
		_boundingBox.expand(_origin + Math::Vector3d(0, _size.y(), _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), _size.y(), _size.z()));

		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[2], 0, (*_ordinates)[1]));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[0], 0, (*_ordinates)[1]));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[0], 0, (*_ordinates)[3]));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[2], 0, (*_ordinates)[3]));
		break;
	case kNorthPyramidType:
		if (!_ordinates)
			error("Ordinates needed to compute bounding box!");
		_boundingBox.expand(_origin + Math::Vector3d(0, _size.y(), 0));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), _size.y(), 0));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), 0, 0));

		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[0], (*_ordinates)[3], _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[2], (*_ordinates)[3], _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[2], (*_ordinates)[1], _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[0], (*_ordinates)[1], _size.z()));
		break;
	case kSouthPyramidType:
		if (!_ordinates)
			error("Ordinates needed to compute bounding box!");
		_boundingBox.expand(_origin + Math::Vector3d(0, 0, _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), 0, _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d(_size.x(), _size.y(), _size.z()));

		_boundingBox.expand(_origin + Math::Vector3d(0, _size.y(), _size.z()));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[0], (*_ordinates)[1], 0));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[2], (*_ordinates)[1], 0));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[2], (*_ordinates)[3], 0));
		_boundingBox.expand(_origin + Math::Vector3d((*_ordinates)[0], (*_ordinates)[3], 0));
		break;
	}
}

GeometricObject::~GeometricObject() {
	delete _colours;
	delete _ordinates;
	delete _initialOrdinates;
}

// This function returns when the object is a line, but it is not a straight line
bool GeometricObject::isLineButNotStraight() {
	if (_type != kLineType)
		return false;

	if (!_ordinates)
		return false;

	if (_ordinates->size() != 6)
		return false;

	// At least two coordinates should be the same to be a straight line
	if ((*_ordinates)[0] == (*_ordinates)[3] && (*_ordinates)[1] == (*_ordinates)[4])
		return false;

	if ((*_ordinates)[0] == (*_ordinates)[3] && (*_ordinates)[2] == (*_ordinates)[5])
		return false;

	if ((*_ordinates)[1] == (*_ordinates)[4] && (*_ordinates)[2] == (*_ordinates)[5])
		return false;

	return true;
}

bool GeometricObject::isDrawable() { return true; }
bool GeometricObject::isPlanar() {
	ObjectType t = this->getType();
	return (t >= kLineType) || t == kRectangleType || !_size.x() || !_size.y() || !_size.z();
}

bool GeometricObject::collides(const Math::AABB &boundingBox_) {
	if (isDestroyed() || isInvisible() || !_boundingBox.isValid() || !boundingBox_.isValid())
		return false;

	return _boundingBox.collides(boundingBox_);
}

void GeometricObject::draw(Renderer *gfx, float offset) {
	if (_cyclingColors) {
		assert(_colours);
		if (g_system->getMillis() % 10 == 0)
			for (uint i = 0; i < _colours->size(); i++) {
				(*_colours)[i] = ((*_colours)[i] + 1) % 0xf;
				if (_ecolours)
					(*_ecolours)[i] = ((*_ecolours)[i] + 1) % 0xf;
			}
	}

	if (this->getType() == kCubeType) {
		gfx->renderCube(_origin, _size, _colours, _ecolours, offset);
	} else if (this->getType() == kRectangleType) {
		gfx->renderRectangle(_origin, _size, _colours, _ecolours, offset);
	} else if (isPyramid(this->getType())) {
		gfx->renderPyramid(_origin, _size, _ordinates, _colours, _ecolours, this->getType());
	} else if (this->isPlanar() && _type <= 14) {
		if (this->getType() == kTriangleType)
			assert(_ordinates->size() == 9);

		gfx->renderPolygon(_origin, _size, _ordinates, _colours, _ecolours, offset);
	}
}

} // End of namespace Freescape
