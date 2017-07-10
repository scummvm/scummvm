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
#include "sludge/backdrop.h"
#include "sludge/sprites.h"
#include "sludge/sludger.h"
#include "sludge/objtypes.h"
#include "sludge/region.h"
#include "sludge/sprbanks.h"
#include "sludge/people.h"
#include "sludge/talk.h"
#include "sludge/sound.h"
#include "sludge/fonttext.h"
#include "sludge/newfatal.h"
#include "sludge/stringy.h"
#include "sludge/moreio.h"

namespace Sludge {

extern int fontHeight, cameraX, cameraY, speechMode;
extern float cameraZoom;
speechStruct *speech;
float speechSpeed = 1;

void initSpeech() {
	speech = new speechStruct;
	if (checkNew(speech)) {
		speech->currentTalker = NULL;
		speech->allSpeech = NULL;
		speech->speechY = 0;
		speech->lastFile = -1;
	}
}

void killAllSpeech() {
	if (speech->lastFile != -1) {
#if 0
		huntKillSound(speech->lastFile);
#endif
		speech->lastFile = -1;
	}

	if (speech->currentTalker) {
		makeSilent(*(speech->currentTalker));
		speech->currentTalker = NULL;
	}

	speechLine *killMe;

	while (speech->allSpeech) {
		killMe = speech->allSpeech;
		speech->allSpeech = speech->allSpeech->next;
		delete[] killMe->textLine;
		delete killMe;
	}
}

inline void setObjFontColour(objectType *t) {
	setFontColour(speech->talkCol, t->r, t->g, t->b);
}

void addSpeechLine(char *theLine, int x, int &offset) {
	int halfWidth = (stringWidth(theLine) >> 1) / cameraZoom;
	int xx1 = x - (halfWidth);
	int xx2 = x + (halfWidth);
	speechLine *newLine = new speechLine;
	checkNew(newLine);

	newLine->next = speech->allSpeech;
	newLine->textLine = copyString(theLine);
	newLine->x = xx1;
	speech->allSpeech = newLine;
	if ((xx1 < 5) && (offset < (5 - xx1))) {
		offset = 5 - xx1;
	} else if ((xx2 >= ((float) winWidth / cameraZoom) - 5)
			&& (offset > (((float) winWidth / cameraZoom) - 5 - xx2))) {
		offset = ((float) winWidth / cameraZoom) - 5 - xx2;
	}
}

int isThereAnySpeechGoingOn() {
	return speech->allSpeech ? speech->lookWhosTalking : -1;
}

int wrapSpeechXY(char *theText, int x, int y, int wrap, int sampleFile) {
	int a, offset = 0;

	killAllSpeech();

	int speechTime = (strlen(theText) + 20) * speechSpeed;
	if (speechTime < 1)
		speechTime = 1;
	if (sampleFile != -1) {
		if (speechMode >= 1) {
#if 0
			if (startSound(sampleFile, false)) {
				speechTime = -10;
				speech->lastFile = sampleFile;
				if (speechMode == 2) return -10;
			}
#endif
		}
	}
	speech->speechY = y;

	while (strlen(theText) > wrap) {
		a = wrap;
		while (theText[a] != ' ') {
			a--;
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

	if (y < 0)
		speech->speechY -= y;
	else if (speech->speechY
			> cameraY + (float) (winHeight - fontHeight / 3) / cameraZoom)
		speech->speechY = cameraY
				+ (float) (winHeight - fontHeight / 3) / cameraZoom;

	if (offset) {
		speechLine *viewLine = speech->allSpeech;
		while (viewLine) {
			viewLine->x += offset;
			viewLine = viewLine->next;
		}
	}
	return speechTime;
}

int wrapSpeechPerson(char *theText, onScreenPerson &thePerson, int sampleFile,
		bool animPerson) {
	int i = wrapSpeechXY(theText, thePerson.x - cameraX,
			thePerson.y - cameraY
					- (thePerson.scale * (thePerson.height - thePerson.floaty))
					- thePerson.thisType->speechGap,
			thePerson.thisType->wrapSpeech, sampleFile);
	if (animPerson) {
		makeTalker(thePerson);
		speech->currentTalker = &thePerson;
	}
	return i;
}

int wrapSpeech(char *theText, int objT, int sampleFile, bool animPerson) {
	int i;

	speech->lookWhosTalking = objT;
	onScreenPerson *thisPerson = findPerson(objT);
	if (thisPerson) {
		setObjFontColour(thisPerson->thisType);
		i = wrapSpeechPerson(theText, *thisPerson, sampleFile, animPerson);
	} else {
		screenRegion *thisRegion = getRegionForObject(objT);
		if (thisRegion) {
			setObjFontColour(thisRegion->thisType);
			i = wrapSpeechXY(theText,
					((thisRegion->x1 + thisRegion->x2) >> 1) - cameraX,
					thisRegion->y1 - thisRegion->thisType->speechGap - cameraY,
					thisRegion->thisType->wrapSpeech, sampleFile);
		} else {
			objectType *temp = findObjectType(objT);
			setObjFontColour(temp);
			i = wrapSpeechXY(theText, winWidth >> 1, 10, temp->wrapSpeech,
					sampleFile);
		}
	}
	return i;
}

void viewSpeech() {
	int viewY = speech->speechY;
	speechLine *viewLine = speech->allSpeech;
	fixFont(speech->talkCol);
	while (viewLine) {
		pasteString(viewLine->textLine, viewLine->x, viewY, speech->talkCol);
		viewY -= fontHeight / cameraZoom;
		viewLine = viewLine->next;
	}
}

void saveSpeech(speechStruct *sS, Common::WriteStream *stream) {
	speechLine *viewLine = sS->allSpeech;

	stream->writeByte(sS->talkCol.originalRed);
	stream->writeByte(sS->talkCol.originalGreen);
	stream->writeByte(sS->talkCol.originalBlue);

	stream->writeFloatLE(speechSpeed);

	// Write y co-ordinate
	stream->writeUint16BE(sS->speechY);

	// Write which character's talking
	stream->writeUint16BE(sS->lookWhosTalking);
	if (sS->currentTalker) {
		stream->writeByte(1);
		stream->writeUint16BE(sS->currentTalker->thisType->objectNum);
	} else {
		stream->writeByte(0);
	}

	// Write what's being said
	while (viewLine) {
		stream->writeByte(1);
		writeString(viewLine->textLine, stream);
		stream->writeUint16BE(viewLine->x);
		viewLine = viewLine->next;
	}
	stream->writeByte(0);
}

bool loadSpeech(speechStruct *sS, Common::SeekableReadStream *stream) {
	speech->currentTalker = NULL;
	killAllSpeech();
	byte r = stream->readByte();
	byte g = stream->readByte();
	byte b = stream->readByte();
	setFontColour(sS->talkCol, r, g, b);

	speechSpeed = stream->readFloatLE();

	// Read y co-ordinate
	sS->speechY = stream->readUint16BE();

	// Read which character's talking
	sS->lookWhosTalking = stream->readUint16BE();

	if (stream->readByte()) {
		sS->currentTalker = findPerson(stream->readUint16BE());
	} else {
		sS->currentTalker = NULL;
	}

	// Read what's being said
	speechLine * * viewLine = &sS->allSpeech;
	speechLine *newOne;
	speech->lastFile = -1;
	while (stream->readByte()) {
		newOne = new speechLine;
		if (! checkNew(newOne)) return false;
		newOne->textLine = readString(stream);
		newOne->x = stream->readUint16BE();
		newOne->next = NULL;
		(* viewLine) = newOne;
		viewLine = &(newOne->next);
	}
	return true;
}

} // End of namespace Sludge
