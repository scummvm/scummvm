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
const Math::Vector3d offset1_,
const Math::Vector3d offset2_,
const Common::Array<AnimationOpcode *> operations_) {
	_objectID = objectID_;
	_flags = flags_;
	_scale = 0;
	_active = true;
	_step = 0;
	_offset1 = offset1_;
	_offset2 = offset2_;

	for (int i = 0; i < 3; i++) { // three is the maximum number of objects in a group
		if (objectIds_[i] == 0 || objectIds_[i] == 0xffff)
			break;
		_objectIds.push_back(objectIds_[i]);
	}

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

Object *Group::duplicate() {
	return new Group(
		_objectID,
		_flags,
		_objectIds,
		_offset1,
		_offset2,
		_operations
		);
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

	debugC(1, kFreescapeDebugParser, "Linking object: %d to group %d", obj->getObjectID(), this->getObjectID());
	_origins.push_back(obj->getOrigin());
	debugC(1, kFreescapeDebugParser, "Origin %f, %f %f", obj->getOrigin().x(), obj->getOrigin().y(), obj->getOrigin().z());

	obj->_partOfGroup = this;
	_objects.push_back(obj);
}

void Group::assemble(int index) {
	GeometricObject *gobj = (GeometricObject *)_objects[index];
	//gobj->makeVisible();
	Math::Vector3d position = _operations[_step]->position;
	Math::Vector3d offset = _origins[index] - _origins[0];
	/*if (index == 0)
		; // offset is always zero
	else if (index == 1)
		offset = _offset1;
	else if (index == 2)
		offset = _offset2;
	else
		error("Invalid index: %d", index);

	offset = 32 * offset / _scale;*/
	position = 32 * position / _scale;

	debugC(1, kFreescapeDebugCode, "Group %d: Assembling object %d originally at %f, %f, %f", _objectID, gobj->getObjectID(), gobj->getOrigin().x(), gobj->getOrigin().y(), gobj->getOrigin().z());
	gobj->offsetOrigin(position + offset);
	debugC(1, kFreescapeDebugCode, "Group %d: Assembling object %d originally at %f, %f, %f", _objectID, gobj->getObjectID(), gobj->getOrigin().x(), gobj->getOrigin().y(), gobj->getOrigin().z());
}

void Group::run() {
	if (!_active)
		return;

	int opcode = _operations[_step]->opcode;
	debugC(1, kFreescapeDebugCode, "Executing opcode 0x%x at step %d", opcode, _step);
	if (opcode == 0x80 || opcode == 0xff) {
		debugC(1, kFreescapeDebugCode, "Executing group rewind");
		_active = true;
		_step = -1;
		//reset();
	} else if (opcode == 0x01) {
		debugC(1, kFreescapeDebugCode, "Executing group condition %s", _operations[_step]->conditionSource.c_str());
		g_freescape->executeCode(_operations[_step]->condition, false, true, false, false);
	} else if (opcode == 0x10) {
		uint32 groupSize = _objects.size();
		for (uint32 i = 0; i < groupSize ; i++)
			assemble(i);
		_active = false;
		_step++;
	} else if (opcode == 0x0) {
		debugC(1, kFreescapeDebugCode, "Executing group assemble");
		uint32 groupSize = _objects.size();
		for (uint32 i = 0; i < groupSize ; i++)
			assemble(i);
	} else {
		uint32 groupSize = _objects.size();
		if (opcode & 0x08) {
			for (uint32 i = 0; i < groupSize ; i++)
				_objects[i]->makeVisible();

			if (opcode & 0x20) {
				for (uint32 i = 0; i < groupSize ; i++)
					_objects[i]->makeInvisible();

			}

			if (opcode & 0x40) {
				for (uint32 i = 0; i < groupSize ; i++)
					_objects[i]->destroy();
			}
		}

	}
}

void Group::reset() {
	/*uint32 groupSize = _objects.size();
	for (uint32 i = 0; i < groupSize ; i++) {
		GeometricObject *gobj = (GeometricObject *)_objects[i];
		if (GeometricObject::isPolygon(_objects[i]->getType())) {
			gobj->setOrigin(_origins[i]);
			gobj->restoreOrdinates();
			//gobj->makeInvisible();
		}
	}*/
}

void Group::draw(Renderer *gfx, float offset) {
	if (!_active)
		return;

	uint32 groupSize = _objects.size();
	for (uint32 i = 0; i < groupSize ; i++) {
		if (!_objects[i]->isDestroyed() && !_objects[i]->isInvisible())
			_objects[i]->draw(gfx);
	}
}

void Group::step() {
	if (!_active)
		return;

	debugC(1, kFreescapeDebugCode, "Stepping group %d", _objectID);
	if (_step < int(_operations.size() - 1))
		_step++;
	else {
		_active = false;
		_step = -1;
	}
}

} // End of namespace Freescape