/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "m4/m4.h"
#include "m4/mads_player.h"
#include "m4/mads_scene.h"

namespace M4 {

const int MadsPlayer::_directionListIndexes[32] = {
	0, 7, 4, 3, 6, 0, 2, 5, 0, 1, 9, 4, 1, 2, 7, 9, 3, 8, 9, 6, 7, 2, 3, 6, 1, 7, 9, 4, 7, 8, 0, 0
};

MadsPlayer::MadsPlayer() {
	_playerPos = Common::Point(160, 78);
	_ticksAmount = 3;
	_forceRefresh = true;
	_stepEnabled = true;
	_visible = true;
	_yScale = 0;
	_moving = false;

	_spriteListStart = 0;
	//TODO:unknown vars
	_special = 0;
	_next = 0;
	_unk4 = false;

	_spritesChanged = true;

	_direction = 0;
	_newDirection = 0;
	_priorTimer = 0;
	_priorVisible = false;
	_visible3 = false;
	_spriteListIdx = 0;
	_currentScale = 0;
	strcpy(_spritesPrefix, "");
	for (int idx = 0; idx < 8; ++idx)
		_spriteSetsPresent[idx] = false;
	_frameNum = 0;
	_frameOffset = 0;
	_unk1 = 0;
	_frameCount = 0;
	_frameListIndex = 0;
	_actionIndex = 0;
	_routeCount = 0;

	resetActionList();
}

/**
 * Loads the sprite set for the player
 */
bool MadsPlayer::loadSprites(const char *prefix) {
	const char suffixList[8] = { '8', '9', '6', '3', '2', '7', '4', '1' };
	char setName[80];
	bool result = true;

	if (prefix)
		strcpy(_spritesPrefix, prefix);

	_spriteSetCount = 0;
	int prefixLen = strlen(_spritesPrefix);

	if (prefixLen == 0) {
		// No player sprites at at all
		for (int idx = 0; idx < 8; ++idx)
			_spriteSetsPresent[idx] = false;
	} else {
		strcpy(setName, "*");
		strcat(setName, _spritesPrefix);
		strcat(setName, "_0.SS");

		char *digitP = strchr(setName, '_') + 1;

		for (int idx = 0; idx < 8; ++idx) {
			*digitP = suffixList[idx];
			_spriteSetsPresent[idx] = true;

			int setIndex = _madsVm->scene()->_spriteSlots.addSprites(setName, true, SPRITE_SET_CHAR_INFO);
			if (setIndex < 0) {
				if (idx < 5)
					break;
				_spriteSetsPresent[idx] = false;
			} else {
				++_spriteSetCount;
			}

			if (idx == 0)
				_spriteListStart = setIndex;
		}

		result = 0;
		// TODO: Unknown flag
		_spritesChanged = false;
	}

	return result;
}

/**
 * Called each frame to update the display of the player
 */
void MadsPlayer::update() {
	if (_forceRefresh || (_visible != _priorVisible)) {
		// If there's an existing player sprite visible, flag it for expiry
		int slotIndex = getSpriteSlot();
		if (slotIndex >= 0)
			_madsVm->scene()->_spriteSlots[slotIndex].spriteType = EXPIRED_SPRITE;

		// Figure out the depth for the sprite
		int newDepth = 1;
		int yp = MIN(_playerPos.y, (int16)155);

		for (int idx = 1; idx < 15; ++idx) {
			if (_madsVm->scene()->getSceneResources()._depthBands[newDepth] >= yp)
				newDepth = idx + 1;
		}
		_currentDepth = newDepth;

		// Get the scale
		int newScale = getScale(_playerPos.y);
		_currentScale = MIN(newScale, 100);

		if (_visible) {
			// Player sprite needs to be rendered
			MadsSpriteSlot slot;
			slot.spriteType = FOREGROUND_SPRITE;
			slot.seqIndex = PLAYER_SEQ_INDEX;
			slot.spriteListIndex = _spriteListStart + _spriteListIdx;
			slot.frameNumber = _frameOffset + _frameNum;
			slot.xp = _playerPos.x;
			slot.yp = _playerPos.y + (_yScale * newScale) / 100;
			slot.depth = newDepth;
			slot.scale = newScale;

			if (slotIndex >= 0) {
				// Check if the existing player slot has the same details, and can be re-used
				MadsSpriteSlot &s2 = _madsVm->scene()->_spriteSlots[slotIndex];
				bool equal = (s2.seqIndex == slot.seqIndex) && (s2.spriteListIndex == slot.spriteListIndex)
					&& (s2.frameNumber == slot.frameNumber) && (s2.xp == slot.xp) && (s2.yp == slot.yp)
					&& (s2.depth == slot.depth) && (s2.scale == slot.scale);

				if (equal)
					// Undo the prior expiry of the player sprite
					s2.spriteType = SPRITE_ZERO;
				else
					slotIndex = -1;
			}

			if (slotIndex < 0) {
				// New slot needed, so allocate one and copy the slot data
				slotIndex = _madsVm->scene()->_spriteSlots.getIndex();
				_madsVm->scene()->_spriteSlots[slotIndex] = slot;
			}

			// TODO: Meaning of _v844c0 block

		}
	}

	_visible3 = _priorVisible = _visible;
	_forceRefresh = false;
}

/**
 * Updates the animation frame for the player
 */
void MadsPlayer::updateFrame() {
	SpriteAsset &spriteSet = _madsVm->scene()->_spriteSlots.getSprite(_spriteListStart + _spriteListIdx);
	assert(spriteSet._charInfo);

	if (!spriteSet._charInfo->_numEntries) {
		_frameNum = 1;
	} else {
		_frameListIndex = _actionList[_actionIndex];

		if (!_visible) {
			_unk2 = 0;
		} else {
			_unk2 = _actionList2[_actionIndex];

			if (_actionIndex > 0)
				--_actionIndex;
		}

		// Set the player frame number
		int frameIndex = ABS(_frameListIndex);
		_frameNum = (_frameListIndex <= 0) ? spriteSet._charInfo->_frameList[frameIndex] :
			spriteSet._charInfo->_frameList2[frameIndex];

		// Set next waiting period in ticks
		if (frameIndex == 0)
			setTicksAmount();
		else
			_madsVm->_player._ticksAmount = spriteSet._charInfo->_ticksList[frameIndex];
	}
}

void MadsPlayer::setupFrame() {
	resetActionList();
	_frameOffset = 0;
	_spriteListIdx = _directionListIndexes[_direction];
	if (!_spriteSetsPresent[_spriteListIdx]) {
		// Direction isn't present, so use alternate direction, with entries flipped
		_spriteListIdx -= 4;
		_frameOffset = 0x8000;
	}

	SpriteAsset &spriteSet = _madsVm->scene()->_spriteSlots.getSprite(_spriteListStart + _spriteListIdx);
	assert(spriteSet._charInfo);
	_unk1 = MAX(spriteSet._charInfo->_unk1, 100);
	setTicksAmount();

	_frameCount = spriteSet._charInfo->_totalFrames;
	if (_frameCount == 0)
		_frameCount = spriteSet.getCount();

	_yScale = spriteSet._charInfo->_yScale;

	if ((_frameNum <= 0) || (_frameNum > _frameCount))
		_frameNum = 1;
	_forceRefresh = true;
}

void MadsPlayer::step() {
	if (_visible && _stepEnabled && !_moving && (_direction == _newDirection) && (_madsVm->_currentTimer >= GET_GLOBAL32(2))) {
		if (_actionIndex == 0) {
			int randVal = _vm->_random->getRandomNumber(29999);

			if (GET_GLOBAL(0) == SEX_MALE) {
				switch (_direction) {
				case 1:
				case 3:
				case 7:
				case 9:
					if (randVal < 200) {
						queueAction(-1, 0);
						queueAction(1, 0);
					}
					break;

				case 2:
					if (randVal < 500) {
						for (int i = 0; i < 10; ++i)
							queueAction((randVal < 250) ? 1 : 2, 0);
					} else if (randVal < 750) {
						for (int i = 0; i < 5; ++i)
							queueAction(1, 0);
						queueAction(0, 0);
						for (int i = 0; i < 5; ++i)
							queueAction(2, 0);
					}
					break;

				case 4:
				case 6:
					if (randVal < 500) {
						for (int i = 0; i < 10; ++i)
							queueAction(1, 0);
					}
					break;

				case 5:
				case 8:
					if (randVal < 200) {
						queueAction(-1, 0);
						queueAction(1, 0);
					}
					break;
				}
			}
		}

		SET_GLOBAL32(2, GET_GLOBAL32(2) + 6);
	}

	if (GET_GLOBAL(138) == 1) {
		uint32 diff = _madsVm->_currentTimer - GET_GLOBAL32(142);
		if (diff > 60) {
			SET_GLOBAL32(144, GET_GLOBAL32(144) + 1);
		} else {
			SET_GLOBAL32(144, GET_GLOBAL32(144) + diff);
		}

		SET_GLOBAL32(142, _madsVm->_currentTimer);
	}
}

void MadsPlayer::nextFrame() {
	if (_madsVm->_currentTimer >= (_priorTimer + _ticksAmount)) {
		_priorTimer = _madsVm->_currentTimer;

		if (_moving)
			move();
		else
			idle();

		// Post update logic
		if (_moving) {
			++_frameNum;
			if (_frameNum > _frameCount)
				_frameNum = 1;
			_forceRefresh = true;
		} else if (!_forceRefresh) {
			idle();
		}

		// Final update
		update();
	}
}

void MadsPlayer::setDest(int destX, int destY, int facing) {
	resetActionList();
	setTicksAmount();
	_moving = true;
	_destFacing = facing;

	_madsVm->scene()->getSceneResources().setRouteNode(_madsVm->scene()->getSceneResources()._nodes.size() - 2,
		_playerPos, _madsVm->scene()->_depthSurface);
	_madsVm->scene()->getSceneResources().setRouteNode(_madsVm->scene()->getSceneResources()._nodes.size() - 1,
		Common::Point(destX, destY), _madsVm->scene()->_depthSurface);

	bool v = _madsVm->scene()->getDepthHighBit(Common::Point(destX, destY));
	setupRoute(v);
	_next = 0;

	if (_routeCount > 0) {
		Common::Point srcPos = _playerPos;
		for (int routeCtr = _routeCount - 1; (routeCtr >= 0) && (_next == 0); --routeCtr) {
			int idx = _routeIndexes[routeCtr];
			const Common::Point &pt = _madsVm->scene()->getSceneResources()._nodes[idx].pt;

			_next = scanPath(_madsVm->scene()->_depthSurface, srcPos, pt);
			srcPos = pt;
		}
	}
}


int MadsPlayer::getScale(int yp) {
	MadsSceneResources &r = _madsVm->scene()->getSceneResources();

	int scale = (r.bandsRange() == 0) ? r._maxScale : (yp - r._yBandsStart) * r.scaleRange() / r.bandsRange()
		+ r._minScale;

	return MIN(scale, 100);
}

/**
 * Scans through the scene's sprite slot list to find any sprite displaying the player
 */
int MadsPlayer::getSpriteSlot() {
	MadsSpriteSlots &slots = _madsVm->scene()->_spriteSlots;
	for (int i = 0; i < slots.startIndex; ++i) {
		if ((slots[i].seqIndex == PLAYER_SEQ_INDEX) && (slots[i].spriteType >= SPRITE_ZERO))
			return i;
	}
	return -1;
}

void MadsPlayer::setTicksAmount() {
	SpriteAsset &spriteSet = _madsVm->scene()->_spriteSlots.getSprite(_spriteListStart + _spriteListIdx);
	assert(spriteSet._charInfo);
	_madsVm->_player._ticksAmount = spriteSet._charInfo->_ticksAmount;
	if (_madsVm->_player._ticksAmount == 0)
		_madsVm->_player._ticksAmount = 6;
}

void MadsPlayer::resetActionList() {
	_actionList[0] = 0;
	_actionList2[0] = 0;
	_actionIndex = 0;
	_unk2 = 0;
	_unk3 = 0;
}

int MadsPlayer::queueAction(int action1, int action2) {
	SpriteAsset &spriteSet = _madsVm->scene()->_spriteSlots.getSprite(_spriteListStart + _spriteListIdx);
	assert(spriteSet._charInfo);

	if ((action1 < spriteSet._charInfo->_numEntries) && (_actionIndex < 11)) {
		++_actionIndex;
		_actionList[_actionIndex] = action1;
		_actionList2[_actionIndex] = action2;
		return false;
	}

	return true;
}

void MadsPlayer::idle() {
	if (_direction != _newDirection) {
		// The direction has changed, so reset for new direction
		dirChanged();
		return;
	}

	SpriteAsset &spriteSet = _madsVm->scene()->_spriteSlots.getSprite(_spriteListStart + _spriteListIdx);
	assert(spriteSet._charInfo);
	if (spriteSet._charInfo->_numEntries == 0)
		// No entries, so exit immediately
		return;

	int frameIndex = ABS(_frameListIndex);
	int direction = (_frameListIndex < 0) ? -1 : 1;

	if (frameIndex >= spriteSet._charInfo->_numEntries)
		// Reset back to the start of the list
		_frameListIndex = 0;
	else {
		_frameNum += direction;
		_forceRefresh = true;

		if (spriteSet._charInfo->_frameList2[frameIndex] < _frameNum) {
			_unk3 = _unk2;
			updateFrame();
		}
		if (spriteSet._charInfo->_frameList[frameIndex] < _frameNum) {
			_unk3 = _unk2;
			updateFrame();
		}
	}
}

void MadsPlayer::move() {
	bool routeFlag = false;

	if (_moving) {
		int idx = _routeCount;
		while (!_v844C0 && (_destPos.x == _playerPos.x) && (_destPos.y == _playerPos.y)) {
			if (idx != 0) {
				--idx;
				SceneNode &node = _madsVm->scene()->getSceneResources()._nodes[_routeIndexes[idx]];
				_destPos = node.pt;
				routeFlag = true;
			} else if (_v844BE == idx) {
				// End of walking path
				_routeCount = 0;
				_moving = false;
				turnToDestFacing();
				routeFlag = true;
				idx = _routeCount;
			} else {
				_v844C0 = _v844BE;
				_v844BC = true;
				_v844BE = 0;
				_stepEnabled = true;
				routeFlag = false;
			}

			if (!_moving)
				break;
		}
		_routeCount = idx;
	}

	if (routeFlag && _moving)
		startMovement();

	if (_newDirection != _direction)
		dirChanged();
	else if (!_moving)
		updateFrame();

	int var1 = _unk1;
	if (_unk4 && (_hypotenuse > 0)) {
		int v1 = -(_currentScale - 100) * (_posDiff.x - 1) / _hypotenuse + _currentScale;
		var1 = MAX(1, 10000 / (v1 * _currentScale * var1));
	}

	if (!_moving || (_direction != _newDirection))
		return;

	Common::Point newPos = _playerPos;

	if (_v8452E < var1) {
		do {
			if (_v8452C < _posDiff.x)
				_v8452C += _posDiff.y;
			if (_v8452C >= _posDiff.x) {
				if ((_posChange.y > 0) || (_v844C0 != 0))
					newPos.y += _yDirection;
				--_posChange.y;
				_v8452C -= _posDiff.x;
			}

			if (_v8452C < _posDiff.x) {
				if ((_posChange.x > 0) || (_v844C0 != 0))
					newPos.x += _xDirection;
				--_posChange.x;
			}

			if ((_v844BC == 0) && (_v844C0 == 0) && (_v844BE == 0)) {
				routeFlag = _madsVm->scene()->getDepthHighBit(newPos);

				if (_special == 0)
					_special = _madsVm->scene()->getDepthHighBits(newPos);
			}

			_v8452E += _v84530;

		} while ((_v8452E < var1) && !routeFlag && ((_posChange.x > 0) || (_posChange.y > 0) || (_v844C0 != 0)));
	}

	_v8452E -= var1;

	if (routeFlag)
		moveComplete();
	else {
		if (!_v844C0) {
			// If the move is complete, make sure the position is exactly on the given destination
			if (_posChange.x == 0)
				newPos.x = _destPos.x;
			if (_posChange.y == 0)
				newPos.y = _destPos.y;
		}

		_playerPos = newPos;
	}
}

void MadsPlayer::dirChanged() {
	int dirIndex = 0, dirIndex2 = 0;
	int newDir = 0, newDir2 = 0;

	if (_direction != _newDirection) {
		// Find the index for the given direction in the player direction list
		int tempDir = _direction;
		do {
			++dirIndex;
			newDir += tempDir;
			tempDir = _directionListIndexes[tempDir + 10];
		} while (tempDir != _newDirection);
	}


	if (_direction != _newDirection) {
		// Find the index for the given direction in the player direction list
		int tempDir = _direction;
		do {
			++dirIndex2;
			newDir2 += tempDir;
			tempDir = _directionListIndexes[tempDir + 20];
		} while (tempDir != _newDirection);
	}

	int diff = dirIndex - dirIndex2;
	if (diff == 0)
		diff = newDir - newDir2;

	_direction = (diff >= 0) ? _directionListIndexes[_direction + 20] : _directionListIndexes[_direction + 10];
	setupFrame();
	if ((_direction == _newDirection) && !_moving)
		updateFrame();

	_priorTimer += 1;
}

void MadsPlayer::moveComplete() {
	reset();
	//todo: Unknown flag
}

void MadsPlayer::reset() {
	_destPos = _playerPos;
	_destFacing = 5;
	_newDirection = _direction;

	_madsVm->scene()->_action._startWalkFlag = false;
	_madsVm->scene()->_action._walkFlag = false;
	_moving = false;
	_v844BC = false;
	_v844C0 = false;
	_v844BE = 0;
	_next = 0;
	_routeCount = 0;
}

/**
 * Scans along an edge connecting two points within the depths/walk surface, and returns the information of the first
 * pixel high nibble encountered with a non-zero value
 */
int MadsPlayer::scanPath(M4Surface *depthSurface, const Common::Point &srcPos, const Common::Point &destPos) {
	// For compressed depth surfaces, always return 0
	if (_madsVm->scene()->getSceneResources()._depthStyle != 2)
		return 0;

	int yDiff = destPos.y - srcPos.y;
	int yAmount = MADS_SURFACE_WIDTH;

	if (yDiff < 0) {
		yDiff = -yDiff;
		yAmount = -yAmount;
	}

	int xDiff = destPos.x - srcPos.y;
	int xDirection = 1;
	int xAmount = 0;
	if (xDiff < 0) {
		xDiff = -xDiff;
		xDirection = -xDirection;
		xAmount = MIN(yDiff, xDiff);
	}

	++xDiff;
	++yDiff;

	const byte *srcP = depthSurface->getBasePtr(srcPos.x, srcPos.y);
	int index = xAmount;

	// Outer horizontal movement loop
	for (int yIndex = 0; yIndex < yDiff; ++yIndex) {
		index += yDiff;
		int v = (*srcP & 0x7F) >> 4;
		if (v)
			return v;

		// Inner loop for handling vertical movement
		while (index >= xDiff) {
			index -= xDiff;

			v = (*srcP & 0x7F) >> 4;
			if (v)
				return v;

			srcP += yAmount;
		}

		srcP += xDirection;
	}

	return 0;
}

/**
 * Starts a player moving to a given destination
 */
void MadsPlayer::startMovement() {
	int xDiff = _destPos.x - _playerPos.x;
	int yDiff = _destPos.y - _playerPos.y;
	int srcScale = getScale(_playerPos.y);
	int destScale = getScale(_destPos.y);

	// Sets the X direction
	if (xDiff > 0)
		_xDirection = 1;
	else if (xDiff < 0)
		_xDirection = -1;
	else
		_xDirection = 0;

	// Sets the Y direction
	if (yDiff > 0)
		_yDirection = 1;
	else if (yDiff < 0)
		_yDirection = -1;
	else
		_yDirection = 0;

	xDiff = ABS(xDiff);
	yDiff = ABS(yDiff);
	int scaleDiff = ABS(srcScale - destScale);

	int xAmt100 = xDiff * 100;
	int yAmt100 = yDiff * 100;
	int xAmt33 = xDiff * 33;

	int scaleAmount = (_unk4 ? scaleDiff * 3 : 0) + 100 * yDiff / 100;
	int scaleAmount100 = scaleAmount * 100;

	// Figure out direction that will need to be moved in
	int majorDir;
	if (xDiff == 0)
		majorDir = 1;
	else if (yDiff == 0)
		majorDir = 3;
	else {
		if ((scaleAmount < xDiff) && ((xAmt33 / scaleAmount) >= 141))
			majorDir = 3;
		else if (yDiff <= xDiff)
			majorDir = 2;
		else if ((scaleAmount100 / xDiff) >= 141)
			majorDir = 1;
		else
			majorDir = 2;
	}

	switch (majorDir) {
	case 1:
		_newDirection = (_yDirection <= 0) ? 8 : 2;
		break;
	case 2: {
		_newDirection = ((_yDirection <= 0) ? 9 : 3) - ((_xDirection <= 0) ? 2 : 0);
		break;
	}
	case 3:
		_newDirection = (_xDirection <= 0) ? 4 : 6;
		break;
	default:
		break;
	}

	_hypotenuse = SqrtF16(xAmt100 * xAmt100 + yAmt100 * yAmt100);
	_posDiff.x = xDiff + 1;
	_posDiff.y = yDiff + 1;
	_posChange.x = xDiff;
	_posChange.y = yDiff;

	int majorChange = MAX(xDiff, yDiff);
	_v84530  = (majorChange == 0) ? 0 : _hypotenuse / majorChange;

	if (_playerPos.x > _destPos.x)
		_v8452C = MAX(_posChange.x, _posChange.y);
	else
		_v8452C = 0;

	_hypotenuse /= 100;
	_v8452E = -_v84530;
}

void MadsPlayer::turnToDestFacing() {
	if (_destFacing != 5)
		_newDirection = _destFacing;
}

void MadsPlayer::setupRoute(bool bitFlag) {
	// Reset the flag set of nodes in use
	SceneNodeList &nodes = _madsVm->scene()->getSceneResources()._nodes;
	for (uint i = 0; i < nodes.size(); ++i)
		nodes[i].active = false;

	// Start constructing route node list
	_routeLength = 0x3FFF;
	_routeCount = 0;

	setupRouteNode(_tempRoute, nodes.size() - 1, bitFlag ? 0xC000 : 0x8000, 0);
}

void MadsPlayer::setupRouteNode(int *routeIndexP, int nodeIndex, int flags, int routeLength) {
	SceneNodeList &nodes = _madsVm->scene()->getSceneResources()._nodes;
	SceneNode &currentNode = nodes[nodeIndex];
	currentNode.active = true;

	*routeIndexP++ = nodeIndex;

	int subIndex = nodes.size() - 2;
	int indexVal = nodes[nodeIndex].indexes[subIndex];
	if (indexVal & flags) {
		routeLength += indexVal & 0x3FFF;
		if (routeLength < _routeLength) {
			// Found a new shorter route to destination, so set up the route with the found one
			Common::copy(_tempRoute, routeIndexP, _routeIndexes);
			_routeCount = routeIndexP - _tempRoute;
			_routeLength = indexVal & 0x3FFF;
		}
	} else {
		for (int idx = nodes.size() - 2; idx > 0; --idx) {
			int nodePos = idx - 1;
			if (!nodes[nodePos].active && ((currentNode.indexes[nodePos] & flags) != 0))
				setupRouteNode(routeIndexP, nodePos, 0x8000, indexVal & 0x3fff);
		}
	}

	currentNode.active = false;
}

} // End of namespace M4
