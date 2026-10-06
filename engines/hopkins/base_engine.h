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

// Source-backed Hopkins WBASE simulation state.

#ifndef HOPKINS_BASE_ENGINE_H
#define HOPKINS_BASE_ENGINE_H

#include "hopkins/base_data.h"
#include "hopkins/base_types.h"

#include "common/array.h"
#include "common/scummsys.h"

namespace Hopkins {

struct BaseInputState {
	bool forward;
	bool backward;
	bool turnLeft;
	bool turnRight;
	bool fire;
	bool exitRequested;
	bool toggleTextures;

	BaseInputState() : forward(false), backward(false), turnLeft(false),
		turnRight(false), fire(false), exitRequested(false), toggleTextures(false) {}
};

enum BaseSoundEvent {
	kBaseSoundEnemyShot = 2,
	kBaseSoundPlayerShot = 3,
	kBaseSoundDoorOrExit = 4,
	kBaseSoundGuardHit = 5
};

class BaseEngine {
public:
	explicit BaseEngine(const BaseData &data);

	void initialize(const BaseEntryPoint &entry);

	/** Run one original 24 Hz WBASE simulation step. Returns -1 while running. */
	int tick(BaseInputState &input);

	const BaseData &data() const { return _data; }
	const uint16 *xGrid() const { return _xGrid; }
	const uint16 *yGrid() const { return _yGrid; }
	const BaseObject &object(int id) const { return _objects[id]; }
	const BaseDoor &door(int id) const { return _doors[id]; }
	const Common::Array<int> &soundEvents() const { return _soundEvents; }
	void clearSoundEvents() { _soundEvents.clear(); }

	int playerX() const { return _playerX; }
	int playerY() const { return _playerY; }
	int playerAngle() const { return _playerAngle; }
	int health() const { return _health; }
	int weaponCounter() const { return _weaponCounter; }
	bool texturesEnabled() const { return _texturesEnabled; }
	int entryReturnId() const { return _entry ? _entry->returnId : 0; }

	BaseRayHit castRay(int angle, int viewColumn) const;
	BaseRayHit castRayFrom(int16 originX, int16 originY, int angle, int viewColumn) const;
	int objectAngle(int32 deltaX, int32 deltaY) const;
	int objectDistance(const BaseObject &object) const;

private:
	void buildGrid();
	void createObjects();
	void updateTurn(const BaseInputState &input);
	void updatePlayer(const BaseInputState &input);
	void updateWeapon(const BaseInputState &input);
	void updateDoors();
	void updateGuards();
	void updateGuard(BaseObject &object);

	uint16 getWallX(int mapPos) const;
	uint16 getWallY(int mapPos) const;
	int moveWithAckCollision(int16 &x, int16 &y, int angle, int amount, int ignoredObject) const;
	int movePlayer(int angle, int amount);
	int moveObject(int objectId, int angle, int amount);
	int checkObjectPosition(int16 x, int16 y, int ignoredObject) const;
	int checkObjectPositionShot(int16 x, int16 y, int angle);
	int checkHitAt(int16 x, int16 y, int angle) const;
	int moveShotStep(int16 &x, int16 &y, int angle, int amount);
	int testGuardShotPosition(int16 x, int16 y) const;
	bool testWall(int mapPos) const;

	int checkDoorOpen(int16 x, int16 y, int angle);
	void checkDoors();
	int findDoor(int mapPos) const;
	int findDoorSlot(int mapPos) const;
	const BaseDoor *doorForMapPosition(int mapPos) const;
	BaseDoor *doorForMapPosition(int mapPos);
	int doorOffsetForMapPosition(int mapPos) const;

	bool guardCanShoot(int objectId);
	void fireWeapon();
	int tryExit() const;

	int castXRay(int16 originX, int16 originY, int angle, BaseRayHit &hit) const;
	int castYRay(int16 originX, int16 originY, int angle, BaseRayHit &hit) const;
	void finalizeRayDistance(BaseRayHit &hit, int viewColumn) const;

	const BaseData &_data;
	const BaseEntryPoint *_entry;
	uint16 _xGrid[kBaseAckGridArray];
	uint16 _yGrid[kBaseAckGridArray];
	BaseObject _objects[kBaseMaxObjects + 1];
	BaseDoor _doors[kBaseMaxDoors];

	int16 _playerX;
	int16 _playerY;
	int16 _playerAngle;
	int _health;
	int _weaponCounter;
	bool _texturesEnabled;
	int _lastObjectHit;

	// Original keyboard inertia: apply the previous delta, halve the ramp,
	// then build the next pending turn from the currently held key.
	int _turnRamp;
	int _pendingTurn;

	Common::Array<int> _soundEvents;
};

} // End of namespace Hopkins

#endif // HOPKINS_BASE_ENGINE_H
