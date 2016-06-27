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

#include "common/system.h"
#include "graphics/cursorman.h"

#include "eventman.h"
#include "dungeonman.h"
#include "movesens.h"
#include "objectman.h"
#include "inventory.h"
#include "menus.h"




namespace DM {

Box gBoxObjectPiles[4] = { // @ G0462_as_Graphic561_Box_ObjectPiles
	/* { X1, X2, Y1, Y2 } */
	Box(24, 111, 148, 168),   /* Front left */
	Box(112, 199, 148, 168),   /* Front right */
	Box(112, 183, 122, 147),   /* Back right */
	Box(40, 111, 122, 147)}; /* Back left */

MouseInput gPrimaryMouseInput_Entrance[4] = { // @ G0445_as_Graphic561_PrimaryMouseInput_Entrance[4]
												  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandEntranceEnterDungeon, 244, 298,  45,  58, kLeftMouseButton),
	// Strangerke - C201_COMMAND_ENTRANCE_RESUME isn't present in the demo
	MouseInput(kCommandEntranceResume,        244, 298,  76,  93, kLeftMouseButton),
	MouseInput(kCommandEntranceDrawCredits,  248, 293, 187, 199, kLeftMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_RestartGame[2] = { // @ G0446_as_Graphic561_PrimaryMouseInput_RestartGame[2]
													 /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandRestartGame, 103, 217, 145, 159, kLeftMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_Interface[20] = { // @ G0447_as_Graphic561_PrimaryMouseInput_Interface[20]
													/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandClickInChampion_0_StatusBox,       0,  42,   0,  28, kLeftMouseButton),
	MouseInput(kCommandClickInChampion_1_StatusBox,      69, 111,   0,  28, kLeftMouseButton),
	MouseInput(kCommandClickInChampion_2_StatusBox,     138, 180,   0,  28, kLeftMouseButton),
	MouseInput(kCommandClickInChampion_3_StatusBox,     207, 249,   0,  28, kLeftMouseButton),
	MouseInput(kCommandClickOnChamptionIcon_Top_Left,    274, 299,   0,  13, kLeftMouseButton),
	MouseInput(kCommandClickOnChamptionIcon_Top_Right,   301, 319,   0,  13, kLeftMouseButton),
	MouseInput(kCommandClickOnChamptionIcon_Lower_Right, 301, 319,  15,  28, kLeftMouseButton),
	MouseInput(kCommandClickOnChamptionIcon_Lower_Left,  274, 299,  15,  28, kLeftMouseButton),
	MouseInput(kCommandToggleInventoryChampion_0,         43,  66,   0,  28, kLeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 =  44. swapped with 4 next entries */
	MouseInput(kCommandToggleInventoryChampion_1,        112, 135,   0,  28, kLeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 113. swapped with 4 next entries */
	MouseInput(kCommandToggleInventoryChampion_2,        181, 204,   0,  28, kLeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 182. swapped with 4 next entries */
	MouseInput(kCommandToggleInventoryChampion_3,        250, 273,   0,  28, kLeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 251. swapped with 4 next entries */
	MouseInput(kCommandToggleInventoryChampion_0,          0,  66,   0,  28, kRightMouseButton), /* Atari ST: swapped with 4 previous entries */
	MouseInput(kCommandToggleInventoryChampion_1,         69, 135,   0,  28, kRightMouseButton), /* Atari ST: swapped with 4 previous entries */
	MouseInput(kCommandToggleInventoryChampion_2,        138, 204,   0,  28, kRightMouseButton), /* Atari ST: swapped with 4 previous entries */
	MouseInput(kCommandToggleInventoryChampion_3,        207, 273,   0,  28, kRightMouseButton), /* Atari ST: swapped with 4 previous entries */
	MouseInput(kCommandClickInSpellArea,                233, 319,  42,  73, kLeftMouseButton),
	MouseInput(kCommandClickInActionArea,               233, 319,  77, 121, kLeftMouseButton),
	MouseInput(kCommandFreezeGame,                          0,   1, 198, 199, kLeftMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gSecondaryMouseInput_Movement[9] = { // @ G0448_as_Graphic561_SecondaryMouseInput_Movement[9]
													/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandTurnLeft,             234, 261, 125, 145, kLeftMouseButton),
	MouseInput(kCommandMoveForward,          263, 289, 125, 145, kLeftMouseButton),
	MouseInput(kCommandTurnRight,            291, 318, 125, 145, kLeftMouseButton),
	MouseInput(kCommandMoveLeft,             234, 261, 147, 167, kLeftMouseButton),
	MouseInput(kCommandMoveBackward,         263, 289, 147, 167, kLeftMouseButton),
	MouseInput(kCommandMoveRight,            291, 318, 147, 167, kLeftMouseButton),
	MouseInput(kCommandClickInDungeonView,   0, 223,  33, 168, kLeftMouseButton),
	MouseInput(kCommandToggleInventoryLeader, 0, 319,  33, 199, kRightMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gSecondaryMouseInput_ChampionInventory[38] = { // @ G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory[38]
															  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandCloseInventory,                                   0, 319,   0, 199, kRightMouseButton),
	MouseInput(kCommandSaveGame,                                       174, 182,  36,  44, kLeftMouseButton),
	MouseInput(kCommandSleep,                                           188, 204,  36,  44, kLeftMouseButton),
	MouseInput(kCommandCloseInventory,                                 210, 218,  36,  44, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryReadyHand ,        6,  21,  86, 101, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryActionHand,       62,  77,  86, 101, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryHead,              34,  49,  59,  74, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryTorso,             34,  49,  79,  94, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryLegs,              34,  49,  99, 114, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryFeet,              34,  49, 119, 134, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryPouch_2,            6,  21, 123, 138, kLeftMouseButton),
	MouseInput(kCommandClickOnMouth,                                   56,  71,  46,  61, kLeftMouseButton),
	MouseInput(kCommandClickOnEye,                                     12,  27,  46,  61, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryQuiverLine_2_1,    79,  94, 106, 121, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryQuiverLine_1_2,    62,  77, 123, 138, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryQuiverLine_2_2,    79,  94, 123, 138, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryNeck,               6,  21,  66,  81, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryPouch_1,            6,  21, 106, 121, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryQuiverLine_1_1,    62,  77, 106, 121, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_1_1,  66,  81,  66,  81, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_2_2,  83,  98,  49,  64, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_2_3, 100, 115,  49,  64, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_2_4, 117, 132,  49,  64, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_2_5, 134, 149,  49,  64, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_2_6, 151, 166,  49,  64, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_2_7, 168, 183,  49,  64, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_2_8, 185, 200,  49,  64, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_2_9, 202, 217,  49,  64, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_1_2,  83,  98,  66,  81, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_1_3, 100, 115,  66,  81, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_1_4, 117, 132,  66,  81, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_1_5, 134, 149,  66,  81, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_1_6, 151, 166,  66,  81, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_1_7, 168, 183,  66,  81, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_1_8, 185, 200,  66,  81, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxInventoryBackpackLine_1_9, 202, 217,  66,  81, kLeftMouseButton),
	MouseInput(kCommandClickInPanel,                                   96, 223,  83, 167, kLeftMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_PartySleeping[3] = { // @ G0450_as_Graphic561_PrimaryMouseInput_PartySleeping[3]
													   /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandWakeUp, 0, 223, 33, 168, kLeftMouseButton),
	MouseInput(kCommandWakeUp, 0, 223, 33, 168, kRightMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_FrozenGame[3] = { // @ G0451_as_Graphic561_PrimaryMouseInput_FrozenGame[3]
													/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandUnfreezeGame, 0, 319, 0, 199, kLeftMouseButton),
	MouseInput(kCommandUnfreezeGame, 0, 319, 0, 199, kRightMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gMouseInput_ActionAreaNames[5] = { // @ G0452_as_Graphic561_MouseInput_ActionAreaNames[5]
												  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandClickInActionAreaPass,     285, 318,  77,  83, kLeftMouseButton),
	MouseInput(kCommandClickInActionAreaAction_0, 234, 318,  86,  96, kLeftMouseButton),
	MouseInput(kCommandClickInActionAreaAction_1, 234, 318,  98, 108, kLeftMouseButton),
	MouseInput(kCommandClickInActionAreaAction_2, 234, 318, 110, 120, kLeftMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gMouseInput_ActionAreaIcons[5] = { // @ G0453_as_Graphic561_MouseInput_ActionAreaIcons[5]
												  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandClickInActionAreaChampion_0_Action, 233, 252, 86, 120, kLeftMouseButton),
	MouseInput(kCommandClickInActionAreaChampion_1_Action, 255, 274, 86, 120, kLeftMouseButton),
	MouseInput(kCommandClickInActionAreaChampion_2_Action, 277, 296, 86, 120, kLeftMouseButton),
	MouseInput(kCommandClickInActionAreaChampion_3_Action, 299, 318, 86, 120, kLeftMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gMouseInput_SpellArea[9] = { // @ G0454_as_Graphic561_MouseInput_SpellArea[9]
											/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandClickInSpellAreaSymbol_1,      235, 247, 51, 61, kLeftMouseButton),
	MouseInput(kCommandClickInSpellAreaSymbol_2,      249, 261, 51, 61, kLeftMouseButton),
	MouseInput(kCommandClickInSpellAreaSymbol_3,      263, 275, 51, 61, kLeftMouseButton),
	MouseInput(kCommandClickInSpellAreaSymbol_4,      277, 289, 51, 61, kLeftMouseButton),
	MouseInput(kCommandClickInSpellAreaSymbol_5,      291, 303, 51, 61, kLeftMouseButton),
	MouseInput(kCommandClickInSpellAreaSymbol_6,      305, 317, 51, 61, kLeftMouseButton),
	MouseInput(kCommandClickInSpeallAreaCastSpell,    234, 303, 63, 73, kLeftMouseButton),
	MouseInput(kCommandClickInSpellAreaRecantSymbol, 305, 318, 63, 73, kLeftMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gMouseInput_ChampionNamesHands[13] = { // @ G0455_as_Graphic561_MouseInput_ChampionNamesHands[13]
													  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandSetLeaderChampion_0,                                    0,  42,  0,  6, kLeftMouseButton),
	MouseInput(kCommandSetLeaderChampion_1,                                   69, 111,  0,  6, kLeftMouseButton),
	MouseInput(kCommandSetLeaderChampion_2,                                  138, 180,  0,  6, kLeftMouseButton),
	MouseInput(kCommandSetLeaderChampion_3,                                  207, 249,  0,  6, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChampion_0_StatusBoxReadyHand,    4,  19, 10, 25, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChampion_0_StatusBoxActionHand,  24,  39, 10, 25, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChampion_1_StatusBoxReadyHand,   73,  88, 10, 25, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChampion_1_StatusBoxActionHand,  93, 108, 10, 25, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChampion_2_StatusBoxReadyHand,  142, 157, 10, 25, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChampion_2_StatusBoxActionHand, 162, 177, 10, 25, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChampion_3_StatusBoxReadyHand,  211, 226, 10, 25, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChampion_3_StatusBoxActionHand, 231, 246, 10, 25, kLeftMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gMouseInput_PanelChest[9] = { // @ G0456_as_Graphic561_MouseInput_PanelChest[9]
											 /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandClickOnSlotBoxChest_1, 117, 132,  92, 107, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChest_2, 106, 121, 109, 124, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChest_3, 111, 126, 126, 141, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChest_4, 128, 143, 131, 146, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChest_5, 145, 160, 134, 149, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChest_6, 162, 177, 136, 151, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChest_7, 179, 194, 137, 152, kLeftMouseButton),
	MouseInput(kCommandClickOnSlotBoxChest_8, 196, 211, 138, 153, kLeftMouseButton),
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gMouseInput_PanelResurrectReincarnateCancel[4] = { // @ G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel[4]
																  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(kCommandClickInPanelResurrect,   108, 158,  90, 138, kLeftMouseButton), /* Atari ST: Box = 104, 158,  86, 142 */
	MouseInput(kCommandClickInPanelReincarnate, 161, 211,  90, 138, kLeftMouseButton), /* Atari ST: Box = 163, 217,  86, 142 */
	MouseInput(kCommandClickInPanelCancel,      108, 211, 141, 153, kLeftMouseButton), /* Atari ST: Box = 104, 217, 146, 156 */
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};


MouseInput gPrimaryMouseInput_ViewportDialog1Choice[2] = { // @ G0471_as_Graphic561_PrimaryMouseInput_ViewportDialog1Choice[2]
	MouseInput(kCommandClickOnDialogChoice_1, 16, 207, 138, 152, kLeftMouseButton), /* Bottom button */
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_ViewportDialog2Choices[3] = { // @ G0472_as_Graphic561_PrimaryMouseInput_ViewportDialog2Choices[3]
	MouseInput(kCommandClickOnDialogChoice_1, 16, 207, 101, 115, kLeftMouseButton), /* Top button */
	MouseInput(kCommandClickOnDialogChoice_2, 16, 207, 138, 152, kLeftMouseButton), /* Bottom button */
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_ViewportDialog3Choices[4] = { // @ G0473_as_Graphic561_PrimaryMouseInput_ViewportDialog3Choices[4]
	MouseInput(kCommandClickOnDialogChoice_1,  16, 207, 101, 115, kLeftMouseButton), /* Top button */
	MouseInput(kCommandClickOnDialogChoice_2,  16, 101, 138, 152, kLeftMouseButton), /* Lower left button */
	MouseInput(kCommandClickOnDialogChoice_3, 123, 207, 138, 152, kLeftMouseButton), /* Lower right button */
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_ViewportDialog4Choices[5] = { // @ G0474_as_Graphic561_PrimaryMouseInput_ViewportDialog4Choices[5]
	MouseInput(kCommandClickOnDialogChoice_1,  16, 101, 101, 115, kLeftMouseButton), /* Top left button */
	MouseInput(kCommandClickOnDialogChoice_2, 123, 207, 101, 115, kLeftMouseButton), /* Top right button */
	MouseInput(kCommandClickOnDialogChoice_3,  16, 101, 138, 152, kLeftMouseButton), /* Lower left button */
	MouseInput(kCommandClickOnDialogChoice_4, 123, 207, 138, 152, kLeftMouseButton), /* Lower right button */
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_ScreenDialog1Choice[2] = { // @ G0475_as_Graphic561_PrimaryMouseInput_ScreenDialog1Choice[2]
	MouseInput(kCommandClickOnDialogChoice_1, 63, 254, 138, 152, kLeftMouseButton), /* Bottom button */
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_ScreenDialog2Choices[3] = { // @ G0476_as_Graphic561_PrimaryMouseInput_ScreenDialog2Choices[3]
	MouseInput(kCommandClickOnDialogChoice_1, 63, 254, 101, 115, kLeftMouseButton), /* Top button */
	MouseInput(kCommandClickOnDialogChoice_2, 63, 254, 138, 152, kLeftMouseButton), /* Bottom button */
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_ScreenDialog3Choices[4] = { // @ G0477_as_Graphic561_PrimaryMouseInput_ScreenDialog3Choices[4]
	MouseInput(kCommandClickOnDialogChoice_1,  63, 254, 101, 115, kLeftMouseButton), /* Top button */
	MouseInput(kCommandClickOnDialogChoice_2,  63, 148, 138, 152, kLeftMouseButton), /* Lower left button */
	MouseInput(kCommandClickOnDialogChoice_3, 170, 254, 138, 152, kLeftMouseButton), /* Lower right button */
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};
MouseInput gPrimaryMouseInput_ScreenDialog4Choices[5] = { // @ G0478_as_Graphic561_PrimaryMouseInput_ScreenDialog4Choices[5]
	MouseInput(kCommandClickOnDialogChoice_1,  63, 148, 101, 115, kLeftMouseButton), /* Top left button */
	MouseInput(kCommandClickOnDialogChoice_2, 170, 254, 101, 115, kLeftMouseButton), /* Top right button */
	MouseInput(kCommandClickOnDialogChoice_3,  63, 148, 138, 152, kLeftMouseButton), /* Lower left button */
	MouseInput(kCommandClickOnDialogChoice_4, 170, 254, 138, 152, kLeftMouseButton), /* Lower right button */
	MouseInput(kCommandNone, 0, 0, 0, 0, kNoneMouseButton)};

MouseInput* gPrimaryMouseInput_DialogSets[2][4] = { // @ G0480_aaps_PrimaryMouseInput_DialogSets
	{gPrimaryMouseInput_ViewportDialog1Choice,
	gPrimaryMouseInput_ViewportDialog2Choices,
	gPrimaryMouseInput_ViewportDialog3Choices,
	gPrimaryMouseInput_ViewportDialog4Choices},
	{gPrimaryMouseInput_ScreenDialog1Choice,
	gPrimaryMouseInput_ScreenDialog2Choices,
	gPrimaryMouseInput_ScreenDialog3Choices,
	gPrimaryMouseInput_ScreenDialog4Choices},};


EventManager::EventManager(DMEngine *vm) : _vm(vm) {
	_primaryMouseInput = nullptr;
	_secondaryMouseInput = nullptr;

	_pendingClickPresent = false;
	_isCommandQueueLocked = true;
	_mousePointerBitmapUpdated = false;
	_refreshMousePointerInMainLoop = false;
	_highlightBoxEnabled = false;

	_dummyMapIndex = 0;
	_pendingClickButton = kNoneMouseButton;
}


// dummy data
static const byte mouseData[] = {
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 7, 1, 0, 0, 0, 0, 0, 0, 0,
	1, 7, 7, 1, 0, 0, 0, 0, 0, 0,
	1, 7, 7, 7, 1, 0, 0, 0, 0, 0,
	1, 7, 7, 7, 7, 1, 0, 0, 0, 0,
	1, 7, 7, 7, 7, 7, 1, 0, 0, 0,
	1, 7, 7, 7, 7, 7, 7, 1, 0, 0,
	1, 7, 7, 7, 7, 7, 7, 7, 1, 0,
	1, 7, 7, 7, 7, 7, 1, 1, 1, 1,
	1, 7, 7, 1, 7, 7, 1, 0, 0, 0,
	1, 7, 1, 0, 1, 7, 7, 1, 0, 0,
	1, 1, 0, 0, 1, 7, 7, 1, 0, 0,
	0, 0, 0, 0, 0, 1, 7, 7, 1, 0,
	0, 0, 0, 0, 0, 1, 7, 7, 1, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 0, 0
};
#define MOUSE_WIDTH 10
#define MOUSE_HEIGHT 15


void EventManager::initMouse() {
	_mousePos = Common::Point(0, 0);
	CursorMan.pushCursor(mouseData, MOUSE_WIDTH, MOUSE_HEIGHT, 0, 0, 0);
	CursorMan.showMouse(false);

	setMousePos(Common::Point(320 / 2, 200 / 2));
	// TODO: add cursor creatin, set to hidden
}

void EventManager::showMouse(bool visibility) {
	CursorMan.showMouse(visibility);
}

void EventManager::setMousePos(Common::Point pos) {
	_vm->_system->warpMouse(pos.x, pos.y);
}


void EventManager::processInput() {
	DungeonMan &dungeonMan = *_vm->_dungeonMan;
	CurrMapData &currMap = dungeonMan._currMap;

	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		// DUMMY CODE: case EVENT_KEYDOWN, only for testing
		case Common::EVENT_KEYDOWN:
			if (event.synthetic)
				break;

			switch (event.kbd.keycode) {
			case Common::KEYCODE_w:
				dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap._partyDir, 1, 0, currMap._partyPosX, currMap._partyPosY);
				break;
			case Common::KEYCODE_a:
				dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap._partyDir, 0, -1, currMap._partyPosX, currMap._partyPosY);
				break;
			case Common::KEYCODE_s:
				dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap._partyDir, -1, 0, currMap._partyPosX, currMap._partyPosY);
				break;
			case Common::KEYCODE_d:
				dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap._partyDir, 0, 1, currMap._partyPosX, currMap._partyPosY);
				break;
			case Common::KEYCODE_q:
				turnDirLeft(currMap._partyDir);
				break;
			case Common::KEYCODE_e:
				turnDirRight(currMap._partyDir);
				break;
			case Common::KEYCODE_UP:
				if (_dummyMapIndex < 13)
					dungeonMan.setCurrentMapAndPartyMap(++_dummyMapIndex);
				break;
			case Common::KEYCODE_DOWN:
				if (_dummyMapIndex > 0)
					dungeonMan.setCurrentMapAndPartyMap(--_dummyMapIndex);
				break;
			default:
				break;
			}
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			_pendingClickPresent = true;
			_pendingClickPos = _mousePos;
			_pendingClickButton = (event.type == Common::EVENT_LBUTTONDOWN) ? kLeftMouseButton : kRightMouseButton;
			break;
		default:
			break;
		}
	}
}

void EventManager::processPendingClick() {
	if (_pendingClickPresent) {
		_pendingClickPresent = false;
		processClick(_pendingClickPos, _pendingClickButton);
	}
}

void EventManager::processClick(Common::Point mousePos, MouseButton button) {
	CommandType commandType;

	commandType = getCommandTypeFromMouseInput(_primaryMouseInput, mousePos, button);
	if (commandType == kCommandNone)
		commandType = getCommandTypeFromMouseInput(_secondaryMouseInput, mousePos, button);

	if (commandType != kCommandNone)
		_commandQueue.push(Command(mousePos, commandType));

	_isCommandQueueLocked = false;
}

CommandType EventManager::getCommandTypeFromMouseInput(MouseInput *input, Common::Point mousePos, MouseButton button) {
	if (!input)
		return kCommandNone;
	CommandType commandType = kCommandNone;

	while ((commandType = input->_commandTypeToIssue) != kCommandNone) {
		if (input->_hitbox.isPointInside(mousePos) && input->_button == button)
			break;
		input++;
	}
	return commandType;
}


void EventManager::processCommandQueue() {
	_isCommandQueueLocked = true;
	if (_commandQueue.empty()) {
		_isCommandQueueLocked = false;
		processPendingClick();
		return;
	}

	Command cmd = _commandQueue.pop();

	int16 commandX = cmd._pos.x;
	int16 commandY = cmd._pos.y;

	_isCommandQueueLocked = false;
	processPendingClick();

	if ((cmd._type == kCommandTurnRight) || (cmd._type == kCommandTurnLeft)) {
		commandTurnParty(cmd._type);
		return;
	}

	if ((cmd._type >= kCommandMoveForward) && (cmd._type <= kCommandMoveLeft)) {
		commandMoveParty(cmd._type);
		return;
	}

	if (cmd._type == kCommandClickInDungeonView) {
		commandProcessType80ClickInDungeonView(commandX, commandY);
	}
	if (cmd._type == kCommandClickInPanel) {
		commandProcess81ClickInPanel(commandX, commandY);
	}

	// MISSING CODE: the rest of the function
}

void EventManager::commandTurnParty(CommandType cmdType) {
	_vm->_stopWaitingForPlayerInput = true;

	// MISSING CODE: highlight turn left/right buttons

	// MISSING CODE: processing stairs

	// MISSING CODE: process sensors

	// DUMMY CODE: should call F0284_CHAMPION_SetPartyDirection instead
	direction &partyDir = _vm->_dungeonMan->_currMap._partyDir;
	(cmdType == kCommandTurnLeft) ? turnDirLeft(partyDir) : turnDirRight(partyDir);

	// MISSING CODE: process sensors
}

void EventManager::commandMoveParty(CommandType cmdType) {
	_vm->_stopWaitingForPlayerInput = true;

	// MISSING CODE: Lots of code

	// DUMMY CODE:
	DungeonMan &dungeonMan = *_vm->_dungeonMan;
	CurrMapData &currMap = dungeonMan._currMap;

	switch (cmdType) {
	case kCommandMoveForward:
		dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap._partyDir, 1, 0, currMap._partyPosX, currMap._partyPosY);
		break;
	case kCommandMoveLeft:
		dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap._partyDir, 0, -1, currMap._partyPosX, currMap._partyPosY);
		break;
	case kCommandMoveBackward:
		dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap._partyDir, -1, 0, currMap._partyPosX, currMap._partyPosY);
		break;
	case kCommandMoveRight:
		dungeonMan.mapCoordsAfterRelMovement(dungeonMan._currMap._partyDir, 0, 1, currMap._partyPosX, currMap._partyPosY);
		break;
	default:
		break;
	}

	// MISSING CODE: Lots of code
}

void EventManager::commandSetLeader(ChampionIndex champIndex) {
	ChampionMan &cm = *_vm->_championMan;
	ChampionIndex leaderIndex;

	if ((cm._leaderIndex == champIndex) || ((champIndex != kChampionNone) && !cm._champions[champIndex]._currHealth))
		return;

	if (cm._leaderIndex != kChampionNone) {
		leaderIndex = cm._leaderIndex;
		cm._champions[leaderIndex].setAttributeFlag(kChampionAttributeLoad, true);
		cm._champions[leaderIndex].setAttributeFlag(kChampionAttributeNameTitle, true);
		cm._champions[leaderIndex]._load -= _vm->_dungeonMan->getObjectWeight(cm._leaderHand);
		cm._leaderIndex = kChampionNone;
		cm.drawChampionState(leaderIndex);
	}
	if (champIndex == kChampionNone) {
		cm._leaderIndex = kChampionNone;
		return;
	}
	cm._leaderIndex = champIndex;
	Champion *champion = &cm._champions[cm._leaderIndex];
	champion->_dir = _vm->_dungeonMan->_currMap._partyDir;
	cm._champions[champIndex]._load += _vm->_dungeonMan->getObjectWeight(cm._leaderHand);
	if (_vm->indexToOrdinal(champIndex) != cm._candidateChampionOrdinal) {
		champion->setAttributeFlag(kChampionAttributeIcon, true);
		champion->setAttributeFlag(kChampionAttributeNameTitle, true);
		cm.drawChampionState(champIndex);
	}
}

void EventManager::commandProcessType80ClickInDungeonViewTouchFrontWall() {
	DungeonMan &dunMan = *_vm->_dungeonMan;
	CurrMapData &currMap = dunMan._currMap;
	int16 mapX = currMap._partyPosX + gDirIntoStepCountEast[currMap._partyDir];
	int16 mapY = currMap._partyPosY + gDirIntoStepCountNorth[currMap._partyDir];
	if ((mapX >= 0) && (mapX < currMap._width) && (mapY >= 0) && (mapY < currMap._height)) {
		_vm->_stopWaitingForPlayerInput = _vm->_movsens->sensorIsTriggeredByClickOnWall(mapX, mapY, returnOppositeDir(currMap._partyDir));
	}
}

void EventManager::commandProcessType80ClickInDungeonView(int16 posX, int16 posY) {
	DungeonMan &dunMan = *_vm->_dungeonMan;
	ChampionMan &champMan = *_vm->_championMan;
	CurrMapData &currMap = _vm->_dungeonMan->_currMap;

	if (dunMan._squareAheadElement == kElementTypeDoorFront) {
		if (champMan._leaderIndex == kChampionNone)
			return;

		if (champMan._leaderEmptyHanded) {
			int16 mapX = currMap._partyPosX + gDirIntoStepCountEast[currMap._partyDir];
			int16 mapY = currMap._partyPosY + gDirIntoStepCountNorth[currMap._partyDir];

			if (Door(dunMan.getSquareFirstThingData(mapX, mapY)).hasButton() &&
				dunMan._dungeonViewClickableBoxes[kViewCellDoorButtonOrWallOrn].isPointInside(Common::Point(posX, posY - 33))) {
				_vm->_stopWaitingForPlayerInput = true;
				warning("MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
				warning("MISSING CODE: F0268_SENSOR_AddEvent");
				return;
			}

			warning("MISSING CODE: F0375_COMMAND_ProcessType80_ClickInDungeonView_IsLeaderHandObjectThrown in elseif condition");
		}
	}

	if (champMan._leaderEmptyHanded) {
		for (int16 viewCell = kViewCellFronLeft; viewCell <= kViewCellDoorButtonOrWallOrn; viewCell++) {
			if (dunMan._dungeonViewClickableBoxes[viewCell].isPointInside(Common::Point(posX, posY - 33))) {
				if (viewCell == kViewCellDoorButtonOrWallOrn) {
					if (!dunMan._isFacingAlcove) {
						commandProcessType80ClickInDungeonViewTouchFrontWall();
					}
				} else {
					warning("MISSING CODE: F0373_COMMAND_ProcessType80_ClickInDungeonView_GrabLeaderHandObject");
				}
				return;
			}
		}
	} else {
		Thing thing = champMan._leaderHand;
		uint16 *rawThingPointer = dunMan.getThingData(thing);
		if (dunMan._squareAheadElement == kElementTypeWall) {
			for (int16 viewCell = kViewCellFronLeft; viewCell <= kViewCellFrontRight; ++viewCell) {
				if (gBoxObjectPiles[viewCell].isPointInside(Common::Point(posX, posY))) {
					warning("F0374_COMMAND_ProcessType80_ClickInDungeonView_DropLeaderHandObject");
					return;
				}
			}

			if (dunMan._dungeonViewClickableBoxes[kViewCellDoorButtonOrWallOrn].isPointInside(Common::Point(posX, posY - 33))) {
				if (dunMan._isFacingAlcove) {
					warning("MISSING CODE: F0374_COMMAND_ProcessType80_ClickInDungeonView_DropLeaderHandObject");
				} else {
					if (dunMan._isFacingFountain) {
						uint16 iconIndex = _vm->_objectMan->getIconIndex(thing);
						int16 weight = dunMan.getObjectWeight(thing);
						if ((iconIndex >= kIconIndiceJunkWater) && (iconIndex <= kIconIndiceJunkWaterSkin)) {
							((Junk*)rawThingPointer)->setChargeCount(3);
						} else if (iconIndex == kIconIndicePotionEmptyFlask) {
							((Potion*)rawThingPointer)->setType(kPotionTypeWaterFlask);
						} else {
							goto T0377019;
						}
						warning("MISSING CODE: F0296_CHAMPION_DrawChangedObjectIcons");
						champMan._champions[champMan._leaderIndex]._load += dunMan.getObjectWeight(thing) - weight;
					}
T0377019:
					commandProcessType80ClickInDungeonViewTouchFrontWall();
				}
			}
		} else {
			warning("MISSING CODE: F0375_COMMAND_ProcessType80_ClickInDungeonView_IsLeaderHandObjectThrown in if branch");
			for (int16 viewCell = kViewCellFronLeft; viewCell <= kViewCellBackLeft; viewCell++) {
				if (gBoxObjectPiles[viewCell].isPointInside(Common::Point(posX, posY))) {
					warning("MISSING CODE: F0374_COMMAND_ProcessType80_ClickInDungeonView_DropLeaderHandObject");
					return;
				}
			}
		}
	}
}

void EventManager::commandProcessCommands160To162ClickInResurrectReincarnatePanel(CommandType commandType) {
	ChampionMan &champMan = *_vm->_championMan;
	InventoryMan &invMan = *_vm->_inventoryMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	CurrMapData &currMap = _vm->_dungeonMan->_currMap;
	DungeonMan &dunMan = *_vm->_dungeonMan;

	uint16 championIndex = champMan._partyChampionCount - 1;
	Champion *champ = &champMan._champions[championIndex];
	if (commandType == kCommandClickInPanelCancel) {
		invMan.toggleInventory(kChampionCloseInventory);
		champMan._candidateChampionOrdinal = _vm->indexToOrdinal(kChampionNone);
		if (champMan._partyChampionCount == 1) {
			commandSetLeader(kChampionNone);
		}
		champMan._partyChampionCount--;
		Box box;
		box._y1 = 0;
		box._y2 = 28 + 1;
		box._x1 = championIndex * kChampionStatusBoxSpacing;
		box._x2 = box._x1 + 66 + 1;
		dispMan._useByteBoxCoordinates = false;
		dispMan.clearScreenBox(kColorBlack, box);
		dispMan.clearScreenBox(kColorBlack, gBoxChampionIcons[champMan.championIconIndex(champ->_cell, currMap._partyDir) * 2]);
		warning("F0457_START_DrawEnabledMenus_CPSF");
		warning("F0078_MOUSE_ShowPointer");
		return;
	}

	champMan._candidateChampionOrdinal = _vm->indexToOrdinal(kChampionNone);
	int16 mapX = currMap._partyPosX + gDirIntoStepCountEast[currMap._partyDir];
	int16 mapY = currMap._partyPosY + gDirIntoStepCountNorth[currMap._partyDir];

	for (uint16 slotIndex = kChampionSlotReadyHand; slotIndex < kChampionSlotChest_1; slotIndex++) {
		Thing thing = champ->getSlot((ChampionSlot)slotIndex);
		if (thing != Thing::_thingNone) {
			warning("MISSING CODE: F0164_DUNGEON_UnlinkThingFromList");
		}
	}
	Thing thing = dunMan.getSquareFirstThing(mapX, mapY);
	for (;;) { // infinite
		if (thing.getType() == kSensorThingType) {
			((Sensor*)dunMan.getThingData(thing))->setTypeDisabled();
			break;
		}
		thing = dunMan.getNextThing(thing);
	}

	if (commandType == kCommandClickInPanelReincarnate) {
		champMan.renameChampion(champ);
		champ->resetSkillsToZero();

		for (uint16 i = 0; i < 12; i++) {
			uint16 statIndex = _vm->_rnd->getRandomNumber(7);
			champ->getStatistic((ChampionStatisticType)statIndex, kChampionStatCurrent)++; // returns reference
			champ->getStatistic((ChampionStatisticType)statIndex, kChampionStatMaximum)++; // returns reference
		}
	}

	if (champMan._partyChampionCount == 1) {
		warning("MISSING CODE: setting time, G0362_l_LastPartyMovementTime , G0313_ul_GameTime");
		commandSetLeader(kChampionFirst);
		_vm->_menuMan->setMagicCasterAndDrawSpellArea(kChampionFirst);
	} else {
		_vm->_menuMan->drawSpellAreaControls(champMan._magicCasterChampionIndex);
	}

	warning("MISSING CODE: F0051_TEXT_MESSAGEAREA_PrintLineFeed");
	Color champColor = gChampionColor[championIndex]; // unreferenced because of missing code
	warning("MISSING CODE: F0047_TEXT_MESSAGEAREA_PrintMessage");
	warning("MISSING CODE: F0047_TEXT_MESSAGEAREA_PrintMessage");

	invMan.toggleInventory(kChampionCloseInventory);
	warning("MISSING CODE: F0457_START_DrawEnabledMenus_CPSF");
	warning("MISSING CODE: F0067_MOUSE_SetPointerToNormal");
}

void EventManager::commandProcess81ClickInPanel(int16 x, int16 y) {
	ChampionMan &champMan = *_vm->_championMan;
	InventoryMan &invMan = *_vm->_inventoryMan;

	CommandType commandType;
	switch (invMan._panelContent) {
	case kPanelContentChest:
		if (champMan._leaderIndex == kChampionNone) // if no leader
			return;
		commandType = getCommandTypeFromMouseInput(gMouseInput_PanelChest, Common::Point(x, y), kLeftMouseButton);
		if (commandType != kCommandNone)
			warning("MISSING CODE: F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox");
		break;
	case kPanelContentResurrectReincarnate:
		if (!champMan._leaderEmptyHanded)
			break;
		commandType = getCommandTypeFromMouseInput(gMouseInput_PanelResurrectReincarnateCancel, Common::Point(x, y), kLeftMouseButton);
		if (commandType != kCommandNone)
			commandProcessCommands160To162ClickInResurrectReincarnatePanel(commandType);
		break;
	}
}

	bool EventManager::hasPendingClick(Common::Point& point, MouseButton button)
	{
		if (_pendingClickButton && button == _pendingClickButton)
			point = _pendingClickPos;
		return _pendingClickPresent;
	}
}; // end of namespace DM
