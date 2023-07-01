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
#include "freescape/freescape.h"
#include "freescape/objects/group.h"
#include "freescape/objects/geometricobject.h"

namespace Freescape {

Group::Group(uint16 objectID_, uint16 flags_, const Common::Array<byte> data_) {
	_objectID = objectID_;
	_flags = flags_;
	_scale = 0;

	int i;
	for (i = 0; i < 5; i++) {
		debugC(1, kFreescapeDebugParser, "group data[%d] = %d", i, data_[i]);
		if (data_[i] > 0)
			_objectIds.push_back(data_[i]);
	}
	i = 5;
	while (i < int(data_.size() - 4)) {
		debugC(1, kFreescapeDebugParser, "group data[%d] = %d (index)	", i, data_[i]);
		_objectIndices.push_back(data_[i]);

		debugC(1, kFreescapeDebugParser, "group data[%d] = %d", i + 1, data_[i + 1]);
		debugC(1, kFreescapeDebugParser, "group data[%d] = %d", i + 2, data_[i + 2]);
		debugC(1, kFreescapeDebugParser, "group data[%d] = %d", i + 3, data_[i + 3]);
		Math::Vector3d position(data_[i + 1], data_[i + 2], data_[i + 3]);
		_objectPositions.push_back(position);

		i = i + 4;
	}

	if (isDestroyed()) // If the object is destroyed, restore it
		restore();

	makeInitiallyVisible();
	makeVisible();
}

void Group::linkObject(Object *obj) {
	int objectIndex = -1;
	for (int i = 0; i < int(_objectIds.size()) ; i++) {
		if (_objectIds[i] == obj->getObjectID()) {
			objectIndex = i;
			break;
		}
	}

	if (objectIndex == -1)
		return;

	_origins.push_back(obj->getOrigin());
	obj->makeInitiallyVisible();
	obj->makeVisible();
	_objects.push_back(obj);
}

void Group::assemble(int frame, int index) {
	GeometricObject *gobj = (GeometricObject *)_objects[index];
	Math::Vector3d position = _objectPositions[frame];

	if (!GeometricObject::isPolygon(gobj->getType()))
		position = 32 * position / _scale;
	else
		position = position / _scale;

	gobj->offsetOrigin(position);
}
} // End of namespace Freescape