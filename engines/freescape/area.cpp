/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#include "common/algorithm.h"
#include "common/hash-ptr.h"

#include "freescape/freescape.h"
#include "freescape/area.h"
#include "freescape/objects/global.h"
#include "freescape/sweepAABB.h"

namespace Freescape {

Object *Area::objectWithIDFromMap(ObjectMap *map, uint16 objectID) {
	if (!map)
		return nullptr;
	if (!map->contains(objectID))
		return nullptr;
	return (*map)[objectID];
}

Object *Area::objectWithID(uint16 objectID) {
	return objectWithIDFromMap(_objectsByID, objectID);
}

Object *Area::entranceWithID(uint16 objectID) {
	return objectWithIDFromMap(_entrancesByID, objectID);
}

uint16 Area::getAreaID() {
	return _areaID;
}

uint16 Area::getAreaFlags() {
	return _areaFlags;
}

uint8 Area::getScale() {
	return _scale;
}

Area::Area(uint16 areaID_, uint16 areaFlags_, ObjectMap *objectsByID_, ObjectMap *entrancesByID_, bool isCastle_) {
	_areaID = areaID_;
	_areaFlags = areaFlags_;
	_objectsByID = objectsByID_;
	_entrancesByID = entrancesByID_;
	_isCastle = isCastle_;

	_scale = 0;
	_hasSyntheticFloor = false;
	_skyColor = 255;
	_groundColor = 255;
	_usualBackgroundColor = 255;
	_underFireBackgroundColor = 255;
	_inkColor = 255;
	_paperColor = 255;
	_colorCycling = false;

	_gasPocketRadius = 0;

	// create a list of drawable objects only
	for (auto &it : *_objectsByID) {
		if (it._value->isDrawable()) {
			_drawableObjects.push_back(it._value);
		}
	}

	_lastTick = 0;
	_lastDepthLayerTick = 0;
	_lastCameraRoll = 0.0f;
	_lastDepthLayerCameraRoll = 0.0f;
	_lastFov = 0.0f;
	_lastAspectRatio = 0.0f;
	_lastNearClipPlane = 0.0f;
	_lastFarClipPlane = 0.0f;
	_lastDepthLayerFov = 0.0f;
	_lastDepthLayerAspectRatio = 0.0f;
	_lastDepthLayerNearClipPlane = 0.0f;
	_lastDepthLayerFarClipPlane = 0.0f;
	_lastRenderDepthLayer = kRenderDepthAll;
	_lastForegroundDistance = 0.0f;
}

Area::~Area() {
	if (_entrancesByID) {
		for (auto &it : *_entrancesByID) {
			if (!_addedObjects.contains(it._value->getObjectID()))
				delete it._value;
		}
	}

	if (_objectsByID) {
		for (auto &it : *_objectsByID) {
			if (!_addedObjects.contains(it._value->getObjectID()))
				delete it._value;
		}
	}

	delete _entrancesByID;
	delete _objectsByID;
}

ObjectArray Area::getSensors() {
	ObjectArray sensors;
	debugC(1, kFreescapeDebugMove, "Area name: %s", _name.c_str());
	for (auto &it : *_objectsByID) {
		if (it._value->getType() == kSensorType)
			sensors.push_back(it._value);
	}
	return sensors;
}

void Area::show() {
	debugC(1, kFreescapeDebugMove, "Area name: %s", _name.c_str());
	for (auto &it : *_objectsByID)
		debugC(1, kFreescapeDebugMove, "objID: %d, type: %d", it._value->getObjectID(), it._value->getType());

	for (auto &it : *_entrancesByID)
		debugC(1, kFreescapeDebugMove, "objID: %d, type: %d (entrance)", it._value->getObjectID(), it._value->getType());
}

void Area::loadObjects(Common::SeekableReadStream *stream, Area *global) {
	int objectsByIDSize = stream->readUint32LE();

	for (int i = 0; i < objectsByIDSize; i++) {
		uint16 key = stream->readUint32LE();
		uint32 flags = stream->readUint32LE();
		float x = stream->readFloatLE();
		float y = stream->readFloatLE();
		float z = stream->readFloatLE();
		Object *obj = nullptr;
		if (!_objectsByID->contains(key))
			addObjectFromArea(key, global);

		obj = (*_objectsByID)[key];
		assert(obj);
		obj->setObjectFlags(flags);
		obj->setOrigin(Math::Vector3d(x, y, z));
	}

	_colorRemaps.clear();
	int colorRemapsSize = stream->readUint32LE();

	for (int i = 0; i < colorRemapsSize; i++) {
		int src = stream->readUint32LE();
		int dst = stream->readUint32LE();
		remapColor(src, dst);
	}
}

void Area::saveObjects(Common::WriteStream *stream) {
	stream->writeUint32LE(_objectsByID->size());

	for (auto &it : *_objectsByID) {
		Object *obj = it._value;
		stream->writeUint32LE(it._key);
		stream->writeUint32LE(obj->getObjectFlags());
		stream->writeFloatLE(obj->getOrigin().x());
		stream->writeFloatLE(obj->getOrigin().y());
		stream->writeFloatLE(obj->getOrigin().z());
	}

	stream->writeUint32LE(_colorRemaps.size());
	for (auto &it : _colorRemaps) {
		stream->writeUint32LE(it._key);
		stream->writeUint32LE(it._value);
	}
}

void Area::remapColor(int index, int color) {
	_colorRemaps[index] = color;
}

void Area::unremapColor(int index) {
	_colorRemaps.clear(index);
}

void Area::resetAreaGroups() {
	debugC(1, kFreescapeDebugMove, "Resetting groups from area: %s", _name.c_str());
	if (_objectsByID) {
		for (auto &it : *_objectsByID) {
			Object *obj = it._value;

			if (obj->getType() == ObjectType::kGroupType)
				((Group *)obj)->reset();
		}
	}
}

void Area::resetArea() {
	debugC(1, kFreescapeDebugMove, "Resetting objects from area: %s", _name.c_str());
	_colorRemaps.clear();
	if (_objectsByID) {
		for (auto &it : *_objectsByID) {
			Object *obj = it._value;
			if (obj->isDestroyed())
				obj->restore();

			if (obj->isInitiallyInvisible())
				obj->makeInvisible();
			else
				obj->makeVisible();
		}
	}
	if (_entrancesByID) {
		for (auto &it : *_entrancesByID) {
			Object *obj = it._value;
			if (obj->isDestroyed())
				obj->restore();

			if (obj->isInitiallyInvisible())
				obj->makeInvisible();
			else
				obj->makeVisible();
		}
	}
}


static float aabbMaxProjection(const Math::AABB &aabb, const Math::Vector3d &axis) {
	const Math::Vector3d min = aabb.getMin();
	const Math::Vector3d max = aabb.getMax();
	Math::Vector3d support(
		axis.x() >= 0.0f ? max.x() : min.x(),
		axis.y() >= 0.0f ? max.y() : min.y(),
		axis.z() >= 0.0f ? max.z() : min.z());

	return support.dotProduct(axis);
}

static float aabbMinProjection(const Math::AABB &aabb, const Math::Vector3d &axis) {
	const Math::Vector3d min = aabb.getMin();
	const Math::Vector3d max = aabb.getMax();
	Math::Vector3d support(
		axis.x() >= 0.0f ? min.x() : max.x(),
		axis.y() >= 0.0f ? min.y() : max.y(),
		axis.z() >= 0.0f ? min.z() : max.z());

	return support.dotProduct(axis);
}

static bool aabbIntersectsViewVolume(const Math::AABB &aabb, const Math::Vector3d &camera, const Math::Vector3d &direction, float roll, float fov, float aspectRatio, float nearClipPlane, float farClipPlane) {
	if (!aabb.isValid())
		return false;

	Math::Vector3d front = direction.getNormalized();
	if (front.getSquareMagnitude() == 0.0f)
		return true;

	// Exclude offscreen objects before sorting; they can change the visible draw order.
	Math::Vector3d right = Math::Vector3d::crossProduct(front, Math::Vector3d(0.0f, 1.0f, 0.0f));
	if (right.getSquareMagnitude() < 0.0001f)
		right = Math::Vector3d(1.0f, 0.0f, 0.0f);
	else
		right.normalize();
	Math::Vector3d up = Math::Vector3d::crossProduct(right, front).getNormalized();
	if (roll != 0.0f) {
		// Match positionCamera's roll by rotating the view axes inversely.
		const float c = cos(Math::deg2rad(roll));
		const float s = sin(Math::deg2rad(roll));
		auto rotateAxis = [c, s](const Math::Vector3d &axis) {
			return Math::Vector3d(c * axis.x() + s * axis.y(), -s * axis.x() + c * axis.y(), axis.z());
		};
		front = rotateAxis(front);
		right = rotateAxis(right);
		up = rotateAxis(up);
	}

	const float padding = 32.0f;
	const float minDepth = aabbMinProjection(aabb, front) - camera.dotProduct(front);
	const float maxDepth = aabbMaxProjection(aabb, front) - camera.dotProduct(front);
	if (maxDepth < nearClipPlane - padding)
		return false;
	if (minDepth > farClipPlane + padding)
		return false;

	// Match updateProjectionMatrix's horizontal FOV.
	const float horizontalScale = tan(Math::deg2rad(fov) / 2.0f);
	const float verticalScale = horizontalScale / aspectRatio;
	const Math::Vector3d planes[] = {
		front * horizontalScale + right, front * horizontalScale - right,
		front * verticalScale + up, front * verticalScale - up
	};
	for (uint i = 0; i < ARRAYSIZE(planes); i++) {
		if (aabbMaxProjection(aabb, planes[i]) - camera.dotProduct(planes[i]) < -padding)
			return false;
	}

	return true;
}

static bool objectIsSortCandidate(Object *obj, const Math::Vector3d &camera, const Math::Vector3d &direction, float roll, float fov, float aspectRatio, float nearClipPlane, float farClipPlane) {
	if (!obj || obj->isDestroyed() || obj->isInvisible() || !obj->isGeometric())
		return false;

	// Sorting bounds may exclude geometry; cull using the actual bounds.
	return aabbIntersectsViewVolume(obj->_boundingBox, camera, direction, roll, fov, aspectRatio, nearClipPlane, farClipPlane);
}

// Returns 0 if incomparable, 1 if A is closer, or 2 if B is closer.
static int compareBoundingBoxAxis(float minA, float maxA, float minB, float maxB) {
	// Touching bounds are comparable; overlapping intervals are not.
	if (minA < maxB && minB < maxA)
		return 0;

	const bool negativeA = minA < 0.0f;
	const bool negativeB = minB < 0.0f;
	if (negativeA != (maxA < 0.0f))
		return 1;
	if (negativeB != (maxB < 0.0f))
		return 2;
	if (negativeA != negativeB)
		return 0;

	float difference = minB - minA;
	if (difference == 0.0f)
		difference = maxB - maxA;
	return (difference < 0.0f) == negativeB ? 1 : 2;
}

static void sortObjectsForRendering(ObjectArray &objects, const Math::Vector3d &camera) {
	const int n = objects.size();
	if (n < 2)
		return;

	// Start in file order, with globals first.
	Common::sort(objects.begin(), objects.end(), [](Object *a, Object *b) {
		return a->_loadIndex < b->_loadIndex;
	});

	// Incomparable pairs also swap, so keep n - 1 passes over all adjacent pairs.
	for (int pass = 1; pass < n; pass++) {
		bool changed = false;
		for (int j = 0; j < n - 1; j++) {
			// Sort using unrotated header bounds relative to the camera.
			const Math::Vector3d minA = objects[j]->_occlusionBox.getMin() - camera;
			const Math::Vector3d maxA = objects[j]->_occlusionBox.getMax() - camera;
			const Math::Vector3d minB = objects[j + 1]->_occlusionBox.getMin() - camera;
			const Math::Vector3d maxB = objects[j + 1]->_occlusionBox.getMax() - camera;
			int result = 0;
			for (int axis = 0; axis < 3; axis++)
				result = (result << 2) | compareBoundingBoxAxis(minA.getValue(axis), maxA.getValue(axis), minB.getValue(axis), maxB.getValue(axis));

			// Keep order only if B is closer on some axis and A is closer on none.
			if (result != 0 && (result & 0x15) == 0)
				continue;

			SWAP(objects[j], objects[j + 1]);
			changed = true;
		}
		if (!changed)
			break;
	}
}

static float aabbNearestDepth(const Math::AABB &aabb, const Math::Vector3d &camera, const Math::Vector3d &direction) {
	const Math::Vector3d min = aabb.getMin();
	const Math::Vector3d max = aabb.getMax();
	float nearest = FLT_MAX;
	float farthest = -FLT_MAX;

	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 2; z++) {
				Math::Vector3d corner(
					x ? max.x() : min.x(),
					y ? max.y() : min.y(),
					z ? max.z() : min.z());
				float depth = (corner - camera).dotProduct(direction);
				nearest = MIN(nearest, depth);
				farthest = MAX(farthest, depth);
			}
		}
	}

	return farthest < 0.0f ? FLT_MAX : MAX(0.0f, nearest);
}

