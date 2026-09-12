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

#include "hopkins/base_enhancements_autoplay.h"

#include "hopkins/base_data.h"
#include "hopkins/base_engine.h"
#include "hopkins/base_types.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/translation.h"
#include "common/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Hopkins {

namespace {

struct AutoplayDestination {
	int roomId;
	int exitMapPos;
	int approachMapPos;
	const char *label;
	const char *mapLabel;
};

// WBASE transition cells and the adjacent cells used to approach them.
static const AutoplayDestination kAutoplayDestinations[] = {
	{ 94, 0x0860, 0x0861, _s("Exit 1: Containers room"), _s("Containers") },
	{ 95, 0x087a, 0x0879, _s("Exit 2: Turbines room"), _s("Turbines") },
	{ 96, 0x01df, 0x021f, _s("Exit 3: Office"), _s("Office") },
	{ 97, 0x05ab, 0x05eb, _s("Exit 4: Laboratory"), _s("Laboratory") },
	{ 98, 0x0bab, 0x0b6b, _s("Exit 5: Resurrection room"), _s("Resurrection") },
	{ 99, 0x01fb, 0x023b, _s("Exit 6: Dome"), _s("Dome") }
};

static const int kAutoplayDestinationCount = ARRAYSIZE(kAutoplayDestinations);
static const int kAutoplayAimTolerance = 24;
static const int kAutoplayMoveTolerance = 48;
static const int kAutoplayWaypointTolerance = 12;
// Keep a four-unit clearance from diagonal corner blockers.
static const int kAutoplayCornerLaneHalfWidth = 4;
static const int kAutoplayCornerLaneBias = 16;
static const int kAutoplayInitialCombatDistance = 600;
static const int kAutoplayNearbyCombatDistance = 128;
static const int kAutoplayFinalCombatHalfAngle = 240;
// Reach exit-first combat priority before the 100-second route limit.
static const int kAutoplayCombatPriorityTicks = 60 * 24;
// At 24 Hz, 60 ticks permits a 180-degree turn before recovery.
static const int kAutoplayStuckTicks = 60;
static const int kAutoplayMaximumTicks = 100 * 24;
static const int kAutoplayRecoveryReverseTicks = 6;
static const int kAutoplayRecoveryTurnTicks = 12;
static const int kAutoplayRecoveryAdvanceTicks = 8;
static const int kAutoplayMenuLeft = 42;
static const int kAutoplayMenuTop = 25;
static const int kAutoplayMenuRight = 277;
static const int kAutoplayMenuBottom = 180;
static const int kAutoplayMenuChoiceTop = 62;
static const int kAutoplayMenuChoiceSpacing = 14;
static const int kAutoplayMenuChoiceHeight = 11;

enum AutoplayRecoveryPhase {
	kAutoplayRecoveryNone,
	kAutoplayRecoveryReverse,
	kAutoplayRecoveryTurn,
	kAutoplayRecoveryAdvance
};

static int absoluteValue(int value) {
	return value < 0 ? -value : value;
}

static int signedAngleDelta(int from, int to) {
	int delta = normalizeBaseAngle(to - from);
	if (delta > kBaseHalfTurn)
		delta -= kBaseAngleCount;
	return delta;
}

static bool traversableEdge(uint16 code) {
	if (code == 0 || (code & kBaseWallPass))
		return true;
	const byte lowCode = code & 0xff;
	return (lowCode == kBaseDoorXCode || lowCode == kBaseDoorYCode) && !(code & kBaseDoorLocked);
}

static bool canStep(const BaseEngine &engine, int from, int to) {
	const int fromX = from & 63;
	const int fromY = from >> 6;
	const int toX = to & 63;
	const int toY = to >> 6;
	if (toX < 0 || toX >= kBaseMapWidth || toY < 0 || toY >= kBaseMapHeight)
		return false;

	if (toX == fromX + 1 && toY == fromY)
		return traversableEdge(engine.xGrid()[from + 1]);
	if (toX == fromX - 1 && toY == fromY)
		return traversableEdge(engine.xGrid()[from]);
	if (toX == fromX && toY == fromY + 1)
		return traversableEdge(engine.yGrid()[from + kBaseMapWidth]);
	if (toX == fromX && toY == fromY - 1)
		return traversableEdge(engine.yGrid()[from]);
	return false;
}

static bool blockingGridCode(const uint16 *grid, int mapPos) {
	if (mapPos < 0 || mapPos >= kBaseAckGridArray)
		return true;
	const uint16 code = grid[mapPos];
	return code && !(code & kBaseWallPass);
}

static bool blockingCorner(const BaseEngine &engine, int xGridPos, int yGridPos) {
	return blockingGridCode(engine.xGrid(), xGridPos) || blockingGridCode(engine.yGrid(), yGridPos);
}

static void routeCornerLaneBias(const BaseEngine &engine, int from, int to, int &biasX, int &biasY) {
	biasX = 0;
	biasY = 0;
	const int centerX = (from & 63) * kBaseCellSize + kBaseCellSize / 2;
	const int centerY = (from >> 6) * kBaseCellSize + kBaseCellSize / 2;
	if (to == from + 1) {
		if (engine.playerY() < centerY - kAutoplayCornerLaneHalfWidth &&
				blockingCorner(engine, from + 1 - kBaseMapWidth, from + 1))
			biasY = kAutoplayCornerLaneBias;
		else if (engine.playerY() > centerY + kAutoplayCornerLaneHalfWidth &&
				blockingCorner(engine, from + 1 + kBaseMapWidth, from + 1 + kBaseMapWidth))
			biasY = -kAutoplayCornerLaneBias;
	} else if (to == from - 1) {
		if (engine.playerY() < centerY - kAutoplayCornerLaneHalfWidth &&
				blockingCorner(engine, from - kBaseMapWidth, from - 1))
			biasY = kAutoplayCornerLaneBias;
		else if (engine.playerY() > centerY + kAutoplayCornerLaneHalfWidth &&
				blockingCorner(engine, from + kBaseMapWidth, from - 1 + kBaseMapWidth))
			biasY = -kAutoplayCornerLaneBias;
	} else if (to == from + kBaseMapWidth) {
		if (engine.playerX() < centerX - kAutoplayCornerLaneHalfWidth &&
				blockingCorner(engine, from + kBaseMapWidth, from - 1 + kBaseMapWidth))
			biasX = kAutoplayCornerLaneBias;
		else if (engine.playerX() > centerX + kAutoplayCornerLaneHalfWidth &&
				blockingCorner(engine, from + 1 + kBaseMapWidth, from + 1 + kBaseMapWidth))
			biasX = -kAutoplayCornerLaneBias;
	} else if (to == from - kBaseMapWidth) {
		if (engine.playerX() < centerX - kAutoplayCornerLaneHalfWidth &&
				blockingCorner(engine, from - kBaseMapWidth, from - 1))
			biasX = kAutoplayCornerLaneBias;
		else if (engine.playerX() > centerX + kAutoplayCornerLaneHalfWidth &&
				blockingCorner(engine, from + 1 - kBaseMapWidth, from + 1))
			biasX = -kAutoplayCornerLaneBias;
	}
}

static int blockingObjectAt(const BaseEngine &engine, int mapPos) {
	for (int id = 1; id <= kBaseMaxObjects; ++id) {
		const BaseObject &object = engine.object(id);
		if (object.active && !object.passable && object.mapPos == mapPos)
			return id;
	}
	return 0;
}

static byte nearestPaletteColor(const byte *palette, int red, int green, int blue) {
	int bestDistance = 0x7fffffff;
	byte bestColor = 0;
	for (int color = 0; color < 256; ++color) {
		const int offset = color * 3;
		const int redDelta = palette[offset] - red;
		const int greenDelta = palette[offset + 1] - green;
		const int blueDelta = palette[offset + 2] - blue;
		const int distance = redDelta * redDelta + greenDelta * greenDelta + blueDelta * blueDelta;
		if (distance < bestDistance) {
			bestDistance = distance;
			bestColor = color;
		}
	}
	return bestColor;
}

static void fillRect(byte *framebuffer, int left, int top, int right, int bottom, byte color) {
	left = MAX(left, 0);
	top = MAX(top, 0);
	right = MIN(right, kBaseFrameWidth - 1);
	bottom = MIN(bottom, kBaseFrameHeight - 1);
	for (int y = top; y <= bottom; ++y)
		Common::fill(framebuffer + y * kBaseFrameWidth + left,
				framebuffer + y * kBaseFrameWidth + right + 1, color);
}

static void drawFrame(byte *framebuffer, int left, int top, int right, int bottom, byte color) {
	fillRect(framebuffer, left, top, right, top, color);
	fillRect(framebuffer, left, bottom, right, bottom, color);
	fillRect(framebuffer, left, top, left, bottom, color);
	fillRect(framebuffer, right, top, right, bottom, color);
}

static Common::String menuDestinationLabel(int destination, int returnRoomId) {
	Common::String label(kAutoplayDestinations[destination].label);
	if (kAutoplayDestinations[destination].roomId == returnRoomId)
		label += " (return)";
	return label;
}

} // End of anonymous namespace

