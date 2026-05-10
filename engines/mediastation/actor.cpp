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
#include "mediastation/actors/camera.h"
#include "mediastation/actors/stage.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediascript/scriptconstants.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

const char *actorTypeToStr(ActorType type) {
	switch (type) {
	case kActorTypeEmpty:
		return "Empty";
	case kActorTypeScreen:
		return "Screen";
	case kActorTypeStage:
		return "Stage";
	case kActorTypePath:
		return "Path";
	case kActorTypeSound:
		return "Sound";
	case kActorTypeTimer:
		return "Timer";
	case kActorTypeImage:
		return "Image";
	case kActorTypeHotspot:
		return "Hotspot";
	case kActorTypeCursor:
		return "Cursor";
	case kActorTypeSprite:
		return "Sprite";
	case kActorTypeLKZazu:
		return "LKZazu";
	case kActorTypeLKConstellations:
		return "LKConstellations";
	case kActorTypeDocument:
		return "Document";
	case kActorTypeDiskImage:
		return "ImageSet";
	case kActorTypeMovie:
		return "Movie";
	case kActorTypeStreamMovieProxy:
		return "StreamMovieProxy";
	case kActorTypePalette:
		return "Palette";
	case kActorTypePrinter:
		return "Printer";
	case kActorTypeText:
		return "Text";
	case kActorTypeFont:
		return "Font";
	case kActorTypeCamera:
		return "Camera";
	case kActorTypeCanvas:
		return "Canvas";
	case kActorTypeXsnd:
		return "Xsnd";
	case kActorTypeXsndMidi:
		return "XsndMidi";
	case kActorTypeRecorder:
		return "Recorder";
	case kActorTypeFunction:
		return "Function";
	default:
		return "UNKNOWN";
	}
}

void Polygon::loadFromParameterStream(Chunk & chunk) {
	uint16 totalPoints = chunk.readTypedUint16();
	for (uint16 i = 0; i < totalPoints; i++) {
		Common::Point point = chunk.readTypedPoint();
		_polygon.push_back(point);
	}
}

bool Polygon::containsPoint(const Common::Point &point) const {
	// We're in the bbox, but there might not be a polygon to check.
	if (_polygon.empty()) {
		return true;
	}

	// Each edge is checked whether it cuts the outgoing stream from the point.
	int rcross = 0; // Number of right-side overlaps
	for (unsigned i = 0; i < _polygon.size(); i++) {
		const Common::Point &edgeStart = _polygon[i];
		const Common::Point &edgeEnd = _polygon[(i + 1) % _polygon.size()];

		// A vertex is a point? Then it lies on one edge of the polygon.
		if (point == edgeStart)
			return true;

		if ((edgeStart.y > point.y) != (edgeEnd.y > point.y)) {
			int term1 = (edgeStart.x - point.x) * (edgeEnd.y - point.y) - (edgeEnd.x - point.x) * (edgeStart.y - point.y);
			int term2 = (edgeEnd.y - point.y) - (edgeStart.y - edgeEnd.y);
			if ((term1 > 0) == (term2 >= 0))
				rcross++;
		}
	}

	// The point is strictly inside the polygon if and only if the number of overlaps is odd.
	return ((rcross % 2) == 1);
}

void Actor::setId(uint id) {
	_id = id;
	_debugName = g_engine->formatActorName(this);
}

const char *Actor::debugName() const {
	return _debugName.c_str();
}

Actor::~Actor() {
	for (auto it = _scriptResponses.begin(); it != _scriptResponses.end(); ++it) {
		Common::Array<ScriptResponse *> &responsesForType = it->_value;
		for (ScriptResponse *response : responsesForType) {
			delete response;
		}
		responsesForType.clear();
	}
	_scriptResponses.clear();
}

void Actor::initFromParameterStream(Chunk &chunk) {
	ActorHeaderSectionType paramType = kActorHeaderEmptySection;
	while (true) {
		paramType = static_cast<ActorHeaderSectionType>(chunk.readTypedUint16());
		debugC(5, kDebugLoading, "[%s] %s: Got section type 0x%x", debugName(), __func__, static_cast<uint>(paramType));
		if (paramType == 0) {
			break;
		} else {
			readParameter(chunk, paramType);
		}
	}
}

