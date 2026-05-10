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

#include "mediastation/mediastation.h"
#include "mediastation/actors/camera.h"
#include "mediastation/actors/stage.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

StageActor::StageActor() : SpatialEntity(kActorTypeStage),
	_children(StageActor::compareSpatialActorByZIndex),
	_cameras(StageActor::compareSpatialActorByZIndex) {
}

StageActor::~StageActor() {
	removeAllChildren();
	if (_parentStage != nullptr) {
		_parentStage->removeChildSpatialEntity(this);
	}
	_currentCamera = nullptr;
}

void StageActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderChildActorId: {
		// In stages, this basically has the oppose meaning it has outside of stages. Here,
		// it specifies an actor that is a parent of this stage.
		uint parentActorId = chunk.readTypedUint16();
		_pendingParent = static_cast<SpatialEntity *>(g_engine->getImtGod()->getActorByIdAndType(parentActorId, kActorTypeStage));
		break;
	}

	case kActorHeaderStageExtent:
		_extent = chunk.readTypedGraphicSize();
		break;

	case kActorHeaderCylindricalX:
		_cylindricalX = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderCylindricalY:
		_cylindricalY = static_cast<bool>(chunk.readTypedByte());
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

void StageActor::preload(const Common::Rect &rect, bool fireStepEvent) {
	if (cylindricalX()) {
		preloadTest(rect, kWrapLeft, fireStepEvent);
	}
	if (cylindricalY()) {
		preloadTest(rect, kWrapUp, fireStepEvent);
	}
	if (cylindricalX() && cylindricalY()) {
		preloadTest(rect, kWrapLeftUp, fireStepEvent);
	}
	preloadTest(rect, kWrapNone, fireStepEvent);
}

void StageActor::preloadTest(const Common::Rect &rect, CylindricalWrapMode wrapMode, bool fireStepEvent) {
	for (SpatialEntity *entity : _children) {
		entity->setAdjustedBounds(wrapMode);
		if (!entity->isRectInMemory(rect) && !entity->isLoading()) {
			entity->preload(rect, fireStepEvent);
		}
	}
}

bool StageActor::inBounds(const Common::Point &point, const Common::Rect &bounds, const Polygon &polygon) {
	if (_cameras.empty()) {
		return inBoundsTest(point, bounds, polygon);
	} else {
		return inBoundsCamera(point, bounds, polygon);
	}
}

bool StageActor::inBoundsTest(Common::Point point, const Common::Rect &bounds, const Polygon &polygon) {
	if (bounds.contains(point)) {
		point -= bounds.origin();
		return polygon.containsPoint(point);
	} else {
		return false;
	}
}

bool StageActor::inBoundsCamera(const Common::Point &point, const Common::Rect &bounds, const Polygon &polygon) {
	for (uint i = 0; i < _cameras.size(); i++) {
		(void)i; // Silence the unused variable warning. The original loop doesn't use this index.
		bool isInCameraBounds = inBoundsObject(point, bounds, polygon);
		if (isInCameraBounds) {
			return true;
		}
	}
	return false;
}

