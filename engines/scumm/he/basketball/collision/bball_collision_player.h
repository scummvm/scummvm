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

#ifndef SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_PLAYER_H
#define SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_PLAYER_H

#ifdef ENABLE_HE

#include "scumm/he/basketball/court.h"
#include "scumm/he/basketball/collision/bball_collision_cylinder.h"

namespace Scumm {

#define PLAYER_CATCH_HEIGHT 250

class CBBallCourt;

class CCollisionPlayer : public CCollisionCylinder {
public:
	CCollisionPlayer() : _playerHasBall(false),
						 _playerIsInGame(true),
						 _blockHeight(0),
						 _blockTime(0),
						 _catchHeight(0),
						 _maxBlockHeight(0) {}

	~CCollisionPlayer() {}

	void startBlocking(int blockHeight, int blockTime);
	void holdBlocking();
	void endBlocking();

	bool _playerHasBall;
	bool _playerIsInGame;

	int _catchHeight; // The extra height that a player gets for detecting the ball.

	bool testCatch(const ICollisionObject &targetObject, U32Distance3D *distance, CBBallCourt *court);

private:
	int _blockHeight = 0;    // The extra height that a player gets when blocking.
	int _maxBlockHeight = 0; // The max extra height that a player gets when blocking.
	int _blockTime = 0;      // The time it takes from the start of a block, to the apex of the block.
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_PLAYER_H