static float objectNearestDepth(Object *obj, const Math::Vector3d &camera, const Math::Vector3d &direction) {
	if (!obj || obj->isDestroyed() || obj->isInvisible())
		return FLT_MAX;

	if (obj->getType() == ObjectType::kGroupType) {
		Group *group = (Group *)obj;
		float nearest = FLT_MAX;
		for (auto &child : group->_objects)
			nearest = MIN(nearest, objectNearestDepth(child, camera, direction));
		return nearest;
	}

	Math::AABB bounds = obj->_boundingBox;
	if (!bounds.isValid()) {
		bounds.expand(obj->_origin);
		bounds.expand(obj->_origin + obj->_size);
	}

	return bounds.isValid() ? aabbNearestDepth(bounds, camera, direction) : FLT_MAX;
}

static bool objectInDepthLayer(Object *obj, const Math::Vector3d &camera, const Math::Vector3d &direction, Area::RenderDepthLayer depthLayer, float foregroundDistance) {
	if (depthLayer == Area::kRenderDepthAll)
		return true;

	float nearestDepth = objectNearestDepth(obj, camera, direction);
	bool foreground = nearestDepth <= foregroundDistance;
	return depthLayer == Area::kRenderDepthForeground ? foreground : !foreground;
}

void Area::draw(Freescape::Renderer *gfx, uint32 animationTicks, Math::Vector3d camera, Math::Vector3d direction, float roll, bool insideWait, float fov, float aspectRatio, float nearClipPlane, float farClipPlane) {
	bool runAnimation = animationTicks != _lastTick;
	bool cameraChanged = camera != _lastCameraPosition;
	bool directionChanged = direction != _lastCameraDirection || roll != _lastCameraRoll;
	bool projectionChanged = fov != _lastFov || aspectRatio != _lastAspectRatio || nearClipPlane != _lastNearClipPlane || farClipPlane != _lastFarClipPlane;
	bool sort = runAnimation || cameraChanged || directionChanged || projectionChanged || _sortedObjects.empty();

	assert(_drawableObjects.size() > 0);
	if (sort)
		_sortedObjects.clear();

	Object *floor = nullptr;

	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible()) {
			if (!gfx->_debugHighlightObjectIDs.empty()) {
				bool found = false;
				for (auto id : gfx->_debugHighlightObjectIDs) {
					if (obj->getObjectID() == id) {
						found = true;
						break;
					}
				}
				// if this object is not in our list, skip it completely.
				// it will not be sorted, and it will not be drawn.
				if (!found)
					continue;
			}
			if (obj->getObjectID() == 0 && _groundColor < 255 && _skyColor < 255) {
				floor = obj;
				continue;
			}

			if (obj->getType() == ObjectType::kGroupType) {
				drawGroup(gfx, (Group *)obj, runAnimation && !insideWait);
				continue;
			}

			if (sort && objectIsSortCandidate(obj, camera, direction, roll, fov, aspectRatio, nearClipPlane, farClipPlane))
				_sortedObjects.push_back(obj);
		}
	}

	if (floor) {
		floor->draw(gfx);
	}

	if (sort)
		sortObjectsForRendering(_sortedObjects, camera);

	for (auto &obj : _sortedObjects) {
		obj->draw(gfx);

		// draw bounding boxes
		if (gfx->_debugRenderBoundingBoxes)
			gfx->drawAABB(obj->_boundingBox, 0, 255, 0);
		if (gfx->_debugRenderOcclusionBoxes)
			gfx->drawAABB(obj->_occlusionBox, 255, 0, 0);
	}
	_lastTick = animationTicks;
	if (sort) {
		_lastCameraPosition = camera;
		_lastCameraDirection = direction;
		_lastCameraRoll = roll;
		_lastFov = fov;
		_lastAspectRatio = aspectRatio;
		_lastNearClipPlane = nearClipPlane;
		_lastFarClipPlane = farClipPlane;
	}
}

