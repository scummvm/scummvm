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
#include "mediastation/actors/stage.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

StageActor::StageActor() : SpatialEntity(kActorTypeStage),
	_children(StageActor::compareSpatialActorByZIndex) {
}

StageActor::~StageActor() {
	removeAllChildren();
}

void StageActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderChildActorId: {
		uint actorId = chunk.readTypedUint16();
		_pendingChild = g_engine->getSpatialEntityById(actorId);
		break;
	}

	case kActorHeaderStageSize: {
		Common::Point size = chunk.readTypedGraphicSize();
		_boundingBox.setWidth(size.x);
		_boundingBox.setHeight(size.y);
		break;
	}

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

void StageActor::draw(const Common::Array<Common::Rect> &dirtyRegion) {
	for (SpatialEntity *entity : _children) {
		if (entity->isVisible()) {
			debugC(5, kDebugGraphics, "%s: Redrawing actor %d", __func__, entity->id());
			entity->draw(dirtyRegion);
		}
	}
}

void StageActor::invalidateRect(const Common::Rect &rect) {
	if (_parentStage != nullptr) {
		Common::Point origin = _boundingBox.origin();
		Common::Rect rectRelativeToParent = rect;
		rectRelativeToParent.translate(origin.x, origin.y);
		_parentStage->invalidateRect(rect);
	} else {
		error("%s: Attempt to invalidate rect without a parent stage", __func__);
	}
}

void StageActor::loadIsComplete() {
	Actor::loadIsComplete();
	if (_pendingChild != nullptr) {
		addChildSpatialEntity(_pendingChild);
		_pendingChild = nullptr;
	}
}

void StageActor::addActorToStage(uint actorId) {
	// If actor has a current parent, remove it from that parent first.
	SpatialEntity *spatialEntity = g_engine->getSpatialEntityById(actorId);
	StageActor *currentParent = spatialEntity->getParentStage();
	if (currentParent != nullptr) {
		currentParent->removeChildSpatialEntity(spatialEntity);
	}
	addChildSpatialEntity(spatialEntity);
}

void StageActor::removeActorFromStage(uint actorId) {
	SpatialEntity *spatialEntity = g_engine->getSpatialEntityById(actorId);
	StageActor *currentParent = spatialEntity->getParentStage();
	if (currentParent == this) {
		// Remove the actor from this stage, and add it
		// back to the root stage.
		removeChildSpatialEntity(spatialEntity);
		RootStage *rootStage = g_engine->getRootStage();
		rootStage->addChildSpatialEntity(spatialEntity);
	}
}

ScriptValue StageActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kAddActorToStageMethod: {
		assert(args.size() == 1);
		uint actorId = args[0].asActorId();
		addActorToStage(actorId);
		return returnValue;
	}

	case kRemoveActorFromStageMethod: {
		assert(args.size() == 1);
		uint actorId = args[0].asActorId();
		removeActorFromStage(actorId);
		return returnValue;
	}

	case kSetBoundsMethod: {
		assert(args.size() == 4);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		int16 width = static_cast<int16>(args[2].asFloat());
		int16 height = static_cast<int16>(args[3].asFloat());
		Common::Rect newBounds(Common::Point(x, y), width, height);
		setBounds(newBounds);
		return returnValue;
	}

	case kStageSetSizeMethod:
		assert(args.size() == 2);
		_boundingBox.setWidth(static_cast<int16>(args[0].asFloat()));
		_boundingBox.setHeight(static_cast<int16>(args[1].asFloat()));
		return returnValue;

	case kStageGetWidthMethod:
		returnValue.setToFloat(_boundingBox.width());
		return returnValue;

	case kStageGetHeightMethod:
		returnValue.setToFloat(_boundingBox.height());
		return returnValue;

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
}

void StageActor::addChildSpatialEntity(SpatialEntity *entity) {
	if (!assertHasNoParent(entity)) {
		error("%s: Attempt to add entity that already has a parent", __func__);
	}

	entity->setParentStage(this);
	_children.insert(entity);

	if (isVisible()) {
		invalidateLocalBounds();
	}
}

