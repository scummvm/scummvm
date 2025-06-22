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

#include "mediastation/asset.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediascript/scriptconstants.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

Asset::~Asset() {
	for (auto it = _eventHandlers.begin(); it != _eventHandlers.end(); ++it) {
		Common::Array<EventHandler *> &handlersForType = it->_value;
		for (EventHandler *handler : handlersForType) {
			delete handler;
		}
		handlersForType.clear();
	}
	_eventHandlers.clear();
}

void Asset::initFromParameterStream(Chunk &chunk) {
	AssetHeaderSectionType paramType = kAssetHeaderEmptySection;
	while (true) {
		paramType = static_cast<AssetHeaderSectionType>(chunk.readTypedUint16());
		if (paramType == 0) {
			break;
		} else {
			readParameter(chunk, paramType);
		}
	}
}

void Asset::readParameter(Chunk &chunk, AssetHeaderSectionType paramType) {
	switch (paramType) {
	case kAssetHeaderEventHandler: {
		EventHandler *eventHandler = new EventHandler(chunk);
		Common::Array<EventHandler *> &eventHandlersForType = _eventHandlers.getOrCreateVal(eventHandler->_type);

		// This is not a hashmap because we don't want to have to hash ScriptValues.
		for (EventHandler *existingEventHandler : eventHandlersForType) {
			if (existingEventHandler->_argumentValue == eventHandler->_argumentValue) {
				error("AssetHeader::readSection(): Event handler for %s (%s) already exists",
					  eventTypeToStr(eventHandler->_type), eventHandler->getDebugHeader().c_str());
			}
		}
		eventHandlersForType.push_back(eventHandler);
		break;
	}

	default:
		error("Got unimplemented asset parameter 0x%x", static_cast<uint>(paramType));
	}
}

ScriptValue Asset::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	error("Got unimplemented method call %d (%s)", static_cast<uint>(methodId), builtInMethodToStr(methodId));
}

void Asset::readChunk(Chunk &chunk) {
	error("Asset::readChunk(): Chunk reading for asset type 0x%x is not implemented", static_cast<uint>(_type));
}

void Asset::readSubfile(Subfile &subfile, Chunk &chunk) {
	error("Asset::readSubfile(): Subfile reading for asset type 0x%x is not implemented", static_cast<uint>(_type));
}

void Asset::processTimeEventHandlers() {
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
			debugC(5, kDebugScript, "Asset::processTimeEventHandlers(): Running On Time handler for time %d ms", timeEventInMilliseconds);
			timeEvent->execute(_id);
		}
	}
	_lastProcessedTime = currentTime - _startTime;
}

void Asset::runEventHandlerIfExists(EventType eventType, const ScriptValue &arg) {
	const Common::Array<EventHandler *> &eventHandlers = _eventHandlers.getValOrDefault(eventType);
	for (EventHandler *eventHandler : eventHandlers) {
		const ScriptValue &argToCheck = eventHandler->_argumentValue;

		if (arg.getType() != argToCheck.getType()) {
			warning("Got event handler arg type %s, expected %s",
					scriptValueTypeToStr(arg.getType()), scriptValueTypeToStr(argToCheck.getType()));
			continue;
		}

		if (arg == argToCheck) {
			debugC(5, kDebugScript, "Executing handler for event type %s on asset %d", eventTypeToStr(eventType), _id);
			eventHandler->execute(_id);
			return;
		}
	}
	debugC(5, kDebugScript, "No event handler for event type %s on asset %d", eventTypeToStr(eventType), _id);
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
		int centerX = _boundingBox.left + (_boundingBox.width() / 2);
		returnValue.setToFloat(centerX);
		break;
	}

	case kGetCenterYMethod: {
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

	default:
		Asset::callMethod(methodId, args);
	}
	return returnValue;
}

void SpatialEntity::readParameter(Chunk &chunk, AssetHeaderSectionType paramType) {
	switch (paramType) {
	case kAssetHeaderBoundingBox:
		_boundingBox = chunk.readTypedRect();
		break;

	case kAssetHeaderDissolveFactor:
		_dissolveFactor = chunk.readTypedDouble();
		break;

	case kAssetHeaderZIndex:
		_zIndex = chunk.readTypedGraphicUnit();
		break;

	case kAssetHeaderTransparency:
		_hasTransparency = static_cast<bool>(chunk.readTypedByte());
		break;

	case kAssetHeaderStageId:
		_stageId = chunk.readTypedUint16();
		break;

	case kAssetHeaderAssetReference:
		_assetReference = chunk.readTypedUint16();
		break;

	default:
		Asset::readParameter(chunk, paramType);
	}
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
}

void SpatialEntity::setZIndex(int zIndex) {
	if (_zIndex == zIndex) {
		// We aren't actually moving anywhere.
		return;
	}

	_zIndex = zIndex;
	invalidateLocalZIndex();
}

void SpatialEntity::setDissolveFactor(double dissolveFactor) {
	CLIP(dissolveFactor, 0.0, 1.0);
	if (dissolveFactor != _dissolveFactor) {
		_dissolveFactor = dissolveFactor;
		invalidateLocalBounds();
	}
}

void SpatialEntity::invalidateLocalBounds() {
	g_engine->addDirtyRect(getBbox());
}

void SpatialEntity::invalidateLocalZIndex() {
	warning("STUB: Asset::invalidateLocalZIndex()");
}

} // End of namespace MediaStation