void Area::drawDepthLayer(Freescape::Renderer *gfx, uint32 animationTicks, Math::Vector3d camera, Math::Vector3d direction, float roll, bool insideWait, RenderDepthLayer depthLayer, float foregroundDistance, float fov, float aspectRatio, float nearClipPlane, float farClipPlane) {
	bool runAnimation = depthLayer != kRenderDepthBackground && animationTicks != _lastDepthLayerTick;
	bool cameraChanged = camera != _lastDepthLayerCameraPosition;
	bool directionChanged = direction != _lastDepthLayerCameraDirection || roll != _lastDepthLayerCameraRoll;
	bool projectionChanged = fov != _lastDepthLayerFov || aspectRatio != _lastDepthLayerAspectRatio || nearClipPlane != _lastDepthLayerNearClipPlane || farClipPlane != _lastDepthLayerFarClipPlane;
	bool layerChanged = depthLayer != _lastRenderDepthLayer || (depthLayer != kRenderDepthAll && ABS(foregroundDistance - _lastForegroundDistance) > 0.001f);
	bool sort = runAnimation || cameraChanged || directionChanged || projectionChanged || layerChanged || _depthLayerSortedObjects.empty();
	Math::Vector3d normalizedDirection = direction.getNormalized();

	assert(_drawableObjects.size() > 0);
	if (sort)
		_depthLayerSortedObjects.clear();

	Object *floor = nullptr;

	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible()) {
			if (!gfx->_debugHighlightObjectIDs.empty()) {
				bool found = false;
				for (auto id : gfx->_debugHighlightObjectIDs) {
					if (obj->getObjectID() == id) {
						found = true;
						break;
					}
				}
				// if this object is not in our list, skip it completely.
				// it will not be sorted, and it will not be drawn.
				if (!found)
					continue;
			}
			if (obj->getObjectID() == 0 && _groundColor < 255 && _skyColor < 255) {
				if (depthLayer != kRenderDepthForeground)
					floor = obj;
				continue;
			}

			if (obj->getType() == ObjectType::kGroupType) {
				if (objectInDepthLayer(obj, camera, normalizedDirection, depthLayer, foregroundDistance))
					drawGroup(gfx, (Group *)obj, runAnimation && !insideWait);
				continue;
			}

			if (sort &&
					objectInDepthLayer(obj, camera, normalizedDirection, depthLayer, foregroundDistance) &&
					objectIsSortCandidate(obj, camera, direction, roll, fov, aspectRatio, nearClipPlane, farClipPlane))
				_depthLayerSortedObjects.push_back(obj);
		}
	}

	if (floor) {
		floor->draw(gfx);
	}

	if (sort)
		sortObjectsForRendering(_depthLayerSortedObjects, camera);

	for (auto &obj : _depthLayerSortedObjects) {
		obj->draw(gfx);

		// draw bounding boxes
		if (gfx->_debugRenderBoundingBoxes)
			gfx->drawAABB(obj->_boundingBox, 0, 255, 0);
		if (gfx->_debugRenderOcclusionBoxes)
			gfx->drawAABB(obj->_occlusionBox, 255, 0, 0);
	}
	if (depthLayer != kRenderDepthBackground)
		_lastDepthLayerTick = animationTicks;
	if (sort) {
		_lastDepthLayerCameraPosition = camera;
		_lastDepthLayerCameraDirection = direction;
		_lastDepthLayerCameraRoll = roll;
		_lastDepthLayerFov = fov;
		_lastDepthLayerAspectRatio = aspectRatio;
		_lastDepthLayerNearClipPlane = nearClipPlane;
		_lastDepthLayerFarClipPlane = farClipPlane;
		_lastRenderDepthLayer = depthLayer;
		_lastForegroundDistance = foregroundDistance;
	}
}

