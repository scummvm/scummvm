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
#include "scumm/he/basketball/collision/bball_collision.h"
#include "scumm/he/basketball/geo_translations.h"

namespace Scumm {

int LogicHEBasketball::u32_userInitCourt(int courtID) {
	static Common::String courtNames[] = {
		"",
		"Dobbaguchi", "Jocindas", "SandyFlats", "Queens",
		"Park", "Scheffler", "Polk", "McMillan",
		"CrownHill", "Memorial", "TechState", "Garden",
		"Moon", "Barn"
	};


	// Make sure nothing on the court is currently initialized.
	_vm->_basketball->_court->_objectTree.~CCollisionObjectTree();
	_vm->_basketball->_court->_objectList.clear();
	_vm->_basketball->_court->_homePlayerList.clear();
	_vm->_basketball->_court->_awayPlayerList.clear();

	// Initialize the shot spots
	_vm->_basketball->_court->_shotSpot[LEFT_BASKET].center.x = BASKET_X;
	_vm->_basketball->_court->_shotSpot[LEFT_BASKET].center.y = BASKET_Y;
	_vm->_basketball->_court->_shotSpot[LEFT_BASKET].center.z = BASKET_Z;
	_vm->_basketball->_court->_shotSpot[LEFT_BASKET].radius = SHOT_SPOT_RADIUS;

	_vm->_basketball->_court->_shotSpot[RIGHT_BASKET].center.x = MAX_WORLD_X - BASKET_X;
	_vm->_basketball->_court->_shotSpot[RIGHT_BASKET].center.y = BASKET_Y;
	_vm->_basketball->_court->_shotSpot[RIGHT_BASKET].center.z = BASKET_Z;
	_vm->_basketball->_court->_shotSpot[RIGHT_BASKET].radius = SHOT_SPOT_RADIUS;

	// Get the name and object file for this court.
	_vm->_basketball->_court->_name = courtNames[courtID];

	// Put together to relative path and filename.
	Common::Path objectFileName = Common::Path(Common::String::format("data/courts/%s.cof", courtNames[courtID].c_str()));

	// Create a file stream to the collision object file
	Common::File objectFile;
	if (!objectFile.open(objectFileName))
		error("LogicHEBasketball::u32_userInitCourt(): Could not open file '%s'", objectFileName.toString(Common::Path::kNativeSeparator).c_str());

	// Read in the object file version
	char fileVersion[32];
	int versionStringLength = objectFile.readUint32LE();

	if (versionStringLength <= 0 || versionStringLength > ARRAYSIZE(fileVersion) - 1)
		error("LogicHEBasketball::u32_userInitCourt(): Read from stream did not read the version string length correctly.");
	
	objectFile.read(fileVersion, versionStringLength);
	fileVersion[versionStringLength] = '\0';

	if (strcmp(fileVersion, "01.05"))
		error("LogicHEBasketball::u32_userInitCourt(): Invalid court version field: %s", fileVersion);

	// Read in the total number of objects
	_vm->_basketball->_court->_objectCount = objectFile.readUint32LE();
	_vm->_basketball->_court->_objectList.resize(_vm->_basketball->_court->_objectCount);

	// Keep a list of pointers to the court objects...
	CCollisionObjectVector objectPtrList;
	objectPtrList.resize(_vm->_basketball->_court->_objectCount);

	// Read in each court object...
	for (int i = 0; i < _vm->_basketball->_court->_objectCount; i++) {
		CCollisionBox *currentObject = &_vm->_basketball->_court->_objectList[i];

		// Read in this object's description...
		int descriptionStringLength = objectFile.readUint32LE();
		char *tmp = (char *)malloc(descriptionStringLength + 1);

		assert(tmp);

		objectFile.read(tmp, descriptionStringLength);
		tmp[descriptionStringLength] = '\0';

		Common::String tmp2(tmp);

		_vm->_basketball->_court->_objectList[i]._description = tmp2;

		free(tmp);

		// Read in all other object attributes...
		currentObject->_objectType = (EObjectType)objectFile.readUint32LE();
		currentObject->_collisionEfficiency = objectFile.readFloatLE();
		currentObject->_friction  = objectFile.readFloatLE();
		currentObject->_soundNumber = objectFile.readUint32LE();
		currentObject->_objectID = objectFile.readUint32LE();
		currentObject->minPoint.x = objectFile.readUint32LE();
		currentObject->minPoint.y = objectFile.readUint32LE();
		currentObject->minPoint.z = objectFile.readUint32LE();
		currentObject->maxPoint.x = objectFile.readUint32LE();
		currentObject->maxPoint.y = objectFile.readUint32LE();
		currentObject->maxPoint.z = objectFile.readUint32LE();
		objectPtrList[i] = currentObject;

		// Decide if this is a backboard, and keep track of it if it is...
		if (currentObject->_objectType == kBackboard) {
			// See which backboard it is...
			if (((currentObject->minPoint.x + currentObject->maxPoint.x) / 2) < (MAX_WORLD_X / 2)) {
				_vm->_basketball->_court->_backboardIndex[LEFT_BASKET] = i;
			} else {
				_vm->_basketball->_court->_backboardIndex[RIGHT_BASKET] = i;
			}
		}
	}

	_vm->_basketball->_court->_objectTree.initialize(objectPtrList);
	

	// Lower all the shields...
	u32_userLowerShields(ALL_SHIELD_ID);

	return 1;
}

int LogicHEBasketball::u32_userDeinitCourt() {
	// Make sure the collision object tree has been cleared...
	_vm->_basketball->_court->_objectTree.~CCollisionObjectTree();

	// Lower all the shields...
	u32_userLowerShields(ALL_SHIELD_ID);

	return 1;
}

int LogicHEBasketball::u32_userInitBall(U32FltPoint3D &ballLocation, U32FltVector3D &bellVelocity, int radius, int ballID) {
	_vm->_basketball->_court->_basketBall._description = "Basketball";
	_vm->_basketball->_court->_basketBall._objectType = kBall;
	_vm->_basketball->_court->_basketBall._objectID = ballID;
	_vm->_basketball->_court->_basketBall.center = ballLocation;
	_vm->_basketball->_court->_basketBall._velocity = bellVelocity;
	_vm->_basketball->_court->_basketBall.radius = radius;
	_vm->_basketball->_court->_basketBall._collisionEfficiency = 1.0F;
	_vm->_basketball->_court->_basketBall._friction = 0.0F;
	_vm->_basketball->_court->_basketBall._ignore = false;

	_vm->_basketball->_court->_basketBall.save();

	return 1;
}

int LogicHEBasketball::u32_userInitVirtualBall(U32FltPoint3D &ballLocation, U32FltVector3D &bellVelocity, int radius, int ballID) {
	_vm->_basketball->_court->_virtualBall._description = "Virtual Basketball";
	_vm->_basketball->_court->_virtualBall._objectType = kBall;
	_vm->_basketball->_court->_virtualBall._objectID = ballID;
	_vm->_basketball->_court->_virtualBall.center = ballLocation;
	_vm->_basketball->_court->_virtualBall._velocity = bellVelocity;
	_vm->_basketball->_court->_virtualBall.radius = radius;
	_vm->_basketball->_court->_virtualBall._collisionEfficiency = 1.0F;
	_vm->_basketball->_court->_virtualBall._friction = 0.0F;
	_vm->_basketball->_court->_virtualBall._ignore = false;

	_vm->_basketball->_court->_virtualBall.save();

	return 1;
}

int LogicHEBasketball::u32_userDeinitBall() {
	_vm->_basketball->_court->_basketBall._ignore = true;

	return 1;
}

int LogicHEBasketball::u32_userDeinitVirtualBall() {
	_vm->_basketball->_court->_virtualBall._ignore = true;

	return 1;
}

int LogicHEBasketball::u32_userInitPlayer(int playerID, U32FltPoint3D &playerLocation, int height, int radius, bool playerIsInGame) {
	if (!((FIRST_PLAYER <= playerID) && (playerID <= LAST_PLAYER)))
		error("LogicHEBasketball::u32_userInitPlayer(): Passed in invalid player ID");

	// Cycle through all of the player slots until an empty one is found...
	Common::Array<CCollisionPlayer> *playerList = _vm->_basketball->_court->getPlayerListPtr(playerID);
	if (playerList->size() < MAX_PLAYERS_ON_TEAM) {
		CCollisionPlayer newPlayer;
		newPlayer._objectType = kPlayer;
		newPlayer._objectID = playerID;
		newPlayer.height = height;
		newPlayer._catchHeight = PLAYER_CATCH_HEIGHT;
		newPlayer.radius = radius;
		newPlayer.center = playerLocation;
		newPlayer.center.z = playerLocation.z + (height / 2);
		newPlayer._collisionEfficiency = 0.5F;
		newPlayer._friction = 0.5F;
		newPlayer._playerIsInGame = playerIsInGame;
		newPlayer.save();
		playerList->push_back(newPlayer);
		return 1;
	} else {
		warning("LogicHEBasketball::u32_userInitPlayer(): There were no empty player slots. You can't initialize a new player until you deinit one.");
		return 0;
	}
}

int LogicHEBasketball::u32_userDeinitPlayer(int playerID) {
	if (!((FIRST_PLAYER <= playerID) && (playerID <= LAST_PLAYER)))
		error("LogicHEBasketball::u32_userDeinitPlayer(): Passed in invalid player ID");

	int index = _vm->_basketball->_court->getPlayerIndex(playerID);
	Common::Array<CCollisionPlayer> *playerList = _vm->_basketball->_court->getPlayerListPtr(playerID);
	playerList->remove_at(index);

	return 1;
}

int LogicHEBasketball::u32_userPlayerOff(int playerID) {
	if (!((FIRST_PLAYER <= playerID) && (playerID <= LAST_PLAYER)))
		error("LogicHEBasketball::u32_userPlayerOff(): Passed in invalid player ID");

	_vm->_basketball->_court->getPlayerPtr(playerID)->_ignore = true;

	return 1;
}

int LogicHEBasketball::u32_userPlayerOn(int playerID) {
	if (!((FIRST_PLAYER <= playerID) && (playerID <= LAST_PLAYER)))
		error("LogicHEBasketball::u32_userPlayerOn(): Passed in invalid player ID");

	_vm->_basketball->_court->getPlayerPtr(playerID)->_ignore = false;

	return 1;
}

static void trackCollisionObject(const ICollisionObject &sourceObject, const ICollisionObject &targetObject, CCollisionObjectVector *objectVector) {
	float currentDist = sourceObject.getObjectDistance(targetObject);

	// As an object moves backwards along its velocity vector and new collisions
	// are detected, older collisions may become invalid. Here, we go through prior
	// collisions, and see which ones are invalidated by this new collision.
	for (CCollisionObjectVector::const_iterator objectIt = objectVector->begin();
		 objectIt != objectVector->end();
		 ++objectIt) {
		float pastDist = sourceObject.getObjectDistance(**objectIt);

		// If the distance between the source object and the current target object
		// is less than or equal to the distance between the source object and the last
		// target object, then the current target object is stored along side the last
		// target object. Otherwise, the current object replaces the last object.
		if ((fabs(pastDist - currentDist) < COLLISION_EPSILON) ||
			(!sourceObject.isCollisionHandled(targetObject)) ||
			(!sourceObject.isCollisionHandled(**objectIt))) {
			break;
		}
	}

	// Make sure that we aren't keeping track of the same object twice...
	if (!objectVector->contains(targetObject)) {
		objectVector->push_back(&targetObject);
	}
}

int LogicHEBasketball::u32_userDetectBallCollision(U32FltPoint3D &ballLocation, U32FltVector3D &ballVector, int recordCollision, int ballID) {
	bool ballIsClear = false; // Flag that indicates if the ball collided with any objects on its current vector
	bool errorOccurred = false;

	U32Distance3D distance;                 // The distance between the ball and a collision object candidate
	CCollisionObjectVector targetList;      // All potential collision candidates
	CCollisionObjectVector collisionVector; // All objects that have been collided with
	CCollisionObjectVector rollingVector;   // All objects that have been rolled on

	int collisionOccurred = 0;
	int rollingHappened = 0;

	// Determine which ball we're dealing with...
	CCollisionBasketball *sourceBall = _vm->_basketball->_court->getBallPtr(ballID);

	// Update the position and vector of the basketball...
	sourceBall->center = ballLocation;
	sourceBall->_velocity = ballVector;

	// Clear the ball's collision stack...
	sourceBall->_objectCollisionHistory.clear();
	sourceBall->_objectRollingHistory.clear();

	// Find out who our potential collision candidates are...
	_vm->_basketball->fillBallTargetList(sourceBall, &targetList);

	// See if there was an error while traversing the object tree,
	// if there was put the player in the last known safe position...
	if (_vm->_basketball->_court->_objectTree.checkErrors()) {
		sourceBall->restore();
	}

	for (int i = 0; (i < MAX_BALL_COLLISION_PASSES) && (!ballIsClear); i++) {
		float totalTime = 0.0F; // The time it takes to back out of all objects we have intersected while on the current vector
		ballIsClear = true;

		// Go through all of the collision candidates....
		for (size_t j = 0; j < targetList.size(); ++j) {
			const ICollisionObject *targetObject = targetList[j];
			assert(targetObject);

			// See if we intersect the current object...
			bool intersectionResult = sourceBall->testObjectIntersection(*targetObject, &distance);
			if (intersectionResult) {
				// If we are intersecting a moving object, make sure that we actually
				// ran into them, and they didn't just run into us...
				if (sourceBall->validateCollision(*targetObject, &distance)) {
					// If we are intersecting the object, back out of it...
					if (sourceBall->backOutOfObject(*targetObject, &distance, &totalTime)) {
						// Move in to the exact point of collision...
						if (sourceBall->nudgeObject(*targetObject, &distance, &totalTime)) {
							// Keep track of this object so we can respond to the collision later...
							trackCollisionObject(*sourceBall, *targetObject, &sourceBall->_objectCollisionHistory);

							if (sourceBall->isCollisionHandled(*targetObject)) {
								trackCollisionObject(*sourceBall, *targetObject, &collisionVector);
								ballIsClear = false;
							}

							collisionOccurred = 1;
						} else {
							errorOccurred = true;
						}
					} else {
						errorOccurred = true;
					}
				}
			} else {
				// See if we are passing over a player...
				if (sourceBall->testCatch(*targetObject, &distance, _vm->_basketball->_court)) {
					trackCollisionObject(*sourceBall, *targetObject, &sourceBall->_objectCollisionHistory);
					collisionOccurred = true;
				}
			}

			// See if we are rolling on the current object...
			if (sourceBall->isOnObject(*targetObject, distance)) {
				rollingHappened = 1;

				if (!intersectionResult) {
					// This is not really a collision, but the ball is rolling, so we want to slow it down...
					trackCollisionObject(*sourceBall, *targetObject, &rollingVector);
					trackCollisionObject(*sourceBall, *targetObject, &sourceBall->_objectRollingHistory);
				}
			}
		}

		// Adjust the ball's velocity and position due to any collisions...
		sourceBall->handleCollisions(&rollingVector, &totalTime, false);
		sourceBall->handleCollisions(&collisionVector, &totalTime, true);
	}

	// Keep track of how long we've been rolling...
	if (rollingHappened) {
		++sourceBall->_rollingCount;
	} else {
		sourceBall->_rollingCount = 0;
	}

	// If there were no errors this frame, save the position...
	if (!errorOccurred) {
		sourceBall->save();
	}

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(sourceBall->center.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(sourceBall->center.y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, _vm->_basketball->u32FloatToInt(sourceBall->center.z));
	writeScummVar(_vm1->VAR_U32_USER_VAR_D, _vm->_basketball->u32FloatToInt(sourceBall->_velocity.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_E, _vm->_basketball->u32FloatToInt(sourceBall->_velocity.y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_F, _vm->_basketball->u32FloatToInt(sourceBall->_velocity.z));
	writeScummVar(_vm1->VAR_U32_USER_VAR_G, collisionOccurred);
	writeScummVar(_vm1->VAR_U32_USER_VAR_H, rollingHappened != 0 ? 0 : 1);

	return 1;
}

int LogicHEBasketball::u32_userDetectPlayerCollision(int playerID, U32FltPoint3D &playerLocation, U32FltVector3D &playerVector, bool playerHasBall) {
	U32Distance3D distance;                 // The distance between the ball and a collision object candidate
	CCollisionObjectVector collisionVector; // All objects that have been collided with

	int playerIsOnObject = 0;
	int collisionOccurred = 0;

	bool playerIsClear = false;
	bool errorOccurred = false;

	float totalTime = 0.0F; // The time it takes to back out of all objects we have intersected on this frame

	if (!((FIRST_PLAYER <= playerID) && (playerID <= LAST_PLAYER)))
		error("LogicHEBasketball::u32_userDetectPlayerCollision(): Passed in invalid player ID");

	// Get the player who is being tested...
	CCollisionPlayer *sourcePlayer = _vm->_basketball->_court->getPlayerPtr(playerID);

	// Update the player's status...
	sourcePlayer->_playerHasBall = playerHasBall;

	// In SCUMM code, the center of a player in the z dimension is at their feet.
	// In U32 code, it is in the middle of the cylinder, so make the translation...
	playerLocation.z += (sourcePlayer->height / 2);

	// Update the player's position and velocity...
	sourcePlayer->center = playerLocation;
	sourcePlayer->_velocity = playerVector;
	sourcePlayer->_movementType = kStraight;

	// Clear the player's collision stack...
	sourcePlayer->_objectCollisionHistory.clear();
	sourcePlayer->_objectRollingHistory.clear();

	// Find out who our potential collision candidates are...
	CCollisionObjectVector targetList;
	_vm->_basketball->fillPlayerTargetList(sourcePlayer, &targetList);

	// See if there was an error while traversing the object tree,
	// if there was put the player in the last known safe position...
	if (_vm->_basketball->_court->_objectTree.checkErrors()) {
		sourcePlayer->restore();
	}

	for (int i = 0; (i < MAX_PLAYER_COLLISION_PASSES) && (!playerIsClear); i++) {
		playerIsClear = 1;

		// Check all of the collision candidates...
		for (size_t j = 0; j < targetList.size(); ++j) {
			const ICollisionObject *targetObject = targetList[j];
			assert(targetObject);

			// See if we intersect the current object...
			bool intersectionResult = sourcePlayer->testObjectIntersection(*targetObject, &distance);
			if (intersectionResult) {
				// If we are intersecting a moving object, make sure that we actually
				// ran into them, and they didn't just run into us...
				if (sourcePlayer->validateCollision(*targetObject, &distance)) {
					// If we are intersecting an object, back out to the exact point of collision...
					if (sourcePlayer->backOutOfObject(*targetObject, &distance, &totalTime)) {
						// Move in to the exact point of collision...
						if (sourcePlayer->nudgeObject(*targetObject, &distance, &totalTime)) {
							trackCollisionObject(*sourcePlayer, *targetObject, &sourcePlayer->_objectCollisionHistory);
							collisionOccurred = true;

							if (sourcePlayer->isCollisionHandled(*targetObject)) {
								trackCollisionObject(*sourcePlayer, *targetObject, &collisionVector);
								playerIsClear = false;
							}
						} else {
							errorOccurred = true;
						}
					} else {
						errorOccurred = true;
					}
				}

			} else {

				// See if the virtual ball is passing over us...
				if (sourcePlayer->testCatch(*targetObject, &distance, _vm->_basketball->_court)) {
					trackCollisionObject(*sourcePlayer, *targetObject, &sourcePlayer->_objectCollisionHistory);
					collisionOccurred = true;
				}
			}

			// See if we are standing on the current object...
			if (sourcePlayer->isOnObject(*targetObject, distance)) {
				playerIsOnObject = true;
			}
		}

		sourcePlayer->handleCollisions(&collisionVector, &totalTime, true);
	}

	// If there were no errors this frame, save the position...
	if (!errorOccurred) {
		sourcePlayer->save();
	}

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(sourcePlayer->center.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(sourcePlayer->center.y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, _vm->_basketball->u32FloatToInt(sourcePlayer->center.z - (sourcePlayer->height / 2)));
	writeScummVar(_vm1->VAR_U32_USER_VAR_D, _vm->_basketball->u32FloatToInt(sourcePlayer->_velocity.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_E, _vm->_basketball->u32FloatToInt(sourcePlayer->_velocity.y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_F, _vm->_basketball->u32FloatToInt(sourcePlayer->_velocity.z));
	writeScummVar(_vm1->VAR_U32_USER_VAR_G, collisionOccurred);
	writeScummVar(_vm1->VAR_U32_USER_VAR_H, playerIsOnObject != 0 ? 0 : 1);

	return 1;
}

int LogicHEBasketball::u32_userGetLastBallCollision(int ballID) {
	EObjectType lastObjectType = kNoObjectType;
	int objectID = 0;

	// Determine which ball we're dealing with
	CCollisionSphere *sourceBall;
	if (ballID == _vm->_basketball->_court->_basketBall._objectID) {
		sourceBall = (CCollisionSphere *)&_vm->_basketball->_court->_basketBall;
	} else if (ballID == _vm->_basketball->_court->_virtualBall._objectID) {
		sourceBall = (CCollisionSphere *)&_vm->_basketball->_court->_virtualBall;
	} else {
		warning("LogicHEBasketball::u32_userGetLastBallCollision(): Invalid ball ID %d.", ballID);
		sourceBall = (CCollisionSphere *)&_vm->_basketball->_court->_basketBall;
	}

	if (!sourceBall->_objectCollisionHistory.empty()) {
		lastObjectType = sourceBall->_objectCollisionHistory.back()->_objectType;
		objectID = sourceBall->_objectCollisionHistory.back()->_objectID;
		sourceBall->_objectCollisionHistory.pop_back();
	} else if (!sourceBall->_objectRollingHistory.empty()) {
		lastObjectType = sourceBall->_objectRollingHistory.back()->_objectType;
		objectID = sourceBall->_objectRollingHistory.back()->_objectID;
		sourceBall->_objectRollingHistory.pop_back();
	}

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, lastObjectType);
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, objectID);

	return 1;
}

int LogicHEBasketball::u32_userGetLastPlayerCollision(int playerID) {
	EObjectType lastObjectType = kNoObjectType;
	int objectID = 0;
	bool playerIsOnObject = false;

	CCollisionPlayer *pPlayer = _vm->_basketball->_court->getPlayerPtr(playerID);

	if (!pPlayer->_objectCollisionHistory.empty()) {
		const ICollisionObject *targetObject = pPlayer->_objectCollisionHistory.back();

		lastObjectType = targetObject->_objectType;
		objectID = targetObject->_objectID;

		// See if we are standing on the current object
		U32Distance3D distance;
		pPlayer->testObjectIntersection(*targetObject, &distance);
		if (pPlayer->isOnObject(*targetObject, distance)) {
			playerIsOnObject = true;
		}

		pPlayer->_objectCollisionHistory.pop_back();
	}

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, lastObjectType);
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, objectID);
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, playerIsOnObject);

	return 1;
}

} // End of namespace Scumm
