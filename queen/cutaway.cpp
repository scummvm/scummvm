/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "cutaway.h"

/*
	 Functions needed:

	 queen.c:   DISP_ROOM
	 MOVE_JOE
	 MOVE_OTHER

	 talk.c:    SPEAK


	 Data needed:

	 JX,JY        (Joe's coordintes)
	 OBJECT_DATA
	 ROOM_DATA

 */

void QueenCutaway::run(
		const char *filename, 
		QueenLogic *queenLogic,
		QueenResource *queenResource) {
	QueenCutaway *cutaway = new QueenCutaway(filename, queenLogic, queenResource);
	cutaway->run();
	delete cutaway;
}

QueenCutaway::QueenCutaway(
		const char *filename, 
		QueenLogic *queenLogic,
		QueenResource *queenResource) : _queenLogic(queenLogic), _quit(false) {
			memset(&_bankNames, 0, sizeof(_bankNames));
			load(filename, queenResource); 
		}

QueenCutaway::~QueenCutaway() {
	delete[] _fileData;
}

void QueenCutaway::load(const char *filename, QueenResource *queenResource) {
	byte *ptr;

	/* XXX: make sure that loadFile is not case sensitive */
	ptr = _fileData = queenResource->loadFile(filename, 20);
	if (!_fileData) {
		/* XXX: error message? */
		_quit = true;
		return;
	}

	strcpy(_basename, filename);
	_basename[strlen(_basename)-4] = '\0';

	int COMPANEL = READ_BE_UINT16(ptr);
	ptr += 2;
	debug(0, "COMPANEL = %i", COMPANEL);

	_cutawayObjectCount = READ_BE_UINT16(ptr);
	ptr += 2;
	debug(0, "_cutawayObjectCount = %i", _cutawayObjectCount);

	if (_cutawayObjectCount < 0) {
		_cutawayObjectCount = -_cutawayObjectCount;
		_canQuit = false;
	}
	else
		_canQuit = true;

	int flags1 = READ_BE_UINT16(ptr);
	ptr += 2;
	debug(0, "flags1 = %i", (int16)flags1);

	if (flags1 < 0) {
		/* ENTRY_OBJ = 0 */
		_finalRoom = -flags1;
	}
	else
		_finalRoom = PREVIOUS_ROOM;

	if (flags1 == 1)
		/* ANOTHER_CUT = 1 */;
	else
		/* ANOTHER_CUT = 0 */;

	/*
		 Pointers to other places in the cutaway data
	 */

	byte *gameStatesPtr = _fileData + READ_BE_UINT16(ptr);
	ptr += 2;

	_nextSentence        = _fileData + READ_BE_UINT16(ptr);
	ptr += 2;

	byte *bankNamesPtr  = _fileData + READ_BE_UINT16(ptr);
	ptr += 2;
	debug(0, "Bank name file offset = %08x", 20 + bankNamesPtr - _fileData);

	_objectData = ptr;

	loadStrings(bankNamesPtr);

	if (_bankNames[0][0]) {
		debug(0, "Loading bank '%s'", _bankNames[0]);
		/* loadbank(_bankNames[0]) */ ;
	}

	char entryString[MAX_STRING_SIZE];
	_nextSentence = getString(_nextSentence, entryString, MAX_STRING_LENGTH);
	debug(0, "Entry string = '%s'", entryString);

	if (entryString[0] == '*' &&
			entryString[1] == 'F' &&
			entryString[3] == '\0') {
		switch (entryString[2]) {
			case 'L':
				/* CUTJOEF = LEFT; */
				break;
			case 'R':
				/* CUTJOEF = RIGHT; */
				break;
			case 'F':
				/* CUTJOEF = FRONT; */
				break;
			case 'B':
				/* CUTJOEF = BACK; */
				break;
		}
	}

}

byte *QueenCutaway::getString(byte *ptr, char *str, int maxLength) {
	int length = *ptr;
	ptr++;

	if (length > maxLength) {
		error("String too long. Length = %i, maxLength = %i, str = '%*s'",
				length, maxLength, length, (const char*)ptr);
	}
	else if (length) {
		memcpy(str, (const char*)ptr, length);
		ptr += length;

		while ((int)ptr % 2)
			ptr++;
	}

	str[length] = '\0';

	return ptr;
}

