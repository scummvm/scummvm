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
#include "scumm/he/basketball/court.h"
#include "scumm/he/basketball/passing.h"

namespace Scumm {

static float getBallImpactTime(CCollisionSphere *ball, int gravity, int height) {
	// Solve the equation:
	// 
	// z + vz * t - 0.5 * g * t * t = height
	// 
	// to find out how long before the ball hits the ground...
	float a = -.5 * gravity;
	float b = ball->_velocity.z;
	float c = ball->center.z - ball->radius - height;

	double tFinal;
	if (((b * b) < (4 * a * c)) || (a == 0)) {
		tFinal = 0;
	} else {
		// See how long before the ball hits the ground...
		tFinal = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
		tFinal = MAX(0.0, tFinal);
	}

	return tFinal;
}

int LogicHEBasketball::u32_userComputeAngleOfPass(int velocity, int hDist, int vDist, int gravity) {
	assert(hDist > 0);

	double theta = _vm->_basketball->getLaunchAngle(velocity, hDist, vDist, gravity);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32DoubleToInt(theta));

	return 1;
}

int LogicHEBasketball::u32_userComputeAngleOfBouncePass(int velocity, int hDist, int currentZ, int destZ, int gravity) {
	double theta;
	int vDist;

	assert(hDist > 0);

	// Aim the ball at the floor 2/3 the distance between you and your target...
	hDist = (hDist * 2) / 3;
	vDist = 0 - currentZ;
	theta = _vm->_basketball->getLaunchAngle(velocity, hDist, vDist, gravity);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32DoubleToInt(theta));

	return 1;
}

int LogicHEBasketball::u32_userHitMovingTarget(U32FltPoint2D sourcePlayer, U32FltPoint2D targetPlayer, U32FltVector2D targetVelocity, int passSpeed) {
	// Calculate the xy speed of the pass. To do this, we need to estimate
	// the elevation angle of the pass. This is because the actual angle
	// will be calculated based on the target location, which is what we
	// are currently getting...
	float xyPassSpeed = passSpeed * cos((PASS_ANGLE * BBALL_M_PI) / 180);

	// The distance between the target player at future time t and and the
	// passing player is equal to the pass speed times t. Solve for t...
	float a = (targetVelocity.x * targetVelocity.x) +
			  (targetVelocity.y * targetVelocity.y) -
			  (xyPassSpeed * xyPassSpeed);

	float b = (2 * targetPlayer.x * targetVelocity.x) +
			  (2 * targetPlayer.y * targetVelocity.y) -
			  (2 * sourcePlayer.x * targetVelocity.x) -
			  (2 * sourcePlayer.y * targetVelocity.y);

	float c = (targetPlayer.x * targetPlayer.x) +
			  (targetPlayer.y * targetPlayer.y) +
			  (sourcePlayer.x * sourcePlayer.x) +
			  (sourcePlayer.y * sourcePlayer.y) -
			  (2 * targetPlayer.x * sourcePlayer.x) -
			  (2 * targetPlayer.y * sourcePlayer.y);

	// Now we have two answer candidates. We want the smallest of the two that is
	// greater than 0...
	double tFinal;
	if (((b * b) < (4 * a * c)) || (a == 0)) {
		tFinal = 0.0;
	} else {
		double t1 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
		double t2 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
		tFinal = MIN_GREATER_THAN_ZERO(t1, t2);
		tFinal -= BALL_LEAD_TIME; // Put the player at the target spot a few frames
								  // before the ball to give them a chance to get ready
								  // to catch it...
		tFinal = MAX(0.0, tFinal);
	}

	assert(tFinal < 50.0);

	U32FltPoint2D targetPoint;
	targetPoint.x = targetPlayer.x + (targetVelocity.x * tFinal);
	targetPoint.y = targetPlayer.y + (targetVelocity.y * tFinal);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(targetPoint.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(targetPoint.y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, _vm->_basketball->u32DoubleToInt(tFinal));

	return 1;
}

int LogicHEBasketball::u32_userGetPassTarget(int playerID, const U32FltVector3D &aimVector) {
	U32Ray3D passerRay;

	// Get our target candidates...
	Common::Array<CCollisionPlayer> *targetList = (_vm->_basketball->_court->getPlayerListPtr(playerID));

	// Get the passer...
	CCollisionPlayer *passer = _vm->_basketball->_court->getPlayerPtr(playerID);

	// Set up a ray from the passer, pointing in the direction of the aim vector...
	passerRay._origin = passer->center;
	passerRay._direction = aimVector.normalize() * MAX_PASSING_DIST;

	// Get the distance from the passers aim vector to each of the remaining target
	// candidates...
	int passTargetID = NO_PLAYER;
	float leastDist = MAX_PASSING_DIST;

	for (size_t i = 0; i < targetList->size(); ++i) {
		// Get the current target candidate...
		CCollisionPlayer *target = &(*targetList)[i];

		// Make sure the target is in the game...
		if (target->_playerIsInGame) {
			// Make sure that the passing player is not a target candidate...
			if (passer->_objectID != target->_objectID) {
				// Point-line distance formula.
				U32FltPoint3D *targetCenter = &target->center;

				float u = (((targetCenter->x - passerRay._origin.x) * passerRay._direction.x) +
						   ((targetCenter->y - passerRay._origin.y) * passerRay._direction.y)) /
						  (passerRay._direction.magnitude() * passerRay._direction.magnitude());

				if (u >= 0) {
					U32FltVector3D distance;
					distance.x = targetCenter->x - (passerRay._origin.x + (u * passerRay._direction.x));
					distance.y = targetCenter->y - (passerRay._origin.y + (u * passerRay._direction.y));
					float totalDist = distance.magnitude();
					if (totalDist < leastDist) {
						leastDist = totalDist;
						passTargetID = target->_objectID;
					}
				}
			}
		}
	}

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, passTargetID);

	return 1;
}

