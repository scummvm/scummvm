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

#ifndef SCUMM_HE_BASKETBALL_BASKETBALL_H
#define SCUMM_HE_BASKETBALL_BASKETBALL_H

#ifdef ENABLE_HE

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"
#include "scumm/he/basketball/court.h"
#include "scumm/he/basketball/collision/bball_collision_player.h"
#include "scumm/he/basketball/collision/bball_collision_shields.h"

#include "common/array.h"
#include "common/stack.h"
#include "common/queue.h"
#include "common/std/set.h"

namespace Scumm {

class LogicHEBasketball;

class Basketball {

public:
	Basketball(ScummEngine_v100he *vm);
	~Basketball();

	int u32FloatToInt(float input);
	int u32DoubleToInt(double input);

	int numOpponentsInCone(int team, float widthDistanceRatio, const U32FltVector2D &end, const U32FltVector2D &focus);
	float congestion(U32FltVector2D pos, bool ignore, int whichPlayer);
	void fillPlayerTargetList(const CCollisionPlayer *sourceObject, CCollisionObjectVector *targetList);
	void fillBallTargetList(const CCollisionSphere *sourceObject, CCollisionObjectVector *targetList);
	double getLaunchAngle(int velocity, int hDist, int vDist, int gravity);
	float getAvoidanceDistance(const U32Circle &playerMarker, const CCollisionPlayer &obstacle);
	CCollisionPlayer *detectObstacle(const U32Circle &playerMarker, int playerID, const U32FltPoint2D &targetLocation, bool targetIsObstacle, U32FltPoint2D *intersection, CBBallCourt *court);
	bool avoidObstacle(const U32Circle &playerMarker, const U32FltPoint2D &targetLocation,  const CCollisionPlayer &obstacle, ERevDirection whichDirection, U32FltPoint2D *newTarget);
	ERevDirection getAvoidanceDirection(const U32Circle &playerMarker, const U32FltPoint2D &targetLocation, const CCollisionPlayer &obstacle);
	bool getPathDistance(U32Circle *playerMarker, int playerID, Common::Stack<U32FltPoint2D> *targetStack, ERevDirection lastTurn, float *pathDistance, Common::Queue<U32FltPoint2D> *wayPointQueue, Std::set<int> *obstacleSet, CBBallCourt *court);

	void pushTargetOutOfObstacle(const U32Circle &playerMarker,
											 const CCollisionPlayer &obstacle,
											 Common::Stack<U32FltPoint2D> *targetStack);

	ERevDirection getBestPath(const U32Circle &playerMarker,
										  int playerID,
										  Common::Stack<U32FltPoint2D> *targetStack,
										  CCollisionPlayer *obstacle,
										  ERevDirection lastTurn,
										  float *distance,
										  Common::Queue<U32FltPoint2D> *wayPointQueue,
										  Std::set<int> *obstacleSet,
										  CBBallCourt *court);

	CBBallCourt *_court;
	CCollisionShieldVector *_shields;

private:
	ScummEngine_v100he *_vm;
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_BASKETBALL_H
