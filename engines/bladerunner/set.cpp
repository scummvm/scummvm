/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/set.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/slice_renderer.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/stream.h"


namespace BladeRunner {

#define kSet0 0x53657430

Set::Set(BladeRunnerEngine *vm) : _vm(vm) {
	_objectCount = 0;
	_walkboxCount = 0;
	_objects = new Object[85];
	_walkboxes = new Walkbox[95];
	_footstepSoundOverride = -1;
	_effects = new SetEffects(vm);
}

Set::~Set() {
	delete _effects;
	delete[] _objects;
	delete[] _walkboxes;
}

bool Set::open(const Common::String &name) {
	Common::ScopedPtr<Common::SeekableReadStream> s(_vm->getResourceStream(name));

	uint32 sig = s->readUint32BE();
	if (sig != kSet0)
		return false;

	int framesCount = s->readUint32LE();

	_objectCount = s->readUint32LE();
	assert(_objectCount <= 85);

	for (uint32 i = 0; i != _objectCount; ++i) {
		s->read(_objects[i]._name, 20);

		float x0, y0, z0, x1, y1, z1;
		x0 = s->readFloatLE();
		y0 = s->readFloatLE();
		z0 = s->readFloatLE();
		x1 = s->readFloatLE();
		y1 = s->readFloatLE();
		z1 = s->readFloatLE();

		_objects[i]._bbox = BoundingBox(x0, y0, z0, x1, y1, z1);

		_objects[i]._isObstacle = s->readByte();
		_objects[i]._isClickable = s->readByte();
		_objects[i]._isHotMouse = 0;
		_objects[i]._isCombatTarget = 0;
		s->skip(4);

		// debug("OBJECT: %s", _objects[i]._name);
	}

	_walkboxCount = s->readUint32LE();
	assert(_walkboxCount <= 95);

	for (uint32 i = 0; i != _walkboxCount; ++i) {
		float x, y, z;

		s->read(_walkboxes[i]._name, 20);
		y = s->readFloatLE();
		_walkboxes[i]._vertexCount = s->readUint32LE();

		assert(_walkboxes[i]._vertexCount <= 8);

		for (uint32 j = 0; j != _walkboxes[i]._vertexCount; ++j) {
			x = s->readFloatLE();
			z = s->readFloatLE();

			_walkboxes[i]._vertices[j] = Vector3(x, y, z);
		}

		// debug("WALKBOX: %s", _walkboxes[i]._name);
	}

	_vm->_lights->reset();
	_vm->_lights->read(s.get(), framesCount);
	_vm->_sliceRenderer->setLights(_vm->_lights);
	_effects->reset();
	_effects->read(s.get(), framesCount);
	_vm->_sliceRenderer->setSetEffects(_effects);

	return true;
}


void Set::addObjectsToScene(SceneObjects* sceneObjects)
{
	uint32 i;
	for (i = 0; i < _objectCount; i++) {
		sceneObjects->addObject(i + SCENE_OBJECTS_OBJECTS_OFFSET, &_objects[i]._bbox, _objects[i]._isClickable, _objects[i]._isObstacle, _objects[i]._unknown1, _objects[i]._isCombatTarget);
	}
}


int Set::findWalkbox(float x, float z) {
	int i;
	float altitude = 0.0f;
	int foundWalkboxId = -1;
	for (i = 0; i < (int)_walkboxCount; i++) {
		if (isXzInWalkbox(x, z, &_walkboxes[i])) {
			if (foundWalkboxId == -1 || altitude < _walkboxes[i]._altitude) {
				altitude = _walkboxes[i]._altitude;
				foundWalkboxId = i;
			}
		}
	}
	return foundWalkboxId;
}

bool Set::isXzInWalkbox(float x, float z, Walkbox* walkbox) {
	int found = 0;
	int i;

	float lastX = walkbox->_vertices[walkbox->_vertexCount - 1].x;
	float lastZ = walkbox->_vertices[walkbox->_vertexCount - 1].z;
	for (i = 0; i < (int)walkbox->_vertexCount; i++) {

		float currentX = walkbox->_vertices[i].x;
		float currentZ = walkbox->_vertices[i].z;

		if ((currentZ > z && z >= lastZ) || (currentZ <= z && z < lastZ)) {
			float lineX = (lastX - currentX) / (lastZ - currentZ) * (z - currentZ) + currentX;
			if (x < lineX)
				found++;
		}

	}
	return found & 1;
}


int Set::findObject(char* objectName) {
	int i;	
	for (i = 0; i < (int)_objectCount; i++) {
		if (scumm_stricmp(objectName, _objects[i]._name) == 0) {
			return i;
		}
	}
	return -1;
}

bool Set::objectSetHotMouse(int objectId) {
	if(!_objects || objectId < 0 || objectId >= (int)_objectCount) {
		return false;
	}
	
	_objects[objectId]._isHotMouse = true;
	return true;
}

bool Set::objectGetBoundingBox(int objectId, BoundingBox* boundingBox) {
	assert(boundingBox);

	if (!_objects || objectId < 0 || objectId >= (int)_objectCount) {
		boundingBox->setXyz(0, 0, 0, 0, 0, 0);
		return false;
	}
	float x0, y0, z0, x1, y1, z1;

	_objects[objectId]._bbox.getXyz(&x0, &y0, &z0, &x1, &y1, &z1);
	boundingBox->setXyz(x0, y0, z0, x1, y1, z1);

	return true;
}

void Set::objectSetIsClickable(int objectId, bool isClickable) {
	_objects[objectId]._isClickable = isClickable;
}

void Set::objectSetIsObstacle(int objectId, bool isObstacle) {
	_objects[objectId]._isObstacle = isObstacle;
}

void Set::objectSetIsCombatTarget(int objectId, bool isCombatTarget) {
	_objects[objectId]._isCombatTarget = isCombatTarget;
}

} // End of namespace BladeRunner
