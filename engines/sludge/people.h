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
#ifndef SLUDGE_PEOPLE_H
#define SLUDGE_PEOPLE_H

#include "sludge/variable.h"

namespace Sludge {

struct animFrame {
	int frameNum, howMany;
	int noise;
};

#define EXTRA_FRONT         1
#define EXTRA_FIXEDSIZE     2
#define EXTRA_NOSCALE       2   // Alternative name
#define EXTRA_NOZB          4
#define EXTRA_FIXTOSCREEN   8
#define EXTRA_NOLITE        16
#define EXTRA_NOREMOVE      32
#define EXTRA_RECTANGULAR   64

struct personaAnimation {
	struct LoadedSpriteBank *theSprites;
	animFrame *frames;
	int numFrames;
};

struct persona {
	personaAnimation **animation;
	int numDirections;
};

struct onScreenPerson {
	float x, y;
	int height, floaty, walkSpeed;
	float scale;
	onScreenPerson *next;
	int walkToX, walkToY, thisStepX, thisStepY, inPoly, walkToPoly;
	bool walking, spinning;
	struct loadedFunction *continueAfterWalking;
	personaAnimation *myAnim;
	personaAnimation *lastUsedAnim;
	persona *myPersona;
	int frameNum, frameTick, angle, wantAngle, angleOffset;
	bool show;
	int direction, directionWhenDoneWalking;
	struct ObjectType *thisType;
	int extra, spinSpeed;
	byte r, g, b, colourmix, transparency;
};

// Initialisation and creation
bool initPeople();
bool addPerson(int x, int y, int objNum, persona *p);

// Draw to screen and to backdrop
void drawPeople();
void freezePeople(int, int);

// Removalisationisms
void killAllPeople();
void killMostPeople();
void removeOneCharacter(int i);

// Things which affect or use all characters
onScreenPerson *findPerson(int v);
void setScale(int16 h, int16 d);

// Things which affect one character
void makeTalker(onScreenPerson &me);
void makeSilent(onScreenPerson &me);
void setShown(bool h, int ob);
void setDrawMode(int h, int ob);
void setPersonTransparency(int ob, byte x);
void setPersonColourise(int ob, byte r, byte g, byte b, byte colourmix);

// Moving 'em
void movePerson(int x, int y, int objNum);
bool makeWalkingPerson(int x, int y, int objNum, struct loadedFunction *func, int di);
bool forceWalkingPerson(int x, int y, int objNum, struct loadedFunction *func, int di);
void jumpPerson(int x, int y, int objNum);
void walkAllPeople();
bool turnPersonToFace(int thisNum, int direc);
bool stopPerson(int o);
bool floatCharacter(int f, int objNum);
bool setCharacterWalkSpeed(int f, int objNum);

// Animating 'em
void animatePerson(int obj, personaAnimation *);
void animatePerson(int obj, persona *per);
personaAnimation *createPersonaAnim(int num, struct variableStack *&stacky);
inline void setBankFile(personaAnimation *newP, LoadedSpriteBank *sB) {
	newP->theSprites = sB;
}
bool setPersonExtra(int f, int newSetting);
int timeForAnim(personaAnimation *fram);
personaAnimation *copyAnim(personaAnimation *orig);
personaAnimation *makeNullAnim();
void deleteAnim(personaAnimation *orig);

// Loading and saving
bool saveAnim(personaAnimation *p, Common::WriteStream *stream);
bool loadAnim(personaAnimation *p, Common::SeekableReadStream *stream);
bool savePeople(Common::WriteStream *stream);
bool loadPeople(Common::SeekableReadStream *stream);
bool saveCostume(persona *cossy, Common::WriteStream *stream);
bool loadCostume(persona *cossy, Common::SeekableReadStream *stream);

} // End of namespace Sludge

#endif