bool StageActor::inBoundsObject(const Common::Point &point, const Common::Rect &bounds, const Polygon &polygon) {
	bool isInCameraBounds = inBoundsTest(point, bounds, polygon);

	if (!isInCameraBounds && cylindricalX()) {
		Common::Rect adjustedBounds = bounds;
		adjustedBounds.translate(-_extent.x, 0);
		isInCameraBounds = inBoundsTest(point, adjustedBounds, polygon);

		if (!isInCameraBounds) {
			adjustedBounds = bounds;
			adjustedBounds.translate(_extent.x, 0);
			isInCameraBounds = inBoundsTest(point, adjustedBounds, polygon);
		}
	}

	if (!isInCameraBounds && cylindricalY()) {
		Common::Rect adjustedBounds = bounds;
		adjustedBounds.translate(0, -_extent.y);
		isInCameraBounds = inBoundsTest(point, adjustedBounds, polygon);

		if (!isInCameraBounds) {
			adjustedBounds = bounds;
			adjustedBounds.translate(0, _extent.y);
			isInCameraBounds = inBoundsTest(point, adjustedBounds, polygon);
		}
	}

	if (!isInCameraBounds && cylindricalX() && cylindricalY()) {
		Common::Rect adjustedBounds = bounds;
		adjustedBounds.translate(_extent.x, _extent.y);
		isInCameraBounds = inBoundsTest(point, adjustedBounds, polygon);

		if (!isInCameraBounds) {
			adjustedBounds = bounds;
			adjustedBounds.translate(-_extent.x, -_extent.y);
			isInCameraBounds = inBoundsTest(point, adjustedBounds, polygon);

			if (!isInCameraBounds) {
				adjustedBounds = bounds;
				adjustedBounds.translate(-_extent.x, _extent.y);
				isInCameraBounds = inBoundsTest(point, adjustedBounds, polygon);

				if (!isInCameraBounds) {
					adjustedBounds = bounds;
					adjustedBounds.translate(_extent.x, -_extent.y);
					isInCameraBounds = inBoundsTest(point, adjustedBounds, polygon);
				}
			}
		}
	}

	return isInCameraBounds;
}

bool StageActor::isRectInMemory(const Common::Rect &rect) {
	bool result = true;
	if (cylindricalX()) {
		result = isRectInMemoryTest(rect, kWrapLeft);
	}
	if (result && cylindricalY()) {
		result = isRectInMemoryTest(rect, kWrapUp);
	}
	if (result && cylindricalY() && cylindricalX()) {
		result = isRectInMemoryTest(rect, kWrapLeftUp);
	}
	if (result) {
		result = isRectInMemoryTest(rect, kWrapNone);
	}
	return result;
}

bool StageActor::isRectInMemoryTest(const Common::Rect &rect, CylindricalWrapMode wrapMode) {
	for (SpatialEntity *entity : _children) {
		entity->setAdjustedBounds(wrapMode);
		if (entity->isVisible() && !entity->isRectInMemory(rect)) {
			return false;
		}
	}
	return true;
}

void StageActor::draw(DisplayContext &displayContext) {
	Clip *currentClip = displayContext.currentClip();
	if (currentClip != nullptr) {
		Clip *previousClip = displayContext.previousClip();
		if (previousClip == nullptr) {
			currentClip->addToRegion(currentClip->_bounds);
		} else {
			currentClip = previousClip;
		}
	}

	displayContext.intersectClipWith(getBbox());
	if (displayContext.clipIsEmpty()) {
		return;
	}

	bool boundsNeedsAdjustment = false;
	Common::Rect bounds = getBbox();
	if (bounds.left != 0 || bounds.top != 0) {
		boundsNeedsAdjustment = true;
	}
	if (boundsNeedsAdjustment) {
		displayContext.pushOrigin();
		displayContext._origin.x = bounds.left + displayContext._origin.x;
		displayContext._origin.y = bounds.top + displayContext._origin.y;
	}

	if (_cameras.empty()) {
		drawUsingStage(displayContext);
	} else {
		for (CameraActor *camera : _cameras) {
			setCurrentCamera(camera);
			camera->drawUsingCamera(displayContext, _children);
		}
	}

	if (boundsNeedsAdjustment) {
		displayContext.popOrigin();
	}
	displayContext.emptyCurrentClip();
}

void StageActor::drawUsingStage(DisplayContext &displayContext) {
	for (SpatialEntity *entity : _children) {
		entity->setAdjustedBounds(kWrapNone);
		if (entity->isVisible()) {
			if (displayContext.rectIsInClip(entity->getBbox())) {
				debugC(8, kDebugGraphics, "[%s] %s: Redrawing actor %s (%d, %d, %d, %d)", debugName(), __func__,
					entity->debugName(), PRINT_RECT(entity->getBbox()));
				entity->draw(displayContext);
			}
		}
	}
}

