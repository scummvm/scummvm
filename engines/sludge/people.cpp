/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "allfiles.h"
#include <math.h>
#include <stdlib.h>

#include "CommonCode/version.h"
#include "sprites.h"
#include "sprbanks.h"
#include "sludger.h"
#include "objtypes.h"
#include "region.h"
#include "people.h"
#include "talk.h"
#include "newfatal.h"
#include "variable.h"
#include "moreio.h"
#include "loadsave.h"
#include "floor.h"
#include "zbuffer.h"
#include "sound.h"
#include "debug.h"

#define ANGLEFIX (180.0 / 3.14157)
#define ANI_STAND 0
#define ANI_WALK 1
#define ANI_TALK 2

namespace Sludge {

extern speechStruct *speech;

extern variableStack *noStack;

extern int ssgVersion;

extern int cameraX, cameraY;
screenRegion personRegion;
extern screenRegion *lastRegion;
extern flor *currentFloor;

extern inputType input;
onScreenPerson *allPeople = NULL;
short int scaleHorizon = 75;
short int scaleDivide = 150;
extern screenRegion *allScreenRegions;

#define TF_max(a, b) ((a > b) ? a : b)

inline int TF_abs(int a) {
	return (a > 0) ? a : -a;
}

void setFrames(onScreenPerson &m, int a) {
	m.myAnim = m.myPersona->animation[(a * m.myPersona->numDirections) + m.direction];
}

personaAnimation *createPersonaAnim(int num, variableStack *&stacky) {
	personaAnimation *newP = new personaAnimation;
	checkNew(newP);

	newP->numFrames = num;
	newP->frames = new animFrame[num];
	checkNew(newP->frames);

	int a = num, frameNum, howMany;

	while (a) {
		a --;
		newP->frames[a].noise = 0;
		if (stacky->thisVar.varType == SVT_FILE) {
			newP->frames[a].noise = stacky->thisVar.varData.intValue;
		} else if (stacky->thisVar.varType == SVT_FUNC) {
			newP->frames[a].noise = - stacky->thisVar.varData.intValue;
		} else if (stacky->thisVar.varType == SVT_STACK) {
			getValueType(frameNum, SVT_INT, stacky->thisVar.varData.theStack->first->thisVar);
			getValueType(howMany, SVT_INT, stacky->thisVar.varData.theStack->first->next->thisVar);
		} else {
			getValueType(frameNum, SVT_INT, stacky->thisVar);
			howMany = 1;
		}
		trimStack(stacky);
		newP->frames[a].frameNum = frameNum;
		newP->frames[a].howMany = howMany;
	}

	return newP;
}

personaAnimation *makeNullAnim() {
	personaAnimation *newAnim  = new personaAnimation;
	if (!checkNew(newAnim)) return NULL;


	newAnim->theSprites       = NULL;
	newAnim->numFrames        = 0;
	newAnim->frames           = NULL;
	return newAnim;
}

personaAnimation *copyAnim(personaAnimation *orig) {
	int num = orig->numFrames;

	personaAnimation *newAnim  = new personaAnimation;
	if (!checkNew(newAnim)) return NULL;

	// Copy the easy bits...
	newAnim->theSprites       = orig->theSprites;
	newAnim->numFrames        = num;

	if (num) {

		// Argh!Frames!We need a whole NEW array of animFrame structures...

		newAnim->frames = new animFrame[num];
		if (!checkNew(newAnim->frames)) return NULL;

		for (int a = 0; a < num; ++a) {
			newAnim->frames[a].frameNum = orig->frames[a].frameNum;
			newAnim->frames[a].howMany = orig->frames[a].howMany;
			newAnim->frames[a].noise = orig->frames[a].noise;
		}
	} else {
		newAnim->frames = NULL;
	}

	return newAnim;
}

void deleteAnim(personaAnimation *orig) {

	if (orig) {
		if (orig->numFrames) {
			delete[] orig->frames;
		}
		delete orig;
		orig = NULL;
	}
}

void turnMeAngle(onScreenPerson *thisPerson, int direc) {
	int d = thisPerson->myPersona->numDirections;
	thisPerson->angle = direc;
	direc += (180 / d) + 180 + thisPerson->angleOffset;
	while (direc >= 360) direc -= 360;
	thisPerson->direction = (direc * d) / 360;
}

bool initPeople() {
	personRegion.sX = 0;
	personRegion.sY = 0;
	personRegion.di = -1;
	allScreenRegions = NULL;

	return true;
}

void spinStep(onScreenPerson *thisPerson) {
	int diff = (thisPerson->angle + 360) - thisPerson->wantAngle;
	int eachSlice = thisPerson->spinSpeed ? thisPerson->spinSpeed : (360 / thisPerson->myPersona->numDirections);
	while (diff > 180) {
		diff -= 360;
	}

	if (diff >= eachSlice) {
		turnMeAngle(thisPerson, thisPerson->angle - eachSlice);
	} else if (diff <= - eachSlice) {
		turnMeAngle(thisPerson, thisPerson->angle + eachSlice);
	} else {
		turnMeAngle(thisPerson, thisPerson->wantAngle);
		thisPerson->spinning = false;
	}
}

void rethinkAngle(onScreenPerson *thisPerson) {
	int d = thisPerson->myPersona->numDirections;
	int direc = thisPerson->angle + (180 / d) + 180 + thisPerson->angleOffset;
	while (direc >= 360) direc -= 360;
	thisPerson->direction = (direc * d) / 360;
}

bool turnPersonToFace(int thisNum, int direc) {
	onScreenPerson *thisPerson = findPerson(thisNum);
	if (thisPerson) {
		if (thisPerson->continueAfterWalking) abortFunction(thisPerson->continueAfterWalking);
		thisPerson->continueAfterWalking = NULL;
		thisPerson->walking = false;
		thisPerson->spinning = false;
		turnMeAngle(thisPerson, direc);
		setFrames(* thisPerson, (thisPerson == speech->currentTalker) ? ANI_TALK : ANI_STAND);
		return true;
	}
	return false;
}

bool setPersonExtra(int thisNum, int extra) {
	onScreenPerson *thisPerson = findPerson(thisNum);
	if (thisPerson) {
		thisPerson->extra = extra;
		if (extra & EXTRA_NOSCALE) thisPerson->scale = 1;
		return true;
	}
	return false;
}

void setScale(short int h, short int d) {
	scaleHorizon = h;
	scaleDivide = d;
}

void moveAndScale(onScreenPerson &me, float x, float y) {
	me.x = x;
	me.y = y;
	if (!(me.extra & EXTRA_NOSCALE) && scaleDivide) me.scale = (me.y - scaleHorizon) / scaleDivide;
}

onScreenPerson *findPerson(int v) {
	onScreenPerson *thisPerson = allPeople;
	while (thisPerson) {
		if (v == thisPerson->thisType->objectNum) break;
		thisPerson = thisPerson->next;
	}
	return thisPerson;
}

void movePerson(int x, int y, int objNum) {
	onScreenPerson *moveMe = findPerson(objNum);
	if (moveMe) moveAndScale(* moveMe, x, y);
}

void setShown(bool h, int ob) {
	onScreenPerson *moveMe = findPerson(ob);
	if (moveMe) moveMe->show = h;
}

enum drawModes {
	drawModeNormal,
	drawModeTransparent1,
	drawModeTransparent2,
	drawModeTransparent3,
	drawModeDark1,
	drawModeDark2,
	drawModeDark3,
	drawModeBlack,
	drawModeShadow1,
	drawModeShadow2,
	drawModeShadow3,
	drawModeFoggy1,
	drawModeFoggy2,
	drawModeFoggy3,
	drawModeFoggy4,
	drawModeGlow1,
	drawModeGlow2,
	drawModeGlow3,
	drawModeGlow4,
	drawModeInvisible,
	numDrawModes
};

void setMyDrawMode(onScreenPerson *moveMe, int h) {
	switch (h) {
	case drawModeTransparent3:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 0;
		moveMe->transparency = 64;
		break;
	case drawModeTransparent2:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 0;
		moveMe->transparency = 128;
		break;
	case drawModeTransparent1:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 0;
		moveMe->transparency = 192;
		break;
	case drawModeInvisible:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 0;
		moveMe->transparency = 254;
		break;
	case drawModeDark1:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 192;
		moveMe->transparency = 0;
		break;
	case drawModeDark2:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 128;
		moveMe->transparency = 0;
		break;
	case drawModeDark3:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 64;
		moveMe->transparency = 0;
		break;
	case drawModeBlack:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 255;
		moveMe->transparency = 0;
		break;
	case    drawModeShadow1:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 255;
		moveMe->transparency = 64;
		break;
	case    drawModeShadow2:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 255;
		moveMe->transparency = 128;
		break;
	case    drawModeShadow3:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 255;
		moveMe->transparency = 192;
		break;
	case    drawModeFoggy3:
		moveMe->r = moveMe->g = moveMe->b = 128;
		moveMe->colourmix = 192;
		moveMe->transparency = 0;
		break;
	case    drawModeFoggy2:
		moveMe->r = moveMe->g = moveMe->b = 128;
		moveMe->colourmix = 128;
		moveMe->transparency = 0;
		break;
	case    drawModeFoggy1:
		moveMe->r = moveMe->g = moveMe->b = 128;
		moveMe->colourmix = 64;
		moveMe->transparency = 0;
		break;
	case    drawModeFoggy4:
		moveMe->r = moveMe->g = moveMe->b = 128;
		moveMe->colourmix = 255;
		moveMe->transparency = 0;
		break;
	case    drawModeGlow3:
		moveMe->r = moveMe->g = moveMe->b = 255;
		moveMe->colourmix = 192;
		moveMe->transparency = 0;
		break;
	case    drawModeGlow2:
		moveMe->r = moveMe->g = moveMe->b = 255;
		moveMe->colourmix = 128;
		moveMe->transparency = 0;
		break;
	case    drawModeGlow1:
		moveMe->r = moveMe->g = moveMe->b = 255;
		moveMe->colourmix = 64;
		moveMe->transparency = 0;
		break;
	case    drawModeGlow4:
		moveMe->r = moveMe->g = moveMe->b = 255;
		moveMe->colourmix = 255;
		moveMe->transparency = 0;
		break;
	default:
		moveMe->r = moveMe->g = moveMe->b = 0;
		moveMe->colourmix = 0;
		moveMe->transparency = 0;
		break;
	}

}

void setDrawMode(int h, int ob) {
	onScreenPerson *moveMe = findPerson(ob);
	if (!moveMe) return;

	setMyDrawMode(moveMe, h);
}

void setPersonTransparency(int ob, unsigned char x) {
	onScreenPerson *moveMe = findPerson(ob);
	if (!moveMe) return;

	if (x > 254) x = 254;
	moveMe->transparency = x;
}

void setPersonColourise(int ob, unsigned char r, unsigned char g, unsigned char b, unsigned char colourmix) {
	onScreenPerson *moveMe = findPerson(ob);
	if (!moveMe) return;

	moveMe->r = r;
	moveMe->g = g;
	moveMe->b = b;
	moveMe->colourmix = colourmix;
}



extern screenRegion *overRegion;

void shufflePeople() {
	onScreenPerson * * thisReference = & allPeople;
	onScreenPerson *A, * B;

	if (!allPeople) return;

	while ((* thisReference)->next) {
		float y1 = (* thisReference)->y;
		if ((* thisReference)->extra & EXTRA_FRONT) y1 += 1000;

		float y2 = (* thisReference)->next->y;
		if ((* thisReference)->next->extra & EXTRA_FRONT) y2 += 1000;

		if (y1 > y2) {
			A = (* thisReference);
			B = (* thisReference)->next;
			A->next = B->next;
			B->next = A;
			(* thisReference) = B;
		} else {
			thisReference = & ((* thisReference)->next);
		}
	}
}



void drawPeople() {
	shufflePeople();

	onScreenPerson *thisPerson = allPeople;
	personaAnimation *myAnim = NULL;
	overRegion = NULL;

	while (thisPerson) {
		if (thisPerson->show) {
			myAnim = thisPerson->myAnim;
			if (myAnim != thisPerson->lastUsedAnim) {
				thisPerson->lastUsedAnim = myAnim;
				thisPerson->frameNum = 0;
				thisPerson->frameTick = myAnim->frames[0].howMany;
				if (myAnim->frames[thisPerson->frameNum].noise > 0) {
					startSound(myAnim->frames[thisPerson->frameNum].noise, false);
					thisPerson->frameNum ++;
					thisPerson->frameNum %= thisPerson->myAnim->numFrames;
					thisPerson->frameTick = thisPerson->myAnim->frames[thisPerson->frameNum].howMany;
				} else if (myAnim->frames[thisPerson->frameNum].noise) {
					startNewFunctionNum(- myAnim->frames[thisPerson->frameNum].noise, 0, NULL, noStack);
					thisPerson->frameNum ++;
					thisPerson->frameNum %= thisPerson->myAnim->numFrames;
					thisPerson->frameTick = thisPerson->myAnim->frames[thisPerson->frameNum].howMany;
				}
			}
			int fNumSign = myAnim->frames[thisPerson->frameNum].frameNum;
			int m = fNumSign < 0;
			int fNum = abs(fNumSign);
			if (fNum >= myAnim->theSprites->bank.total) {
				fNum = 0;
				m = 2 - m;
			}
			if (m != 2) {
				bool r = false;
				r = scaleSprite(myAnim->theSprites->bank.sprites[fNum], myAnim->theSprites->bank.myPalette, thisPerson, m);
				if (r) {
					if (thisPerson->thisType->screenName[0]) {
						if (personRegion.thisType != thisPerson->thisType) lastRegion = NULL;
						personRegion.thisType = thisPerson->thisType;
						overRegion = & personRegion;
					}
				}
			}
		}
		if (!-- thisPerson->frameTick) {
			thisPerson->frameNum ++;
			thisPerson->frameNum %= thisPerson->myAnim->numFrames;
			thisPerson->frameTick = thisPerson->myAnim->frames[thisPerson->frameNum].howMany;
			if (thisPerson->show && myAnim && myAnim->frames) {
				if (myAnim->frames[thisPerson->frameNum].noise > 0) {
					startSound(myAnim->frames[thisPerson->frameNum].noise, false);
					thisPerson->frameNum ++;
					thisPerson->frameNum %= thisPerson->myAnim->numFrames;
					thisPerson->frameTick = thisPerson->myAnim->frames[thisPerson->frameNum].howMany;
				} else if (myAnim->frames[thisPerson->frameNum].noise) {
					startNewFunctionNum(- myAnim->frames[thisPerson->frameNum].noise, 0, NULL, noStack);
					thisPerson->frameNum ++;
					thisPerson->frameNum %= thisPerson->myAnim->numFrames;
					thisPerson->frameTick = thisPerson->myAnim->frames[thisPerson->frameNum].howMany;
				}
			}
		}
		thisPerson = thisPerson->next;
	}
}

void makeTalker(onScreenPerson &me) {
	setFrames(me, ANI_TALK);
}

void makeSilent(onScreenPerson &me) {
	setFrames(me, ANI_STAND);
}

bool handleClosestPoint(int &setX, int &setY, int &setPoly) {
	int gotX = 320, gotY = 200, gotPoly = -1, i, j, xTest1, yTest1,
	    xTest2, yTest2, closestX, closestY, oldJ, currentDistance = 0xFFFFF,
	                                              thisDistance;

//	FILE * dbug = fopen ("debug_closest.txt", "at");
//	fprintf (dbug, "\nGetting closest point to %i, %i\n", setX, setY);

	for (i = 0; i < currentFloor->numPolygons; i ++) {
		oldJ = currentFloor->polygon[i].numVertices - 1;
		for (j = 0; j < currentFloor->polygon[i].numVertices; j ++) {
//			fprintf (dbug, "Polygon %i, line %i... ", i, j);
			xTest1 = currentFloor->vertex[currentFloor->polygon[i].vertexID[j]].x;
			yTest1 = currentFloor->vertex[currentFloor->polygon[i].vertexID[j]].y;
			xTest2 = currentFloor->vertex[currentFloor->polygon[i].vertexID[oldJ]].x;
			yTest2 = currentFloor->vertex[currentFloor->polygon[i].vertexID[oldJ]].y;
			closestPointOnLine(closestX, closestY, xTest1, yTest1, xTest2, yTest2, setX, setY);
//			fprintf (dbug, "closest point is %i, %i... ", closestX, closestY);
			xTest1 = setX - closestX;
			yTest1 = setY - closestY;
			thisDistance = xTest1 * xTest1 + yTest1 * yTest1;
//			fprintf (dbug, "Distance squared %i\n", thisDistance);

			if (thisDistance < currentDistance) {
//				fprintf (dbug, "** We have a new winner!**\n");

				currentDistance = thisDistance;
				gotX = closestX;
				gotY = closestY;
				gotPoly = i;
			}
			oldJ = j;
		}
	}
//	fclose (dbug);

	if (gotPoly == -1) return false;
	setX = gotX;
	setY = gotY;
	setPoly = gotPoly;

	return true;
}

bool doBorderStuff(onScreenPerson *moveMe) {
	if (moveMe->inPoly == moveMe->walkToPoly) {
		moveMe->inPoly = -1;
		moveMe->thisStepX = moveMe->walkToX;
		moveMe->thisStepY = moveMe->walkToY;
	} else {
		// The section in which we need to be next...
		int newPoly = currentFloor->matrix[moveMe->inPoly][moveMe->walkToPoly];
		if (newPoly == -1) return false;

		// Grab the index of the second matching corner...
		int ID, ID2;
		if (!getMatchingCorners(currentFloor->polygon[moveMe->inPoly], currentFloor->polygon[newPoly], ID, ID2))
			return fatal("Not a valid floor plan!");

		// Remember that we're walking to the new polygon...
		moveMe->inPoly = newPoly;

		// Calculate the destination position on the coincidantal line...
		int x1 = moveMe->x, y1 = moveMe->y;
		int x2 = moveMe->walkToX, y2 = moveMe->walkToY;
		int x3 = currentFloor->vertex[ID].x, y3 = currentFloor->vertex[ID].y;
		int x4 = currentFloor->vertex[ID2].x, y4 = currentFloor->vertex[ID2].y;

		int xAB = x1 - x2;
		int yAB = y1 - y2;
		int xCD = x4 - x3;
		int yCD = y4 - y3;

		double m = (yAB * (x3 - x1) - xAB * (y3 - y1));
		m /= ((xAB * yCD) - (yAB * xCD));

		if (m > 0 && m < 1) {
			moveMe->thisStepX = x3 + m * xCD;
			moveMe->thisStepY = y3 + m * yCD;
		} else {
			int dx13 = x1 - x3, dx14 = x1 - x4, dx23 = x2 - x3, dx24 = x2 - x4;
			int dy13 = y1 - y3, dy14 = y1 - y4, dy23 = y2 - y3, dy24 = y2 - y4;

			dx13 *= dx13;
			dx14 *= dx14;
			dx23 *= dx23;
			dx24 *= dx24;
			dy13 *= dy13;
			dy14 *= dy14;
			dy23 *= dy23;
			dy24 *= dy24;

			if (sqrt((double) dx13 + dy13) + sqrt((double) dx23 + dy23) <
			        sqrt((double) dx14 + dy14) + sqrt((double) dx24 + dy24)) {
				moveMe->thisStepX = x3;
				moveMe->thisStepY = y3;
			} else {
				moveMe->thisStepX = x4;
				moveMe->thisStepY = y4;
			}
		}
	}

	float yDiff = moveMe->thisStepY - moveMe->y;
	float xDiff = moveMe->x - moveMe->thisStepX;
	if (xDiff || yDiff) {
		moveMe->wantAngle = 180 + ANGLEFIX * atan2(xDiff, yDiff * 2);
		moveMe->spinning = true;
	}

	setFrames(* moveMe, ANI_WALK);
	return true;
}

bool walkMe(onScreenPerson *thisPerson, bool move = true) {
	float xDiff, yDiff, maxDiff, s;

	for (;;) {
		xDiff = thisPerson->thisStepX - thisPerson->x;
		yDiff = (thisPerson->thisStepY - thisPerson->y) * 2;
		s = thisPerson->scale * thisPerson->walkSpeed;
		if (s < 0.2) s = 0.2;

		maxDiff = (TF_abs(xDiff) >= TF_abs(yDiff)) ? TF_abs(xDiff) : TF_abs(yDiff);

		if (TF_abs(maxDiff) > s) {
			if (thisPerson->spinning) {
				spinStep(thisPerson);
				setFrames(* thisPerson, ANI_WALK);
			}
			s = maxDiff / s;
			if (move)
				moveAndScale(* thisPerson,
				             thisPerson->x + xDiff / s,
				             thisPerson->y + yDiff / (s * 2));
			return true;
		}

		if (thisPerson->inPoly == -1) {
			if (thisPerson->directionWhenDoneWalking != -1) {
				thisPerson->wantAngle = thisPerson->directionWhenDoneWalking;
				thisPerson->spinning = true;
				spinStep(thisPerson);
			}
			break;
		}
		if (!doBorderStuff(thisPerson)) break;
	}

	thisPerson->walking = false;
	setFrames(* thisPerson, ANI_STAND);
	moveAndScale(* thisPerson,
	             thisPerson->walkToX,
	             thisPerson->walkToY);
	return false;
}

bool makeWalkingPerson(int x, int y, int objNum, loadedFunction *func, int di) {
	if (x == 0 && y == 0) return false;
	if (currentFloor->numPolygons == 0) return false;
	onScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe) return false;