void Actor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderScriptResponse: {
		ScriptResponse *scriptResponse = new ScriptResponse(chunk);
		Common::Array<ScriptResponse *> &scriptResponsesForType = _scriptResponses.getOrCreateVal(scriptResponse->_type);

		// This is not a hashmap because we don't want to have to hash ScriptValues.
		for (ScriptResponse *existingScriptResponse : scriptResponsesForType) {
			if (existingScriptResponse->_argumentValue == scriptResponse->_argumentValue) {
				error("[%s] %s: Script response for %s (%s) already exists", debugName(), __func__,
					eventTypeToStr(scriptResponse->_type), scriptResponse->_argumentValue.getDebugString().c_str());
			}
		}
		scriptResponsesForType.push_back(scriptResponse);
		break;
	}

	case kActorHeaderActorName:
		_debugName = chunk.readTypedString();
		break;

	default:
		error("[%s] %s: Got unimplemented actor parameter 0x%x", debugName(), __func__, static_cast<uint>(paramType));
	}
}

void Actor::loadIsComplete() {
	if (_loadIsComplete) {
		warning("[%s] %s: Already loaded", debugName(), __func__);
	}
	_loadIsComplete = true;
}

ScriptValue Actor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	warning("[%s] %s: Got unimplemented method call 0x%x (%s)",
		debugName(), __func__, static_cast<uint>(methodId), builtInMethodToStr(methodId));
	return ScriptValue();
}

void Actor::onEvent(const ActorEvent &event) {
	warning("[%s] %s: No handler for engine event %s", debugName(), __func__, eventTypeToStr(event.type));
}

ScriptResponse *Actor::findNextTimeScriptResponseAfter(uint32 after) const {
	const Common::Array<ScriptResponse *> &_timeResponses = _scriptResponses.getValOrDefault(kTimerScriptEvent);
	for (ScriptResponse *timeEvent : _timeResponses) {
		double timeEventInFractionalSeconds = timeEvent->_argumentValue.asFloat();
		uint32 timeEventInMilliseconds = static_cast<uint32>(timeEventInFractionalSeconds * 1000);
		if (timeEventInMilliseconds >= after) {
			return timeEvent;
		}
	}

	return nullptr;
}

void Actor::runScriptResponseIfExists(EventType eventType, const ScriptValue &arg) {
	const Common::Array<ScriptResponse *> &scriptResponses = _scriptResponses.getValOrDefault(eventType);
	for (ScriptResponse *scriptResponse : scriptResponses) {
		const ScriptValue &argToCheck = scriptResponse->_argumentValue;

		if (arg.getType() != argToCheck.getType()) {
			warning("[%s] %s: Got script response arg type %s, expected %s", debugName(), __func__,
				scriptValueTypeToStr(arg.getType()), scriptValueTypeToStr(argToCheck.getType()));
			continue;
		}

		if (arg == argToCheck) {
			debugC(5, kDebugScript, "[%s] %s: Executing response for event type %s", debugName(), __func__, eventTypeToStr(eventType));
			scriptResponse->execute(_id);
			return;
		}
	}

	debugC(5, kDebugScript, "[%s] %s: No script response for event type %s", debugName(), __func__, eventTypeToStr(eventType));
}

void Actor::runScriptResponseIfExists(EventType eventType) {
	ScriptValue scriptValue;
	runScriptResponseIfExists(eventType, scriptValue);
}

void Actor::processTimeScriptResponses() {
	// The original had this logic duplicated across several actors, but it made more sense
	// to consolidate it into the main Actor in the reimplementation. This does NOT set up the
	// next timer - client code has to do that itself.
	// Get current runtime time.
	uint32 currentTimeInMilliseconds = g_engine->getTotalPlayTime();
	uint32 elapsedTimeInMilliseconds = currentTimeInMilliseconds - _startTime;

	// Process all events that have elapsed up to current time.
	ScriptResponse *nextTimeScriptResponse = findNextTimeScriptResponseAfter(_lastProcessedTime);
	while (nextTimeScriptResponse != nullptr) {
		// If this event is in the future, stop processing.
		double eventTimeInSeconds = nextTimeScriptResponse->_argumentValue.asFloat();
		uint32 eventTimeInMilliseconds = eventTimeInSeconds * 1000;
		if (eventTimeInMilliseconds > elapsedTimeInMilliseconds) {
			break;
		}

		// Execute the event.
		debugC(5, kDebugScript, "[%s] %s: Running On Time response for time %d ms (lastProcessedTime: %d, elapsedTime: %d)",
			debugName(), __func__, eventTimeInMilliseconds, _lastProcessedTime, elapsedTimeInMilliseconds);
		nextTimeScriptResponse->execute(_id);

		// Increment by 1 to prevent re-triggering the same event. This works because in the original,
		// timer events are at least 10 ms apart anyway.
		_lastProcessedTime = eventTimeInMilliseconds + 1;
		nextTimeScriptResponse = findNextTimeScriptResponseAfter(_lastProcessedTime);
	}
}