WBASEEnhancementsAutoplay::WBASEEnhancementsAutoplay() {
	reset();
}

void WBASEEnhancementsAutoplay::reset() {
	_active = false;
	_selectedDestination = 0;
	_hoveredDestination = -1;
	_destination = -1;
	_route.clear();
	_routeIndex = 0;
	_lastPlayerX = -1;
	_lastPlayerY = -1;
	_stuckTicks = 0;
	_routeInvalidated = false;
	_blockedRouteCell = -1;
	_routeBlocker = 0;
	_movementBlocker = 0;
	_activeTicks = 0;
	_recoveryPhase = kAutoplayRecoveryNone;
	_recoveryTicks = 0;
	_recoveryAttempts = 0;
	_recoveryTurnRight = true;
}

void WBASEEnhancementsAutoplay::clearMenuPointer() {
	_hoveredDestination = -1;
}

void WBASEEnhancementsAutoplay::selectPrevious() {
	_selectedDestination = (_selectedDestination + kAutoplayDestinationCount - 1) % kAutoplayDestinationCount;
	_hoveredDestination = -1;
}

void WBASEEnhancementsAutoplay::selectNext() {
	_selectedDestination = (_selectedDestination + 1) % kAutoplayDestinationCount;
	_hoveredDestination = -1;
}

