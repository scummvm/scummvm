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
#include "queen/cutaway.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/input.h"
#include "queen/sound.h"
#include "queen/talk.h"
#include "queen/walk.h"

namespace Queen {

/*
   TODO soon

   - Implement SCENE_START and SCENE_END

   - Finish Cutaway::actionSpecialMove


   TODO later

   - Finish Cutaway::stop

   - Show credits

   - Play voices

   - Play songs 

 */

void Cutaway::run(
		const char *filename, 
		char *nextFilename,
		Graphics *graphics,
		Input *input,
		Logic *logic,
		Resource *resource,
		Sound *sound) {
	Cutaway *cutaway = new Cutaway(filename, graphics, input, logic, resource, sound);
	cutaway->run(nextFilename);
	delete cutaway;
}

Cutaway::Cutaway(
		const char *filename, 
		Graphics *graphics,
		Input *input,
		Logic *logic,
		Resource *resource,
		Sound *sound) 
: _graphics(graphics), _input(input), _logic(logic), _resource(resource), _sound(sound), _walk(logic->walk()),
	_personDataCount(0), _personFaceCount(0), _lastSong(0), _songBeforeComic(0) {
	memset(&_bankNames, 0, sizeof(_bankNames));
	_input->cutawayQuitReset();
	load(filename); 
}

Cutaway::~Cutaway() {
	delete[] _fileData;
}

void Cutaway::load(const char *filename) {
	byte *ptr;

	debug(0, "----- Cutaway::load(\"%s\") -----", filename);

	ptr = _fileData = _resource->loadFile(filename, 20);
	if (!_fileData) {
		error("Failed to load resource data file '%s'", filename);
	}

	if (0 == scumm_stricmp(filename, "comic.cut"))
		/* XXX _songBeforeComic = CURRSONG */; 

	strcpy(_basename, filename);
	_basename[strlen(_basename)-4] = '\0';

	_comPanel = READ_BE_UINT16(ptr);
	ptr += 2;
	debug(0, "_comPanel = %i", _comPanel);

	_cutawayObjectCount = (int16)READ_BE_UINT16(ptr);
	ptr += 2;
	debug(0, "_cutawayObjectCount = %i", _cutawayObjectCount);

	if (_cutawayObjectCount < 0) {
		_cutawayObjectCount = -_cutawayObjectCount;
		_input->canQuit(false);
	}
	else
		_input->canQuit(true);

	int16 flags1 = (int16)READ_BE_UINT16(ptr);
	ptr += 2;
	debug(0, "flags1 = %i", flags1);

	if (flags1 < 0) {
		_logic->entryObj(0);
		_finalRoom = -flags1;
	}
	else
		_finalRoom = PREVIOUS_ROOM;

	_anotherCutaway = (flags1 == 1);

	debug(0, "[Cutaway::load] _finalRoom      = %i", _finalRoom);
	debug(0, "[Cutaway::load] _anotherCutaway = %i", _anotherCutaway);

	/*
		 Pointers to other places in the cutaway data
	 */

	_gameStatePtr       = _fileData + READ_BE_UINT16(ptr);
	ptr += 2;

	_nextSentence       = _fileData + READ_BE_UINT16(ptr);
	ptr += 2;

	byte *bankNamesPtr  = _fileData + READ_BE_UINT16(ptr);
	ptr += 2;
	//debug(0, "Bank name file offset = %08x", 20 + bankNamesPtr - _fileData);

	_objectData = ptr;

	loadStrings(bankNamesPtr);

	if (_bankNames[0][0]) {
		debug(0, "Loading bank '%s'", _bankNames[0]);
		_graphics->bankLoad(_bankNames[0], CUTAWAY_BANK);
	}

	char entryString[MAX_STRING_SIZE];
	_nextSentence = Talk::getString(_nextSentence, entryString, MAX_STRING_LENGTH);
	debug(0, "Entry string = '%s'", entryString);

	if (entryString[0] == '*' &&
			entryString[1] == 'F' &&
			entryString[3] == '\0') {
		warning("[Cutaway::load] CUTJOEF not handled");
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

void Cutaway::loadStrings(byte *ptr) {
	int i,j;

	int bankNameCount = READ_BE_UINT16(ptr);
	ptr += 2;

	//debug(0, "Bank name count = %i", bankNameCount);

	/*
		 The _bankNames zero-based array is the one-based BANK_NAMEstr array in
		 the original source code.
	 */

	for (i = 0, j = 0; i < bankNameCount; i++) {
		ptr = Talk::getString(ptr, _bankNames[j], MAX_FILENAME_LENGTH);

		if (_bankNames[j][0]) {
			//debug(0, "Bank name %i = '%s'", j, _bankNames[j]);
			j++;
		}
	}

	debug(0, "Getting talk file");
	ptr = Talk::getString(ptr, _talkFile, MAX_FILENAME_LENGTH);
	debug(0, "Talk file = '%s'", _talkFile);

	_talkTo = (int16)READ_BE_UINT16(ptr);
	ptr += 2;
	debug(0, "_talkTo = %i", _talkTo);
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
	if (object.limitBobX1 < 0) {
		object.song = -object.limitBobX1;
		object.limitBobX1 = 0;
	}
	else
		object.song = 0;

	return ptr;
}

void Cutaway::dumpCutawayObject(int index, CutawayObject &object)
{
	debug(0, "----- CutawayObject[%i] -----", index);

	const char *objectNumberStr;

	switch (object.objectNumber) {
		case -1:  
			objectNumberStr = "MESSAGE";  break;
		case 0:   
			objectNumberStr = "Joe";      break;
		default:
			if (object.objectNumber > 0)
				objectNumberStr = _logic->objectName(abs(_logic->objectData(object.objectNumber)->name));
			else
				objectNumberStr = "Unknown!";
		  break;
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

	if (index <= 0)
		return;
	
	debug(0, "Special move: %i", index);

	switch (index) {

		case 2:
			_logic->joeUseDress(false);
			break;

		// c74a.cut - use clothes
		case 3:
			_logic->joeUseClothes(false);
			break;

		case 4:
			_logic->joeUseUnderwear();
			break;

		// cdres.cut
		case 5:
			_logic->display()->palSetJoe(JP_DRESS);
			break;

		// cdclo.cut - Set Joe's palette
		case 6:
			_logic->display()->palSetJoe(JP_CLOTHES);
			break;

		// c73e.cut - carbam background animation
		case 7:
			_graphics->initCarBamScene();
			break;

		// c74a.cut - Turn off big oil splat and gun shots!
		case 8:
			_graphics->cleanupCarBamScene(_logic->findBob(594)); // Oil object
			break;

		// cdint.cut - put camera on Joe
		case 16:
			_graphics->cameraBob(0);
			break;

		// c75b.cut - Lightning hits plane
		case 28:
			{
				_graphics->cameraBob(-1);
				
				short iy = 0, x, ydir = -1, j, k;
				
				BobSlot *planeBob       = _graphics->bob(5);
				BobSlot *lightningBob   = _graphics->bob(20);

				planeBob->box.y2 = lightningBob->box.y2 = 199;
				planeBob->y = 135;

				planeBob->scale = 20;

				for (x = 660; x > 163; x -= 6) {
					planeBob->x = x;
					planeBob->y = 135 + iy;

					iy -= ydir;
					if (iy < -9 || iy > 9)
						ydir = -ydir;

					planeBob->scale++;
					if (planeBob->scale > 100)
						planeBob->scale = 100;

					int scrollX = x - 163;
					if (scrollX > 320)
						scrollX = 320;
					_logic->display()->horizontalScroll(scrollX);

					_logic->update();
				}

				planeBob->scale = 100;
				_logic->display()->horizontalScroll(0);

				planeBob->x -= -8;
				planeBob->y += 6;

				lightningBob->x = 160;
				lightningBob->y = 0;

				// 23/2/95 - Play lightning SFX
				// XXX sfxplay(NULLstr);

				_graphics->bankUnpack(18, lightningBob->frameNum, 15);
				_graphics->bankUnpack(4,  planeBob    ->frameNum, 15);

				// Plane plunges into the jungle!

				BobSlot *fireBob = _graphics->bob(6);

				fireBob->animating = true;
				fireBob->x = planeBob->x;
				fireBob->y = planeBob->y + 10;
				
				_graphics->bankUnpack(19, fireBob->frameNum, 15);
				_logic->update();

				k = 20;
				j = 1;

				for (x = 163; x > -30; x -= 10) {
					planeBob->y += 4;
					fireBob->y += 4;
					planeBob->x = fireBob->x = x;

					if (k < 40) {
						_graphics->bankUnpack(j, planeBob->frameNum, 15);
						_graphics->bankUnpack(k, fireBob ->frameNum, 15);
						k++;
						j++;

						if (j == 4)
							j = 1;
					}
					
					_logic->update();
				}

				_graphics->cameraBob(0);
			}
			break;
			
		// c74a.cut - Wait for car to reach correct position before pouring oil
		case 31:
			while (_graphics->bamData()->index != 60) {
				_logic->update();
			}
			break;

		// c75b.cut - Screen shake
		case 32:
			// TODO implement
			break;

		// cred.cut - scale title
		case 34:
			{
				BobSlot *bob = _graphics->bob(5);
				bob->animating = false;
				bob->x = 161;
				bob->y = 200;
				bob->scale = 100;

				int i;
				for (i = 5; i <= 100; i +=5) {
					bob->scale = i;
					bob->y -= 4;
					_logic->update();
				}
			}
			break;

		// cdint.cut - pan right fast
		case 36:
			{
				Display *display = _logic->display();

				BobSlot *bob_thugA1 = _graphics->bob(20);
				BobSlot *bob_thugA2 = _graphics->bob(21);
				BobSlot *bob_thugA3 = _graphics->bob(22);
				BobSlot *bob_hugh1  = _graphics->bob(1);
				BobSlot *bob_hugh2  = _graphics->bob(23);
				BobSlot *bob_hugh3  = _graphics->bob(24);
				BobSlot *bob_thugB1 = _graphics->bob(25);
				BobSlot *bob_thugB2 = _graphics->bob(26);

				_graphics->cameraBob(-1);
				_input->fastMode(true);
				_logic->update();
				
				int i = 4, k = 160;

				// Adjust thug1 gun so it matches rest of body
				bob_thugA1->x += (k / 2) * 2 - 45; 
				bob_thugA2->x += (k / 2) * 2; 
				bob_thugA3->x += (k / 2) * 2;

				bob_hugh1->x += (k / 2) * 3 + (k / 2);
				bob_hugh2->x += (k / 2) * 3 + (k / 2);
				bob_hugh3->x += (k / 2) * 3 + (k / 2);

				bob_thugB1->x += (k / 2) * 4 + k; 
				bob_thugB2->x += (k / 2) * 4 + k; 

				if (i == 3) {
					bob_thugB1->x += 10;
					bob_thugB2->x += 10;
				}

				i *= 2;

				int horizontalScroll = 0;

				while (horizontalScroll < k) {

					horizontalScroll = horizontalScroll + i;
					if (horizontalScroll > k)
						horizontalScroll = k;

					//debug(0, "horizontalScroll = %i", horizontalScroll);

					display->horizontalScroll(horizontalScroll);

					bob_thugA1->x -= i * 2; 
					bob_thugA2->x -= i * 2; 
					bob_thugA3->x -= i * 2;

					bob_hugh1->x -= i * 3;
					bob_hugh2->x -= i * 3;
					bob_hugh3->x -= i * 3;

					bob_thugB1->x -= i * 4;
					bob_thugB2->x -= i * 4;

					_logic->update();

					if (_input->cutawayQuit())
						return;
				}

				_input->fastMode(false);
			}
			break;

		// cdint.cut - flash white
		case 37:
			_logic->display()->palCustomFlash();
			break;

		// cdint.cut - pan right
		case 38:
			{
				Display *display = _logic->display();

				BobSlot *bob_box   = _graphics->bob(20);
				BobSlot *bob_beam  = _graphics->bob(21);
				BobSlot *bob_crate = _graphics->bob(22);
				BobSlot *bob_clock = _graphics->bob(23);
				BobSlot *bob_hands = _graphics->bob(24);

				_graphics->cameraBob(-1);
				_input->fastMode(true);
					
				_logic->update();

				bob_box  ->x += 280 * 2;
				bob_beam ->x += 30;
				bob_crate->x += 180 * 3;

				int horizontalScroll = display->horizontalScroll();

				int i = 1;
				while (horizontalScroll < 290) {

					horizontalScroll = horizontalScroll + i;
					if (horizontalScroll > 290)
						horizontalScroll = 290;

					//debug(0, "horizontalScroll = %i", horizontalScroll);

					display->horizontalScroll(horizontalScroll);

					bob_box  ->x -= i * 2;
					bob_beam ->x -= i;
					bob_crate->x -= i * 3;
					bob_clock->x -= i * 2;
					bob_hands->x -= i * 2;

					_logic->update();

					if (_input->cutawayQuit())
						return;

				}

				_input->fastMode(false);
			}
			break;

		// cdint.cut - pan left to bomb
		case 39: 
			{
				Display *display = _logic->display();

				BobSlot *bob21 = _graphics->bob(21);
				BobSlot *bob22 = _graphics->bob(22);

				_graphics->cameraBob(-1);
				_input->fastMode(true);
				
				int horizontalScroll = display->horizontalScroll();

				int i = 5;
				while (horizontalScroll > 0 || bob21->x < 136) {

					horizontalScroll -= i;
					if (horizontalScroll < 0)
						horizontalScroll = 0;

					//debug(0, "horizontalScroll = %i", horizontalScroll);
					display->horizontalScroll(horizontalScroll);

					if (horizontalScroll < 272 && bob21->x < 136)
						bob21->x += (i/2);

					bob22->x += i;

					_logic->update();

					if (_input->cutawayQuit())
						return;

				}

				_input->fastMode(false);
			}
			break;

		default:
			warning("Unhandled special move: %i", index);
			break;
	}
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
		//debug(0, "[%i] Turn on person %i", i, object.person[i]);
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

	debug(0, "Changing from room %i to room %i", 
			_temporaryRoom, 
			object.room);

	restorePersonData();
	_personDataCount = 0;

	if (_finalRoom != object.room) {
		int firstObjectInRoom = _logic->roomData(object.room) + 1;
		int lastObjectInRoom  = _logic->roomData(object.room) + _logic->objMax(object.room);

		for (int i = firstObjectInRoom; i <= lastObjectInRoom; i++) {
			ObjectData *objectData  = _logic->objectData(i);
			
			if (objectData->image == -3 || objectData->image == -4) {

				//  The object is a person! So record the details...
				_personData[_personDataCount].index = i;
				_personData[_personDataCount].name  = objectData->name;
				_personData[_personDataCount].image = objectData->image;
				_personDataCount++;

				// Now, check to see if we need to keep the person on
				bool on = false;
				for (int j = 0; j < object.personCount; j++) {
					if (object.person[j] == i) {
						on = true;
						break;
					}
				}

				/*debug(0, "Person '%s' (%i) is %s", 
						_logic->objectName(objectData->name),
						objectData->name,
						on ? "on" : "off");*/

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

	_logic->joeX(0);
	_logic->joeY(0);

	for (int i = 0; i < object.personCount; i++) {
		if (PERSON_JOE == object.person[i]) {
			_logic->joeX(object.bobStartX);
			_logic->joeY(object.bobStartY);
		}
	}

	_logic->oldRoom(_initialRoom);

	RoomDisplayMode mode;

	if (!_logic->joeX() && !_logic->joeY()) {
		mode = RDM_FADE_NOJOE;
	}
	else {
		// We need to display Joe on screen
		if (_roomFade == 1)
			mode = RDM_NOFADE_JOE;
		else
			mode = RDM_FADE_JOE_XY;
	}

	_logic->roomDisplay(_logic->roomName(_logic->currentRoom()), mode, 0, _comPanel, true);

	_currentImage = _logic->numFrames();

	_temporaryRoom = _logic->currentRoom();

	restorePersonData();
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
			_logic->objectCopy(object.fromObject, object.objectNumber);
		}
		else {
			// Same object, so just turn it on!
			ObjectData *objectData = _logic->objectData(object.objectNumber);
			objectData->name = abs(objectData->name);
		}

		_logic->roomRefreshObject(object.objectNumber);

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
	// lines 1531-1607 in cutaway.c

	//debug(0, "[Cutaway::getCutawayAnim] header=%i", header);

	anim.currentFrame = 0;
	anim.originalFrame = 0;

	if (-1 == header)
		header = 0;

	if (0 == header) {
		anim.object = 0;
		anim.originalFrame = 29 + FRAMES_JOE_XTRA;
		
		// 21/9/94, Make sure that bobs are clipped on 150 screens
		if (_logic->display()->fullscreen())
			_graphics->bob(0)->box.y2 = 199;
	}
	else {
		//warning("Stuff not yet implemented in Cutaway::getCutawayAnim()");
		
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

	if (anim.bank == 0) {
		anim.bank = 15;
	}
	else {
		if (anim.bank != 13) {
			/* XXX if (OLDBANK != T) */ {
				//debug(0, "Loading bank '%s'", _bankNames[anim.bank-1]);
				_graphics->bankLoad(_bankNames[anim.bank-1], 8);
				// XXX OLDBANK=T;
			}

			anim.bank = 8;
		}
		else {
			// Make sure we ref correct JOE bank (7)
			anim.bank = 7;
		}
	}

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

	if (_resource->isDemo()) {
		anim.song = 0;
	}
	else {
		anim.song = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
	}

	// Extract information that depend on the signedness of values
	if (anim.unpackFrame < 0) {
		anim.flip = true;
		anim.unpackFrame = -anim.unpackFrame;
	}
	else
		anim.flip = false;

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
	// lines 1517-1770 in cutaway.c
	int frameCount = 0;
	int header = 0;
	int i;
		
	CutawayAnim objAnim[56];

	// Read animation frames
	for (;;) {

		header = (int16)READ_BE_UINT16(ptr);
		ptr += 2;

		if (-2 == header)
			break;

		//debug(0, "Animation frame %i, header = %i", frameCount, header);

		if (header > 1000)
			error("Header too large");

		ptr = getCutawayAnim(ptr, header, objAnim[frameCount]);
		//dumpCutawayAnim(objAnim[frameCount]);

		frameCount++;

		if (_input->cutawayQuit())
			return NULL;
	}

	if (object.animType == 1) {
		// lines 1615-1636 in cutaway.c
		
		debug(0, "----- Complex cutaway animation (animType = %i) -----", object.animType);

		if (/*(P_BNUM==1) &&*/ (_logic->currentRoom() == 47 || _logic->currentRoom() == 63)) {
			// The oracle
			warning("The oracle is not yet handled");
		}
		else {
			_currentImage = makeComplexAnimation(_currentImage, objAnim, frameCount);
		}

		if (object.bobStartX || object.bobStartY) {
			BobSlot *bob = _graphics->bob(objAnim[0].object);
			bob->x = object.bobStartX;
			bob->y = object.bobStartY;
		}
	}

	// Setup the SYNCHRO bob channels

	for (i = 0; i < frameCount; i++) {
		if (objAnim[i].mx || objAnim[i].my) {
			BobSlot *bob = _graphics->bob(objAnim[i].object);
			bob->frameNum = objAnim[i].originalFrame;
			_graphics->bobMove(
					objAnim[i].object, 
					objAnim[i].mx, 
					objAnim[i].my,
					(object.specialMove > 0) ? object.specialMove : 4);
		}
	}

	// Boat room hard coded
	if (_logic->currentRoom() == ROOM_TEMPLE_OUTSIDE) {
		BobSlot *bob = _graphics->bob(0);
		if (bob->x < 320)
			_graphics->bobMove(
					0, 
					bob->x + 346, 
					bob->y,
					4);
	}

	// Normal cutaway

	if (object.animType != 1) {
		// lines 1657-1761 in cutaway.c

		debug(0, "----- Normal cutaway animation (animType = %i) -----", object.animType);
		
		for (i = 0; i < frameCount; i++) {
			//debug(0, "===== Animating frame %i =====", i);
			//dumpCutawayAnim(objAnim[i]);

			BobSlot *bob = _graphics->bob(objAnim[i].object);
			bob->active = true;
			if (bob->animating) {
				bob->animating = false;
				bob->frameNum = objAnim[i].originalFrame;
			}

			if (objAnim[i].object < 4)
				bob->frameNum = 29 + objAnim[i].object + FRAMES_JOE_XTRA;

			if (objAnim[i].unpackFrame == 0) {
				// Turn off the bob
				bob->active = false;
			}
			else {
				if (object.animType == 2 || object.animType == 0) {
					// Unpack animation, but do not unpack moving people

					if (!((objAnim[i].mx > 0 || objAnim[i].my > 0) && InRange(objAnim[i].object, 1, 3))) {
						/*debug(0, "Animation - bankUnpack(%i, %i, %i);",
								objAnim[i].unpackFrame, 
								objAnim[i].originalFrame,
								objAnim[i].bank);*/
						_graphics->bankUnpack(
								objAnim[i].unpackFrame, 
								objAnim[i].originalFrame,
								objAnim[i].bank);
					}

					if (0 == objAnim[i].object) {
						// Scale Joe
						bob->scale = scale(object);
					}
				}

				if (objAnim[i].cx || objAnim[i].cy) {
					bob->x = objAnim[i].cx;
					bob->y = objAnim[i].cy;
				}

				// Only flip if we are not moving or it is not a person object
				if (!(objAnim[i].object > 0 && objAnim[i].object < 4) ||
						!(objAnim[i].mx || objAnim[i].my) )
					bob->xflip = objAnim[i].flip;

				// Add frame alteration
				if (!(objAnim[i].object > 0 && objAnim[i].object < 4)) {
					bob->frameNum = objAnim[i].originalFrame;
				}

				int j;
				for (j = 0; j < objAnim[i].speed; j++)
					_logic->update();
			}

			if (_input->cutawayQuit())
				return NULL;

			if (objAnim[i].song > 0)
				_sound->playSong(objAnim[i].song);

			// Load but don't play
			if(objAnim[i].song < 0) {
				// XXX loadnoplay=1;
				// XXX playsong(abs(OBJ_ANIM[J][11]));
				// XXX loadnoplay=0;
			}

		} // for()
	}

	bool moving = true;

	while (moving) {
		moving = false;
		_logic->update();
		
		for (i = 0; i < frameCount; i++) {
			BobSlot *bob = _graphics->bob(objAnim[i].object);
			if (bob->moving) {
				moving = true;
				break;
			}
		}

		if (_input->cutawayQuit())
			return NULL;
	}

	return ptr;
}

static void findCdCut(const char *basename, int index, char *result) {
	// Function find_cd_cut, lines 5-15 in execute.c

	strcpy(result, basename);
	for (int i = strlen(basename); i < 5; i++)
		result[i] = '_';
	snprintf(result + 5, 3, "%02i", index);
	//debug(0, "findCdCut(\"%s\", %i, \"%s\")", basename, index, result);
}

void Cutaway::handlePersonRecord(
		int index, 
		CutawayObject &object, 
		const char *sentence) {
	// Lines 1455-1516 in cutaway.c

	Person p;

	_personFaceCount = 0;	//Hello, please verify me. (Fixes crash on OSX)

	if (object.objectNumber == OBJECT_JOE) {
		if (object.moveToX || object.moveToY) {
			_walk->joeMove(0, object.moveToX, object.moveToY, true);
		} 
	}
	else {
		_logic->personSetData(
				object.objectNumber - _logic->roomData(_logic->currentRoom()), 
				"", true, &p);

		if (object.bobStartX || object.bobStartY) {
			BobSlot *bob = _graphics->bob(p.actor->bobNum);
			bob->scale = scale(object);
			bob->x = object.bobStartX;
			bob->y = object.bobStartY;
		}

		if (object.moveToX || object.moveToY)
			_walk->personMove(
					&p, 
					object.moveToX, object.moveToY,
					_currentImage + 1, 		// XXX CI+1
					_logic->objectData(object.objectNumber)->image
					);
	}

	if (_input->cutawayQuit())
		return;

	if (0 != strcmp(sentence, "*")) {
		if (sentence[0] == '#') {
			warning("Credit scripting system not yet implemented");
			// XXX Cinit(sentence + 1);
		}
		else {
			if (object.objectNumber > 0) {
				bool foundPerson = false;

				for (int i = 1; i <= _personFaceCount; i++) {
					if (_personFace[i].index == object.objectNumber) {
						foundPerson = true;
						break;
					}
				}

				if (!foundPerson) {
					_personFaceCount++;
					_personFace[_personFaceCount].index = object.objectNumber;
					_personFace[_personFaceCount].image = _logic->objectData(object.objectNumber)->image;
				}
			}

			char voiceFilePrefix[MAX_STRING_SIZE];
			findCdCut(_basename, index, voiceFilePrefix);
			Talk::speak(sentence, (object.objectNumber == OBJECT_JOE) ? NULL : &p, voiceFilePrefix,
				_graphics, _input, _logic, _resource, _sound);
		}

	}

	if (_input->cutawayQuit())
		return;
}

void Cutaway::run(char *nextFilename) {
	int i;
	nextFilename[0] = '\0';

	_currentImage = _logic->numFrames();

	BobSlot *joeBob = _graphics->bob(0);
	int initialJoeX = joeBob->x;
	int initialJoeY = joeBob->y;
	debug(0, "[Cutaway::run] Joe started at (%i, %i)", initialJoeX, initialJoeY);

	_input->cutawayRunning(true);

	_initialRoom = _temporaryRoom = _logic->currentRoom();

	// FIXME: hack to hide the panel *before* displaying a talking head.
	// This was not handled in the original game, but I think it is 
	// better like that.
	if (_talkTo != 0) {
		_comPanel = 2;
	}

	_logic->display()->screenMode(_comPanel, true);

	if (_comPanel == 0 || _comPanel == 2) {
		_logic->sceneStart();
	}

	byte *ptr = _objectData;

	for (i = 0; i < _cutawayObjectCount; i++) {
		CutawayObject object;
		ptr = getCutawayObject(ptr, object);
		//dumpCutawayObject(i, object);

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
		_nextSentence = Talk::getString(_nextSentence, sentence, MAX_STRING_LENGTH);
		//debug(0, "Sentence = '%s'", sentence);

		if (OBJECT_ROOMFADE == object.objectNumber) {
			_roomFade = true;
			object.objectNumber = OBJECT_JOE;
		}
		else {
			_roomFade = false;
		}

		if (object.room != _temporaryRoom)
			changeRooms(object);

		ObjectType objectType = getObjectType(object);

		if (object.song)
			_sound->playSong(object.song);

		switch (objectType) {
			case OBJECT_TYPE_ANIMATION:
				ptr = handleAnimation(ptr, object);
				break;

			case OBJECT_TYPE_PERSON:
				handlePersonRecord(i + 1, object, sentence);
				break;

			case OBJECT_TYPE_NO_ANIMATION:
				// Do nothing?
				break;

			case OBJECT_TYPE_TEXT_SPEAK:
			case OBJECT_TYPE_TEXT_DISPLAY_AND_SPEAK:
			case OBJECT_TYPE_TEXT_DISPLAY:
				handleText(i + 1, objectType, object, sentence);
				break;

			default:
				warning("Unhandled object type: %i", objectType);
				break;
		}

		if (_input->cutawayQuit())
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

	stop();

	_input->cutawayQuitReset();

	updateGameState();

	_graphics->bankErase(CUTAWAY_BANK);

	talk(nextFilename);

	if (_comPanel == 0 || (_comPanel == 2 && !_anotherCutaway)) {
		_logic->sceneStop();
		_comPanel = 0;
	}

	if (nextFilename[0] == '\0' && !_anotherCutaway) {
		_logic->display()->fullscreen(false);

		// Lines 2138-2182 in cutaway.c
		if (_finalRoom) {
			_logic->newRoom(0);
			_logic->entryObj(0);
		}
		else {
			/// No need to stay in current room, so return to previous room
			//  if one exists. Reset Joe's X,Y coords to those when first entered 

			restorePersonData();

			debug(0, "_logic->entryObj() = %i", _logic->entryObj());
			if (_logic->entryObj() > 0) {
				_initialRoom = _logic->objectData(_logic->entryObj())->room;
			}
			else {
				// We're not returning to new room, so return to old Joe X,Y coords
				debug(0, "[Cutaway::run] Moving joe to (%i, %i)", initialJoeX, initialJoeY);
				_logic->joeX(initialJoeX);
				_logic->joeY(initialJoeY);
			}

			if (_logic->currentRoom() != _initialRoom) {
				// XXX should call SETUP_ROOM here but that would introduce a
				// circual dependency, so we try to set newRoom to the room
				// instead
				debug(0, "[Cutaway::run] Not calling SETUP_ROOM here, just setting newRoom to %i", _initialRoom);
				_logic->newRoom(_initialRoom);
				_logic->display()->fullscreen(true);
			}
			else {
				_logic->joeX(0);
				_logic->joeY(0);
			}
		}

		// XXX CUTJOEF=0;
		_comPanel = 0;

		int k = 0;
		for (i = _logic->roomData(_logic->currentRoom());
				i <= _logic->roomData(_logic->currentRoom() + 1); i++) {

			ObjectData *object = _logic->objectData(i);			
			if (object->image == -3 || object->image == -4) {
				k++;
				if (object->name > 0) {
					BobSlot *bs = _graphics->bob(k);
					bs->animating = true;
					bs->animReset();
				}
			}
		}

		// function CUTAWAY_SPECIAL(), lines 885-896 in cutaway.c
		if (_logic->currentRoom() == 1 && _logic->gameState(3) == 0) {
			// XXX hard-coded room and inventory items
			_logic->inventoryDeleteItem(ITEM_CROWBAR, false);
			_logic->inventoryDeleteItem(ITEM_DRESS, false);
			_logic->inventoryDeleteItem(ITEM_CLOTHES, false);
			_logic->inventoryDeleteItem(ITEM_HAY, false);
			_logic->inventoryDeleteItem(ITEM_OIL, false);
			_logic->inventoryDeleteItem(ITEM_CHICKEN, false);
			_logic->gameState(3, 1);
			_logic->inventoryRefresh();
		}

	}

	joeBob->animating = 0;
	joeBob->moving    = 0;
	// Make sure Joe is clipped!
	joeBob->box.y2    = 149;

	_input->cutawayRunning(false);
	_input->cutawayQuitReset();

	if (_songBeforeComic > 0)
		_sound->playSong(_songBeforeComic);
	else if (_lastSong > 0)
		_sound->playSong(_lastSong);
}

void Cutaway::stop() {
	// Lines 1901-2032 in cutaway.c
	byte *ptr = _gameStatePtr;

	// Skipping GAMESTATE data
	int gameStateCount = (int16)READ_BE_UINT16(ptr); ptr += 2;
	if (gameStateCount > 0)
		ptr += (gameStateCount * 12);

	// Get the final room and Joe's final position

	int16 joeRoom = READ_BE_UINT16(ptr); ptr += 2;
	int16 joeX    = READ_BE_UINT16(ptr); ptr += 2;
	int16 joeY    = READ_BE_UINT16(ptr); ptr += 2;
	
	debug(0, "[Cutaway::stop] Final position is room %i and coordinates (%i, %i)", 
			joeRoom, joeX, joeY);

	if ((!_input->cutawayQuit() || (!_anotherCutaway && joeRoom == _finalRoom)) &&
			joeRoom != _temporaryRoom &&
			joeRoom != 0) {
		
		debug(0, "[Cutaway::stop] Changing rooms and moving Joe");

		_logic->joeX(joeX);
		_logic->joeY(joeY);
		_logic->currentRoom(joeRoom);
		_logic->oldRoom(_initialRoom);
		_logic->roomDisplay(_logic->roomName(_logic->currentRoom()), RDM_FADE_JOE_XY, 0, _comPanel, true);
	}

	if (_input->cutawayQuit()) {
		// Lines 1927-2032 in cutaway.c
		int i;
		
		// Stop the credits from running
		// XXX CFlag = 0;
		
		_graphics->bobStopAll();

		for (i = 1; i <= _personFaceCount; i++) {
			int index =  _personFace[i].index;
			if (index > 0) {
				_logic->objectData(_personFace[i].index)->image = _personFace[i].image;
				
				_graphics->bob(_logic->findBob(index))->xflip = 
					(_personFace[i].image != -4);
			}
		}

		int quitObjectCount = (int16)READ_BE_UINT16(ptr); ptr += 2;

		for (i = 0; i < quitObjectCount; i++) {
			int16 objectIndex  = (int16)READ_BE_UINT16(ptr); ptr += 2;
			int16 fromIndex    = (int16)READ_BE_UINT16(ptr); ptr += 2;
			int16 x       = (int16)READ_BE_UINT16(ptr); ptr += 2;
			int16 y       = (int16)READ_BE_UINT16(ptr); ptr += 2;
			int16 room    = (int16)READ_BE_UINT16(ptr); ptr += 2;
			int16 frame   = (int16)READ_BE_UINT16(ptr); ptr += 2;
			int16 bank    = (int16)READ_BE_UINT16(ptr); ptr += 2;

			int bobIndex = _logic->findBob(objectIndex);
			ObjectData *object = _logic->objectData(objectIndex);

			if (fromIndex > 0) {
				if (fromIndex == objectIndex) {
					// Enable object
					object->name = abs(object->name);
				}
				else {
					_logic->objectCopy(fromIndex, objectIndex);

					ObjectData *from = _logic->objectData(fromIndex);
					if (object->image && !from->image && bobIndex && _logic->currentRoom() == object->room)
						_graphics->bobClear(bobIndex);
				}

				if (_logic->currentRoom() == room)
					_logic->roomRefreshObject(objectIndex);
			}

			if (_logic->currentRoom() == object->room) {
				BobSlot *pbs = _graphics->bob(bobIndex);

				if (x || y) {
					pbs->x = x;
					pbs->y = y;
					if (InRange(object->image, -4, -3))
						pbs->scale = _logic->findScale(x, y);
				}

				if (frame) {
					if (0 == bank)
						bank = 15;
					else if (bank != 13) {
						// XXX if(bank != oldBank) {
						_graphics->bankLoad(_bankNames[bank-1], 8);
						// XXX	oldBank = bank;
						// XXX }
						bank = 8;
					}

					int objectFrame = _logic->findFrame(objectIndex);

					if (objectFrame == 1000) {
						_graphics->bobClear(bobIndex);
					}
					else if (objectFrame) {
						_graphics->bankUnpack(abs(frame), objectFrame, bank);
						pbs->frameNum = objectFrame;
						if (frame < 0)
							pbs->xflip = true;
							
					}
				}
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
		BobSlot *joeBob = _graphics->bob(0);
		
		debug(0, "[Cutaway::stop] Moving Joe");

		joeBob->x = joeX;
		joeBob->y = joeY;
		joeBob->scale = _logic->findScale(joeX, joeY);
		_logic->joeFace();
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
					_logic->objectCopy(fromObject, objectIndex);
				_logic->roomRefreshObject(objectIndex);
			}
			else if (objectIndex < 0) {               // Hide the object
				objectIndex             = -objectIndex;
				ObjectData *objectData  = _logic->objectData(objectIndex);
				objectData->name        = -abs(objectData->name);
				_logic->roomRefreshObject(objectIndex);
			}

			if (areaIndex > 0) {

				// Turn area on or off

				if (areaSubIndex > 0) {
					Area *area = _logic->area(areaIndex, areaSubIndex);
					area->mapNeighbours = abs(area->mapNeighbours);
				}
				else {
					Area *area = _logic->area(areaIndex, abs(areaSubIndex));
					area->mapNeighbours = -abs(area->mapNeighbours);
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
		nextFilename[0] = '\0';

		int personInRoom;

		if (_talkTo > 0)
			personInRoom = _talkTo - _logic->roomData(_logic->currentRoom());
		else {
			warning("_talkTo is 0!");
			personInRoom = 0; 			// XXX is this correct?
		}

		Talk::talk(_talkFile, personInRoom, nextFilename, _graphics, _input, _logic, _resource, _sound);
	}
}

int Cutaway::makeComplexAnimation(int16 currentImage, Cutaway::CutawayAnim *objAnim, int frameCount) {
	// function MAKE_COMPLEX_ANIM, lines 816-883 in cutaway.c
	int frameIndex[256];
	int i;
	int bobNum = objAnim[0].object;

	memset(frameIndex, 0, sizeof(frameIndex));
	debug(0, "[Cutaway::makeComplexAnimation] currentImage = %i", currentImage);

	BobSlot *bob = _graphics->bob(bobNum);
	bob->xflip = objAnim[0].flip;

	for (i = 0; i < frameCount; i++) {
		_cutAnim[bobNum][i].frame = objAnim[i].unpackFrame;
		_cutAnim[bobNum][i].speed = objAnim[i].speed;
		frameIndex[objAnim[i].unpackFrame] = 1;
	}

	_cutAnim[bobNum][frameCount].frame = 0;	
	_cutAnim[bobNum][frameCount].speed = 0;

	int nextFrameIndex = 1;

	for (i = 1; i < 256; i++)
		if (frameIndex[i])
			frameIndex[i] = nextFrameIndex++;

	for (i = 0; i < frameCount; i++) {
		_cutAnim[bobNum][i].frame = currentImage + frameIndex[objAnim[i].unpackFrame];
		//debug(0, "_cutAnim[%i][%i].frame = %i", bobNum, i, _cutAnim[bobNum][i].frame);
	}

	for (i = 1; i < 256; i++) {
		if (frameIndex[i]) {
			currentImage++;
			//debug(0, "bankUnpack(%i, %i, %i)", i, currentImage, objAnim[0].bank);
			_graphics->bankUnpack(i, currentImage, objAnim[0].bank);
		}
	}

	_graphics->bobAnimString(bobNum, _cutAnim[bobNum]);

	return currentImage;
}

void Cutaway::handleText(
		int index,
		ObjectType type, 
		CutawayObject &object,
		const char *sentence) {
	// lines 1776-1863 in cutaway.c

	//debug(0, "----- Write '%s' ----", sentence);

	int spaces = countSpaces(type, sentence);

	int x;
	int flags;

	if (OBJECT_TYPE_TEXT_DISPLAY == type) {
		x = _graphics->textCenterX(sentence);
		flags = 2;
	}
	else {
		x = object.bobStartX;
		flags = 1;
	}

	BobSlot *bob = 
		_graphics->bob( _logic->findBob(abs(object.objectNumber)) );

	_graphics->bobSetText(bob, sentence, x, object.bobStartY, object.specialMove, flags);

	if (OBJECT_TYPE_TEXT_SPEAK == type || OBJECT_TYPE_TEXT_DISPLAY_AND_SPEAK == type) {
		char voiceFileName[MAX_STRING_SIZE];
		findCdCut(_basename, index, voiceFileName);
		strcat(voiceFileName, "1");
		_sound->sfxPlay(voiceFileName);
	}

	int i;
	for (i = 0; i < spaces; i++) {
		_logic->update();

		if (OBJECT_TYPE_TEXT_SPEAK == type || OBJECT_TYPE_TEXT_DISPLAY_AND_SPEAK == type) {
			// XXX: see if speaking is finished
		}

		if (_input->cutawayQuit())
			return;

		if (_input->keyVerb().isSkipText()) {
			_input->clearKeyVerb();
			break;
		}
	}

	_graphics->textClear(0,198);
	_logic->update();
}
		
int Cutaway::countSpaces(ObjectType type, const char *segment) {
	int tmp = 0;

	while (*segment++)
		tmp++;
	
	if (tmp < 10)
		tmp = 10;

	if (OBJECT_TYPE_TEXT_DISPLAY == type)
		tmp *= 3;

	return (tmp * 2) / _logic->talkSpeed();

}

int Cutaway::scale(CutawayObject &object) {
	int scaling = 100;

	if (object.scale > 0)
		scaling = object.scale;
	else if (!object.objectNumber) {
		// Only scale Joe 
		int x, y;

		if (object.bobStartX > 0 || object.bobStartY > 0) {
			x = object.bobStartX;
			y = object.bobStartY;
		}
		else {
			BobSlot *bob = _graphics->bob(0);
			x = bob->x;
			y = bob->y;
		}

		int zone = _logic->zoneInArea(0, x, y);
		if (zone > 0) {
			Area *area = _logic->area(_logic->currentRoom(), zone);
			scaling = area->calcScale(y);
		}
	}

	return scaling;
}

} // End of namespace Queen
