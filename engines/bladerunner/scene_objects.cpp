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

#include "bladerunner/scene_objects.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/obstacles.h"
#include "bladerunner/view.h"


namespace BladeRunner {

SceneObjects::SceneObjects(BladeRunnerEngine *vm, View *view) {
	_vm = vm;
	_view = view;

	_count = 0;
	_sceneObjects = new SceneObject[SCENE_OBJECTS_COUNT];
	_sceneObjectsSortedByDistance = new int[SCENE_OBJECTS_COUNT];

	for (int i = 0; i < SCENE_OBJECTS_COUNT; ++i) {
		_sceneObjectsSortedByDistance[i] = -1;
	}
}

SceneObjects::~SceneObjects() {
	_vm = nullptr;
	_view = nullptr;
	_count = 0;

	delete[] _sceneObjectsSortedByDistance;
	delete[] _sceneObjects;
}

void SceneObjects::clear() {
	for (int i = 0; i < SCENE_OBJECTS_COUNT; ++i) {
		_sceneObjects[i]._sceneObjectId = -1;
		_sceneObjects[i]._sceneObjectType = SceneObjectTypeUnknown;
		_sceneObjects[i]._distanceToCamera = 0;
		_sceneObjects[i]._present = 0;
		_sceneObjects[i]._isClickable = 0;
		_sceneObjects[i]._isObstacle = 0;
		_sceneObjects[i]._unknown1 = 0;
		_sceneObjects[i]._isTarget = 0;
		_sceneObjects[i]._isMoving = 0;
		_sceneObjects[i]._isRetired = 0;
	}
	_count = 0;
}

bool SceneObjects::addActor(int sceneObjectId, BoundingBox *boundingBox, Common::Rect *screenRectangle, uint8 isClickable, uint8 isMoving, uint8 isTarget, uint8 isRetired) {
	return addSceneObject(sceneObjectId, SceneObjectTypeActor, boundingBox, screenRectangle, isClickable, 0, 0, isTarget, isMoving, isRetired);
}

bool SceneObjects::addObject(int sceneObjectId, BoundingBox *boundingBox, uint8 isClickable, uint8 isObstacle, uint8 unknown1, uint8 isTarget) {
	Common::Rect rect(-1, -1, -1, -1);
	return addSceneObject(sceneObjectId, SceneObjectTypeObject, boundingBox, &rect, isClickable, isObstacle, unknown1, isTarget, 0, 0);
}

bool SceneObjects::addItem(int sceneObjectId, BoundingBox *boundingBox, Common::Rect *screenRectangle, uint8 isTarget, uint8 isObstacle) {
	return addSceneObject(sceneObjectId, SceneObjectTypeItem, boundingBox, screenRectangle, isObstacle, 0, 0, isTarget, 0, 0);
}

bool SceneObjects::remove(int sceneObjectId) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return false;
	}
	_sceneObjects[i]._present = 0;
	int j;
	for (j = 0; j < _count; ++j) {
		if (_sceneObjectsSortedByDistance[j] == i) {
			break;
		}
	}
	for (int k = j; k < _count - 1; ++k) {
		_sceneObjectsSortedByDistance[k] = _sceneObjectsSortedByDistance[k + 1];
	}

	--_count;
	return true;
}

int SceneObjects::findByXYZ(int *isClickable, int *isObstacle, int *isTarget, float x, float y, float z, int findClickables, int findObstacles, int findTargets) {
	*isClickable = 0;
	*isObstacle = 0;
	*isTarget = 0;

	for (int i = 0; i < _count; ++i) {
		assert(_sceneObjectsSortedByDistance[i] < SCENE_OBJECTS_COUNT);

		SceneObject &sceneObject = _sceneObjects[_sceneObjectsSortedByDistance[i]];

		if ((findClickables && sceneObject._isClickable) ||
			(findObstacles  && sceneObject._isObstacle) ||
			(findTargets    && sceneObject._isTarget)) {
			BoundingBox boundingBox = sceneObject._boundingBox;

			if (sceneObject._sceneObjectType == SceneObjectTypeObject || sceneObject._sceneObjectType == SceneObjectTypeItem) {
				boundingBox.expand(-4.0, 0.0, -4.0, 4.0, 0.0, 4.0);
			}

			if (boundingBox.inside(x, y, z)) {
				*isClickable = sceneObject._isClickable;
				*isObstacle = sceneObject._isObstacle;
				*isTarget = sceneObject._isTarget;

				return sceneObject._sceneObjectId;
			}
		}
	}

	return -1;
}

bool SceneObjects::existsOnXZ(int exceptSceneObjectId, float x, float z, bool a5, bool a6) {
	float xMin = x - 12.5f;
	float xMax = x + 12.5f;
	float zMin = z - 12.5f;
	float zMax = z + 12.5f;

	int count = this->_count;

	if (count > 0) {
		for (int i = 0; i < count; i++) {
			SceneObject *sceneObject = &this->_sceneObjects[this->_sceneObjectsSortedByDistance[i]];
			bool v13 = false;
			if (sceneObject->_sceneObjectType == SceneObjectTypeActor) {
				if (sceneObject->_isRetired) {
					v13 = false;
				} else if (sceneObject->_isMoving) {
					v13 = a5 != 0;
				} else {
					v13 = a6 != 0;
				}
			} else {
				v13 = sceneObject->_isObstacle;
			}

			if (v13 && sceneObject->_sceneObjectId != exceptSceneObjectId) {
				float x1, y1, z1, x2, y2, z2;
				sceneObject->_boundingBox.getXYZ(&x1, &y1, &z1, &x2, &y2, &z2);
				if (z1 <= zMax && z2 >= zMin && x1 <= xMax && x2 >= xMin) {
					return true;
				}
			}
		}
	}
	return false;
}

