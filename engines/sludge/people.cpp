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

#include "sludge/allfiles.h"
#include "sludge/floor.h"
#include "sludge/graphics.h"
#include "sludge/loadsave.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/sound.h"
#include "sludge/speech.h"
#include "sludge/sprbanks.h"
#include "sludge/sprites.h"
#include "sludge/variable.h"
#include "sludge/version.h"
#include "sludge/zbuffer.h"

#define ANGLEFIX (180.0 / 3.14157)
#define ANI_STAND 0
#define ANI_WALK 1
#define ANI_TALK 2

namespace Sludge {

extern VariableStack *noStack;

extern int ssgVersion;

ScreenRegion personRegion;
extern ScreenRegion *lastRegion;
extern Floor *currentFloor;

OnScreenPerson *allPeople = NULL;
int16 scaleHorizon = 75;
int16 scaleDivide = 150;
extern ScreenRegion *allScreenRegions;

void setFrames(OnScreenPerson &m, int a) {
	m.myAnim = m.myPersona->animation[(a * m.myPersona->numDirections) + m.direction];
}

PersonaAnimation  *createPersonaAnim(int num, VariableStack *&stacky) {
	PersonaAnimation  *newP = new PersonaAnimation ;
	checkNew(newP);

	newP->numFrames = num;
	newP->frames = new AnimFrame [num];
	checkNew(newP->frames);

	int a = num, frameNum, howMany;

	while (a) {
		a--;
		newP->frames[a].noise = 0;
		if (stacky->thisVar.varType == SVT_FILE) {
			newP->frames[a].noise = stacky->thisVar.varData.intValue;
		} else if (stacky->thisVar.varType == SVT_FUNC) {
			newP->frames[a].noise = -stacky->thisVar.varData.intValue;
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

PersonaAnimation  *makeNullAnim() {
	PersonaAnimation  *newAnim = new PersonaAnimation ;
	if (!checkNew(newAnim))
		return NULL;

	newAnim->theSprites = NULL;
	newAnim->numFrames = 0;
	newAnim->frames = NULL;
	return newAnim;
}

PersonaAnimation  *copyAnim(PersonaAnimation  *orig) {
	int num = orig->numFrames;

	PersonaAnimation  *newAnim = new PersonaAnimation ;
	if (!checkNew(newAnim))
		return NULL;

	// Copy the easy bits...
	newAnim->theSprites = orig->theSprites;
	newAnim->numFrames = num;

	if (num) {

		// Argh!Frames!We need a whole NEW array of AnimFrame  structures...

		newAnim->frames = new AnimFrame [num];
		if (!checkNew(newAnim->frames))
			return NULL;

		for (int a = 0; a < num; a++) {
			newAnim->frames[a].frameNum = orig->frames[a].frameNum;
			newAnim->frames[a].howMany = orig->frames[a].howMany;
			newAnim->frames[a].noise = orig->frames[a].noise;
		}
	} else {
		newAnim->frames = NULL;
	}

	return newAnim;
}

void deleteAnim(PersonaAnimation  *orig) {

	if (orig) {
		if (orig->numFrames) {
			delete[] orig->frames;
		}
		delete orig;
		orig = NULL;
	}
}

void turnMeAngle(OnScreenPerson *thisPerson, int direc) {
	int d = thisPerson->myPersona->numDirections;
	thisPerson->angle = direc;
	direc += (180 / d) + 180 + thisPerson->angleOffset;
	while (direc >= 360)
		direc -= 360;
	thisPerson->direction = (direc * d) / 360;
}

bool initPeople() {
	personRegion.sX = 0;
	personRegion.sY = 0;
	personRegion.di = -1;
	allScreenRegions = NULL;

	return true;
}

void spinStep(OnScreenPerson *thisPerson) {
	int diff = (thisPerson->angle + 360) - thisPerson->wantAngle;
	int eachSlice = thisPerson->spinSpeed ? thisPerson->spinSpeed : (360 / thisPerson->myPersona->numDirections);
	while (diff > 180) {
		diff -= 360;
	}

	if (diff >= eachSlice) {
		turnMeAngle(thisPerson, thisPerson->angle - eachSlice);
	} else if (diff <= -eachSlice) {
		turnMeAngle(thisPerson, thisPerson->angle + eachSlice);
	} else {
		turnMeAngle(thisPerson, thisPerson->wantAngle);
		thisPerson->spinning = false;
	}
}

void rethinkAngle(OnScreenPerson *thisPerson) {
	int d = thisPerson->myPersona->numDirections;
	int direc = thisPerson->angle + (180 / d) + 180 + thisPerson->angleOffset;
	while (direc >= 360)
		direc -= 360;
	thisPerson->direction = (direc * d) / 360;
}

bool turnPersonToFace(int thisNum, int direc) {
	OnScreenPerson *thisPerson = findPerson(thisNum);
	if (thisPerson) {
		if (thisPerson->continueAfterWalking)
			abortFunction(thisPerson->continueAfterWalking);
		thisPerson->continueAfterWalking = NULL;
		thisPerson->walking = false;
		thisPerson->spinning = false;
		turnMeAngle(thisPerson, direc);
		setFrames(*thisPerson, g_sludge->_speechMan->isCurrentTalker(thisPerson) ? ANI_TALK : ANI_STAND);
		return true;
	}
	return false;
}

bool setPersonExtra(int thisNum, int extra) {
	OnScreenPerson *thisPerson = findPerson(thisNum);
	if (thisPerson) {
		thisPerson->extra = extra;
		if (extra & EXTRA_NOSCALE)
			thisPerson->scale = 1;
		return true;
	}
	return false;
}

void setScale(int16 h, int16 d) {
	scaleHorizon = h;
	scaleDivide = d;
}

void moveAndScale(OnScreenPerson &me, float x, float y) {
	me.x = x;
	me.y = y;
	if (!(me.extra & EXTRA_NOSCALE) && scaleDivide)
		me.scale = (me.y - scaleHorizon) / scaleDivide;
}

OnScreenPerson *findPerson(int v) {
	OnScreenPerson *thisPerson = allPeople;
	while (thisPerson) {
		if (v == thisPerson->thisType->objectNum)
			break;
		thisPerson = thisPerson->next;
	}
	return thisPerson;
}

void movePerson(int x, int y, int objNum) {
	OnScreenPerson *moveMe = findPerson(objNum);
	if (moveMe)
		moveAndScale(*moveMe, x, y);
}

void setShown(bool h, int ob) {
	OnScreenPerson *moveMe = findPerson(ob);
	if (moveMe)
		moveMe->show = h;
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

void setMyDrawMode(OnScreenPerson *moveMe, int h) {
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
		case drawModeShadow1:
			moveMe->r = moveMe->g = moveMe->b = 0;
			moveMe->colourmix = 255;
			moveMe->transparency = 64;
			break;
		case drawModeShadow2:
			moveMe->r = moveMe->g = moveMe->b = 0;
			moveMe->colourmix = 255;
			moveMe->transparency = 128;
			break;
		case drawModeShadow3:
			moveMe->r = moveMe->g = moveMe->b = 0;
			moveMe->colourmix = 255;
			moveMe->transparency = 192;
			break;
		case drawModeFoggy3:
			moveMe->r = moveMe->g = moveMe->b = 128;
			moveMe->colourmix = 192;
			moveMe->transparency = 0;
			break;
		case drawModeFoggy2:
			moveMe->r = moveMe->g = moveMe->b = 128;
			moveMe->colourmix = 128;
			moveMe->transparency = 0;
			break;
		case drawModeFoggy1:
			moveMe->r = moveMe->g = moveMe->b = 128;
			moveMe->colourmix = 64;
			moveMe->transparency = 0;
			break;
		case drawModeFoggy4:
			moveMe->r = moveMe->g = moveMe->b = 128;
			moveMe->colourmix = 255;
			moveMe->transparency = 0;
			break;
		case drawModeGlow3:
			moveMe->r = moveMe->g = moveMe->b = 255;
			moveMe->colourmix = 192;
			moveMe->transparency = 0;
			break;
		case drawModeGlow2:
			moveMe->r = moveMe->g = moveMe->b = 255;
			moveMe->colourmix = 128;
			moveMe->transparency = 0;
			break;
		case drawModeGlow1:
			moveMe->r = moveMe->g = moveMe->b = 255;
			moveMe->colourmix = 64;
			moveMe->transparency = 0;
			break;
		case drawModeGlow4:
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
	OnScreenPerson *moveMe = findPerson(ob);
	if (!moveMe)
		return;

	setMyDrawMode(moveMe, h);
}

void setPersonTransparency(int ob, byte x) {
	OnScreenPerson *moveMe = findPerson(ob);
	if (!moveMe)
		return;

	if (x > 254)
		x = 254;
	moveMe->transparency = x;
}

void setPersonColourise(int ob, byte r, byte g, byte b, byte colourmix) {
	OnScreenPerson *moveMe = findPerson(ob);
	if (!moveMe)
		return;

	moveMe->r = r;
	moveMe->g = g;
	moveMe->b = b;
	moveMe->colourmix = colourmix;
}

extern ScreenRegion *overRegion;

void shufflePeople() {
	OnScreenPerson **thisReference = &allPeople;
	OnScreenPerson *A, *B;

	if (!allPeople)
		return;

	while ((*thisReference)->next) {
		float y1 = (*thisReference)->y;
		if ((*thisReference)->extra & EXTRA_FRONT)
			y1 += 1000;

		float y2 = (*thisReference)->next->y;
		if ((*thisReference)->next->extra & EXTRA_FRONT)
			y2 += 1000;

		if (y1 > y2) {
			A = (*thisReference);
			B = (*thisReference)->next;
			A->next = B->next;
			B->next = A;
			(*thisReference) = B;
		} else {
			thisReference = &((*thisReference)->next);
		}
	}
}

void drawPeople() {
	shufflePeople();

	OnScreenPerson *thisPerson = allPeople;
	PersonaAnimation  *myAnim = NULL;
	overRegion = NULL;

	while (thisPerson) {
		if (thisPerson->show) {
			myAnim = thisPerson->myAnim;
			if (myAnim != thisPerson->lastUsedAnim) {
				thisPerson->lastUsedAnim = myAnim;
				thisPerson->frameNum = 0;
				thisPerson->frameTick = myAnim->frames[0].howMany;
				if (myAnim->frames[thisPerson->frameNum].noise > 0) {
					g_sludge->_soundMan->startSound(myAnim->frames[thisPerson->frameNum].noise, false);
					thisPerson->frameNum++;
					thisPerson->frameNum %= thisPerson->myAnim->numFrames;
					thisPerson->frameTick = thisPerson->myAnim->frames[thisPerson->frameNum].howMany;
				} else if (myAnim->frames[thisPerson->frameNum].noise) {
					startNewFunctionNum(-myAnim->frames[thisPerson->frameNum].noise, 0,
					NULL, noStack);
					thisPerson->frameNum++;
					thisPerson->frameNum %= thisPerson->myAnim->numFrames;
					thisPerson->frameTick = thisPerson->myAnim->frames[thisPerson->frameNum].howMany;
				}
			}
			int fNumSign = myAnim->frames[thisPerson->frameNum].frameNum;
			int m = fNumSign < 0;
			int fNum = ABS(fNumSign);
			if (fNum >= myAnim->theSprites->bank.total) {
				fNum = 0;
				m = 2 - m;
			}
			if (m != 2) {
				bool r = false;
				r = g_sludge->_gfxMan->scaleSprite(myAnim->theSprites->bank.sprites[fNum], myAnim->theSprites->bank.myPalette, thisPerson, m);
				if (r) {
					if (!thisPerson->thisType->screenName.empty()) {
						if (personRegion.thisType != thisPerson->thisType)
							lastRegion = NULL;
						personRegion.thisType = thisPerson->thisType;
						overRegion = &personRegion;
					}
				}
			}
		}
		if (!--thisPerson->frameTick) {
			thisPerson->frameNum++;
			thisPerson->frameNum %= thisPerson->myAnim->numFrames;
			thisPerson->frameTick = thisPerson->myAnim->frames[thisPerson->frameNum].howMany;
			if (thisPerson->show && myAnim && myAnim->frames) {
				if (myAnim->frames[thisPerson->frameNum].noise > 0) {
					g_sludge->_soundMan->startSound(myAnim->frames[thisPerson->frameNum].noise, false);
					thisPerson->frameNum++;
					thisPerson->frameNum %= thisPerson->myAnim->numFrames;
					thisPerson->frameTick = thisPerson->myAnim->frames[thisPerson->frameNum].howMany;
				} else if (myAnim->frames[thisPerson->frameNum].noise) {
					startNewFunctionNum(-myAnim->frames[thisPerson->frameNum].noise, 0,
					NULL, noStack);
					thisPerson->frameNum++;
					thisPerson->frameNum %= thisPerson->myAnim->numFrames;
					thisPerson->frameTick = thisPerson->myAnim->frames[thisPerson->frameNum].howMany;
				}
			}
		}
		thisPerson = thisPerson->next;
	}
}

void makeTalker(OnScreenPerson &me) {
	setFrames(me, ANI_TALK);
}

void makeSilent(OnScreenPerson &me) {
	setFrames(me, ANI_STAND);
}

bool handleClosestPoint(int &setX, int &setY, int &setPoly) {
	int gotX = 320, gotY = 200, gotPoly = -1, i, j, xTest1, yTest1, xTest2, yTest2, closestX, closestY, oldJ, currentDistance = 0xFFFFF, thisDistance;

//	FILE * dbug = fopen ("debug_closest.txt", "at");
//	fprintf (dbug, "\nGetting closest point to %i, %i\n", setX, setY);

	for (i = 0; i < currentFloor->numPolygons; i++) {
		oldJ = currentFloor->polygon[i].numVertices - 1;
		for (j = 0; j < currentFloor->polygon[i].numVertices; j++) {
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

	if (gotPoly == -1)
		return false;
	setX = gotX;
	setY = gotY;
	setPoly = gotPoly;

	return true;
}

bool doBorderStuff(OnScreenPerson *moveMe) {
	if (moveMe->inPoly == moveMe->walkToPoly) {
		moveMe->inPoly = -1;
		moveMe->thisStepX = moveMe->walkToX;
		moveMe->thisStepY = moveMe->walkToY;
	} else {
		// The section in which we need to be next...
		int newPoly = currentFloor->matrix[moveMe->inPoly][moveMe->walkToPoly];
		if (newPoly == -1)
			return false;

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

			if (sqrt((double)dx13 + dy13) + sqrt((double)dx23 + dy23) < sqrt((double)dx14 + dy14) + sqrt((double)dx24 + dy24)) {
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

	setFrames(*moveMe, ANI_WALK);
	return true;
}

bool walkMe(OnScreenPerson *thisPerson, bool move = true) {
	float xDiff, yDiff, maxDiff, s;

	for (;;) {
		xDiff = thisPerson->thisStepX - thisPerson->x;
		yDiff = (thisPerson->thisStepY - thisPerson->y) * 2;
		s = thisPerson->scale * thisPerson->walkSpeed;
		if (s < 0.2)
			s = 0.2;

		maxDiff = (ABS(xDiff) >= ABS(yDiff)) ? ABS(xDiff) : ABS(yDiff);

		if (ABS(maxDiff) > s) {
			if (thisPerson->spinning) {
				spinStep(thisPerson);
				setFrames(*thisPerson, ANI_WALK);
			}
			s = maxDiff / s;
			if (move)
				moveAndScale(*thisPerson, thisPerson->x + xDiff / s, thisPerson->y + yDiff / (s * 2));
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
		if (!doBorderStuff(thisPerson))
			break;
	}

	thisPerson->walking = false;
	setFrames(*thisPerson, ANI_STAND);
	moveAndScale(*thisPerson, thisPerson->walkToX, thisPerson->walkToY);
	return false;
}

bool makeWalkingPerson(int x, int y, int objNum, LoadedFunction *func, int di) {
	if (x == 0 && y == 0)
		return false;
	if (currentFloor->numPolygons == 0)
		return false;
	OnScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe)
		return false;

	if (moveMe->continueAfterWalking)
		abortFunction(moveMe->continueAfterWalking);
	moveMe->continueAfterWalking = NULL;
	moveMe->walking = true;
	moveMe->directionWhenDoneWalking = di;

	moveMe->walkToX = x;
	moveMe->walkToY = y;
	moveMe->walkToPoly = inFloor(x, y);
	if (moveMe->walkToPoly == -1) {
		if (!handleClosestPoint(moveMe->walkToX, moveMe->walkToY, moveMe->walkToPoly))
			return false;
	}

	moveMe->inPoly = inFloor(moveMe->x, moveMe->y);
	if (moveMe->inPoly == -1) {
		int xxx = moveMe->x, yyy = moveMe->y;
		if (!handleClosestPoint(xxx, yyy, moveMe->inPoly))
			return false;
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
	OnScreenPerson *moveMe = findPerson(o);
	if (moveMe)
		if (moveMe->continueAfterWalking) {
			abortFunction(moveMe->continueAfterWalking);
			moveMe->continueAfterWalking = NULL;
			moveMe->walking = false;
			moveMe->spinning = false;
			setFrames(*moveMe, ANI_STAND);
			return true;
		}
	return false;
}

bool forceWalkingPerson(int x, int y, int objNum, LoadedFunction *func, int di) {
	if (x == 0 && y == 0)
		return false;
	OnScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe)
		return false;

	if (moveMe->continueAfterWalking)
		abortFunction(moveMe->continueAfterWalking);
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
	if (x == 0 && y == 0)
		return;
	OnScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe)
		return;
	if (moveMe->continueAfterWalking)
		abortFunction(moveMe->continueAfterWalking);
	moveMe->continueAfterWalking = NULL;
	moveMe->walking = false;
	moveMe->spinning = false;
	moveAndScale(*moveMe, x, y);
}

bool floatCharacter(int f, int objNum) {
	OnScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe)
		return false;
	moveMe->floaty = f;
	return true;
}

bool setCharacterWalkSpeed(int f, int objNum) {
	if (f <= 0)
		return false;
	OnScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe)
		return false;
	moveMe->walkSpeed = f;
	return true;
}

void walkAllPeople() {
	OnScreenPerson *thisPerson = allPeople;

	while (thisPerson) {
		if (thisPerson->walking) {
			walkMe(thisPerson);
		} else if (thisPerson->spinning) {
			spinStep(thisPerson);
			setFrames(*thisPerson, ANI_STAND);
		}
		if ((!thisPerson->walking) && (!thisPerson->spinning) && thisPerson->continueAfterWalking) {
			restartFunction(thisPerson->continueAfterWalking);
			thisPerson->continueAfterWalking = NULL;
		}
		thisPerson = thisPerson->next;
	}
}

bool addPerson(int x, int y, int objNum, Persona *p) {
	OnScreenPerson *newPerson = new OnScreenPerson;
	if (!checkNew(newPerson))
		return false;

	// EASY STUFF
	newPerson->thisType = g_sludge->_objMan->loadObjectType(objNum);
	newPerson->scale = 1;
	newPerson->extra = 0;
	newPerson->continueAfterWalking = NULL;
	moveAndScale(*newPerson, x, y);
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
	newPerson->lastUsedAnim = 0;
	newPerson->frameTick = 0;

	setFrames(*newPerson, ANI_STAND);

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
	OnScreenPerson **changethat = &allPeople;

	while (((*changethat) != NULL) && ((*changethat)->y < y))
		changethat = &((*changethat)->next);

	newPerson->next = (*changethat);
	(*changethat) = newPerson;

	return (bool)(newPerson->thisType != NULL);
}

int timeForAnim(PersonaAnimation  *fram) {
	int total = 0;
	for (int a = 0; a < fram->numFrames; a++) {
		total += fram->frames[a].howMany;
	}
	return total;
}

void animatePerson(int obj, PersonaAnimation  *fram) { // Set a new SINGLE animation
	OnScreenPerson *moveMe = findPerson(obj);
	if (moveMe) {
		if (moveMe->continueAfterWalking)
			abortFunction(moveMe->continueAfterWalking);
		moveMe->continueAfterWalking = NULL;
		moveMe->walking = false;
		moveMe->spinning = false;
		moveMe->myAnim = fram;
	}
}

void animatePerson(int obj, Persona *per) {             // Set a new costume
	OnScreenPerson *moveMe = findPerson(obj);
	if (moveMe) {
		//  if (moveMe->continueAfterWalking) abortFunction (moveMe->continueAfterWalking);
		//  moveMe->continueAfterWalking = NULL;
		//  moveMe->walking = false;
		moveMe->spinning = false;
		moveMe->myPersona = per;
		rethinkAngle(moveMe);
		if (moveMe->walking) {
			setFrames(*moveMe, ANI_WALK);
		} else {
			setFrames(*moveMe, ANI_STAND);
		}
	}
}

void killAllPeople() {
	OnScreenPerson *killPeople;
	while (allPeople) {
		if (allPeople->continueAfterWalking)
			abortFunction(allPeople->continueAfterWalking);
		allPeople->continueAfterWalking = NULL;
		killPeople = allPeople;
		allPeople = allPeople->next;
		g_sludge->_objMan->removeObjectType(killPeople->thisType);
		delete killPeople;
	}
}

void killMostPeople() {
	OnScreenPerson *killPeople;
	OnScreenPerson **lookyHere = &allPeople;

	while (*lookyHere) {
		if ((*lookyHere)->extra & EXTRA_NOREMOVE) {
			lookyHere = &(*lookyHere)->next;
		} else {
			killPeople = (*lookyHere);

			// Change last pointer to NEXT in the list instead
			(*lookyHere) = killPeople->next;

			// Gone from the list... now free some memory
			if (killPeople->continueAfterWalking)
				abortFunction(killPeople->continueAfterWalking);
			killPeople->continueAfterWalking = NULL;
			g_sludge->_objMan->removeObjectType(killPeople->thisType);
			delete killPeople;
		}
	}
}

void removeOneCharacter(int i) {
	OnScreenPerson *p = findPerson(i);

	if (p) {
		if (overRegion == &personRegion && overRegion->thisType == p->thisType) {
			overRegion = NULL;
		}

		if (p->continueAfterWalking)
			abortFunction(p->continueAfterWalking);
		p->continueAfterWalking = NULL;
		OnScreenPerson **killPeople;

		for (killPeople = &allPeople; *killPeople != p; killPeople = &((*killPeople)->next)) {
			;
		}

		*killPeople = p->next;
		g_sludge->_objMan->removeObjectType(p->thisType);
		delete p;
	}
}

bool saveAnim(PersonaAnimation  *p, Common::WriteStream *stream) {
	stream->writeUint16BE(p->numFrames);
	if (p->numFrames) {
		stream->writeUint32LE(p->theSprites->ID);

		for (int a = 0; a < p->numFrames; a++) {
			stream->writeUint32LE(p->frames[a].frameNum);
			stream->writeUint32LE(p->frames[a].howMany);
			stream->writeUint32LE(p->frames[a].noise);
		}
	}
	return true;
}

bool loadAnim(PersonaAnimation  *p, Common::SeekableReadStream *stream) {
	p->numFrames = stream->readUint16BE();

	if (p->numFrames) {
		int a = stream->readUint32LE();
		p->frames = new AnimFrame [p->numFrames];
		if (!checkNew(p->frames))
			return false;
		p->theSprites = g_sludge->_gfxMan->loadBankForAnim(a);

		for (a = 0; a < p->numFrames; a++) {
			p->frames[a].frameNum = stream->readUint32LE();
			p->frames[a].howMany = stream->readUint32LE();
			if (ssgVersion >= VERSION(2, 0)) {
				p->frames[a].noise = stream->readUint32LE();
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

bool saveCostume(Persona *cossy, Common::WriteStream *stream) {
	int a;
	stream->writeUint16BE(cossy->numDirections);
	for (a = 0; a < cossy->numDirections * 3; a++) {
		if (!saveAnim(cossy->animation[a], stream))
			return false;
	}
//	debugCostume ("Saved", cossy);
	return true;
}

bool loadCostume(Persona *cossy, Common::SeekableReadStream *stream) {
	int a;
	cossy->numDirections = stream->readUint16BE();
	cossy->animation = new PersonaAnimation  *[cossy->numDirections * 3];
	if (!checkNew(cossy->animation))
		return false;
	for (a = 0; a < cossy->numDirections * 3; a++) {
		cossy->animation[a] = new PersonaAnimation ;
		if (!checkNew(cossy->animation[a]))
			return false;

		if (!loadAnim(cossy->animation[a], stream))
			return false;
	}
//	debugCostume ("Loaded", cossy);
	return true;
}

bool savePeople(Common::WriteStream *stream) {
	OnScreenPerson *me = allPeople;
	int countPeople = 0, a;

	stream->writeSint16LE(scaleHorizon);
	stream->writeSint16LE(scaleDivide);

	while (me) {
		countPeople++;
		me = me->next;
	}

	stream->writeUint16BE(countPeople);

	me = allPeople;
	for (a = 0; a < countPeople; a++) {

		stream->writeFloatLE(me->x);
		stream->writeFloatLE(me->y);

		saveCostume(me->myPersona, stream);
		saveAnim(me->myAnim, stream);
		stream->writeByte(me->myAnim == me->lastUsedAnim);

		stream->writeFloatLE(me->scale);

		stream->writeUint16BE(me->extra);
		stream->writeUint16BE(me->height);
		stream->writeUint16BE(me->walkToX);
		stream->writeUint16BE(me->walkToY);
		stream->writeUint16BE(me->thisStepX);
		stream->writeUint16BE(me->thisStepY);
		stream->writeUint16BE(me->frameNum);
		stream->writeUint16BE(me->frameTick);
		stream->writeUint16BE(me->walkSpeed);
		stream->writeUint16BE(me->spinSpeed);
		stream->writeSint16LE(me->floaty);
		stream->writeByte(me->show);
		stream->writeByte(me->walking);
		stream->writeByte(me->spinning);
		if (me->continueAfterWalking) {
			stream->writeByte(1);
			saveFunction(me->continueAfterWalking, stream);
		} else {
			stream->writeByte(0);
		}
		stream->writeUint16BE(me->direction);
		stream->writeUint16BE(me->angle);
		stream->writeUint16BE(me->angleOffset);
		stream->writeUint16BE(me->wantAngle);
		stream->writeSint16LE(me->directionWhenDoneWalking);
		stream->writeSint16LE(me->inPoly);
		stream->writeSint16LE(me->walkToPoly);

		stream->writeByte(me->r);
		stream->writeByte(me->g);
		stream->writeByte(me->b);
		stream->writeByte(me->colourmix);
		stream->writeByte(me->transparency);

		g_sludge->_objMan->saveObjectRef(me->thisType, stream);

		me = me->next;
	}
	return true;
}

bool loadPeople(Common::SeekableReadStream *stream) {
	OnScreenPerson **pointy = &allPeople;
	OnScreenPerson *me;

	scaleHorizon = stream->readSint16LE();
	scaleDivide = stream->readSint16LE();

	int countPeople = stream->readUint16BE();
	int a;

	allPeople = NULL;
	for (a = 0; a < countPeople; a++) {
		me = new OnScreenPerson;
		if (!checkNew(me))
			return false;

		me->myPersona = new Persona;
		if (!checkNew(me->myPersona))
			return false;

		me->myAnim = new PersonaAnimation ;
		if (!checkNew(me->myAnim))
			return false;

		me->x = stream->readFloatLE();
		me->y = stream->readFloatLE();

		loadCostume(me->myPersona, stream);
		loadAnim(me->myAnim, stream);

		me->lastUsedAnim = stream->readByte() ? me->myAnim : NULL;

		me->scale = stream->readFloatLE();

		me->extra = stream->readUint16BE();
		me->height = stream->readUint16BE();
		me->walkToX = stream->readUint16BE();
		me->walkToY = stream->readUint16BE();
		me->thisStepX = stream->readUint16BE();
		me->thisStepY = stream->readUint16BE();
		me->frameNum = stream->readUint16BE();
		me->frameTick = stream->readUint16BE();
		me->walkSpeed = stream->readUint16BE();
		me->spinSpeed = stream->readUint16BE();
		me->floaty = stream->readSint16LE();
		me->show = stream->readByte();
		me->walking = stream->readByte();
		me->spinning = stream->readByte();
		if (stream->readByte()) {
			me->continueAfterWalking = loadFunction(stream);
			if (!me->continueAfterWalking)
				return false;
		} else {
			me->continueAfterWalking = NULL;
		}
		me->direction = stream->readUint16BE();
		me->angle = stream->readUint16BE();
		if (ssgVersion >= VERSION(2, 0)) {
			me->angleOffset = stream->readUint16BE();
		} else {
			me->angleOffset = 0;
		}
		me->wantAngle = stream->readUint16BE();
		me->directionWhenDoneWalking = stream->readSint16LE();
		me->inPoly = stream->readSint16LE();
		me->walkToPoly = stream->readSint16LE();
		if (ssgVersion >= VERSION(2, 0)) {
			me->r = stream->readByte();
			me->g = stream->readByte();
			me->b = stream->readByte();
			me->colourmix = stream->readByte();
			me->transparency = stream->readByte();
		} else {
			setMyDrawMode(me, stream->readUint16BE());
		}
		me->thisType = g_sludge->_objMan->loadObjectRef(stream);

		// Anti-aliasing settings
		if (ssgVersion >= VERSION(1, 6)) {
			if (ssgVersion < VERSION(2, 0)) {
				// aaLoad
				stream->readByte();
				stream->readFloatLE();
				stream->readFloatLE();
			}
		}

		me->next = NULL;
		*pointy = me;
		pointy = &(me->next);
	}
//	db ("End of loadPeople");
	return true;
}

} // End of namespace Sludge
