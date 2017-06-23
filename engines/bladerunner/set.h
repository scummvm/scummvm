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

class VQADecoder;
class SetEffects;
class SceneObjects;

struct Object {
	char        _name[20];
	BoundingBox _bbox;
	uint8       _isObstacle;
	uint8       _isClickable;
	uint8       _isHotMouse;
	uint8       _isTarget;
	uint8       _unknown1;
};

struct Walkbox {
	char    _name[20];
	float   _altitude;
	int     _vertexCount;
	Vector3 _vertices[8];
};

class Set {
#if _DEBUG
	friend class BladeRunnerEngine;
#endif

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
	int findObject(const char *objectName) const;

	bool objectSetHotMouse(int objectId) const;
	bool objectGetBoundingBox(int objectId, BoundingBox *boundingBox) const;
	void objectSetIsClickable(int objectId, bool isClickable) const;
	void objectSetIsObstacle(int objectId, bool isObstacle) const;
	void objectSetIsTarget(int objectId, bool isTarget) const;
	const char *objectGetName(int objectId) const;

	void setWalkboxStepSound(int walkboxId, int soundId);
	void setFoodstepSoundOverride(int soundId);
	void resetFoodstepSoundOverride();
	int getWalkboxSoundWalkLeft(int walkboxId);
	int getWalkboxSoundWalkRight(int walkboxId);
	int getWalkboxSoundRunLeft(int walkboxId);
	int getWalkboxSoundRunRight(int walkboxId);
	
};

} // End of namespace BladeRunner

#endif
