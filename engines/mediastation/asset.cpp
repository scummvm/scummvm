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

#include "mediastation/debugchannels.h"
#include "mediastation/asset.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

Asset::~Asset() {
	delete _header;
	_header = nullptr;
}

ScriptValue Asset::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	error("Got unimplemented method call %d (%s)", static_cast<uint>(methodId), builtInMethodToStr(methodId));
}

void Asset::readChunk(Chunk &chunk) {
	error("Asset::readChunk(): Chunk reading for asset type 0x%x is not implemented", static_cast<uint>(_header->_type));
}

void Asset::readSubfile(Subfile &subfile, Chunk &chunk) {
	error("Asset::readSubfile(): Subfile reading for asset type 0x%x is not implemented", static_cast<uint>(_header->_type));
}

void Asset::setActive() {
	_isActive = true;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	g_engine->addPlayingAsset(this);
}

void Asset::setInactive() {
	_isActive = false;
	_startTime = 0;
	_lastProcessedTime = 0;
}

void Asset::processTimeEventHandlers() {
	if (!_isActive) {
		warning("Asset::processTimeEventHandlers(): Attempted to process time event handlers while asset %d is not playing", _header->_id);
		return;
	}

	// TODO: Replace with a queue.
	uint currentTime = g_system->getMillis();
	const Common::Array<EventHandler *> &_timeHandlers = _header->_eventHandlers.getValOrDefault(kTimerEvent);
	for (EventHandler *timeEvent : _timeHandlers) {
		// Indeed float, not time.
		double timeEventInFractionalSeconds = timeEvent->_argumentValue.asFloat();
		uint timeEventInMilliseconds = timeEventInFractionalSeconds * 1000;
		bool timeEventAlreadyProcessed = timeEventInMilliseconds < _lastProcessedTime;
		bool timeEventNeedsToBeProcessed = timeEventInMilliseconds <= currentTime - _startTime;
		if (!timeEventAlreadyProcessed && timeEventNeedsToBeProcessed) {
			debugC(5, kDebugScript, "Asset::processTimeEventHandlers(): Running On Time handler for time %d ms", timeEventInMilliseconds);
			timeEvent->execute(_header->_id);
		}
	}
	_lastProcessedTime = currentTime - _startTime;
}

void Asset::runEventHandlerIfExists(EventType eventType, const ScriptValue &arg) {
	const Common::Array<EventHandler *> &_eventHandlers = _header->_eventHandlers.getValOrDefault(eventType);
	for (EventHandler *eventHandler : _eventHandlers) {
		const ScriptValue &argToCheck = eventHandler->_argumentValue;

		if (arg.getType() != argToCheck.getType()) {
			warning("Got event handler arg type %s, expected %s",
				scriptValueTypeToStr(arg.getType()), scriptValueTypeToStr(argToCheck.getType()));
			continue;
		}

		if (arg == argToCheck) {
			debugC(5, kDebugScript, "Executing handler for event type %s on asset %d", eventTypeToStr(eventType), _header->_id);
			eventHandler->execute(_header->_id);
			return;
		}
	}
	debugC(5, kDebugScript, "No event handler for event type %s on asset %d", eventTypeToStr(eventType), _header->_id);
}

void Asset::runEventHandlerIfExists(EventType eventType) {
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

		Common::Point currentPos = getTopLeft();
		int16 newX = currentPos.x + dx;
		int16 newY = currentPos.y + dy;
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
		returnValue.setToFloat(_header->_boundingBox.left);
		break;

	case kGetTopYMethod:
		assert(args.empty());
		returnValue.setToFloat(_header->_boundingBox.top);
		break;

	case kGetWidthMethod:
		assert(args.empty());
		returnValue.setToFloat(_header->_boundingBox.width());
		break;

	case kGetHeightMethod:
		assert(args.empty());
		returnValue.setToFloat(_header->_boundingBox.height());
		break;

	case kGetCenterXMethod: {
		int centerX = _header->_boundingBox.left + (_header->_boundingBox.width() / 2);
		returnValue.setToFloat(centerX);
		break;
	}

	case kGetCenterYMethod: {
		int centerY = _header->_boundingBox.top + (_header->_boundingBox.height() / 2);
		returnValue.setToFloat(centerY);
		break;
	}

	case kGetZCoordinateMethod:
		assert(args.empty());
		returnValue.setToFloat(_header->_zIndex);
		break;

	case kIsVisibleMethod:
		assert(args.empty());
		returnValue.setToBool(isVisible());
		break;

	default:
		Asset::callMethod(methodId, args);
	}
	return returnValue;
}

void SpatialEntity::moveTo(int16 x, int16 y) {
	Common::Point dest(x, y);
	if (dest == getTopLeft()) {
		// We aren't actually moving anywhere.
		return;
	}

	if (isVisible()) {
		invalidateLocalBounds();
	}
	_header->_boundingBox.moveTo(dest);
	if (isVisible()) {
		invalidateLocalBounds();
	}
}

void SpatialEntity::moveToCentered(int16 x, int16 y) {
	int16 targetX = x - (_header->_boundingBox.width() / 2);
	int16 targetY = y - (_header->_boundingBox.height() / 2);
	moveTo(targetX, targetY);
}

void SpatialEntity::setBounds(const Common::Rect &bounds) {
	if (_header->_boundingBox == bounds) {
		// We aren't actually moving anywhere.
		return;
	}

	if (isVisible()) {
		invalidateLocalBounds();
	}
	_header->_boundingBox = bounds;
	if (isVisible()) {
		invalidateLocalBounds();
	}
}

void SpatialEntity::setZIndex(int zIndex) {
	if (_header->_zIndex == zIndex) {
		// We aren't actually moving anywhere.
		return;
	}

	_header->_zIndex = zIndex;
	invalidateLocalZIndex();
}

void SpatialEntity::invalidateLocalBounds() {
	g_engine->_dirtyRects.push_back(_header->_boundingBox);
}

void SpatialEntity::invalidateLocalZIndex() {
	warning("STUB: Asset::invalidateLocalZIndex()");
}

} // End of namespace MediaStation