	if (moveMe->continueAfterWalking) abortFunction(moveMe->continueAfterWalking);
	moveMe->continueAfterWalking = NULL;
	moveMe->walking = true;
	moveMe->directionWhenDoneWalking = di;

	moveMe->walkToX = x;
	moveMe->walkToY = y;
	moveMe->walkToPoly = inFloor(x, y);
	if (moveMe->walkToPoly == -1) {
		if (!handleClosestPoint(moveMe->walkToX, moveMe->walkToY, moveMe->walkToPoly)) return false;
	}

	moveMe->inPoly = inFloor(moveMe->x, moveMe->y);
	if (moveMe->inPoly == -1) {
		int xxx = moveMe->x, yyy = moveMe->y;
		if (!handleClosestPoint(xxx, yyy, moveMe->inPoly)) return false;
	}

	doBorderStuff(moveMe);
	if (walkMe(moveMe, false) || moveMe->spinning) {
		moveMe->continueAfterWalking = func;
		return true;
	} else {
		return false;
	}
}

bool stopPerson(int o) {
	onScreenPerson *moveMe = findPerson(o);
	if (moveMe)
		if (moveMe->continueAfterWalking) {
			abortFunction(moveMe->continueAfterWalking);
			moveMe->continueAfterWalking = NULL;
			moveMe->walking = false;
			moveMe->spinning = false;
			setFrames(* moveMe, ANI_STAND);
			return true;
		}
	return false;
}