void Area::drawGroup(Freescape::Renderer *gfx, Group* group, bool runAnimation) {
	if (runAnimation) {
		group->run();
		group->draw(gfx);
		group->step();
	} else
		group->draw(gfx);
}

bool Area::hasActiveGroups() {
	for (auto &obj : _drawableObjects) {
		if (obj->getType() == kGroupType) {
			Group *group = (Group *)obj;
			if (group->isActive())
				return true;
		}
	}
	return false;
}

Object *Area::checkCollisionRay(const Math::Ray &ray, int raySize, bool skipTransparent) {
	float distance = 1.0;
	float size = 16.0 * 8192.0; // TODO: check if this is the max size
	Math::AABB boundingBox(ray.getOrigin(), ray.getOrigin());
	Object *collided = nullptr;
	for (auto &obj : _drawableObjects) {
		if (obj->getType() == kLineType) {
			// If the line is not along an axis, the AABB is wildly inaccurate so we skip it
			if (((GeometricObject *)obj)->isLineButNotStraight())
				continue;
		}

		if (!obj->isDestroyed() && !obj->isInvisible() && obj->isGeometric()) {
			GeometricObject *gobj = (GeometricObject *)obj;
			if (skipTransparent && gobj->isFullyTransparent())
				continue;

			Math::Vector3d collidedNormal;
			float collidedDistance = sweepAABB(boundingBox, gobj->_boundingBox, raySize * ray.getDirection(), collidedNormal);
			debugC(1, kFreescapeDebugMove, "reached obj id: %d with distance %f", obj->getObjectID(), collidedDistance);
			if (collidedDistance >= 1.0)
				continue;

			if (collidedDistance == 0.0 && signbit(collidedDistance))
				continue;

			if (collidedDistance < distance || (ABS(collidedDistance - distance) <= 0.05 && gobj->getSize().length() < size)) {
				collided = obj;
				size = gobj->getSize().length();
				distance = collidedDistance;
			}
		}
	}
	return collided;
}