void QueenCutaway::loadStrings(byte *ptr) {
	int i,j;

	int bankNameCount = READ_BE_UINT16(ptr);
	ptr += 2;

	debug(0, "Bank name count = %i", bankNameCount);

	/*
		 The _bankNames zero-based array is the one-based BANK_NAMEstr array in
		 the original source code.
	 */

	for (i = 0, j = 0; i < bankNameCount; i++) {
		ptr = getString(ptr, _bankNames[j], MAX_FILENAME_LENGTH);

		if (_bankNames[j][0]) {
			debug(0, "Bank name %i = '%s'", _bankNames[j]);
			j++;
		}
	}

	debug(0, "Getting talk file");
	ptr = getString(ptr, _talkFile, MAX_FILENAME_LENGTH);
	debug(0, "Talk file = '%s'", _talkFile);

	int TALKTO = READ_BE_UINT16(ptr);
	ptr += 2;
	debug(0, "TALKTO = %i", TALKTO);
}

byte *QueenCutaway::getCutawayObject(byte *ptr, CutawayObject &object)
{
	byte *oldPtr = ptr;

	object.objectNumber  = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.moveToX      = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.moveToY      = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.bank           = (int16)READ_BE_UINT16(ptr); ptr += 2;      
	object.animList      = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.execute        = (int16)READ_BE_UINT16(ptr); ptr += 2;    
	object.limitBobX1   = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.limitBobY1   = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.limitBobX2   = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.limitBobY2   = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.specialMove   = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.animType      = (int16)READ_BE_UINT16(ptr); ptr += 2;   
	object.fromObject    = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.bobStartX    = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.bobStartY    = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.room           = (int16)READ_BE_UINT16(ptr); ptr += 2;
	object.scale          = (int16)READ_BE_UINT16(ptr); ptr += 2;

	if ((ptr - oldPtr) != 17*sizeof(int16))
		error("Wrong number of values read");

	// Make ugly reuse of data less ugly
	if (object.limitBobX1 < 0)
		object.song = -object.limitBobX1;
	else
		object.song = 0;

	return ptr;
}

void QueenCutaway::dumpCutawayObject(int index, CutawayObject &object)
{
	debug(0, "----- CutawayObject[%i] -----", index);

	const char *objectNumberStr;

	switch (object.objectNumber) {
		case -1:  objectNumberStr = "MESSAGE";  break;
		case 0:   objectNumberStr = "JOE";      break;
		default:  objectNumberStr = "unknown";  break;
	}

	debug(0, "objectNumber = %i (%s)", object.objectNumber, objectNumberStr);

	if (object.moveToX) debug(0, "moveToX = %i", object.moveToX);
	if (object.moveToY) debug(0, "moveToY = %i", object.moveToY);
	if (object.bank) debug(0, "bank = %i", object.bank);
	if (object.animList) debug(0, "animList = %i", object.animList);
	if (object.execute) debug(0, "execute = %i", object.execute);
	if (object.limitBobX1) debug(0, "limitBobX1 = %i", object.limitBobX1);
	if (object.limitBobY1) debug(0, "limitBobY1 = %i", object.limitBobY1);
	if (object.limitBobX2) debug(0, "limitBobX2 = %i", object.limitBobX2);
	if (object.limitBobY2) debug(0, "limitBobY2 = %i", object.limitBobY2);
	if (object.specialMove) debug(0, "specialMove = %i", object.specialMove);
	if (object.animType) debug(0, "animType = %i", object.animType);
	if (object.fromObject) debug(0, "fromObject = %i", object.fromObject);
	if (object.bobStartX) debug(0, "bobStartX = %i", object.bobStartX);
	if (object.bobStartY) debug(0, "bobStartY = %i", object.bobStartY);
	if (object.room) debug(0, "room = %i", object.room);
	if (object.scale) debug(0, "scale = %i", object.scale);

}

void QueenCutaway::specialMove(int index) {

	switch (index) {
		default:
			warning("Unhandled special move: %i", index);
			break;
	}
}

