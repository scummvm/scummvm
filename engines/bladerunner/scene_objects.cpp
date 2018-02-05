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

	for (int i = 0; i < kSceneObjectCount; ++i) {
		_sceneObjectsSortedByDistance[i] = -1;
	}
}

SceneObjects::~SceneObjects() {
	_vm = nullptr;
	_view = nullptr;
	_count = 0;
}

void SceneObjects::clear() {
	for (int i = 0; i < kSceneObjectCount; ++i) {
		_sceneObjects[i].sceneObjectId    = -1;
		_sceneObjects[i].sceneObjectType  = kSceneObjectTypeUnknown;
		_sceneObjects[i].distanceToCamera = 0;
		_sceneObjects[i].isPresent        = false;
		_sceneObjects[i].isClickable      = false;
		_sceneObjects[i].isObstacle       = false;
		_sceneObjects[i].unknown1         = 0;
		_sceneObjects[i].isTarget         = false;
		_sceneObjects[i].isMoving         = false;
		_sceneObjects[i].isRetired        = false;
	}
	_count = 0;
}

bool SceneObjects::addActor(int sceneObjectId, BoundingBox *boundingBox, Common::Rect *screenRectangle, bool isClickable, bool isMoving, bool isTarget, bool isRetired) {
	return addSceneObject(sceneObjectId, kSceneObjectTypeActor, boundingBox, screenRectangle, isClickable, false, 0, isTarget, isMoving, isRetired);
}

bool SceneObjects::addObject(int sceneObjectId, BoundingBox *boundingBox, bool isClickable, bool isObstacle, uint8 unknown1, bool isTarget) {
	Common::Rect rect(-1, -1, -1, -1);
	return addSceneObject(sceneObjectId, kSceneObjectTypeObject, boundingBox, &rect, isClickable, isObstacle, unknown1, isTarget, false, false);
}

bool SceneObjects::addItem(int sceneObjectId, BoundingBox *boundingBox, Common::Rect *screenRectangle, bool isTarget, bool isObstacle) {
	return addSceneObject(sceneObjectId, kSceneObjectTypeItem, boundingBox, screenRectangle, isObstacle, 0, 0, isTarget, 0, 0);
}

bool SceneObjects::remove(int sceneObjectId) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return false;
	}
	_sceneObjects[i].isPresent = false;
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

int SceneObjects::findByXYZ(bool *isClickable, bool *isObstacle, bool *isTarget, float x, float y, float z, bool findClickables, bool findObstacles, bool findTargets) const {
	*isClickable = false;
	*isObstacle  = false;
	*isTarget    = false;

	for (int i = 0; i < _count; ++i) {
		assert(_sceneObjectsSortedByDistance[i] < kSceneObjectCount);

		const SceneObject *sceneObject = &_sceneObjects[_sceneObjectsSortedByDistance[i]];

		if ((findClickables && sceneObject->isClickable) ||
			(findObstacles  && sceneObject->isObstacle) ||
			(findTargets    && sceneObject->isTarget)) {
			BoundingBox boundingBox = sceneObject->boundingBox;

			if (sceneObject->sceneObjectType == kSceneObjectTypeObject || sceneObject->sceneObjectType == kSceneObjectTypeItem) {
				boundingBox.expand(-4.0, 0.0, -4.0, 4.0, 0.0, 4.0);
			}

			if (boundingBox.inside(x, y, z)) {
				*isClickable = sceneObject->isClickable;
				*isObstacle  = sceneObject->isObstacle;
				*isTarget    = sceneObject->isTarget;

				return sceneObject->sceneObjectId;
			}
		}
	}

	return -1;
}

bool SceneObjects::existsOnXZ(int exceptSceneObjectId, float x, float z, bool a5, bool a6) const {
	float xMin = x - 12.5f;
	float xMax = x + 12.5f;
	float zMin = z - 12.5f;
	float zMax = z + 12.5f;

	int count = _count;

	if (count > 0) {
		for (int i = 0; i < count; i++) {
			const SceneObject *sceneObject = &_sceneObjects[_sceneObjectsSortedByDistance[i]];
			bool v13 = false;
			if (sceneObject->sceneObjectType == kSceneObjectTypeActor) {
				if (sceneObject->isRetired) {
					v13 = false;
				} else if (sceneObject->isMoving) {
					v13 = a5 != 0;
				} else {
					v13 = a6 != 0;
				}
			} else {
				v13 = sceneObject->isObstacle;
			}

			if (v13 && sceneObject->sceneObjectId != exceptSceneObjectId) {
				float x1, y1, z1, x2, y2, z2;
				sceneObject->boundingBox.getXYZ(&x1, &y1, &z1, &x2, &y2, &z2);
				if (z1 <= zMax && z2 >= zMin && x1 <= xMax && x2 >= xMin) {
					return true;
				}
			}
		}
	}
	return false;
}

