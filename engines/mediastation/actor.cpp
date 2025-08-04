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

#include "common/util.h"

#include "mediastation/actor.h"
#include "mediastation/actors/stage.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediascript/scriptconstants.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

Actor::~Actor() {
	for (auto it = _eventHandlers.begin(); it != _eventHandlers.end(); ++it) {
		Common::Array<EventHandler *> &handlersForType = it->_value;
		for (EventHandler *handler : handlersForType) {
			delete handler;
		}
		handlersForType.clear();
	}
	_eventHandlers.clear();
}

void Actor::initFromParameterStream(Chunk &chunk) {
	ActorHeaderSectionType paramType = kActorHeaderEmptySection;
	while (true) {
		paramType = static_cast<ActorHeaderSectionType>(chunk.readTypedUint16());
		if (paramType == 0) {
			break;
		} else {
			readParameter(chunk, paramType);
		}
	}
}

void Actor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderEventHandler: {
		EventHandler *eventHandler = new EventHandler(chunk);
		Common::Array<EventHandler *> &eventHandlersForType = _eventHandlers.getOrCreateVal(eventHandler->_type);

		// This is not a hashmap because we don't want to have to hash ScriptValues.
		for (EventHandler *existingEventHandler : eventHandlersForType) {
			if (existingEventHandler->_argumentValue == eventHandler->_argumentValue) {
				error("%s: Event handler for %s (%s) already exists", __func__,
					  eventTypeToStr(eventHandler->_type), eventHandler->getDebugHeader().c_str());
			}
		}
		eventHandlersForType.push_back(eventHandler);
		break;
	}

	default:
		error("Got unimplemented actor parameter 0x%x", static_cast<uint>(paramType));
	}
}

void Actor::loadIsComplete() {
	if (_loadIsComplete) {
		warning("%s: Called more than once for actor %d", __func__, _id);
	}
	_loadIsComplete = true;
}

ScriptValue Actor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	error("%s: Got unimplemented method call 0x%x (%s)", __func__, static_cast<uint>(methodId), builtInMethodToStr(methodId));
}

void Actor::readChunk(Chunk &chunk) {
	error("%s: Chunk reading for actor type 0x%x is not implemented", __func__, static_cast<uint>(_type));
}

void Actor::readSubfile(Subfile &subfile, Chunk &chunk) {
	error("%s: Subfile reading for actor type 0x%x is not implemented", __func__, static_cast<uint>(_type));
}

void Actor::processTimeEventHandlers() {
	// TODO: Replace with a queue.
	uint currentTime = g_system->getMillis();
	const Common::Array<EventHandler *> &_timeHandlers = _eventHandlers.getValOrDefault(kTimerEvent);
	for (EventHandler *timeEvent : _timeHandlers) {
		// Indeed float, not time.
		double timeEventInFractionalSeconds = timeEvent->_argumentValue.asFloat();
		uint timeEventInMilliseconds = timeEventInFractionalSeconds * 1000;
		bool timeEventAlreadyProcessed = timeEventInMilliseconds < _lastProcessedTime;
		bool timeEventNeedsToBeProcessed = timeEventInMilliseconds <= currentTime - _startTime;
		if (!timeEventAlreadyProcessed && timeEventNeedsToBeProcessed) {
			debugC(5, kDebugScript, "Actor::processTimeEventHandlers(): Running On Time handler for time %d ms", timeEventInMilliseconds);
			timeEvent->execute(_id);
		}
	}
	_lastProcessedTime = currentTime - _startTime;
}

void Actor::runEventHandlerIfExists(EventType eventType, const ScriptValue &arg) {
	const Common::Array<EventHandler *> &eventHandlers = _eventHandlers.getValOrDefault(eventType);
	for (EventHandler *eventHandler : eventHandlers) {
		const ScriptValue &argToCheck = eventHandler->_argumentValue;

		if (arg.getType() != argToCheck.getType()) {
			warning("Got event handler arg type %s, expected %s",
					scriptValueTypeToStr(arg.getType()), scriptValueTypeToStr(argToCheck.getType()));
			continue;
		}

		if (arg == argToCheck) {
			debugC(5, kDebugScript, "Executing handler for event type %s on actor %d", eventTypeToStr(eventType), _id);
			eventHandler->execute(_id);
			return;
		}
	}
	debugC(5, kDebugScript, "No event handler for event type %s on actor %d", eventTypeToStr(eventType), _id);
}

