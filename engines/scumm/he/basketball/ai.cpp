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

#include "scumm/he/basketball/ai.h"
#include "scumm/he/basketball/basketball.h"
#include "scumm/he/basketball/court.h"
#include "scumm/he/basketball/geo_translations.h"
#include "scumm/he/logic_he.h"

#include "math/utils.h"

namespace Scumm {

int LogicHEBasketball::u32_userGetPlayerClosestToBall(int teamIndex) {
	assert((TEAM_HOME <= teamIndex) && (teamIndex <= TEAM_AWAY));

	int finalPlayerIndex = NO_PLAYER;
	int shortestDistance2 = 0x7FFFFFFF;

	// Go through all of the players on the team and calculate the distance between
	// them and the ball...
	Common::Array<CCollisionPlayer> *playerList =
		(teamIndex == TEAM_HOME) ?
		&_vm->_basketball->_court->_homePlayerList : &_vm->_basketball->_court->_awayPlayerList;

	for (size_t playerIndex = 0; playerIndex < playerList->size(); ++playerIndex) {
		CCollisionPlayer *currentPlayer = &((*playerList)[playerIndex]);

		if (currentPlayer->_playerIsInGame) {

			int distance2 = (
				((currentPlayer->center.x - _vm->_basketball->_court->_basketBall.center.x) *
				 (currentPlayer->center.x - _vm->_basketball->_court->_basketBall.center.x)) +
				((currentPlayer->center.y - _vm->_basketball->_court->_basketBall.center.y) *
				 (currentPlayer->center.y - _vm->_basketball->_court->_basketBall.center.y)));

			// Keep track of the player closest to the ball...
			if (distance2 < shortestDistance2) {
				finalPlayerIndex = playerIndex;
				shortestDistance2 = distance2;
			}
		}
	}

	assert(finalPlayerIndex != NO_PLAYER);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, (*playerList)[finalPlayerIndex]._objectID);

	return 1;
}

int LogicHEBasketball::u32_userGetPlayerClosestToBall() {
	int finalPlayerIndex = NO_PLAYER;
	int shortestDistance2 = 0x7FFFFFFF;
	Common::Array<CCollisionPlayer> *playerList = nullptr;

	// Go through all of the players on both teams and calculate the distance between
	// them and the ball...
	for (size_t playerIndex = 0; playerIndex < _vm->_basketball->_court->_homePlayerList.size(); ++playerIndex) {
		CCollisionPlayer *currentPlayer = &_vm->_basketball->_court->_homePlayerList[playerIndex];

		if (currentPlayer->_playerIsInGame) {
			int distance2 = (
				((currentPlayer->center.x - _vm->_basketball->_court->_basketBall.center.x) *
				 (currentPlayer->center.x - _vm->_basketball->_court->_basketBall.center.x)) +
				((currentPlayer->center.y - _vm->_basketball->_court->_basketBall.center.y) *
				 (currentPlayer->center.y - _vm->_basketball->_court->_basketBall.center.y)));

			// Keep track of the player closest to the ball...
			if (distance2 < shortestDistance2) {
				finalPlayerIndex = playerIndex;
				shortestDistance2 = distance2;
				playerList = &_vm->_basketball->_court->_homePlayerList;
			}
		}
	}

	for (size_t playerIndex = 0; playerIndex < _vm->_basketball->_court->_awayPlayerList.size(); ++playerIndex) {
		CCollisionPlayer *currentPlayer = &_vm->_basketball->_court->_awayPlayerList[playerIndex];

		if (currentPlayer->_playerIsInGame) {
			int distance2 = (
				((currentPlayer->center.x - _vm->_basketball->_court->_basketBall.center.x) *
				 (currentPlayer->center.x - _vm->_basketball->_court->_basketBall.center.x)) +
				((currentPlayer->center.y - _vm->_basketball->_court->_basketBall.center.y) *
				 (currentPlayer->center.y - _vm->_basketball->_court->_basketBall.center.y)));

			// Keep track of the player closest to the ball...
			if (distance2 < shortestDistance2) {
				finalPlayerIndex = playerIndex;
				shortestDistance2 = distance2;
				playerList = &_vm->_basketball->_court->_awayPlayerList;
			}
		}
	}

	assert(finalPlayerIndex != NO_PLAYER);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, (*playerList)[finalPlayerIndex]._objectID);

	return 1;
}