void StageActor::invalidateRect(const Common::Rect &rect) {
	Common::Point origin = _boundingBox.origin();
	Common::Rect rectRelativeToParent = rect;
	rectRelativeToParent.translate(origin.x, origin.y);

	if (_parentStage != nullptr) {
		debugC(8, kDebugGraphics, "[%s] %s: (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
				debugName(), __func__, PRINT_RECT(rect), PRINT_RECT(rectRelativeToParent));

		if (_cameras.size() == 0) {
			_parentStage->invalidateRect(rectRelativeToParent);
		} else {
			invalidateUsingCameras(rectRelativeToParent);
		}
	} else {
		debugC(5, kDebugGraphics, "[%s] %s (%d, %d, %d, %d): Attempt to invalidate rect (%d, %d, %d, %d) without a parent stage",
			debugName(), __func__, PRINT_RECT(_boundingBox), PRINT_RECT(rectRelativeToParent));
	}
}

void StageActor::invalidateUsingCameras(const Common::Rect &rect) {
	for (CameraActor *camera : _cameras) {
		Common::Rect adjustedRectToInvalidate = rect;
		Common::Rect cameraBounds = camera->getBbox();

		Common::Rect cameraBoundsInStageCoordinates = cameraBounds;
		Common::Rect stageOrigin = getBbox();
		cameraBoundsInStageCoordinates.translate(stageOrigin.left, stageOrigin.top);

		Common::Point cameraViewportOrigin = camera->getViewportOrigin();
		Common::Point viewportOffsetFromCameraBounds(
			cameraViewportOrigin.x - cameraBounds.left,
			cameraViewportOrigin.y - cameraBounds.top
		);

		adjustedRectToInvalidate.translate(
			-viewportOffsetFromCameraBounds.x,
			-viewportOffsetFromCameraBounds.y
		);
		invalidateObject(adjustedRectToInvalidate, cameraBoundsInStageCoordinates);
	}
}

void StageActor::invalidateObject(const Common::Rect &rect, const Common::Rect &visibleRegion) {
	Common::Point xyAdjustment(0, 0);
	invalidateTest(rect, visibleRegion, xyAdjustment);

	if (_cylindricalX) {
		xyAdjustment.x = _extent.x;
		xyAdjustment.y = 0;
		invalidateTest(rect, visibleRegion, xyAdjustment);

		xyAdjustment.x = -_extent.x;
		xyAdjustment.y = 0;
		invalidateTest(rect, visibleRegion, xyAdjustment);
	}

	if (_cylindricalY) {
		xyAdjustment.x = 0;
		xyAdjustment.y = _extent.y;
		invalidateTest(rect, visibleRegion, xyAdjustment);

		xyAdjustment.x = 0;
		xyAdjustment.y = -_extent.y;
		invalidateTest(rect, visibleRegion, xyAdjustment);
	}

	if (_cylindricalX && _cylindricalY) {
		xyAdjustment.x = _extent.x;
		xyAdjustment.y = _extent.y;
		invalidateTest(rect, visibleRegion, xyAdjustment);

		xyAdjustment.x = -_extent.x;
		xyAdjustment.y = -_extent.y;
		invalidateTest(rect, visibleRegion, xyAdjustment);

		xyAdjustment.x = -_extent.x;
		xyAdjustment.y = _extent.y;
		invalidateTest(rect, visibleRegion, xyAdjustment);

		xyAdjustment.x = _extent.x;
		xyAdjustment.y = -_extent.y;
		invalidateTest(rect, visibleRegion, xyAdjustment);
	}
}

void StageActor::invalidateTest(const Common::Rect &rect, const Common::Rect &visibleRegion, const Common::Point &originAdjustment) {
	Common::Rect rectToInvalidateRelative = rect;
	rectToInvalidateRelative.translate(-originAdjustment.x, -originAdjustment.y);
	rectToInvalidateRelative.clip(visibleRegion);
	_parentStage->invalidateRect(rectToInvalidateRelative);
}