void WBASEEnhancementsAutoplay::updateMenuPointer(int x, int y) {
	_hoveredDestination = -1;
	if (x < kAutoplayMenuLeft || x > kAutoplayMenuRight || y < kAutoplayMenuChoiceTop)
		return;
	const int destination = (y - kAutoplayMenuChoiceTop) / kAutoplayMenuChoiceSpacing;
	const int lineY = kAutoplayMenuChoiceTop + destination * kAutoplayMenuChoiceSpacing;
	if (destination >= 0 && destination < kAutoplayDestinationCount && y < lineY + kAutoplayMenuChoiceHeight) {
		_hoveredDestination = destination;
		_selectedDestination = destination;
	}
}

bool WBASEEnhancementsAutoplay::selectMenuPointer(int x, int y) {
	updateMenuPointer(x, y);
	return _hoveredDestination >= 0;
}

bool WBASEEnhancementsAutoplay::startSelected(const BaseEngine &engine) {
	return startDestination(engine, _selectedDestination);
}

bool WBASEEnhancementsAutoplay::startDestination(const BaseEngine &engine, int destination) {
	if (destination < 0 || destination >= kAutoplayDestinationCount) {
		cancel();
		return false;
	}
	_selectedDestination = destination;
	_destination = _selectedDestination;
	_active = true;
	_lastPlayerX = engine.playerX();
	_lastPlayerY = engine.playerY();
	_stuckTicks = 0;
	_routeInvalidated = false;
	_blockedRouteCell = -1;
	_routeBlocker = 0;
	_movementBlocker = 0;
	_activeTicks = 0;
	_recoveryPhase = kAutoplayRecoveryNone;
	_recoveryTicks = 0;
	_recoveryAttempts = 0;
	if (!rebuildRoute(engine, false)) {
		warning("Hopkins WBASE enhancement autoplay: no route to room %d",
				kAutoplayDestinations[_destination].roomId);
		cancel();
		return false;
	} else {
		debug(1, "Hopkins WBASE enhancement autoplay: room %d, route %u cells",
				kAutoplayDestinations[_destination].roomId, _route.size());
	}
	return true;
}

