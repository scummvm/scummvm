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

#include "bladerunner/bladerunner.h"

#include "bladerunner/boundingbox.h"
#include "bladerunner/view.h"

#include "common/rect.h"

namespace BladeRunner {
	class BladeRunnerEngine;
	class SceneObject;

	enum SceneObjectType
	{
		SceneObjectTypeUnknown = -1,
		SceneObjectTypeActor = 0,
		SceneObjectTypeObject = 1,
		SceneObjectTypeItem = 2,
	};

#define SCENE_OBJECTS_COUNT 115
#define SCENE_OBJECTS_ACTORS_OFFSET 0
#define SCENE_OBJECTS_ITEMS_OFFSET 74
#define SCENE_OBJECTS_OBJECTS_OFFSET 198



	class SceneObjects {
		BladeRunnerEngine *_vm;
	private:
		View *_view;
		int _count;
		SceneObject *_sceneObjects;
		int *_sceneObjectsSortedByDistance;

	public:
		SceneObjects(BladeRunnerEngine *vm, View *view);
		~SceneObjects();
		void reset();
		bool addActor(int sceneObjectId, BoundingBox* boundingBox, Common::Rect* screenRectangle, uint8 isClickable, uint8 unknown1, uint8 isCombatTarget, uint8 isRetired);
		bool addObject(int sceneObjectId, BoundingBox* boundingBox, uint8 isClickable, uint8 isObstacle, uint8 unknown1, uint8 isCombatTarget);
		bool addItem(int sceneObjectId, BoundingBox* boundingBox, Common::Rect* screenRectangle, uint8 isCombatTarget, uint8 isObstacle);
		bool remove(int sceneObjectId);
		int findByXYZ(int* clickable, int* obstacle, int* targetable, float x, float y, float z, int firstClickable, int firstObstacle, int firstTargetable);		
	private:
		int findById(int sceneObjectId);
		bool addSceneObject(int sceneObjectId, SceneObjectType sceneObjectType, BoundingBox* boundingBox, Common::Rect* screenRectangle, uint8 isClickable, uint8 isObstacle, uint8 unknown1, uint8 isCombatTarget, uint unknown2, uint isRetired);
		int findEmpty();
	};

	class SceneObject
	{
		friend class SceneObjects;
	private:
		int _sceneObjectId;
		SceneObjectType _sceneObjectType;
		BoundingBox _boundingBox;
		Common::Rect _screenRectangle;
		float _distanceToCamera;
		int _present;
		int _isClickable;
		int _isObstacle;
		int _unknown1;
		int _isCombatTarget;
		int _unknown2;
		int _isRetired;
	public:
		SceneObject();
		~SceneObject();
	};
}
#endif
