/** @file collision.cpp
	@brief
	This file contains movies routines

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

#include "collision.h"
#include "scene.h"
#include "actor.h"
#include "movements.h"
#include "grid.h"
#include "main.h"
#include "animations.h"
#include "renderer.h"
#include "extra.h"

/** Check if actor 1 is standing in actor2
	@param actorIdx1 Actor 1 index
	@param actorIdx2 Actor 2 index */
int32 standingOnActor(int32 actorIdx1, int32 actorIdx2) { // CheckZvOnZv
	int32 x1Left, y1Left, z1Left, x1Right, y1Right, z1Right;
	int32 x2Left, y2Left, z2Left, x2Right, y2Right, z2Right;
	ActorStruct *actor1;
	ActorStruct *actor2;

	actor1 = &sceneActors[actorIdx1];
	actor2 = &sceneActors[actorIdx2];

	// Current actor (actor 1)
	x1Left = processActorX + actor1->boudingBox.X.bottomLeft;
	x1Right = processActorX + actor1->boudingBox.X.topRight;

	y1Left = processActorY + actor1->boudingBox.Y.bottomLeft;
	y1Right = processActorY + actor1->boudingBox.Y.topRight;

	z1Left = processActorZ + actor1->boudingBox.Z.bottomLeft;
	z1Right = processActorZ + actor1->boudingBox.Z.topRight;

	// Actor 2
	x2Left = actor2->X + actor2->boudingBox.X.bottomLeft;
	x2Right = actor2->X + actor2->boudingBox.X.topRight;

	y2Left = actor2->Y + actor2->boudingBox.Y.bottomLeft;
	y2Right = actor2->Y + actor2->boudingBox.Y.topRight;

	z2Left = actor2->Z + actor2->boudingBox.Z.bottomLeft;
	z2Right = actor2->Z + actor2->boudingBox.Z.topRight;

	if (x1Left >= x2Right)
		return 0; // not standing

	if (x1Right <= x2Left)
		return 0;

	if (y1Left > (y2Right + 1))
		return 0;

	if (y1Left <= (y2Right - 0x100))
		return 0;

	if (y1Right <= y2Left)
		return 0;

	if (z1Left >= z2Right)
		return 0;

	if (z1Right <= z2Left)
		return 0;

	return 1; // standing
}

int32 getAverageValue(int32 var0, int32 var1, int32 var2, int32 var3) {
	if (var3 <= 0) {
		return var0;
	}

	if (var3 >= var2) {
		return var1;
	}

    return ((((var1 - var0) * var3) / var2) + var0);
}

/** Reajust actor position in scene according with brick shape bellow actor
	@param brickShape Shape of brick bellow the actor */