void WBASEEnhancementsAutoplay::cancel() {
	_active = false;
	_hoveredDestination = -1;
	_destination = -1;
	_route.clear();
	_routeIndex = 0;
	_blockedRouteCell = -1;
	_routeBlocker = 0;
	_movementBlocker = 0;
	_activeTicks = 0;
	_recoveryPhase = kAutoplayRecoveryNone;
	_recoveryTicks = 0;
}

int WBASEEnhancementsAutoplay::destinationCount() {
	return kAutoplayDestinationCount;
}

int WBASEEnhancementsAutoplay::destinationRoomId(int index) {
	return index >= 0 && index < kAutoplayDestinationCount ? kAutoplayDestinations[index].roomId : 0;
}

int WBASEEnhancementsAutoplay::destinationExitMapPos(int index) {
	return index >= 0 && index < kAutoplayDestinationCount ? kAutoplayDestinations[index].exitMapPos : -1;
}

const char *WBASEEnhancementsAutoplay::destinationLabel(int index) {
	return index >= 0 && index < kAutoplayDestinationCount ? kAutoplayDestinations[index].label : "";
}

const char *WBASEEnhancementsAutoplay::destinationMapLabel(int index) {
	return index >= 0 && index < kAutoplayDestinationCount ? kAutoplayDestinations[index].mapLabel : "";
}

int WBASEEnhancementsAutoplay::destinationIndexForExitMapPos(int mapPos) {
	for (int index = 0; index < kAutoplayDestinationCount; ++index) {
		if (kAutoplayDestinations[index].exitMapPos == mapPos)
			return index;
	}
	return -1;
}

bool WBASEEnhancementsAutoplay::rebuildRoute(const BaseEngine &engine, bool avoidBlockingObjects) {
	_route.clear();
	_routeIndex = 0;
	_routeInvalidated = false;
	if (_destination < 0 || _destination >= kAutoplayDestinationCount)
		return false;

	const int start = baseWorldMapIndex(engine.playerX(), engine.playerY());
	const int goal = kAutoplayDestinations[_destination].approachMapPos;
	if (start < 0 || start >= kBaseMapCellCount || goal < 0 || goal >= kBaseMapCellCount)
		return false;

	static const int kOffsets[] = { 1, -1, kBaseMapWidth, -kBaseMapWidth };
	int parent[kBaseMapCellCount];
	int queue[kBaseMapCellCount];
	bool found = false;
	// Retry around a current blocker, but ignore moving guards on initial routes.
	for (int avoidObjects = avoidBlockingObjects ? 1 : 0; avoidObjects >= 0 && !found; --avoidObjects) {
		Common::fill(parent, parent + ARRAYSIZE(parent), -2);
		int queueBegin = 0;
		int queueEnd = 0;
		queue[queueEnd++] = start;
		parent[start] = -1;
		while (queueBegin < queueEnd && parent[goal] == -2) {
			const int current = queue[queueBegin++];
			for (uint direction = 0; direction < ARRAYSIZE(kOffsets); ++direction) {
				const int next = current + kOffsets[direction];
				if (next < 0 || next >= kBaseMapCellCount || parent[next] != -2)
					continue;
				if (!canStep(engine, current, next))
					continue;
				if (avoidObjects && next != goal && blockingObjectAt(engine, next))
					continue;
				parent[next] = current;
				queue[queueEnd++] = next;
			}
		}
		found = parent[goal] != -2;
	}

	if (!found)
		return false;

	Common::Array<int> reverseRoute;
	for (int mapPos = goal; mapPos >= 0; mapPos = parent[mapPos])
		reverseRoute.push_back(mapPos);
	for (int index = (int)reverseRoute.size() - 1; index >= 0; --index)
		_route.push_back(reverseRoute[index]);
	_routeIndex = _route.size() > 1 ? 1 : 0;
	return true;
}