bool forceWalkingPerson(int x, int y, int objNum, loadedFunction *func, int di) {
	if (x == 0 && y == 0) return false;
	onScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe) return false;

	if (moveMe->continueAfterWalking) abortFunction(moveMe->continueAfterWalking);
	moveMe->walking = true;
	moveMe->continueAfterWalking = NULL;
	moveMe->directionWhenDoneWalking = di;

	moveMe->walkToX = x;
	moveMe->walkToY = y;

	// Let's pretend the start and end points are both in the same
	// polygon (which one isn't important)
	moveMe->inPoly = 0;
	moveMe->walkToPoly = 0;

	doBorderStuff(moveMe);
	if (walkMe(moveMe) || moveMe->spinning) {
		moveMe->continueAfterWalking = func;
		return true;
	} else {
		return false;
	}
}

void jumpPerson(int x, int y, int objNum) {
	if (x == 0 && y == 0) return;
	onScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe) return;
	if (moveMe->continueAfterWalking) abortFunction(moveMe->continueAfterWalking);
	moveMe->continueAfterWalking = NULL;
	moveMe->walking = false;
	moveMe->spinning = false;
	moveAndScale(* moveMe, x, y);
}

bool floatCharacter(int f, int objNum) {
	onScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe) return false;
	moveMe->floaty = f;
	return true;
}