void reajustActorPosition(int32 brickShape) {
	int32 brkX, brkY, brkZ;

	if (!brickShape) {
		return;
	}

	brkX = (collisionX << 9) - 0x100;
	brkY = collisionY << 8;
	brkZ = (collisionZ << 9) - 0x100;

	// double-side stairs
	if (brickShape >= kDoubleSideStairsTop1 && brickShape <= kDoubleSideStairsRight2) {
		switch (brickShape) {
		case kDoubleSideStairsTop1:
			if (processActorZ - collisionZ <= processActorX - collisionX) {
				brickShape = kStairsTopLeft;
			} else {
				brickShape = kStairsTopRight;
			}
			break;
		case kDoubleSideStairsBottom1:
			if (processActorZ - collisionZ <= processActorX - collisionX) {
				brickShape = kStairsBottomLeft;
			} else {
				brickShape = kStairsBottomRight;
			}
			break;
		case kDoubleSideStairsLeft1:
			if (512 - processActorX - collisionX <= processActorZ - collisionZ) {
				brickShape = kStairsTopLeft;
			} else {
				brickShape = kStairsBottomLeft;
			}
			break;
		case kDoubleSideStairsRight1:
			if (512 - processActorX - collisionX <= processActorZ - collisionZ) {
				brickShape = kStairsTopRight;
			} else {
				brickShape = kStairsBottomRight;
			}
			break;
		case kDoubleSideStairsTop2:
			if (processActorX - collisionX >= processActorZ - collisionZ) {
				brickShape = kStairsTopRight;
			} else {
				brickShape = kStairsTopLeft;
			}
			break;
		case kDoubleSideStairsBottom2:
			if (processActorZ - collisionZ <= processActorX - collisionX) {
				brickShape = kStairsBottomRight;
			} else {
				brickShape = kStairsBottomLeft;
			}
			break;
		case kDoubleSideStairsLeft2:
			if (512 - processActorX - collisionX <= processActorZ - collisionZ) {
				brickShape = kStairsBottomLeft;
			} else {
				brickShape = kStairsTopLeft;
			}
			break;
		case kDoubleSideStairsRight2:
			if (512 - processActorX - collisionX <= processActorZ - collisionZ) {
				brickShape = kStairsBottomRight;
			} else {
				brickShape = kStairsTopRight;
			}
			break;
		default:
			if (cfgfile.Debug == 1) {
				printf("Brick Shape %d unsupported\n", brickShape);
			}
			break;
		}
	}

	if (brickShape >= kStairsTopLeft && brickShape <= kStairsBottomRight) {
		switch (brickShape) {
		case kStairsTopLeft:
			processActorY = brkY + getAverageValue(0, 0x100, 0x200, processActorX - brkX);
			break;
		case kStairsTopRight:
			processActorY = brkY + getAverageValue(0, 0x100, 0x200, processActorZ - brkZ);
			break;
		case kStairsBottomLeft:
			processActorY = brkY + getAverageValue(0x100, 0, 0x200, processActorZ - brkZ);
			break;
		case kStairsBottomRight:
			processActorY = brkY + getAverageValue(0x100, 0, 0x200, processActorX - brkX);
			break;
		default:
			break;
		}
	}
}

/** Check collision with actors
	@param actorIx Current process actor index */