int WBASEEnhancementsAutoplay::findVisibleGuard(const BaseEngine &engine) const {
	const int priorityTicks = MIN(_activeTicks, kAutoplayCombatPriorityTicks);
	const int combatDistance = kAutoplayInitialCombatDistance -
			(kAutoplayInitialCombatDistance - kAutoplayNearbyCombatDistance) *
			priorityTicks / kAutoplayCombatPriorityTicks;
	const int combatHalfAngle = kBaseHalfTurn -
			(kBaseHalfTurn - kAutoplayFinalCombatHalfAngle) *
			priorityTicks / kAutoplayCombatPriorityTicks;
	int bestGuard = 0;
	int bestDistance = combatDistance + 1;
	for (int id = 1; id <= kBaseMaxObjects; ++id) {
		const BaseObject &object = engine.object(id);
		if (!object.active || object.passable || object.mode > kBaseObjectFlee)
			continue;
		const int distance = engine.objectDistance(object);
		if (distance > combatDistance || distance >= bestDistance)
			continue;

		const int angle = engine.objectAngle(object.x - engine.playerX(), object.y - engine.playerY());
		const int angleDelta = absoluteValue(signedAngleDelta(engine.playerAngle(), angle));
		if (distance > kAutoplayNearbyCombatDistance && angleDelta > combatHalfAngle)
			continue;
		const BaseRayHit wall = engine.castRay(angle, kBaseViewHalfWidth);
		// A visibility margin can expose guards behind thin partitions.
		if (wall.hit && wall.distance <= distance)
			continue;
		bestGuard = id;
		bestDistance = distance;
	}
	return bestGuard;
}

bool WBASEEnhancementsAutoplay::aimAt(const BaseEngine &engine, int targetX, int targetY,
		BaseInputState &input) const {
	const int desiredAngle = engine.objectAngle(targetX - engine.playerX(), targetY - engine.playerY());
	const int delta = signedAngleDelta(engine.playerAngle(), desiredAngle);
	if (delta > kAutoplayAimTolerance)
		input.turnRight = true;
	else if (delta < -kAutoplayAimTolerance)
		input.turnLeft = true;
	return absoluteValue(delta) <= kAutoplayAimTolerance;
}

void WBASEEnhancementsAutoplay::updateStuckState(const BaseEngine &engine, bool triedToMove) {
	if (!triedToMove || engine.playerX() != _lastPlayerX || engine.playerY() != _lastPlayerY) {
		_stuckTicks = 0;
	} else if (++_stuckTicks >= kAutoplayStuckTicks) {
		beginRecovery();
		_stuckTicks = 0;
	}
	_lastPlayerX = engine.playerX();
	_lastPlayerY = engine.playerY();
}

void WBASEEnhancementsAutoplay::beginRecovery() {
	_recoveryPhase = kAutoplayRecoveryReverse;
	_recoveryTicks = kAutoplayRecoveryReverseTicks;
	_recoveryTurnRight = (_recoveryAttempts++ & 1) == 0;
	_routeInvalidated = true;
	debug(1, "Hopkins WBASE enhancement autoplay: zero progress; recovery %d turning %s",
			_recoveryAttempts, _recoveryTurnRight ? "right" : "left");
}