bool Actor::setupNextScriptResponseTimer() {
	// Find the next event after the last processed time.
	ScriptResponse *nextEvent = findNextTimeScriptResponseAfter(_lastProcessedTime);
	if (nextEvent == nullptr) {
		// No more events to schedule.
		debugC(5, kDebugScript, "[%s] %s: No more events to schedule", debugName(), __func__);
		return false;
	}

	// Calculate duration until next event from current elapsed time.
	double nextEventTimeInFractionalSeconds = nextEvent->_argumentValue.asFloat();
	uint32 nextEventTimeInMilliseconds = nextEventTimeInFractionalSeconds * 1000;
	uint32 currentTimeInMilliseconds = g_engine->getTotalPlayTime();
	uint32 elapsedTimeInMilliseconds = currentTimeInMilliseconds - _startTime;
	uint32 durationUntilNextEventInMilliseconds = nextEventTimeInMilliseconds - elapsedTimeInMilliseconds;
	debugC(5, kDebugEvents, "[%s] %s: Scheduling timer for %d ms (next event at %d ms)",
		debugName(), __func__, durationUntilNextEventInMilliseconds, nextEventTimeInMilliseconds);
	g_engine->getTimerService()->startTimer(_timer, durationUntilNextEventInMilliseconds);
	return true;
}

void Actor::triggerRemainingTimerEvents() {
	uint32 currentTimeInMilliseconds = g_engine->getTotalPlayTime();
	uint32 elapsedTimeInMilliseconds = currentTimeInMilliseconds - _startTime;

	ScriptResponse *nextTimeScriptResponse = findNextTimeScriptResponseAfter(_lastProcessedTime);
	while (nextTimeScriptResponse != nullptr) {
		double eventTimeInSeconds = nextTimeScriptResponse->_argumentValue.asFloat();
		uint32 eventTimeInMilliseconds = eventTimeInSeconds * 1000;

		debugC(5, kDebugEvents, "[%s] %s: Running remaining On Time response for time %d ms (lastProcessedTime: %d, elapsedTime: %d)",
			debugName(), __func__, eventTimeInMilliseconds,  _lastProcessedTime, elapsedTimeInMilliseconds);

		// Increment by 1 to prevent re-triggering the same event.
		_lastProcessedTime = eventTimeInMilliseconds + 1;
		nextTimeScriptResponse->execute(_id);
		nextTimeScriptResponse = findNextTimeScriptResponseAfter(_lastProcessedTime);
	}
}

SpatialEntity::~SpatialEntity() {
	if (_parentStage != nullptr) {
		_parentStage->removeChildSpatialEntity(this);
	}
}