void StageActor::loadIsComplete() {
	// This is deliberately calling down to Actor, rather than calling
	// to SpatialEntity first.
	Actor::loadIsComplete();

	if (_pendingParent != nullptr) {
		StageActor *parentStage = static_cast<StageActor *>(_pendingParent);
		parentStage->addChildSpatialEntity(this);
		_pendingParent = nullptr;
	}

	if (_extent.x == 0 || _extent.y == 0) {
		_extent.x = _boundingBox.width();
		_extent.y = _boundingBox.height();
	}
}

void StageActor::addActorToStage(uint actorId) {
	// If actor has a current parent, remove it from that parent first.
	SpatialEntity *spatialEntity = g_engine->getImtGod()->getSpatialEntityById(actorId);
	StageActor *currentParent = spatialEntity->getParentStage();
	if (currentParent != nullptr) {
		currentParent->removeChildSpatialEntity(spatialEntity);
	}
	addChildSpatialEntity(spatialEntity);
}

void StageActor::removeActorFromStage(uint actorId) {
	SpatialEntity *spatialEntity = g_engine->getImtGod()->getSpatialEntityById(actorId);
	StageActor *currentParent = spatialEntity->getParentStage();
	if (currentParent == this) {
		// Remove the actor from this stage, and add it back to the root stage.
		removeChildSpatialEntity(spatialEntity);
		RootStage *rootStage = g_engine->getRootStage();
		rootStage->addChildSpatialEntity(spatialEntity);
	}
}

void StageActor::addCamera(CameraActor *camera) {
	_cameras.insert(camera);
}

void StageActor::removeCamera(CameraActor *camera) {
	for (auto it = _cameras.begin(); it != _cameras.end(); ++it) {
		if (*it == camera) {
			_cameras.erase(it);
			break;
		}
	}
}

void StageActor::setCurrentCamera(CameraActor *camera) {
	_currentCamera = camera;
}

ScriptValue StageActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kAddActorToStageMethod:
	case kAddActorToStageMethod2: {
		ARGCOUNTCHECK(1);
		uint actorId = args[0].asActorId();
		addActorToStage(actorId);
		break;
	}

	case kRemoveActorFromStageMethod:
	case kRemoveActorFromStageMethod2: {
		ARGCOUNTCHECK(1);
		uint actorId = args[0].asActorId();
		removeActorFromStage(actorId);
		break;
	}

	case kSetBoundsMethod: {
		ARGCOUNTCHECK(4);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		int16 width = static_cast<int16>(args[2].asFloat());
		int16 height = static_cast<int16>(args[3].asFloat());
		Common::Rect newBounds(Common::Point(x, y), width, height);
		setBounds(newBounds);
		break;
	}

	case kStageSetSizeMethod:
		ARGCOUNTCHECK(2);
		_boundingBox.setWidth(static_cast<int16>(args[0].asFloat()));
		_boundingBox.setHeight(static_cast<int16>(args[1].asFloat()));
		break;

	case kStageGetWidthMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_boundingBox.width());
		break;

	case kStageGetHeightMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_boundingBox.height());
		break;

	default:
		returnValue = SpatialEntity::callMethod(methodId, args);
	}
	return returnValue;
}

void StageActor::addChildSpatialEntity(SpatialEntity *entity) {
	if (!assertHasNoParent(entity)) {
		error("[%s] %s: Attempt to add entity that already has a parent", debugName(), __func__);
	}

	entity->setParentStage(this);
	_children.insert(entity);

	if (isVisible()) {
		invalidateLocalBounds();
	}
}

void StageActor::removeChildSpatialEntity(SpatialEntity *entity) {
	if (!assertIsMyChild(entity)) {
		warning("[%s] %s: Attempt to remove entity that is not a child", debugName(), __func__);
	}

	if (isVisible()) {
		invalidateLocalBounds();
	}

	entity->setToNoParentStage();
	for (auto it = _children.begin(); it != _children.end(); ++it) {
		if (*it == entity) {
			_children.erase(it);
			break;
		}
	}
}