ObjectArray Area::checkCollisions(const Math::AABB &boundingBox) {
	ObjectArray collided;
	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible() && obj->isGeometric()) {
			GeometricObject *gobj = (GeometricObject *)obj;
			if (gobj->collides(boundingBox)) {
				collided.push_back(gobj);
			}
		}
	}
	return collided;
}

bool Area::checkIfPlayerWasCrushed(const Math::AABB &boundingBox) {
	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible() && obj->getType() == kGroupType) {
			Group *group = (Group *)obj;
			if (group->collides(boundingBox)) {
				return true;
			}
		}
	}
	return false;
}

Math::Vector3d Area::separateFromWall(const Math::Vector3d &_position) {
	Math::Vector3d position = _position;
	float sep = 8 / _scale;
	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible() && obj->isGeometric()) {
			GeometricObject *gobj = (GeometricObject *)obj;
			Math::Vector3d distance = gobj->_boundingBox.distance(position);
			if (distance.length() > 0.0001)
				continue;

			position.z() = position.z() + sep;
			distance = gobj->_boundingBox.distance(position);
			if (distance.length() > 0.0001)
				return position;

			position = _position;
			position.z() = position.z() - sep;
			distance = gobj->_boundingBox.distance(position);
			if (distance.length() > 0.0001)
				return position;

			position = _position;
			position.x() = position.x() + sep;
			distance = gobj->_boundingBox.distance(position);
			if (distance.length() > 0.0001)
				return position;

			position = _position;
			position.x() = position.x() - sep;
			distance = gobj->_boundingBox.distance(position);
			if (distance.length() > 0.0001)
				return position;
		}
	}
	return position;
}