bool setCharacterWalkSpeed(int f, int objNum) {
	if (f <= 0) return false;
	onScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe) return false;
	moveMe->walkSpeed = f;
	return true;
}

void walkAllPeople() {
	onScreenPerson *thisPerson = allPeople;

	while (thisPerson) {
		if (thisPerson->walking) {
			walkMe(thisPerson);
		} else if (thisPerson->spinning) {
			spinStep(thisPerson);
			setFrames(* thisPerson, ANI_STAND);
		}
		if ((!thisPerson->walking) && (!thisPerson->spinning) && thisPerson->continueAfterWalking) {
			restartFunction(thisPerson->continueAfterWalking);
			thisPerson->continueAfterWalking = NULL;
		}
		thisPerson = thisPerson->next;
	}
}

bool addPerson(int x, int y, int objNum, persona *p) {
	onScreenPerson *newPerson = new onScreenPerson;
	if (!checkNew(newPerson)) return false;

	// EASY STUFF
	newPerson->thisType = loadObjectType(objNum);
	newPerson->scale = 1;
	newPerson->extra = 0;
	newPerson->continueAfterWalking = NULL;
	moveAndScale(* newPerson, x, y);
	newPerson->frameNum = 0;
	newPerson->walkToX = x;
	newPerson->walkToY = y;
	newPerson->walking = false;
	newPerson->spinning = false;
	newPerson->show = true;
	newPerson->direction = 0;
	newPerson->angle = 180;
	newPerson->wantAngle = 180;
	newPerson->angleOffset = 0;
	newPerson->floaty = 0;
	newPerson->walkSpeed = newPerson->thisType->walkSpeed;
	newPerson->myAnim = NULL;
	newPerson->spinSpeed = newPerson->thisType->spinSpeed;
	newPerson->r = 0;
	newPerson->g = 0;
	newPerson->b = 0;
	newPerson->colourmix = 0;
	newPerson->transparency = 0;
	newPerson->myPersona = p;

	setFrames(* newPerson, ANI_STAND);

	// HEIGHT (BASED ON 1st FRAME OF 1st ANIMATION... INC. SPECIAL CASES)
	int fNumSigned = p->animation[0]->frames[0].frameNum;
	int fNum = abs(fNumSigned);
	if (fNum >= p->animation[0]->theSprites->bank.total) {
		if (fNumSigned < 0) {
			newPerson->height = 5;
		} else {
			newPerson->height = p->animation[0]->theSprites->bank.sprites[0].yhot + 5;
		}
	} else {
		newPerson->height = p->animation[0]->theSprites->bank.sprites[fNum].yhot + 5;
	}

	// NOW ADD IT IN THE RIGHT PLACE
	onScreenPerson * * changethat = & allPeople;

	while (((* changethat) != NULL) && ((* changethat)->y < y))
		changethat = & ((* changethat)->next);

	newPerson->next = (* changethat);
	(* changethat) = newPerson;

	return (bool)(newPerson->thisType != NULL);
}