uint16 StageActor::queryChildrenAboutMouseEvents(
	const Common::Point &point, uint16 eventMask, MouseActorState &state, bool clipMouseEvents, CylindricalWrapMode wrapMode) {

	uint16 result = 0;
	for (auto childIterator = _children.end(); childIterator != _children.begin();) {
		--childIterator; // Decrement first, then dereference
		SpatialEntity *child = *childIterator;
		debugC(8, kDebugEvents, "  [%s] %s: Checking %s (mousePos: %d, %d) (bounds: %d, %d, %d, %d) (z-index: %d) (eventMask: 0x%02x) (result: 0x%02x) (wrapMode: %d)",
			debugName(), __func__, child->debugName(), point.x, point.y,
			PRINT_RECT(child->getBbox()), child->zIndex(), eventMask, result, wrapMode);

		child->setAdjustedBounds(wrapMode);
		uint16 handledEvents = child->findActorToAcceptMouseEvents(point, eventMask, state, clipMouseEvents);
		child->setAdjustedBounds(kWrapNone);

		eventMask &= ~handledEvents;
		result |= handledEvents;
		if (eventMask == 0) {
			break;
		}
	}
	return result;
}

uint16 StageActor::findActorToAcceptMouseEventsObject(
	const Common::Point &point, uint16 eventMask, MouseActorState &state, bool clipMouseEvents) {

	uint16 result = 0;
	uint16 handledEvents = queryChildrenAboutMouseEvents(point, eventMask, state, clipMouseEvents, kWrapNone);
	if (handledEvents != 0) {
		eventMask &= ~handledEvents;
		result |= handledEvents;
	}

	if ((eventMask != 0) && cylindricalX()) {
		handledEvents = queryChildrenAboutMouseEvents(point, eventMask, state, clipMouseEvents, kWrapLeft);
		if (handledEvents != 0) {
			eventMask &= ~handledEvents;
			result |= handledEvents;
		}
	}

	if ((eventMask != 0) && cylindricalX()) {
		handledEvents = queryChildrenAboutMouseEvents(point, eventMask, state, clipMouseEvents, kWrapRight);
		if (handledEvents != 0) {
			eventMask &= ~handledEvents;
			result |= handledEvents;
		}
	}

	if ((eventMask != 0) && cylindricalY()) {
		handledEvents = queryChildrenAboutMouseEvents(point, eventMask, state, clipMouseEvents, kWrapUp);
		if (handledEvents != 0) {
			eventMask &= ~handledEvents;
			result |= handledEvents;
		}
	}

	if ((eventMask != 0) && cylindricalY()) {
		handledEvents = queryChildrenAboutMouseEvents(point, eventMask, state, clipMouseEvents, kWrapDown);
		if (handledEvents != 0) {
			eventMask &= ~handledEvents;
			result |= handledEvents;
		}
	}

	if ((eventMask != 0) && cylindricalY() && cylindricalX()) {
		handledEvents = queryChildrenAboutMouseEvents(point, eventMask, state, clipMouseEvents, kWrapLeftUp);
		if (handledEvents != 0) {
			eventMask &= ~handledEvents;
			result |= handledEvents;
		}
	}

	if ((eventMask != 0) && cylindricalY() && cylindricalX()) {
		handledEvents = queryChildrenAboutMouseEvents(point, eventMask, state, clipMouseEvents, kWrapRightDown);
		if (handledEvents != 0) {
			result |= handledEvents;
		}
	}

	return result;
}