int Basketball::numOpponentsInCone(int team, float widthDistanceRatio, const U32FltVector2D &end, const U32FltVector2D &focus) {
	// The end out parameter is the end point of the center line of the cone,
	// and widthDistanceRatio defines the width of the cone = width at dist 1
	int count = 0;

	Line2D line(focus, end);

	Common::Array<CCollisionPlayer> *playerList = (team == TEAM_HOME) ? &_vm->_basketball->_court->_homePlayerList : &_vm->_basketball->_court->_awayPlayerList;

	for (Common::Array<CCollisionPlayer>::iterator currentPlayer = playerList->begin(); currentPlayer != playerList->end(); ++currentPlayer) {
		U32FltVector2D point = line.projectPoint(currentPlayer->center);

		if (focus.distance2(currentPlayer->center) < (point.distance2(focus) * widthDistanceRatio * widthDistanceRatio) && line.inBetween(point, focus, end)) {
			count++;
		}
	}

	return count;
}

static bool isPointInBounds(const U32FltVector2D &point) {
	return ((point.x > 0) &&
			(point.x < MAX_WORLD_X) &&
			(point.y > 0) &&
			(point.y < MAX_WORLD_Y));
}

float Basketball::congestion(U32FltVector2D pos, bool ignore, int whichPlayer) {
	float congestion = 0.0F;

	for (int team = TEAM_HOME; team <= TEAM_AWAY; ++team) {
		Common::Array<CCollisionPlayer> *playerList = (team == TEAM_HOME) ? &_court->_homePlayerList : &_court->_awayPlayerList;

		for (Common::Array<CCollisionPlayer>::iterator currentPlayer = playerList->begin(); currentPlayer != playerList->end(); ++currentPlayer) {
			if (currentPlayer->_playerIsInGame && (ignore && !(currentPlayer->_objectID == whichPlayer))) {
				float distance2 = pos.distance2(currentPlayer->center);

				if (distance2 == 0.0F)
					return FLT_MAX;

				congestion += (1 / distance2);
			}
		}
	}

	return congestion;
}

int LogicHEBasketball::u32_userGetOpenSpot(int whichPlayer, U32FltVector2D upperLeft, U32FltVector2D lowerRight, U32FltVector2D passer, bool attract, U32FltVector2D attractPoint) {
	int xGranularity = 5;
	int yGranularity = 5;

	int rectWidth = fabs(upperLeft.x - lowerRight.x);
	int rectHeight = fabs(upperLeft.y - lowerRight.y);

	// Integer operation cast to float, this is intended!
	float xMesh = (float)(rectWidth / (xGranularity + 1));
	float yMesh = (float)(rectHeight / (yGranularity + 1));

	float startX = upperLeft.x + xMesh / 2;
	float startY = upperLeft.y + yMesh / 2;

	float x = startX, y = startY;

	float bestCongestion = FLT_MAX;

	U32FltVector2D bestPoint, point;

	float tmp;

	for (int ii = 0; ii < xGranularity; ii++) {
		for (int jj = 0; jj < yGranularity; jj++) {
			tmp = _vm->_basketball->congestion(point = U32FltVector2D(x, y), true, whichPlayer);

			if (attract) {
				tmp -= (4 * (1 / point.distance2(attractPoint)));
			}

			if (tmp < bestCongestion &&
				isPointInBounds(point)) {
				bestCongestion = tmp;
				bestPoint = point;
			}

			y += yMesh;
		}

		x += xMesh;
		y = startY;
	}

	if (bestCongestion == FLT_MAX) {
		return 0;
	} else {
		writeScummVar(_vm1->VAR_U32_USER_VAR_A, bestPoint.x);
		writeScummVar(_vm1->VAR_U32_USER_VAR_B, bestPoint.y);

		return 1;
	}
}

static int isPlayerInBounds(U32Sphere *player) {
	assert(player);

	return (((player->center.x - player->radius) > -COLLISION_EPSILON) &&
			((player->center.x + player->radius) < (MAX_WORLD_X + COLLISION_EPSILON)) &&
			((player->center.y - player->radius) > -COLLISION_EPSILON) &&
			((player->center.y + player->radius) < (MAX_WORLD_Y + COLLISION_EPSILON)));
}

int LogicHEBasketball::u32_userIsPlayerInBounds(int playerID) {
	U32Cylinder *player = _vm->_basketball->_court->getPlayerPtr(playerID);

	int isPlayerInBoundsRes = isPlayerInBounds(player);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, isPlayerInBoundsRes);

	return 1;
}

int LogicHEBasketball::u32_userIsBallInBounds() {
	int isBallInBounds = isPlayerInBounds(&_vm->_basketball->_court->_basketBall);
	writeScummVar(_vm1->VAR_U32_USER_VAR_A, isBallInBounds);
	return 1;
}

} // End of namespace Scumm
