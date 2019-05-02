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
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
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

	char buf[20];
	for (int i = 0; i < _objectCount; ++i) {
		s->read(buf, sizeof(buf));
		_objects[i].name = buf;

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
		_objects[i].unknown1 = 0;
		_objects[i].isTarget = 0;
		s->skip(4);
	}

	_walkboxCount = s->readUint32LE();
	assert(_walkboxCount <= 95);

	for (int i = 0; i < _walkboxCount; ++i) {
		s->read(buf, sizeof(buf));
		_walkboxes[i].name = buf;

		_walkboxes[i].altitude = s->readFloatLE();
		_walkboxes[i].vertexCount = s->readUint32LE();

		assert(_walkboxes[i].vertexCount <= 8);

		for (int j = 0; j < _walkboxes[i].vertexCount; ++j) {
			float x = s->readFloatLE();
			float z = s->readFloatLE();

			_walkboxes[i].vertices[j] = Vector3(x, _walkboxes[i].altitude, z);
		}
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
		overrideSceneObjectInfo(i); // For bugfixes with respect to clickable/targetable box positioning/bounding box
		sceneObjects->addObject(i + kSceneObjectOffsetObjects, _objects[i].bbox, _objects[i].isClickable, _objects[i].isObstacle, _objects[i].unknown1, _objects[i].isTarget);
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

int Set::findObject(const Common::String &objectName) const {
	for (int i = 0; i < _objectCount; ++i) {
		if (objectName.compareToIgnoreCase(_objects[i].name) == 0) {
			return i;
		}
	}

	warning("Set::findObject didn't find \"%s\"", objectName.c_str());

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

const Common::String &Set::objectGetName(int objectId) const {
	return _objects[objectId].name;
}

void Set::setWalkboxStepSound(int walkboxId, int floorType) {
	_walkboxStepSound[walkboxId] = floorType;
}

void Set::setFoodstepSoundOverride(int floorType) {
	_footstepSoundOverride = floorType;
}

void Set::resetFoodstepSoundOverride() {
	_footstepSoundOverride = -1;
}

int Set::getWalkboxSoundWalkLeft(int walkboxId) const{
	int floorType;
	if (_footstepSoundOverride >= 0) {
		floorType = _footstepSoundOverride;
	} else {
		floorType = _walkboxStepSound[walkboxId];
	}

	if (floorType == 0) { //stone floor
		// one of kSfxCEMENTL1, kSfxCEMENTL2, kSfxCEMENTL3, kSfxCEMENTL4, kSfxCEMENTL5
		return _vm->_rnd.getRandomNumberRng(kSfxCEMENTL1, kSfxCEMENTL5);
	}
	if (floorType == 1) { //gravel floor
#if BLADERUNNER_ORIGINAL_BUGS
		// A bug?
		// one of kSfxCEMENTL5, kSfxCEMENTR1, kSfxCEMENTR2, kSfxCEMENTR3, kSfxCEMENTR4, kSfxCEMENTR5, kSfxCEMWETL1
		return _vm->_rnd.getRandomNumberRng(kSfxCEMENTL5, kSfxCEMWETL1);
#else
		// one of kSfxCEMWETL1, kSfxCEMWETL2, kSfxCEMWETL3, kSfxCEMWETL4, kSfxCEMWETL5
		return _vm->_rnd.getRandomNumberRng(kSfxCEMWETL1, kSfxCEMWETL5);
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	if (floorType == 2) { //wooden floor
		// one of kSfxWOODL1, kSfxWOODL2, kSfxWOODL3, kSfxWOODL4, kSfxWOODL5
		return _vm->_rnd.getRandomNumberRng(kSfxWOODL1, kSfxWOODL5);
	}
	if (floorType == 3) { //metal floor
		// one of kSfxMETALL1, kSfxMETALL2, kSfxMETALL3, kSfxMETALL4, kSfxMETALL5
		return _vm->_rnd.getRandomNumberRng(kSfxMETALL1, kSfxMETALL5);
	}

	return -1;
}

int Set::getWalkboxSoundWalkRight(int walkboxId) const {
	int floorType;
	if (_footstepSoundOverride >= 0) {
		floorType = _footstepSoundOverride;
	} else {
		floorType = _walkboxStepSound[walkboxId];
	}

	if (floorType == 0) { //stone floor
		// one of kSfxCEMENTR1, kSfxCEMENTR2, kSfxCEMENTR3, kSfxCEMENTR4, kSfxCEMENTR5
		return _vm->_rnd.getRandomNumberRng(kSfxCEMENTR1, kSfxCEMENTR5);
	}
	if (floorType == 1) { //gravel floor
#if BLADERUNNER_ORIGINAL_BUGS
		// A bug?
		// one of kSfxCEMENTR5, kSfxCEMWETL1, kSfxCEMWETL2, kSfxCEMWETL3, kSfxCEMWETL4, kSfxCEMWETL5, kSfxCEMWETR1
		return _vm->_rnd.getRandomNumberRng(kSfxCEMENTR5, kSfxCEMWETR1);
#else
		// one of kSfxCEMWETR1, kSfxCEMWETR2, kSfxCEMWETR3, kSfxCEMWETR4, kSfxCEMWETR5
		return _vm->_rnd.getRandomNumberRng(kSfxCEMWETR1, kSfxCEMWETR5);
#endif // BLADERUNNER_ORIGINAL_BUGS

	}
	if (floorType == 2) { //wooden floor
		// one of kSfxWOODR1, kSfxWOODR2, kSfxWOODR3, kSfxWOODR4, kSfxWOODR5
		return _vm->_rnd.getRandomNumberRng(kSfxWOODR1, kSfxWOODR5);
	}
	if (floorType == 3) { //metal floor
		// one of kSfxMETALR1, kSfxMETALR2, kSfxMETALR3, kSfxMETALR4, kSfxMETALR5
		return _vm->_rnd.getRandomNumberRng(kSfxMETALR1, kSfxMETALR5);
	}

	return -1;
}

int Set::getWalkboxSoundRunLeft(int walkboxId) const {
	return getWalkboxSoundWalkLeft(walkboxId);
}

int Set::getWalkboxSoundRunRight(int walkboxId) const {
	return getWalkboxSoundWalkRight(walkboxId);
}

void Set::save(SaveFileWriteStream &f) {
	f.writeBool(_loaded);
	f.writeInt(_objectCount);
	f.writeInt(_walkboxCount);

	for (int i = 0; i != _objectCount; ++i) {
		f.writeStringSz(_objects[i].name, 20);
		f.writeBoundingBox(_objects[i].bbox, true);
		f.writeBool(_objects[i].isObstacle);
		f.writeBool(_objects[i].isClickable);
		f.writeBool(_objects[i].isHotMouse);
		f.writeInt(_objects[i].unknown1);
		f.writeBool(_objects[i].isTarget);
	}

	for (int i = 0; i != _walkboxCount; ++i) {
		f.writeStringSz(_walkboxes[i].name, 20);
		f.writeFloat(_walkboxes[i].altitude);
		f.writeInt(_walkboxes[i].vertexCount);
		for (int j = 0; j != 8; ++j) {
			f.writeVector3(_walkboxes[i].vertices[j]);

			// In BLADE.EXE vertices are a vec5
			f.writeInt(0);
			f.writeInt(0);
		}
	}

	for (int i = 0; i != 85; ++i) {
		f.writeInt(_walkboxStepSound[i]);
	}

	f.writeInt(_footstepSoundOverride);
}

void Set::load(SaveFileReadStream &f) {
	_loaded = f.readBool();
	_objectCount = f.readInt();
	_walkboxCount = f.readInt();

	for (int i = 0; i != _objectCount; ++i) {
		_objects[i].name = f.readStringSz(20);
		_objects[i].bbox = f.readBoundingBox(true);
		_objects[i].isObstacle = f.readBool();
		_objects[i].isClickable = f.readBool();
		_objects[i].isHotMouse = f.readBool();
		_objects[i].unknown1 = f.readInt();
		_objects[i].isTarget = f.readBool();
	}

	for (int i = 0; i != _walkboxCount; ++i) {
		_walkboxes[i].name = f.readStringSz(20);
		_walkboxes[i].altitude = f.readFloat();
		_walkboxes[i].vertexCount = f.readInt();
		for (int j = 0; j != 8; ++j) {
			_walkboxes[i].vertices[j] = f.readVector3();

			// In BLADE.EXE vertices are a vec5
			f.skip(8);
		}
	}

	for (int i = 0; i != 85; ++i) {
		_walkboxStepSound[i] = f.readInt();
	}

	_footstepSoundOverride = f.readInt();
}

/**
* Used for bugfixes mainly with respect to bad box positioning / bounding box fixes
* TODO If we have many such cases, perhaps we could use a lookup table
*		using sceneId, objectId (or name) as keys
*/
void Set::overrideSceneObjectInfo(int objectId) const { // For bugfixes with respect to clickable/targetable box positioning/bounding box
	if (_vm->_scene->getSceneId() == kSceneBB06) { /// Sebastian's room with doll
		if (_objects[objectId].name == "BOX31") { // dollhouse box in BB06
			_objects[objectId].bbox.setXYZ(-161.47f, 30.0f, 53.75f, -110.53f, 69.81f, 90.90f);
		}
	}
}

} // End of namespace BladeRunner
