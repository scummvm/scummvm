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
#include "gfx.h"
#include "champion.h"
#include "dungeonman.h"



namespace DM {

#define k69_ChampionStatusBoxSpacing 69 // @ C69_CHAMPION_STATUS_BOX_SPACING
#define k38_SlotBoxChestFirstSlot 38 // @ C38_SLOT_BOX_CHEST_FIRST_SLOT


extern Box g32_BoxPanel; // @ G0032_s_Graphic562_Box_Panel

enum PanelContent {
	k0_PanelContentFoodWaterPoisoned = 0, // @ C00_PANEL_FOOD_WATER_POISONED
	k2_PanelContentScroll = 2, // @ C02_PANEL_SCROLL
	k4_PanelContentChest = 4, // @ C04_PANEL_CHEST
	k5_PanelContentResurrectReincarnate = 5 // @ C05_PANEL_RESURRECT_REINCARNATE
};



class InventoryMan {
	DMEngine *_vm;
public:
	explicit InventoryMan(DMEngine *vm);

	int16 _g432_inventoryChampionOrdinal; // @ G0423_i_InventoryChampionOrdinal
	PanelContent _g424_panelContent; // @ G0424_i_PanelContent 
	Thing _g425_chestSlots[8]; // @ G0425_aT_ChestSlots
	Thing _g426_openChest; // @ G0426_T_OpenChest
	int16 _g421_objDescTextXpos; // @ G0421_i_ObjectDescriptionTextX
	int16 _g422_objDescTextYpos; // @ G0422_i_ObjectDescriptionTextY

	void toggleInventory(ChampionIndex championIndex); // @ F0355_INVENTORY_Toggle_CPSE
	void drawStatusBoxPortrait(ChampionIndex championIndex); // @ F0354_INVENTORY_DrawStatusBoxPortrait
	void drawPanelHorizontalBar(int16 x, int16 y, int16 pixelWidth, Color color); // @ F0343_INVENTORY_DrawPanel_HorizontalBar
	void drawPanelFoodOrWaterBar(int16 amount, int16 y, Color color); // @ F0344_INVENTORY_DrawPanel_FoodOrWaterBar 
	void drawPanelFoodWaterPoisoned(); // @ F0345_INVENTORY_DrawPanel_FoodWaterPoisoned
	void drawPanelResurrectReincarnate(); // @ F0346_INVENTORY_DrawPanel_ResurrectReincarnate
	void drawPanel(); // @ F0347_INVENTORY_DrawPanel
	void closeChest(); // @ F0334_INVENTORY_CloseChest
	void drawPanelScrollTextLine(int16 yPos, char *text); // @ F0340_INVENTORY_DrawPanel_ScrollTextLine
	void drawPanelScroll(Scroll *scoll); // @ F0341_INVENTORY_DrawPanel_Scroll
	void openAndDrawChest(Thing thingToOpen, Container *chest, bool isPressingEye); // @ F0333_INVENTORY_OpenAndDrawChest
	void drawIconToViewport(IconIndice iconIndex, int16 xPos, int16 yPos); // @ F0332_INVENTORY_DrawIconToViewport
	void buildObjectAttributeString(int16 potentialAttribMask, int16 actualAttribMask, char ** attribStrings,
									char *destString, char *prefixString, char *suffixString); // @ F0336_INVENTORY_DrawPanel_BuildObjectAttributesString
	void drawPanelObjectDescriptionString(char *descString); // @ F0335_INVENTORY_DrawPanel_ObjectDescriptionString
	void drawPanelArrowOrEye(bool pressingEye); // @ F0339_INVENTORY_DrawPanel_ArrowOrEye
	void drawPanelObject(Thing thingToDraw, bool pressingEye); // @ F0342_INVENTORY_DrawPanel_Object

};

}