// Render-only: nudge the eye at least `separation` away from wall sides so they never cross the near plane.
Math::Vector3d Area::separateCameraFromWall(const Math::Vector3d &eye, float separation) {
	Math::Vector3d cam = eye;
	for (int pass = 0; pass < 4; pass++) { // corners: leaving one wall's band can enter another's
		bool adjusted = false;
		for (auto &obj : _drawableObjects) {
			if (obj->isDestroyed() || obj->isInvisible() || !obj->isGeometric())
				continue;
			const Math::AABB &box = ((GeometricObject *)obj)->_boundingBox;
			if (!box.isValid())
				continue;
			Math::Vector3d mn = box.getMin();
			Math::Vector3d mx = box.getMax();
			if (cam.y() <= mn.y() || cam.y() >= mx.y()) // only walls at eye level can clip the view
				continue;

			float dx = cam.x() - CLIP<float>(cam.x(), mn.x(), mx.x());
			float dz = cam.z() - CLIP<float>(cam.z(), mn.z(), mx.z());
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist >= separation)
				continue;

			if (dist > 0.0001f) { // in the band: push straight out
				cam.x() += dx / dist * (separation - dist);
				cam.z() += dz / dist * (separation - dist);
			} else { // inside the footprint: eject through the nearest side
				float left = cam.x() - mn.x(), right = mx.x() - cam.x();
				float back = cam.z() - mn.z(), front = mx.z() - cam.z();
				if (MIN(left, right) <= MIN(back, front))
					cam.x() += (left < right) ? -(left + separation) : (right + separation);
				else
					cam.z() += (back < front) ? -(back + separation) : (front + separation);
			}
			adjusted = true;
		}
		if (!adjusted)
			break;
	}
	return cam;
}

