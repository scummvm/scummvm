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
#include "mediastation/actors/hotspot.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

void HotspotActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderMouseActiveArea: {
		uint16 total_points = chunk.readTypedUint16();
		for (int i = 0; i < total_points; i++) {
			Common::Point point = chunk.readTypedPoint();
			_mouseActiveArea.push_back(point);
		}
		break;
	}

	case kActorHeaderStartup:
		_isActive = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderCursorResourceId:
		_cursorResourceId = chunk.readTypedUint16();
		break;

	case kActorHeaderGetOffstageEvents:
		_getOffstageEvents = static_cast<bool>(chunk.readTypedByte());
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

bool HotspotActor::isInside(const Common::Point &pointToCheck) {
	// No sense checking the polygon if we're not even in the bbox.
	if (!_boundingBox.contains(pointToCheck)) {
		return false;
	}

	// We're in the bbox, but there might not be a polygon to check.
	if (_mouseActiveArea.empty()) {
		return true;
	}

	// Polygon intersection code adapted from HADESCH engine, might need more
	// refinement once more testing is possible.
	Common::Point point = pointToCheck - Common::Point(_boundingBox.left, _boundingBox.top);
	int rcross = 0; // Number of right-side overlaps

	// Each edge is checked whether it cuts the outgoing stream from the point
	Common::Array<Common::Point> _polygon = _mouseActiveArea;
	for (unsigned i = 0; i < _polygon.size(); i++) {
		const Common::Point &edgeStart = _polygon[i];
		const Common::Point &edgeEnd = _polygon[(i + 1) % _polygon.size()];

		// A vertex is a point? Then it lies on one edge of the polygon
		if (point == edgeStart)
			return true;

		if ((edgeStart.y > point.y) != (edgeEnd.y > point.y)) {
			int term1 = (edgeStart.x - point.x) * (edgeEnd.y - point.y) - (edgeEnd.x - point.x) * (edgeStart.y - point.y);
			int term2 = (edgeEnd.y - point.y) - (edgeStart.y - edgeEnd.y);
			if ((term1 > 0) == (term2 >= 0))
				rcross++;
		}
	}

	// The point is strictly inside the polygon if and only if the number of overlaps is odd
	return ((rcross % 2) == 1);
}

ScriptValue HotspotActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kMouseActivateMethod: {
		assert(args.empty());
		activate();
		return returnValue;
	}

	case kMouseDeactivateMethod: {
		assert(args.empty());
		deactivate();
		return returnValue;
	}

	case kIsActiveMethod: {
		assert(args.empty());
		returnValue.setToBool(_isActive);
		return returnValue;
	}

	case kTriggerAbsXPositionMethod: {
		double mouseX = static_cast<double>(g_system->getEventManager()->getMousePos().x);
		returnValue.setToFloat(mouseX);
		return returnValue;
	}

	case kTriggerAbsYPositionMethod: {
		double mouseY = static_cast<double>(g_system->getEventManager()->getMousePos().y);
		returnValue.setToFloat(mouseY);
		return returnValue;
	}

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
}

uint16 HotspotActor::findActorToAcceptMouseEvents(
	const Common::Point &point,
	uint16 eventMask,
	MouseActorState &state,
	bool clipMouseEvents) {

	uint16 result = 0;
	if (isActive()) {
		if (isInside(point)) {
			if (eventMask & kMouseDownFlag) {
				state.mouseDown = this;
				result |= kMouseDownFlag;
			}

			if (eventMask & kMouseEnterFlag) {
				state.mouseEnter = this;
				result |= kMouseEnterFlag;
			}

			if (eventMask & kMouseMovedFlag) {
				state.mouseMoved = this;
				result |= kMouseMovedFlag;
			}
		}

		if (this == g_engine->getMouseInsideHotspot() && (eventMask & kMouseExitFlag)) {
			state.mouseExit = this;
			result |= kMouseExitFlag;
		}

		if (this == g_engine->getMouseDownHotspot() && (eventMask & kMouseUpFlag)) {
			state.mouseUp = this;
			result |= kMouseUpFlag;
		}
	} else {
		debugC(5, kDebugEvents, "%s: %d: Inactive", __func__, id());
	}

	return result;
}

void HotspotActor::activate() {
	if (!_isActive) {
		_isActive = true;
		invalidateMouse();
	}
}

void HotspotActor::deactivate() {
	if (_isActive) {
		_isActive = false;
		if (g_engine->getMouseDownHotspot() == this) {
			g_engine->setMouseDownHotspot(nullptr);
		}
		if (g_engine->getMouseInsideHotspot() == this) {
			g_engine->setMouseDownHotspot(nullptr);
		}

		invalidateMouse();
	}
}

void HotspotActor::mouseDownEvent(const Common::Event &event) {
	if (!_isActive) {
		warning("%s: Called on inactive hotspot", __func__);
		return;
	}

	g_engine->setMouseDownHotspot(this);
	runEventHandlerIfExists(kMouseDownEvent);
}

void HotspotActor::mouseUpEvent(const Common::Event &event) {
	if (!_isActive) {
		warning("%s: Called on inactive hotspot", __func__);
		return;
	}

	g_engine->setMouseDownHotspot(nullptr);
	runEventHandlerIfExists(kMouseUpEvent);
}

void HotspotActor::mouseEnteredEvent(const Common::Event &event) {
	if (!_isActive) {
		warning("%s: Called on inactive hotspot", __func__);
		return;
	}

	g_engine->setMouseInsideHotspot(this);
	if (_cursorResourceId != 0) {
		debugC(5, kDebugEvents, "%s: Setting cursor %d for asset %d", __func__, _cursorResourceId, id());
		g_engine->getCursorManager()->setAsTemporary(_cursorResourceId);
	} else {
		debugC(5, kDebugEvents, "%s: Unsetting cursor for asset %d", __func__, id());
		g_engine->getCursorManager()->unsetTemporary();
	}

	runEventHandlerIfExists(kMouseEnteredEvent);
}

void HotspotActor::mouseMovedEvent(const Common::Event &event) {
	if (!_isActive) {
		warning("%s: Called on inactive hotspot", __func__);
		return;
	}

	runEventHandlerIfExists(kMouseMovedEvent);
}

void HotspotActor::mouseExitedEvent(const Common::Event &event) {
	if (!_isActive) {
		warning("%s: Called on inactive hotspot", __func__);
		return;
	}

	g_engine->setMouseInsideHotspot(nullptr);
	runEventHandlerIfExists(kMouseExitedEvent);
}

} // End of namespace MediaStation