byte *QueenCutaway::turnOnPeople(byte *ptr, CutawayObject &object) {
	// Lines 1248-1259 in cutaway.c
	object.personCount = (int16)READ_BE_UINT16(ptr);
	ptr += 2;

	if (object.personCount > MAX_PERSON_COUNT)
		error("[QueenCutaway::turnOnPeople] object.personCount > MAX_PERSON_COUNT");

	for (int i = 0; i < object.personCount; i++) {
		object.person[i] = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		debug(0, "[%i] Turn on person %i", i, object.person[i]);
	}

	return ptr;
}

void QueenCutaway::limitBob(CutawayObject &object) {
	if (object.limitBobX1) {
		warning("QueenCutaway::limitBob() not implemented");
		// FIND_BOB(object.objectNumber);
		// XXX K=Param;
		// bobs[K].x1 = object.limitBobX1;
		// bobs[K].y1 = object.limitBobY1;
		// bobs[K].x2 = object.limitBobX2;
		// bobs[K].y2 = object.limitBobY2;
	}
}

void QueenCutaway::changeRooms(CutawayObject &object) {
	// Lines 1291-1385 in cutaway.c
	warning("QueenCutaway::changeRooms() not implemented properly");

	for (int i = 0; i < _savedPersonCount; i++) {
		// restore parts of OBJECT_DATA array from PERSON_DATA array
	}

	_savedPersonCount = 0;

	if (_finalRoom != object.room) {
		// XXX loop room data and do stuff
		// uses ROOM_DATA
		// uses OBJECT_DATA
		// backup parts of OBJECT_DATA array in PERSON_DATA array
		// increase _savedPersonCount
	}

	// set coordinates for Joe if he is on screen

	// XXX this is global data!
	int joeX = 0;
	int joeY = 0;

	for (int i = 0; i < object.personCount; i++) {
		if (PERSON_JOE == object.person[i]) {
			joeX = object.bobStartX;
			joeY = object.bobStartY;
		}
	}

	// XXX OLDROOM=IROOM;

	if (!joeX && !joeY) {
		// XXX DISP_ROOM(ROOM_NAMEstr[ROOM],0,SF);
	}
	else {
		// XXX We need to display Joe on screen
#if 0
		if(ROOMFADE==1)
			DISP_ROOM(ROOM_NAMEstr[ROOM],2,SF);
		else
			DISP_ROOM(ROOM_NAMEstr[ROOM],3,SF);
#endif
	}

	// XXX CI=FRAMES;

	// XXX TROOM=ROOM;

	for (int i = 0; i < _savedPersonCount; i++) {
		// restore parts of OBJECT_DATA array from PERSON_DATA array
	}

	// XXX CUTAWAY_SCALE(OBJECT);
	// XXX SF=Param;
	// XXX if(SCALE_FACTOR>0 && SCALE_FACTOR<100)
	// XXX   SF=SCALE_FACTOR;

}

QueenCutaway::ObjectType QueenCutaway::getObjectType(CutawayObject &object) {
	ObjectType objectType = OBJECT_TYPE_ANIMATION;

	if (object.objectNumber > 0) {
		if (!object.animList) {
			// No anim frames, so treat as a PERSON, ie. allow to speak/walk

			//if(OBJECT_DATA[OBJECT][7]==-3 || OBJECT_DATA[OBJECT][7]==-4)
			//  OBJTYPE=1;
		}
	}
	else if (object.objectNumber == OBJECT_JOE) {
		// It's Joe. See if he's to be treated as a person.
		if (!object.animList) {
			// There's no animation list, so Joe must be talking.
			objectType = OBJECT_TYPE_PERSON;
		}
	}

	if (object.fromObject > 0) {
		warning("Cannot copy objects yet");

		/* Copy FROM_OBJECT into OBJECT */

#if 0
		if(FROM_OBJECT!=OBJECT)
			P3_COPY_FROM(FROM_OBJECT,OBJECT);
		else

			// Same object, so just turn it on!

			OBJECT_DATA[OBJECT][0]=abs(OBJECT_DATA[OBJECT][0]);
		REDISP_OBJECT(OBJECT);
#endif

		// Skip doing any anim stuff

		objectType = OBJECT_TYPE_NO_ANIMATION;
	}

	switch(object.objectNumber) {
		case -2:
			// Text to be spoken
			objectType = OBJECT_TYPE_TEXT_SPEAK;
			break;
		case -3:
			// Text to be displayed AND spoken
			objectType = OBJECT_TYPE_TEXT_DISPLAY_AND_SPEAK;
			break;
		case -4:
			// Text to be displayed only (not spoken)
			objectType = OBJECT_TYPE_TEXT_DISPLAY;
			break;
	}

	if (OBJECT_TYPE_ANIMATION == objectType && !object.execute) {
		// Execute is not on, and it's an object, so ignore any Anims
		objectType = OBJECT_TYPE_NO_ANIMATION;
	}

	return objectType;
}