int SceneObjects::findById(int sceneObjectId) const {
	for (int i = 0; i < _count; ++i) {
		int j = this->_sceneObjectsSortedByDistance[i];

		if (_sceneObjects[j].isPresent && _sceneObjects[j].sceneObjectId == sceneObjectId) {
			return j;
		}
	}
	return -1;
}

bool SceneObjects::addSceneObject(int sceneObjectId, SceneObjectType sceneObjectType, BoundingBox *boundingBox, Common::Rect *screenRectangle, bool isClickable, bool isObstacle, uint8 unknown1, bool isTarget, bool isMoving, bool isRetired) {
	int index = findEmpty();
	if (index == -1) {
		return false;
	}

	_sceneObjects[index].sceneObjectId   = sceneObjectId;
	_sceneObjects[index].sceneObjectType = sceneObjectType;
	_sceneObjects[index].isPresent       = true;
	_sceneObjects[index].boundingBox     = *boundingBox;
	_sceneObjects[index].screenRectangle = *screenRectangle;
	_sceneObjects[index].isClickable     = isClickable;
	_sceneObjects[index].isObstacle      = isObstacle;
	_sceneObjects[index].unknown1        = unknown1;
	_sceneObjects[index].isTarget        = isTarget;
	_sceneObjects[index].isMoving        = isMoving;
	_sceneObjects[index].isRetired       = isRetired;

	float centerZ = (_sceneObjects[index].boundingBox.getZ0() + _sceneObjects[index].boundingBox.getZ1()) / 2.0;

	float distanceToCamera = fabs(_view->_cameraPosition.z - centerZ);
	_sceneObjects[index].distanceToCamera = distanceToCamera;

	// insert according to distance from camera
	int i;
	for (i = 0; i < _count; ++i) {
		if (distanceToCamera < _sceneObjects[_sceneObjectsSortedByDistance[i]].distanceToCamera) {
			break;
		}
	}
	for (int j = CLIP(_count - 1, 0, kSceneObjectCount - 2); j >= i; --j) {
		_sceneObjectsSortedByDistance[j + 1] = _sceneObjectsSortedByDistance[j];
	}

	_sceneObjectsSortedByDistance[i] = index;
	++_count;
	return true;
}

int SceneObjects::findEmpty() const {
	for (int i = 0; i < kSceneObjectCount; ++i) {
		if (!_sceneObjects[i].isPresent)
			return i;
	}
	return -1;
}

void SceneObjects::setMoving(int sceneObjectId, bool isMoving) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return;
	}
	_sceneObjects[i].isMoving = isMoving;
}

void SceneObjects::setRetired(int sceneObjectId, bool isRetired) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return;
	}
	_sceneObjects[i].isRetired = isRetired;
}

bool SceneObjects::isBetweenTwoXZ(int sceneObjectId, float x1, float z1, float x2, float z2) const {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return false;
	}

	float objectX1, objectY1, objectZ1, objectX2, objectY2, objectZ2;
	_sceneObjects[i].boundingBox.getXYZ(&objectX1, &objectY1, &objectZ1, &objectX2, &objectY2, &objectZ2);

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
	_sceneObjects[i].isClickable = isClickable;
}

void SceneObjects::setIsObstacle(int sceneObjectId, bool isObstacle) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return;
	}
	_sceneObjects[i].isObstacle = isObstacle;
}

void SceneObjects::setIsTarget(int sceneObjectId, bool isTarget) {
	int i = findById(sceneObjectId);
	if (i == -1) {
		return;
	}
	_sceneObjects[i].isTarget = isTarget;
}

void SceneObjects::updateObstacles() {
	_vm->_obstacles->clear();
	for(int i = 0; i < _count; i++) {
		int index = _sceneObjectsSortedByDistance[i];
		SceneObject sceneObject = _sceneObjects[index];
		if(sceneObject.isObstacle) {
			float x0, y0, z0, x1, y1, z1;
			sceneObject.boundingBox.getXYZ(&x0, &y0, &z0, &x1, &y1, &z1);
			_vm->_obstacles->add(x0, z0, x1, z1);
		}
	}
	_vm->_obstacles->backup();
}

} // End of namespace BladeRunner
