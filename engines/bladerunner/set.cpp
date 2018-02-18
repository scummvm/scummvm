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
#include "bladerunner/game_constants.h"
#include "bladerunner/lights.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/set_effects.h"
#include "bladerunner/slice_renderer.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/stream.h"

namespace BladeRunner {

#define kSet0 0x53657430

Set::Set(BladeRunnerEngine *vm) {
	_vm = vm;
	_objectCount = 0;
	_walkboxCount = 0;
	_objects = new Object[85];
	_walkboxes = new Walkbox[95];
	_footstepSoundOverride = -1;
	_effects = new SetEffects(vm);
	_loaded = false;
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

	int frameCount = s->readUint32LE();

	_objectCount = s->readUint32LE();
	assert(_objectCount <= 85);

	for (int i = 0; i < _objectCount; ++i) {
		s->read(_objects[i].name, 20);

		float x0, y0, z0, x1, y1, z1;
		x0 = s->readFloatLE();
		y0 = s->readFloatLE();
		z0 = s->readFloatLE();
		x1 = s->readFloatLE();
		y1 = s->readFloatLE();
		z1 = s->readFloatLE();

		_objects[i].bbox = BoundingBox(x0, y0, z0, x1, y1, z1);
		_objects[i].isObstacle = s->readByte();
		_objects[i].isClickable = s->readByte();
		_objects[i].isHotMouse = 0;
		_objects[i].isTarget = 0;
		s->skip(4);

		// debug("OBJECT: %s [%d%d%d%d]", _objects[i]._name, _objects[i]._isObstacle, _objects[i]._isClickable, _objects[i]._isHotMouse, _objects[i]._isTarget);
	}

	_walkboxCount = s->readUint32LE();
	assert(_walkboxCount <= 95);

	for (int i = 0; i < _walkboxCount; ++i) {
		float x, z;

		s->read(_walkboxes[i].name, 20);
		_walkboxes[i].altitude = s->readFloatLE();
		_walkboxes[i].vertexCount = s->readUint32LE();

		assert(_walkboxes[i].vertexCount <= 8);

		for (int j = 0; j < _walkboxes[i].vertexCount; ++j) {
			x = s->readFloatLE();
			z = s->readFloatLE();

			_walkboxes[i].vertices[j] = Vector3(x, _walkboxes[i].altitude, z);
		}

		// debug("WALKBOX: %s", _walkboxes[i]._name);
	}

	_vm->_lights->reset();
	_vm->_lights->read(s.get(), frameCount);
	_vm->_sliceRenderer->setLights(_vm->_lights);
	_effects->reset();
	_effects->read(s.get(), frameCount);
	_vm->_sliceRenderer->setSetEffects(_effects);

	// _vm->_sliceRenderer->set_setColors(&colors);
	_loaded = true;

	for (int i = 0; i < _walkboxCount; ++i) {
		setWalkboxStepSound(i, 0);
	}

	return true;
}

void Set::addObjectsToScene(SceneObjects *sceneObjects) const {
	for (int i = 0; i < _objectCount; i++) {
		sceneObjects->addObject(i + kSceneObjectOffsetObjects, &_objects[i].bbox, _objects[i].isClickable, _objects[i].isObstacle, _objects[i].unknown1, _objects[i].isTarget);
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

bool Set::isXZInWalkbox(float x, float z, const Walkbox &walkbox) {
	int found = 0;

	float lastX = walkbox.vertices[walkbox.vertexCount - 1].x;
	float lastZ = walkbox.vertices[walkbox.vertexCount - 1].z;
	for (int i = 0; i < walkbox.vertexCount; i++) {
		float currentX = walkbox.vertices[i].x;
		float currentZ = walkbox.vertices[i].z;

		if ((currentZ > z && z >= lastZ) || (currentZ <= z && z < lastZ)) {
			float lineX = (lastX - currentX) / (lastZ - currentZ) * (z - currentZ) + currentX;
			if (x < lineX)
				found++;
		}
		lastX = currentX;
		lastZ = currentZ;
	}
	return found & 1;
}

float Set::getAltitudeAtXZ(float x, float z, bool *inWalkbox) const {
	float altitude = _walkboxes[0].altitude;
	*inWalkbox = false;

	for (int i = 0; i < _walkboxCount; ++i) {
		const Walkbox &walkbox = _walkboxes[i];

		if (isXZInWalkbox(x, z, walkbox)) {
			if (!*inWalkbox || altitude < walkbox.altitude) {
				altitude = walkbox.altitude;
				*inWalkbox = true;
			}
		}
	}

	return altitude;
}

int Set::findWalkbox(float x, float z) const {
	int result = -1;

	for (int i = 0; i < _walkboxCount; ++i) {
		const Walkbox &w = _walkboxes[i];

		if (isXZInWalkbox(x, z, w)) {
			if (result == -1 || w.altitude > _walkboxes[result].altitude) {
				result = i;
			}
		}
	}

	return result;
}

int Set::findObject(const char *objectName) const {
	int i;
	for (i = 0; i < (int)_objectCount; i++) {
		if (scumm_stricmp(objectName, _objects[i].name) == 0) {
			return i;
		}
	}

	debug("Set::findObject didn't find \"%s\"", objectName);

	return -1;
}

bool Set::objectSetHotMouse(int objectId) const {
	if (!_objects || objectId < 0 || objectId >= (int)_objectCount) {
		return false;
	}

	_objects[objectId].isHotMouse = true;
	return true;
}

bool Set::objectGetBoundingBox(int objectId, BoundingBox *boundingBox) const {
	assert(boundingBox);

	if (!_objects || objectId < 0 || objectId >= (int)_objectCount) {
		boundingBox->setXYZ(0, 0, 0, 0, 0, 0);
		return false;
	}
	float x0, y0, z0, x1, y1, z1;

	_objects[objectId].bbox.getXYZ(&x0, &y0, &z0, &x1, &y1, &z1);
	boundingBox->setXYZ(x0, y0, z0, x1, y1, z1);

	return true;
}

void Set::objectSetIsClickable(int objectId, bool isClickable) {
	_objects[objectId].isClickable = isClickable;
}

void Set::objectSetIsObstacle(int objectId, bool isObstacle) {
	_objects[objectId].isObstacle = isObstacle;
}

void Set::objectSetIsTarget(int objectId, bool isTarget) {
	_objects[objectId].isTarget = isTarget;
}

const char *Set::objectGetName(int objectId) const {
	return _objects[objectId].name;
}

void Set::setWalkboxStepSound(int walkboxId, int stepSound) {
	_walkboxStepSound[walkboxId] = stepSound;
}

void Set::setFoodstepSoundOverride(int soundId) {
	_footstepSoundOverride = soundId;
}

void Set::resetFoodstepSoundOverride() {
	_footstepSoundOverride = -1;
}

int Set::getWalkboxSoundWalkLeft(int walkboxId) const{
	int soundId;
	if (_footstepSoundOverride >= 0) {
		soundId = _footstepSoundOverride;
	} else {
		soundId = _walkboxStepSound[walkboxId];
	}

	if (soundId == 0) { //stone floor
		return _vm->_rnd.getRandomNumberRng(160, 164);
	}
	if (soundId == 1) { //gravel floor
		return _vm->_rnd.getRandomNumberRng(164, 170);
	}
	if (soundId == 2) { //wooden floor
		return _vm->_rnd.getRandomNumberRng(476, 480);
	}
	if (soundId == 3) { //metal floor
		return _vm->_rnd.getRandomNumberRng(466, 470);
	}

	return -1;
}

int Set::getWalkboxSoundWalkRight(int walkboxId) const {
	int soundId;
	if (_footstepSoundOverride >= 0) {
		soundId = _footstepSoundOverride;
	} else {
		soundId = _walkboxStepSound[walkboxId];
	}

	if (soundId == 0) { //stone floor
		return _vm->_rnd.getRandomNumberRng(165, 169);
	}
	if (soundId == 1) { //gravel floor
		return _vm->_rnd.getRandomNumberRng(169, 175);
	}
	if (soundId == 2) { //wooden floor
		return _vm->_rnd.getRandomNumberRng(481, 485);
	}
	if (soundId == 3) { //metal floor
		return _vm->_rnd.getRandomNumberRng(471, 475);
	}

	return -1;
}

int Set::getWalkboxSoundRunLeft(int walkboxId) const {
	return getWalkboxSoundWalkLeft(walkboxId);
}

int Set::getWalkboxSoundRunRight(int walkboxId) const {
	return getWalkboxSoundWalkRight(walkboxId);
}
} // End of namespace BladeRunner
