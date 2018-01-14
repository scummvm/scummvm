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

#ifndef BLADERUNNER_SCENE_OBJECTS_H
#define BLADERUNNER_SCENE_OBJECTS_H

#include "bladerunner/boundingbox.h"

#include "common/rect.h"

namespace BladeRunner {

class BladeRunnerEngine;
class View;

enum SceneObjectType {
	kSceneObjectTypeUnknown = -1,
	kSceneObjectTypeActor = 0,
	kSceneObjectTypeObject = 1,
	kSceneObjectTypeItem = 2
};

enum SceneObjectOffset {
	kSceneObjectOffsetActors = 0,
	kSceneObjectOffsetItems = 74,
	kSceneObjectOffsetObjects = 198
};

class SceneObjects {
#if BLADERUNNER_DEBUG_RENDERING
	friend class BladeRunnerEngine;
#endif
	static const int kSceneObjectCount = 115;

	struct SceneObject {
		int             sceneObjectId;
		SceneObjectType sceneObjectType;
		BoundingBox     boundingBox;
		Common::Rect    screenRectangle;
		float           distanceToCamera;
		int             present;
		int             isClickable;
		int             isObstacle;
		int             unknown1;
		int             isTarget;
		int             isMoving;
		int             isRetired;
	};

	BladeRunnerEngine *_vm;

	View        *_view;
	int          _count;
	SceneObject  _sceneObjects[kSceneObjectCount];
	int          _sceneObjectsSortedByDistance[kSceneObjectCount];

public:
	SceneObjects(BladeRunnerEngine *vm, View *view);
	~SceneObjects();

	bool addActor(int sceneObjectId, BoundingBox *boundingBox, Common::Rect *screenRectangle, uint8 isClickable, uint8 unknown1, uint8 isTarget, uint8 isRetired);
	bool addObject(int sceneObjectId, BoundingBox *boundingBox, uint8 isClickable, uint8 isObstacle, uint8 unknown1, uint8 isTarget);
	bool addItem(int sceneObjectId, BoundingBox *boundingBox, Common::Rect *screenRectangle, uint8 isTarget, uint8 isObstacle);
	bool remove(int sceneObjectId);
	void clear();
	int findByXYZ(int *isClickable, int *isObstacle, int *isTarget, float x, float y, float z, int findClickables, int findObstacles, int findTargets) const;
	bool existsOnXZ(int exceptSceneObjectId, float x, float z, bool a5, bool a6) const;
	void setMoving(int sceneObjectId, bool isMoving);
	void setRetired(int sceneObjectId, bool isRetired);
	bool isBetweenTwoXZ(int sceneObjectId, float x1, float z1, float x2, float z2) const;
	void setIsClickable(int sceneObjectId, bool isClickable);
	void setIsObstacle(int sceneObjectId, bool isObstacle);
	void setIsTarget(int sceneObjectId, bool isTarget);
	void updateObstacles();

private:
	int findById(int sceneObjectId) const;
	bool addSceneObject(int sceneObjectId, SceneObjectType sceneObjectType, BoundingBox *boundingBox, Common::Rect *screenRectangle, uint8 isClickable, uint8 isObstacle, uint8 unknown1, uint8 isTarget, uint unknown2, uint isRetired);
	int findEmpty() const;
};

} // End of namespace BladeRunner

#endif
