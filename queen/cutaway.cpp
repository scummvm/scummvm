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
#include "graphics.h"

namespace Queen {

/*
	 Functions needed:

	 queen.c:
	 
	 DISP_ROOM
	 FACE_JOE
	 FIND_GRAPHIC
	 FIND_SCALE
	 MOVE_JOE
	 MOVE_OTHER
	 SET_PERSON_DATA

	 talk.c:
	 
	 SPEAK


	 Data needed:

	 CURRSONG
	 GAMESTATE
	 JOEF					(Joe's face direction)
	 JX,JY        (Joe's coordintes)
	 PERSON_FACE
	 PERSON_FACE_MAX

 */

void Cutaway::run(
		const char *filename, 
		char *nextFilename,
		Logic *logic,
		Resource *resource) {
	Cutaway *cutaway = new Cutaway(filename, logic, resource);
	cutaway->run(nextFilename);
	delete cutaway;
}

Cutaway::Cutaway(
		const char *filename, 
		Logic *logic,
		Resource *resource) 
: _logic(logic), _quit(false), _lastSong(0), _songBeforeComic(0) {
	// XXX should not create this object ourselves
	_graphics = new Graphics(resource);
	memset(&_bankNames, 0, sizeof(_bankNames));
	load(filename, resource); 
}

Cutaway::~Cutaway() {
	// XXX only delete this if we created it
	delete _graphics;
	delete[] _fileData;
}

void Cutaway::load(const char *filename, Resource *resource) {
	byte *ptr;

	ptr = _fileData = resource->loadFile(filename, 20);
	if (!_fileData) {
		error("Failed to load resource data file '%s'", filename);
		_quit = true;
		return;
	}

	if (0 == scumm_stricmp(filename, "comic.cut"))
		/* XXX _songBeforeComic = CURRSONG */; 

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

	_anotherCutaway = (flags1 == 1);

	/*
		 Pointers to other places in the cutaway data
	 */

	_gameStatePtr       = _fileData + READ_BE_UINT16(ptr);
	ptr += 2;

	_nextSentence       = _fileData + READ_BE_UINT16(ptr);
	ptr += 2;

	byte *bankNamesPtr  = _fileData + READ_BE_UINT16(ptr);
	ptr += 2;
	debug(0, "Bank name file offset = %08x", 20 + bankNamesPtr - _fileData);

	_objectData = ptr;

	loadStrings(bankNamesPtr);

	if (_bankNames[0][0]) {
		debug(0, "Loading bank '%s'", _bankNames[0]);
		_graphics->bankLoad(_bankNames[0], CUTAWAY_BANK);
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

byte *Cutaway::getString(byte *ptr, char *str, int maxLength) {
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

void Cutaway::loadStrings(byte *ptr) {
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

byte *Cutaway::getCutawayObject(byte *ptr, CutawayObject &object)
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

void Cutaway::dumpCutawayObject(int index, CutawayObject &object)
{
	debug(0, "----- CutawayObject[%i] -----", index);

	const char *objectNumberStr;

	switch (object.objectNumber) {
		case -1:  objectNumberStr = "MESSAGE";  break;
		case 0:   objectNumberStr = "Joe";      break;
		case 548: objectNumberStr = "Anderson"; break;
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

void Cutaway::actionSpecialMove(int index) {

//	switch (index) {
//		default:
			warning("Unhandled special move: %i", index);
//			break;
//	}
}

byte *Cutaway::turnOnPeople(byte *ptr, CutawayObject &object) {
	// Lines 1248-1259 in cutaway.c
	object.personCount = (int16)READ_BE_UINT16(ptr);
	ptr += 2;

	if (object.personCount > MAX_PERSON_COUNT)
		error("[Cutaway::turnOnPeople] object.personCount > MAX_PERSON_COUNT");

	for (int i = 0; i < object.personCount; i++) {
		object.person[i] = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		debug(0, "[%i] Turn on person %i", i, object.person[i]);
	}

	return ptr;
}

void Cutaway::limitBob(CutawayObject &object) {
	if (object.limitBobX1) {

		if (object.objectNumber < 0) {
			warning("QueenCutaway::limitBob called with objectNumber = %i", object.objectNumber);
			return;
		}	
		
		BobSlot *bob = 
			_graphics->bob( _logic->findBob(object.objectNumber) );

		if (!bob) {
			warning("Failed to find bob");
			return;
		}

		bob->box.x1 = object.limitBobX1;
		bob->box.y1 = object.limitBobY1;
		bob->box.x2 = object.limitBobX2;
		bob->box.y2 = object.limitBobY2;
	}
}

void Cutaway::restorePersonData() {
	for (int i = 0; i < _personDataCount; i++) {
		int index           = _personData[i].index;
		ObjectData *objectData  = _logic->objectData(index);
		objectData->name        = _personData[i].name;
		objectData->image       = _personData[i].image;
	}
}

void Cutaway::changeRooms(CutawayObject &object) {
	// Lines 1291-1385 in cutaway.c

	restorePersonData();
	_personDataCount = 0;

	if (_finalRoom != object.room) {
		int firstObjectInRoom = _logic->roomData(object.room) + 1;
		int lastObjectInRoom  = _logic->roomData(object.room) + 0; // XXX _logic->objMax(object.room);

		for (int i = firstObjectInRoom; i <= lastObjectInRoom; i++) {
			ObjectData *objectData  = _logic->objectData(i);
			
			if (objectData->image == -3 || objectData->image == -4) {

				//  The object is a person! So record the details...
				_personData[_personDataCount].index = i;
				_personData[_personDataCount].name  = objectData->name;
				_personData[_personDataCount].image = objectData->image;
				_personDataCount++;

				// Now, check to see if(we need to keep the person on
				bool on = false;
				for (int j = 0; j < object.personCount; j++) {
					if (object.person[j] == i) {
						on = true;
						break;
					}
				}

				if (on) {
					// It is needed, so ensure it's ON
					objectData->name = abs(objectData->name);
				}
				else {
					// Not needed, so switch off!
					objectData->name = -abs(objectData->name);
				}

			}
		} // for()
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

	restorePersonData();

	// XXX CUTAWAY_SCALE(OBJECT);
	// XXX SF=Param;
	// XXX if(SCALE_FACTOR>0 && SCALE_FACTOR<100)
	// XXX   SF=SCALE_FACTOR;

}

Cutaway::ObjectType Cutaway::getObjectType(CutawayObject &object) {
	// Lines 1387-1449 in cutaway.c
	
	ObjectType objectType = OBJECT_TYPE_ANIMATION;

	if (object.objectNumber > 0) {
		if (!object.animList) {
			// No anim frames, so treat as a PERSON, ie. allow to speak/walk
			ObjectData *objectData = _logic->objectData(object.objectNumber);
			if (objectData->image == -3 || objectData->image == -4)
				objectType = OBJECT_TYPE_PERSON;
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
		/* Copy FROM_OBJECT into OBJECT */

		if(object.objectNumber != object.fromObject) {
			objectCopy(object.fromObject, object.objectNumber);
		}
		else {
			// Same object, so just turn it on!
			ObjectData *objectData = _logic->objectData(object.objectNumber);
			objectData->name = abs(objectData->name);
		}
		// XXX REDISP_OBJECT(OBJECT);

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

byte *Cutaway::getCutawayAnim(byte *ptr, int header, CutawayAnim &anim) {

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
		warning("Stuff not yet implemented in Cutaway::handleAnimation()");
		
		anim.object = _logic->findBob(header);

		// If fullscreen cutaway then clip to 199 down

		// 21/9/94, Make sure that bobs are clipped on 150 screens
		// XXX if(COMPANEL==2 && OBJ_CUT[6]<=0 && BDyres==200) bobs[Param].y2=199;

		anim.originalFrame = _logic->findFrame(header);
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

void Cutaway::dumpCutawayAnim(CutawayAnim &anim) {
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

byte *Cutaway::handleAnimation(byte *ptr, CutawayObject &object) {
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

void Cutaway::handlePersonRecord(
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

void Cutaway::run(char *nextFilename) {
	nextFilename[0] = '\0';

	byte *ptr = _objectData;

	_initialRoom = _temporaryRoom = _logic->currentRoom();

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
			actionSpecialMove(object.specialMove);
			object.specialMove = 0;
		}

		if (CURRENT_ROOM == object.room) {
			// Get current room
			object.room = _logic->currentRoom();
		}
		else {
			// Change current room
			_logic->currentRoom(object.room);
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

		if (object.room != _temporaryRoom)
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

		// XXX
#if 0
		if(ROOMFADE==1)
		{
			update();
			//CR 2 - CD-Rom
			if(ROOM>=114)
				fadein(0,255);
			else
				fadein(0,223);
			ROOMFADE=0;
		}
#endif 
	} // for()

	goToFinalRoom();

	_quit = false;

	updateGameState();

	_graphics->bankErase(CUTAWAY_BANK);

	talk(nextFilename);

	// XXX if(COMPANEL==0 || (COMPANEL==2 && ANOTHER_CUT==0)) {
	// XXX 	SCENE_END(1);
	// XXX 	COMPANEL=0;
	// XXX }

	if (nextFilename[0] == '\0' && !_anotherCutaway) {
		// Lines 2138-2182 in cutaway.c
		warning("Clean-up stuff needed but not yet implemented");
	}

	// XXX bobs[0].animating=0;
	// XXX bobs[0].moving=0;
	// Make sure Joe is clipped!
	// XXX bobs[0].y2=149;
	// XXX CUTON=0;
	_quit = false;

	if (_songBeforeComic > 0)
		/* XXX playsong(_songBeforeComic) */ ;
	else if (_lastSong > 0)
		/* XXX playsong(_lastSong) */ ;
}

void Cutaway::objectCopy(int dummyObjectIndex, int realObjectIndex) {
	// P3_COPY_FROM function in cutaway.c
	/* Copy data from Dummy (D) object to object (K)
		 If COPY_FROM Object images are greater than COPY_TO Object
		 images then swap the objects around. */

	ObjectData *dummyObject = _logic->objectData(dummyObjectIndex);
	ObjectData *realObject  = _logic->objectData(realObjectIndex);
	
	int fromState = (dummyObject->name < 0) ? -1 : 0;

	int frameCountReal = 1;
	int frameCountDummy = 1;

	int graphic = realObject->image;
	if (graphic > 0) {
		if (graphic > 5000)
			graphic -= 5000;

		// XXX FIND_GRAPHIC(graphic)
		// XXX if(EFRAME>0) frameCountReal=(EFRAME-SFRAME)+1;

		graphic = dummyObject->image;
		if (graphic > 0) {
			if (graphic > 5000)
				graphic -= 5000;

		// XXX FIND_GRAPHIC(graphic)
		// XXX if(EFRAME>0) frameCountDummy=(EFRAME-SFRAME)+1;

		}
	}

	ObjectData temp = *realObject;
	*realObject = *dummyObject;

	if (frameCountDummy > frameCountReal)
		*dummyObject = temp;

	realObject->name = abs(realObject->name);

	if  (fromState == -1)
		dummyObject->name = -abs(dummyObject->name);

	//  Make sure that WALK_OFF_DATA is copied too!

	for (int i = 1; i <= _logic->walkOffCount(); i++) {
		WalkOffData *walkOffData = _logic->walkOffData(i);
		if (walkOffData->entryObj == (int16)dummyObjectIndex) {
			walkOffData->entryObj = (int16)realObjectIndex;
			break;
		}
	}

}

void Cutaway::goToFinalRoom() {
	// Lines 1901-2032 in cutaway.c
	byte *ptr = _gameStatePtr;

	// Skipping GAMESTATE data
	int gameStateCount = (int16)READ_BE_UINT16(ptr); ptr += 2;
	if (gameStateCount > 0)
		ptr += (gameStateCount * 12);

	// Get the final room and Joe's final position

	uint16 joeRoom = READ_BE_UINT16(ptr); ptr += 2;
	uint16 joeX    = READ_BE_UINT16(ptr); ptr += 2;
	uint16 joeY    = READ_BE_UINT16(ptr); ptr += 2;

	if ((!_quit || (!_anotherCutaway && joeRoom != _finalRoom)) &&
			joeRoom != _temporaryRoom &&
			joeRoom != 0) {

		// XXX update global Joe coordinates
		// JX = joeX;
		// JY = joeY;

		_logic->currentRoom(joeRoom);
		_logic->oldRoom(_initialRoom);
		// XXX  DISP_ROOM(ROOM_NAMEstr[ROOM],3,0);
	}

	if (_quit) {
		// Lines 1927-2032 in cutaway.c
		
		// Stop the credits from running
		// XXX CFlag = 0;
		
		// Stop all moving bobs
		warning("Not stopping moving bobs yet");

		// XXX Loop person faces

		int quitObjectCount = (int16)READ_BE_UINT16(ptr); ptr += 2;

		for (int i = 0; i < quitObjectCount; i++) {
			int16 objectIndex  = (int16)READ_BE_UINT16(ptr); ptr += 2;
			int16 from    = (int16)READ_BE_UINT16(ptr); ptr += 2;
			/*int16 x       = (int16)READ_BE_UINT16(ptr);*/ ptr += 2;
			/*int16 y       = (int16)READ_BE_UINT16(ptr);*/ ptr += 2;
			int16 room    = (int16)READ_BE_UINT16(ptr); ptr += 2;
			/*int16 frame   = (int16)READ_BE_UINT16(ptr);*/ ptr += 2;

			// XXX int bob = _logic->findBob(objectIndex);

			if (from > 0) {
				// XXX
			}

			ObjectData *objectData = _logic->objectData(objectIndex);

			if (objectData->room == room) {
				// XXX
			}
		} // for()
		
		int16 specialMove = (int16)READ_BE_UINT16(ptr); ptr += 2;
		actionSpecialMove(specialMove);

		_lastSong = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}

	if (joeRoom == _temporaryRoom &&
			joeRoom != 37 &&    // XXX hard coded room number
			joeRoom != 105 &&   // XXX hard coded room number
			joeRoom != 106 &&   // XXX hard coded room number
			(joeX || joeY)) {
		// XXX bobs[0].x=J_X;
		// XXX bobs[0].y=J_Y;
		// XXX FIND_SCALE(J_X,J_Y);
		// XXX SFACTOR=Param;
		// XXX FACE_JOE()
	}
}

void Cutaway::updateGameState() {
	// Lines 2047-2115 in cutaway.c
	byte *ptr = _gameStatePtr;

	int gameStateCount = (int16)READ_BE_UINT16(ptr); ptr += 2;

	for (int i = 0; i < gameStateCount; i++) {
		int16 stateIndex    = (int16)READ_BE_UINT16(ptr); ptr += 2;
		int16 stateValue    = (int16)READ_BE_UINT16(ptr); ptr += 2;
		int16 objectIndex   = (int16)READ_BE_UINT16(ptr); ptr += 2;
		int16 areaIndex     = (int16)READ_BE_UINT16(ptr); ptr += 2;
		int16 areaSubIndex  = (int16)READ_BE_UINT16(ptr); ptr += 2;
		int16 fromObject    = (int16)READ_BE_UINT16(ptr); ptr += 2;

		bool update = false;

		if (stateIndex > 0) {
			if(_logic->gameState(stateIndex) == stateValue) 
				update = true;
		}
		else {
			_logic->gameState(abs(stateIndex), stateValue);
			update = true;
		}

		if (update) {

			// Show or hide an object

			if (objectIndex > 0) {                    // Show the object
				ObjectData *objectData  = _logic->objectData(objectIndex);
				objectData->name        = abs(objectData->name);
				if (fromObject > 0)
					objectCopy(fromObject, objectIndex);
				// XXX REDISP_OBJECT(objectIndex);
			}
			else if (objectIndex < 0) {               // Hide the object
				objectIndex             = -objectIndex;
				ObjectData *objectData  = _logic->objectData(objectIndex);
				objectData->name        = -abs(objectData->name);
				// XXX REDISP_OBJECT(objectIndex);
			}

			if (areaIndex > 0) {

				// Turn area on or off

				if (areaSubIndex > 0) {
					int16* area = _logic->area(areaIndex, areaSubIndex);
					area[0] = abs(area[0]);
				}
				else {
					int16* area = _logic->area(areaIndex, abs(areaSubIndex));
					area[0] = -abs(area[0]);
				}
			}

		}
	} // for()
}

// XXX this function could probably be useful at other places too
static char *right(char *str, int count) {
	// This function does _not_ use static data (the implementation in talk.c does!)
	int length = strlen(str);
	if (count > length)
		return str;
	else
		return str + length - count;
}

void Cutaway::talk(char *nextFilename) {
	// Lines 2119-2131 in cutaway.c
	
	if (0 == scumm_stricmp(right(_talkFile, 4), ".dog")) {
		warning("Cutaway::talk() needed but not yet implemented");
		nextFilename[0] = '\0';
	}
}

} // End of namespace Queen