uint16 StageActor::findActorToAcceptMouseEventsCamera(
	const Common::Point &point, uint16 eventMask, MouseActorState &state, bool clipMouseEvents) {

	uint16 result = 0;
	for (auto cameraIterator = _cameras.end(); cameraIterator != _cameras.begin();) {
		--cameraIterator; // Decrement first, then dereference
		CameraActor *camera = *cameraIterator;
		Common::Point mousePosRelativeToCamera = point + camera->getViewportOrigin();
		setCurrentCamera(camera);

		if (!clipMouseEvents) {
			Common::Rect viewportBounds = camera->getViewportBounds();
			if (!viewportBounds.contains(mousePosRelativeToCamera)) {
				clipMouseEvents = true;
			}
		}

		result = findActorToAcceptMouseEventsObject(mousePosRelativeToCamera, eventMask, state, clipMouseEvents);
	}

	return result;
}

uint16 StageActor::findActorToAcceptMouseEvents(
	const Common::Point &point, uint16 eventMask, MouseActorState &state, bool clipMouseEvents) {

	Common::Point mousePosRelativeToStageOrigin = point - _boundingBox.origin();
	debugC(8, kDebugEvents, "[%s] %s: mousePos: (%d, %d), relativeToStage: (%d, %d)", debugName(), __func__,
		point.x, point.y, mousePosRelativeToStageOrigin.x, mousePosRelativeToStageOrigin.y);

	uint16 result;
	if (_cameras.empty()) {
		if (!clipMouseEvents) {
			if (!_boundingBox.contains(point)) {
				clipMouseEvents = true;
			}
		}
		result = queryChildrenAboutMouseEvents(mousePosRelativeToStageOrigin, eventMask, state, clipMouseEvents, kWrapNone);
	} else {
		result = findActorToAcceptMouseEventsCamera(mousePosRelativeToStageOrigin, eventMask, state, clipMouseEvents);
	}

	return result;
}

uint16 StageActor::findActorToAcceptKeyboardEvents(
	uint16 asciiCode,
	uint16 eventMask,
	MouseActorState &state) {

	uint16 result = 0;
	for (SpatialEntity *child : _children) {
		uint16 handledEvents = child->findActorToAcceptKeyboardEvents(asciiCode, eventMask, state);
		if (handledEvents != 0) {
			eventMask &= ~handledEvents;
			result |= handledEvents;
		}
	}

	return result;
}

bool StageActor::assertHasNoParent(const SpatialEntity *entity) {
	// Make sure entity is not in our children.
	for (SpatialEntity *child : _children) {
		if (child == entity) {
			return false;
		}
	}

	// Make sure entity doesn't have a parent.
	if (entity->getParentStage() != nullptr) {
		return false;
	}

	return true;
}

void StageActor::removeAllChildren() {
	for (SpatialEntity *child : _children) {
		child->setToNoParentStage();
	}
	_children.clear();
}

void StageActor::setMousePosition(int16 x, int16 y) {
	if (_parentStage != nullptr) {
		x += _boundingBox.left;
		y += _boundingBox.top;
		_parentStage->setMousePosition(x, y);
	}
}

void StageActor::invalidateLocalBounds() {
	for (SpatialEntity *child : _children) {
		if (child->isVisible()) {
			child->invalidateLocalBounds();
		}
	}
}

void StageActor::invalidateLocalZIndex() {
	if (_parentStage != nullptr) {
		_parentStage->invalidateZIndexOf(this);
	}
}

void StageActor::invalidateZIndexOf(const SpatialEntity *entity) {
	if (!assertIsMyChild(entity)) {
		error("[%s] %s: Attempt to invalidate local z-index of non-child", debugName(), __func__);
	}

	// Remove the entity from the sorted array and re-insert it at the correct position.
	for (auto it = _children.begin(); it != _children.end(); ++it) {
		if (*it == entity) {
			_children.erase(it);
			_children.insert(const_cast<SpatialEntity *>(entity));
			break;
		}
	}

	// Mark the whole stage dirty since z-order changed.
	if (isVisible()) {
		invalidateLocalBounds();
	}
}

int StageActor::compareSpatialActorByZIndex(const SpatialEntity *a, const SpatialEntity *b) {
	int diff = b->zIndex() - a->zIndex();
	if (diff < 0)
		return -1; // a should come before b
	else if (diff > 0)
		return 1; // b should come before a
	else {
		// If z-indices are equal, compare pointers for stable sort
		return (a < b) ? -1 : 1;
	}
}

