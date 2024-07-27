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

#include "scumm/he/intern_he.h"

#include "scumm/he/basketball/basketball.h"
#include "scumm/he/basketball/court.h"

namespace Scumm {

int CBBallCourt::getPlayerIndex(int playerID) {
	assert((FIRST_PLAYER <= playerID) && (playerID <= LAST_PLAYER));

	Common::Array<CCollisionPlayer> *playerList = getPlayerListPtr(playerID);
	for (size_t i = 0; i < playerList->size(); i++) {
		if ((*playerList)[i]._objectID == playerID) {
			return i;
		}
	}

	warning("CBBallCourt::getPlayerIndex(): Tried to find a player in the player list that was not there.");
	return 0;
}

CCollisionPlayer *CBBallCourt::getPlayerPtr(int playerID) {
	assert((FIRST_PLAYER <= playerID) && (playerID <= LAST_PLAYER));

	Common::Array<CCollisionPlayer> *playerList = getPlayerListPtr(playerID);

	size_t listSize = playerList->size();
	for (size_t i = 0; i < listSize; i++) {
		if ((*playerList)[i]._objectID == playerID) {
			return &((*playerList)[i]);
		}
	}

	warning("CBBallCourt::getPlayerPtr(): Tried to find a player in the player list that was not there.");
	return nullptr;
}

CCollisionBasketball *CBBallCourt::getBallPtr(int ballID) {
	CCollisionBasketball *sourceBall;

	if (ballID == _basketBall._objectID) {
		sourceBall = &_basketBall;
	} else if (ballID == _virtualBall._objectID) {
		sourceBall = &_virtualBall;
	} else {
		warning("CBBallCourt::getBallPtr(): Invalid ball ID passed to u32_userDetectBallCollision.");
		sourceBall = &_basketBall;
	}

	return sourceBall;
}

} // End of namespace Scumm
