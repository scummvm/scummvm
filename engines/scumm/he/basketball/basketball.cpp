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
#include "scumm/he/basketball/collision/bball_collision_shields.h"

namespace Scumm {

Basketball::Basketball(ScummEngine_v100he *vm) {
	_vm = vm;
	_court = new CBBallCourt();
	_shields = new CCollisionShieldVector();
}

Basketball::~Basketball() {}

int Basketball::u32FloatToInt(float input) {
	int output = 0;

	if (input < 0)
		output = (int)(input - 0.5F);
	else if (input > 0)
		output = (int)(input + 0.5F);

	return output;
}

int Basketball::u32DoubleToInt(double input) {
	int output = 0;

	if (input < 0)
		output = (int)(input - 0.5);
	else if (input > 0)
		output = (int)(input + 0.5);

	return output;
}

void Basketball::fillBallTargetList(const CCollisionSphere *sourceObject, CCollisionObjectVector *targetList) {
	// Add all of the court objects...
	_court->_objectTree.selectObjectsInBound(sourceObject->getBigBoundingBox(), targetList);

	// Add the shields...
	CCollisionShieldVector::const_iterator shieldIt;

	for (shieldIt = _shields->begin(); shieldIt != _shields->end(); ++shieldIt) {
		if (!shieldIt->_ignore) {
			targetList->push_back(&(*shieldIt));
		}
	}

	// Add all of the home players...
	Common::Array<CCollisionPlayer>::const_iterator homePlayerIt;

	for (homePlayerIt = _court->_homePlayerList.begin();
		 homePlayerIt != _court->_homePlayerList.end();
		 ++homePlayerIt) {
		if (!homePlayerIt->_ignore) {
			targetList->push_back(&(*homePlayerIt));
		}
	}

	// Add all of the away players...
	Common::Array<CCollisionPlayer>::const_iterator awayPlayerIt;

	for (awayPlayerIt = _court->_awayPlayerList.begin();
		 awayPlayerIt != _court->_awayPlayerList.end();
		 ++awayPlayerIt) {
		if (!awayPlayerIt->_ignore) {
			targetList->push_back(&(*awayPlayerIt));
		}
	}
}

void Basketball::fillPlayerTargetList(const CCollisionPlayer *sourceObject, CCollisionObjectVector *targetList) {
	// Add all of the court objects...
	_court->_objectTree.selectObjectsInBound(sourceObject->getBigBoundingBox(), targetList);

	// Add the shields if the player has the ball...
	if (sourceObject->_playerHasBall) {
		CCollisionShieldVector::const_iterator shieldIt;

		for (shieldIt = _shields->begin(); shieldIt != _shields->end(); ++shieldIt) {
			if (!shieldIt->_ignore) {
				targetList->push_back(&(*shieldIt));
			}
		}
	}

	// Add the basketball...
	if (!_court->_basketBall._ignore) {
		targetList->push_back((ICollisionObject *)&_court->_basketBall);
	}

	// Add the virtual basketball...
	if (!_court->_virtualBall._ignore) {
		targetList->push_back((ICollisionObject *)&_court->_virtualBall);
	}

	// Add all of the home players...
	Common::Array<CCollisionPlayer>::const_iterator homePlayerIt;

	for (homePlayerIt = _court->_homePlayerList.begin();
		 homePlayerIt != _court->_homePlayerList.end();
		 ++homePlayerIt) {
		if ((sourceObject != &(*homePlayerIt)) &&
			(!homePlayerIt->_ignore)) {
			targetList->push_back(&(*homePlayerIt));
		}
	}

	// Add all of the away players...
	Common::Array<CCollisionPlayer>::const_iterator awayPlayerIt;

	for (awayPlayerIt = _court->_awayPlayerList.begin();
		 awayPlayerIt != _court->_awayPlayerList.end();
		 ++awayPlayerIt) {
		if ((sourceObject != &(*awayPlayerIt)) &&
			(!awayPlayerIt->_ignore)) {
			targetList->push_back(&(*awayPlayerIt));
		}
	}
}

double Basketball::getLaunchAngle(int velocity, int hDist, int vDist, int gravity) {
	double theta;
	double bestTheta;
	double tempTheta;
	double thetaMin, thetaMax;

	double answer;
	double bestAnswer;
	double hiAnswer, loAnswer;

	double time;
	double bestTime;

	double targetAngle;
	int counter;

	assert(hDist > 0);

	// Set the search limits, and get a first guess...
	targetAngle = atan2(vDist, hDist);

	// The minimum angle of the pass is the angle directly between us and the target...
	thetaMin = targetAngle;

	// The maximum angle is straight up...
	thetaMax = BBALL_M_PI / 4;

	theta = (thetaMin + thetaMax) / 2;
	bestTheta = theta;
	bestTime = hDist / (velocity * cos(theta));
	if (bestTime < 0)
		bestTime *= -1;

	// Start binary searching for a close answer...
	counter = 0;
	answer = (2 * velocity * velocity * hDist * sin(theta) * cos(theta)) - (2 * velocity * velocity * vDist * cos(theta) * cos(theta)) - (gravity * hDist * hDist);
	if (answer < 0)
		answer *= -1;
	bestAnswer = answer;

	while ((fabs(answer * 1000.0) > 10.0) && (++counter <= 100)) {
		if (theta < 0)
			targetAngle += (2 * BBALL_M_PI);

		// Get a theta above and below the current one and see which one gets us closer
		// to satisfying the equation...
		tempTheta = (thetaMin + theta) / 2;
		loAnswer = (2 * velocity * velocity * hDist * sin(tempTheta) * cos(tempTheta)) - (2 * velocity * velocity * vDist * cos(tempTheta) * cos(tempTheta)) - (gravity * hDist * hDist);
		if (loAnswer < 0)
			loAnswer = 0 - loAnswer;

		tempTheta = (thetaMax + theta) / 2;
		hiAnswer = (2 * velocity * velocity * hDist * sin(tempTheta) * cos(tempTheta)) - (2 * velocity * velocity * vDist * cos(tempTheta) * cos(tempTheta)) - (gravity * hDist * hDist);
		if (hiAnswer < 0)
			hiAnswer = 0 - hiAnswer;

		if (loAnswer < hiAnswer) {
			thetaMax = theta;
			answer = loAnswer;
		} else {
			thetaMin = theta;
			answer = hiAnswer;
		}
		theta = (thetaMin + thetaMax) / 2;
		if (answer < 0)
			answer *= -1;

		if (answer <= bestAnswer) {
			if (answer == bestAnswer) {
				time = hDist / (velocity * cos(theta));
				if (time < 0)
					time *= -1;
				if (time < bestTime) {
					bestTheta = theta;
					bestTime = time;
					bestAnswer = answer;
				}
			} else {
				bestTheta = theta;
				bestTime = hDist / (velocity * cos(theta));
				if (bestTime < 0)
					bestTime *= -1;
				bestAnswer = answer;
			}
		}
	}

	bestTheta = (bestTheta * 180) / BBALL_M_PI;
	return bestTheta;
}

} // End of namespace Scumm
