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

#ifndef SKYMOUSE_H
#define SKYMOUSE_H

#include "stdafx.h"
#include "sky/disk.h"
#include "sky/logic.h"
#include "sky/struc.h"

class SkyLogic;

class SkyMouse {

public:

	SkyMouse(OSystem *system, SkyDisk *skyDisk);
	~SkyMouse(void);

	void mouseEngine(uint16 mouseX, uint16 mouseY);
	void replaceMouseCursors(uint16 fileNo);
	bool fnAddHuman(void);
	void fnSaveCoods(void);
	void lockMouse(void);
	void unlockMouse(void);
	void restoreMouseData(uint16 frameNum);
	void drawNewMouse(void);
	void spriteMouse(uint16 frameNum, uint8 mouseX, uint8 mouseY);
	void useLogicInstance(SkyLogic *skyLogic) { _skyLogic = skyLogic; };
	void buttonPressed(uint8 button);
	void waitMouseNotPressed(void);
	uint16 giveMouseX(void) { return _aMouseX; };
	uint16 giveMouseY(void) { return _aMouseY; };
	uint16 giveCurrentMouseType(void) { return _mouseType2; };

protected:

	void pointerEngine(void);
	void buttonEngine1(void);
	void fixMouseTransparency(byte *mouseData, uint32 size);
	
	uint16 _mouseB;		//used to check for repeat presses
	uint16 _eMouseB;
	uint16 _bMouseB;
	
	uint16 _aMouseX;	//actual mouse coordinates
	uint16 _aMouseY;	

	uint16	_tMouseX;
	uint16	_tMouseY;

	uint16 _newSafeX;
	uint16 _newSafeY;

	uint16 _lockMouseX;
	uint16 _lockMouseY;

	uint16	_mouseType2;	//number of current mouse
	byte *_mouseData2;	//pointer to mouse data

	uint16 _mouseWidth;	//mouse width and height
	uint16 _mouseHeight;

	byte *_mousePosition;	//current screen address of mouse
	uint16 _maskWidth;	//width on screen
	uint16 _maskHeight;	//height on screen

	uint32 _mouseFlag;	//bit 0 set when in handler
				//bit 1 set when screen data has been saved
				//bit 2 set when we don't want to show mouse

	byte *_miceData;	//address of mouse sprites
	byte *_objectMouseData;	//address of object mouse sprites

	uint16	_mouseXOff;

	static uint32 _mouseObjectList[];

	OSystem *_system;
	SkyDisk *_skyDisk;
	SkyLogic *_skyLogic;
};

#endif //SKYMOUSE_H
