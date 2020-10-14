/** @file movements.cpp
	@brief
	This file contains 3d actor movement rendering routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <math.h>

#include "movements.h"
#include "actor.h"
#include "lbaengine.h"
#include "renderer.h"
#include "grid.h"
#include "scene.h"
#include "keyboard.h"
#include "animations.h"
#include "collision.h"
#include "gamestate.h"

/** Get shadow position
	@param X Shadow X coordinate
	@param Y Shadow Y coordinate
	@param Z Shadow Z coordinate */
void getShadowPosition(int32 X, int32 Y, int32 Z) {
	int32 tempX;
	int32 tempY;
	int32 tempZ;
	uint8* ptr;

	tempX = (X + 0x100) >> 9;
	tempY = Y >> 8;
	tempZ = (Z + 0x100) >> 9;

	ptr = blockBuffer + tempY * 2 + tempX * 25 * 2 + (tempZ << 6) * 25 * 2;

	while (tempY) { // search down until either ground is found or lower border of the cube is reached
		if (*(int16*)ptr) // found the ground
			break;

		tempY--;
		ptr -= 2;
	}

	shadowCollisionType = 0;

	collisionX = tempX;
	collisionY = tempY;
	collisionZ = tempZ;

	processActorX = X;
	processActorY = (tempY + 1) << 8;
	processActorZ = Z;

	if (*ptr) { //*((uint8 *)(blockPtr))
        uint8 *blockPtr;
        uint8 brickShape;

        blockPtr = getBlockLibrary(*(ptr) - 1) + 3 + *(ptr + 1) * 4;
        brickShape = *((uint8 *)(blockPtr));

		shadowCollisionType = brickShape;
		reajustActorPosition(shadowCollisionType);
	}

	shadowX = processActorX;
	shadowY = processActorY;
	shadowZ = processActorZ;
}

/** Set actor safe angle
	@param startAngle start angle
	@param endAngle end angle
	@param stepAngle number of steps
	@param movePtr Pointer to process movements */
void setActorAngleSafe(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct * movePtr) {
	movePtr->from = startAngle & 0x3FF;
	movePtr->to = endAngle & 0x3FF;
	movePtr->numOfStep = stepAngle & 0x3FF;
	movePtr->timeOfChange = lbaTime;
}

/** Clear actors safe angle
	@param actorPtr actor pointer */
void clearRealAngle(ActorStruct * actorPtr) {
	setActorAngleSafe(actorPtr->angle, actorPtr->angle, 0, &actorPtr->move);
}

/** Set actor safe angle
	@param startAngle start angle
	@param endAngle end angle
	@param stepAngle number of steps
	@param movePtr Pointer to process movements */
void setActorAngle(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct * movePtr) {
	movePtr->from = startAngle;
	movePtr->to = endAngle;
	movePtr->numOfStep = stepAngle;
	movePtr->timeOfChange = lbaTime;
}

/** Get actor angle
	@param x1 Actor 1 X
	@param z1 Actor 1 Z
	@param x2 Actor 2 X
	@param z2 Actor 2 Z */