int timeForAnim(personaAnimation *fram) {
	int total = 0;
	for (int a = 0; a < fram->numFrames; ++a) {
		total += fram->frames[a].howMany;
	}
	return total;
}

void animatePerson(int obj, personaAnimation *fram) {   // Set a new SINGLE animation
	onScreenPerson *moveMe = findPerson(obj);
	if (moveMe) {
		if (moveMe->continueAfterWalking) abortFunction(moveMe->continueAfterWalking);
		moveMe->continueAfterWalking = NULL;
		moveMe->walking = false;
		moveMe->spinning = false;
		moveMe->myAnim = fram;
	}
}

void animatePerson(int obj, persona *per) {             // Set a new costume
	onScreenPerson *moveMe = findPerson(obj);
	if (moveMe) {
		//  if (moveMe->continueAfterWalking) abortFunction (moveMe->continueAfterWalking);
		//  moveMe->continueAfterWalking = NULL;
		//  moveMe->walking = false;
		moveMe->spinning = false;
		moveMe->myPersona = per;
		rethinkAngle(moveMe);
		if (moveMe-> walking) {
			setFrames(* moveMe, ANI_WALK);
		} else {
			setFrames(* moveMe, ANI_STAND);
		}
	}
}

void killAllPeople() {
	onScreenPerson *killPeople;
	while (allPeople) {
		if (allPeople->continueAfterWalking) abortFunction(allPeople->continueAfterWalking);
		allPeople->continueAfterWalking = NULL;
		killPeople = allPeople;
		allPeople = allPeople->next;
		removeObjectType(killPeople->thisType);
		delete killPeople;
	}
}