byte *QueenCutaway::getCutawayAnim(byte *ptr, int header, CutawayAnim &anim) {

	anim.currentFrame = 0;
	anim.originalFrame = 0;

	if (-1 == header)
		header = 0;

	if (0 == header) {
		anim.object = 0;
		// XXX
#if 0
		anim.originalFrame =29+XTRA;
		// 21/9/94, Make sure that bobs are clipped on 150 screens
		if(FULLSCREEN) bobs[0].y2=199;
#endif
	}
	else {
		warning("Stuff not yet implemented in QueenCutaway::handleAnimation()");
		// XXX
		// FIND_BOB(header);
		// anim.object = Param;

#if 0
		// If fullscreen cutaway then clip to 199 down

		// 21/9/94, Make sure that bobs are clipped on 150 screens
		if(COMPANEL==2 && OBJ_CUT[6]<=0 && BDyres==200) bobs[Param].y2=199;
		FIND_FRAME(J);
		anim.originalFrame =Param;
#endif
	}

	anim.unpackFrame = (int16)READ_BE_UINT16(ptr);
	ptr += 2;

	anim.speed = ((int16)READ_BE_UINT16(ptr)) / 3 + 1;
	ptr += 2;

	anim.bank = (int16)READ_BE_UINT16(ptr);
	ptr += 2;

	anim.mx = (int16)READ_BE_UINT16(ptr);
	ptr += 2;

	anim.my = (int16)READ_BE_UINT16(ptr);
	ptr += 2;

	anim.cx = (int16)READ_BE_UINT16(ptr);
	ptr += 2;

	anim.cy = (int16)READ_BE_UINT16(ptr);
	ptr += 2;

	anim.scale = (int16)READ_BE_UINT16(ptr);
	ptr += 2;

#if 1
	anim.song = (int16)READ_BE_UINT16(ptr);
	ptr += 2;
#else
	anim.song = 0;
#endif

	return ptr;
}

void QueenCutaway::dumpCutawayAnim(CutawayAnim &anim) {
	debug(0, "----- CutawayAnim -----");
	if (anim.object) debug(0, "object = %i", anim.object);
	if (anim.unpackFrame) debug(0, "unpackFrame = %i", anim.unpackFrame);
	if (anim.speed) debug(0, "speed = %i", anim.speed);
	if (anim.bank) debug(0, "bank = %i", anim.bank);
	if (anim.mx) debug(0, "mx = %i", anim.mx);
	if (anim.my) debug(0, "my = %i", anim.my);
	if (anim.cx) debug(0, "cx = %i", anim.cx);
	if (anim.cy) debug(0, "cy = %i", anim.cy);
	if (anim.scale) debug(0, "scale = %i", anim.scale);
	if (anim.currentFrame) debug(0, "currentFrame = %i", anim.currentFrame);
	if (anim.originalFrame) debug(0, "originalFrame = %i", anim.originalFrame);
	if (anim.song) debug(0, "song = %i", anim.song);
}

byte *QueenCutaway::handleAnimation(byte *ptr, CutawayObject &object) {
	int frameCount = 0;
	int header = 0;

	// Read animation frames
	for (;;) {

		CutawayAnim tmp;      // make array
		header = (int16)READ_BE_UINT16(ptr);
		ptr += 2;

		if (-2 == header)
			break;

		debug(0, "Animation frame %i, header = %i", frameCount, header);

		if (header > 1000)
			error("Header too large");

		ptr = getCutawayAnim(ptr, header, tmp);
		dumpCutawayAnim(tmp);

		frameCount++;

		if (_quit)
			return NULL;
	}

	if (object.animType == 1) {
		// XXX
	}

	// Setup the SYNCHRO bob channels
	// XXX

	// Normal cutaway

	if (object.animType != 1) {
		// XXX
	}


	return ptr;
}