ScriptValue SpatialEntity::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kSpatialMoveToMethod: {
		ARGCOUNTCHECK(2);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		moveTo(x, y);
		break;
	}

	case kSpatialMoveToByOffsetMethod: {
		ARGCOUNTCHECK(2);
		int16 dx = static_cast<int16>(args[0].asFloat());
		int16 dy = static_cast<int16>(args[1].asFloat());
		int16 newX = _boundingBox.left + dx;
		int16 newY = _boundingBox.top + dy;
		moveTo(newX, newY);
		break;
	}

	case kSpatialZMoveToMethod: {
		ARGCOUNTCHECK(1);
		int zIndex = static_cast<int>(args[0].asFloat());
		setZIndex(zIndex);
		break;
	}

	case kSpatialCenterMoveToMethod: {
		ARGCOUNTCHECK(2);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		moveToCentered(x, y);
		break;
	}

	case kGetLeftXMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_boundingBox.left);
		break;

	case kGetTopYMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_boundingBox.top);
		break;

	case kGetWidthMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_boundingBox.width());
		break;

	case kGetHeightMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_boundingBox.height());
		break;

	case kGetCenterXMethod: {
		ARGCOUNTCHECK(0);
		int centerX = _boundingBox.left + (_boundingBox.width() / 2);
		returnValue.setToFloat(centerX);
		break;
	}

	case kGetCenterYMethod: {
		ARGCOUNTCHECK(0);
		int centerY = _boundingBox.top + (_boundingBox.height() / 2);
		returnValue.setToFloat(centerY);
		break;
	}

	case kGetZCoordinateMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_zIndex);
		break;

	case kIsPointInsideMethod: {
		ARGCOUNTCHECK(2);
		int16 xToCheck = static_cast<int16>(args[0].asFloat());
		int16 yToCheck = static_cast<int16>(args[1].asFloat());
		Common::Point pointToCheck(xToCheck, yToCheck);
		bool pointIsInside = getBbox().contains(pointToCheck);
		returnValue.setToBool(pointIsInside);
		break;
	}

	case kSetDissolveFactorMethod: {
		ARGCOUNTCHECK(1);
		double dissolveFactor = args[0].asFloat();
		setDissolveFactor(dissolveFactor);
		break;
	}

	case kGetMouseXOffsetMethod: {
		Common::Point mouseOffset;
		currentMousePosition(mouseOffset);
		mouseOffset -= _originalBoundingBox.origin();
		returnValue.setToFloat(static_cast<double>(mouseOffset.x));
		break;
	}

	case kGetMouseYOffsetMethod: {
		Common::Point mouseOffset;
		currentMousePosition(mouseOffset);
		mouseOffset -= _originalBoundingBox.origin();
		returnValue.setToFloat(static_cast<double>(mouseOffset.y));
		break;
	}

	case kIsVisibleMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(isVisible());
		break;

	case kSetMousePositionMethod: {
		ARGCOUNTCHECK(2);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		setMousePosition(x, y);
		break;
	}

	case kGetParallaxFactorXMethod1:
	case kGetParallaxFactorXMethod2:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_parallaxFactorX);
		break;

	case kSetParallaxFactorMethod:
		ARGCOUNTCHECK(1);
		invalidateLocalBounds();
		_parallaxFactorX = _parallaxFactorY = args[0].asFloat();
		invalidateLocalBounds();
		break;

	case kSetParallaxFactorXMethod:
		ARGCOUNTCHECK(1);
		invalidateLocalBounds();
		_parallaxFactorX = args[0].asFloat();
		invalidateLocalBounds();
		break;

	case kGetParallaxFactorYMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_parallaxFactorY);
		break;

	case kSetParallaxFactorYMethod:
		ARGCOUNTCHECK(1);
		invalidateLocalBounds();
		_parallaxFactorY = args[0].asFloat();
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
		_originalBoundingBox = chunk.readTypedRect();
		setAdjustedBounds(kWrapNone);
		break;

	case kActorHeaderDissolveFactor: {
		double dissolveFactor = chunk.readTypedDouble();
		setDissolveFactor(dissolveFactor);
		break;
	}

	case kActorHeaderZIndex:
		_zIndex = chunk.readTypedGraphicUnit();
		break;

	case kActorHeaderTransparency:
		_hasTransparency = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderChildActorId:
		_stageId = chunk.readTypedUint16();
		break;

	case kActorHeaderScaleXAndY:
		_parallaxFactorX = _parallaxFactorY = chunk.readTypedDouble();
		break;

	case kActorHeaderScaleX:
		_parallaxFactorX = chunk.readTypedDouble();
		break;

	case kActorHeaderScaleY:
		_parallaxFactorY = chunk.readTypedDouble();
		break;

	default:
		Actor::readParameter(chunk, paramType);
	}
}

void SpatialEntity::loadIsComplete() {
	Actor::loadIsComplete();
	if (_stageId != 0) {
		StageActor *pendingParentStage = static_cast<StageActor *>(g_engine->getImtGod()->getActorByIdAndType(_stageId, kActorTypeStage));
		pendingParentStage->addChildSpatialEntity(this);
	}
}

void SpatialEntity::currentMousePosition(Common::Point &point) {
	if (_parentStage != nullptr) {
		_parentStage->currentMousePosition(point);
	}
}

void SpatialEntity::invalidateMouse() {
	MouseEvent event(kMouseEnterExitEvent, g_system->getEventManager()->getMousePos());
	g_engine->getEventLoop()->queueEvent(event);
}

void SpatialEntity::moveTo(int16 x, int16 y) {
	Common::Point dest(x, y);
	debugC(3, kDebugGraphics, "[%s] %s: (%d, %d) -> (%d, %d)", debugName(), __func__,
		_originalBoundingBox.origin().x, _originalBoundingBox.origin().y, x, y);

	if (dest == _boundingBox.origin()) {
		// We aren't actually moving anywhere.
		return;
	}

	if (isVisible()) {
		invalidateLocalBounds();
	}
	_originalBoundingBox.moveTo(dest);
	setAdjustedBounds(kWrapNone);
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
	debugC(3, kDebugGraphics, "[%s] %s: (%d, %d)", debugName(), __func__, targetX, targetY);
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
	_originalBoundingBox = bounds;
	setAdjustedBounds(kWrapNone);
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
	if (_parentStage != nullptr) {
		setAdjustedBounds(kWrapNone);
		_parentStage->invalidateRect(getBbox());
	}
}

