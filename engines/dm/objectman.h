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
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#ifndef DM_OBJECTMAN_H
#define DM_OBJECTMAN_H

#include "dm/dm.h"
#include "dm/champion.h"

namespace DM {

#define kDMObjectNameMaximumLength 14 // @ C014_OBJECT_NAME_MAXIMUM_LENGTH
#define kDMObjectNameCount 199 // @ C199_OBJECT_NAME_COUNT
#define kDMObjectNamesGraphicIndice 556 // @ C556_GRAPHIC_OBJECT_NAMES

class SlotBox {
public:
	int16 _x;
	int16 _y;
	int16 _iconIndex;
	SlotBox(int16 x, int16 y, int16 iconIndex): _x(x), _y(y), _iconIndex(iconIndex) {}
	SlotBox(): _x(-1), _y(-1), _iconIndex(-1) {}
}; // @ SLOT_BOX

class ObjectMan {
	DMEngine *_vm;

public:
	explicit ObjectMan(DMEngine *vm);
	~ObjectMan();
	void loadObjectNames();	// @ F0031_OBJECT_LoadNames

	SlotBox _slotBoxes[46]; // @ G0030_as_Graphic562_SlotBoxes;
	char *_objectNames[kDMObjectNameCount]; // @ G0352_apc_ObjectNames
	byte *_objectIconForMousePointer; // @ G0412_puc_Bitmap_ObjectIconForMousePointer

	IconIndice getObjectType(Thing thing); // @ F0032_OBJECT_GetType
	IconIndice getIconIndex(Thing thing); // @ F0033_OBJECT_GetIconIndex
	void extractIconFromBitmap(uint16 iconIndex, byte *destBitmap); // @ F0036_OBJECT_ExtractIconFromBitmap
	void drawIconInSlotBox(uint16 slotBoxIndex, int16 iconIndex); // @ F0038_OBJECT_DrawIconInSlotBox
	void drawLeaderObjectName(Thing thing); // @ F0034_OBJECT_DrawLeaderHandObjectName
	IconIndice getIconIndexInSlotBox(uint16 slotBoxIndex); // @ F0039_OBJECT_GetIconIndexInSlotBox
	void clearLeaderObjectName(); // @ F0035_OBJECT_ClearLeaderHandObjectName
	void drawIconToScreen(int16 iconIndex, int16 posX, int16 posY); // @ F0037_OBJECT_DrawIconToScreen

	int16 _iconGraphicHeight[7]; // @ K0077_ai_IconGraphicHeight
	int16 _iconGraphicFirstIndex[7]; // G0026_ai_Graphic562_IconGraphicFirstIconIndex

	void initConstants();
};
}

#endif