void StageActor::currentMousePosition(Common::Point &point) {
	if (getParentStage() != nullptr) {
		getParentStage()->currentMousePosition(point);
		point -= getBbox().origin();
	}
}

RootStage::RootStage() : StageActor() {
	_id = ROOT_STAGE_ACTOR_ID;
	_debugName = Common::String("Stage ROOT");
}

void RootStage::invalidateRect(const Common::Rect &rect) {
	Common::Rect rectToAdd = rect;
	rectToAdd.clip(_boundingBox);
	_dirtyRegion.addRect(rectToAdd);
}

void RootStage::currentMousePosition(Common::Point &point) {
	point = g_engine->getEventManager()->getMousePos();
	point -= getBbox().origin();
}

void RootStage::drawAll(DisplayContext &displayContext) {
	StageActor::draw(displayContext);
}

void RootStage::drawDirtyRegion(DisplayContext &displayContext) {
	displayContext.setClipTo(_dirtyRegion);
	StageActor::draw(displayContext);
	displayContext.emptyCurrentClip();
}

uint16 RootStage::findActorToAcceptMouseEvents(
	const Common::Point &point, uint16 eventMask, MouseActorState &state, bool clipMouseEvents) {
	// Handle any mouse moved events.
	uint16 result = StageActor::findActorToAcceptMouseEvents(point, eventMask, state, clipMouseEvents);
	eventMask &= ~result;
	if (eventMask & kMouseEnterFlag) {
		state.mouseEnter = this;
		result |= kMouseEnterFlag;
	}
	if (eventMask & kMouseOutOfFocusFlag) {
		state.mouseOutOfFocus = this;
		result |= kMouseOutOfFocusFlag;
	}
	return result;
}

void RootStage::mouseEnteredEvent(const MouseEvent &event) {
	_isMouseInside = true;
	g_engine->getCursorManager()->unsetTemporary();
}

void RootStage::mouseExitedEvent(const MouseEvent &event) {
	_isMouseInside = false;
}

void RootStage::mouseOutOfFocusEvent(const MouseEvent &event) {
	_isMouseInside = true;
	g_engine->getCursorManager()->unsetTemporary();
}

void RootStage::deleteChildrenFromContextId(uint contextId) {
	for (auto it = _children.begin(); it != _children.end();) {
		uint actorContextId = (*it)->contextId();
		if (actorContextId == contextId) {
			it = _children.erase(it);
		} else {
			++it;
		}
	}
}

void RootStage::setMousePosition(int16 x, int16 y) {
	x += _boundingBox.left;
	y += _boundingBox.top;
	warning("[%s] %s: STUB: (%d, %d)", debugName(), __func__, x, y);
}

StageDirector::StageDirector() {
	_rootStage = new RootStage;
	Common::Rect rootStageBounds(MediaStationEngine::SCREEN_WIDTH, MediaStationEngine::SCREEN_HEIGHT);
	_rootStage->setBounds(rootStageBounds);
	g_engine->getImtGod()->addConstructedActor(_rootStage);
}

StageDirector::~StageDirector() {
	g_engine->getImtGod()->destroyActor(RootStage::ROOT_STAGE_ACTOR_ID);
	_rootStage = nullptr;
}

void StageDirector::drawAll() {
	_rootStage->drawAll(g_engine->getDisplayManager()->_displayContext);
}

void StageDirector::drawDirtyRegion() {
	_rootStage->drawDirtyRegion(g_engine->getDisplayManager()->_displayContext);
}

void StageDirector::clearDirtyRegion() {
	_rootStage->_dirtyRegion._rects.clear();
	_rootStage->_dirtyRegion._bounds = Common::Rect(0, 0, 0, 0);
}

