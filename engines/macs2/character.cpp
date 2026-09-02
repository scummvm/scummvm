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

#include "macs2/character.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/util.h"
#include "macs2/detection.h"
#include "macs2/events.h"
#include "macs2/macs2.h"
#include "macs2/macs2_constants.h"
#include "macs2/scriptexecutor.h"
#include "macs2/view1.h"

namespace Macs2 {

void resetCharacterWalkPath(Character *character) {
	if (character == nullptr || character->_gameObject == nullptr) {
		return;
	}
	const Common::Point &pos = character->getPosition();
	character->_path.clear();
	character->_currentPathIndex = 0;
	character->_targetPosition = pos;
	character->_pathFinalDestination = pos;
	character->_stepDeltaX = 0;
	character->_stepDeltaY = 0;
	character->_stepError = 0;
	character->_stepDirectionSet = false;
}

// Effective world position for pickup walk targets and bounds-attached props.
// Inventory items use their holder's position; attached objects use parent + offset.
static Common::Point getObjectEffectivePosition(const GameObject *object) {
	if (object == nullptr) {
		return Common::Point();
	}
	if (object->_hasBoundsAttachment) {
		const GameObject *parent = GameObjects::getObjectByIndex(object->_boundsAttachmentObjectID);
		if (parent != nullptr) {
			return Common::Point(
				parent->_position.x + (int16)object->_boundsAttachmentValue1,
				parent->_position.y + (int16)object->_boundsAttachmentValue2);
		}
	}
	if (object->_sceneIndex > 0x400) {
		const GameObject *holder = GameObjects::getObjectByIndex(object->_sceneIndex - 0x400);
		if (holder != nullptr) {
			return holder->_position;
		}
	}
	return object->_position;
}

bool Character::handleWalkability(Character *c) {
	// Wall-sliding obstacle avoidance from walkAlongPath (1008:1b8f).
	// When the character steps into a non-walkable pixel (walkability >= 200),
	// the original code samples walkability at +/-1 and +/-2 pixels in each
	// axis to build a gradient vector, then slides the character along that
	// vector until it reaches a walkable position.
	if (c->_gameObject->_index != 1) {
		return false;
	}
	if (g_engine->_scriptExecutor->isExecuting()) {
		return false;
	}

	Common::Point pos = c->getPosition();
	if (isWalkable(pos)) {
		return false;
	}

	// Build a push vector by sampling the walkability map around the current
	// position. Non-walkable neighbors push us away from them.
	int pushX = 0;
	int pushY = 0;

	// Sample at distance 1
	if (!isWalkable(Common::Point(pos.x + 1, pos.y)))
		pushX -= 1;
	if (!isWalkable(Common::Point(pos.x - 1, pos.y)))
		pushX += 1;
	if (!isWalkable(Common::Point(pos.x, pos.y + 1)))
		pushY -= 1;
	if (!isWalkable(Common::Point(pos.x, pos.y - 1)))
		pushY += 1;

	// Sample at distance 2 for stronger gradient
	if (!isWalkable(Common::Point(pos.x + 2, pos.y)))
		pushX -= 1;
	if (!isWalkable(Common::Point(pos.x - 2, pos.y)))
		pushX += 1;
	if (!isWalkable(Common::Point(pos.x, pos.y + 2)))
		pushY -= 1;
	if (!isWalkable(Common::Point(pos.x, pos.y - 2)))
		pushY += 1;

	// Slide along the push vector
	while (pushX != 0 || pushY != 0) {
		if (pushX < 0) {
			if (isWalkable(Common::Point(pos.x - 1, pos.y)))
				pos.x -= 1;
			pushX += 1;
		}
		if (pushX > 0) {
			if (isWalkable(Common::Point(pos.x + 1, pos.y)))
				pos.x += 1;
			pushX -= 1;
		}
		if (pushY < 0) {
			if (isWalkable(Common::Point(pos.x, pos.y - 1)))
				pos.y -= 1;
			pushY += 1;
		}
		if (pushY > 0) {
			if (isWalkable(Common::Point(pos.x, pos.y + 1)))
				pos.y += 1;
			pushY -= 1;
		}
	}

	c->setPosition(pos);
	return true;
}

uint16 Character::lookupWalkability(const Common::Point &p) const {
	return g_engine->getWalkabilityAt((int16)p.y, (int16)p.x);
}

bool Character::isWalkable(const Common::Point &p) const {
	return Macs2Engine::isWalkabilityWalkable(lookupWalkability(p));
}

Character::Character() : _pathfindingOverlay(g_engine->screenWidth() * g_engine->gameHeight(), 0) {
}

bool Character::calculatePath(Common::Point target) {
	// Binary calculatePath (1008:1966). Params: charY, charX, finalDestY, finalDestX, actorIndex.
	// The binary operates on the runtime struct directly; we store equivalent state in _path etc.
	const Common::Point &charPos = _gameObject->_position;
	const int nodeCount = g_engine->getPathfindingNodeCount();

	// Step 1: Mark reachability anchored on FINAL DESTINATION (not character)
	// scene[i + 0x50C2] = isPathWalkable(finalDest, node[i])
	bool reachable[kPathNodeSlots + 1] = {};
	for (int i = 1; i <= nodeCount; i++) {
		const Common::Point &nodePos = g_engine->_pathfindingPoints[i - 1]._position;
		reachable[i] = g_engine->isPathWalkable(target.y, target.x, nodePos.y, nodePos.x);
	}

	// Step 2: Find best entry node (lowest combined distance to both source and dest)
	int bestCost = 0x7777;
	int bestNode = 0;
	for (int i = 1; i <= nodeCount; i++) {
		const Common::Point &nodePos = g_engine->_pathfindingPoints[i - 1]._position;
		int costToDest = g_engine->euclideanDistance(nodePos, target);
		int costToChar = g_engine->euclideanDistance(nodePos, charPos);
		if (costToDest + costToChar < bestCost) {
			// Verify this node can connect source to target
			// Binary calls canNodeConnectSourceToTarget(destY, destX, charY, charX, i)
			// due to calculatePath being invoked with swapped source/dest params.
			// This means the gate check is "can node see CHARACTER" and the flood-fill
			// checks "any node reachable from DEST" AND "any node visible from CHARACTER".
			// TODO: validate this with a playthought:
			//   PVS-Studio V764: Possible incorrect order of arguments passed to
			//   'canNodeConnectSourceToTarget' function: 'target' and 'charPos'.
			// I didn't had any issues in previous runs
			if (canNodeConnectSourceToTarget(i, target, charPos, reachable, nodeCount)) {
				// Recompute cost (binary does this twice)
				costToDest = g_engine->euclideanDistance(nodePos, target);
				costToChar = g_engine->euclideanDistance(nodePos, charPos);
				bestCost = costToDest + costToChar;
				bestNode = i;
			}
		}
	}

	if (bestNode == 0) {
		// No path found - go directly to target
		// Binary: pathNodeCount=0, pathIndex=1
		_path.clear();
		_currentPathIndex = 1;
		_targetPosition = target;
		return false;
	}

	// Step 3: smoothPath - build path from bestNode toward a reachable node
	_path.clear();
	_path.push_back(bestNode);
	int currentNode = bestNode;
	while (!reachable[currentNode]) {
		const PathfindingPoint &curPt = g_engine->_pathfindingPoints[currentNode - 1];
		int localBestCost = 0x7777;
		int nextNode = currentNode;
		for (uint a = 0; a < curPt._adjacentPoints.size(); a++) {
			const int adjIdx = curPt._adjacentPoints[a];
			const int cost = g_engine->computeMinCostToReachable(adjIdx, 0x7fff, _gameObject->_index, reachable, nodeCount, target);
			const int edgeCost = g_engine->walkableDistance(adjIdx, currentNode);
			if (cost + edgeCost < localBestCost) {
				nextNode = adjIdx;
				localBestCost = cost + edgeCost;
			}
		}
		currentNode = nextNode;
		_path.push_back(currentNode);
		if (_path.size() > kPathNodeSlots)
			break; // safety
	}

	// Step 4: Validate path - consecutive nodes must be walkable to each other
	for (uint i = 0; i + 1 < _path.size(); i++) {
		const Common::Point &p1 = g_engine->_pathfindingPoints[_path[i + 1] - 1]._position;
		const Common::Point &p2 = g_engine->_pathfindingPoints[_path[i] - 1]._position;
		if (!g_engine->isPathWalkable(p1.y, p1.x, p2.y, p2.x)) {
			// Path invalid - abort, go directly to target
			_path.clear();
			_targetPosition = target;
			return false;
		}
	}

	// Step 5: Skip-forward optimization - skip nodes the character can already reach directly.
	// Binary: checks isPathWalkable(nextNode, charPos) - "can character see the next node?"
	// Note: binary's calculatePath is called with swapped params, so its 'finalDest' param
	// is actually the character position.
	_currentPathIndex = 0;
	while (_currentPathIndex + 1 < (int16)_path.size()) {
		const Common::Point &nextNodePos = g_engine->_pathfindingPoints[_path[_currentPathIndex + 1] - 1]._position;
		if (!g_engine->isPathWalkable(nextNodePos.y, nextNodePos.x, charPos.y, charPos.x))
			break;
		_currentPathIndex++;
	}

	// Set immediate target to the current path node
	const Common::Point &firstTarget = g_engine->_pathfindingPoints[_path[_currentPathIndex] - 1]._position;
	_targetPosition = firstTarget;
	return true;
}

bool Character::canNodeConnectSourceToTarget(uint16 nodeIndex, const Common::Point &charPos, const Common::Point &target, const bool *reachable, int nodeCount) {
	// Checks if node can connect source (charPos) to target:
	// 1. Node must be able to see the target
	// 2. Flood-fill connected component from node
	// 3. Some node in component must see target AND some node must be seen from source
	const Common::Point &nodePos = g_engine->_pathfindingPoints[nodeIndex - 1]._position;
	if (!g_engine->isPathWalkable(nodePos.y, nodePos.x, target.y, target.x))
		return false;

	// Flood-fill connected nodes
	bool visited[kPathNodeSlots + 1] = {};
	floodFillConnectedNodes(nodeIndex, visited, nodeCount);

	// Check both conditions
	bool anySeesTarget = false;
	bool anySeenFromSource = false;
	for (int i = 1; i <= nodeCount; i++) {
		if (!visited[i])
			continue;
		const Common::Point &p = g_engine->_pathfindingPoints[i - 1]._position;
		if (g_engine->isPathWalkable(p.y, p.x, target.y, target.x))
			anySeesTarget = true;
		if (g_engine->isPathWalkable(charPos.y, charPos.x, p.y, p.x))
			anySeenFromSource = true;
	}
	return anySeesTarget && anySeenFromSource;
}

void Character::floodFillConnectedNodes(int nodeIndex, bool *visited, int nodeCount) {
	if (nodeIndex < 1 || nodeIndex > nodeCount)
		return;
	if (visited[nodeIndex])
		return;
	visited[nodeIndex] = true;
	const PathfindingPoint &pt = g_engine->_pathfindingPoints[nodeIndex - 1];
	for (uint i = 0; i < pt._adjacentPoints.size(); i++) {
		floodFillConnectedNodes(pt._adjacentPoints[i], visited, nodeCount);
	}
}

const Common::Point &Character::getPosition() const {
	return _gameObject->_position;
}

void Character::setPosition(const Common::Point &newPosition) {
	_gameObject->_position = newPosition;
}

uint16 Character::getVerticalOffset() const {
	uint16 result = g_engine->getWalkabilityAt(getPosition());
	if (Macs2Engine::isWalkabilityBlocking(result)) {
		result = 0;
	}

	if (_gameObject->_verticalOffsetScale != 0) {
		const int16 charY = getPosition().y;
		const int32 depthOffset = ((int32)charY - (int32)g_engine->_walkDepthThresholdY) *
							(int32)g_engine->_walkDepthScaleFactor / 100;
		const uint16 scalingFactor = (uint16)((int32)g_engine->_walkBaseSpeedPct + depthOffset);
		result = (scalingFactor * _gameObject->_verticalOffsetScale) / 100;
	}

	return result;
}

bool Character::walkAlongPath() {
	if (_currentPathIndex >= 0 && _currentPathIndex < (int16)_path.size()) {
		const uint16 snapIdx = _path[_currentPathIndex];
		const Common::Point &snapPos = g_engine->_pathfindingPoints[snapIdx - 1]._position;
		_gameObject->_position = snapPos;
	}
	_currentPathIndex++;
	if (_currentPathIndex >= (int16)_path.size()) {
		// Past end of path - walk to final destination, then stop
		_targetPosition = _pathFinalDestination;
		_stepDeltaX = (int16)ABS(_targetPosition.x - _gameObject->_position.x);
		_stepDeltaY = (int16)ABS(_targetPosition.y - _gameObject->_position.y);
		_stepError = 0;
		_stepDirectionSet = false;
		return false; // No more path segments after this
	}
	const uint16 nodeIdx = _path[_currentPathIndex];
	const Common::Point &nodePos = g_engine->_pathfindingPoints[nodeIdx - 1]._position;
	_targetPosition = nodePos;
	_stepDeltaX = (int16)ABS(_targetPosition.x - _gameObject->_position.x);
	_stepDeltaY = (int16)ABS(_targetPosition.y - _gameObject->_position.y);
	_stepError = 0;
	_stepDirectionSet = false;
	return true;
}

bool Character::isAnimationMirrored() const {
	switch (_gameObject->_orientation) {
	case OrientationSouthWest:
	case OrientationWest:
	case OrientationNorthWest:
	case OrientationStandingSouthWest:
	case OrientationStandingWest:
	case OrientationStandingNorthWest:
		return true;
	default:
		break;
	}
	return false;
}

bool Character::fillCurrentAnimationFrame(uint16 advanceMode, Macs2::AnimFrame &out) const {
	const uint16 animSlot = g_engine->resolveAnimSlotIndex(_gameObject);

	Common::Array<uint8> *blobPtr = _gameObject->getAnimSlotBlob(animSlot);
	if (blobPtr == nullptr || blobPtr->empty()) {
		return false;
	}

	Common::Array<uint8> &blob = *blobPtr;
	const uint16 frameStart = BackgroundAnimationBlob::advanceAnimFrame(blob, true, advanceMode);
	out._offsetX = (int16)READ_LE_UINT16(&blob[frameStart]);
	out._offsetY = (int16)READ_LE_UINT16(&blob[frameStart + 2]);
	const uint16 offset = frameStart + 6;
	out._width = READ_LE_UINT16(&blob[offset]);
	out._height = READ_LE_UINT16(&blob[offset + 2]);
	out._data.resize(out._width * out._height);
	memcpy(out._data.data(), &blob[offset + 4], out._width * out._height);
	return true;
}

Macs2::AnimFrame *Character::getCurrentPortrait(bool onRightSide, uint16 frameIndex) {
	if (_gameObject->_blobs.size() <= 17) {
		return nullptr;
	}

	uint portraitBlobIndex = 17;
	if (_gameObject->_blobs.size() > 18 && !_gameObject->_blobs[18].empty()) {
		if (onRightSide || _gameObject->_blobs[portraitBlobIndex].empty()) {
			portraitBlobIndex = 18;
		}
	}

	if (_gameObject->_blobs[portraitBlobIndex].empty()) {
		return nullptr;
	}

	const uint16 offset = BackgroundAnimationBlob::advanceAnimFrame(_gameObject->_blobs[portraitBlobIndex], true, frameIndex);
	// offset points to per-frame: offsetX(2), offsetY(2), unknown(2), width(2), height(2), pixels
	Common::Array<uint8> &blob = _gameObject->_blobs[portraitBlobIndex];
	AnimFrame *result = new AnimFrame();
	result->_width = READ_LE_UINT16(&blob[offset + 6]);
	result->_height = READ_LE_UINT16(&blob[offset + 8]);
	result->_data.resize(result->_width * result->_height);
	memcpy(result->_data.data(), &blob[offset + 10], result->_width * result->_height);
	return result;
}

// Leftover lerp-era entry point. Duration and ignoreObstacles are unused.
// Binary walkAlongPath (1008:1b8f) has no time lerp: Phase 0 sets 8-way
// orientation and returns (1-frame delay); Phase 1 loops stepCounter 1..walkSpeed
// with one-pixel Bresenham (error >= deltaX -> step Y else step X).
// walkSpeed = animSpeed * (scene[0x5201] + depth) / 100, min 1;
// depth = (posY - scene[0x51FD]) * scene[0x51FF] / 100.
// Character::update() implements that. C++ walkAlongPath() is only the inlined
// waypoint advance. Path setup is walkToScreenPosition / scriptWalkToPosition.
void Character::startLerpTo(const Common::Point &target, uint32 duration, bool ignoreObstacles) {
	_startPosition = getPosition();
	_targetPosition = target;
	_startTime = g_events->currentMillis;
	_duration = duration;

	// Reset Bresenham state - direction will be calculated on first Update()
	_stepDirectionSet = false;
	_stepDeltaX = (int16)ABS(_targetPosition.x - _startPosition.x);
	_stepDeltaY = (int16)ABS(_targetPosition.y - _startPosition.y);
	_stepError = 0;
}

void Character::startPickup(Macs2::GameObject *object) {
	_pickedUpObject = object;
	_pathFinalDestination = getObjectEffectivePosition(object);
	_pickupFrameCounter = 0;
	_pickupItemTransferred = false;

	const Common::Point &current = getPosition();
	const int16 destX = _pathFinalDestination.x;
	const int16 destY = _pathFinalDestination.y;

	_currentPathIndex = 0;
	_path.clear();

	const bool directPath = g_engine->isPathWalkable(destY, destX, current.y, current.x);
	if (!directPath && Macs2Engine::isWalkabilityWalkable(g_engine->getWalkabilityAt(destY, destX))) {
		calculatePath(Common::Point(destX, destY));
	}

	if (_path.empty()) {
		_targetPosition = _pathFinalDestination;
	}

	_stepDeltaX = (int16)ABS(_targetPosition.x - current.x);
	_stepDeltaY = (int16)ABS(_targetPosition.y - current.y);
	_stepError = 0;
	_stepDirectionSet = false;
}

bool Character::hasPendingVerticalMotion() const {
	return (int16)_motionTargetVerticalOffset >= 0 &&
		   _motionTargetVerticalOffset != _gameObject->_verticalOffsetScale;
}

bool Character::shouldStepVerticalMotion() const {
	return (int16)_motionTargetVerticalOffset < 0 ||
		   _motionTargetVerticalOffset != _gameObject->_verticalOffsetScale;
}

void Character::update() {
	Script::ScriptExecutor *exec = g_engine->_scriptExecutor;
	if (_gameObject->_orientation == OrientationPickup) {
		if (_pickedUpObject != nullptr) {
			View1 *currentView = (View1 *)g_engine->findView("View1");

			if (!_pickupItemTransferred && _pickupFrameCounter == _gameObject->_pickupFrameStart) {
				_pickupItemTransferred = true;
				currentView->transferPickupTarget(_pickedUpObject);
			}

			if (_pickupFrameCounter == _gameObject->_pickupFrameEnd) {
				_gameObject->_orientation = _previousOrientation;
				if (exec->_pickupInProgress) {
					exec->_pickupInProgress = false;
					exec->_pickupActorObjectID = 0;
					exec->_pickupTargetObjectID = 0;
					g_engine->setCursorMode(exec->_cursorModeBeforeWait);
					currentView->updateCursor();
				}
				exec->_walkTargetObjectIndex = 0;
				_pickedUpObject = nullptr;
				exec->_interactedObjectID = 0;
				exec->_interactedInventoryItemId = 0;
				g_engine->_movementFinishedFlag = true;
				return;
			}

			_pickupFrameCounter++;
		}
		return;
	}

	Common::Point pos = getPosition();
	const int32 depthOffset = ((int32)pos.y - (int32)g_engine->_walkDepthThresholdY) *
							(int32)g_engine->_walkDepthScaleFactor / 100;
	// Per-animation speed from blob data
	// Walk speed from binary walkAlongPath
	uint16 animSpeed = 2; // default fallback
	ObjectOrientation orient = _gameObject->_orientation;
	if (orient >= OrientationNorth && orient <= g_engine->maxAnimSlots() && (uint)(orient - 1) < _gameObject->_blobWalkSpeeds.size()) {
		animSpeed = _gameObject->_blobWalkSpeeds[orient - 1];
		if (animSpeed == 0) {
			animSpeed = 2;
		}
	}
	int walkSpeed = ((int)animSpeed * ((int)g_engine->_walkBaseSpeedPct + (int)depthOffset)) / 100;
	if (walkSpeed < 1) {
		walkSpeed = 1;
	}

	// Proximity arrival check from walkAlongPath
	bool arrived = (ABS(pos.x - _targetPosition.x) <= walkSpeed) &&
				   (ABS(pos.y - _targetPosition.y) <= walkSpeed);
	if (arrived && hasPendingVerticalMotion()) {
		arrived = false;
	}
	if (arrived) {
		const bool atFinalDest = (_targetPosition.x == _pathFinalDestination.x &&
							_targetPosition.y == _pathFinalDestination.y);

		if (!atFinalDest && !_path.empty()) {
			// Mid-path waypoint arrival: advance to next node
			// Binary (23b0): snap pos to current path node, advance pathIndex
			walkAlongPath();
			return;
		}

		// Final destination arrival (or direct walk arrival)
		if (_gameObject->_snapToTarget) {
			setPosition(_targetPosition);
			_pathFinalDestination = _targetPosition;
		} else {
			_targetPosition = pos;
			_pathFinalDestination = pos;
			if ((int16)_motionTargetVerticalOffset >= 0) {
				_motionTargetVerticalOffset = _gameObject->_verticalOffsetScale;
			}
		}
		_path.clear();
		if (hasPendingVerticalMotion()) {
			_gameObject->_verticalOffsetScale = _motionTargetVerticalOffset;
			_motionProgress = _motionDistanceUnits;
		}
		// Walk arrival: orientation changes to standing (walking dir + 8).
		// Script resumption is handled by position polling in View1::tick().
		const bool wasWalking = (_gameObject->_orientation < OrientationStandingNorth);
		if (wasWalking) {
			_gameObject->_orientation = (ObjectOrientation)(_gameObject->_orientation + OrientationNorthWest);
			g_engine->_movementFinishedFlag = true;
		}
		return;
	}

	// Binary: if target==current position, skip Phase 0 turn delay (set directionCalculated=1)
	if (!_stepDirectionSet && _targetPosition.x == pos.x && _targetPosition.y == pos.y) {
		_stepDirectionSet = true;
	}

	// Calculate direction if not yet set (first frame of movement)
	if (!_stepDirectionSet) {
		_stepDirectionSet = true;
		// Phase 0 from walkAlongPath (1008:1b8f): direction calculation.
		// Binary returns after setting direction (1-frame turn delay).
		const uint16 absDx = (uint16)ABS(pos.x - _targetPosition.x);
		const uint16 absDy = (uint16)ABS(pos.y - _targetPosition.y);
		ObjectOrientation dir = _gameObject->_orientation;
		if (dir >= OrientationStandingNorth && dir <= OrientationStandingNorthWest)
			dir = (ObjectOrientation)(dir - OrientationNorthWest);
		if (dir > OrientationStandingNorthWest)
			dir = OrientationNorth;
		// Cardinal directions (only if animation available for that direction)
		if (_targetPosition.y < pos.y && absDx <= absDy &&
			_gameObject->_blobs.size() > 0 && !_gameObject->_blobs[0].empty())
			dir = OrientationNorth;
		if (pos.x < _targetPosition.x && absDy <= absDx &&
			_gameObject->_blobs.size() > 2 && !_gameObject->_blobs[2].empty())
			dir = OrientationEast;
		if (pos.y < _targetPosition.y && absDx <= absDy &&
			_gameObject->_blobs.size() > 4 && !_gameObject->_blobs[4].empty())
			dir = OrientationSouth;
		if (_targetPosition.x < pos.x && absDy <= absDx &&
			_gameObject->_blobs.size() > 6 && !_gameObject->_blobs[6].empty())
			dir = OrientationWest;
		// Diagonals: absDx/4 < absDy AND absDy/2 < absDx
		if ((absDx >> 2) < absDy && (absDy >> 1) < absDx) {
			if (_targetPosition.y < pos.y && pos.x < _targetPosition.x &&
				_gameObject->_blobs.size() > 1 && !_gameObject->_blobs[1].empty())
				dir = OrientationNorthEast;
			if (pos.x < _targetPosition.x && pos.y < _targetPosition.y &&
				_gameObject->_blobs.size() > 3 && !_gameObject->_blobs[3].empty())
				dir = OrientationSouthEast;
			if (pos.y < _targetPosition.y && _targetPosition.x < pos.x &&
				_gameObject->_blobs.size() > 5 && !_gameObject->_blobs[5].empty())
				dir = OrientationSouthWest;
			if (_targetPosition.x < pos.x && _targetPosition.y < pos.y &&
				_gameObject->_blobs.size() > 7 && !_gameObject->_blobs[7].empty())
				dir = OrientationNorthWest;
		}
		_gameObject->_orientation = dir;
		_stepDeltaX = (int16)absDx;
		_stepDeltaY = (int16)absDy;
		_stepError = 0;
		// 1-frame turn delay: return after setting direction (binary Phase 0)
		return;
	}

	// Phase 1: Bresenham stepping loop - exact 1:1 match of binary (1008:1ea1..2280)
	// Binary: stepCounter from 1 to walkSpeed, NO early break. Loop always completes.
	// After loop: if pixelsMoved != walkSpeed -> revert pos to savedPos and cancel path.
	int pixelsMoved = 0;
	Common::Point savedPos = pos;
	for (int stepCounter = 1; stepCounter <= walkSpeed; stepCounter++) {
		savedPos = pos; // Binary: savedX/savedY at top of each iteration
		// Bresenham: if error >= deltaX -> step Y, else step X
		if (_stepError >= _stepDeltaX) {
			// Step Y axis
			if (_targetPosition.y != pos.y)
				pixelsMoved++;
			if (_targetPosition.y < pos.y)
				pos.y--;
			else if (_targetPosition.y > pos.y)
				pos.y++;
			_stepError -= _stepDeltaX;
		} else {
			// Step X axis
			if (_targetPosition.x != pos.x)
				pixelsMoved++;
			if (_targetPosition.x < pos.x)
				pos.x--;
			else if (_targetPosition.x > pos.x)
				pos.x++;
			_stepError += _stepDeltaY;
		}
		// Vertical offset interpolation
		if (shouldStepVerticalMotion()) {
			_motionProgress += _motionVerticalOffsetDelta;
			while (_motionProgress >= _motionDistanceUnits && _motionDistanceUnits > 0) {
				_motionProgress -= _motionDistanceUnits;
				if (_motionTargetVerticalOffset < _gameObject->_verticalOffsetScale)
					_gameObject->_verticalOffsetScale--;
				else if (_motionTargetVerticalOffset > _gameObject->_verticalOffsetScale)
					_gameObject->_verticalOffsetScale++;
			}
		}
		// Walkability check - binary uses getWalkabilityAt(posY, posX) >= 0xC8
		if (!isWalkable(pos)) {
			const uint16 tileArea = exec->getAreaAtPoint(pos.x, pos.y);
			if (tileArea >= 210 && tileArea <= 215) {
				debugC(kDebugPath,
						"walk blocked on plate area %u at (%d,%d) walk=%u int16=%d target=(%d,%d)",
						tileArea, pos.x, pos.y, lookupWalkability(pos), (int16)lookupWalkability(pos),
						_targetPosition.x, _targetPosition.y);
			}
			// Revert position
			pos = savedPos;
			// Wall-sliding: build push vector from +/-1 and +/-2 samples
			int pushX = 0, pushY = 0;
			if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x + 1, pos.y))))
				pushX--;
			if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x - 1, pos.y))))
				pushX++;
			if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x, pos.y + 1))))
				pushY--;
			if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x, pos.y - 1))))
				pushY++;
			if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x + 2, pos.y))))
				pushX--;
			if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x - 2, pos.y))))
				pushX++;
			if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x, pos.y + 2))))
				pushY--;
			if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x, pos.y - 2))))
				pushY++;
			// Apply push vector
			while (pushX != 0 || pushY != 0) {
				if (pushX < 0) {
					if (Macs2Engine::isWalkabilityWalkable(lookupWalkability(Common::Point(pos.x - 1, pos.y))))
						pos.x--;
					pushX++;
				}
				if (pushX > 0) {
					if (Macs2Engine::isWalkabilityWalkable(lookupWalkability(Common::Point(pos.x + 1, pos.y))))
						pos.x++;
					pushX--;
				}
				if (pushY < 0) {
					if (Macs2Engine::isWalkabilityWalkable(lookupWalkability(Common::Point(pos.x, pos.y - 1))))
						pos.y--;
					pushY++;
				}
				if (pushY > 0) {
					if (Macs2Engine::isWalkabilityWalkable(lookupWalkability(Common::Point(pos.x, pos.y + 1))))
						pos.y++;
					pushY--;
				}
			}
			// Binary: target = finalDest = pos (cancel path, but loop continues)
			_targetPosition = pos;
			_pathFinalDestination = pos;
			_path.clear();
		}
		// Binary: loop continues unconditionally until stepCounter == walkSpeed
	}

	if (pixelsMoved != walkSpeed) {
		const uint16 tileArea = exec->getAreaAtPoint(pos.x, pos.y);
		if (tileArea >= 210 && tileArea <= 215) {
			debugC(kDebugPath,
				   "walk cancelled pixelsMoved=%d walkSpeed=%d at (%d,%d) area=%u walk=%u finalDest=(%d,%d)",
				   pixelsMoved, walkSpeed, pos.x, pos.y, tileArea, lookupWalkability(pos),
				   _pathFinalDestination.x, _pathFinalDestination.y);
		} else if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(pos))) {
			debugC(kDebugPath,
				   "walk cancelled (non-walkable) pixelsMoved=%d walkSpeed=%d at (%d,%d) walk=%u",
				   pixelsMoved, walkSpeed, pos.x, pos.y, lookupWalkability(pos));
		}
		pos = savedPos;
		_targetPosition = pos;
		_pathFinalDestination = pos;
		_path.clear();
	}

	setPosition(pos);
}

} // namespace Macs2
