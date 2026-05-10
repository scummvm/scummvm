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
	case kActorHeaderMouseActiveArea:
		_mouseActiveArea.loadFromParameterStream(chunk);
		break;

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

bool HotspotActor::inBounds(const Common::Point &point) {
	if (_parentStage != nullptr) {
		return _parentStage->inBounds(point, getBbox(), _mouseActiveArea);
	}
	return false;
}

ScriptValue HotspotActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kMouseActivateMethod: {
		ARGCOUNTCHECK(0);
		activate();
		break;
	}

	case kMouseDeactivateMethod: {
		ARGCOUNTCHECK(0);
		deactivate();
		break;
	}


	case kIsPointInsideMethod: {
		ARGCOUNTCHECK(2);
		int16 xToCheck = static_cast<int16>(args[0].asFloat());
		int16 yToCheck = static_cast<int16>(args[1].asFloat());
		Common::Point pointToCheck(xToCheck, yToCheck);
		bool pointIsInside = inBounds(pointToCheck);
		returnValue.setToBool(pointIsInside);
		break;
	}

	case kTriggerAbsXPositionMethod: {
		ARGCOUNTCHECK(0);
		double mouseX = static_cast<double>(g_system->getEventManager()->getMousePos().x);
		returnValue.setToFloat(mouseX);
		break;
	}

	case kTriggerAbsYPositionMethod: {
		ARGCOUNTCHECK(0);
		double mouseY = static_cast<double>(g_system->getEventManager()->getMousePos().y);
		returnValue.setToFloat(mouseY);
		break;
	}

	case kIsActiveMethod: {
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_isActive);
		break;
	}

	default:
		returnValue = SpatialEntity::callMethod(methodId, args);
	}
	return returnValue;
}

uint16 HotspotActor::findActorToAcceptMouseEvents(
	const Common::Point &point,
	uint16 eventMask,
	MouseActorState &state,
	bool clipMouseEvents) {

	uint16 result = 0;
	if (isActive()) {
		if (clipMouseEvents && !_getOffstageEvents) {
			eventMask &= ~(kMouseDownFlag | kMouseMovedFlag | kMouseEnterFlag | kMouseUnk1Flag);
		}

		if (inBounds(point)) {
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
		debugC(8, kDebugEvents, "[%s] %s: Inactive", debugName(),  __func__);
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
			g_engine->setMouseInsideHotspot(nullptr);
		}

		invalidateMouse();
	}
}

void HotspotActor::mouseDownEvent(const MouseEvent &event) {
	if (!_isActive) {
		warning("[%s] %s: Inactive", debugName(), __func__);
		return;
	}

	g_engine->setMouseDownHotspot(this);
	runScriptResponseIfExists(kMouseDownEvent);
}

void HotspotActor::mouseUpEvent(const MouseEvent &event) {
	if (!_isActive) {
		warning("[%s] %s: Inactive", debugName(), __func__);
		return;
	}

	g_engine->setMouseDownHotspot(nullptr);
	runScriptResponseIfExists(kMouseUpEvent);
}

void HotspotActor::mouseEnteredEvent(const MouseEvent &event) {
	if (!_isActive) {
		warning("[%s] %s: Inactive", debugName(), __func__);
		return;
	}

	g_engine->setMouseInsideHotspot(this);
	if (_cursorResourceId != 0) {
		debugC(5, kDebugEvents, "[%s] %s: Setting cursor %d", debugName(), __func__, _cursorResourceId);
		g_engine->getCursorManager()->setAsTemporary(_cursorResourceId);
	} else {
		debugC(5, kDebugEvents, "[%s] %s: Unsetting cursor", debugName(), __func__);
		g_engine->getCursorManager()->unsetTemporary();
	}

	runScriptResponseIfExists(kMouseEnteredEvent);
}

void HotspotActor::mouseMovedEvent(const MouseEvent &event) {
	if (!_isActive) {
		warning("[%s] %s: Inactive", debugName(), __func__);
		return;
	}

	runScriptResponseIfExists(kMouseMovedEvent);
}

void HotspotActor::mouseExitedEvent(const MouseEvent &event) {
	if (!_isActive) {
		warning("[%s] %s: Inactive", debugName(), __func__);
		return;
	}

	g_engine->setMouseInsideHotspot(nullptr);
	runScriptResponseIfExists(kMouseExitedEvent);
}

} // End of namespace MediaStation
