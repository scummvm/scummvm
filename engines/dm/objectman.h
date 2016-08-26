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

#include "dm.h"
#include "champion.h"


namespace DM {

#define kSlotBoxInventoryFirstSlot 8 // @ C08_SLOT_BOX_INVENTORY_FIRST_SLOT   
#define kSlotBoxInventoryActionHand 9 // @ C09_SLOT_BOX_INVENTORY_ACTION_HAND  
#define kSlotBoxChestFirstSlot 38 // @ C38_SLOT_BOX_CHEST_FIRST_SLOT      

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

	SlotBox _slotBoxes[46]; // @ G0030_as_Graphic562_SlotBoxes;

	IconIndice getObjectType(Thing thing); // @ F0032_OBJECT_GetType
	IconIndice getIconIndex(Thing thing); // @ F0033_OBJECT_GetIconIndex
	void extractIconFromBitmap(uint16 iconIndex, byte *srcBitmap); // F0036_OBJECT_ExtractIconFromBitmap
	void drawIconInSlotBox(uint16 slotBoxIndex, int16 iconIndex); // @ F0038_OBJECT_DrawIconInSlotBox
};

}
