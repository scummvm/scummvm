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
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

Group::Group(uint16 objectID_, uint16 flags_,
const Common::Array<uint16> objectIds_,
const Common::Array<AnimationOpcode *> operations_) {
	_objectID = objectID_;
	_flags = flags_;
	_scale = 0;
	_active = false;
	_finished = false;
	_step = 0;

	_objectIds = objectIds_;
	_operations = operations_;

	if (isDestroyed()) // If the object is destroyed, restore it
		restore();

	makeInitiallyVisible();
	makeVisible();
}

Group::~Group() {
	for (int i = 0; i < int(_operations.size()); i++)
		delete _operations[i];
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
	obj->_partOfGroup = this;
	_objects.push_back(obj);
}

void Group::assemble(int index) {
	GeometricObject *gobj = (GeometricObject *)_objects[index];
	Math::Vector3d position = _operations[_step]->position;

	if (!GeometricObject::isPolygon(gobj->getType()))
		position = 32 * position / _scale;
	else
		position = position / _scale;

	gobj->offsetOrigin(position);
}

void Group::run() {
	if (_finished)
		return;

	uint32 groupSize = _objects.size();
	for (uint32 i = 0; i < groupSize ; i++) {
		run(i);
	}
}

void Group::run(int index) {
	if (_operations[_step]->opcode == 0x80) {
		_step = -1;
		_active = false;
		_finished = false;
	} else if (_operations[_step]->opcode == 0x01) {
		g_freescape->executeCode(_operations[_step]->condition, false, true, false, false);
	} else {
		if (_operations[_step]->opcode == 0x10)
			if (!_active) {
				_step = -1;
				return;
			}
		assemble(index);
	}
}

void Group::draw(Renderer *gfx) {
	uint32 groupSize = _objects.size();
	for (uint32 i = 0; i < groupSize ; i++) {
		_objects[i]->draw(gfx);
	}
}

void Group::step() {
	if (_finished)
		return;

	if (_step < int(_operations.size() - 1))
		_step++;
	else {
		_finished = true;
	}
}

} // End of namespace Freescape