void killMostPeople() {
	onScreenPerson *killPeople;
	onScreenPerson * * lookyHere = & allPeople;

	while (* lookyHere) {
		if ((* lookyHere)->extra & EXTRA_NOREMOVE) {
			lookyHere = & (* lookyHere)->next;
		} else {
			killPeople = (* lookyHere);

			// Change last pointer to NEXT in the list instead
			(* lookyHere) = killPeople->next;

			// Gone from the list... now free some memory
			if (killPeople->continueAfterWalking) abortFunction(killPeople->continueAfterWalking);
			killPeople->continueAfterWalking = NULL;
			removeObjectType(killPeople->thisType);
			delete killPeople;
		}
	}
}

void removeOneCharacter(int i) {
	onScreenPerson *p = findPerson(i);

	if (p) {
		if (overRegion == &personRegion && overRegion->thisType == p->thisType) {
			overRegion = NULL;
		}

		if (p->continueAfterWalking) abortFunction(p->continueAfterWalking);
		p->continueAfterWalking = NULL;
		onScreenPerson * * killPeople;

		for (killPeople = & allPeople;
		        * killPeople != p;
		        killPeople = & ((* killPeople)->next)) {
			;
		}

		* killPeople = p->next;
		removeObjectType(p->thisType);
		delete p;
	}
}