void WBASEEnhancementsAutoplay::updateRecovery(const BaseEngine &engine, BaseInputState &input) {
	if (_recoveryPhase == kAutoplayRecoveryReverse) {
		input.backward = true;
	} else if (_recoveryPhase == kAutoplayRecoveryTurn) {
		input.turnRight = _recoveryTurnRight;
		input.turnLeft = !_recoveryTurnRight;
	} else if (_recoveryPhase == kAutoplayRecoveryAdvance) {
		input.forward = true;
	}

	if (--_recoveryTicks <= 0) {
		if (_recoveryPhase == kAutoplayRecoveryReverse) {
			_recoveryPhase = kAutoplayRecoveryTurn;
			_recoveryTicks = kAutoplayRecoveryTurnTicks;
		} else if (_recoveryPhase == kAutoplayRecoveryTurn) {
			_recoveryPhase = kAutoplayRecoveryAdvance;
			_recoveryTicks = kAutoplayRecoveryAdvanceTicks;
		} else {
			_recoveryPhase = kAutoplayRecoveryNone;
			_recoveryTicks = 0;
			_routeInvalidated = true;
		}
	}
	_lastPlayerX = engine.playerX();
	_lastPlayerY = engine.playerY();
}

int WBASEEnhancementsAutoplay::update(const BaseEngine &engine, BaseInputState &input) {
	input = BaseInputState();
	_routeBlocker = 0;
	_movementBlocker = 0;
	if (!_active || _destination < 0 || _destination >= kAutoplayDestinationCount)
		return -1;

	// Complete after 100 active seconds so forced autoplay cannot remain stuck.
	// Paused time is excluded because update() is not called.
	if (++_activeTicks >= kAutoplayMaximumTicks) {
		const int roomId = kAutoplayDestinations[_destination].roomId;
		warning("Hopkins WBASE enhancement autoplay: forcing room %d after %d active ticks",
				roomId, _activeTicks);
		cancel();
		return roomId;
	}

	const int playerMapPos = baseWorldMapIndex(engine.playerX(), engine.playerY());
	_movementBlocker = blockingObjectAt(engine, playerMapPos);

	if (_recoveryPhase != kAutoplayRecoveryNone) {
		updateRecovery(engine, input);
		return -1;
	}

	const int guardId = findVisibleGuard(engine);
	if (guardId) {
		const BaseObject &guard = engine.object(guardId);
		if (aimAt(engine, guard.x, guard.y, input) && engine.weaponCounter() == 0)
			input.fire = true;
		// Fighting without movement also advances stall recovery.
		updateStuckState(engine, true);
		return -1;
	}

	const AutoplayDestination &destination = kAutoplayDestinations[_destination];
	if (playerMapPos == destination.approachMapPos) {
		const int exitX = (destination.exitMapPos & 63) * kBaseCellSize + kBaseCellSize / 2;
		const int exitY = (destination.exitMapPos >> 6) * kBaseCellSize + kBaseCellSize / 2;
		const int approachX = (destination.approachMapPos & 63) * kBaseCellSize + kBaseCellSize / 2;
		const int approachY = (destination.approachMapPos >> 6) * kBaseCellSize + kBaseCellSize / 2;
		const int centerDistance = absoluteValue(engine.playerX() - approachX) + absoluteValue(engine.playerY() - approachY);
		if (centerDistance > kAutoplayWaypointTolerance) {
			const bool aligned = aimAt(engine, approachX, approachY, input);
			input.forward = aligned;
			updateStuckState(engine, input.forward);
			return -1;
		}
		if (aimAt(engine, exitX, exitY, input))
			input.exitRequested = true;
		updateStuckState(engine, false);
		return -1;
	}

	if (_routeInvalidated || _route.empty() || _routeIndex >= _route.size())
		rebuildRoute(engine, false);
	if (_route.empty()) {
		cancel();
		return -1;
	}

	while (_routeIndex < _route.size() && playerMapPos == _route[_routeIndex]) {
		const int waypointX = (_route[_routeIndex] & 63) * kBaseCellSize + kBaseCellSize / 2;
		const int waypointY = (_route[_routeIndex] >> 6) * kBaseCellSize + kBaseCellSize / 2;
		if (absoluteValue(engine.playerX() - waypointX) + absoluteValue(engine.playerY() - waypointY) >
				kAutoplayWaypointTolerance)
			break;
		++_routeIndex;
	}

	if (_routeIndex >= _route.size()) {
		_routeInvalidated = true;
		updateStuckState(engine, false);
		return -1;
	}

	const int routeBlocker = blockingObjectAt(engine, _route[_routeIndex]);
	_routeBlocker = routeBlocker;
	if (!routeBlocker)
		_blockedRouteCell = -1;
	if (routeBlocker && _route[_routeIndex] != destination.approachMapPos &&
			_route[_routeIndex] != _blockedRouteCell) {
		const int oldTarget = _route[_routeIndex];
		_blockedRouteCell = oldTarget;
		_routeInvalidated = true;
		rebuildRoute(engine, true);
		debug(2, "Hopkins WBASE enhancement autoplay: tick %d target %04x occupied by guard %d; replanned to %04x",
				_activeTicks, oldTarget, routeBlocker,
				_routeIndex < _route.size() ? _route[_routeIndex] : -1);
		if (_route.empty()) {
			cancel();
			return -1;
		}
	}

	const int expectedCell = _routeIndex > 0 ? _route[_routeIndex - 1] : _route[0];
	if (playerMapPos != expectedCell && playerMapPos != _route[_routeIndex]) {
		_routeInvalidated = true;
		updateStuckState(engine, false);
		return -1;
	}

	const int routeTarget = _route[_routeIndex];
	if (playerMapPos == expectedCell) {
		int biasX = 0;
		int biasY = 0;
		routeCornerLaneBias(engine, playerMapPos, routeTarget, biasX, biasY);
		if (biasX || biasY) {
			const int forwardX = (routeTarget & 63) - (playerMapPos & 63);
			const int forwardY = (routeTarget >> 6) - (playerMapPos >> 6);
			const int steeringX = engine.playerX() + forwardX * kBaseCellSize + biasX;
			const int steeringY = engine.playerY() + forwardY * kBaseCellSize + biasY;
			input.forward = aimAt(engine, steeringX, steeringY, input);
			updateStuckState(engine, input.forward);
			return -1;
		}
	}

	const int waypointX = (_route[_routeIndex] & 63) * kBaseCellSize + kBaseCellSize / 2;
	const int waypointY = (_route[_routeIndex] >> 6) * kBaseCellSize + kBaseCellSize / 2;
	const int desiredAngle = engine.objectAngle(waypointX - engine.playerX(), waypointY - engine.playerY());
	const int delta = signedAngleDelta(engine.playerAngle(), desiredAngle);
	if (delta > kAutoplayAimTolerance)
		input.turnRight = true;
	else if (delta < -kAutoplayAimTolerance)
		input.turnLeft = true;
	input.forward = absoluteValue(delta) <= kAutoplayMoveTolerance;
	updateStuckState(engine, input.forward);
	return -1;
}