void QueenCutaway::handlePersonRecord(
		int index, 
		CutawayObject &object, 
		const char *sentence) {
	// Lines 1455-1516 in cutaway.c

	char name[MAX_STRING_SIZE];

	if (object.objectNumber == OBJECT_JOE) {
		if (object.moveToX || object.moveToY) {
			// XXX X=IX ; Y=IY;
			// XXX MOVE_JOE(0);
		} 
		strcpy(name, "JOE");
	}
	else {
		// Not Joe XXX
		warning("Person not Joe");
		strcpy(name, "unknown");
#if 0
		K=OBJECT-ROOM_DATA[ROOM];

		/* Find out which object position on the screen the person is */

		SET_PERSON_DATA(K,NULLstr,0);
		if(OBJ_CUT[13]>0 || OBJ_CUT[14]>0)
		{
			bobs[P_BNUM].scale=SF;
			bobs[P_BNUM].x=OBJ_CUT[13];
			bobs[P_BNUM].y=OBJ_CUT[14];
		}
		/* Add person's direction to be passed across */

		MOVE_OTHER(P_NAMEstr,IX,IY,CI+1,OBJECT_DATA[OBJECT][7]);
#endif
	}

	if (_quit)
		return;

	if (0 != strcmp(sentence, "*")) {
		if (sentence[0] == '#') {
			warning("Credit scripting system not yet implemented");
			// XXX Cinit(sentence + 1);
		}
		else {
			if (object.objectNumber > 0) {
				// XXX
				warning("Stuff not implemented");
#if 0
				MTCH=0;
				for(K=1;K<=PERSON_FACE_MAX;K++)
				{
					if(PERSON_FACE[K][0]==OBJECT) MTCH=1;
				}
				if(MTCH==0)
				{
					PERSON_FACE_MAX++;
					PERSON_FACE[PERSON_FACE_MAX][0]=OBJECT;
					PERSON_FACE[PERSON_FACE_MAX][1]=OBJECT_DATA[OBJECT][7];
				}
#endif
			}

			// XXX SPEAK(sentence, name, findCdCut(_basename, index));
		}

	}

	if (_quit)
		return;
}

void QueenCutaway::run() {

	byte *ptr = _objectData;

	int initialRoom = _queenLogic->currentRoom();
	int previousRoom = initialRoom;

	// XXX if(COMPANEL==0 || COMPANEL==2) SCENE_START(0);

	for (int i = 0; i < _cutawayObjectCount; i++) {
		CutawayObject object;
		ptr = getCutawayObject(ptr, object);
#if 1
		dumpCutawayObject(i, object);
#endif

		if (!object.moveToX && 
				!object.moveToY && 
				object.specialMove && 
				object.objectNumber >= 0) {
			specialMove(object.specialMove);
			object.specialMove = 0;
		}

		if (CURRENT_ROOM == object.room) {
			// Get current room
			object.room = _queenLogic->currentRoom();
		}
		else {
			// Change current room
			warning("Not yet able to change current room");
			// XXX queenLogic->currentRoom(object.room);
		}

		ptr = turnOnPeople(ptr, object);

		limitBob(object);

		char sentence[MAX_STRING_SIZE];
		_nextSentence = getString(_nextSentence, sentence, MAX_STRING_LENGTH);
		debug(0, "Sentence = '%s'", sentence);

		if (OBJECT_ROOMFADE == object.objectNumber) {
			// ROOMFADE = 1;
			object.objectNumber = OBJECT_JOE;
		}
		else {
			// ROOMFADE = 0;
		}

		if (object.room != previousRoom)
			changeRooms(object);

		ObjectType objectType = getObjectType(object);

		if (object.song) {
			debug(0, "Should play song %i here", object.song);
			// XXX playsong(object.song);
		}

		switch (objectType) {
			case OBJECT_TYPE_ANIMATION:
				ptr = handleAnimation(ptr, object);
				break;

			case OBJECT_TYPE_PERSON:
				handlePersonRecord(i, object, sentence);
				break;

			case OBJECT_TYPE_NO_ANIMATION:
				// Do nothing?
				break;

			default:
				error("Unhandled object type: %i", objectType);
				break;
		}

		if (_quit)
			break;
	}
}

