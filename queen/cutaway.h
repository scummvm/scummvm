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

class QueenResource;
class QueenLogic;

class QueenCutaway {
	public:
		//! Public interface to run a cutaway from a file
		static void run(
				const char *filename, 
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
			int objectNumber;    // 0 = JOE, -1 = MESSAGE
			int moveToX;
			int moveToY;
			int bank;             // 0 = PBOB, 13 = Joe Bank, else BANK NAMEstr()
			int animList;
			int execute;          // 1 Yes, 0 No
			int limitBobX1;
			int limitBobY1;
			int limitBobX2;
			int limitBobY2;
			int specialMove;
			int animType;        // 0 - Packet, 1 - Amal, 2 - Unpack
			int fromObject;
			int bobStartX;
			int bobStartY;
			int room;
			int scale;

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

		QueenLogic *_queenLogic;

		//! Raw .cut file data (without 20 byte header)
		byte *_fileData;

		//! Actual cutaway data inside of _fileData
		byte *_objectData;

		//! Pointer to next sentence string in _fileData
		byte *_nextSentence;

		//! Number of cutaway objects at _cutawayData
		int _cutawayObjectCount;

		//! Specify if the player can quit this cutaway or not 
		bool _canQuit;

		//! Set to true to abort the cutaway
		bool _quit;

		//! Room to stay in 
		int _finalRoom; 

		//! Bank names
		char _bankNames[MAX_BANK_NAME_COUNT][MAX_FILENAME_SIZE];

		//! Filename without ".cut"
		char _basename[MAX_FILENAME_SIZE];

		//! Name of .dog file
		char _talkFile[MAX_FILENAME_SIZE];

		//! Used by changeRooms
		int _savedPersonCount;

		QueenCutaway(
				const char *filename, 
				QueenLogic *queenLogic,
				QueenResource *queenResource);
		~QueenCutaway();

		//! Run this cutaway object 
		void run();

		//! Load cutaway data from file 
		void load(const char *filename, QueenResource *queenResource);

		//! Used by load to read string data
		void loadStrings(byte *ptr);

		//! Do something special
		void specialMove(int index);

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

		//! Read a string from ptr and return new ptr
		static byte *getString(byte *ptr, char *str, int maxLength);

		//! Read a CutawayObject from ptr and return new ptr
		static byte *getCutawayObject(byte *ptr, CutawayObject &object);

		//! Dump a CutawayObject with debug()
		static void dumpCutawayObject(int index, CutawayObject &object);

		static byte *getCutawayAnim(byte *ptr, int header, CutawayAnim &anim);
		static void dumpCutawayAnim(CutawayAnim &anim);


};

#endif