void WBASEEnhancementsAutoplay::renderMenu(const BaseData &data, int returnRoomId,
		byte *framebuffer, bool forcedMode) const {
	if (!framebuffer)
		return;

	Graphics::Surface surface;
	surface.init(kBaseFrameWidth, kBaseFrameHeight, kBaseFrameWidth, framebuffer,
			Graphics::PixelFormat::createFormatCLUT8());
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	if (!font)
		return;

	const byte *palette = data.palette();
	const byte background = nearestPaletteColor(palette, 0, 0, 0);
	const byte border = nearestPaletteColor(palette, 190, 190, 190);
	const byte text = nearestPaletteColor(palette, 255, 255, 255);
	const byte selected = nearestPaletteColor(palette, 255, 220, 0);

	fillRect(framebuffer, kAutoplayMenuLeft, kAutoplayMenuTop,
			kAutoplayMenuRight, kAutoplayMenuBottom, background);
	drawFrame(framebuffer, kAutoplayMenuLeft, kAutoplayMenuTop,
			kAutoplayMenuRight, kAutoplayMenuBottom, border);
	font->drawString(&surface, _("WBASE AUTOPLAY"), kAutoplayMenuLeft + 4,
			kAutoplayMenuTop + 7, kAutoplayMenuRight - kAutoplayMenuLeft - 7,
			text, Graphics::kTextAlignCenter);
	font->drawString(&surface, _("Fight your way to:"), kAutoplayMenuLeft + 4,
			kAutoplayMenuTop + 21, kAutoplayMenuRight - kAutoplayMenuLeft - 7,
			text, Graphics::kTextAlignCenter);
	for (int index = 0; index < kAutoplayDestinationCount; ++index) {
		const Common::String label = menuDestinationLabel(index, returnRoomId);
		const bool highlighted = index ==
				(_hoveredDestination >= 0 ? _hoveredDestination : _selectedDestination);
		font->drawString(&surface, Common::String::format("%c %s", highlighted ? '>' : ' ', label.c_str()),
				kAutoplayMenuLeft + 14, kAutoplayMenuChoiceTop + index * kAutoplayMenuChoiceSpacing,
				kAutoplayMenuRight - kAutoplayMenuLeft - 27,
				highlighted ? selected : text);
	}
	font->drawString(&surface, _("Up/Down: choose"), kAutoplayMenuLeft + 8,
			kAutoplayMenuBottom - 25, 104, text);
	font->drawString(&surface, _("Enter/Space: start"), kAutoplayMenuLeft + 116,
			kAutoplayMenuBottom - 25, 111, text);
	const Common::U32String footer = forcedMode ?
			(_active ? _("A: resume / M: map") : _("Select a destination")) :
			_("Esc/A: cancel");
	font->drawString(&surface, footer, kAutoplayMenuLeft + 4,
			kAutoplayMenuBottom - 13, kAutoplayMenuRight - kAutoplayMenuLeft - 7,
			text, Graphics::kTextAlignCenter);
}

