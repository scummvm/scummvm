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
	uint8 type;	   //ATFTEXT, ATFSND, ATFEVENT
	uint8 child;	   // 0 1 2 3 4
	uint16 nframe;
	uint16 index;
};

struct SAnim {
	char name[14];
	uint16 flag;		// 1- se e' background 2- se e' icona
	// 3- se e' actionomino 4- se e' attiva	 4bits per i figli
	uint16 lim[MAXCHILD][4];
	uint8 nbox;
	struct SAtFrame atframe[MAXATFRAME];
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


// MESSAGGI PER LO SCHEDULER
typedef struct tagMessage {
	uint8 cls    ;                       // message class
	uint8 event    ;                       // message name
	uint8 priority ;                       // message priority

	uint8 bparam;
	uint16 wparam1  ;                       // byte parameter 1
	uint16 wparam2  ;                       // byte parameter 2
	uint32 lparam   ;                       // int parameter
	uint32 timestamp;
} Message;

// RETTANGOLO DI SCHERMO (no erase, please!)
struct screenrect {
	uint16 x0, y0, x1, y1;
};

struct MessageQueue {
	uint8   head, tail, len;
	Message *event[MAXMESSAGE];
};

struct SSortTable {
	uint16 index;                           // indice nell' elenco
	uint16 typology;                        // tipo di bitmap
	bool  togli;                           // se copiare o togliere
	uint16 roomindex;                       // indice nella room
	uint16 framecur;                        // frame corrente se animazione
};

struct SScriptFrame {
	uint8 cls;
	uint8 event;

	uint8 bparam;

	uint16 wparam1;
	uint16 wparam2;

	uint16 lparam;

	bool  nowait;
};

struct SScript {
	uint16 firstframe;
	uint8 flag;                            // 0 - BREAKABLE - Se si interrompe con la pressione dei tasti del mouse
};                                      //  1 - DIALOGEXITNOANIM - Se lo script e' lanciato da un dialogo all'ultima battuta esce dal dialogo senza anim di link e di default

struct SSound {
	char  name[14];
	uint8 volume;
	uint8 flag;
	int8  panning;
};

} // End of namespace Trecision
#endif
