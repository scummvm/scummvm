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

#include "stdafx.h"
#include "sky/sky.h"

#define MICE_FILE	60300
#define NO_MAIN_OBJECTS	24
#define NO_LINC_OBJECTS	21

uint32 _mouseObjectList[] = {
	65,
	9,
	66,
	64,
	8,
	63,
	10,
	11,
	71,
	76,
	37,
	36,
	42,
	75,
	79,
	6,
	74,
	39,
	49,
	43,
	34,
	35,
	77,
	38,

	//Link cursors

	24625,
	24649,
	24827,
	24651,
	24583,
	24581,
	24582,
	24628,
	24650,
	24629,
	24732,
	24631,
	24584,
	24630,
	24626,
	24627,
	24632,
	24643,
	24828,
	24830,
	24829
};

SkyMouse::SkyMouse(OSystem *system, SkyDisk *skyDisk) {

	_skyDisk = skyDisk;
	_system = system;
	_mouseWidth = 6;
	_mouseHeight = 6;
	_maskWidth = 6;
	_maskHeight = 6;

	
	_miceData = _skyDisk->loadFile(MICE_FILE, NULL);
	_mouseData2 = _miceData;

	uint16 width = FROM_LE_16(((struct dataFileHeader *)_miceData)->s_width);
	uint16 height = FROM_LE_16(((struct dataFileHeader *)_miceData)->s_height);

	_savedData = (byte *)malloc((width * height) + sizeof(struct dataFileHeader));

	//load in the object mouse file
	_objectMouseData = _skyDisk->loadFile(MICE_FILE + 1, NULL);
	_mouseWidth = 1;
	_mouseHeight = 1;
	//_systemFlags |= SF_MOUSE;;
}

SkyMouse::~SkyMouse( ){

	free (_miceData);
	free (_savedData);
	free (_objectMouseData);
}

void SkyMouse::replaceMouseCursors(uint16 fileNo) {

	_skyDisk->loadFile(fileNo, _objectMouseData);
}

bool SkyMouse::fnBlankMouse(void) {

	_mouseXOff = 0;	//re-align mouse
	spriteMouse(MOUSE_BLANK, 0, 0);
	return true;
}

bool SkyMouse::fnDiskMouse(void) {

	//turn the mouse into a disk mouse
	spriteMouse(MOUSE_DISK, 11, 11);
	return true;	//don't quit from the interpreter
	
}

void SkyMouse::lockMouse(void) {

	_lockMouseX = _aMouseX;
	_lockMouseY = _aMouseY;
}

void SkyMouse::unlockMouse(void) {

	_aMouseX = _lockMouseX;
	_aMouseY = _lockMouseY;
}

void SkyMouse::restoreMouseData(uint16 frameNum) {

	warning("Stub: SkyMouse::restoreMouseData");
}

void SkyMouse::drawNewMouse() {

	warning("Stub: SkyMouse::drawNewMouse");
	//calculateMouseValues();
	//saveMouseData();
	//drawMouse();
}

void SkyMouse::spriteMouse(uint16 frameNum, uint16 mouseX, uint16 mouseY) {

	//_mouseFlag |= MF_IN_INT;
	_mouseType2 = frameNum;
	_mouseOffsetX = mouseX;
	_mouseOffsetY = mouseY;

	//restoreMouseData(frameNum);
	byte *mouseData = _miceData;
	uint32 pos = ((struct dataFileHeader *)mouseData)->s_sp_size * ((struct dataFileHeader *)mouseData)->s_sp_size;
	pos += sizeof(struct dataFileHeader);
	_mouseData2 = mouseData + pos;	

	_mouseWidth = ((struct dataFileHeader *)mouseData)->s_width;
	_mouseHeight = ((struct dataFileHeader *)mouseData)->s_height;

	_system->set_mouse_cursor(_mouseData2, _mouseWidth, _mouseHeight, mouseX, mouseY);
	if (frameNum == MOUSE_BLANK) 
		_system->show_mouse(false);
	else
		_system->show_mouse(true);
	//drawNewMouse();

	//_mouseFlag ^= (~_mouseFlag | MF_IN_INT);
}
