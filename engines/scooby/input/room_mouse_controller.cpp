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
#include "scooby/rooms/room_collision_probe.h"
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
	  _walkBestDistance(0), _walkStall(0), _walkInterfaceWait(0), _busySamples(0),
	  _walkPushToTarget(false), _walkEndsWithPush(false), _walkPathIndex(0), _walkReplans(0) {
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
				startWalk(state, static_cast<int16>(_clickPosition.x + state.CameraX),
						  static_cast<int16>(_clickPosition.y + state.CameraY), false);
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
	startWalk(state, static_cast<int16>(_clickPosition.x + state.CameraX),
			  static_cast<int16>(_clickPosition.y + state.CameraY), false);
}

void RoomMouseController::walkIntoClickedObject(RuntimeState &state, MouseInput &mouse) {
	_clickStage = ClickStage::None;
	int16 worldX;
	int16 worldY;
	Common::Rect shapeArea;
	if (!findWalkInTarget(state, _clickInteraction, worldX, worldY, shapeArea)) {
		worldX = static_cast<int16>(_clickPosition.x + state.CameraX);
		worldY = static_cast<int16>(_clickPosition.y + state.CameraY);
	}

	// Close the cursor (original A); the walk then lets the original walk-in check
	// (InteractionController::updateAutomaticInteraction) run once Shaggy reaches the object.
	mouse.pulse(ControllerButtons::A);
	startWalk(state, worldX, worldY, true, shapeArea);
}

