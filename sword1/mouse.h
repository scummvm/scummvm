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

#ifndef BSMOUSE_H
#define BSMOUSE_H

#include "scummsys.h"
#include "sworddefs.h"
#include "object.h"

#define MAX_MOUSE 30

#define BS1L_BUTTON_DOWN		2
#define BS1L_BUTTON_UP			4
#define BS1R_BUTTON_DOWN		8
#define BS1R_BUTTON_UP			16
#define MOUSE_BOTH_BUTTONS		(BS1L_BUTTON_DOWN | BS1R_BUTTON_DOWN)

struct MouseObj {
	int id;
	BsObject *compact;
};

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct MousePtr {
	uint16 numFrames;
	uint16 sizeX;
	uint16 sizeY;
	uint16 hotSpotX;
	uint16 hotSpotY;
	uint8  data[2]; // arbitrary number.
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

class SwordLogic;
class ResMan;
class ObjectMan;
class OSystem;

class SwordMouse {
public:
	SwordMouse(OSystem *system, ResMan *pResMan, ObjectMan *pObjMan);
    void addToList(int id, BsObject *compact);
    void useLogic(SwordLogic *pLogic);
	void setLuggage(uint32 resID, uint32 rate);
	void setPointer(uint32 resID, uint32 rate);
	void animate(void);
	void engine(uint16 x, uint16 y, uint16 eventFlags);
	uint16 testEvent(void);
	void flushEvents(void);
	void giveCoords(uint16 *x, uint16 *y);
	void fnNoHuman(void);
	void fnAddHuman(void);
	void fnBlankMouse(void);
	void fnNormalMouse(void);
	void fnLockMouse(void);
	void fnUnlockMouse(void);
private:
	void fixTransparency(uint8 *data, uint32 size);
	MousePtr *_pointers[17];
	uint32 _currentPtrId, _rate, _rateCnt, _frame;
	OSystem *_system;
	SwordLogic *_logic;
	MouseObj _objList[MAX_MOUSE];
	ResMan *_resMan;
	ObjectMan *_objMan;
	uint16 _mouseX, _mouseY;

	uint8 _mouseStatus, _mouseCount;
	uint16 _numObjs;
	uint16 _lastState, _state;
	uint32 _getOff;
};

#endif //BSMOUSE_H