int LogicHEBasketball::u32_userDetectPassBlocker(int playerID, const U32FltPoint3D &targetPoint) {
	int blockerPresent = 0;

	// Get our blocker candidates....
	Common::Array<CCollisionPlayer> *targetList = (_vm->_basketball->_court->getOpponentListPtr(playerID));

	// Get the passer and catcher...
	CCollisionPlayer *passer = _vm->_basketball->_court->getPlayerPtr(playerID);

	// Set up a ray from the passer, pointing in the direction of the aim vector...
	U32FltVector2D passVector = targetPoint - passer->center;

	// Get the distance from the passers aim vector to each of the remaining target
	// candidates...
	for (size_t i = 0; i < targetList->size(); ++i) {
		CCollisionPlayer *blocker = &(*targetList)[i];

		if (blocker->_playerIsInGame) {
			// --- Ray -> Circle intersection test ---
			// Get a vector from the passer to the potential blocker...
			U32FltVector2D enemyVector = blocker->center - passer->center;

			// Project that vector onto the pass vector
			float enemyDistance = enemyVector.projectScalar(passVector);

			// Test to see if the blocker is behind the pass or behind the pass target...
			float maxBlockerDistance = passVector.magnitude();
			if ((0 > enemyDistance) || (enemyDistance > maxBlockerDistance)) {
				continue;
			}

			// Find the distance between the blocker and the passVector...
			float mSquared = (enemyVector.magnitude() * enemyVector.magnitude()) -
							 (enemyDistance * enemyDistance);

			// If that distance is less than the blocker's radius, we are done...
			if (mSquared <= ((blocker->radius + _vm->_basketball->_court->_basketBall.radius) * (blocker->radius + _vm->_basketball->_court->_basketBall.radius))) {
				blockerPresent = 1;
				break;
			}
		}
	}

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, blockerPresent);

	return 1;
}

int LogicHEBasketball::u32_userGetBallIntercept(int playerID, int ballID, int playerSpeed, int gravity) {
	double tFinal = 0.0;

	CCollisionSphere *ball = _vm->_basketball->_court->getBallPtr(ballID);
	CCollisionPlayer *player = _vm->_basketball->_court->getPlayerPtr(playerID);

	assert(!ball->_ignore);

	U32Circle playerMarker;
	playerMarker.center = player->center;
	playerMarker.radius = player->radius + ball->radius;

	U32Ray2D playerRay;
	playerRay._origin = player->center;
	playerRay._direction = player->_velocity;

	U32Ray2D ballRay;
	ballRay._origin = ball->center;
	ballRay._direction = ball->_velocity;

	// See if the ball is standing still...
	if (ballRay._direction.magnitude() == 0) {
		tFinal = 0.0;
	} else {
		// See when the ball is going to fall to the player's height...
		float tFall = getBallImpactTime(ball, gravity, (player->height / 2));

		// See if the ball is headed straight for us...
		U32FltPoint2D intersection;
		if (ballRay.nearIntersection(playerMarker, &intersection)) {
			// Get the time till the ball reaches the player...
			float tPlayerImpact = (ball->center - player->center).xyMagnitude() / ball->_velocity.xyMagnitude();

			// Decide if it's best to stay put, or run back where the ball will fall to...
			tFinal = MAX(tPlayerImpact, tFall);
		} else {
			// The distance between the target player at future time t and and the
			// ball is equal to the playerSpeed times t. Solve for t...
			float a = (ball->_velocity.x * ball->_velocity.x) +
					  (ball->_velocity.y * ball->_velocity.y) -
					  (playerSpeed * playerSpeed);

			float b = (2 * ball->center.x * ball->_velocity.x) +
					  (2 * ball->center.y * ball->_velocity.y) -
					  (2 * player->center.x * ball->_velocity.x) -
					  (2 * player->center.y * ball->_velocity.y);

			float c = (ball->center.x * ball->center.x) +
					  (ball->center.y * ball->center.y) +
					  (player->center.x * player->center.x) +
					  (player->center.y * player->center.y) -
					  (2 * ball->center.x * player->center.x) -
					  (2 * ball->center.y * player->center.y);

			if (((b * b) < (4 * a * c)) || (a == 0)) {
				// Now see if we can get away with just going the way we were...
				if (playerRay.intersection(ballRay, &intersection)) {
					tFinal = (ball->center - intersection).xyMagnitude() / ball->_velocity.xyMagnitude();
				}
			} else {
				// Find the closest place we could intercept the ball...
				tFinal = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
				if (tFinal < 0) {
					tFinal = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
				}

				// See if the ball will come down low enough to catch by that time...
				tFinal = MAX((double)tFall, tFinal);
			}

			// Now see if we're just better off just going the way we were...
			if (playerRay.intersection(ballRay, &intersection)) {
				float tFutureImpact = (ball->center - intersection).xyMagnitude() / ball->_velocity.xyMagnitude();
				tFinal = MAX((double)tFutureImpact, tFinal);
			}
		}

		// Now see if we're better off just chasing the ball...
		tFinal = MAX(0.0, tFinal);
	}

	U32FltPoint2D targetPoint;
	targetPoint.x = ball->center.x + (ball->_velocity.x * tFinal);
	targetPoint.y = ball->center.y + (ball->_velocity.y * tFinal);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(targetPoint.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(targetPoint.y));

	return 1;
}

} // End of namespace Scumm
