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

#ifndef BLADERUNNER_SET_H
#define BLADERUNNER_SET_H

#include "bladerunner/boundingbox.h"

#include "common/scummsys.h"
#include "common/str.h"

namespace BladeRunner {

class BladeRunnerEngine;

class SaveFileReadStream;
class SaveFileWriteStream;
class SetEffects;
class SceneObjects;
class VQADecoder;

class Set {
	friend class Debugger;

	struct Object {
		Common::String name;
		BoundingBox    bbox;
		uint8          isObstacle;
		uint8          isClickable;
		uint8          isHotMouse;
		uint8          isTarget;
		uint8          unknown1;
	};

	struct Walkbox {
		Common::String name;
		float          altitude;
		int            vertexCount;
		Vector3        vertices[8];
	};

	BladeRunnerEngine *_vm;

	bool        _loaded;
	int         _objectCount;
	int         _walkboxCount;
	Object     *_objects;
	Walkbox    *_walkboxes;
	int         _walkboxStepSound[85];
	int         _footstepSoundOverride;
//	float       _unknown[10];

public:
	SetEffects *_effects;

public:
	Set(BladeRunnerEngine *vm);
	~Set();

	bool open(const Common::String &name);

	void addObjectsToScene(SceneObjects *sceneObjects) const;
	uint32 getObjectCount() const { return _objectCount; }

	float getAltitudeAtXZ(float x, float z, bool *inWalkbox) const;

	int findWalkbox(float x, float z) const;
	int findObject(const Common::String &objectName) const;

	bool objectSetHotMouse(int objectId) const;
	bool objectGetBoundingBox(int objectId, BoundingBox *boundingBox) const;
	void objectSetIsClickable(int objectId, bool isClickable);
	void objectSetIsObstacle(int objectId, bool isObstacle);
	void objectSetIsTarget(int objectId, bool isTarget);
	const Common::String &objectGetName(int objectId) const;

	void setWalkboxStepSound(int walkboxId, int floorType);
	void setFoodstepSoundOverride(int floorType);
	void resetFoodstepSoundOverride();

	int getWalkboxSoundWalkLeft(int walkboxId) const;
	int getWalkboxSoundWalkRight(int walkboxId) const;
	int getWalkboxSoundRunLeft(int walkboxId) const;
	int getWalkboxSoundRunRight(int walkboxId) const;

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

private:
	static bool isXZInWalkbox(float x, float z, const Walkbox &walkbox);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	void overrideSceneObjectInfo(int objectId) const;
	void setupNewObjectInSet(Common::String objName, BoundingBox objBbox);
	void patchInAdditionalObjectsInSet();
	void patchOutBadObjectsFromSet();
#endif // BLADERUNNER_ORIGINAL_BUGS
};

} // End of namespace BladeRunner

#endif
