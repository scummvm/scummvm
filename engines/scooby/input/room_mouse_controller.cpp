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

// MANAGED-ONLY MOUSE SUPPORT (see mouse_input.h and room_mouse_controller.h).

#include "room_mouse_controller.h"

#include "common/util.h"

#include "mouse_input.h"
#include "scooby/interactions/interaction_resolver.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/types.h"

namespace Scooby {

namespace {

int floorDivide(int value, int divisor) {
	return value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor);
}

int16 integerCoordinate(int32 fixedCoordinate) {
	return static_cast<int16>(fixedCoordinate >> 16);
}

} // namespace

RoomMouseController::RoomMouseController()
	: _rom(nullptr), _roomId(-1), _suppressAutoOpen(true), _clickStage(ClickStage::None), _clickPlaced(false),
	  _clickWait(0), _clickInteraction(0), _walking(false), _walkTargetX(0), _walkTargetY(0),
	  _walkBestDistance(0), _walkStall(0), _walkInterfaceWait(0), _busySamples(0) {
}

void RoomMouseController::reset() {
	_suppressAutoOpen = true;
	_clickStage = ClickStage::None;
	_walking = false;
	_busySamples = 0;
}

ControllerButtons RoomMouseController::updateIdle(RuntimeState &state, MouseInput &mouse) {
	// The open request lives for exactly one actor update; a room that keeps the cursor closed ignores it.
	state.MouseOpenInterfaceRequest = false;
	if (state.RoomId != _roomId) {
		reset();
		_roomId = state.RoomId;
	}

	_busySamples = 0;
	bool interfaceOpen = (state.DisplayFlags & kDisplayInterfaceMask) != 0;
	if (mouse.takeRightClick()) {
		_clickStage = ClickStage::None;
		if (_walking) {
			_walking = false;
		} else if (interfaceOpen) {
			mouse.pulse(ControllerButtons::A);
		}

		_suppressAutoOpen = true;
	}

	Common::Point click;
	if (mouse.takeLeftClick(click)) {
		if (_clickStage == ClickStage::DoubleClickWindow) {
			// A second click on the same spot walks into the object; elsewhere the first click stands alone.
			if (ABS(click.x - _clickPosition.x) <= kDoubleClickDistance &&
				ABS(click.y - _clickPosition.y) <= kDoubleClickDistance) {
				walkIntoClickedObject(state, mouse);
				return ControllerButtons::None;
			}

			_clickStage = ClickStage::None;
			mouse.pulse(ControllerButtons::B);
			return ControllerButtons::None;
		}

		_walking = false;
		_clickStage = ClickStage::Placing;
		_clickPlaced = false;
		_clickWait = 0;
		_clickPosition = click;
		_suppressAutoOpen = false;
	}

	if (mouse.takeMoved()) {
		_suppressAutoOpen = false;
	}

	if (_walking) {
		return stepWalk(state);
	}

	if (!interfaceOpen) {
		if (_clickStage == ClickStage::DoubleClickWindow) {
			_clickStage = ClickStage::None;
		}

		// Ask the original open path to show the cursor (honoured only where an A/B press would open it).
		if (_clickStage == ClickStage::Placing || !_suppressAutoOpen) {
			state.MouseOpenInterfaceRequest = true;
		}

		// A room that keeps the cursor closed (scripted suppression) still lets the click walk.
		if (_clickStage == ClickStage::Placing && ++_clickWait > kInterfaceWaitLimit) {
			_clickStage = ClickStage::None;
			if (_clickPosition.y < kInterfaceTopY) {
				startWalk(static_cast<int16>(_clickPosition.x + state.CameraX),
						  static_cast<int16>(_clickPosition.y + state.CameraY));
			}
		}

		return ControllerButtons::None;
	}

	switch (_clickStage) {
	case ClickStage::None:
		placeCursor(state, mouse.contentPosition());
		break;
	case ClickStage::Placing:
		placeCursor(state, _clickPosition);
		if (!_clickPlaced) {
			_clickPlaced = true;
		} else {
			resolveClick(state, mouse);
		}
		break;
	case ClickStage::DoubleClickWindow:
		// Keep the cursor on the clicked object so the delayed B still targets it.
		placeCursor(state, _clickPosition);
		if (++_clickWait >= kDoubleClickWindow) {
			_clickStage = ClickStage::None;
			mouse.pulse(ControllerButtons::B);
		}
		break;
	}

	return ControllerButtons::None;
}

