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

SkyMouse::SkyMouse(OSystem *system, SkyDisk *skyDisk, SkyLogic *skyLogic) {

	_skyDisk = skyDisk;
	_skyLogic = skyLogic;
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

/*bool SkyMouse::fnBlankMouse(void) {
	_mouseXOff = 0;	//re-align mouse
	spriteMouse(MOUSE_BLANK, 0, 0);
	return true;
}

bool SkyMouse::fnDiskMouse(void) {
	//turn the mouse into a disk mouse
	spriteMouse(MOUSE_DISK, 11, 11);
	return true;	//don't quit from the interpreter
}

bool SkyMouse::fnNormalMouse(void) {
	spriteMouse(MOUSE_NORMAL, 0, 0);
	return true;
}*/

bool SkyMouse::fnAddHuman(void) {
	//reintroduce the mouse so that the human can control the player
	//could still be switched out at high-level

	if (!SkyLogic::_scriptVariables[MOUSE_STOP]) {
		SkyLogic::_scriptVariables[MOUSE_STATUS] |= 6;	//cursor & mouse
		_tMouseX = _newSafeX;
		_tMouseY = _newSafeY;

		if (_aMouseY < 2)	//stop mouse activating top line
			_aMouseY = 2;
	
		//force the pointer engine into running a get-off
		//even if it's over nothing

		//KWIK-FIX
		//get off may contain script to remove mouse pointer text
		//surely this script should be run just in case
		//I am going to try it anyway
		uint32 getOff = SkyLogic::_scriptVariables[GET_OFF];
		if (getOff)
			_skyLogic->script((uint16)(getOff & 0xFFFF), (uint16)(getOff >> 16));
	
		SkyLogic::_scriptVariables[SPECIAL_ITEM] = 0xFFFFFFFF;
		SkyLogic::_scriptVariables[GET_OFF] = RESET_MOUSE;
	}

	return true;
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
	SkyState::_systemVars.mouseFlag |= MF_IN_INT;
	_mouseType2 = frameNum;
	_mouseOffsetX = mouseX;
	_mouseOffsetY = mouseY;

	//restoreMouseData(frameNum);
	printf("drawing mouse %d\n",frameNum);
	byte *mouseData = _miceData;
	uint32 pos = ((struct dataFileHeader *)mouseData)->s_sp_size * frameNum;
	pos += sizeof(struct dataFileHeader);
	_mouseData2 = mouseData + pos;	

	_mouseWidth = ((struct dataFileHeader *)mouseData)->s_width;
	_mouseHeight = ((struct dataFileHeader *)mouseData)->s_height;

	_system->set_mouse_cursor(_mouseData2, _mouseWidth, _mouseHeight, mouseX, mouseY);
	if (frameNum == MOUSE_BLANK) _system->show_mouse(false);
	else _system->show_mouse(true);

	//drawNewMouse();

	SkyState::_systemVars.mouseFlag &= ~MF_IN_INT;
}

void SkyMouse::mouseEngine(void) {
	_tMouseX = _aMouseX + TOP_LEFT_X;
	_tMouseY = _aMouseY + TOP_LEFT_Y;

	_eMouseB = _bMouseB;
	_bMouseB = 0;
	
	if (!SkyLogic::_scriptVariables[MOUSE_STOP]) {
		if (SkyLogic::_scriptVariables[MOUSE_STATUS] & (1 << 1)) {
			pointerEngine();
			if (SkyLogic::_scriptVariables[MOUSE_STATUS] & (1 << 2)) //buttons enabled?
				buttonEngine1();
		}
	}	
	_eMouseB = 0;	//don't save up buttons
}

void SkyMouse::pointerEngine(void) {
	warning("Stub: pointerEngine()");
}

void SkyMouse::buttonEngine1(void) {
	//checks for clicking on special item
	//"compare the size of this routine to S1 mouse_button"

	if (_eMouseB) {	//anything pressed?
		SkyLogic::_scriptVariables[BUTTON] = _eMouseB;
		_eMouseB = 0;
		if (SkyLogic::_scriptVariables[SPECIAL_ITEM]) { //over anything?
			Compact *item = SkyState::fetchCompact(SkyLogic::_scriptVariables[SPECIAL_ITEM]);
			if (item->mouseClick)
				_skyLogic->script(item->mouseClick, 0);
		}
	}
}