void Actor::runEventHandlerIfExists(EventType eventType) {
	ScriptValue scriptValue;
	runEventHandlerIfExists(eventType, scriptValue);
}

ScriptValue SpatialEntity::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kSpatialMoveToMethod: {
		assert(args.size() == 2);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		moveTo(x, y);
		break;
	}

	case kSpatialMoveToByOffsetMethod: {
		assert(args.size() == 2);
		int16 dx = static_cast<int16>(args[0].asFloat());
		int16 dy = static_cast<int16>(args[1].asFloat());
		int16 newX = _boundingBox.left + dx;
		int16 newY = _boundingBox.top + dy;
		moveTo(newX, newY);
		break;
	}

	case kSpatialZMoveToMethod: {
		assert(args.size() == 1);
		int zIndex = static_cast<int>(args[0].asFloat());
		setZIndex(zIndex);
		break;
	}

	case kSpatialCenterMoveToMethod: {
		assert(args.size() == 2);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		moveToCentered(x, y);
		break;
	}

	case kGetLeftXMethod:
		assert(args.empty());
		returnValue.setToFloat(_boundingBox.left);
		break;

	case kGetTopYMethod:
		assert(args.empty());
		returnValue.setToFloat(_boundingBox.top);
		break;

	case kGetWidthMethod:
		assert(args.empty());
		returnValue.setToFloat(_boundingBox.width());
		break;

	case kGetHeightMethod:
		assert(args.empty());
		returnValue.setToFloat(_boundingBox.height());
		break;

	case kGetCenterXMethod: {
		assert(args.empty());
		int centerX = _boundingBox.left + (_boundingBox.width() / 2);
		returnValue.setToFloat(centerX);
		break;
	}

	case kGetCenterYMethod: {
		assert(args.empty());
		int centerY = _boundingBox.top + (_boundingBox.height() / 2);
		returnValue.setToFloat(centerY);
		break;
	}

	case kGetZCoordinateMethod:
		assert(args.empty());
		returnValue.setToFloat(_zIndex);
		break;

	case kSetDissolveFactorMethod: {
		assert(args.size() == 1);
		double dissolveFactor = args[0].asFloat();
		setDissolveFactor(dissolveFactor);
		break;
	}

	case kIsVisibleMethod:
		assert(args.empty());
		returnValue.setToBool(isVisible());
		break;

	case kSetMousePositionMethod: {
		assert(args.size() == 2);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		setMousePosition(x, y);
		break;
	}

	case kGetXScaleMethod1:
	case kGetXScaleMethod2:
		assert(args.empty());
		returnValue.setToFloat(_scaleX);
		break;

	case kSetScaleMethod:
		assert(args.size() == 1);
		invalidateLocalBounds();
		_scaleX = _scaleY = args[0].asFloat();
		invalidateLocalBounds();
		break;

	case kSetXScaleMethod:
		assert(args.size() == 1);
		invalidateLocalBounds();
		_scaleX = args[0].asFloat();
		invalidateLocalBounds();
		break;

	case kGetYScaleMethod:
		assert(args.empty());
		returnValue.setToFloat(_scaleY);
		break;

	case kSetYScaleMethod:
		assert(args.size() == 1);
		invalidateLocalBounds();
		_scaleY = args[0].asFloat();
		invalidateLocalBounds();
		break;

	default:
		Actor::callMethod(methodId, args);
	}
	return returnValue;
}

