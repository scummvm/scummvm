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
#include "bladerunner/scene_objects.h"

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
}

Set::~Set() {
	delete[] _objects;
	delete[] _walkboxes;
}

bool Set::open(const Common::String &name) {
	Common::ScopedPtr<Common::SeekableReadStream> s(_vm->getResourceStream(name));

	uint32 sig = s->readUint32BE();
	if (sig != kSet0)
		return false;

	s->skip(4); // TODO: LITE length

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
		_objects[i]._isTarget = 0;
		s->skip(4);

		// debug("OBJECT: %s [%d%d%d%d]", _objects[i]._name, _objects[i]._isObstacle, _objects[i]._isClickable, _objects[i]._isHotMouse, _objects[i]._isTarget);
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

	// TODO: Read LITE

	return true;
}

void Set::addObjectsToScene(SceneObjects* sceneObjects)
{
	uint32 i;
	for (i = 0; i < _objectCount; i++) {
		sceneObjects->addObject(i + SCENE_OBJECTS_OBJECTS_OFFSET, &_objects[i]._bbox, _objects[i]._isClickable, _objects[i]._isObstacle, _objects[i]._unknown1, _objects[i]._isTarget);
	}
}

// Source: http://www.faqs.org/faqs/graphics/algorithms-faq/ section 2.03
/*
static
bool pointInWalkbox(float x, float z, const Walkbox &w)
{
	uint32 i, j;
	bool c = false;

	for (i = 0, j = w._vertexCount - 1; i < w._vertexCount; j = i++) {
		if ((((w._vertices[i].z <= z) && (z < w._vertices[j].z)) ||
		     ((w._vertices[j].z <= z) && (z < w._vertices[i].z))) &&
		    (x < (w._vertices[j].x - w._vertices[i].x) * (z - w._vertices[i].z) / (w._vertices[j].z - w._vertices[i].z) + w._vertices[i].x))
		{
			c = !c;
		}
	}
	return c;
}
*/

static
bool isXZInWalkbox(float x, float z, const Walkbox &walkbox) {
	int found = 0;
	int i;

	float lastX = walkbox._vertices[walkbox._vertexCount - 1].x;
	float lastZ = walkbox._vertices[walkbox._vertexCount - 1].z;
	for (i = 0; i < (int)walkbox._vertexCount; i++) {
		float currentX = walkbox._vertices[i].x;
		float currentZ = walkbox._vertices[i].z;

		if ((currentZ > z && z >= lastZ) || (currentZ <= z && z < lastZ)) {
			float lineX = (lastX - currentX) / (lastZ - currentZ) * (z - currentZ) + currentX;
			if (x < lineX)
				found++;
		}

	}
	return found & 1;
}

float Set::getAltitudeAtXZ(float x, float z, bool *inWalkbox) {
	float altitude = _walkboxes[0]._altitude;
	*inWalkbox = false;

	for (uint32 i = 0; i != _walkboxCount; ++i) {
		const Walkbox &w = _walkboxes[i];

		if (isXZInWalkbox(x, z, w)) {
			*inWalkbox = true;
			if (w._altitude > altitude) {
				altitude = w._altitude;
			}
		}
	}

	return altitude;
}

int Set::findWalkbox(float x, float z) {
	int result = -1;

	for (uint32 i = 0; i != _walkboxCount; ++i) {
		const Walkbox &w = _walkboxes[i];

		if (isXZInWalkbox(x, z, w)) {
			if (result == -1 || w._altitude > _walkboxes[result]._altitude) {
				result = i;
			}
		}
	}

	return result;
}

int Set::findObject(const char *objectName) {
	int i;
	for (i = 0; i < (int)_objectCount; i++) {
		if (scumm_stricmp(objectName, _objects[i]._name) == 0) {
			return i;
		}
	}

	debug("Set::findObject didn't find \"%s\"", objectName);

	return -1;
}

bool Set::objectSetHotMouse(int objectId) {
	if(!_objects || objectId < 0 || objectId >= (int)_objectCount) {
		return false;
	}

	_objects[objectId]._isHotMouse = true;
	return true;
}

bool Set::objectGetBoundingBox(int objectId, BoundingBox *boundingBox) {
	assert(boundingBox);

	if (!_objects || objectId < 0 || objectId >= (int)_objectCount) {
		boundingBox->setXYZ(0, 0, 0, 0, 0, 0);
		return false;
	}
	float x0, y0, z0, x1, y1, z1;

	_objects[objectId]._bbox.getXYZ(&x0, &y0, &z0, &x1, &y1, &z1);
	boundingBox->setXYZ(x0, y0, z0, x1, y1, z1);

	return true;
}

void Set::objectSetIsClickable(int objectId, bool isClickable) {
	_objects[objectId]._isClickable = isClickable;
}

void Set::objectSetIsObstacle(int objectId, bool isObstacle) {
	_objects[objectId]._isObstacle = isObstacle;
}

void Set::objectSetIsTarget(int objectId, bool isTarget) {
	_objects[objectId]._isTarget = isTarget;
}

const char *Set::objectGetName(int objectId) {
	return _objects[objectId]._name;
}

} // End of namespace BladeRunner