void StageActor::removeChildSpatialEntity(SpatialEntity *entity) {
	if (!assertIsMyChild(entity)) {
		error("%s: Attempt to remove entity that is not a child", __func__);
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

uint16 StageActor::findActorToAcceptMouseEvents(
	const Common::Point &point,
	uint16 eventMask,
	MouseActorState &state,
	bool inBounds) {

	debugC(6, kDebugEvents, " --- %s ---", __func__);
	if (!inBounds) {
		if (!_boundingBox.contains(point)) {
			inBounds = true;
		}
	}

	uint16 result = 0;
	Common::Point adjustedPoint = point - _boundingBox.origin();
	for (auto childIterator = _children.end(); childIterator != _children.begin();) {
		--childIterator; // Decrement first, then dereference
		SpatialEntity *child = *childIterator;
		debugC(7, kDebugEvents, " %s: Checking actor %d (z-index: %d) (eventMask: 0x%02x) (result: 0x%02x)", __func__, child->id(), child->zIndex(), eventMask, result);
		uint16 handledEvents = child->findActorToAcceptMouseEvents(adjustedPoint, eventMask, state, inBounds);

		eventMask &= ~handledEvents;
		result |= handledEvents;
	}

	debugC(6, kDebugEvents, " --- END %s ---", __func__);
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
		error("%s: Attempt to invalidate local z-index of non-child", __func__);
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

void RootStage::invalidateRect(const Common::Rect &rect) {
	Common::Rect rectToAdd = rect;
	rectToAdd.clip(_boundingBox);
	_dirtyRegion.push_back(rectToAdd);
}

void RootStage::currentMousePosition(Common::Point &point) {
	point = g_engine->getEventManager()->getMousePos();
	point -= getBbox().origin();
}

void RootStage::drawAll() {
	Common::Array<Common::Rect> region;
	region.push_back(_boundingBox);
	StageActor::draw(region);
}

void RootStage::drawDirtyRegion() {
	if (!_dirtyRegion.empty()) {
		StageActor::draw(_dirtyRegion);
		_dirtyRegion.clear();
	}
}

uint16 RootStage::findActorToAcceptMouseEvents(
	const Common::Point &point,
	uint16 eventMask,
	MouseActorState &state,
	bool inBounds) {

	// Handle any mouse moved events.
	uint16 result = StageActor::findActorToAcceptMouseEvents(point, eventMask, state, inBounds);
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

void RootStage::mouseEnteredEvent(const Common::Event &event) {
	_isMouseInside = true;
	g_engine->getCursorManager()->unsetTemporary();
}

void RootStage::mouseExitedEvent(const Common::Event &event) {
	_isMouseInside = false;
}

void RootStage::mouseOutOfFocusEvent(const Common::Event &event) {
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
	warning("%s: STUB: (%d, %d)", __func__, x, y);
}

StageDirector::StageDirector() {
	_rootStage = new RootStage;
	Common::Rect rootStageBounds(MediaStationEngine::SCREEN_WIDTH, MediaStationEngine::SCREEN_HEIGHT);
	_rootStage->setBounds(rootStageBounds);
	g_engine->registerActor(_rootStage);
}

StageDirector::~StageDirector() {
	delete _rootStage;
	_rootStage = nullptr;
}

void StageDirector::drawAll() {
	// TODO: This just passes through for now,
	// but once the original DisplayContext stuff
	// is reimplemented, there will be more here.
	_rootStage->drawAll();
}

void StageDirector::drawDirtyRegion() {
	// TODO: This just passes through for now,
	// but once the original DisplayContext stuff
	// is reimplemented, there will be more here.
	_rootStage->drawDirtyRegion();
}

void StageDirector::clearDirtyRegion() {
	// TODO: This just passes through for now,
	// but once the original DisplayContext stuff
	// is reimplemented, there will be more here.
	_rootStage->clearDirtyRegion();
}

void StageDirector::handleKeyboardEvent(const Common::Event &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptKeyboardEvents(event.kbd.ascii, kKeyDownFlag, state);
	if (flags & kKeyDownFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching to actor %d", __func__, state.keyDown->id());
		state.keyDown->keyboardEvent(event);
	}
}

void StageDirector::handleMouseDownEvent(const Common::Event &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptMouseEvents(event.mouse, kMouseDownFlag, state, false);
	if (flags & kMouseDownFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching to actor %d", __func__, state.mouseDown->id());
		state.mouseDown->mouseDownEvent(event);
	}
}

void StageDirector::handleMouseUpEvent(const Common::Event &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptMouseEvents(event.mouse, kMouseUpFlag, state, false);
	if (flags & kMouseUpFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching to actor %d", __func__, state.mouseUp->id());
		state.mouseUp->mouseUpEvent(event);
	}
}

void StageDirector::handleMouseMovedEvent(const Common::Event &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptMouseEvents(
		event.mouse,
		kMouseEnterFlag | kMouseExitFlag | kMouseMovedFlag,
		state, false);
	debugC(5, kDebugEvents, "%s: Calling sendMouseEnterExitEvent", __func__);

	sendMouseEnterExitEvent(flags, state, event);
	if (flags & kMouseMovedFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching mouse moved to actor %d", __func__, state.mouseMoved->id());
		state.mouseMoved->mouseMovedEvent(event);
	}
}

void StageDirector::handleMouseEnterExitEvent(const Common::Event &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptMouseEvents(event.mouse, kMouseEnterFlag | kMouseExitFlag, state, false);
	sendMouseEnterExitEvent(flags, state, event);
}

void StageDirector::handleMouseOutOfFocusEvent(const Common::Event &event) {
	MouseActorState state;
	uint16 flags = _rootStage->findActorToAcceptMouseEvents(event.mouse, kMouseExitFlag | kMouseOutOfFocusFlag, state, false);

	if (flags & kMouseExitFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching mouse enter to actor %d", __func__, state.mouseExit->id());
		state.mouseExit->mouseExitedEvent(event);
	}

	if (flags & kMouseOutOfFocusFlag) {
		debugC(5, kDebugEvents, "%s: Dispatching mouse out of focus to actor %d", __func__, state.mouseOutOfFocus->id());
		state.mouseOutOfFocus->mouseOutOfFocusEvent(event);
	}
}

void StageDirector::sendMouseEnterExitEvent(uint16 flags, MouseActorState &state, const Common::Event &event) {
	if (state.mouseMoved != state.mouseEnter || state.mouseMoved != state.mouseExit) {
		if (flags & kMouseEnterFlag) {
			debugC(5, kDebugEvents, "%s: Dispatching mouse enter to actor %d", __func__, state.mouseEnter->id());
			state.mouseEnter->mouseEnteredEvent(event);
		}

		if (flags & kMouseExitFlag) {
			debugC(5, kDebugEvents, "%s: Dispatching mouse exit to actor %d", __func__, state.mouseExit->id());
			state.mouseExit->mouseExitedEvent(event);
		}
	} else {
		debugC(5, kDebugEvents, "%s: No hotspot to dispatch to", __func__);
	}
}

} // End of namespace MediaStation