bool RoomMouseController::findWalkInTarget(RuntimeState &state, int16 interaction, int16 &worldX,
										   int16 &worldY, Common::Rect &shapeArea) const {
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

	shapeArea = Common::Rect(static_cast<int16>(tiles.left * 8), static_cast<int16>(tiles.top * 8),
							 static_cast<int16>(tiles.right * 8), static_cast<int16>(tiles.bottom * 8));
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

void RoomMouseController::startWalk(RuntimeState &state, int16 worldX, int16 worldY, bool pushToTarget,
									const Common::Rect &goalArea) {
	_walking = true;
	_walkTargetX = worldX;
	_walkTargetY = worldY;
	_walkPushToTarget = pushToTarget;
	_walkGoalArea = goalArea;
	_walkInterfaceWait = 0;
	_walkReplans = 0;
	_suppressAutoOpen = true;
	if (!planWalkPath(state)) {
		_walkPath.clear();
		_walkPath.push_back(Common::Point(worldX, worldY));
		_walkEndsWithPush = true;
	}

	_walkPathIndex = 0;
	_walkBestDistance = 0x7FFFFFFF;
	_walkStall = 0;
}

ControllerButtons RoomMouseController::stepWalk(RuntimeState &state) {
	// The A press that closes the cursor lands first; the original ignores the d-pad while it is open.
	if ((state.DisplayFlags & kDisplayInterfaceMask) != 0) {
		if (++_walkInterfaceWait > kInterfaceWaitLimit) {
			_walking = false;
		}

		return ControllerButtons::None;
	}

	int leadX = integerCoordinate(state.ActorXFixedCoordinates[0]);
	int leadY = integerCoordinate(state.ActorYFixedCoordinates[0]);
	bool finalWaypoint = false;
	bool moveX = false;
	bool moveY = false;
	int deltaX = 0;
	int deltaY = 0;
	while (true) {
		// Pass each waypoint once Shaggy is close to it; only the destination needs the exact tolerances.
		const Common::Point &waypoint = _walkPath[_walkPathIndex];
		finalWaypoint = _walkPathIndex + 1 >= _walkPath.size();
		deltaX = waypoint.x - leadX;
		deltaY = waypoint.y - leadY;
		moveX = ABS(deltaX) > (finalWaypoint ? kArrivalToleranceX : kWaypointTolerance);
		moveY = ABS(deltaY) > (finalWaypoint ? kArrivalToleranceY : kWaypointTolerance);
		if (moveX || moveY || finalWaypoint) {
			break;
		}

		_walkPathIndex++;
		_walkBestDistance = 0x7FFFFFFF;
		_walkStall = 0;
	}

	if (!moveX && !moveY) {
		_walking = false;
		return ControllerButtons::None;
	}

	// The original diagonal movement barely advances vertically and gets no help sliding along walls, so the
	// planned route is followed one axis at a time: the axis with more travel left (vertical steps are half as
	// fast). Only the final push into a walk-in target holds both, as a pad player heading for it would.
	if (moveX && moveY && !(finalWaypoint && _walkEndsWithPush)) {
		if (ABS(deltaX) > 2 * ABS(deltaY)) {
			moveY = false;
		} else {
			moveX = false;
		}
	}

	ControllerButtons buttons = ControllerButtons::None;
	if (moveX) {
		buttons |= deltaX < 0 ? ControllerButtons::Left : ControllerButtons::Right;
	}

	if (moveY) {
		bool up = deltaY < 0;
		// Rooms with the inverted foreground polarity move Shaggy down the screen on Up.
		if ((state.TransitionFlags & kForegroundPolarityMask) != 0) {
			up = !up;
		}

		buttons |= up ? ControllerButtons::Up : ControllerButtons::Down;
	}

	// No progress: the original movement drifted off the planned route, so plan again from here a couple of
	// times; pushing into a walk-in target simply ends as near as it got.
	int distance = ABS(deltaX) + 2 * ABS(deltaY);
	if (distance < _walkBestDistance) {
		_walkBestDistance = distance;
		_walkStall = 0;
	} else if (++_walkStall > kWalkStallLimit) {
		if (!(finalWaypoint && _walkEndsWithPush) && _walkReplans < kMaxPathReplans && planWalkPath(state)) {
			_walkReplans++;
			_walkPathIndex = 0;
			_walkBestDistance = 0x7FFFFFFF;
			_walkStall = 0;
			return ControllerButtons::None;
		}

		_walking = false;
		return ControllerButtons::None;
	}

	return buttons;
}

bool RoomMouseController::planWalkPath(RuntimeState &state) {
	// Plans over the same point test the original lead-actor movement applies to Shaggy's position
	// (RoomCollisionProbe::probeLeadActorCollision), sampled on a kPathCellSize grid. The route only runs
	// horizontally and vertically (see stepWalk): a vertical step costs twice a horizontal one, matching
	// Shaggy's speeds, and every turn costs extra, so the route keeps to a few long straight legs.
	if (_rom == nullptr || !state.ActiveRoomScene) {
		return false;
	}

	const int16 maximumX = static_cast<int16>(state.RoomWidthTiles * 8 - 1);
	const int16 maximumY = static_cast<int16>(state.RoomHeightTiles * 8 - 1);
	if (maximumX <= 0 || maximumY <= 0) {
		return false;
	}

	RoomCollisionProbe probe(*_rom, state);
	const int columns = maximumX / kPathCellSize + 1;
	const int rows = maximumY / kPathCellSize + 1;
	const int cellCount = columns * rows;
	Common::Array<Common::Point> centers(cellCount);
	Common::Array<uint8> open(cellCount);
	for (int row = 0; row < rows; row++) {
		for (int column = 0; column < columns; column++) {
			int cell = row * columns + column;
			int centerX = MIN<int>(column * kPathCellSize + kPathCellSize / 2, maximumX);
			int centerY = MIN<int>(row * kPathCellSize + kPathCellSize / 2, maximumY);
			centers[cell] = Common::Point(static_cast<int16>(centerX), static_cast<int16>(centerY));
			open[cell] = probe.probeLeadActorCollision(centers[cell].x << 16, centers[cell].y << 16, maximumX,
													   maximumY).IsClear ? 1 : 0;
		}
	}

	const int leadX = CLIP<int>(integerCoordinate(state.ActorXFixedCoordinates[0]), 0, maximumX);
	const int leadY = CLIP<int>(integerCoordinate(state.ActorYFixedCoordinates[0]), 0, maximumY);
	const int startCell = (leadY / kPathCellSize) * columns + leadX / kPathCellSize;

	// Dijkstra over (cell, arrival direction) with circular buckets (Dial's algorithm): every edge costs at
	// most kVerticalStepCost + kTurnCost, so that many buckets plus one suffice.
	static const int kDirectionCount = 4;
	static const int kDirectionX[kDirectionCount] = {-1, 1, 0, 0};
	static const int kDirectionY[kDirectionCount] = {0, 0, -1, 1};
	static const uint32 kHorizontalStepCost = 2;
	static const uint32 kVerticalStepCost = 4;
	static const uint32 kTurnCost = 8;
	static const int kBucketCount = kVerticalStepCost + kTurnCost + 1;
	const uint32 unreached = 0xFFFFFFFF;
	Common::Array<uint32> cost(cellCount * kDirectionCount, unreached);
	Common::Array<int32> parent(cellCount * kDirectionCount, -1);
	Common::Array<int32> buckets[kBucketCount];
	int pending = 0;
	for (int direction = 0; direction < kDirectionCount; direction++) {
		cost[startCell * kDirectionCount + direction] = 0;
		buckets[0].push_back(startCell * kDirectionCount + direction);
		pending++;
	}

	for (uint32 current = 0; pending > 0; current++) {
		Common::Array<int32> &bucket = buckets[current % kBucketCount];
		while (!bucket.empty()) {
			int32 stateIndex = bucket.back();
			bucket.pop_back();
			pending--;
			if (cost[stateIndex] != current) {
				continue;
			}

			int cell = stateIndex / kDirectionCount;
			int arrival = stateIndex % kDirectionCount;
			int column = cell % columns;
			int row = cell / columns;
			// Shaggy starts off-grid: test the first steps from where he actually stands.
			int originX = cell == startCell ? leadX : centers[cell].x;
			int originY = cell == startCell ? leadY : centers[cell].y;
			for (int direction = 0; direction < kDirectionCount; direction++) {
				int nextColumn = column + kDirectionX[direction];
				int nextRow = row + kDirectionY[direction];
				if (nextColumn < 0 || nextColumn >= columns || nextRow < 0 || nextRow >= rows) {
					continue;
				}

				int nextCell = nextRow * columns + nextColumn;
				uint32 nextCost = current + (direction < 2 ? kHorizontalStepCost : kVerticalStepCost) +
								  (cell != startCell && direction != arrival ? kTurnCost : 0);
				int32 nextState = nextCell * kDirectionCount + direction;
				if (!open[nextCell] || cost[nextState] <= nextCost ||
					!isSegmentClear(probe, originX, originY, centers[nextCell].x, centers[nextCell].y, maximumX,
									maximumY)) {
					continue;
				}

				cost[nextState] = nextCost;
				parent[nextState] = stateIndex;
				buckets[nextCost % kBucketCount].push_back(nextState);
				pending++;
			}
		}
	}

	// A walk-in ends at the nearest reachable point inside the object's shape: entering it triggers the
	// walk-in (InteractionController::updateAutomaticInteraction), and a doorway set into a wall may only be
	// reachable along a narrow edge. Otherwise head for the clicked point when Shaggy can reach it, or for the
	// nearest point he can reach.
	const int targetX = CLIP<int>(_walkTargetX, 0, maximumX);
	const int targetY = CLIP<int>(_walkTargetY, 0, maximumY);
	int32 goalState = startCell * kDirectionCount;
	uint32 bestDistance = unreached;
	bool goalInArea = false;
	if (!_walkGoalArea.isEmpty()) {
		uint32 bestCost = unreached;
		for (int32 stateIndex = 0; stateIndex < cellCount * kDirectionCount; stateIndex++) {
			const Common::Point &center = centers[stateIndex / kDirectionCount];
			if (cost[stateIndex] < bestCost && _walkGoalArea.contains(center)) {
				bestCost = cost[stateIndex];
				goalState = stateIndex;
				goalInArea = true;
			}
		}
	}

	for (int32 stateIndex = 0; stateIndex < cellCount * kDirectionCount && !goalInArea; stateIndex++) {
		if (cost[stateIndex] == unreached) {
			continue;
		}

		const Common::Point &center = centers[stateIndex / kDirectionCount];
		int dx = center.x - targetX;
		int dy = center.y - targetY;
		uint32 distance = static_cast<uint32>(dx * dx + dy * dy);
		if (distance < bestDistance || (distance == bestDistance && cost[stateIndex] < cost[goalState])) {
			bestDistance = distance;
			goalState = stateIndex;
		}
	}

	Common::Point destination = centers[goalState / kDirectionCount];
	if (!goalInArea && probe.probeLeadActorCollision(targetX << 16, targetY << 16, maximumX, maximumY).IsClear &&
		isSegmentClear(probe, destination.x, destination.y, targetX, targetY, maximumX, maximumY)) {
		destination = Common::Point(static_cast<int16>(targetX), static_cast<int16>(targetY));
	}

	// Walk back from the goal and keep one waypoint per straight leg.
	Common::Array<int32> route;
	for (int32 stateIndex = goalState; stateIndex >= 0 && stateIndex / kDirectionCount != startCell;
		 stateIndex = parent[stateIndex]) {
		route.push_back(stateIndex);
	}

	_walkPath.clear();
	for (int i = static_cast<int>(route.size()) - 1; i > 0; i--) {
		if (route[i] % kDirectionCount != route[i - 1] % kDirectionCount) {
			_walkPath.push_back(centers[route[i] / kDirectionCount]);
		}
	}

	_walkPath.push_back(destination);
	_walkEndsWithPush = _walkPushToTarget && (destination.x != _walkTargetX || destination.y != _walkTargetY);
	if (_walkEndsWithPush) {
		_walkPath.push_back(Common::Point(_walkTargetX, _walkTargetY));
	}

	return true;
}

bool RoomMouseController::isSegmentClear(const RoomCollisionProbe &probe, int x0, int y0, int x1, int y1,
										 int16 maximumX, int16 maximumY) const {
	int steps = MAX(ABS(x1 - x0), ABS(y1 - y0));
	for (int step = 1; step <= steps; step++) {
		int x = x0 + ((x1 - x0) * step + (x1 >= x0 ? steps / 2 : -steps / 2)) / steps;
		int y = y0 + ((y1 - y0) * step + (y1 >= y0 ? steps / 2 : -steps / 2)) / steps;
		if (!probe.probeLeadActorCollision(x << 16, y << 16, maximumX, maximumY).IsClear) {
			return false;
		}
	}

	return true;
}

} // namespace Scooby