int32 checkCollisionWithActors(int32 actorIdx) {
	int32 a, xLeft, xRight, yLeft, yRight, zLeft, zRight;
	ActorStruct *actor, *actorTest;

	actor = &sceneActors[actorIdx];

	xLeft  = processActorX + actor->boudingBox.X.bottomLeft;
	xRight = processActorX + actor->boudingBox.X.topRight;

	yLeft  = processActorY + actor->boudingBox.Y.bottomLeft;
	yRight = processActorY + actor->boudingBox.Y.topRight;

	zLeft  = processActorZ + actor->boudingBox.Z.bottomLeft;
	zRight = processActorZ + actor->boudingBox.Z.topRight;

	actor->collision = -1;

	for (a = 0; a < sceneNumActors; a++) {
		actorTest = &sceneActors[a];

		// aviod current processed actor
		if (a != actorIdx && actorTest->entity != -1 && !actor->staticFlags.bComputeLowCollision && actorTest->standOn != actorIdx) {
			int32 xLeftTest, xRightTest, yLeftTest, yRightTest, zLeftTest, zRightTest;

			xLeftTest  = actorTest->X + actorTest->boudingBox.X.bottomLeft;
			xRightTest = actorTest->X + actorTest->boudingBox.X.topRight;

			yLeftTest  = actorTest->Y + actorTest->boudingBox.Y.bottomLeft;
			yRightTest = actorTest->Y + actorTest->boudingBox.Y.topRight;

			zLeftTest  = actorTest->Z + actorTest->boudingBox.Z.bottomLeft;
			zRightTest = actorTest->Z + actorTest->boudingBox.Z.topRight;

			if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
				actor->collision = a; // mark as collision with actor a

				if (actorTest->staticFlags.bIsCarrierActor) {
					if (actor->dynamicFlags.bIsFalling) {
						processActorY = yRightTest - actor->boudingBox.Y.bottomLeft + 1;
						actor->standOn = a;
					} else {
						if (standingOnActor(actorIdx, a)) {
							processActorY = yRightTest - actor->boudingBox.Y.bottomLeft + 1;
							actor->standOn = a;
						} else {
							int32 newAngle;

							newAngle = getAngleAndSetTargetActorDistance(processActorX, processActorZ, actorTest->X, actorTest->Z);

							if (actorTest->staticFlags.bCanBePushed && !actor->staticFlags.bCanBePushed) {
								actorTest->lastY = 0;

								if (actorTest->staticFlags.bUseMiniZv) {
									if (newAngle >= 0x80 && newAngle < 0x180 && actor->angle > 0x80 && actor->angle < 0x180) {
										actorTest->lastX = 192;
									}
									if (newAngle >= 0x180 && newAngle < 0x280 && actor->angle > 0x180 && actor->angle < 0x280) {
										actorTest->lastZ = -64;
									}
									if (newAngle >= 0x280 && newAngle < 0x380 && actor->angle > 0x280 && actor->angle < 0x380) {
										actorTest->lastX = -64;
									}
									if ((newAngle >= 0x380 || newAngle < 0x80) && (actor->angle > 0x380 || actor->angle < 0x80)) {
										actorTest->lastX = 192;
									}
								} else {
									actorTest->lastX = processActorX - actor->collisionX;
									actorTest->lastZ = processActorZ - actor->collisionZ;
								}
							}

							if ((actorTest->boudingBox.X.topRight - actorTest->boudingBox.X.bottomLeft == actorTest->boudingBox.Z.topRight - actorTest->boudingBox.Z.bottomLeft) &&
								(actor->boudingBox.X.topRight - actor->boudingBox.X.bottomLeft == actor->boudingBox.Z.topRight - actor->boudingBox.Z.bottomLeft)) {
								if (newAngle < 0x180) {
									processActorX = xLeftTest - actor->boudingBox.X.topRight;
								}
								if (newAngle >= 0x180 && newAngle < 0x280) {
									processActorZ = zRightTest - actor->boudingBox.Z.bottomLeft;
								}
								if (newAngle >= 0x280 && newAngle < 0x380) {
									processActorX = xRightTest - actor->boudingBox.X.bottomLeft;
								}
								if (newAngle >= 0x380 || (newAngle < 0x380 && newAngle < 0x80)) {
									processActorZ = zLeftTest - actor->boudingBox.Z.topRight;
								}
							} else {
								if (!actor->dynamicFlags.bIsFalling) {
									processActorX = previousActorX;
									processActorY = previousActorY;
									processActorZ = previousActorZ;
								}
							}
						}
					}
				} else {
					int32 newAngle;

					if (standingOnActor(actorIdx, a)) {
						hitActor(actorIdx, a, 1, -1);
					}

					newAngle = getAngleAndSetTargetActorDistance(processActorX, processActorZ, actorTest->X, actorTest->Z);

					if (actorTest->staticFlags.bCanBePushed && !actor->staticFlags.bCanBePushed) {
						actorTest->lastY = 0;

						if (actorTest->staticFlags.bUseMiniZv) {
							if (newAngle >= 0x80 && newAngle < 0x180 && actor->angle > 0x80 && actor->angle < 0x180) {
								actorTest->lastX = 192;
							}
							if (newAngle >= 0x180 && newAngle < 0x280 && actor->angle > 0x180 && actor->angle < 0x280) {
								actorTest->lastZ = -64;
							}
							if (newAngle >= 0x280 && newAngle < 0x380 && actor->angle > 0x280 && actor->angle < 0x380) {
								actorTest->lastX = -64;
							}
							if ((newAngle >= 0x380 || newAngle < 0x80) && (actor->angle > 0x380 || actor->angle < 0x80)) {
								actorTest->lastX = 192;
							}
						} else {
							actorTest->lastX = processActorX - actor->collisionX;
							actorTest->lastZ = processActorZ - actor->collisionZ;
						}
					}

					if ((actorTest->boudingBox.X.topRight - actorTest->boudingBox.X.bottomLeft == actorTest->boudingBox.Z.topRight - actorTest->boudingBox.Z.bottomLeft) &&
						(actor->boudingBox.X.topRight - actor->boudingBox.X.bottomLeft == actor->boudingBox.Z.topRight - actor->boudingBox.Z.bottomLeft)) {
						if (newAngle < 0x180) {
							processActorX = xLeftTest - actor->boudingBox.X.topRight;
						}
						if (newAngle >= 0x180 && newAngle < 0x280) {
							processActorZ = zRightTest - actor->boudingBox.Z.bottomLeft;
						}
						if (newAngle >= 0x280 && newAngle < 0x380) {
							processActorX = xRightTest - actor->boudingBox.X.bottomLeft;
						}
						if (newAngle >= 0x380 || (newAngle < 0x380 && newAngle < 0x80)) {
							processActorZ = zLeftTest - actor->boudingBox.Z.topRight;
						}
					} else {
						if (!actor->dynamicFlags.bIsFalling) {
							processActorX = previousActorX;
							processActorY = previousActorY;
							processActorZ = previousActorZ;
						}
					}
				}
			}
		}
	}

	if (actor->dynamicFlags.bIsHitting) {
   		rotateActor(0, 200, actor->angle);

		xLeft  = destX + processActorX + actor->boudingBox.X.bottomLeft;
		xRight = destX + processActorX + actor->boudingBox.X.topRight;

		yLeft  = processActorY + actor->boudingBox.Y.bottomLeft;
		yRight = processActorY + actor->boudingBox.Y.topRight;

		zLeft  = destZ + processActorZ + actor->boudingBox.Z.bottomLeft;
		zRight = destZ + processActorZ + actor->boudingBox.Z.topRight;

		for (a = 0; a < sceneNumActors; a++) {
			actorTest = &sceneActors[a];

			// aviod current processed actor
			if (a != actorIdx && actorTest->entity != -1 && !actorTest->staticFlags.bIsHidden && actorTest->standOn != actorIdx) {
				int32 xLeftTest, xRightTest, yLeftTest, yRightTest, zLeftTest, zRightTest;

				xLeftTest  = actorTest->X + actorTest->boudingBox.X.bottomLeft;
				xRightTest = actorTest->X + actorTest->boudingBox.X.topRight;

				yLeftTest  = actorTest->Y + actorTest->boudingBox.Y.bottomLeft;
				yRightTest = actorTest->Y + actorTest->boudingBox.Y.topRight;

				zLeftTest  = actorTest->Z + actorTest->boudingBox.Z.bottomLeft;
				zRightTest = actorTest->Z + actorTest->boudingBox.Z.topRight;

				if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
					hitActor(actorIdx, a, actor->strengthOfHit, actor->angle + 0x200);
					actor->dynamicFlags.bIsHitting = 0;
				}
			}
		}
	}

	return actor->collision;
}

