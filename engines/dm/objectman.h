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

#define k8_SlotBoxInventoryFirstSlot 8 // @ C08_SLOT_BOX_INVENTORY_FIRST_SLOT   
#define k9_SlotBoxInventoryActionHand 9 // @ C09_SLOT_BOX_INVENTORY_ACTION_HAND  
#define k38_SlotBoxChestFirstSlot 38 // @ C38_SLOT_BOX_CHEST_FIRST_SLOT      

#define k199_ObjectNameCount 199 // @ C199_OBJECT_NAME_COUNT

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

	SlotBox _g30_slotBoxes[46]; // @ G0030_as_Graphic562_SlotBoxes;
	char *_g352_objectNames[k199_ObjectNameCount]; // @ G0352_apc_ObjectNames
	byte *_g412_objectIconForMousePointer; // @ G0412_puc_Bitmap_ObjectIconForMousePointer

	IconIndice f32_getObjectType(Thing thing); // @ F0032_OBJECT_GetType
	IconIndice f33_getIconIndex(Thing thing); // @ F0033_OBJECT_GetIconIndex
	void f36_extractIconFromBitmap(uint16 iconIndex, byte *destBitmap); // @ F0036_OBJECT_ExtractIconFromBitmap
	void f38_drawIconInSlotBox(uint16 slotBoxIndex, int16 iconIndex); // @ F0038_OBJECT_DrawIconInSlotBox
	void f34_drawLeaderObjectName(Thing thing); // @ F0034_OBJECT_DrawLeaderHandObjectName
	IconIndice f39_getIconIndexInSlotBox(uint16 slotBoxIndex); // @ F0039_OBJECT_GetIconIndexInSlotBox
	void f35_clearLeaderObjectName(); // @ F0035_OBJECT_ClearLeaderHandObjectName


};

}
