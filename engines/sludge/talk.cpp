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
#include "backdrop.h"
#include "sprites.h"
#include "sludger.h"
#include "objtypes.h"
#include "region.h"
#include "sprbanks.h"
#include "people.h"
#include "talk.h"
#include "sound.h"
#include "fonttext.h"
#include "newfatal.h"
#include "stringy.h"
#include "moreio.h"

namespace Sludge {

extern int fontHeight, cameraX, cameraY, speechMode;
extern float cameraZoom;
speechStruct *speech;
float speechSpeed = 1;

void initSpeech() {
	speech = new speechStruct;
	if (checkNew(speech)) {
		speech -> currentTalker = NULL;
		speech -> allSpeech = NULL;
		speech -> speechY = 0;
		speech -> lastFile = -1;
	}
}

void killAllSpeech() {
	if (speech -> lastFile != -1) {
#if 0
		huntKillSound(speech -> lastFile);
#endif
		speech -> lastFile = -1;
	}

	if (speech -> currentTalker) {
		makeSilent(* (speech -> currentTalker));
		speech -> currentTalker = NULL;
	}

	speechLine *killMe;

	while (speech -> allSpeech) {
		killMe = speech -> allSpeech;
		speech -> allSpeech = speech -> allSpeech -> next;
		delete killMe -> textLine;
		delete killMe;
	}
}

#define TF_max(a, b) ((a > b) ? a : b)
#define TF_min(a, b) ((a > b) ? b : a)

inline void setObjFontColour(objectType *t) {
#if 0
	setFontColour(speech -> talkCol, t -> r, t -> g, t -> b);
#endif
}

void addSpeechLine(char *theLine, int x, int &offset) {
	int halfWidth = (stringWidth(theLine) >> 1) / cameraZoom;
	int xx1 = x - (halfWidth);
	int xx2 = x + (halfWidth);
	speechLine *newLine = new speechLine;
	checkNew(newLine);

	newLine -> next = speech -> allSpeech;
	newLine -> textLine = copyString(theLine);
	newLine -> x = xx1;
	speech -> allSpeech = newLine;
	if ((xx1 < 5) && (offset < (5 - xx1))) {
		offset = 5 - xx1;
	} else if ((xx2 >= ((float)winWidth / cameraZoom) - 5) && (offset > (((float)winWidth / cameraZoom) - 5 - xx2))) {
		offset = ((float)winWidth / cameraZoom) - 5 - xx2;
	}
}

int isThereAnySpeechGoingOn() {
	return speech -> allSpeech ? speech -> lookWhosTalking : -1;
}

int wrapSpeechXY(char *theText, int x, int y, int wrap, int sampleFile) {
	int a, offset = 0;

	killAllSpeech();

	int speechTime = (strlen(theText) + 20) * speechSpeed;
	if (speechTime < 1) speechTime = 1;
	if (sampleFile != -1) {
		if (speechMode >= 1) {
#if 0
			if (startSound(sampleFile, false)) {
				speechTime = -10;
				speech -> lastFile = sampleFile;
				if (speechMode == 2) return -10;
			}
#endif
		}
	}
	speech -> speechY = y;

	while (strlen(theText) > wrap) {
		a = wrap;
		while (theText[a] != ' ') {
			a --;
			if (a == 0) {
				a = wrap;
				break;
			}
		}
		theText[a] = 0;
		addSpeechLine(theText, x, offset);
		theText[a] = ' ';
		theText += a + 1;
		y -= fontHeight / cameraZoom;
	}
	addSpeechLine(theText, x, offset);
	y -= fontHeight / cameraZoom;

	if (y < 0) speech -> speechY -= y;
	else if (speech -> speechY > cameraY + (float)(winHeight - fontHeight / 3) / cameraZoom) speech -> speechY = cameraY + (float)(winHeight - fontHeight / 3) / cameraZoom;

	if (offset) {
		speechLine *viewLine = speech -> allSpeech;
		while (viewLine) {
			viewLine -> x += offset;
			viewLine = viewLine -> next;
		}
	}
	return speechTime;
}

int wrapSpeechPerson(char *theText, onScreenPerson &thePerson, int sampleFile, bool animPerson) {
	int i = wrapSpeechXY(theText, thePerson.x - cameraX, thePerson.y - cameraY - (thePerson.scale * (thePerson.height - thePerson.floaty)) - thePerson.thisType -> speechGap, thePerson.thisType -> wrapSpeech, sampleFile);
	if (animPerson) {
		makeTalker(thePerson);
		speech -> currentTalker = & thePerson;
	}
	return i;
}

int wrapSpeech(char *theText, int objT, int sampleFile, bool animPerson) {
	int i;

	speech -> lookWhosTalking = objT;
	onScreenPerson *thisPerson = findPerson(objT);
	if (thisPerson) {
		setObjFontColour(thisPerson -> thisType);
		i = wrapSpeechPerson(theText, * thisPerson, sampleFile, animPerson);
	} else {
		screenRegion *thisRegion = getRegionForObject(objT);
		if (thisRegion) {
			setObjFontColour(thisRegion -> thisType);
			i = wrapSpeechXY(theText, ((thisRegion -> x1 + thisRegion -> x2) >> 1) - cameraX, thisRegion -> y1 - thisRegion -> thisType -> speechGap - cameraY, thisRegion -> thisType -> wrapSpeech, sampleFile);
		} else {
			objectType *temp = findObjectType(objT);
			setObjFontColour(temp);
			i = wrapSpeechXY(theText, winWidth >> 1, 10, temp -> wrapSpeech, sampleFile);
		}
	}
	return i;
}

void viewSpeech() {
#if 0
	int viewY = speech -> speechY;
	speechLine *viewLine = speech -> allSpeech;
	fixFont(speech -> talkCol);
	while (viewLine) {
		pasteString(viewLine -> textLine, viewLine -> x, viewY, speech -> talkCol);
		viewY -= fontHeight / cameraZoom;
		viewLine = viewLine -> next;
	}
#endif
}

#if ALLOW_FILE
void saveSpeech(speechStruct *sS, FILE *fp) {
	speechLine *viewLine = sS -> allSpeech;

	fputc(sS -> talkCol.originalRed, fp);
	fputc(sS -> talkCol.originalGreen, fp);
	fputc(sS -> talkCol.originalBlue, fp);

	putFloat(speechSpeed, fp);

	// Write y co-ordinate
	put2bytes(sS -> speechY, fp);

	// Write which character's talking
	put2bytes(sS -> lookWhosTalking, fp);
	if (sS -> currentTalker) {
		fputc(1, fp);
		put2bytes(sS -> currentTalker -> thisType -> objectNum, fp);
	} else {
		fputc(0, fp);
	}

	// Write what's being said
	while (viewLine) {
		fputc(1, fp);
		writeString(viewLine -> textLine, fp);
		put2bytes(viewLine -> x, fp);
		viewLine = viewLine -> next;
	}
	fputc(0, fp);
}

bool loadSpeech(speechStruct *sS, FILE *fp) {
	speech -> currentTalker = NULL;
	killAllSpeech();
	byte r = fgetc(fp);
	byte g = fgetc(fp);
	byte b = fgetc(fp);
	setFontColour(sS -> talkCol, r, g, b);

	speechSpeed = getFloat(fp);

	// Read y co-ordinate
	sS -> speechY = get2bytes(fp);

	// Read which character's talking
	sS -> lookWhosTalking = get2bytes(fp);

	if (fgetc(fp)) {
		sS -> currentTalker = findPerson(get2bytes(fp));
	} else {
		sS -> currentTalker = NULL;
	}

	// Read what's being said
	speechLine * * viewLine = & sS -> allSpeech;
	speechLine *newOne;
	speech -> lastFile = -1;
	while (fgetc(fp)) {
		newOne = new speechLine;
		if (! checkNew(newOne)) return false;
		newOne -> textLine = readString(fp);
		newOne -> x = get2bytes(fp);
		newOne -> next = NULL;
		(* viewLine) = newOne;
		viewLine = & (newOne -> next);
	}

	return true;
}
#endif

} // End of namespace Sludge
