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
#include "common/scummsys.h"

class OSystem;

namespace Sky {

class SkyDisk;
class SkyLogic;

class SkyMouse {

public:

	SkyMouse(OSystem *system, SkyDisk *skyDisk);
	~SkyMouse(void);

	void mouseEngine(uint16 mouseX, uint16 mouseY);
	void replaceMouseCursors(uint16 fileNo);
	bool fnAddHuman(void);
	void fnSaveCoods(void);
	void fnOpenCloseHand(bool open);
	uint16 findMouseCursor(uint32 itemNum);
	void lockMouse(void);
	void unlockMouse(void);
	void restoreMouseData(uint16 frameNum);
	void drawNewMouse(void);
	void spriteMouse(uint16 frameNum, uint8 mouseX, uint8 mouseY);
	void useLogicInstance(SkyLogic *skyLogic) { _skyLogic = skyLogic; };
	void buttonPressed(uint8 button);
	void waitMouseNotPressed(void);
	uint16 giveMouseX(void) { return _mouseX; };
	uint16 giveMouseY(void) { return _mouseY; };
	uint16 giveCurrentMouseType(void) { return _currentCursor; };
	bool wasClicked(void);
	void logicClick(void) { _logicClick = true; };

protected:

	void pointerEngine(uint16 xPos, uint16 yPos);
	void buttonEngine1(void);
	void fixMouseTransparency(byte *mouseData, uint32 size);

	bool _logicClick;
	
	uint16 _mouseB;	//mouse button
	uint16 _mouseX;	//actual mouse coordinates
	uint16 _mouseY;	

	uint16 _currentCursor;

	byte *_miceData;	//address of mouse sprites
	byte *_objectMouseData;	//address of object mouse sprites

	static uint32 _mouseMainObjects[24];
	static uint32 _mouseLincObjects[21];

	OSystem *_system;
	SkyDisk *_skyDisk;
	SkyLogic *_skyLogic;
};

} // End of namespace Sky

#endif //SKYMOUSE_H