bool saveAnim(personaAnimation *p, Common::WriteStream *stream) {
	put2bytes(p->numFrames, stream);
	if (p->numFrames) {
		put4bytes(p->theSprites->ID, stream);

		for (int a = 0; a < p->numFrames; ++a) {
			put4bytes(p->frames[a].frameNum, stream);
			put4bytes(p->frames[a].howMany, stream);
			put4bytes(p->frames[a].noise, stream);
		}
	}
	return true;
}

bool loadAnim(personaAnimation *p, Common::SeekableReadStream *stream) {
	p->numFrames = get2bytes(stream);

	if (p->numFrames) {
		int a = get4bytes(stream);
		p->frames = new animFrame[p->numFrames];
		if (!checkNew(p->frames)) return false;
		p->theSprites = loadBankForAnim(a);

		for (a = 0; a < p->numFrames; ++a) {
			p->frames[a].frameNum = get4bytes(stream);
			p->frames[a].howMany = get4bytes(stream);
			if (ssgVersion >= VERSION(2, 0)) {
				p->frames[a].noise = get4bytes(stream);
			} else {
				p->frames[a].noise = 0;
			}
		}
	} else {
		p->theSprites = NULL;
		p->frames = NULL;
	}
	return true;
}
/*
void debugCostume (char * message, persona * cossy) {
    FILE * db = fopen ("debuTURN.txt", "at");
    fprintf (db, "  %s costume with %i directions...\n", message, cossy->numDirections);
    for (int a = 0; a < cossy->numDirections * 3; ++a) {
        fprintf (db, "      %i frames:", cossy->animation[a]->numFrames);
        for (int b = 0; b < cossy->animation[a]->numFrames; b ++) {
            fprintf (db, " %i", cossy->animation[a]->frames[b]);
        }
        fprintf (db, "\n");

    }
    fclose (db);
}
*/
bool saveCostume(persona *cossy, Common::WriteStream *stream) {
	int a;
	put2bytes(cossy->numDirections, stream);
	for (a = 0; a < cossy->numDirections * 3; ++a) {
		if (!saveAnim(cossy->animation[a], stream)) return false;
	}
//	debugCostume ("Saved", cossy);
	return true;
}

bool loadCostume(persona *cossy, Common::SeekableReadStream *stream) {
	int a;
	cossy->numDirections = get2bytes(stream);
	cossy->animation = new personaAnimation * [cossy->numDirections * 3];
	if (!checkNew(cossy->animation)) return false;
	for (a = 0; a < cossy->numDirections * 3; ++a) {
		cossy->animation[a] = new personaAnimation;
		if (!checkNew(cossy->animation[a])) return false;

		if (!loadAnim(cossy->animation[a], stream)) return false;
	}
//	debugCostume ("Loaded", cossy);
	return true;
}