void SpatialEntity::invalidateLocalZIndex() {
	if (_parentStage != nullptr) {
		_parentStage->invalidateZIndexOf(this);
	}
}

void SpatialEntity::setAdjustedBounds(CylindricalWrapMode wrapMode) {
	_boundingBox = _originalBoundingBox;
	if (_parentStage == nullptr) {
		return;
	}

	Common::Point offset(0, 0);
	Common::Point stageExtent = _parentStage->extent();

	// Calculate position offset for cylindrical wrapping.
	switch (wrapMode) {
	case kWrapRight:
		offset.x = stageExtent.x;
		offset.y = 0;
		break;

	case kWrapDown:
		offset.x = 0;
		offset.y = stageExtent.y;
		break;

	case kWrapRightDown:
		offset.x = stageExtent.x;
		offset.y = stageExtent.y;
		break;

	case kWrapLeft:
		offset.x = -stageExtent.x;
		offset.y = 0;
		break;

	case kWrapUp:
		offset.x = 0;
		offset.y = -stageExtent.y;
		break;

	case kWrapLeftUp:
		offset.x = -stageExtent.x;
		offset.y = -stageExtent.y;
		break;

	case kWrapLeftDown:
		offset.x = -stageExtent.x;
		offset.y = stageExtent.y;
		break;

	case kWrapRightUp:
		offset.x = stageExtent.x;
		offset.y = -stageExtent.y;
		break;

	case kWrapNone:
	default:
		// No offset adjustment.
		break;
	}
	_boundingBox.translate(-offset.x, -offset.y);

	// Apply parallax scrolling if parallax factors are set.
	// This simulates depth by adjusting position based on distance from viewport center.
	// parallax 0.0 means no parallax (ignores camera movement).
	// parallax 1.0 means neutral depth (moves with camera at focal plane).
	// parallax >1.0 means appears closer (moves more than camera).
	// parallax <1.0 means appears farther (moves less than camera).
	bool hasHorizontalParallax = _parallaxFactorX != 0.0;
	bool hasVerticalParallax = _parallaxFactorY != 0.0;
	if (!hasHorizontalParallax && !hasVerticalParallax) {
		return;
	}

	// Transform camera viewport to object's local coordinate space (inside any stages).
	CameraActor *currentCamera = _parentStage->getCurrentCamera();
	if (currentCamera == nullptr) {
		return;
	}

	Common::Rect viewportBounds = currentCamera->getViewportBounds();
	Common::Rect localViewport = viewportBounds;
	Common::Point accumulatedOffset = viewportBounds.origin();
	StageActor *currentStage = _parentStage;
	while (currentStage != nullptr && currentStage != currentCamera->getParentStage()) {
		Common::Rect parentBounds = currentStage->getBbox();
		accumulatedOffset -= parentBounds.origin();
		currentStage = currentStage->getParentStage();
	}
	localViewport.moveTo(accumulatedOffset.x, accumulatedOffset.y);

	if (_boundingBox.intersects(localViewport)) {
		// Apply horizontal parallax.
		int16 parallaxAdjustedLeft = _boundingBox.left;
		if (hasHorizontalParallax) {
			// Calculate offset from viewport center to object center.
			int16 viewportHalfWidth = localViewport.width() / 2;
			int16 boundsHalfWidth = _boundingBox.width() / 2;
			int centerOffset = (_boundingBox.left + boundsHalfWidth) - (localViewport.left + viewportHalfWidth);

			// Multiply by parallax factor to simulate depth.
			double parallaxOffset = static_cast<double>(centerOffset) * _parallaxFactorX;
			parallaxAdjustedLeft += static_cast<int16>(parallaxOffset);
		}

		// Apply vertical parallax.
		int16 parallaxAdjustedTop = _boundingBox.top;
		if (hasVerticalParallax) {
			// Calculate offset from viewport center to object center.
			int16 viewportHalfHeight = localViewport.height() / 2;
			int16 boundsHalfHeight = _boundingBox.height() / 2;
			int centerOffset = (_boundingBox.top + boundsHalfHeight) - (localViewport.top + viewportHalfHeight);

			// Multiply by parallax factor to simulate depth.
			double parallaxOffset = static_cast<double>(centerOffset) * _parallaxFactorY;
			parallaxAdjustedTop += static_cast<int16>(parallaxOffset);
		}

		_boundingBox.moveTo(parallaxAdjustedLeft, parallaxAdjustedTop);
	}
}

} // End of namespace MediaStation