void WBASEEnhancementsAutoplay::renderStatus(const BaseData &data, byte *framebuffer, bool forcedMode) const {
	if (!framebuffer || !_active)
		return;

	Graphics::Surface surface;
	surface.init(kBaseFrameWidth, kBaseFrameHeight, kBaseFrameWidth, framebuffer,
			Graphics::PixelFormat::createFormatCLUT8());
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	if (!font)
		return;

	const byte *palette = data.palette();
	const byte background = nearestPaletteColor(palette, 0, 0, 0);
	const byte border = nearestPaletteColor(palette, 190, 190, 190);
	const byte text = nearestPaletteColor(palette, 255, 255, 255);

	const int secondsRemaining = MAX(0, (kAutoplayMaximumTicks - _activeTicks + 23) / 24);
	const Common::String status = Common::String::format(forcedMode ? "LOCKED AUTO: %s - %ds" : "AUTO: %s - %ds",
			kAutoplayDestinations[_destination].label, secondsRemaining);
	const int statusWidth = MIN(font->getStringWidth(status) + 10, kBaseFrameWidth - 8);
	const int statusLeft = (kBaseFrameWidth - statusWidth) / 2;
	fillRect(framebuffer, statusLeft, 2, statusLeft + statusWidth - 1, 14, background);
	drawFrame(framebuffer, statusLeft, 2, statusLeft + statusWidth - 1, 14, border);
	font->drawString(&surface, status, statusLeft + 4, 4, statusWidth - 8, text,
			Graphics::kTextAlignCenter);
}

} // End of namespace Hopkins