int SceneObjects::findById(int sceneObjectId) {
	for (int i = 0; i < _count; ++i) {
		int j = this->_sceneObjectsSortedByDistance[i];

		if (_sceneObjects[j]._present && _sceneObjects[j]._sceneObjectId == sceneObjectId) {
			return j;
		}
	}
	return -1;
}

bool SceneObjects::addSceneObject(int sceneObjectId, SceneObjectType sceneObjectType, BoundingBox *boundingBox, Common::Rect *screenRectangle, uint8 isClickable, uint8 isObstacle, uint8 unknown1, uint8 isTarget, uint isMoving, uint isRetired) {
	int index = findEmpty();
	if (index == -1) {
		return false;
	}

	_sceneObjects[index]._sceneObjectId = sceneObjectId;
	_sceneObjects[index]._sceneObjectType = sceneObjectType;
	_sceneObjects[index]._present = 1;
	_sceneObjects[index]._boundingBox = *boundingBox;
	_sceneObjects[index]._screenRectangle = *screenRectangle;
	_sceneObjects[index]._isClickable = isClickable;
	_sceneObjects[index]._isObstacle = isObstacle;
	_sceneObjects[index]._unknown1 = unknown1;
	_sceneObjects[index]._isTarget = isTarget;
	_sceneObjects[index]._isMoving = isMoving;
	_sceneObjects[index]._isRetired = isRetired;

	float centerZ = (_sceneObjects[index]._boundingBox.getZ0() + _sceneObjects[index]._boundingBox.getZ1()) / 2.0;

	float distanceToCamera = fabs(_view->_cameraPosition.z - centerZ);
	_sceneObjects[index]._distanceToCamera = distanceToCamera;

	// insert according to distance from camera
	int i;
	for (i = 0; i < _count; ++i) {
		if (distanceToCamera < _sceneObjects[_sceneObjectsSortedByDistance[i]]._distanceToCamera) {
			break;
		}
	}
	for (int j = _count - 2; j >= i; --j) {
		_sceneObjectsSortedByDistance[j + 1] = _sceneObjectsSortedByDistance[j];
	}

	_sceneObjectsSortedByDistance[i] = index;
	++_count;
	return true;
}

int SceneObjects::findEmpty() {
	for (int i = 0; i < SCENE_OBJECTS_COUNT; ++i) {
		if (!_sceneObjects[i]._present)
			return i;
	}
	return -1;
}

void SceneObjects::setMoving(int sceneObjectId, bool isMoving) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return;
	}
	_sceneObjects[i]._isMoving = isMoving;
}

void SceneObjects::setRetired(int sceneObjectId, bool isRetired) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return;
	}
	_sceneObjects[i]._isRetired = isRetired;
}

bool SceneObjects::isBetweenTwoXZ(int sceneObjectId, float x1, float z1, float x2, float z2) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return false;
	}

	float objectX1, objectY1, objectZ1, objectX2, objectY2, objectZ2;
	_sceneObjects[i]._boundingBox.getXYZ(&objectX1, &objectY1, &objectZ1, &objectX2, &objectY2, &objectZ2);

	//TODO
	//		if (!lineIntersectection(sourceX, sourceZ, targetX, targetZ, objectX1, objectZ1, objectX2, objectZ1, &intersectionX, &intersectionY, &v18)
	//			&& !lineIntersectection(sourceX, sourceZ, targetX, targetZ, objectX2, objectZ1, objectX2, objectZ2, &intersectionX, &intersectionY, &v18)
	//			&& !lineIntersectection(sourceX, sourceZ, targetX, targetZ, objectX2, objectZ2, objectX1, objectZ2, &intersectionX, &intersectionY, &v18)
	//			&& !lineIntersectection(sourceX, sourceZ, targetX, targetZ, objectX1, objectZ2, objectX1, objectZ1, &intersectionX, &intersectionY, &v18))
	//			return false;
	return true;
}


void SceneObjects::setIsClickable(int sceneObjectId, bool isClickable) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return;
	}
	_sceneObjects[i]._isClickable = isClickable;
}

void SceneObjects::setIsObstacle(int sceneObjectId, bool isObstacle) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return;
	}
	_sceneObjects[i]._isObstacle = isObstacle;
}

void SceneObjects::setIsTarget(int sceneObjectId, bool isTarget) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return;
	}
	_sceneObjects[i]._isTarget = isTarget;
}


void SceneObjects::updateObstacles() {
	_vm->_obstacles->clear();
	for(int i = 0; i < _count; i++) {
		int index = _sceneObjectsSortedByDistance[i];
		SceneObject sceneObject = _sceneObjects[index];
		if(sceneObject._isObstacle) {
			float x0, y0, z0, x1, y1, z1;
			sceneObject._boundingBox.getXYZ(&x0, &y0, &z0, &x1, &y1, &z1);
			_vm->_obstacles->add(x0, z0, x1, z1);
		}
	}
	_vm->_obstacles->backup();
}

} // End of namespace BladeRunner