void RoomMouseController::updateBusy(RuntimeState &state) {
	state.MouseOpenInterfaceRequest = false;
	_clickStage = ClickStage::None;
	if (_walking && ++_busySamples > kBusyLimit) {
		_walking = false;
	}
}

void RoomMouseController::resolveClick(RuntimeState &state, MouseInput &mouse) {
	_clickStage = ClickStage::None;
	if (_clickPosition.y >= kInterfaceTopY) {
		mouse.pulse(isBarArrow(state, _clickPosition) ? ControllerButtons::C : ControllerButtons::B);
		return;
	}

	if ((state.InteractionFlags & kAlternateInteractionMask) != 0) {
		mouse.pulse(ControllerButtons::B);
		return;
	}

	if (state.CurrentInteraction != 0) {
		_clickInteraction = state.CurrentInteraction;
		// B would use the chosen verb, else the default one (activeVerbIcon, else cachedVerbIcon). An open
		// door's default verb is SHUT: walk into it instead.
		if (state.activeVerbIcon == NONE && state.cachedVerbIcon == SHUT) {
			walkIntoClickedObject(state, mouse);
			return;
		}

		_clickStage = ClickStage::DoubleClickWindow;
		_clickWait = 0;
		return;
	}

	// Nothing to interact with: close the cursor (original A) and walk to the point.
	mouse.pulse(ControllerButtons::A);
	startWalk(static_cast<int16>(_clickPosition.x + state.CameraX),
			  static_cast<int16>(_clickPosition.y + state.CameraY));
}

void RoomMouseController::walkIntoClickedObject(RuntimeState &state, MouseInput &mouse) {
	_clickStage = ClickStage::None;
	int16 worldX;
	int16 worldY;
	if (!findWalkInTarget(state, _clickInteraction, worldX, worldY)) {
		worldX = static_cast<int16>(_clickPosition.x + state.CameraX);
		worldY = static_cast<int16>(_clickPosition.y + state.CameraY);
	}

	// Close the cursor (original A); the walk then lets the original walk-in check
	// (InteractionController::updateAutomaticInteraction) run once Shaggy reaches the object.
	mouse.pulse(ControllerButtons::A);
	startWalk(worldX, worldY);
}

bool RoomMouseController::findWalkInTarget(RuntimeState &state, int16 interaction, int16 &worldX,
										   int16 &worldY) const {
	// InteractionResolver::findAutomaticInteraction tests Shaggy's tile against a dynamic object's shape,
	// and a doorway's walk-in only leads through once Shaggy is well inside it. So walk through the shape
	// towards its far side along the axis Shaggy approaches from (the original collision stops him where a
	// pad player would be stopped). Fixed (actor-backed) objects and the two actors have no such shape; the
	// caller walks to the click point instead.
	int index = interaction - kRoomObjectFirstInteraction;
	if (_rom == nullptr || index < 0 || index >= static_cast<int>(state.RoomObjects.size())) {
		return false;
	}

	const RoomObject &roomObject = state.RoomObjects[static_cast<size_t>(index)];
	Common::Rect tiles;
	if ((roomObject.Flags & kFixedRoomObjectMask) != 0 ||
		!InteractionResolver(*_rom, state).getDynamicObjectTileBounds(roomObject, tiles) || tiles.isEmpty()) {
		return false;
	}

	int leadX = integerCoordinate(state.ActorXFixedCoordinates[0]);
	int leadY = integerCoordinate(state.ActorYFixedCoordinates[0]);
	int minimumX = tiles.left * 8 + 4;
	int maximumX = tiles.right * 8 - 4;
	int minimumY = tiles.top * 8 + 4;
	int maximumY = tiles.bottom * 8 - 4;
	int outsideX = leadX < minimumX ? minimumX - leadX : MAX(leadX - maximumX, 0);
	int outsideY = leadY < minimumY ? minimumY - leadY : MAX(leadY - maximumY, 0);
	if (outsideX >= outsideY) {
		worldX = static_cast<int16>(leadX <= (minimumX + maximumX) / 2 ? maximumX : minimumX);
		worldY = static_cast<int16>(CLIP(leadY, minimumY, maximumY));
	} else {
		worldX = static_cast<int16>(CLIP(leadX, minimumX, maximumX));
		worldY = static_cast<int16>(leadY <= (minimumY + maximumY) / 2 ? maximumY : minimumY);
	}

	return true;
}

