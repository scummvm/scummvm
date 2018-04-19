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
extern Floor *currentFloor;

PersonaAnimation::PersonaAnimation() {
	theSprites = nullptr;
	numFrames = 0;
	frames = nullptr;
}

PersonaAnimation::~PersonaAnimation() {
	if (numFrames) {
		delete[] frames;
		frames = nullptr;
	}
}

PersonaAnimation::PersonaAnimation(int num, VariableStack *&stacky) {
	theSprites = nullptr;
	numFrames = num;
	frames = new AnimFrame[num];
	int a = num, frameNum, howMany;

	while (a) {
		a--;
		frames[a].noise = 0;
		if (stacky->thisVar.varType == SVT_FILE) {
			frames[a].noise = stacky->thisVar.varData.intValue;
		} else if (stacky->thisVar.varType == SVT_FUNC) {
			frames[a].noise = -stacky->thisVar.varData.intValue;
		} else if (stacky->thisVar.varType == SVT_STACK) {
			getValueType(frameNum, SVT_INT, stacky->thisVar.varData.theStack->first->thisVar);
			getValueType(howMany, SVT_INT, stacky->thisVar.varData.theStack->first->next->thisVar);
		} else {
			getValueType(frameNum, SVT_INT, stacky->thisVar);
			howMany = 1;
		}
		trimStack(stacky);
		frames[a].frameNum = frameNum;
		frames[a].howMany = howMany;
	}
}

PersonaAnimation::PersonaAnimation(PersonaAnimation *orig) {
	int num = orig->numFrames;

	// Copy the easy bits...
	theSprites = orig->theSprites;
	numFrames = num;

	if (num) {
		// Argh! Frames! We need a whole NEW array of AnimFrame  structures...
		frames = new AnimFrame[num];

		for (int a = 0; a < num; a++) {
			frames[a].frameNum = orig->frames[a].frameNum;
			frames[a].howMany = orig->frames[a].howMany;
			frames[a].noise = orig->frames[a].noise;
		}
	} else {
		frames = nullptr;
	}
}

int PersonaAnimation::getTotalTime() {
	int total = 0;
	for (int a = 0; a < numFrames; a++) {
		total += frames[a].howMany;
	}
	return total;
}

bool PersonaAnimation::save(Common::WriteStream *stream) {
	stream->writeUint16BE(numFrames);
	if (numFrames) {
		stream->writeUint32LE(theSprites->ID);

		for (int a = 0; a < numFrames; a++) {
			stream->writeUint32LE(frames[a].frameNum);
			stream->writeUint32LE(frames[a].howMany);
			stream->writeUint32LE(frames[a].noise);
		}
	}
	return true;
}

bool PersonaAnimation::load(Common::SeekableReadStream *stream) {
	numFrames = stream->readUint16BE();

	if (numFrames) {
		int a = stream->readUint32LE();
		frames = new AnimFrame [numFrames];
		if (!checkNew(frames))
			return false;
		theSprites = g_sludge->_gfxMan->loadBankForAnim(a);

		for (a = 0; a < numFrames; a++) {
			frames[a].frameNum = stream->readUint32LE();
			frames[a].howMany = stream->readUint32LE();
			if (ssgVersion >= VERSION(2, 0)) {
				frames[a].noise = stream->readUint32LE();
			} else {
				frames[a].noise = 0;
			}
		}
	} else {
		theSprites = NULL;
		frames = NULL;
	}
	return true;
}

bool Persona::save(Common::WriteStream *stream) {
	int a;
	stream->writeUint16BE(numDirections);
	for (a = 0; a < numDirections * 3; a++) {
		if (!animation[a]->save(stream))
			return false;
	}
	return true;
}

bool Persona::load(Common::SeekableReadStream *stream) {
	int a;
	numDirections = stream->readUint16BE();
	animation = new PersonaAnimation *[numDirections * 3];
	if (!checkNew(animation))
		return false;
	for (a = 0; a < numDirections * 3; a++) {
		animation[a] = new PersonaAnimation;
		if (!checkNew(animation[a]))
			return false;

		if (!animation[a]->load(stream))
			return false;
	}
	return true;
}

void OnScreenPerson::setFrames(int a) {
	myAnim = myPersona->animation[(a * myPersona->numDirections) + direction];
}

void OnScreenPerson::makeTalker() {
	setFrames(ANI_TALK);
}

void OnScreenPerson::makeSilent() {
	setFrames(ANI_STAND);
}

PeopleManager::PeopleManager(SludgeEngine *vm) {
	_vm = vm;
	_allPeople = nullptr;
	_scaleHorizon = 75;
	_scaleDivide = 150;
	_personRegion = new ScreenRegion;
}