bool savePeople(Common::WriteStream *stream) {
	onScreenPerson *me = allPeople;
	int countPeople = 0, a;

	putSigned(scaleHorizon, stream);
	putSigned(scaleDivide, stream);

	while (me) {
		countPeople ++;
		me = me->next;
	}

	put2bytes(countPeople, stream);

	me = allPeople;
	for (a = 0; a < countPeople; ++a) {

		putFloat(me->x, stream);
		putFloat(me->y, stream);

		saveCostume(me->myPersona, stream);
		saveAnim(me->myAnim, stream);
		putch(me->myAnim == me->lastUsedAnim, stream);

		putFloat(me->scale, stream);

		put2bytes(me->extra, stream);
		put2bytes(me->height, stream);
		put2bytes(me->walkToX, stream);
		put2bytes(me->walkToY, stream);
		put2bytes(me->thisStepX, stream);
		put2bytes(me->thisStepY, stream);
		put2bytes(me->frameNum, stream);
		put2bytes(me->frameTick, stream);
		put2bytes(me->walkSpeed, stream);
		put2bytes(me->spinSpeed, stream);
		putSigned(me->floaty, stream);
		putch(me->show, stream);
		putch(me->walking, stream);
		putch(me->spinning, stream);
		if (me->continueAfterWalking) {
			putch(1, stream);
			saveFunction(me->continueAfterWalking, stream);
		} else {
			putch(0, stream);
		}
		put2bytes(me->direction, stream);
		put2bytes(me->angle, stream);
		put2bytes(me->angleOffset, stream);
		put2bytes(me->wantAngle, stream);
		putSigned(me->directionWhenDoneWalking, stream);
		putSigned(me->inPoly, stream);
		putSigned(me->walkToPoly, stream);

		putch(me->r, stream);
		putch(me->g, stream);
		putch(me->b, stream);
		putch(me->colourmix, stream);
		putch(me->transparency, stream);

		saveObjectRef(me->thisType, stream);

		me = me->next;
	}
	return true;
}

bool loadPeople(Common::SeekableReadStream *stream) {
	onScreenPerson * * pointy = & allPeople;
	onScreenPerson *me;

	scaleHorizon = getSigned(stream);
	scaleDivide = getSigned(stream);

	int countPeople = get2bytes(stream);
	int a;

	allPeople = NULL;
	for (a = 0; a < countPeople; ++a) {
		me = new onScreenPerson;
		if (!checkNew(me)) return false;

		me->myPersona = new persona;
		if (!checkNew(me->myPersona)) return false;

		me->myAnim = new personaAnimation;
		if (!checkNew(me->myAnim)) return false;

		me->x = getFloat(stream);
		me->y = getFloat(stream);

		loadCostume(me->myPersona, stream);
		loadAnim(me->myAnim, stream);

		me->lastUsedAnim = getch(stream) ? me->myAnim : NULL;

		me->scale = getFloat(stream);

		me->extra = get2bytes(stream);
		me->height = get2bytes(stream);
		me->walkToX = get2bytes(stream);
		me->walkToY = get2bytes(stream);
		me->thisStepX = get2bytes(stream);
		me->thisStepY = get2bytes(stream);
		me->frameNum = get2bytes(stream);
		me->frameTick = get2bytes(stream);
		me->walkSpeed = get2bytes(stream);
		me->spinSpeed = get2bytes(stream);
		me->floaty = getSigned(stream);
		me->show = getch(stream);
		me->walking = getch(stream);
		me->spinning = getch(stream);
		if (getch(stream)) {
			me->continueAfterWalking = loadFunction(stream);
			if (!me->continueAfterWalking) return false;
		} else {
			me->continueAfterWalking = NULL;
		}
		me->direction = get2bytes(stream);
		me->angle = get2bytes(stream);
		if (ssgVersion >= VERSION(2, 0)) {
			me->angleOffset = get2bytes(stream);
		} else {
			me->angleOffset = 0;
		}
		me->wantAngle = get2bytes(stream);
		me->directionWhenDoneWalking = getSigned(stream);
		me->inPoly = getSigned(stream);
		me->walkToPoly = getSigned(stream);
		if (ssgVersion >= VERSION(2, 0)) {
			me->r = getch(stream);
			me->g = getch(stream);
			me->b = getch(stream);
			me->colourmix = getch(stream);
			me->transparency = getch(stream);
		} else {
			setMyDrawMode(me, get2bytes(stream));
		}
		me->thisType = loadObjectRef(stream);

		// Anti-aliasing settings
		if (ssgVersion >= VERSION(1, 6)) {
			if (ssgVersion < VERSION(2, 0)) {
				// aaLoad
				getch(stream);
				getFloat(stream);
				getFloat(stream);
			}
		}

		me->next = NULL;
		* pointy = me;
		pointy = & (me->next);
	}
//	db ("End of loadPeople");
	return true;
}

} // End of namespace Sludge