void RoomMouseController::placeCursor(RuntimeState &state, const Common::Point &target) const {
	if (target.y < kInterfaceTopY) {
		state.CursorX = static_cast<int16>(CLIP<int>(target.x - kCrosshairHotspot, kCursorMinimumX, kCursorMaximumX));
		state.CursorY = static_cast<int16>(CLIP<int>(target.y - kCrosshairHotspot, 0, kInterfaceTopY - 1));
		return;
	}

	if (isBarArrow(state, target)) {
		// The C arrow has no original highlight; park the cursor at the top of the room (CursorY < 8 is never
		// resolved) so no verb is highlighted while the host crosshair points at the arrow.
		state.CursorY = 0;
		return;
	}

	// Snap to the centre the original pad movement uses for the hovered bar entry, so the highlight sprite
	// (RoomSpriteRenderer) and MenuCommand (RoomActorUpdater) agree on it.
	bool verbBar = (state.DisplayFlags & kDisplayRightInterfaceMask) == 0;
	int column = CLIP(floorDivide(target.x - (verbBar ? 16 : 8), kBarColumnWidth), 0, kBarColumnCount - 1);
	state.CursorX = static_cast<int16>(column * kBarColumnWidth + (verbBar ? 0x1C : 0x14));
	state.CursorY = static_cast<int16>(target.y >= kInterfaceSecondRowY ? kInterfaceSecondRowY + 3 : 0xB4);
}

bool RoomMouseController::isBarArrow(const RuntimeState &state, const Common::Point &target) const {
	if (target.y < kInterfaceTopY) {
		return false;
	}

	bool verbBar = (state.DisplayFlags & kDisplayRightInterfaceMask) == 0;
	int column = CLIP(floorDivide(target.x - (verbBar ? 16 : 8), kBarColumnWidth), 0, kBarColumnCount - 1);
	return verbBar ? column == kBarColumnCount - 1 : column == 0;
}

void RoomMouseController::startWalk(int16 worldX, int16 worldY) {
	_walking = true;
	_walkTargetX = worldX;
	_walkTargetY = worldY;
	_walkBestDistance = 0x7FFFFFFF;
	_walkStall = 0;
	_walkInterfaceWait = 0;
	_suppressAutoOpen = true;
}

ControllerButtons RoomMouseController::stepWalk(RuntimeState &state) {
	// The A press that closes the cursor lands first; the original ignores the d-pad while it is open.
	if ((state.DisplayFlags & kDisplayInterfaceMask) != 0) {
		if (++_walkInterfaceWait > kInterfaceWaitLimit) {
			_walking = false;
		}

		return ControllerButtons::None;
	}

	int deltaX = _walkTargetX - integerCoordinate(state.ActorXFixedCoordinates[0]);
	int deltaY = _walkTargetY - integerCoordinate(state.ActorYFixedCoordinates[0]);
	ControllerButtons buttons = ControllerButtons::None;
	if (deltaX < -kArrivalToleranceX) {
		buttons |= ControllerButtons::Left;
	} else if (deltaX > kArrivalToleranceX) {
		buttons |= ControllerButtons::Right;
	}

	if (ABS(deltaY) > kArrivalToleranceY) {
		bool up = deltaY < 0;
		// Rooms with the inverted foreground polarity move Shaggy down the screen on Up.
		if ((state.TransitionFlags & kForegroundPolarityMask) != 0) {
			up = !up;
		}

		buttons |= up ? ControllerButtons::Up : ControllerButtons::Down;
	}

	if (buttons == ControllerButtons::None) {
		_walking = false;
		return buttons;
	}

	// Blocked: stop as near as the original movement got (vertical steps are half as fast).
	int distance = ABS(deltaX) + 2 * ABS(deltaY);
	if (distance < _walkBestDistance) {
		_walkBestDistance = distance;
		_walkStall = 0;
	} else if (++_walkStall > kWalkStallLimit) {
		_walking = false;
		return ControllerButtons::None;
	}

	return buttons;
}

} // namespace Scooby
