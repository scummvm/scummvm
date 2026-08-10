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

// Source-backed Hopkins WBASE simulation and ACK-style ray traversal.

#include "hopkins/base_engine.h"

#include "common/algorithm.h"
#include "common/util.h"

namespace Hopkins {

namespace {

enum AckMoveResult {
	kAckNothing = 0,
	kAckXWall = 1,
	kAckYWall = 2,
	kAckObject = 3,
	kAckPlayer = 4,
	kAckSlideX = 5,
	kAckSlideY = 6
};

static const int kPlayerMoveAmount = 12;
static const int kGuardMoveAmount = 5;
static const int kAckCollisionDistance = 28;
static const int kDoorSpeed = 4;
static const int kDoorMaximumOffset = 0xa0;
static const int kWeaponStartCounter = 8;
static const int kDeathExit = 152;

static int iabs(int value) {
	return value < 0 ? -value : value;
}

static int imax(int a, int b) {
	return a > b ? a : b;
}

static uint16 roundedIntegerSquareRoot(uint32 value) {
	if (value <= 1)
		return (uint16)value;

	uint32 remainingBits = value;
	uint32 remainder = 0;
	uint16 result = 0;
	for (int bitPair = 0; bitPair < 16; ++bitPair) {
		result += result;
		remainder = (remainder << 2) | ((remainingBits >> 30) & 3);
		remainingBits <<= 2;
		const uint16 trial = result + result + 1;
		if (remainder >= trial) {
			++result;
			remainder -= trial;
		}
	}

	// Hopkins adds this rounding step after ACK's 16-pass integer root.
	// Preserve the executable's comparison rather than substituting a
	// floating-point or conventional nearest-integer square root.
	const uint32 squareRemainder = value - (uint32)result * result;
	if (squareRemainder >= (uint32)(result - 1))
		++result;
	return result;
}

} // End of anonymous namespace

BaseEngine::BaseEngine(const BaseData &data) :
		_data(data), _entry(nullptr), _playerX(0), _playerY(0), _playerAngle(0),
		_health(1000), _weaponCounter(0), _texturesEnabled(true), _lastObjectHit(0),
		_turnRamp(0), _pendingTurn(0) {
	Common::fill(_xGrid, _xGrid + ARRAYSIZE(_xGrid), 0);
	Common::fill(_yGrid, _yGrid + ARRAYSIZE(_yGrid), 0);
}

void BaseEngine::initialize(const BaseEntryPoint &entry) {
	_entry = &entry;
	_playerX = entry.playerX;
	_playerY = entry.playerY;
	_playerAngle = normalizeBaseAngle(entry.playerAngle);
	_health = 1000;
	_weaponCounter = 0;
	_texturesEnabled = true;
	_lastObjectHit = 0;
	_turnRamp = 0;
	_pendingTurn = 0;
	_soundEvents.clear();

	for (int i = 0; i < kBaseMaxDoors; ++i)
		_doors[i] = BaseDoor();

	buildGrid();
	createObjects();
}

void BaseEngine::buildGrid() {
	Common::fill(_xGrid, _xGrid + ARRAYSIZE(_xGrid), 0);
	Common::fill(_yGrid, _yGrid + ARRAYSIZE(_yGrid), 0);

	for (int y = 0; y < kBaseMapHeight; ++y) {
		for (int x = 0; x < kBaseMapWidth; ++x) {
			const int mapPos = y * kBaseMapWidth + x;
			const uint16 code = _data.mapCodeAt(mapPos);
			const byte lowCode = code & 0xff;

			if (code == 0xfc) {
				_playerX = x * kBaseCellSize + kBaseCellSize / 2;
				_playerY = y * kBaseCellSize + kBaseCellSize / 2;
				continue;
			}
			if (code == 0xfd || code == 0xfe || code == 0xff)
				continue;
			if (!code)
				continue;

			// This is the original ACK BuildAckGrid algorithm. A map square
			// contributes its right/bottom edge and normally its left/top edge.
			// Doors replace the perpendicular pair with the jamb texture (59).
			if (lowCode == kBaseDoorYCode) {
				_xGrid[mapPos] = kBaseDoorSideCode;
				_xGrid[mapPos + 1] = kBaseDoorSideCode;
			} else {
				if (_xGrid[mapPos] != kBaseDoorSideCode)
					_xGrid[mapPos] = code;
				_xGrid[mapPos + 1] = code;
			}

			if (lowCode == kBaseDoorXCode) {
				_yGrid[mapPos] = kBaseDoorSideCode;
				_yGrid[mapPos + kBaseMapWidth] = kBaseDoorSideCode;
			} else {
				if (_yGrid[mapPos] != kBaseDoorSideCode)
					_yGrid[mapPos] = code;
				_yGrid[mapPos + kBaseMapWidth] = code;
			}
		}
	}
}

void BaseEngine::createObjects() {
	for (int i = 0; i <= kBaseMaxObjects; ++i)
		_objects[i] = BaseObject();

	for (int mapPos = 0; mapPos < kBaseMapCellCount; ++mapPos) {
		const int id = _data.objectCodeAt(mapPos) & 0x7f;
		if (id < 1 || id > kBaseMaxObjects)
			continue;

		BaseObject &object = _objects[id];
		object.active = true;
		object.passable = false;
		object.id = id;
		object.x = (mapPos & 63) * kBaseCellSize + kBaseCellSize / 2;
		object.y = (mapPos >> 6) * kBaseCellSize + kBaseCellSize / 2;
		object.mapPos = mapPos;
		object.bitmap = 1;
		object.mode = kBaseObjectChase;
		object.direction = 0;
		object.timer = 0;
		object.animationTick = 0;
		object.oldX = object.x;
		object.oldY = object.y;
	}
}

int BaseEngine::tick(BaseInputState &input) {
	_soundEvents.clear();

	if (input.toggleTextures) {
		_texturesEnabled = !_texturesEnabled;
		input.toggleTextures = false;
	}

	updateTurn(input);
	updatePlayer(input);
	updateWeapon(input);

	int exitResult = -1;
	if (input.exitRequested) {
		exitResult = tryExit();
		input.exitRequested = false;
		if (exitResult >= 94 && exitResult <= 99)
			_soundEvents.push_back(kBaseSoundDoorOrExit);
	}

	updateGuards();

	if (checkDoorOpen(_playerX, _playerY, _playerAngle) != 0)
		_soundEvents.push_back(kBaseSoundDoorOrExit);
	updateDoors();

	if (_weaponCounter > 0)
		--_weaponCounter;

	if (_health < 10) {
		_health = 1;
		return kDeathExit;
	}

	return exitResult;
}

void BaseEngine::updateTurn(const BaseInputState &input) {
	if (_turnRamp != 0) {
		_turnRamp >>= 1;
		_playerAngle = normalizeBaseAngle(_playerAngle + _pendingTurn);
	}

	if (input.turnRight) {
		++_turnRamp;
		_pendingTurn = _turnRamp * 20;
	}
	if (input.turnLeft) {
		++_turnRamp;
		_pendingTurn = -_turnRamp * 20;
	}
}

void BaseEngine::updatePlayer(const BaseInputState &input) {
	if (input.forward)
		movePlayer(_playerAngle, kPlayerMoveAmount);
	if (input.backward)
		movePlayer(normalizeBaseAngle(_playerAngle + kBaseHalfTurn), kPlayerMoveAmount);
}

void BaseEngine::updateWeapon(const BaseInputState &input) {
	if (input.fire && _weaponCounter == 0)
		fireWeapon();
}

uint16 BaseEngine::getWallX(int mapPos) const {
	if (mapPos < 0 || mapPos >= kBaseAckGridArray)
		return 1;
	const uint16 code = _xGrid[mapPos];
	return (code & kBaseWallPass) ? 0 : code;
}

uint16 BaseEngine::getWallY(int mapPos) const {
	if (mapPos < 0 || mapPos >= kBaseAckGridArray)
		return 1;
	const uint16 code = _yGrid[mapPos];
	return (code & kBaseWallPass) ? 0 : code;
}

int BaseEngine::checkObjectPosition(int16 x, int16 y, int ignoredObject) const {
	const int mapPos = baseWorldMapIndex(x, y);
	for (int id = 1; id <= kBaseMaxObjects; ++id) {
		const BaseObject &object = _objects[id];
		if (!object.active || object.passable || id == ignoredObject)
			continue;
		if (object.mapPos == mapPos)
			return id;
	}
	return 0;
}

int BaseEngine::moveWithAckCollision(int16 &x, int16 &y, int angle, int amount, int ignoredObject) const {
	angle = normalizeBaseAngle(angle);
	const int xp = x;
	const int yp = y;
	int x1 = xp + (int)(((int64)_data.cosQ16(angle) * amount) >> 16);
	int y1 = yp + (int)(((int64)_data.sinQ16(angle) * amount) >> 16);
	const int xLeft = xp & kBaseGridMask;
	const int yTop = yp & kBaseGridMask;
	const bool movingObject = ignoredObject != 0;
	const int xRight = xLeft + kBaseCellSize - (movingObject ? 1 : 0);
	const int yBottom = yTop + kBaseCellSize - (movingObject ? 1 : 0);
	const int mapPos = yTop + (xp >> 6);
	int result = kAckNothing;

	// ACK checks object-vs-object overlap before applying edge-grid collision.
	// Player movement performs the same check after wall sliding instead.
	if (movingObject && checkObjectPosition((int16)x1, (int16)y1, ignoredObject))
		return kAckObject;

	if (x1 < xp && getWallX(mapPos) && (x1 < xLeft || iabs(x1 - xLeft) < kAckCollisionDistance)) {
		x1 = xp;
		result = kAckSlideX;
	}
	if (x1 > xp && getWallX(mapPos + 1) && (x1 > xRight || iabs(xRight - x1) < kAckCollisionDistance)) {
		x1 = xp;
		result = kAckSlideX;
	}
	if (y1 < yp && getWallY(mapPos) && (y1 < yTop || iabs(y1 - yTop) < kAckCollisionDistance)) {
		y1 = yp;
		result = kAckSlideY;
	}
	if (y1 > yp && getWallY(mapPos + kBaseMapWidth) && (y1 > yBottom || iabs(yBottom - y1) < kAckCollisionDistance)) {
		y1 = yp;
		result = kAckSlideY;
	}

	if (!result) {
		uint16 wallX = 0;
		uint16 wallY = 0;
		if (y1 < yTop + 32) {
			if (x1 < xLeft + 32) {
				wallX = getWallX(mapPos - kBaseMapWidth);
				wallY = getWallY(mapPos - 1);
				if (wallX && y1 < yTop + 28 && x1 < xLeft + 28) {
					if (xp > xLeft + 27) { x1 = xp; result = kAckSlideX; }
					else { y1 = yp; result = kAckSlideY; }
				}
				if (wallY && x1 < xLeft + 28 && y1 < yTop + 28) {
					if (yp > yTop + 27) { y1 = yp; result = kAckSlideY; }
					else { x1 = xp; result = kAckSlideX; }
				}
			}
			if (x1 > xRight - 32 && !result) {
				wallX = getWallX(mapPos + 1 - kBaseMapWidth);
				wallY = getWallY(mapPos + 1);
				if (wallX && y1 < yTop + 28 && x1 > xRight - 28) {
					if (xp < xRight - 27) { x1 = xp; result = kAckSlideX; }
					else { y1 = yp; result = kAckSlideY; }
				}
				if (wallY && x1 > xRight - 28 && y1 < yTop + 28) {
					if (yp > yTop + 27) { y1 = yp; result = kAckSlideY; }
					else { x1 = xp; result = kAckSlideX; }
				}
			}
		}

		if (y1 > yTop + 32 && !result) {
			if (x1 < xLeft + 32) {
				wallX = getWallX(mapPos + kBaseMapWidth);
				wallY = getWallY(mapPos - 1 + kBaseMapWidth);
				if (wallX && y1 > yBottom - 28 && x1 < xLeft + 28) {
					if (xp > xLeft + 27) { x1 = xp; result = kAckSlideX; }
					else { y1 = yp; result = kAckSlideY; }
				}
				if (wallY && x1 < xLeft + 28 && y1 > yBottom - 28) {
					if (yp < yBottom - 27) { y1 = yp; result = kAckSlideY; }
					else { x1 = xp; result = kAckSlideX; }
				}
			}
			if (x1 > xRight - 32 && !result) {
				wallX = getWallX(mapPos + 1 + kBaseMapWidth);
				wallY = getWallY(mapPos + 1 + kBaseMapWidth);
				if (wallX && y1 > yBottom - 28 && x1 > xRight - 28) {
					if (xp < xRight - 27) { x1 = xp; result = kAckSlideX; }
					else { y1 = yp; result = kAckSlideY; }
				}
				if (wallY && x1 > xRight - 28 && y1 > yBottom - 28) {
					if (yp < yBottom - 27) { y1 = yp; result = kAckSlideY; }
					else { x1 = xp; result = kAckSlideX; }
				}
			}
		}
	}

	if (!movingObject && checkObjectPosition((int16)x1, (int16)y1, 0))
		return kAckObject;

	if (result == kAckSlideX && y1 == yp)
		result = kAckXWall;
	if (result == kAckSlideY && x1 == xp)
		result = kAckYWall;

	x = (int16)x1;
	y = (int16)y1;
	return result;
}

int BaseEngine::movePlayer(int angle, int amount) {
	int16 x = _playerX;
	int16 y = _playerY;
	const int result = moveWithAckCollision(x, y, angle, amount, 0);
	if (result != kAckObject) {
		_playerX = x;
		_playerY = y;
	}
	return result;
}

int BaseEngine::moveObject(int objectId, int angle, int amount) {
	if (objectId < 1 || objectId > kBaseMaxObjects || !_objects[objectId].active)
		return kAckNothing;
	BaseObject &object = _objects[objectId];
	int16 x = object.x;
	int16 y = object.y;
	const int result = moveWithAckCollision(x, y, angle, amount, objectId);
	if (result == kAckObject)
		return result;
	object.x = x;
	object.y = y;
	object.mapPos = baseWorldMapIndex(x, y);
	if (object.mapPos == baseWorldMapIndex(_playerX, _playerY))
		return kAckPlayer;
	return result;
}

int BaseEngine::findDoor(int mapPos) const {
	for (int i = 0; i < kBaseMaxDoors; ++i) {
		if (_doors[i].mPos == mapPos || _doors[i].mPos1 == mapPos)
			return i;
	}
	return -1;
}

int BaseEngine::findDoorSlot(int mapPos) const {
	const int existing = findDoor(mapPos);
	if (existing >= 0 && _doors[existing].offset)
		return -1;
	for (int i = 0; i < kBaseMaxDoors; ++i) {
		if (_doors[i].mPos == -1)
			return i;
	}
	return -1;
}

const BaseDoor *BaseEngine::doorForMapPosition(int mapPos) const {
	const int index = findDoor(mapPos);
	return index >= 0 ? &_doors[index] : nullptr;
}

BaseDoor *BaseEngine::doorForMapPosition(int mapPos) {
	const int index = findDoor(mapPos);
	return index >= 0 ? &_doors[index] : nullptr;
}

int BaseEngine::doorOffsetForMapPosition(int mapPos) const {
	const BaseDoor *door = doorForMapPosition(mapPos);
	return door ? door->offset : 0;
}

int BaseEngine::checkDoorOpen(int16 x, int16 y, int angle) {
	angle = normalizeBaseAngle(angle);
	if (angle == 240 || angle == 720 || angle == 1200 || angle == 1680)
		angle = normalizeBaseAngle(angle + 1);

	const BaseRayHit hit = castRayFrom(x, y, angle, kBaseViewHalfWidth);
	if (!hit.hit)
		return 0;
	int checkDistance = 56;
	if (hit.code & (kBaseDoorSlide | kBaseDoorSplit))
		checkDistance += 64;
	if (hit.distance > checkDistance)
		return 0;

	const byte lowCode = hit.code & 0xff;
	if (!(hit.code & (kBaseDoorSlide | kBaseDoorSplit)))
		return 0;
	if (lowCode != kBaseDoorXCode && lowCode != kBaseDoorYCode)
		return 0;

	const int slot = findDoorSlot(hit.mapPos);
	if (slot < 0)
		return 0;
	if (hit.code & kBaseDoorLocked)
		return 0x80 | (hit.axis == kBaseRayX ? 1 : 2);

	BaseDoor &door = _doors[slot];
	door.mPos = hit.mapPos;
	if (hit.axis == kBaseRayX)
		door.mPos1 = (hit.worldX > x) ? hit.mapPos + 1 : hit.mapPos - 1;
	else
		door.mPos1 = (hit.worldY > y) ? hit.mapPos + kBaseMapWidth : hit.mapPos - kBaseMapWidth;
	door.mCode = hit.axis == kBaseRayX ? _xGrid[door.mPos] : _yGrid[door.mPos];
	door.mCode1 = hit.axis == kBaseRayX ? _xGrid[door.mPos1] : _yGrid[door.mPos1];
	door.offset = 1;
	door.speed = kDoorSpeed;
	door.type = hit.axis == kBaseRayX ? kBaseDoorXCode : kBaseDoorYCode;
	door.flags = kBaseDoorOpening;
	return hit.axis == kBaseRayX ? 1 : 2;
}

void BaseEngine::updateDoors() {
	checkDoors();
}

void BaseEngine::checkDoors() {
	const int playerMapPos = baseWorldMapIndex(_playerX, _playerY);
	for (int i = 0; i < kBaseMaxDoors; ++i) {
		BaseDoor &door = _doors[i];
		if (!door.offset)
			continue;

		door.offset += door.speed;
		const int openColumn = (door.mCode & kBaseDoorSplit) ? 31 : 63;

		// In ACKVIEW, the first ray through a fully open door leaves the two
		// wall-grid entries at zero. Doing it here makes that renderer side
		// effect deterministic and preserves collision behavior.
		if (door.speed > 0 && door.offset >= openColumn) {
			if (door.type == kBaseDoorXCode) {
				_xGrid[door.mPos] = 0;
				_xGrid[door.mPos1] = 0;
			} else {
				_yGrid[door.mPos] = 0;
				_yGrid[door.mPos1] = 0;
			}
		}

		if (door.speed < 1 && door.offset < 65) {
			if (playerMapPos == door.mPos || playerMapPos == door.mPos1) {
				door.offset -= door.speed;
				continue;
			}
			if (door.type == kBaseDoorXCode) {
				_xGrid[door.mPos] = door.mCode;
				_xGrid[door.mPos1] = door.mCode1;
			} else {
				_yGrid[door.mPos] = door.mCode;
				_yGrid[door.mPos1] = door.mCode1;
			}
			if (door.offset < 3)
				door = BaseDoor();
		}

		if (door.offset > kDoorMaximumOffset) {
			door.speed = -door.speed;
			door.flags &= ~kBaseDoorOpening;
			door.flags |= kBaseDoorClosing;
		}
	}
}

int BaseEngine::testGuardShotPosition(int16 x, int16 y) const {
	const int mapPos = baseWorldMapIndex(x, y);
	if (mapPos < 0 || mapPos >= kBaseMapCellCount)
		return kAckXWall;

	const byte lowCode = _data.mapCodeAt(mapPos) & 0xff;
	if (lowCode != 0) {
		if (lowCode != kBaseDoorXCode && lowCode != kBaseDoorYCode)
			return kAckXWall;

		const int doorIndex = findDoor(mapPos);
		if (doorIndex < 0 || _doors[doorIndex].offset < 30)
			return kAckXWall;
	}

	if (mapPos == baseWorldMapIndex(_playerX, _playerY))
		return kAckPlayer;
	return kAckNothing;
}

bool BaseEngine::guardCanShoot(int objectId) {
	BaseObject &object = _objects[objectId];
	const int xDistance = iabs(object.x - _playerX) + 1;
	const int yDistance = iabs(object.y - _playerY) + 1;
	int maximumDistance = imax(xDistance, yDistance);
	if (maximumDistance <= 0)
		return false;

	int32 xStep = (xDistance * 1000) / maximumDistance;
	int32 yStep = (yDistance * 1000) / maximumDistance;
	int64 fixedX = (int64)object.x * 1000;
	int64 fixedY = (int64)object.y * 1000;
	if (object.x > _playerX)
		xStep = -xStep;
	if (object.y > _playerY)
		yStep = -yStep;

	maximumDistance = MIN(maximumDistance, 600);
	const int sampleCount = maximumDistance / 20;
	int hitSample = -1;
	for (int sample = 0; sample <= sampleCount; ++sample) {
		fixedX += (int64)xStep * 20;
		fixedY += (int64)yStep * 20;
		const int16 testX = (int16)(fixedX / 1000);
		const int16 testY = (int16)(fixedY / 1000);
		const int result = testGuardShotPosition(testX, testY);
		if (result == kAckPlayer) {
			hitSample = sample;
			break;
		}
		if (result == kAckXWall)
			break;
	}

	if (hitSample < 0)
		return false;

	object.bitmap = 5;
	object.timer = 50;
	object.mode = kBaseObjectFiring;
	_soundEvents.push_back(kBaseSoundEnemyShot);

	const int damageDistance = hitSample * 15;
	if (damageDistance > 0 && damageDistance < 50)
		_health -= 40;
	else if (damageDistance > 50 && damageDistance < 150)
		_health -= 30;
	else if (damageDistance > 150 && damageDistance < 250)
		_health -= 20;
	else if (damageDistance > 250 && damageDistance < 400)
		_health -= 15;
	else if (damageDistance > 400)
		_health -= 5;
	return true;
}

void BaseEngine::updateGuards() {
	for (int id = 1; id <= kBaseMaxObjects; ++id) {
		if (_objects[id].active)
			updateGuard(_objects[id]);
	}
}

void BaseEngine::updateGuard(BaseObject &object) {
	int absX = iabs(_playerX - object.x);
	int absY = iabs(_playerY - object.y);
	if (absX >= 900 || absY >= 900)
		return;

	if (object.mode == kBaseObjectFiring) {
		--object.timer;
		if (object.timer < 20) {
			object.mode = kBaseObjectChase;
			object.timer = 10;
		}

		switch (object.timer) {
		case 50: case 46: case 42: case 38: case 34: case 30: case 26: case 22:
			object.bitmap = 6;
			break;
		case 48: case 44: case 40: case 36: case 32: case 28: case 24: case 20:
			object.bitmap = 5;
			break;
		default:
			break;
		}

		if (object.mode == kBaseObjectFiring)
			return;
	}

	if (object.mode == kBaseObjectDying) {
		if (object.bitmap < 16)
			++object.bitmap;
		if (object.bitmap >= 16) {
			object.bitmap = 16;
			object.passable = true;
			object.mode = kBaseObjectDead;
		}
		return;
	}
	if (object.mode == kBaseObjectDead)
		return;

	if (object.mode == kBaseObjectChase) {
		if (object.timer > 0)
			--object.timer;
		if (absX < 500 && absY < 500 && object.timer == 0)
			guardCanShoot(object.id);

		object.animationTick = 0;
		++object.bitmap;
		if (object.bitmap >= 5)
			object.bitmap = 1;

		int result = kAckNothing;
		if (absX < absY) {
			if (absX < 24) {
				object.x = _playerX;
				object.mapPos = baseWorldMapIndex(object.x, object.y);
				absX = 0;
			}
			if (absX > 24)
				result = moveObject(object.id, object.x < _playerX ? 0 : 915, kGuardMoveAmount);
			if (result != kAckNothing || absX < 24)
				result = moveObject(object.id, object.y < _playerY ? 440 : 1440, kGuardMoveAmount);
		} else {
			if (absY < 24) {
				object.y = _playerY;
				object.mapPos = baseWorldMapIndex(object.x, object.y);
				absY = 0;
			}
			if (absY > 24)
				result = moveObject(object.id, object.y < _playerY ? 440 : 1440, kGuardMoveAmount);
			if (result != kAckNothing || absY < 24)
				result = moveObject(object.id, object.x < _playerX ? 0 : 915, kGuardMoveAmount);
		}

		if (result != kAckNothing || (absX < 100 && absY < 100))
			object.mode = kBaseObjectFlee;
		if (object.mode != kBaseObjectFlee)
			return;
	}

	if (object.mode != kBaseObjectFlee)
		return;

	// A chase guard that switches to escape mode enters this block in the same
	// update in DEPLACE_GARDE, after its chase movement. Recompute the deltas
	// from that new position before choosing the escape axis.
	absX = iabs(_playerX - object.x);
	absY = iabs(_playerY - object.y);

	// The original escape state recomputes the dominant axis every update.
	object.animationTick = 0;
	++object.bitmap;
	if (object.bitmap <= 7 || object.bitmap >= 10)
		object.bitmap = 7;

	const int16 savedDirection = object.direction;
	const int16 oldX = object.x;
	const int16 oldY = object.y;
	int result = kAckNothing;
	if (absX < absY) {
		result = moveObject(object.id, object.x + 24 > _playerX ? 0 : 915, kGuardMoveAmount);
		if (result != kAckNothing)
			result = moveObject(object.id, object.y > _playerY ? 440 : 1440, kGuardMoveAmount);
	} else {
		result = moveObject(object.id, object.y + 24 > _playerY ? 440 : 1440, kGuardMoveAmount);
		if (result != kAckNothing)
			result = moveObject(object.id, object.x > _playerX ? 0 : 915, kGuardMoveAmount);
	}
	object.direction = savedDirection;
	if (result != kAckNothing)
		object.mode = kBaseObjectChase;
	object.oldX = oldX;
	object.oldY = oldY;

	if (absX < 600 && absY < 600 && (absX < 100 || absY < 100))
		guardCanShoot(object.id);
}

int BaseEngine::checkObjectPositionShot(int16 x, int16 y, int angle) {
	angle = normalizeBaseAngle(angle);
	const int mapPos = baseWorldMapIndex(x, y);
	const int32 cosine = _data.cosQ16(angle);
	const int32 sine = _data.sinQ16(angle);
	const int32 halfFovTangent = _data.longTanQ16(kBaseAngleHalfFov);
	int bestMetric = 0x2dc6c0;
	int bestObject = 0;

	for (int id = 1; id <= kBaseMaxObjects; ++id) {
		const BaseObject &object = _objects[id];
		if (!object.active || object.mode > kBaseObjectFlee || object.passable || object.mapPos != mapPos)
			continue;

		int deltaX = object.x - x;
		int deltaY = object.y - y;
		if (angle > kBaseHalfTurn && deltaY > 63)
			continue;
		if (angle < kBaseHalfTurn && deltaY < -63)
			continue;
		if ((angle > kBaseThreeQuarterTurn || angle < kBaseQuarterTurn) && deltaX < -63)
			continue;
		if (angle > kBaseQuarterTurn && angle < kBaseThreeQuarterTurn && deltaX > 63)
			continue;
		if ((angle == 0 || angle == kBaseHalfTurn) && deltaX == 0)
			continue;
		if ((angle == kBaseQuarterTurn || angle == kBaseThreeQuarterTurn) && deltaY == 0)
			continue;

		const int32 forward = (int32)(((int64)deltaX * cosine + (int64)deltaY * sine) >> 16);
		int32 lateral = (int32)(((int64)deltaY * cosine - (int64)deltaX * sine) >> 16);
		int32 boundary = (int32)(((int64)halfFovTangent * forward) >> 16);

		// This odd comparison is present in the Hopkins executable. With normal
		// positive world coordinates it negates both values for every candidate.
		if (deltaY < object.y) {
			boundary = -boundary;
			lateral = -lateral;
		}
		if (lateral + 32 < boundary)
			continue;

		deltaX = iabs(deltaX);
		deltaY = iabs(deltaY);
		const int metric = deltaX + deltaY - MIN(deltaX, deltaY) / 2;
		if (metric > kBaseMaximumDistance || metric >= bestMetric)
			continue;

		bestMetric = metric;
		bestObject = id;
	}

	_lastObjectHit = bestObject;
	return bestObject ? kAckObject : kAckNothing;
}

bool BaseEngine::testWall(int mapPos) const {
	if (mapPos < 0 || mapPos >= kBaseMapCellCount)
		return true;
	const uint16 code = _data.mapCodeAt(mapPos);
	const byte low = code & 0xff;
	return low > 0 && low < 60 && !(code & kBaseDoorSecret);
}

int BaseEngine::checkHitAt(int16 x, int16 y, int angle) const {
	angle = normalizeBaseAngle(angle);
	if (angle == 240 || angle == 720 || angle == 1200 || angle == 1680)
		angle = normalizeBaseAngle(angle + 1);

	// AckCheckHit casts and filters the two axes independently. In
	// particular, an upper/pass X wall must not hide a solid Y wall (or vice
	// versa) before the nearest surviving hit is selected.
	BaseRayHit xHit;
	BaseRayHit yHit;
	if (angle != kBaseQuarterTurn && angle != kBaseThreeQuarterTurn) {
		castXRay(x, y, angle, xHit);
		if (xHit.code & (kBaseWallUpper | kBaseWallPass))
			xHit = BaseRayHit();
	}
	if (angle != 0 && angle != kBaseHalfTurn) {
		castYRay(x, y, angle, yHit);
		if (yHit.code & (kBaseWallUpper | kBaseWallPass))
			yHit = BaseRayHit();
	}

	BaseRayHit hit;
	if (xHit.hit && (!yHit.hit || xHit.rawDistance <= yHit.rawDistance))
		hit = xHit;
	else if (yHit.hit)
		hit = yHit;
	if (!hit.hit)
		return kAckNothing;

	finalizeRayDistance(hit, kBaseViewHalfWidth);
	int collisionDistance = 48;
	if ((hit.code & 0xff) >= kBaseDoorXCode)
		collisionDistance += 64;
	if (hit.distance > collisionDistance)
		return kAckNothing;
	return hit.axis == kBaseRayY ? kAckYWall : kAckXWall;
}

int BaseEngine::moveShotStep(int16 &x, int16 &y, int angle, int amount) {
	angle = normalizeBaseAngle(angle);
	const int16 targetX = x + (int16)(((int64)_data.cosQ16(angle) * amount) >> 16);
	const int16 targetY = y + (int16)(((int64)_data.sinQ16(angle) * amount) >> 16);

	const int hit = checkHitAt(x, y, angle);
	if (hit != kAckNothing)
		return hit;

	if (checkObjectPositionShot(targetX, targetY, angle) != kAckNothing)
		return kAckObject;

	if (testWall(baseWorldMapIndex(targetX, targetY)))
		return kAckXWall;

	x = targetX;
	y = targetY;
	return kAckNothing;
}

void BaseEngine::fireWeapon() {
	_weaponCounter = kWeaponStartCounter;
	_soundEvents.push_back(kBaseSoundPlayerShot);

	int16 shotX = _playerX;
	int16 shotY = _playerY;
	_lastObjectHit = 0;
	for (int step = 0; step < 20; ++step) {
		const int result = moveShotStep(shotX, shotY, _playerAngle, 30);
		if (result == kAckXWall)
			break;
		if (result != kAckObject)
			continue;

		if (_lastObjectHit >= 1 && _lastObjectHit <= kBaseMaxObjects) {
			BaseObject &object = _objects[_lastObjectHit];
			_soundEvents.push_back(kBaseSoundGuardHit);
			object.animationTick = 0;
			object.bitmap = 11;
			object.mode = kBaseObjectDying;
		}
		break;
	}
}

int BaseEngine::tryExit() const {
	const int16 x = _playerX + (int16)(((int64)_data.cosQ16(_playerAngle) * 63) >> 16);
	const int16 y = _playerY + (int16)(((int64)_data.sinQ16(_playerAngle) * 63) >> 16);
	const int mapPos = baseWorldMapIndex(x, y);
	if ((_data.mapCodeAt(mapPos) & 0xff) != 15)
		return -1;

	switch (mapPos) {
	case 0x0860: return 94;
	case 0x087a: return 95;
	case 0x01df: return 96;
	case 0x05ab: return 97;
	case 0x0bab: return 98;
	case 0x01fb: return 99;
	default: return -1;
	}
}

int BaseEngine::castXRay(int16 originX, int16 originY, int angle, BaseRayHit &hit) const {
	int32 yNext = _data.yNextQ16(angle);
	const int xBegin = originX & kBaseGridMask;
	int32 xPos;
	int32 xNext;
	if (angle > 1440 || angle < 480) {
		xPos = xBegin + kBaseCellSize;
		xNext = kBaseCellSize;
	} else {
		xPos = xBegin;
		xNext = -kBaseCellSize;
		yNext = -yNext;
	}
	int64 yPos = (int64)(xPos - originX) * _data.longTanQ16(angle) + ((int64)originY << 16);
	int ignoredDoor = -1;

	while (xPos >= 0 && xPos <= kBaseWorldExtent && yPos >= 0 && yPos <= ((int64)kBaseWorldExtent << 16)) {
		const int mapPos = (int)((yPos >> 16) & kBaseGridMask) + (xPos >> 6);
		uint16 code = (mapPos >= 0 && mapPos < kBaseAckGridArray) ? _xGrid[mapPos] : 0;
		if (ignoredDoor >= 0 && (_doors[ignoredDoor].mPos == mapPos || _doors[ignoredDoor].mPos1 == mapPos))
			code = 0;
		if (code) {
			int32 hitX = xPos;
			int64 hitY = yPos;
			if ((code & 0xff) == kBaseDoorXCode) {
				const int side = (int)((yPos >> 16) & kBaseGridMask);
				const int doorIntercept = (int)((yPos + (yNext >> 1)) >> 16);
				if (doorIntercept < side || doorIntercept > side + kBaseCellSize) {
					xPos += xNext;
					yPos += yNext;
					continue;
				}
				hitY = yPos + (yNext >> 1);
				hitX += xNext >> 1;
			}

			int column = (int)((hitY >> 16) & 63);
			if (hitX < originX)
				column = 63 - column;
			const int doorIndex = findDoor(mapPos);
			if (doorIndex >= 0 && (code & (kBaseDoorSlide | kBaseDoorSplit))) {
				int offset = _doors[doorIndex].offset;
				bool gap = false;
				if (code & kBaseDoorSlide) {
					if (hitX > originX)
						offset = -offset;
					column += offset;
					gap = column < 0 || column > 63;
				} else {
					if (column < 32) {
						column += offset;
						gap = column > 31;
					} else {
						column -= offset;
						gap = column < 32;
					}
				}
				if (gap) {
					ignoredDoor = doorIndex;
					xPos += xNext;
					yPos += yNext;
					continue;
				}
			}

			hit.hit = true;
			hit.axis = kBaseRayX;
			hit.code = code;
			hit.mapPos = mapPos;
			hit.textureColumn = column & 63;
			hit.worldX = hitX;
			hit.worldY = (int32)(hitY >> 16);
			int64 raw = ((int64)(hitX - originX) * _data.invCos(angle)) >> 10;
			if (raw < 0)
				raw = -raw;
			hit.rawDistance = (int32)MIN(raw, (int64)0x7fffffff);
			return code;
		}
		xPos += xNext;
		yPos += yNext;
	}
	return 0;
}

int BaseEngine::castYRay(int16 originX, int16 originY, int angle, BaseRayHit &hit) const {
	int32 xNext = _data.xNextQ16(angle);
	const int yBegin = originY & kBaseGridMask;
	int32 yPos;
	int32 yNext;
	if (angle < 960) {
		yPos = yBegin + kBaseCellSize;
		yNext = kBaseCellSize;
	} else {
		yPos = yBegin;
		yNext = -kBaseCellSize;
		xNext = -xNext;
	}
	int64 xPos = (int64)(yPos - originY) * _data.longInvTanQ16(angle) + ((int64)originX << 16);
	int ignoredDoor = -1;

	while (xPos >= 0 && xPos <= ((int64)kBaseWorldExtent << 16) && yPos >= 0 && yPos <= kBaseWorldExtent) {
		const int mapPos = (yPos & kBaseGridMask) + (int)(xPos >> 22);
		uint16 code = (mapPos >= 0 && mapPos < kBaseAckGridArray) ? _yGrid[mapPos] : 0;
		if (ignoredDoor >= 0 && (_doors[ignoredDoor].mPos == mapPos || _doors[ignoredDoor].mPos1 == mapPos))
			code = 0;
		if (code) {
			int64 hitX = xPos;
			int32 hitY = yPos;
			if ((code & 0xff) == kBaseDoorYCode) {
				const int side = (int)((xPos >> 16) & kBaseGridMask);
				const int doorIntercept = (int)((xPos + (xNext >> 1)) >> 16);
				if (doorIntercept < side || doorIntercept > side + kBaseCellSize) {
					xPos += xNext;
					yPos += yNext;
					continue;
				}
				hitX = xPos + (xNext >> 1);
				hitY += yNext >> 1;
			}

			int column = (int)((hitX >> 16) & 63);
			if (hitY > originY)
				column = 63 - column;
			const int doorIndex = findDoor(mapPos);
			if (doorIndex >= 0 && (code & (kBaseDoorSlide | kBaseDoorSplit))) {
				int offset = _doors[doorIndex].offset;
				bool gap = false;
				if (code & kBaseDoorSlide) {
					if (hitY < originY)
						offset = -offset;
					column += offset;
					gap = column < 0 || column > 63;
				} else {
					if (column < 32) {
						column += offset;
						gap = column > 31;
					} else {
						column -= offset;
						gap = column < 32;
					}
				}
				if (gap) {
					ignoredDoor = doorIndex;
					xPos += xNext;
					yPos += yNext;
					continue;
				}
			}

			hit.hit = true;
			hit.axis = kBaseRayY;
			hit.code = code;
			hit.mapPos = mapPos;
			hit.textureColumn = column & 63;
			hit.worldX = (int32)(hitX >> 16);
			hit.worldY = hitY;
			int64 raw = ((int64)(hitY - originY) * _data.invSin(angle)) >> 8;
			if (raw < 0)
				raw = -raw;
			hit.rawDistance = (int32)MIN(raw, (int64)0x7fffffff);
			return code;
		}
		xPos += xNext;
		yPos += yNext;
	}
	return 0;
}

void BaseEngine::finalizeRayDistance(BaseRayHit &hit, int viewColumn) const {
	if (!hit.hit)
		return;
	viewColumn = CLIP(viewColumn, 0, kBaseFrameWidth - 1);
	int64 value = (int64)hit.rawDistance * (_data.viewCosQ16(viewColumn) >> 3);
	int64 rounded = value >> 12;
	if (value - (rounded << 12) >= 2048)
		++rounded;
	int64 distance = rounded >> 5;
	if (rounded - (distance << 5) >= 16)
		++distance;
	if (distance < 1)
		distance = 1;
	if (distance >= kBaseMaximumDistance)
		distance = kBaseMaximumDistance - 1;
	hit.distance = (int32)distance;
}

BaseRayHit BaseEngine::castRayFrom(int16 originX, int16 originY, int angle, int viewColumn) const {
	angle = normalizeBaseAngle(angle);
	BaseRayHit xHit;
	BaseRayHit yHit;
	if (angle != 480 && angle != 1440)
		castXRay(originX, originY, angle, xHit);
	if (angle != 0 && angle != 960)
		castYRay(originX, originY, angle, yHit);

	BaseRayHit result;
	if (xHit.hit && (!yHit.hit || xHit.rawDistance <= yHit.rawDistance))
		result = xHit;
	else if (yHit.hit)
		result = yHit;
	finalizeRayDistance(result, viewColumn);
	return result;
}

BaseRayHit BaseEngine::castRay(int angle, int viewColumn) const {
	return castRayFrom(_playerX, _playerY, angle, viewColumn);
}

int BaseEngine::objectAngle(int32 deltaX, int32 deltaY) const {
	if (deltaX == 0 || deltaY == 0) {
		if (deltaX == 0)
			return deltaY < 0 ? kBaseThreeQuarterTurn : kBaseQuarterTurn;
		return deltaX < 0 ? kBaseHalfTurn : 0;
	}

	int quadrant = 0;
	if (deltaX < 0 && deltaY > 0)
		quadrant = kBaseHalfTurn;
	else if (deltaX < 0 && deltaY < 0)
		quadrant = kBaseThreeQuarterTurn;
	else if (deltaX > 0 && deltaY < 0)
		quadrant = kBaseAngleCount;

	const uint32 absoluteX = deltaX < 0 ? (uint32)-deltaX : (uint32)deltaX;
	const uint32 absoluteY = deltaY < 0 ? (uint32)-deltaY : (uint32)deltaY;
	const int32 ratio = (int32)(((uint64)absoluteY << 16) / absoluteX);

	// The original returns this near-vertical clamp before applying the
	// quadrant correction. Although unusual, it is observable ACK behavior.
	if (_data.longTanQ16(kBaseQuarterTurn - 1) <= ratio)
		return kBaseQuarterTurn - 1;

	int firstAngle = 0;
	const int lowerPivot = kBaseQuarterTurn / 2;
	const int upperPivot = kBaseQuarterTurn * 3 / 4;
	if (_data.longTanQ16(lowerPivot) < ratio)
		firstAngle = _data.longTanQ16(upperPivot) < ratio ? upperPivot : lowerPivot;

	int objectAngle = 0;
	for (int angle = firstAngle; angle < kBaseQuarterTurn; ++angle) {
		if (_data.longTanQ16(angle) > ratio) {
			objectAngle = angle - 1;
			break;
		}
	}
	objectAngle = MAX(0, objectAngle);

	if (quadrant) {
		if (quadrant != kBaseThreeQuarterTurn)
			objectAngle = quadrant - objectAngle;
		else
			objectAngle += kBaseHalfTurn;
	}
	return objectAngle;
}

int BaseEngine::objectDistance(const BaseObject &object) const {
	const int dx = object.x - _playerX;
	const int dy = object.y - _playerY;
	return roundedIntegerSquareRoot((uint32)(dx * dx + dy * dy));
}

} // End of namespace Hopkins