/** Check Hero collision with bricks
	@param X Hero X coordinate
	@param Y Hero Y coordinate
	@param Z Hero Z coordinate
	@param damageMask Cause damage mask */
void checkHeroCollisionWithBricks(int32 X, int32 Y, int32 Z, int32 damageMask) {
	int32 brickShape;

	brickShape = getBrickShape(processActorX, processActorY, processActorZ);

	processActorX += X;
	processActorY += Y;
	processActorZ += Z;

	if (processActorX >= 0 && processActorZ >= 0 && processActorX <= 0x7E00 && processActorZ <= 0x7E00) {
		reajustActorPosition(brickShape);
		brickShape = getBrickShapeFull(processActorX, processActorY, processActorZ, processActorPtr->boudingBox.Y.topRight);

		if (brickShape == kSolid) {
			causeActorDamage |= damageMask;
			brickShape = getBrickShapeFull(processActorX, processActorY, previousActorZ + Z, processActorPtr->boudingBox.Y.topRight);

			if (brickShape == kSolid) {
				brickShape = getBrickShapeFull(X + previousActorX, processActorY, processActorZ, processActorPtr->boudingBox.Y.topRight);

				if (brickShape != kSolid) {
					processCollisionX = previousActorX;
				}
			} else {
				processCollisionZ = previousActorZ;
			}
		}
	}

	processActorX = processCollisionX;
	processActorY = processCollisionY;
	processActorZ = processCollisionZ;
}