#define PI 3.14159265
int32 getAngleAndSetTargetActorDistance(int32 x1, int32 z1, int32 x2, int32 z2) {
    /*
	//Pythagoras
    targetActorDistance = (int32)sqrt((int64)(((z2 - z1)*(z2 - z1) + (x2 - x1)*(x2 - x1))));

	if (targetActorDistance == 0)
        return 0;

    //given two points, we calculate its arc-tangent in radians
    //Then we convert from radians (360 degrees == 2*PI) to a 10bit value (360 degrees == 1024) and invert the rotation direction
    //Then we add an offset of 90 degrees (256) and limit it to the 10bit value range.
    return (256 + ((int32)floor((-1024 * atan2((int64)(z2-z1), (int32)(x2-x1))) / (2*PI)))) % 1024;
	*/

	int32 newX, newZ, difX, difZ, tmpX, tmpZ, tmpEx, flag, destAngle, startAngle, finalAngle;

	difZ = tmpZ = z2 - z1;
	newZ = tmpZ * tmpZ;

	difX = tmpX = x2 - x1;
	newX = tmpX * tmpX;

	// Exchange X and Z
	if (newX < newZ) {
		tmpEx = difX;
		difX = difZ;
		difZ = tmpEx;

		flag = 1;
	} else {
		flag = 0;
	}

	targetActorDistance = (int32)sqrt((int64)(newX + newZ));

	if (!targetActorDistance) {
		return 0;
	}

	destAngle = (difZ << 14) / targetActorDistance;

	startAngle = 0;
//	stopAngle  = 0x100;

	while (shadeAngleTab3[startAngle] > destAngle) {
		startAngle++;
	}

	if (shadeAngleTab3[startAngle] != destAngle) {
		if ((shadeAngleTab3[startAngle - 1] + shadeAngleTab3[startAngle]) / 2 <= destAngle) {
			startAngle--;
		}
	}

	finalAngle = 128 + startAngle;

	if (difX <= 0) {
		finalAngle = -finalAngle;
	}

	if (flag & 1) {
		finalAngle = -finalAngle + 0x100;
	}

	return finalAngle & 0x3FF;
}

/** Get actor real angle
	@param movePtr Pointer to process movements */
int32 getRealAngle(ActorMoveStruct * movePtr) {
	int32 timePassed;
	int32 remainingAngle;

	if (movePtr->numOfStep) {
		timePassed = lbaTime - movePtr->timeOfChange;

		if (timePassed >= movePtr->numOfStep) {	// rotation is finished
			movePtr->numOfStep = 0;
			return movePtr->to;
		}

		remainingAngle = movePtr->to - movePtr->from;

		if (remainingAngle < -0x200) {
			remainingAngle += 0x400;
		} else if (remainingAngle > 0x200) {
			remainingAngle -= 0x400;
		}

		remainingAngle *= timePassed;
		remainingAngle /= movePtr->numOfStep;
		remainingAngle += movePtr->from;

		return (remainingAngle);
	}

	return movePtr->to;
}

/** Get actor step
	@param movePtr Pointer to process movements */
int32 getRealValue(ActorMoveStruct * movePtr) {
	int32 tempStep;

	if (!movePtr->numOfStep)
		return movePtr->to;

	if (!(lbaTime - movePtr->timeOfChange < movePtr->numOfStep)) {
		movePtr->numOfStep = 0;
		return movePtr->to;
	}

	tempStep = movePtr->to - movePtr->from;
	tempStep *= lbaTime - movePtr->timeOfChange;
	tempStep /= movePtr->numOfStep;

	return tempStep + movePtr->from;
}

/** Rotate actor with a given angle
	@param X Actor current X coordinate
	@param Z Actor current Z coordinate
	@param angle Actor angle to rotate */
void rotateActor(int32 X, int32 Z, int32 angle) {
    double radians = 2*PI*angle/0x400;
    destX = (int32)(X*cos(radians) + Z*sin(radians));
    destZ = (int32)(-X*sin(radians) + Z*cos(radians));
}

/** Get distance value in 2D
	@param x1 Actor 1 X coordinate
	@param z1 Actor 1 Z coordinate
	@param x2 Actor 2 X coordinate
	@param z2 Actor 2 Z coordinate */
int32 getDistance2D(int32 x1, int32 z1, int32 x2, int32 z2) {
	return (int32)sqrt((int64)((x2-x1)*(x2-x1) + (z2-z1)*(z2-z1)));
}

/** Get distance value in 3D
	@param x1 Actor 1 X coordinate
	@param y1 Actor 1 Y coordinate
	@param z1 Actor 1 Z coordinate
	@param x2 Actor 2 X coordinate
	@param y2 Actor 2 Y coordinate
	@param z2 Actor 2 Z coordinate */