PeopleManager::~PeopleManager() {
	kill();

	delete _personRegion;
	_personRegion = nullptr;
}

void PeopleManager::turnMeAngle(OnScreenPerson *thisPerson, int direc) {
	int d = thisPerson->myPersona->numDirections;
	thisPerson->angle = direc;
	direc += (180 / d) + 180 + thisPerson->angleOffset;
	while (direc >= 360)
		direc -= 360;
	thisPerson->direction = (direc * d) / 360;
}

bool PeopleManager::init() {
	_personRegion->sX = 0;
	_personRegion->sY = 0;
	_personRegion->di = -1;
	return true;
}

void PeopleManager::spinStep(OnScreenPerson *thisPerson) {
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

void PeopleManager::rethinkAngle(OnScreenPerson *thisPerson) {
	int d = thisPerson->myPersona->numDirections;
	int direc = thisPerson->angle + (180 / d) + 180 + thisPerson->angleOffset;
	while (direc >= 360)
		direc -= 360;
	thisPerson->direction = (direc * d) / 360;
}

bool PeopleManager::turnPersonToFace(int thisNum, int direc) {
	OnScreenPerson *thisPerson = findPerson(thisNum);
	if (thisPerson) {
		if (thisPerson->continueAfterWalking)
			abortFunction(thisPerson->continueAfterWalking);
		thisPerson->continueAfterWalking = NULL;
		thisPerson->walking = false;
		thisPerson->spinning = false;
		turnMeAngle(thisPerson, direc);
		_vm->_speechMan->isCurrentTalker(thisPerson) ?
				thisPerson->makeTalker() : thisPerson->makeSilent();
		return true;
	}
	return false;
}

bool PeopleManager::setPersonExtra(int thisNum, int extra) {
	OnScreenPerson *thisPerson = findPerson(thisNum);
	if (thisPerson) {
		thisPerson->extra = extra;
		if (extra & EXTRA_NOSCALE)
			thisPerson->scale = 1;
		return true;
	}
	return false;
}

void PeopleManager::setScale(int16 h, int16 d) {
	_scaleHorizon = h;
	_scaleDivide = d;
}

void PeopleManager::moveAndScale(OnScreenPerson &me, float x, float y) {
	me.x = x;
	me.y = y;
	if (!(me.extra & EXTRA_NOSCALE) && _scaleDivide)
		me.scale = (me.y - _scaleHorizon) / _scaleDivide;
}

OnScreenPerson *PeopleManager::findPerson(int v) {
	OnScreenPerson *thisPerson = _allPeople;
	while (thisPerson) {
		if (v == thisPerson->thisType->objectNum)
			break;
		thisPerson = thisPerson->next;
	}
	return thisPerson;
}

void PeopleManager::movePerson(int x, int y, int objNum) {
	OnScreenPerson *moveMe = findPerson(objNum);
	if (moveMe)
		moveAndScale(*moveMe, x, y);
}

void PeopleManager::setShown(bool h, int ob) {
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

void PeopleManager::setMyDrawMode(OnScreenPerson *moveMe, int h) {
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

void PeopleManager::setDrawMode(int h, int ob) {
	OnScreenPerson *moveMe = findPerson(ob);
	if (!moveMe)
		return;

	setMyDrawMode(moveMe, h);
}

void PeopleManager::setPersonTransparency(int ob, byte x) {
	OnScreenPerson *moveMe = findPerson(ob);
	if (!moveMe)
		return;

	if (x > 254)
		x = 254;
	moveMe->transparency = x;
}

void PeopleManager::setPersonColourise(int ob, byte r, byte g, byte b, byte colourmix) {
	OnScreenPerson *moveMe = findPerson(ob);
	if (!moveMe)
		return;

	moveMe->r = r;
	moveMe->g = g;
	moveMe->b = b;
	moveMe->colourmix = colourmix;
}

void PeopleManager::shufflePeople() {
	OnScreenPerson **thisReference = &_allPeople;
	OnScreenPerson *A, *B;

	if (!_allPeople)
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

void PeopleManager::drawPeople() {
	shufflePeople();

	OnScreenPerson *thisPerson = _allPeople;
	PersonaAnimation *myAnim = NULL;
	_vm->_regionMan->resetOverRegion();

	while (thisPerson) {
		if (thisPerson->show) {
			myAnim = thisPerson->myAnim;
			if (myAnim != thisPerson->lastUsedAnim) {
				thisPerson->lastUsedAnim = myAnim;
				thisPerson->frameNum = 0;
				thisPerson->frameTick = myAnim->frames[0].howMany;
				if (myAnim->frames[thisPerson->frameNum].noise > 0) {
					_vm->_soundMan->startSound(myAnim->frames[thisPerson->frameNum].noise, false);
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
				r = _vm->_gfxMan->scaleSprite(myAnim->theSprites->bank.sprites[fNum], myAnim->theSprites->bank.myPalette, thisPerson, m);
				if (r) {
					if (!thisPerson->thisType->screenName.empty()) {
						if (_personRegion->thisType != thisPerson->thisType)
							_vm->_regionMan->resetLastRegion();
						_personRegion->thisType = thisPerson->thisType;
						_vm->_regionMan->setOverRegion(_personRegion);
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
					_vm->_soundMan->startSound(myAnim->frames[thisPerson->frameNum].noise, false);
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

bool PeopleManager::handleClosestPoint(int &setX, int &setY, int &setPoly) {
	int gotX = 320, gotY = 200, gotPoly = -1, i, j, xTest1, yTest1, xTest2, yTest2, closestX, closestY, oldJ, currentDistance = 0xFFFFF, thisDistance;

	for (i = 0; i < currentFloor->numPolygons; i++) {
		oldJ = currentFloor->polygon[i].numVertices - 1;
		for (j = 0; j < currentFloor->polygon[i].numVertices; j++) {
			xTest1 = currentFloor->vertex[currentFloor->polygon[i].vertexID[j]].x;
			yTest1 = currentFloor->vertex[currentFloor->polygon[i].vertexID[j]].y;
			xTest2 = currentFloor->vertex[currentFloor->polygon[i].vertexID[oldJ]].x;
			yTest2 = currentFloor->vertex[currentFloor->polygon[i].vertexID[oldJ]].y;
			closestPointOnLine(closestX, closestY, xTest1, yTest1, xTest2, yTest2, setX, setY);
			xTest1 = setX - closestX;
			yTest1 = setY - closestY;
			thisDistance = xTest1 * xTest1 + yTest1 * yTest1;

			if (thisDistance < currentDistance) {
				currentDistance = thisDistance;
				gotX = closestX;
				gotY = closestY;
				gotPoly = i;
			}
			oldJ = j;
		}
	}

	if (gotPoly == -1)
		return false;
	setX = gotX;
	setY = gotY;
	setPoly = gotPoly;

	return true;
}

bool PeopleManager::doBorderStuff(OnScreenPerson *moveMe) {
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

	moveMe->setFrames(ANI_WALK);
	return true;
}

bool PeopleManager::walkMe(OnScreenPerson *thisPerson, bool move) {
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
				thisPerson->setFrames(ANI_WALK);
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
	thisPerson->setFrames(ANI_STAND);
	moveAndScale(*thisPerson, thisPerson->walkToX, thisPerson->walkToY);
	return false;
}

bool PeopleManager::makeWalkingPerson(int x, int y, int objNum, LoadedFunction *func, int di) {
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

bool PeopleManager::stopPerson(int o) {
	OnScreenPerson *moveMe = findPerson(o);
	if (moveMe)
		if (moveMe->continueAfterWalking) {
			abortFunction(moveMe->continueAfterWalking);
			moveMe->continueAfterWalking = NULL;
			moveMe->walking = false;
			moveMe->spinning = false;
			moveMe->setFrames(ANI_STAND);
			return true;
		}
	return false;
}

bool PeopleManager::forceWalkingPerson(int x, int y, int objNum, LoadedFunction *func, int di) {
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

void PeopleManager::jumpPerson(int x, int y, int objNum) {
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

bool PeopleManager::floatCharacter(int f, int objNum) {
	OnScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe)
		return false;
	moveMe->floaty = f;
	return true;
}

bool PeopleManager::setCharacterWalkSpeed(int f, int objNum) {
	if (f <= 0)
		return false;
	OnScreenPerson *moveMe = findPerson(objNum);
	if (!moveMe)
		return false;
	moveMe->walkSpeed = f;
	return true;
}

void PeopleManager::walkAllPeople() {
	OnScreenPerson *thisPerson = _allPeople;

	while (thisPerson) {
		if (thisPerson->walking) {
			walkMe(thisPerson);
		} else if (thisPerson->spinning) {
			spinStep(thisPerson);
			thisPerson->setFrames(ANI_STAND);
		}
		if ((!thisPerson->walking) && (!thisPerson->spinning) && thisPerson->continueAfterWalking) {
			restartFunction(thisPerson->continueAfterWalking);
			thisPerson->continueAfterWalking = NULL;
		}
		thisPerson = thisPerson->next;
	}
}

bool PeopleManager::addPerson(int x, int y, int objNum, Persona *p) {
	OnScreenPerson *newPerson = new OnScreenPerson;
	if (!checkNew(newPerson))
		return false;

	// EASY STUFF
	newPerson->thisType = _vm->_objMan->loadObjectType(objNum);
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

	newPerson->setFrames(ANI_STAND);

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
	OnScreenPerson **changethat = &_allPeople;

	while (((*changethat) != NULL) && ((*changethat)->y < y))
		changethat = &((*changethat)->next);

	newPerson->next = (*changethat);
	(*changethat) = newPerson;

	return (bool)(newPerson->thisType != NULL);
}

void PeopleManager::animatePerson(int obj, PersonaAnimation *fram) { // Set a new SINGLE animation
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

void PeopleManager::animatePerson(int obj, Persona *per) {             // Set a new costume
	OnScreenPerson *moveMe = findPerson(obj);
	if (moveMe) {
		//  if (moveMe->continueAfterWalking) abortFunction (moveMe->continueAfterWalking);
		//  moveMe->continueAfterWalking = NULL;
		//  moveMe->walking = false;
		moveMe->spinning = false;
		moveMe->myPersona = per;
		rethinkAngle(moveMe);
		if (moveMe->walking) {
			moveMe->setFrames(ANI_WALK);
		} else {
			moveMe->setFrames(ANI_STAND);
		}
	}
}

void PeopleManager::kill() {
	OnScreenPerson *killPeople;
	while (_allPeople) {
		if (_allPeople->continueAfterWalking)
			abortFunction(_allPeople->continueAfterWalking);
		_allPeople->continueAfterWalking = NULL;
		killPeople = _allPeople;
		_allPeople = _allPeople->next;
		_vm->_objMan->removeObjectType(killPeople->thisType);
		delete killPeople;
	}
}

void PeopleManager::killMostPeople() {
	OnScreenPerson *killPeople;
	OnScreenPerson **lookyHere = &_allPeople;

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
			_vm->_objMan->removeObjectType(killPeople->thisType);
			delete killPeople;
		}
	}
}

void PeopleManager::removeOneCharacter(int i) {
	OnScreenPerson *p = findPerson(i);

	if (p) {
		ScreenRegion *overRegion = _vm->_regionMan->getOverRegion();
		if (overRegion == _personRegion && overRegion->thisType == p->thisType) {
			overRegion = nullptr;
		}

		if (p->continueAfterWalking)
			abortFunction(p->continueAfterWalking);
		p->continueAfterWalking = NULL;
		OnScreenPerson **killPeople;

		for (killPeople = &_allPeople; *killPeople != p; killPeople = &((*killPeople)->next)) {
			;
		}

		*killPeople = p->next;
		_vm->_objMan->removeObjectType(p->thisType);
		delete p;
	}
}

bool PeopleManager::savePeople(Common::WriteStream *stream) {
	OnScreenPerson *me = _allPeople;
	int countPeople = 0, a;

	stream->writeSint16LE(_scaleHorizon);
	stream->writeSint16LE(_scaleDivide);

	while (me) {
		countPeople++;
		me = me->next;
	}

	stream->writeUint16BE(countPeople);

	me = _allPeople;
	for (a = 0; a < countPeople; a++) {

		stream->writeFloatLE(me->x);
		stream->writeFloatLE(me->y);

		me->myPersona->save(stream);
		me->myAnim->save(stream);
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

		_vm->_objMan->saveObjectRef(me->thisType, stream);

		me = me->next;
	}
	return true;
}

bool PeopleManager::loadPeople(Common::SeekableReadStream *stream) {
	kill();

	OnScreenPerson **pointy = &_allPeople;
	OnScreenPerson *me;

	_scaleHorizon = stream->readSint16LE();
	_scaleDivide = stream->readSint16LE();

	int countPeople = stream->readUint16BE();
	int a;

	_allPeople = NULL;
	for (a = 0; a < countPeople; a++) {
		me = new OnScreenPerson;
		if (!checkNew(me))
			return false;

		me->myPersona = new Persona;
		if (!checkNew(me->myPersona))
			return false;

		me->myAnim = new PersonaAnimation;
		if (!checkNew(me->myAnim))
			return false;

		me->x = stream->readFloatLE();
		me->y = stream->readFloatLE();

		me->myPersona->load(stream);
		me->myAnim->load(stream);

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
		me->thisType = _vm->_objMan->loadObjectRef(stream);

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
	return true;
}

void PeopleManager::freeze(FrozenStuffStruct *frozenStuff) {
	frozenStuff->allPeople = _allPeople;
	_allPeople = nullptr;
}

void PeopleManager::resotre(FrozenStuffStruct *frozenStuff) {
	kill();
	_allPeople = frozenStuff->allPeople;
}

} // End of namespace Sludge
