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
#include "scumm/he/basketball/obstacle_avoidance.h"

#include "common/array.h"
#include "common/stack.h"
#include "common/queue.h"
#include "common/std/set.h"

namespace Scumm {

float Basketball::getAvoidanceDistance(const U32Circle &playerMarker, const CCollisionPlayer &obstacle) {
	// Figure out how close we want to buzz the obstacle...
	float passByDistance = (playerMarker.radius + obstacle.radius + OBSTACLE_AVOIDANCE_DISTANCE);

	// Figure out how close we are to the obstacle...
	float currentDistance = (playerMarker.center - obstacle.center).magnitude();

	// Use the smaller of the two...
	float avoidanceDistance = MIN(currentDistance, passByDistance);
	return avoidanceDistance;
}

CCollisionPlayer *Basketball::detectObstacle(const U32Circle &playerMarker,
								 int playerID,
								 const U32FltPoint2D &targetLocation,
								 bool targetIsObstacle,
								 U32FltPoint2D *intersection,
								 CBBallCourt *court) {
	// Create a ray whose origin is at the player's center, and whose direction
	// is the player's target vector...
	U32Ray2D poRay;
	poRay._origin = playerMarker.center;
	poRay._direction = (targetLocation - poRay._origin);

	// Cycle through all potential obstacles and see which obstacle is closest...
	CCollisionPlayer *finalObstacle = nullptr;
	float minDistance = (float)0x7FFFFFFF;

	for (int i = 0; i <= LAST_PLAYER; ++i) {
		// Get a pointer to the current obstacle...
		CCollisionPlayer *currentObstacle = court->getPlayerPtr(i);

		// Make sure we are not checking ourselves or a bench player...
		if ((currentObstacle->_objectID != playerID) &&
			(currentObstacle->_playerIsInGame)) {
			U32Circle obstacleMarker;
			obstacleMarker.center = currentObstacle->center;
			obstacleMarker.radius = getAvoidanceDistance(playerMarker, *currentObstacle);

			if (poRay.nearIntersection(obstacleMarker, intersection)) {
				float obstacleDist = (*intersection - playerMarker.center).magnitude();

				float alertDistance = MIN(AVOIDANCE_LOOK_AHEAD_DISTANCE, poRay._direction.magnitude());

				// See if this obstacle is in our way...
				if (obstacleDist < alertDistance) {
					// See if the target is within the obstacle...
					float otDist = (targetLocation - currentObstacle->center).magnitude();
					if ((otDist > obstacleMarker.radius) || targetIsObstacle) {
						// See if this obstacle is the closest one yet...
						if (obstacleDist < minDistance) {
							finalObstacle = currentObstacle;
							minDistance = obstacleDist;
						}
					}
				}
			}
		}
	}

	return finalObstacle;
}

bool Basketball::avoidObstacle(const U32Circle &playerMarker, const U32FltPoint2D &targetLocation, const CCollisionPlayer &obstacle, ERevDirection whichDirection, U32FltPoint2D *newTarget) {
	double sinTheta;
	double theta;

	// Get a vector from the player center to the obstacle center...
	U32FltVector2D poVector = obstacle.center - playerMarker.center;

	// Get a vector from the target center to the obstacle center...
	U32FltVector2D toVector = obstacle.center - targetLocation;

	// Figure out how close we want to buzz the obstacle...
	float avoidanceDistance = getAvoidanceDistance(playerMarker, obstacle);
	avoidanceDistance += AVOIDANCE_SAFETY_DISTANCE;

	// Figure out if we are avoinding the obstacle in the clockwise or
	// counter-clockwise direction...
	if (whichDirection == kNone) {
		whichDirection = getAvoidanceDirection(playerMarker, targetLocation, obstacle);
	}

	// Figure out how many radians of roatation away from the center of the
	// obstacle (using our center as the origin) we need to aim for in order to
	// perfectly buzz the obstacle...
	if (poVector.magnitude() == 0) {
		sinTheta = 0;
	} else {
		sinTheta = avoidanceDistance / poVector.magnitude();
		if ((sinTheta < -MAX_AVOIDANCE_ANGLE_SIN) || (MAX_AVOIDANCE_ANGLE_SIN < sinTheta)) {
			sinTheta = MAX_AVOIDANCE_ANGLE_SIN;
		}
	}

	theta = asin(sinTheta);

	// Create a vector that points in the direction we must now travel...
	poVector.rotate(whichDirection, theta);

	// This vector will take us as close as we want to go to the obstacle, but
	// we don't know how far in that direction we need to travel. To get to the,
	// target we need to buzz the obstacle once, continue on a little bit, then
	// turn and buzz the obstacle again. Right now we are going to calculate
	// that second buzz path...

	// We are caluclating from the target to the obstacle, so swap the direction...
	whichDirection = (whichDirection == kClockwise) ? kCounterClockwise : kClockwise;

	if (toVector.magnitude() == 0) {
		sinTheta = 0;
	} else {
		sinTheta = avoidanceDistance / toVector.magnitude();
		if (sinTheta < -MAX_AVOIDANCE_ANGLE_SIN) {
			sinTheta = -MAX_AVOIDANCE_ANGLE_SIN;
		} else if (sinTheta > MAX_AVOIDANCE_ANGLE_SIN) {
			sinTheta = MAX_AVOIDANCE_ANGLE_SIN;
		}
	}

	theta = asin(sinTheta);

	// Create a vector that points in the direction we must travel after our
	// initial pass-by of the target...
	toVector.rotate(whichDirection, theta);

	// Where the player and target vector's meet is where we should aim for...
	U32Ray2D poRay;
	poRay._origin = playerMarker.center;
	poRay._direction = poVector;

	U32Ray2D toRay;
	toRay._origin = targetLocation;
	toRay._direction = toVector;

	return poRay.intersection(toRay, newTarget);
}

ERevDirection Basketball::getAvoidanceDirection(const U32Circle &playerMarker, const U32FltPoint2D &targetLocation, const CCollisionPlayer &obstacle) {
	U32FltVector3D obstacleVector = obstacle.center - playerMarker.center;
	U32FltVector3D targetVector = targetLocation - playerMarker.center;
	U32FltVector3D crossVector = obstacleVector.cross(targetVector);

	return (crossVector.z > 0) ? kCounterClockwise : kClockwise;
}

bool Basketball::getPathDistance(U32Circle *playerMarker, int playerID, Common::Stack<U32FltPoint2D> *targetStack, ERevDirection lastTurn, float *pathDistance, Common::Queue<U32FltPoint2D> *wayPointQueue, Std::set<int> *obstacleSet, CBBallCourt *court) {
	U32FltPoint2D intersection;

	// See if we're going for the final target...
	bool shootingForFinalTarget = (targetStack->size() == 1);

	U32FltPoint2D &currentTarget = targetStack->top();

	// See if there is an obstacle between us and the target...
	CCollisionPlayer *obstacle = detectObstacle(*playerMarker, playerID, currentTarget, !shootingForFinalTarget,  &intersection, court);
	if (obstacle) {
		// Make sure we haven't run into this obstacle already...
		Std::set<int>::const_iterator obstacleIt = obstacleSet->find(obstacle->_objectID);
		if (obstacleIt != obstacleSet->end()) {
			if (targetStack->size() != 1) {
				targetStack->pop();
			}

			return false;
		}

		// Recursively call getPathDistance for both the left and right paths
		// around the current obstacle...
		ERevDirection turnDirection = getBestPath(*playerMarker,  playerID, targetStack,
												  obstacle, lastTurn, pathDistance,
												  wayPointQueue, obstacleSet, court);

		if (turnDirection == kNone) {
			if (targetStack->size() != 1) {
				targetStack->pop();
			}

			return false;
		} else {
			return true;
		}
	} else {
		// Get the distance between the player and the target...
		float targetDistance = (currentTarget - playerMarker->center).magnitude();

		// Add that distance to the current total for this path...
		*pathDistance += targetDistance;

		// Advance the player to the target...
		playerMarker->center = currentTarget;

		// See if this gets us to the final destination...
		if (shootingForFinalTarget) {
			return true;
		} else {
			wayPointQueue->push(currentTarget);
			targetStack->pop();

			// Keep on going for the final target...
			return getPathDistance(playerMarker, playerID, targetStack,
								   kNone, pathDistance, wayPointQueue,
								   obstacleSet, court);
		}
	}
}

void Basketball::pushTargetOutOfObstacle(const U32Circle &playerMarker, const CCollisionPlayer &obstacle, Common::Stack<U32FltPoint2D> *targetStack) {
	// Get the effective radius of the obstacle...
	float avoidanceDistance = getAvoidanceDistance(playerMarker, obstacle);

	// See if the target is within that radius...
	if ((targetStack->top() - obstacle.center).magnitude() < avoidanceDistance) {
		// Make sure this isn't the final target...
		if (targetStack->size() == 1) {
			warning("Basketball::pushTargetOutOfObstacle(): Should not be calling this function on the final target");
		}

		// Make a ray from the player to the target...
		U32Ray2D playerRay;
		playerRay._origin = playerMarker.center;
		playerRay._direction = targetStack->top() - playerMarker.center;

		// Make a circle around the obstacle...
		U32Circle obstacleCircle;
		obstacleCircle.center = obstacle.center;
		obstacleCircle.radius = avoidanceDistance;

		// Find the farthest intersection of the ray and the circle...
		U32FltPoint2D newTarget;
		if (playerRay.farIntersection(obstacleCircle, &newTarget)) {
			targetStack->pop();
			targetStack->push(newTarget);
		} else {
			warning("Basketball::pushTargetOutOfObstacle(): Unable to intersect the player ray with the obstacle circle");
		}
	}
}

ERevDirection Basketball::getBestPath(const U32Circle &playerMarker, int playerID, Common::Stack<U32FltPoint2D> *targetStack, CCollisionPlayer *obstacle,
						  ERevDirection lastTurn, float *distance, Common::Queue<U32FltPoint2D> *wayPointQueue, Std::set<int> *obstacleSet, CBBallCourt *court) {
	U32Circle tempLocation;

	float leftDistance = 0;
	float rightDistance = 0;

	bool leftPath = false;
	bool rightPath = false;

	Common::Queue<U32FltPoint2D> leftWayPointQueue;
	Common::Queue<U32FltPoint2D> rightWayPointQueue;

	// Add this obstacle to the obstacle set...
	obstacleSet->insert(obstacle->_objectID);

	// Clear out any unnecessary interim targets...
	while (targetStack->size() != 1) {
		targetStack->pop();
	}

	// Get the avoidance path to the left...
	if (kClockwise != lastTurn) {
		U32FltPoint2D leftTarget;

		if (avoidObstacle(playerMarker, targetStack->top(), *obstacle, kCounterClockwise, &leftTarget)) {
			tempLocation = playerMarker;

			targetStack->push(leftTarget);

			leftWayPointQueue = *wayPointQueue;

			leftPath = getPathDistance(&tempLocation, playerID, targetStack,
										kCounterClockwise, &leftDistance, &leftWayPointQueue,
										obstacleSet, court);
		}
	}

	// Get the avoidance path to the right...
	if (kCounterClockwise != lastTurn) {
		U32FltPoint2D rightTarget;

		if (avoidObstacle(playerMarker, targetStack->top(),
						  *obstacle, kClockwise, &rightTarget)) {
			tempLocation = playerMarker;

			targetStack->push(rightTarget);

			rightWayPointQueue = *wayPointQueue;

			rightPath = getPathDistance(&tempLocation, playerID, targetStack,
										 kClockwise, &rightDistance, &rightWayPointQueue,
										 obstacleSet, court);
		}
	}

	// See which path is better
	if (leftPath && rightPath) {
		*distance = MIN(leftDistance, rightDistance);
		if (*distance == leftDistance) {
			*wayPointQueue = leftWayPointQueue;
			return kCounterClockwise;
		} else {
			*wayPointQueue = rightWayPointQueue;
			return kClockwise;
		}
	} else if (leftPath && !rightPath) {
		*distance = leftDistance;
		*wayPointQueue = leftWayPointQueue;
		return kCounterClockwise;
	} else if (!leftPath && rightPath) {
		*distance = rightDistance;
		*wayPointQueue = rightWayPointQueue;
		return kClockwise;
	} else {
		*distance = 0;
		return kNone;
	}
}

int LogicHEBasketball::u32_userGetAvoidancePath(int playerID, const U32FltPoint2D &targetLocation, EAvoidanceType type) {
	U32FltPoint2D newTarget;
	U32FltPoint2D intersection;

	// Get a pointer to the source player...
	CCollisionPlayer *player = _vm->_basketball->_court->getPlayerPtr(playerID);

	// Make sure the player isn't already at its target...
	if (player->center == targetLocation) {
		newTarget = targetLocation;
	} else {
		// Extract the player's current location...
		U32Circle playerMarker;
		playerMarker.center = player->center;
		playerMarker.radius = player->radius;

		// See if an obstacle was found...
		CCollisionPlayer *obstacle = _vm->_basketball->detectObstacle(playerMarker, player->_objectID,
													 targetLocation, false, &intersection,
													 _vm->_basketball->_court);

		if (obstacle) {
			if (type == kMultipleObject) {
				Common::Queue<U32FltPoint2D> wayPointQueue;
				Common::Stack<U32FltPoint2D> targetStack;
				Std::set<int> obstacleSet;

				targetStack.push(targetLocation);

				float pathDistance;

				ERevDirection turnDirection = _vm->_basketball->getBestPath(playerMarker, player->_objectID, &targetStack,
														  obstacle, kNone, &pathDistance, &wayPointQueue,  &obstacleSet,
														  _vm->_basketball->_court);

				targetStack.pop();

				if (!targetStack.empty()) {
					warning("LogicHEBasketball::u32_userGetAvoidancePath(): It doesn't look like we calculated things out to the final target.");
				}

				if (wayPointQueue.empty()) {
					assert(turnDirection == kNone);

					// We were unable to find a valid path to the target,
					// so treat this like a single obstacle...
					if (!_vm->_basketball->avoidObstacle(playerMarker, targetLocation, *obstacle, kNone, &newTarget)) {
						warning("LogicHEBasketball::u32_userGetAvoidancePath(): Unable to go around the primary obstacle");
						newTarget = targetLocation;
					}
				} else {
					newTarget = wayPointQueue.front();
				}
			} else {
				if (!_vm->_basketball->avoidObstacle(playerMarker, targetLocation, *obstacle, kNone, &newTarget)) {
					warning("LogicHEBasketball::u32_userGetAvoidancePath(): Unable to go around the primary obstacle");
					newTarget = targetLocation;
				}
			}
		} else {
			newTarget = targetLocation;
		}
	}

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(newTarget.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(newTarget.y));

	return 1;
}

} // End of namespace Scumm