int32 getDistance3D(int32 x1, int32 y1, int32 z1, int32 x2, int32 y2, int32 z2) {
	return (int32)sqrt((int64)((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1)));
}

/** Move actor around the scene
	@param angleFrom Current actor angle
	@param angleTo Angle to rotate
	@param speed Rotate speed
	@param movePtr Pointer to process movements */
void moveActor(int32 angleFrom, int32 angleTo, int32 speed, ActorMoveStruct *movePtr) { // ManualRealAngle
	int32 numOfStepInt;
	int16 numOfStep;
	int16 from;
	int16 to;

	from = angleFrom & 0x3FF;
	to   = angleTo & 0x3FF;

	movePtr->from = from;
	movePtr->to   = to;

	numOfStep = (from - to) << 6;

	if (numOfStep < 0) {
		numOfStepInt = -numOfStep;
	} else {
		numOfStepInt = numOfStep;
	}

	numOfStepInt >>= 6;

	numOfStepInt *= speed;
	numOfStepInt >>= 8;

	movePtr->numOfStep = (int16)numOfStepInt;
	movePtr->timeOfChange = lbaTime;
}

void processActorMovements(int32 actorIdx) {
	ActorStruct *actor = &sceneActors[actorIdx];

	if (actor->entity == -1)
		return;

	if (actor->dynamicFlags.bIsFalling) {
		int16 tempAngle = 0;

		if (actor->controlMode != 1)
			return;

		if (key & 4)
			tempAngle = 0x100;

		if (key & 8)
			tempAngle = -0x100;

		moveActor(actor->angle, actor->angle + tempAngle, actor->speed, &actor->move);

		heroPressedKey = key;
	} else {
		int16 tempAngle;

		if (!actor->staticFlags.bIsSpriteActor) {
			if (actor->controlMode != kManual) {
				actor->angle = getRealAngle(&actor->move);
			}
		}

		switch (actor->controlMode) {
		case kNoMove:
			break;
		case kManual:
			if (!actorIdx) { // take this out when we want to give manual movements to other characters than Hero
				heroAction = 0;

				// If press W for action
				if (skipIntro == 0x11) {
					heroAction = 1;
				}

				// Process hero actions
				switch (heroBehaviour) {
				case kNormal:
					if (loopPressedKey & 1) {
						heroAction = 1;
					}
					break;
				case kAthletic:
					if (loopPressedKey & 1) {
						initAnim(kJump, 1, 0, actorIdx);
					}
					break;
				case kAggressive:
					if (loopPressedKey & 1) {
						if (autoAgressive) {
							heroMoved = 1;
							actor->angle = getRealAngle(&actor->move);
							if (!(previousLoopPressedKey & 1) || !actor->anim) {
								int32 aggresiveMode = Rnd(3);

								switch (aggresiveMode) {
								case 0:
									initAnim(kKick, 1, 0, actorIdx);
									break;
								case 1:
									initAnim(kRightPunch, 1, 0, actorIdx);
									break;
								case 2:
									initAnim(kLeftPunch, 1, 0, actorIdx);
									break;
								}
							}
						} else {
							if (key & 8) {
								initAnim(kRightPunch, 1, 0, actorIdx);
							}

							if (key & 4) {
								initAnim(kLeftPunch, 1, 0, actorIdx);
							}

							if (key & 1) {
								initAnim(kKick, 1, 0, actorIdx);
							}
						}
					}
					break;
				case kDiscrete:
					if (loopPressedKey & 1) {
						initAnim(kHide, 0, 255, actorIdx);
					}
					break;
				}
			}

			if ((loopPressedKey & 8) && !gameFlags[GAMEFLAG_INVENTORY_DISABLED]) {
				if (usingSabre == 0) { // Use Magic Ball
					if (gameFlags[GAMEFLAG_HAS_MAGICBALL]) {
						if (magicBallIdx == -1) {
							initAnim(kThrowBall, 1, 0, actorIdx);
						}

						heroMoved = 1;
						actor->angle = getRealAngle(&actor->move);
					}
				} else {
					if (gameFlags[GAMEFLAG_HAS_SABRE]) {
						if (actor->body != GAMEFLAG_HAS_SABRE) {
							initModelActor(GAMEFLAG_HAS_SABRE, actorIdx);
						}

						initAnim(kSabreAttack, 1, 0, actorIdx);

						heroMoved = 1;
						actor->angle = getRealAngle(&actor->move);
					}
				}
			}

			if (!loopPressedKey || heroAction) {

				if (key & 3) {  // if continue walking
					heroMoved = 0; // don't break animation
				}

				if (key != heroPressedKey || loopPressedKey != heroPressedKey2) {
					if (heroMoved) {
						initAnim(kStanding, 0, 255, actorIdx);
					}
				}

				heroMoved = 0;

				if (key & 1) { // walk forward
					if (!currentActorInZone) {
						initAnim(kForward, 0, 255, actorIdx);
					}
					heroMoved = 1;
				}

				if (key & 2 && !(key & 1)) { // walk backward
					initAnim(kBackward, 0, 255, actorIdx);
					heroMoved = 1;
				}

				if (key & 4) { // turn left
					heroMoved = 1;
					if (actor->anim == 0) {
						initAnim(kTurnLeft, 0, 255, actorIdx);
					} else {
						if (!actor->dynamicFlags.bIsRotationByAnim) {
							actor->angle = getRealAngle(&actor->move);
						}
					}
				}

				if (key & 8) { // turn right
					heroMoved = 1;
					if (actor->anim == 0) {
						initAnim(kTurnRight, 0, 255, actorIdx);
					} else {
						if (!actor->dynamicFlags.bIsRotationByAnim) {
							actor->angle = getRealAngle(&actor->move);
						}
					}
				}
			}

			tempAngle = 0;

			if (key & 4) {
				tempAngle = 0x100;
			}

			if (key & 8) {
				tempAngle = -0x100;
			}

			moveActor(actor->angle, actor->angle + tempAngle, actor->speed, &actor->move);

			heroPressedKey  = key;
			heroPressedKey2 = loopPressedKey;

			break;
		case kFollow: {
			int32 newAngle = getAngleAndSetTargetActorDistance(actor->X, actor->Z, sceneActors[actor->followedActor].X, sceneActors[actor->followedActor].Z);
			if (actor->staticFlags.bIsSpriteActor) {
				actor->angle = newAngle;
			} else {
				moveActor(actor->angle, newAngle, actor->speed, &actor->move);
			}
		}
			break;
		case kTrack:
			if (actor->positionInMoveScript == -1) {
				actor->positionInMoveScript = 0;
			}
			break;
		case kFollow2:		// unused
		case kTrackAttack: // unused
			break;
		case kSameXZ:
			actor->X = sceneActors[actor->followedActor].X;
			actor->Z = sceneActors[actor->followedActor].Z;
			break;
		case kRandom: {
			if (!actor->dynamicFlags.bIsRotationByAnim) {
				if (actor->brickShape & 0x80) {
					moveActor(actor->angle, (((rand() & 0x100) + (actor->angle - 0x100)) & 0x3FF ), actor->speed, &actor->move);
					actor->info0 = Rnd(300) + lbaTime + 300;
					initAnim(0, 0, 255, actorIdx);
				}

				if (!actor->move.numOfStep) {
					initAnim(1, 0, 255, actorIdx);
					if(lbaTime > actor->info0) {
						moveActor(actor->angle, (((rand() & 0x100) + (actor->angle - 0x100)) & 0x3FF), actor->speed, &actor->move);
                        actor->info0 = Rnd(300) + lbaTime + 300;
                    }
				}
			}
		}
			break;
		default:
			printf("Unknown Control mode %d\n", actor->controlMode);
			break;
		}
	}
}
