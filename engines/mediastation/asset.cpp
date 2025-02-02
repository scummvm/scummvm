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

namespace MediaStation {

Asset::~Asset() {
	delete _header;
	_header = nullptr;
}

void Asset::readChunk(Chunk &chunk) {
	error("Asset::readChunk(): Chunk reading for asset type 0x%x is not implemented", static_cast<uint>(_header->_type));
}

void Asset::readSubfile(Subfile &subfile, Chunk &chunk) {
	error("Asset::readSubfile(): Subfile reading for asset type 0x%x is not implemented", static_cast<uint>(_header->_type));
}

AssetType Asset::type() const {
	return _header->_type;
}

int Asset::zIndex() const {
	return _header->_zIndex;
}

Common::Rect *Asset::getBbox() {
	return _header->_boundingBox;
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
	for (EventHandler *timeEvent : _header->_timeHandlers) {
		double timeEventInFractionalSeconds = timeEvent->_argumentValue.u.f;
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

void Asset::runEventHandlerIfExists(EventType eventType) {
	EventHandler *eventHandler = _header->_eventHandlers.getValOrDefault(eventType);
	if (eventHandler != nullptr) {
		debugC(5, kDebugScript, "Executing handler for event type %d on asset %d", static_cast<uint>(eventType), _header->_id);
		eventHandler->execute(_header->_id);
	} else {
		debugC(5, kDebugScript, "No event handler for event type %d on asset %d", static_cast<uint>(eventType), _header->_id);
	}
}

void Asset::runKeyDownEventHandlerIfExists(Common::KeyState keyState) {
	EventHandler *keyDownEvent = _header->_keyDownHandlers.getValOrDefault(keyState.ascii);
	if (keyDownEvent != nullptr) {
		debugC(5, kDebugScript, "Executing keydown event handler for ASCII code %d on asset %d", keyState.ascii, _header->_id);
		keyDownEvent->execute(_header->_id);
	} else {
		debugC(5, kDebugScript, "No keydown event handler for ASCII code %d on asset %d", keyState.ascii, _header->_id);
	}
}

} // End of namespace MediaStation