/** Check other actor collision with bricks
	@param X Actor X coordinate
	@param Y Actor Y coordinate
	@param Z Actor Z coordinate
	@param damageMask Cause damage mask */
void checkActorCollisionWithBricks(int32 X, int32 Y, int32 Z, int32 damageMask) {
	int32 brickShape;

	brickShape = getBrickShape(processActorX, processActorY, processActorZ);

	processActorX += X;
	processActorY += Y;
	processActorZ += Z;

	if (processActorX >= 0 && processActorZ >= 0 && processActorX <= 0x7E00 && processActorZ <= 0x7E00) {
		reajustActorPosition(brickShape);
		brickShape = getBrickShape(processActorX, processActorY, processActorZ);

		if (brickShape == kSolid) {
			causeActorDamage |= damageMask;
			brickShape = getBrickShape(processActorX, processActorY, previousActorZ + Z);

			if (brickShape == kSolid) {
				brickShape = getBrickShape(X + previousActorX, processActorY, processActorZ);

				if (brickShape != kSolid) {
					processCollisionX = previousActorX;
				}
			} else {
				processCollisionZ = previousActorZ;
			}
		}
	}

	processActorX = processCollisionX;
	processActorY = processCollisionY;
	processActorZ = processCollisionZ;
}

/** Make actor to stop falling */
void stopFalling() { // ReceptionObj()
	int32 fall;

	if (currentlyProcessedActorIdx == 0) {
		fall = heroYBeforeFall - processActorY;

		if (fall >= 0x1000) {
			addExtraSpecial(processActorPtr->X, processActorPtr->Y + 1000, processActorPtr->Z, kHitStars);
			processActorPtr->life--;
			initAnim(kLandingHit, 2, 0, currentlyProcessedActorIdx);
		} else if (fall >= 0x800) {
			addExtraSpecial(processActorPtr->X, processActorPtr->Y + 1000, processActorPtr->Z, kHitStars);
			processActorPtr->life--;
			initAnim(kLandingHit, 2, 0, currentlyProcessedActorIdx);
		} else if (fall > 10) {
			initAnim(kLanding, 2, 0, currentlyProcessedActorIdx);
		} else {
			initAnim(kStanding, 0, 0, currentlyProcessedActorIdx);
		}

		heroYBeforeFall = 0;
	} else {
		initAnim(kLanding, 2, processActorPtr->animExtra, currentlyProcessedActorIdx);
	}

	processActorPtr->dynamicFlags.bIsFalling = 0;
}

/** Check extra collision with actors
	@param extra to process
	@param actorIdx actor to check collision */