Math::Vector3d Area::resolveCollisions(const Math::Vector3d &lastPosition_, const Math::Vector3d &newPosition_, int playerHeight) {
	Math::Vector3d position = newPosition_;
	Math::Vector3d lastPosition = lastPosition_;

	float reductionHeight = 0.0;
	// Ugly hack to fix the collisions in tight spaces in the stores and junk room
	// for Castle Master
	if (_isCastle && _areaID == 62) {
		reductionHeight = 0.3f;
	} else if (_isCastle && _areaID == 61) {
		reductionHeight = 0.3f;
	}

	Math::AABB boundingBox = createPlayerAABB(lastPosition, playerHeight, reductionHeight);

	float epsilon = 1.5;
	int i = 0;
	while (true) {
		float distance = 1.0;
		Math::Vector3d normal;
		Math::Vector3d direction = position - lastPosition;

		for (auto &obj : _drawableObjects) {
			if (!obj->isDestroyed() && !obj->isInvisible() && obj->isGeometric()) {
				GeometricObject *gobj = (GeometricObject *)obj;
				Math::Vector3d collidedNormal;
				float collidedDistance = sweepAABB(boundingBox, gobj->_boundingBox, direction, collidedNormal);
				if (collidedDistance < distance) {
					distance = collidedDistance;
					normal = collidedNormal;
				}
			}
		}
		position = lastPosition + distance * direction + epsilon * normal;
		if (i > 1 || distance >= 1.0)
			break;
		i++;
	}
	return position;
}

bool Area::checkInSight(const Math::Ray &ray, float maxDistance) {
	Math::Vector3d direction = ray.getDirection();
	direction.normalize();
	GeometricObject point(kCubeType,
			0,
			0,
			Math::Vector3d(0, 0, 0),
			Math::Vector3d(maxDistance / 30, maxDistance / 30, maxDistance / 30), // size
			nullptr,
			nullptr,
			nullptr,
			FCLInstructionVector(),
			"");

	for (int distanceMultiplier = 2; distanceMultiplier <= 10; distanceMultiplier++) {
		Math::Vector3d origin = ray.getOrigin() + distanceMultiplier * (maxDistance / 10) * direction;
		point.setOrigin(origin);

		for (auto &obj : _drawableObjects) {
			if (obj->getType() != kSensorType && !obj->isDestroyed() && !obj->isInvisible() && obj->_boundingBox.isValid() && point.collides(obj->_boundingBox)) {
				return false;
			}
		}
	}

	return true;
}

