#include "bladerunner/scene_objects.h"

namespace BladeRunner {

	SceneObjects::SceneObjects(BladeRunnerEngine *vm, View *view) {
		_vm = vm;
		_view = view;

		_count = 0;
		_sceneObjects = new SceneObject[SCENE_OBJECTS_COUNT];
		_sceneObjectsSortedByDistance = new int[SCENE_OBJECTS_COUNT];

		int i;
		for (i = 0; i < SCENE_OBJECTS_COUNT; i++) {
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
		int i;
		for (i = 0; i < SCENE_OBJECTS_COUNT; i++) {
			_sceneObjects[i]._sceneObjectId = -1;
			_sceneObjects[i]._sceneObjectType = SceneObjectTypeUnknown;
			_sceneObjects[i]._distanceToCamera = 0;
			_sceneObjects[i]._present = 0;
			_sceneObjects[i]._isClickable = 0;
			_sceneObjects[i]._isObstacle = 0;
			_sceneObjects[i]._unknown1 = 0;
			_sceneObjects[i]._isCombatTarget = 0;
			_sceneObjects[i]._isMoving = 0;
			_sceneObjects[i]._isRetired = 0;
		}
	}

	bool SceneObjects::addActor(int sceneObjectId, BoundingBox* boundingBox, Common::Rect* screenRectangle, uint8 isClickable, uint8 isMoving, uint8 isCombatTarget, uint8 isRetired) {
		return addSceneObject(sceneObjectId, SceneObjectTypeActor, boundingBox, screenRectangle, isClickable, 0, 0, isCombatTarget, isMoving, isRetired);
	}

	bool SceneObjects::addObject(int sceneObjectId, BoundingBox* boundingBox, uint8 isClickable, uint8 isObstacle, uint8 unknown1, uint8 isCombatTarget) {
		Common::Rect rect(-1, -1, -1, -1);
		return addSceneObject(sceneObjectId, SceneObjectTypeObject, boundingBox, &rect, isClickable, isObstacle, unknown1, isCombatTarget, 0, 0);
	}

	bool SceneObjects::addItem(int sceneObjectId, BoundingBox* boundingBox, Common::Rect* screenRectangle, uint8 isCombatTarget, uint8 isObstacle) {
		return addSceneObject(sceneObjectId, SceneObjectTypeItem, boundingBox, screenRectangle, isObstacle, 0, 0, isCombatTarget, 0, 0);
	}

	bool SceneObjects::remove(int sceneObjectId)
	{
		int i = findById(sceneObjectId);
		if (i == -1 || !_sceneObjects[i]._present) {
			return false;
		}
		int j;
		for (j = 0; j < _count; j++) {
			if (_sceneObjectsSortedByDistance[j] == i) {
				break;
			}
		}
		int k;
		for (k = j; k < _count - 1; k++) {
			_sceneObjectsSortedByDistance[k] = _sceneObjectsSortedByDistance[k + 1];
		}

		--_count;
		return true;
	}

	int SceneObjects::findByXYZ(int *isClickable, int *isObstacle, int *isCombatTarget, float x, float y, float z, int mustBeClickable, int mustBeObstacle, int mustBeCombatTarget) {
		BoundingBox boundingBox;
		*isClickable = 0;
		*isObstacle = 0;
		*isCombatTarget = 0;

		if (!_count)
			return -1;

		int i;
		for (i = 0; i < _count; i++) {
			//assert(_sceneObjectsSortedByDistance[i] < _count);
			SceneObject *sceneObject = &_sceneObjects[_sceneObjectsSortedByDistance[i]];
			if ((mustBeClickable && !sceneObject->_isClickable)
				|| (mustBeObstacle && !sceneObject->_isObstacle)
				|| (mustBeCombatTarget && !sceneObject->_isCombatTarget)) {
				continue;
			}

			boundingBox = sceneObject->_boundingBox;

			if (sceneObject->_sceneObjectType == SceneObjectTypeObject || sceneObject->_sceneObjectType == SceneObjectTypeItem) {
				boundingBox.expand(-4.0, 0.0, -4.0, 4.0, 0.0, 4.0);
			}

			if (boundingBox.inside(x, y, z)) {
				*isClickable = sceneObject->_isClickable;
				*isObstacle = sceneObject->_isObstacle;
				*isCombatTarget = sceneObject->_isCombatTarget;
				return sceneObject->_sceneObjectId;
			}
		}

		return -1;
	}

	int SceneObjects::findById(int sceneObjectId)
	{
		int i;
		for (i = 0; i < _count; i++) {
			if (_sceneObjects[i]._present && _sceneObjects[i]._sceneObjectId == sceneObjectId) {
				return i;
			}
		}
		return -1;
	}

	bool SceneObjects::addSceneObject(int sceneObjectId, SceneObjectType sceneObjectType, BoundingBox* boundingBox, Common::Rect* screenRectangle, uint8 isClickable, uint8 isObstacle, uint8 unknown1, uint8 isCombatTarget, uint isMoving, uint isRetired) {
		int index = findEmpty();
		if (_count >= SCENE_OBJECTS_COUNT || index == -1) {
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
		_sceneObjects[index]._isCombatTarget = isCombatTarget;
		_sceneObjects[index]._isMoving = isMoving;
		_sceneObjects[index]._isRetired = isRetired;

		float centerZ = (_sceneObjects[index]._boundingBox.getZ0() + _sceneObjects[index]._boundingBox.getZ1()) / 2.0;

		float distanceToCamera = fabs(_view->_cameraPosition.z - centerZ);
		_sceneObjects[index]._distanceToCamera = distanceToCamera;

		// insert according to distance from camera
		int i, j;
		for (i = 0; i < _count; i++) {
			if (distanceToCamera < _sceneObjects[_sceneObjectsSortedByDistance[i]]._distanceToCamera) {
				break;
			}
		}

		for (j = _count - 1; j >= i; j--) {
			_sceneObjectsSortedByDistance[j + 1] = _sceneObjectsSortedByDistance[j];
		}

		_sceneObjectsSortedByDistance[i] = index;
		++_count;
		return true;
	}

	int SceneObjects::findEmpty() {
		int i;
		for (i = 0; i < SCENE_OBJECTS_COUNT; i++)
		{
			if (!_sceneObjects[i]._present)
				return i;
		}
		return -1;
	}

	void SceneObjects::setMoving(int sceneObjectId, bool isMoving) {
		int i = findById(sceneObjectId);
		if (i == -1 || !_sceneObjects[i]._present) {
			return;
		}
		_sceneObjects[i]._isMoving = isMoving;
	}

}