int32 checkExtraCollisionWithActors(ExtraListStruct* extra, int32 actorIdx) {
	int32 a;
	int32 xLeft, xRight, yLeft, yRight, zLeft, zRight;
	int16 * spriteBounding;
	ActorStruct *actorTest;

	spriteBounding = (int16*)(spriteBoundingBoxPtr + extra->info0 * 16 + 4);

	xLeft  = *(spriteBounding++) + extra->X;
	xRight = *(spriteBounding++) + extra->X;

	yLeft  = *(spriteBounding++) + extra->Y;
	yRight = *(spriteBounding++) + extra->Y;

	zLeft  = *(spriteBounding++) + extra->Z;
	zRight = *(spriteBounding++) + extra->Z;

	for (a = 0; a < sceneNumActors; a++) {
		actorTest = &sceneActors[a];

		if (a != actorIdx && actorTest->entity != -1) {
			int32 xLeftTest, xRightTest, yLeftTest, yRightTest, zLeftTest, zRightTest;

			xLeftTest  = actorTest->X + actorTest->boudingBox.X.bottomLeft;
			xRightTest = actorTest->X + actorTest->boudingBox.X.topRight;

			yLeftTest  = actorTest->Y + actorTest->boudingBox.Y.bottomLeft;
			yRightTest = actorTest->Y + actorTest->boudingBox.Y.topRight;

			zLeftTest  = actorTest->Z + actorTest->boudingBox.Z.bottomLeft;
			zRightTest = actorTest->Z + actorTest->boudingBox.Z.topRight;

			if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
				if (extra->strengthOfHit != 0) {
					hitActor(actorIdx, a, extra->strengthOfHit, -1);
				}

				return a;
			}
		}
	}

	return -1;
}

/** Check extra collision with bricks */
int32 checkExtraCollisionWithBricks(int32 X, int32 Y, int32 Z, int32 oldX, int32 oldY, int32 oldZ) {
	int32 averageX, averageY, averageZ;

	if (getBrickShape(oldX, oldY, oldZ)) {
		return 1;
	}

	averageX = Abs(X + oldX)/2;
	averageY = Abs(Y + oldY)/2;
	averageZ = Abs(Z + oldZ)/2;

	if (getBrickShape(averageX, averageY, averageZ)) {
		return 1;
	}

	if (getBrickShape(Abs(oldX + averageX)/2, Abs(oldY + averageY)/2, Abs(oldZ + averageZ)/2)) {
		return 1;
	}

	if (getBrickShape(Abs(X + averageX)/2, Abs(Y + averageY)/2, Abs(Z + averageZ)/2)) {
		return 1;
	}

	return 0;
}

/** Check extra collision with another extra
	@param extra to process
	@param extraIdx extra index to check collision */
int32 checkExtraCollisionWithExtra(ExtraListStruct* extra, int32 extraIdx) {
	int32 i;
	int32 xLeft, xRight, yLeft, yRight, zLeft, zRight;
	int16 * spriteBounding;

	spriteBounding = (int16*)(spriteBoundingBoxPtr + extra->info0 * 16 + 4);

	xLeft  = *(spriteBounding++) + extra->X;
	xRight = *(spriteBounding++) + extra->X;

	yLeft  = *(spriteBounding++) + extra->Y;
	yRight = *(spriteBounding++) + extra->Y;

	zLeft  = *(spriteBounding++) + extra->Z;
	zRight = *(spriteBounding++) + extra->Z;

    for (i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extraTest = &extraList[i];
		if ( i != extraIdx && extraTest->info0 != -1) {
			int32 xLeftTest, xRightTest, yLeftTest, yRightTest, zLeftTest, zRightTest;
//            int16 * spriteBoundingTest;
//	        spriteBoundingTest = (int16*)(spriteBoundingBoxPtr + extraTest->info0 * 16 + 4);

			xLeftTest  = *(spriteBounding++) + extraTest->X;
	        xRightTest = *(spriteBounding++) + extraTest->X;

	        yLeftTest  = *(spriteBounding++) + extraTest->Y;
	        yRightTest = *(spriteBounding++) + extraTest->Y;

	        zLeftTest  = *(spriteBounding++) + extraTest->Z;
	        zRightTest = *(spriteBounding++) + extraTest->Z;

            if (xLeft < xLeftTest) {
			    if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
				    return i;
			    }
            }
		}
	}

	return -1;
}
