/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENCUTAWAY_H
#define QUEENCUTAWAY_H

#include "queen/queen.h"

namespace Queen {

class QueenResource;
class QueenLogic;
class QueenGraphics;

class QueenCutaway {
	public:
		//! Public interface to run a cutaway from a file
		static void run(
				const char *filename,
				char *nextFilename,
				QueenLogic *queenLogic,
				QueenResource *queenResource);
	private:
		//! Collection of constants used by QueenCutaway
		enum {
			PREVIOUS_ROOM = 0,
			CURRENT_ROOM = 0,
			OBJECT_ROOMFADE = -1,
			PERSON_JOE = -1,
			OBJECT_JOE = 0,
			MAX_PERSON_COUNT = 6,
			CUTAWAY_BANK = 8,
			MAX_BANK_NAME_COUNT = 5,
			MAX_FILENAME_LENGTH = 12,
			MAX_FILENAME_SIZE = (MAX_FILENAME_LENGTH + 1),
			MAX_STRING_LENGTH = 255,
			MAX_STRING_SIZE = (MAX_STRING_LENGTH + 1),
			LEFT = 1,
			RIGHT = 2,
			FRONT = 3,
			BACK = 4
		};

		//! Different kinds of cutaway objects
		enum ObjectType {
			OBJECT_TYPE_ANIMATION               = 0,
			OBJECT_TYPE_PERSON                  = 1,
			OBJECT_TYPE_NO_ANIMATION            = 2,
			OBJECT_TYPE_TEXT_SPEAK              = 3,
			OBJECT_TYPE_TEXT_DISPLAY_AND_SPEAK  = 4,
			OBJECT_TYPE_TEXT_DISPLAY            = 5
		};

		//! Data for a cutaway object
		struct CutawayObject {
			int16 objectNumber;    // 0 = JOE, -1 = MESSAGE
			int16 moveToX;
			int16 moveToY;
			int16 bank;             // 0 = PBOB, 13 = Joe Bank, else BANK NAMEstr()
			int16 animList;
			int16 execute;          // 1 Yes, 0 No
			int16 limitBobX1;
			int16 limitBobY1;
			int16 limitBobX2;
			int16 limitBobY2;
			int16 specialMove;
			int16 animType;        // 0 - Packet, 1 - Amal, 2 - Unpack
			int16 fromObject;
			int16 bobStartX;
			int16 bobStartY;
			int16 room;
			int16 scale;

			// Variables derived from the variables above
			int song;

			//! People to turn on
			int person[MAX_PERSON_COUNT];

			//! Number of elements used in _person array
			int personCount;
		};

		struct CutawayAnim {
			int object;
			int unpackFrame;          // Frame to unpack 
			int speed;
			int bank;
			int mx;
			int my;
			int cx;
			int cy;
			int scale;
			int currentFrame;  // Index to Current Frame
			int originalFrame;  // Index to Original Object Frame
			int song;
		};

		struct ObjectDataBackup {
			int index;
			int16 name;
			int16 image;
		};

		QueenLogic 		*_queenLogic;
		QueenGraphics *_queenGraphics;

		//! Raw .cut file data (without 20 byte header)
		byte *_fileData;

		//! Game state data inside of _fileDat
		byte *_gameStatePtr;

		//! Actual cutaway data inside of _fileData
		byte *_objectData;

		//! Pointer to next sentence string in _fileData
		byte *_nextSentence;

		//! Number of cutaway objects at _cutawayData
		int _cutawayObjectCount;

		//! This cutaway is followed by another
		bool _anotherCutaway;

		//! Specify if the player can quit this cutaway or not 
		bool _canQuit;

		//! Set to true to abort the cutaway
		bool _quit;

		//! Room before cutaway
		int _initialRoom; 

		//! Temporary room for cutaway
		int _temporaryRoom; 

		//! Room to stay in 
		int _finalRoom; 

		//! Bank names
		char _bankNames[MAX_BANK_NAME_COUNT][MAX_FILENAME_SIZE];

		//! Filename without ".cut"
		char _basename[MAX_FILENAME_SIZE];

		//! Name of .dog file
		char _talkFile[MAX_FILENAME_SIZE];

		//! Used by changeRooms
		ObjectDataBackup _personData[MAX_PERSON_COUNT];

		//! Number of elements used in _personData array
		int _personDataCount;

		//! Play this song when leaving cutaway
		int16 _lastSong;

		//! Song played before running comic.cut
		int16 _songBeforeComic;


		QueenCutaway(
				const char *filename, 
				QueenLogic *queenLogic,
				QueenResource *queenResource);
		~QueenCutaway();

		//! Run this cutaway object 
		void run(char *nextFilename);

		//! Load cutaway data from file 
		void load(const char *filename, QueenResource *queenResource);

		//! Used by load to read string data
		void loadStrings(byte *ptr);

		//! Do something special
		void actionSpecialMove(int index);

		//! Get persons
		byte *turnOnPeople(byte *ptr, CutawayObject &object);

		//! Limit the BOB
		void limitBob(CutawayObject &object);

		//! This cutaway object occurs in another room
		void changeRooms(CutawayObject &object);

		//! Get the object type for this CutawayObject
		ObjectType getObjectType(CutawayObject &object);

		//! Perform actions for an animation
		byte *handleAnimation(byte *ptr, CutawayObject &object);

		//! Perform actions for a person record
		void handlePersonRecord(
				int index, 
				CutawayObject &object, 
				const char *sentence);

		//! Restore QueenLogic::_objectData from _personData
		void restorePersonData();

		//! Copy data from dummy object to object
		void objectCopy(int dummyObjectIndex, int objectIndex);

		//! Go to the final room
		void goToFinalRoom();

		//! Update game state after cutaway
		void updateGameState();

		//! Prepare for talk after cutaway
		void talk(char *nextFilename);

		//! Get CutawayAnim data from ptr and return new ptr
		byte *getCutawayAnim(byte *ptr, int header, CutawayAnim &anim);

		//! Read a string from ptr and return new ptr
		static byte *getString(byte *ptr, char *str, int maxLength);

		//! Read a CutawayObject from ptr and return new ptr
		static byte *getCutawayObject(byte *ptr, CutawayObject &object);

		//! Dump a CutawayObject with debug()
		static void dumpCutawayObject(int index, CutawayObject &object);

		//! Dump CutawayAnum data with debug()
		static void dumpCutawayAnim(CutawayAnim &anim);


};

} // End of namespace Queen

#endif