void Area::addObject(Object *obj) {
	assert(obj);
	int id = obj->getObjectID();
	debugC(1, kFreescapeDebugParser, "Adding object %d to room %d", id, _areaID);
	assert(!_objectsByID->contains(id));
	(*_objectsByID)[id] = obj;
	if (obj->isDrawable())
		_drawableObjects.insert_at(0, obj);

	_addedObjects[id] = obj;
}

void Area::removeObject(int16 id) {
	assert(_objectsByID->contains(id));
	for (uint i = 0; i < _drawableObjects.size(); i++) {
		if (_drawableObjects[i]->getObjectID() == id) {
			_drawableObjects.remove_at(i);
			break;
		}
	}
	_objectsByID->erase(id);
	_addedObjects.erase(id);
}

Common::List<int> Area::getEntranceIds() {
	Common::List<int> ids;
	for (auto &it : *_entrancesByID) {
		ids.push_back(it._key);
	}
	return ids;
}

void Area::addObjectFromArea(int16 id, Area *global) {
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure in area %d", id, _areaID);
	Object *obj = global->objectWithID(id);
	if (!obj) {
		assert(global->entranceWithID(id));
		obj = global->entranceWithID(id);
		obj = obj->duplicate();
		obj->scale(_scale);
		_addedObjects[id] = obj;
		(*_entrancesByID)[id] = obj;
	} else {
		obj = obj->duplicate();
		obj->scale(_scale);
		(*_objectsByID)[id] = obj;
		_addedObjects[id] = obj;
		if (obj->isDrawable()) {
			_drawableObjects.insert_at(0, obj);
		}
	}
}

void Area::addGroupFromArea(int16 id, Area *global) {
	debugC(1, kFreescapeDebugParser, "Adding group %d to room structure in area %d", id, _areaID);
	Object *obj = global->objectWithID(id);
	assert(obj);
	assert(obj->getType() == ObjectType::kGroupType);

	addObjectFromArea(id, global);
	Group *group = (Group *)objectWithID(id);
	for (auto &it : ((Group *)obj)->_objectIds) {
		if (it == 0 || it == 0xffff)
			break;
		if (!global->objectWithID(it))
			continue;

		if (!objectWithID(it))
			addObjectFromArea(it, global);
		group->linkObject(objectWithID(it));
	}
}


void Area::addFloor() {
	_hasSyntheticFloor = true;
	int id = 0;
	assert(!_objectsByID->contains(id));
	Common::Array<uint8> *gColors = new Common::Array<uint8>;
	for (int i = 0; i < 6; i++)
		gColors->push_back(_groundColor);

	int maxSize = 10000000 / 4;
	Object *obj = (Object *)new GeometricObject(
		ObjectType::kCubeType,
		id,
		0,                                           // flags
		Math::Vector3d(-maxSize, -3, -maxSize),      // Position
		Math::Vector3d(maxSize * 4, 3, maxSize * 4), // size
		gColors,
		nullptr,
		nullptr,
		FCLInstructionVector());
	(*_objectsByID)[id] = obj;
	_drawableObjects.insert_at(0, obj);
}

void Area::addStructure(Area *global) {
	if (!global || !_entrancesByID->contains(255)) {
		return;
	}
	GlobalStructure *rs = (GlobalStructure *)(*_entrancesByID)[255];

	for (uint i = 0; i < rs->_structure.size(); i++) {
		int16 id = rs->_structure[i];
		if (id == 0)
			continue;

		addObjectFromArea(id, global);
	}
}

void Area::changeObjectID(uint16 objectID, uint16 newObjectID) {
	assert(!objectWithID(newObjectID));
	Object *obj = objectWithID(objectID);
	assert(obj);
	obj->_objectID = newObjectID;
	_addedObjects.erase(objectID);
	_addedObjects[newObjectID] = obj;

	(*_objectsByID).erase(objectID);
	(*_objectsByID)[newObjectID] = obj;
}


bool Area::isOutside() {
	// Castle outdoor areas are exactly the ones that get the synthetic floor (Wilderness and Courtyard).
	if (_isCastle)
		return _hasSyntheticFloor;
	return _skyColor < 255 && _groundColor < 255;
}

} // End of namespace Freescape
