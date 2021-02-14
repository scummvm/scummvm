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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/* -----------------08/07/97 21.43-------------------
			Structures definitions
 --------------------------------------------------*/

#ifndef TRECISION_NL_STRUCT_H
#define TRECISION_NL_STRUCT_H

namespace Trecision {

struct SRoom {
	int8  _baseName[4];                     // Room name
	uint8 _flag;							// Room visited or not
	uint16 _bkgAnim;						    // Background animation
	uint16 _object[MAXOBJINROOM];            // Objects in the room
	uint16 _sounds[MAXSOUNDSINROOM];         // Sounds of the room
	uint16 _actions[MAXACTIONINROOM];        // Character actions in the room
};

struct SObject {
	uint16 _dx, _dy;
	uint16 _px, _py;
	uint16 _lim[4];
	int8  _position;						// -1 if no position
	uint16 _name;
	uint16 _examine;
	uint16 _action;
	uint8 _goRoom;                          // If direction room num - if person num dialog
	uint8 _nbox;                            // Which 3d box the object is associated with
	uint8 _ninv;                            // ptr inventory
	uint8 _mode;                              /* 0- full
												1- mask
												2- limits
												3- objStatus
												6- center when click !
												7- inventory referenceable */
	uint8 _flag;							/*  Examine = 0
												Direction = 1
												Person = 2
												Carried = 3
												2- Take
												3- Open
												4- Close
												5- Use
												6- Extra
												7- Operated     */
	uint16 _anim;
};

struct SInvObject {
	uint16 _name;                            // Object name in the inventory
	uint16 _examine;                         // Sentence if examined
	uint16 _action;
	uint8 _flag;                              /* 0-
												1-
												2-
												3- Open
												4- Close
												5- Use      (SET=use OFF & notSET open and close = use with)
												6- Extra
												7- Operated  */
	uint16 _anim;
};

struct SAtFrame {
	uint8 _type;	   //ATFTEXT, ATFSND, ATFEVENT
	uint8 _child;	   // 0 1 2 3 4
	uint16 _numFrame;
	uint16 _index;
};

struct SAnim {
	char _name[14];
	uint16 _flag;		// 1- background 2- icon 3- action 4- qctive	  -   4bits per child
	uint16 _lim[MAXCHILD][4];
	uint8 _nbox;
	struct SAtFrame _atFrame[MAXATFRAME];
};

typedef struct {
	uint16 _sentence;
	uint16 _x, _y, _color;          // you can compact this info using a bit field
	uint16 _startFrame, _length;    // Frame at which the subtitle starts and its length
} DialogSubTitle;

typedef struct {
	uint16	_flag;			// DLGCHOICE_HIDE|DLGCHOICE_ONETIME|DLGCHOICE_FRAUD...if used...
	uint16	_sentenceIndex;	// Index in the sentence array.
	uint16	_firstSubTitle, _subTitleNumb;	 	// starting index and number of sub title sentences
	uint16	_on[MAXDISPSCELTE], _off[MAXDISPSCELTE];
	uint16  _startFrame;    // Starting frame of the choice
	uint16  _nextDialog;
} DialogChoice;

typedef struct {
	uint16	_flag;				// DONT_SKIP .. and more
	uint16 	_interlocutor;		// Person I'm talking to... Maybe it's not needed
	int8	_startAnim[14];		// aANIMATION or text table index by filename.
	uint16	_startLen;
	uint16 	_firstChoice;
	uint16	_choiceNumb;
	uint16	_newPal[MAXNEWSMKPAL];
} Dialog;


// MESSAGES FOR THE SCHEDULER
struct Message {
	uint8 _class;							// message class
	uint8 _event;							// message name
	uint8 _priority;						// message priority

	uint8 _byteParam;
	uint16 _wordParam1;						// byte parameter 1
	uint16 _wordParam2;						// byte parameter 2
	uint32 _longParam;						// int parameter
	uint32 _timestamp;
};

// Screen rectangle (don't remove, please!)
struct ScreenRect {
	uint16 x0, y0, x1, y1;
};

struct MessageQueue {
	uint8   _head, _tail, _len;
	Message *_event[MAXMESSAGE];

	void orderEvents();
	bool testEmptyQueue(uint8 cls);
	bool testEmptyCharacterQueue4Script();
	bool getMessage();
	void initQueue();
};

struct SSortTable {
	uint16 _index;                          // Index in the list
	uint16 _typology;                       // Bitmap type
	bool  _remove;                          // Whether to copy or remove
	uint16 _roomIndex;                       // Room index
	uint16 _curFrame;                        // Current frame (if animation)
};

struct SScriptFrame {
	uint8 _class;
	uint8 _event;

	uint8 _byteParam;

	uint16 _wordParam1;
	uint16 _wordParam2;

	uint16 _longParam;

	bool  _noWait;

	void SendFrame();
};

struct SScript {
	uint16 _firstFrame;
	uint8 _flag;         // 0 - BREAKABLE - If it may be stopped by a mouse click
};                       // 1 - DIALOGEXITNOANIM - If the script is launched from a dialogue at the last choice it exits the dialogue without link anim and by default

struct SSound {
	char  _name[14];
	uint8 _volume;
	uint8 _flag;
	int8  _panning;
};

} // End of namespace Trecision
#endif