void StageDirector::handleMouseEvent(const MouseEvent &event) {
	switch (event.type) {
	case kMouseDownEvent:
		handleMouseDownEvent(event);
		break;

	case kMouseUpEvent:
		handleMouseUpEvent(event);
		break;

	case kMouseMovedEvent:
		handleMouseMovedEvent(event);
		break;

	case kMouseEnterExitEvent:
		handleMouseEnterExitEvent(event);
		break;

	case kMouseOutOfFocusEvent:
		handleMouseOutOfFocusEvent(event);
		break;

	default:
		warning("%s: Unhandled mouse event: %s", __func__, event.debugString().c_str());
	}
}

void StageDirector::handleKeyboardEvent(const KeyboardEvent &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptKeyboardEvents(event.keyCode, kKeyDownFlag, state);
	if (flags & kKeyDownFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching to %s from root stage", __func__, state.keyDown->debugName());
		state.keyDown->keyboardEvent(event);
	}
}

void StageDirector::handleMouseDownEvent(const MouseEvent &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptMouseEvents(event.position, kMouseDownFlag, state, false);
	if (flags & kMouseDownFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching to %s from root stage (mousePos: %d, %d) (bounds: %d, %d, %d, %d)",
			__func__, state.mouseDown->debugName(), event.position.x, event.position.y, PRINT_RECT(state.mouseDown->getBbox()));
		state.mouseDown->mouseDownEvent(event);
	}
}

void StageDirector::handleMouseUpEvent(const MouseEvent &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptMouseEvents(event.position, kMouseUpFlag, state, false);
	if (flags & kMouseUpFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching to %s from root stage (mousePos: %d, %d) (bounds: %d, %d, %d, %d)",
			__func__, state.mouseUp->debugName(), event.position.x, event.position.y, PRINT_RECT(state.mouseUp->getBbox()));
		state.mouseUp->mouseUpEvent(event);
	}
}

void StageDirector::handleMouseMovedEvent(const MouseEvent &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptMouseEvents(
		event.position,
		kMouseEnterFlag | kMouseExitFlag | kMouseMovedFlag,
		state, false);

	sendMouseEnterExitEvent(flags, state, event);
	if (flags & kMouseMovedFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching to %s (mousePos: %d, %d) (bounds: %d, %d, %d, %d)",
			__func__, state.mouseMoved->debugName(), event.position.x, event.position.y, PRINT_RECT(state.mouseMoved->getBbox()));
		state.mouseMoved->mouseMovedEvent(event);
	}
}

void StageDirector::handleMouseEnterExitEvent(const MouseEvent &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptMouseEvents(event.position, kMouseEnterFlag | kMouseExitFlag, state, false);
	sendMouseEnterExitEvent(flags, state, event);
}

void StageDirector::handleMouseOutOfFocusEvent(const MouseEvent &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptMouseEvents(event.position, kMouseExitFlag | kMouseOutOfFocusFlag, state, false);

	if (flags & kMouseExitFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching mouse enter to %s", __func__, state.mouseExit->debugName());
		state.mouseExit->mouseExitedEvent(event);
	}

	if (flags & kMouseOutOfFocusFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching mouse out of focus to %s", __func__, state.mouseOutOfFocus->debugName());
		state.mouseOutOfFocus->mouseOutOfFocusEvent(event);
	}
}

void StageDirector::sendMouseEnterExitEvent(uint16 flags, MouseActorState &state, const MouseEvent &event) {
	if (state.mouseEnter != state.mouseExit) {
		if (flags & kMouseExitFlag) {
			debugC(5, kDebugEvents, "%s: Dispatching mouse exit to %s", __func__, state.mouseExit->debugName());
			state.mouseExit->mouseExitedEvent(event);
		}

		if (flags & kMouseEnterFlag) {
			debugC(5, kDebugEvents, "%s: Dispatching mouse enter to %s", __func__, state.mouseEnter->debugName());
			state.mouseEnter->mouseEnteredEvent(event);
		}
	} else {
		debugC(5, kDebugEvents, "%s: No actor to accept event", __func__);
	}
}

} // End of namespace MediaStation