void SpatialEntity::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderBoundingBox:
		_boundingBox = chunk.readTypedRect();
		break;

	case kActorHeaderDissolveFactor:
		_dissolveFactor = chunk.readTypedDouble();
		break;

	case kActorHeaderZIndex:
		_zIndex = chunk.readTypedGraphicUnit();
		break;

	case kActorHeaderTransparency:
		_hasTransparency = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderChildActorId:
		_stageId = chunk.readTypedUint16();
		break;

	case kActorHeaderActorReference:
		_actorReference = chunk.readTypedUint16();
		break;

	case kActorHeaderScaleXAndY:
		_scaleX = _scaleY = chunk.readTypedDouble();
		break;

	case kActorHeaderScaleX:
		_scaleX = chunk.readTypedDouble();
		break;

	case kActorHeaderScaleY:
		_scaleY = chunk.readTypedDouble();
		break;

	default:
		Actor::readParameter(chunk, paramType);
	}
}

void SpatialEntity::loadIsComplete() {
	Actor::loadIsComplete();
	Actor *pendingParentStageActor = g_engine->getActorById(_stageId);
	if (pendingParentStageActor == nullptr) {
		error("%s: Actor %d doesn't exist", __func__, _stageId);
	} else if (pendingParentStageActor->type() != kActorTypeStage) {
		error("%s: Requested parent stage %d is not a stage", __func__, _stageId);
	}
	StageActor *pendingParentStage = static_cast<StageActor *>(pendingParentStageActor);
	pendingParentStage->addChildSpatialEntity(this);
}

void SpatialEntity::invalidateMouse() {
	// TODO: Invalidate the mouse properly when we have custom events.
	// For now, we simulate the mouse update event with a mouse moved event.
	Common::Event mouseEvent;
	mouseEvent.type = Common::EVENT_MOUSEMOVE;
	mouseEvent.mouse = g_system->getEventManager()->getMousePos();
	g_system->getEventManager()->pushEvent(mouseEvent);
}

void SpatialEntity::moveTo(int16 x, int16 y) {
	Common::Point dest(x, y);
	if (dest == _boundingBox.origin()) {
		// We aren't actually moving anywhere.
		return;
	}

	if (isVisible()) {
		invalidateLocalBounds();
	}
	_boundingBox.moveTo(dest);
	if (isVisible()) {
		invalidateLocalBounds();
	}
	if (interactsWithMouse()) {
		invalidateMouse();
	}
}

void SpatialEntity::moveToCentered(int16 x, int16 y) {
	int16 targetX = x - (_boundingBox.width() / 2);
	int16 targetY = y - (_boundingBox.height() / 2);
	moveTo(targetX, targetY);
}

void SpatialEntity::setBounds(const Common::Rect &bounds) {
	if (_boundingBox == bounds) {
		// We aren't actually moving anywhere.
		return;
	}

	if (isVisible()) {
		invalidateLocalBounds();
	}
	_boundingBox = bounds;
	if (isVisible()) {
		invalidateLocalBounds();
	}
	if (interactsWithMouse()) {
		invalidateMouse();
	}
}

void SpatialEntity::setZIndex(int zIndex) {
	if (_zIndex == zIndex) {
		// We aren't actually moving anywhere.
		return;
	}

	_zIndex = zIndex;
	invalidateLocalZIndex();
	if (interactsWithMouse()) {
		invalidateMouse();
	}
}

void SpatialEntity::setMousePosition(int16 x, int16 y) {
	if (_parentStage) {
		_parentStage->setMousePosition(x, y);
	}
}

void SpatialEntity::setDissolveFactor(double dissolveFactor) {
	dissolveFactor = CLIP(dissolveFactor, 0.0, 1.0);
	if (dissolveFactor != _dissolveFactor) {
		_dissolveFactor = dissolveFactor;
		invalidateLocalBounds();
	}
}

void SpatialEntity::invalidateLocalBounds() {
	g_engine->addDirtyRect(getBbox());
}

void SpatialEntity::invalidateLocalZIndex() {
	warning("STUB: %s", __func__);
}

} // End of namespace MediaStation
