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
#include "graphics/thumbnail.h"

#include "dm/eventman.h"
#include "dm/dungeonman.h"
#include "dm/movesens.h"
#include "dm/objectman.h"
#include "dm/inventory.h"
#include "dm/menus.h"
#include "dm/timeline.h"
#include "dm/projexpl.h"
#include "dm/text.h"
#include "dm/group.h"
#include "dm/dialog.h"
#include "dm/sounds.h"


namespace DM {

void EventManager::initArrays() {
	KeyboardInput primaryKeyboardInputInterface[7] = { // @ G0458_as_Graphic561_PrimaryKeyboardInput_Interface
		/* { Command, Code } */
		KeyboardInput(kDMCommandToggleInventoryChampion0, Common::KEYCODE_F1, 0), /* F1 (<CSI>1~) Atari ST: Code = 0x3B00 */
		KeyboardInput(kDMCommandToggleInventoryChampion1, Common::KEYCODE_F2, 0), /* F2 (<CSI>2~) Atari ST: Code = 0x3C00 */
		KeyboardInput(kDMCommandToggleInventoryChampion2, Common::KEYCODE_F3, 0), /* F3 (<CSI>3~) Atari ST: Code = 0x3D00 */
		KeyboardInput(kDMCommandToggleInventoryChampion3, Common::KEYCODE_F4, 0), /* F4 (<CSI>4~) Atari ST: Code = 0x3E00 */
		KeyboardInput(kDMCommandSaveGame, Common::KEYCODE_s, Common::KBD_CTRL), /* CTRL-S       Atari ST: Code = 0x0013 */
		KeyboardInput(kDMCommandFreezeGame, Common::KEYCODE_ESCAPE, 0), /* Esc (0x1B)   Atari ST: Code = 0x001B */
		KeyboardInput(kDMCommandNone, Common::KEYCODE_INVALID, 0)
	};

	KeyboardInput secondaryKeyboardInputMovement[19] = { // @ G0459_as_Graphic561_SecondaryKeyboardInput_Movement
		/* { Command, Code } */
		KeyboardInput(kDMCommandTurnLeft, Common::KEYCODE_KP4, 0), /* Numeric pad 4 Atari ST: Code = 0x5200 */
		KeyboardInput(kDMCommandMoveForward, Common::KEYCODE_KP5, 0), /* Numeric pad 5 Atari ST: Code = 0x4800 */
		KeyboardInput(kDMCommandTurnRight, Common::KEYCODE_KP6, 0), /* Numeric pad 6 Atari ST: Code = 0x4700 */
		KeyboardInput(kDMCommandMoveLeft, Common::KEYCODE_KP1, 0), /* Numeric pad 1 Atari ST: Code = 0x4B00 */
		KeyboardInput(kDMCommandMoveBackward, Common::KEYCODE_KP2, 0), /* Numeric pad 2 Atari ST: Code = 0x5000 */
		KeyboardInput(kDMCommandMoveRight, Common::KEYCODE_KP3, 0), /* Numeric pad 3 Atari ST: Code = 0x4D00. Remaining entries below not present */
		KeyboardInput(kDMCommandMoveForward, Common::KEYCODE_w, 0), /* Up Arrow (<CSI>A) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandMoveForward, Common::KEYCODE_w, Common::KBD_SHIFT), /* Shift Up Arrow (<CSI>T) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandMoveLeft, Common::KEYCODE_a, 0), /* Backward Arrow (<CSI>D) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandMoveLeft, Common::KEYCODE_a, Common::KBD_SHIFT), /* Shift Forward Arrow (<CSI> A) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandMoveRight, Common::KEYCODE_d, 0), /* Forward Arrow (<CSI>C) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandMoveRight, Common::KEYCODE_d, Common::KBD_SHIFT), /* Shift Backward Arrow (<CSI> @) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandMoveBackward, Common::KEYCODE_s, 0), /* Down arrow (<CSI>B) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandMoveBackward, Common::KEYCODE_s, Common::KBD_SHIFT), /* Shift Down arrow (<CSI>S) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandTurnLeft, Common::KEYCODE_q, 0), /* Del (0x7F) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandTurnLeft, Common::KEYCODE_q, Common::KBD_SHIFT), /* Shift Del (0x7F) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandTurnRight, Common::KEYCODE_e, 0), /* Help (<CSI>?~) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandTurnRight, Common::KEYCODE_e, Common::KBD_SHIFT), /* Shift Help (<CSI>?~) */ /*Differs for testing convenience*/
		KeyboardInput(kDMCommandNone, Common::KEYCODE_INVALID, 0)
	};
	KeyboardInput primaryKeyboardInputPartySleeping[3] = { // @ G0460_as_Graphic561_PrimaryKeyboardInput_PartySleeping
		/* { Command, Code } */
		KeyboardInput(kDMCommandWakeUp, Common::KEYCODE_RETURN, 0), /* Return */
		KeyboardInput(kDMCommandFreezeGame, Common::KEYCODE_ESCAPE, 0), /* Esc */
		KeyboardInput(kDMCommandNone, Common::KEYCODE_INVALID, 0)
	};
	KeyboardInput primaryKeyboardInputFrozenGame[2] = { // @ G0461_as_Graphic561_PrimaryKeyboardInput_FrozenGame
		/* { Command, Code } */
		KeyboardInput(kDMCommandUnfreezeGame, Common::KEYCODE_ESCAPE, 0), /* Esc */
		KeyboardInput(kDMCommandNone, Common::KEYCODE_INVALID, 0)
	};
	MouseInput primaryMouseInputEntrance[4] = { // @ G0445_as_Graphic561_PrimaryMouseInput_Entrance[4]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandEntranceEnterDungeon, 244, 298,  45,  58, kDMMouseButtonLeft),
		// Strangerke - C201_COMMAND_ENTRANCE_RESUME isn't present in the demo
		MouseInput(kDMCommandEntranceResume,        244, 298,  76,  93, kDMMouseButtonLeft),
		MouseInput(kDMCommandEntranceDrawCredits,  248, 293, 187, 199, kDMMouseButtonLeft),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputRestartGame[2] = { // @ G0446_as_Graphic561_PrimaryMouseInput_RestartGame[2]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandRestartGame, 103, 217, 145, 159, kDMMouseButtonLeft),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputInterface[20] = { // @ G0447_as_Graphic561_PrimaryMouseInput_Interface[20]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandClickInChampion0StatusBox,       0,  42,   0,  28, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInChampion1StatusBox,      69, 111,   0,  28, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInChampion2StatusBox,     138, 180,   0,  28, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInChampion3StatusBox,     207, 249,   0,  28, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnChamptionIconTopLeft,    274, 299,   0,  13, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnChamptionIconTopRight,   301, 319,   0,  13, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnChamptionIconLowerRight, 301, 319,  15,  28, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnChamptionIconLowerLeft,  274, 299,  15,  28, kDMMouseButtonLeft),
		MouseInput(kDMCommandToggleInventoryChampion0,         43,  66,   0,  28, kDMMouseButtonLeft), /* Atari ST: Only present in CSB 2.x and with Box.X1 =  44. swapped with 4 next entries */
		MouseInput(kDMCommandToggleInventoryChampion1,        112, 135,   0,  28, kDMMouseButtonLeft), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 113. swapped with 4 next entries */
		MouseInput(kDMCommandToggleInventoryChampion2,        181, 204,   0,  28, kDMMouseButtonLeft), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 182. swapped with 4 next entries */
		MouseInput(kDMCommandToggleInventoryChampion3,        250, 273,   0,  28, kDMMouseButtonLeft), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 251. swapped with 4 next entries */
		MouseInput(kDMCommandToggleInventoryChampion0,          0,  66,   0,  28, kDMMouseButtonRight), /* Atari ST: swapped with 4 previous entries */
		MouseInput(kDMCommandToggleInventoryChampion1,         69, 135,   0,  28, kDMMouseButtonRight), /* Atari ST: swapped with 4 previous entries */
		MouseInput(kDMCommandToggleInventoryChampion2,        138, 204,   0,  28, kDMMouseButtonRight), /* Atari ST: swapped with 4 previous entries */
		MouseInput(kDMCommandToggleInventoryChampion3,        207, 273,   0,  28, kDMMouseButtonRight), /* Atari ST: swapped with 4 previous entries */
		MouseInput(kDMCommandClickInSpellArea,                233, 319,  42,  73, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInActionArea,               233, 319,  77, 121, kDMMouseButtonLeft),
		MouseInput(kDMCommandFreezeGame,                          0,   1, 198, 199, kDMMouseButtonLeft),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput secondaryMouseInputMovement[9] = { // @ G0448_as_Graphic561_SecondaryMouseInput_Movement[9]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandTurnLeft,             234, 261, 125, 145, kDMMouseButtonLeft),
		MouseInput(kDMCommandMoveForward,          263, 289, 125, 145, kDMMouseButtonLeft),
		MouseInput(kDMCommandTurnRight,            291, 318, 125, 145, kDMMouseButtonLeft),
		MouseInput(kDMCommandMoveLeft,             234, 261, 147, 167, kDMMouseButtonLeft),
		MouseInput(kDMCommandMoveBackward,         263, 289, 147, 167, kDMMouseButtonLeft),
		MouseInput(kDMCommandMoveRight,            291, 318, 147, 167, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInDungeonView,   0, 223,  33, 168, kDMMouseButtonLeft),
		MouseInput(kDMCommandToggleInventoryLeader, 0, 319,  33, 199, kDMMouseButtonRight),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput secondaryMouseInputChampionInventory[38] = { // @ G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory[38]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandCloseInventory,                                   0, 319,   0, 199, kDMMouseButtonRight),
		MouseInput(kDMCommandSaveGame,                                       174, 182,  36,  44, kDMMouseButtonLeft),
		MouseInput(kDMCommandSleep,                                           188, 204,  36,  44, kDMMouseButtonLeft),
		MouseInput(kDMCommandCloseInventory,                                 210, 218,  36,  44, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryReadyHand ,        6,  21,  86, 101, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryActionHand,       62,  77,  86, 101, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryHead,              34,  49,  59,  74, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryTorso,             34,  49,  79,  94, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryLegs,              34,  49,  99, 114, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryFeet,              34,  49, 119, 134, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryPouch2,            6,  21, 123, 138, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnMouth,                                   56,  71,  46,  61, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnEye,                                     12,  27,  46,  61, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryQuiverLine2_1,    79,  94, 106, 121, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryQuiverLine1_2,    62,  77, 123, 138, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryQuiverLine2_2,    79,  94, 123, 138, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryNeck,               6,  21,  66,  81, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryPouch1,            6,  21, 106, 121, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryQuiverLine1_1,    62,  77, 106, 121, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine1_1,  66,  81,  66,  81, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine2_2,  83,  98,  49,  64, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine2_3, 100, 115,  49,  64, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine2_4, 117, 132,  49,  64, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine2_5, 134, 149,  49,  64, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine2_6, 151, 166,  49,  64, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine2_7, 168, 183,  49,  64, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine2_8, 185, 200,  49,  64, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine2_9, 202, 217,  49,  64, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine1_2,  83,  98,  66,  81, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine1_3, 100, 115,  66,  81, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine1_4, 117, 132,  66,  81, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine1_5, 134, 149,  66,  81, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine1_6, 151, 166,  66,  81, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine1_7, 168, 183,  66,  81, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine1_8, 185, 200,  66,  81, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxInventoryBackpackLine1_9, 202, 217,  66,  81, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInPanel,                                   96, 223,  83, 167, kDMMouseButtonLeft),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputPartySleeping[3] = { // @ G0450_as_Graphic561_PrimaryMouseInput_PartySleeping[3]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandWakeUp, 0, 223, 33, 168, kDMMouseButtonLeft),
		MouseInput(kDMCommandWakeUp, 0, 223, 33, 168, kDMMouseButtonRight),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputFrozenGame[3] = { // @ G0451_as_Graphic561_PrimaryMouseInput_FrozenGame[3]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandUnfreezeGame, 0, 319, 0, 199, kDMMouseButtonLeft),
		MouseInput(kDMCommandUnfreezeGame, 0, 319, 0, 199, kDMMouseButtonRight),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput mouseInputActionAreaNames[5] = { // @ G0452_as_Graphic561_MouseInput_ActionAreaNames[5]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandClickInActionAreaPass,     285, 318,  77,  83, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInActionAreaAction0, 234, 318,  86,  96, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInActionAreaAction1, 234, 318,  98, 108, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInActionAreaAction2, 234, 318, 110, 120, kDMMouseButtonLeft),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput mouseInputActionAreaIcons[5] = { // @ G0453_as_Graphic561_MouseInput_ActionAreaIcons[5]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandClickInActionAreaChampion0Action, 233, 252, 86, 120, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInActionAreaChampion1Action, 255, 274, 86, 120, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInActionAreaChampion2Action, 277, 296, 86, 120, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInActionAreaChampion3Action, 299, 318, 86, 120, kDMMouseButtonLeft),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput mouseInputSpellArea[9] = { // @ G0454_as_Graphic561_MouseInput_SpellArea[9]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandClickInSpellAreaSymbol1,      235, 247, 51, 61, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInSpellAreaSymbol2,      249, 261, 51, 61, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInSpellAreaSymbol3,      263, 275, 51, 61, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInSpellAreaSymbol4,      277, 289, 51, 61, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInSpellAreaSymbol5,      291, 303, 51, 61, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInSpellAreaSymbol6,      305, 317, 51, 61, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInSpeallAreaCastSpell,    234, 303, 63, 73, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickInSpellAreaRecantSymbol, 305, 318, 63, 73, kDMMouseButtonLeft),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput mouseInputChampionNamesHands[13] = { // @ G0455_as_Graphic561_MouseInput_ChampionNamesHands[13]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandSetLeaderChampion0,                                    0,  42,  0,  6, kDMMouseButtonLeft),
		MouseInput(kDMCommandSetLeaderChampion1,                                   69, 111,  0,  6, kDMMouseButtonLeft),
		MouseInput(kDMCommandSetLeaderChampion2,                                  138, 180,  0,  6, kDMMouseButtonLeft),
		MouseInput(kDMCommandSetLeaderChampion3,                                  207, 249,  0,  6, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChampion0StatusBoxReadyHand,    4,  19, 10, 25, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChampion0StatusBoxActionHand,  24,  39, 10, 25, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChampion1StatusBoxReadyHand,   73,  88, 10, 25, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChampion1StatusBoxActionHand,  93, 108, 10, 25, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChampion2StatusBoxReadyHand,  142, 157, 10, 25, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChampion2StatusBoxActionHand, 162, 177, 10, 25, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChampion3StatusBoxReadyHand,  211, 226, 10, 25, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChampion3StatusBoxActionHand, 231, 246, 10, 25, kDMMouseButtonLeft),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput mouseInputPanelChest[9] = { // @ G0456_as_Graphic561_MouseInput_PanelChest[9]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandClickOnSlotBoxChest1, 117, 132,  92, 107, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChest2, 106, 121, 109, 124, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChest3, 111, 126, 126, 141, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChest4, 128, 143, 131, 146, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChest5, 145, 160, 134, 149, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChest6, 162, 177, 136, 151, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChest7, 179, 194, 137, 152, kDMMouseButtonLeft),
		MouseInput(kDMCommandClickOnSlotBoxChest8, 196, 211, 138, 153, kDMMouseButtonLeft),
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput mouseInputPanelResurrectReincarnateCancel[4] = { // @ G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel[4]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(kDMCommandClickInPanelResurrect,   108, 158,  90, 138, kDMMouseButtonLeft), /* Atari ST: Box = 104, 158,  86, 142 */
		MouseInput(kDMCommandClickInPanelReincarnate, 161, 211,  90, 138, kDMMouseButtonLeft), /* Atari ST: Box = 163, 217,  86, 142 */
		MouseInput(kDMCommandClickInPanelCancel,      108, 211, 141, 153, kDMMouseButtonLeft), /* Atari ST: Box = 104, 217, 146, 156 */
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputViewportDialog1Choice[2] = { // @ G0471_as_Graphic561_PrimaryMouseInput_ViewportDialog1Choice[2]
		MouseInput(kDMCommandClickOnDialogChoice1, 16, 207, 138, 152, kDMMouseButtonLeft), /* Bottom button */
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputViewportDialog2Choices[3] = { // @ G0472_as_Graphic561_PrimaryMouseInput_ViewportDialog2Choices[3]
		MouseInput(kDMCommandClickOnDialogChoice1, 16, 207, 101, 115, kDMMouseButtonLeft), /* Top button */
		MouseInput(kDMCommandClickOnDialogChoice2, 16, 207, 138, 152, kDMMouseButtonLeft), /* Bottom button */
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputViewportDialog3Choices[4] = { // @ G0473_as_Graphic561_PrimaryMouseInput_ViewportDialog3Choices[4]
		MouseInput(kDMCommandClickOnDialogChoice1,  16, 207, 101, 115, kDMMouseButtonLeft), /* Top button */
		MouseInput(kDMCommandClickOnDialogChoice2,  16, 101, 138, 152, kDMMouseButtonLeft), /* Lower left button */
		MouseInput(kDMCommandClickOnDialogChoice3, 123, 207, 138, 152, kDMMouseButtonLeft), /* Lower right button */
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputViewportDialog4Choices[5] = { // @ G0474_as_Graphic561_PrimaryMouseInput_ViewportDialog4Choices[5]
		MouseInput(kDMCommandClickOnDialogChoice1,  16, 101, 101, 115, kDMMouseButtonLeft), /* Top left button */
		MouseInput(kDMCommandClickOnDialogChoice2, 123, 207, 101, 115, kDMMouseButtonLeft), /* Top right button */
		MouseInput(kDMCommandClickOnDialogChoice3,  16, 101, 138, 152, kDMMouseButtonLeft), /* Lower left button */
		MouseInput(kDMCommandClickOnDialogChoice4, 123, 207, 138, 152, kDMMouseButtonLeft), /* Lower right button */
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputScreenDialog1Choice[2] = { // @ G0475_as_Graphic561_PrimaryMouseInput_ScreenDialog1Choice[2]
		MouseInput(kDMCommandClickOnDialogChoice1, 63, 254, 138, 152, kDMMouseButtonLeft), /* Bottom button */
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputScreenDialog2Choices[3] = { // @ G0476_as_Graphic561_PrimaryMouseInput_ScreenDialog2Choices[3]
		MouseInput(kDMCommandClickOnDialogChoice1, 63, 254, 101, 115, kDMMouseButtonLeft), /* Top button */
		MouseInput(kDMCommandClickOnDialogChoice2, 63, 254, 138, 152, kDMMouseButtonLeft), /* Bottom button */
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputScreenDialog3Choices[4] = { // @ G0477_as_Graphic561_PrimaryMouseInput_ScreenDialog3Choices[4]
		MouseInput(kDMCommandClickOnDialogChoice1,  63, 254, 101, 115, kDMMouseButtonLeft), /* Top button */
		MouseInput(kDMCommandClickOnDialogChoice2,  63, 148, 138, 152, kDMMouseButtonLeft), /* Lower left button */
		MouseInput(kDMCommandClickOnDialogChoice3, 170, 254, 138, 152, kDMMouseButtonLeft), /* Lower right button */
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};
	MouseInput primaryMouseInputScreenDialog4Choices[5] = { // @ G0478_as_Graphic561_PrimaryMouseInput_ScreenDialog4Choices[5]
		MouseInput(kDMCommandClickOnDialogChoice1,  63, 148, 101, 115, kDMMouseButtonLeft), /* Top left button */
		MouseInput(kDMCommandClickOnDialogChoice2, 170, 254, 101, 115, kDMMouseButtonLeft), /* Top right button */
		MouseInput(kDMCommandClickOnDialogChoice3,  63, 148, 138, 152, kDMMouseButtonLeft), /* Lower left button */
		MouseInput(kDMCommandClickOnDialogChoice4, 170, 254, 138, 152, kDMMouseButtonLeft), /* Lower right button */
		MouseInput(kDMCommandNone, 0, 0, 0, 0, kDMMouseButtonNone)
	};

	MouseInput *primaryMouseInputDialogSets[2][4] = { // @ G0480_aaps_PrimaryMouseInput_DialogSets
		{
			_primaryMouseInputViewportDialog1Choice,
			_primaryMouseInputViewportDialog2Choices,
			_primaryMouseInputViewportDialog3Choices,
			_primaryMouseInputViewportDialog4Choices
		},
		{
			_primaryMouseInputScreenDialog1Choice,
			_primaryMouseInputScreenDialog2Choices,
			_primaryMouseInputScreenDialog3Choices,
			_primaryMouseInputScreenDialog4Choices
		}
	};

	for (int i = 0; i < 2; i++) {
		_primaryKeyboardInputFrozenGame[i] = primaryKeyboardInputFrozenGame[i];
		_primaryMouseInputRestartGame[i] = primaryMouseInputRestartGame[i];
		_primaryMouseInputViewportDialog1Choice[i] = primaryMouseInputViewportDialog1Choice[i];
		_primaryMouseInputScreenDialog1Choice[i] = primaryMouseInputScreenDialog1Choice[i];
		for (int j = 0; j < 4; j++)
			_primaryMouseInputDialogSets[i][j] = primaryMouseInputDialogSets[i][j];
	}

	for (int i = 0; i < 3 ; i++) {
		_primaryKeyboardInputPartySleeping[i] = primaryKeyboardInputPartySleeping[i];
		_primaryMouseInputPartySleeping[i] = primaryMouseInputPartySleeping[i];
		_primaryMouseInputFrozenGame[i] = primaryMouseInputFrozenGame[i];
		_primaryMouseInputViewportDialog2Choices[i] = primaryMouseInputViewportDialog2Choices[i];
		_primaryMouseInputScreenDialog2Choices[i] = primaryMouseInputScreenDialog2Choices[i];
	}

	for (int i = 0; i < 4; i++) {
		_primaryMouseInputEntrance[i] = primaryMouseInputEntrance[i];
		_mouseInputPanelResurrectReincarnateCancel[i] = mouseInputPanelResurrectReincarnateCancel[i];
		_primaryMouseInputViewportDialog3Choices[i] = primaryMouseInputViewportDialog3Choices[i];
		_primaryMouseInputScreenDialog3Choices[i] = primaryMouseInputScreenDialog3Choices[i];
	}

	for (int i = 0; i < 5; i++) {
		_mouseInputActionAreaNames[i] = mouseInputActionAreaNames[i];
		_mouseInputActionAreaIcons[i] = mouseInputActionAreaIcons[i];
		_primaryMouseInputViewportDialog4Choices[i] = primaryMouseInputViewportDialog4Choices[i];
		_primaryMouseInputScreenDialog4Choices[i] = primaryMouseInputScreenDialog4Choices[i];
	}

	for (int i = 0; i < 7; i++)
		_primaryKeyboardInputInterface[i] = primaryKeyboardInputInterface[i];

	for (int i = 0; i < 9; i++) {
		_secondaryMouseInputMovement[i] = secondaryMouseInputMovement[i];
		_mouseInputSpellArea[i] = mouseInputSpellArea[i];
		_mouseInputPanelChest[i] = mouseInputPanelChest[i];
	}

	for (int i = 0; i < 13; i++)
		_mouseInputChampionNamesHands[i] = mouseInputChampionNamesHands[i];

	for (int i = 0; i < 19; i++)
		_secondaryKeyboardInputMovement[i] = secondaryKeyboardInputMovement[i];

	for (int i = 0; i < 20; i++)
		_primaryMouseInputInterface[i] = primaryMouseInputInterface[i];

	for (int i = 0; i < 38; i++)
		_secondaryMouseInputChampionInventory[i] = secondaryMouseInputChampionInventory[i];
}
EventManager::EventManager(DMEngine *vm) : _vm(vm) {
	_mousePos = Common::Point(0, 0);
	_dummyMapIndex = 0;
	_pendingClickPresent = false;
	_pendingClickPos = Common::Point(0, 0);
	_mousePointerOriginalColorsObject = nullptr;
	_mousePointerOriginalColorsChampionIcon = nullptr;
	_mousePointerTempBuffer = nullptr;
	_isCommandQueueLocked = true;
	_mousePointerType = 0;
	_previousMousePointerType = 0;
	_primaryMouseInput = nullptr;
	_secondaryMouseInput = nullptr;
	_mousePointerBitmapUpdated = true;
	_refreshMousePointerInMainLoop = false;
	_highlightBoxEnabled = false;
	_useChampionIconOrdinalAsMousePointerBitmap = 0;
	_pendingClickButton = kDMMouseButtonNone;
	_useObjectAsMousePointerBitmap = false;
	_useHandAsMousePointerBitmap = false;
	_preventBuildPointerScreenArea = false;
	_primaryKeyboardInput = nullptr;
	_secondaryKeyboardInput = nullptr;
	_ignoreMouseMovements = false;
	warning("_g587_hideMousePointerRequestCount should start with value 1");
	_hideMousePointerRequestCount = 0;
	_mouseButtonStatus = 0;
	_highlightScreenBox.setToZero();

	initArrays();
}

EventManager::~EventManager() {
	delete[] _mousePointerOriginalColorsObject;
	delete[] _mousePointerTempBuffer;
	delete[] _mousePointerOriginalColorsChampionIcon;
}

void EventManager::initMouse() {
	static uint16 gK150_PalMousePointer[16] = {0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0, 0xF00, 0xFA0, 0xC86, 0xFF0, 0x000, 0xAAA, 0x00F, 0xFFF}; // @ K0150_aui_Palette_MousePointer

	if (!_mousePointerOriginalColorsObject)
		_mousePointerOriginalColorsObject = new byte[32 * 18];
	if (!_mousePointerTempBuffer)
		_mousePointerTempBuffer = new byte[32 * 18];
	if (!_mousePointerOriginalColorsChampionIcon)
		_mousePointerOriginalColorsChampionIcon = new byte[32 * 18];

	_mousePointerType = k0_pointerArrow;
	_previousMousePointerType = k1_pointerHand;

	byte mousePalette[16 * 3];
	for (int i = 0; i < 16; ++i) {
		mousePalette[i * 3] = (gK150_PalMousePointer[i] >> 8) * (256 / 16);
		mousePalette[i * 3 + 1] = (gK150_PalMousePointer[i] >> 4) * (256 / 16);
		mousePalette[i * 3 + 2] = gK150_PalMousePointer[i] * (256 / 16);
	}
	CursorMan.pushCursorPalette(mousePalette, 0, 16);

	_mousePos = Common::Point(0, 0);
	buildpointerScreenArea(_mousePos.x, _mousePos.y);
	CursorMan.showMouse(false);

	setMousePos(Common::Point(320 / 2, 200 / 2));
}

void EventManager::setMousePointerToNormal(int16 mousePointer) {
	_preventBuildPointerScreenArea = true;
	_useObjectAsMousePointerBitmap = false;
	_useHandAsMousePointerBitmap = (mousePointer == k1_pointerHand);
	_mousePointerBitmapUpdated = true;
	_preventBuildPointerScreenArea = false;
	buildpointerScreenArea(_mousePos.x, _mousePos.y);
}

void EventManager::setPointerToObject(byte *bitmap) {
	static byte palChangesMousepointerOjbectIconShadow[16] = {120, 120, 120, 120, 120, 120, 120, 120,
		120, 120, 120, 120, 0, 120, 120, 120}; // @ K0027_auc_PaletteChanges_MousePointerObjectIconShadow
	static byte palChangesMousePointerIcon[16] = {120, 10, 20, 30, 40, 50, 60, 70, 80, 90,
		100, 110, 0, 130, 140, 150}; // @ G0044_auc_Graphic562_PaletteChanges_MousePointerIcon
	static Box boxMousePointerObjectShadow(2, 17, 2, 17); // @ G0619_s_Box_MousePointer_ObjectShadow
	static Box boxMousePointerObject(0, 15, 0, 15); // @ G0620_s_Box_MousePointer_Object

	DisplayMan &displMan = *_vm->_displayMan;

	_preventBuildPointerScreenArea = true;
	_useObjectAsMousePointerBitmap = true;
	_useHandAsMousePointerBitmap = false;
	_mousePointerBitmapUpdated = true;
	displMan._useByteBoxCoordinates = true;
	byte *L0051_puc_Bitmap = _mousePointerOriginalColorsObject;
	memset(L0051_puc_Bitmap, 0, 32 * 18);

	displMan.blitToBitmapShrinkWithPalChange(bitmap, _mousePointerTempBuffer, 16, 16, 16, 16, palChangesMousepointerOjbectIconShadow);
	displMan.blitToBitmap(_mousePointerTempBuffer, L0051_puc_Bitmap, boxMousePointerObjectShadow, 0, 0, 8, 16, kDMColorNoTransparency, 16, 18);
	displMan.blitToBitmapShrinkWithPalChange(bitmap, _mousePointerTempBuffer, 16, 16, 16, 16, palChangesMousePointerIcon);
	displMan.blitToBitmap(_mousePointerTempBuffer, L0051_puc_Bitmap, boxMousePointerObject, 0, 0, 8, 16, kDMColorBlack, 16, 18);

	_preventBuildPointerScreenArea = false;
	buildpointerScreenArea(_mousePos.x, _mousePos.y);
}

void EventManager::mouseDropChampionIcon() {
	DisplayMan &displMan = *_vm->_displayMan;

	_preventBuildPointerScreenArea = true;
	uint16 championIconIndex = _vm->ordinalToIndex(_useChampionIconOrdinalAsMousePointerBitmap);
	_useChampionIconOrdinalAsMousePointerBitmap = _vm->indexToOrdinal(kDMChampionNone);
	_mousePointerBitmapUpdated = true;
	bool useByteBoxCoordinatesBackup = displMan._useByteBoxCoordinates;
	displMan.blitToScreen(_mousePointerOriginalColorsChampionIcon, &_vm->_championMan->_boxChampionIcons[championIconIndex << 2], 16, kDMColorDarkestGray, 18);
	displMan._useByteBoxCoordinates = useByteBoxCoordinatesBackup;
	_preventBuildPointerScreenArea = false;
}

void EventManager::buildpointerScreenArea(int16 mousePosX, int16 mousePosY) {
	static unsigned char bitmapArrowPointer[288] = { // @ G0042_auc_Graphic562_Bitmap_ArrowPointer
		0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00,
		0x78, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00,
		0x7F, 0x80, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00,
		0x06, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
		0x60, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00,
		0x7E, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x7F, 0x80, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00,
		0x6C, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
		0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xC0, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
		0x84, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
		0x80, 0x40, 0x00, 0x00, 0x83, 0xC0, 0x00, 0x00, 0x92, 0x00, 0x00, 0x00, 0xA9, 0x00, 0x00, 0x00,
		0xC9, 0x00, 0x00, 0x00, 0x04, 0x80, 0x00, 0x00, 0x04, 0x80, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00,
		0xF0, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00,
		0xFF, 0x00, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x00, 0xFF, 0xC0, 0x00, 0x00, 0xFF, 0xC0, 0x00, 0x00,
		0xFE, 0x00, 0x00, 0x00, 0xEF, 0x00, 0x00, 0x00, 0xCF, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00,
		0x07, 0x80, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	static unsigned char bitmapHanPointer[288] = { // @ G0043_auc_Graphic562_Bitmap_HandPointer
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x6A, 0x00, 0x00, 0x00,
		0x35, 0x40, 0x00, 0x00, 0x1A, 0xA0, 0x00, 0x00, 0x0D, 0x50, 0x00, 0x00, 0x0E, 0xA8, 0x00, 0x00,
		0x07, 0xF8, 0x00, 0x00, 0xC7, 0xFC, 0x00, 0x00, 0x67, 0xFC, 0x00, 0x00, 0x77, 0xFC, 0x00, 0x00,
		0x3F, 0xFC, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x1F, 0xFE, 0x00, 0x00, 0x07, 0xFF, 0x00, 0x00,
		0x01, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x00, 0x00, 0x95, 0x00, 0x00, 0x00, 0x4A, 0xA0, 0x00, 0x00, 0x25, 0x50, 0x00, 0x00,
		0x12, 0xA8, 0x00, 0x00, 0x11, 0x54, 0x00, 0x00, 0xC8, 0x04, 0x00, 0x00, 0x28, 0x02, 0x00, 0x00,
		0x98, 0x02, 0x00, 0x00, 0x88, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00,
		0x20, 0x01, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
		0x7F, 0xE0, 0x00, 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x00,
		0xCF, 0xFC, 0x00, 0x00, 0xEF, 0xFE, 0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00,
		0x7F, 0xFE, 0x00, 0x00, 0x7F, 0xFE, 0x00, 0x00, 0x3F, 0xFF, 0x00, 0x00, 0x1F, 0xFF, 0x00, 0x00,
		0x07, 0xFF, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	_preventBuildPointerScreenArea = true;
	if (_useChampionIconOrdinalAsMousePointerBitmap) {
		if ((mousePosY > 28) || (mousePosX < 274)) {
			_mousePointerType = k4_pointerTypeAutoselect;
			mouseDropChampionIcon();
		} else
			_mousePointerType = k2_pointerTypeChampionIcon;
	} else if (mousePosY >= 169)
		_mousePointerType = k0_pointerTypeArrow;
	else if (mousePosX >= 274)
		_mousePointerType = k0_pointerTypeArrow;
	else if (mousePosY <= 28) {
		uint16 championIdx = mousePosX / 69;
		uint16 xOverChampionStatusBox = mousePosX % 69;
		if (championIdx >= _vm->_championMan->_partyChampionCount)
			_mousePointerType = k4_pointerTypeAutoselect;
		else if (xOverChampionStatusBox > 42)
			_mousePointerType = k4_pointerTypeAutoselect;
		else {
			championIdx++;
			if (championIdx == _vm->_inventoryMan->_inventoryChampionOrdinal)
				_mousePointerType = k0_pointerTypeArrow;
			else if (mousePosY <= 6)
				_mousePointerType = k0_pointerTypeArrow;
			else
				_mousePointerType = k4_pointerTypeAutoselect;
		}
	} else if (mousePosX >= 224)
		_mousePointerType = k0_pointerTypeArrow;
	else
		_mousePointerType = k4_pointerTypeAutoselect;

	if (_mousePointerType == k4_pointerTypeAutoselect)
		_mousePointerType = (_useObjectAsMousePointerBitmap) ? k1_pointerTypeObjectIcon : (_useHandAsMousePointerBitmap) ? k3_pointerTypeHand : k0_pointerTypeArrow;

	if (_mousePointerBitmapUpdated || (_mousePointerType != _previousMousePointerType)) {
		_mousePointerBitmapUpdated = false;
		switch (_mousePointerType) {
		case k0_pointerTypeArrow:
			setMousePointerFromSpriteData(bitmapArrowPointer);
			break;
		case k1_pointerTypeObjectIcon:
			CursorMan.replaceCursor(_mousePointerOriginalColorsObject, 32, 18, 0, 0, 0);
			break;
		case k2_pointerTypeChampionIcon:
			CursorMan.replaceCursor(_mousePointerOriginalColorsChampionIcon, 32, 18, 0, 0, 0);
			break;
		case k3_pointerTypeHand:
			setMousePointerFromSpriteData(bitmapHanPointer);
			break;
		default:
			break;
		}
	}
	_previousMousePointerType = _mousePointerType;
	_preventBuildPointerScreenArea = false;
}

void EventManager::setMousePointer() {
	if (_vm->_championMan->_leaderEmptyHanded)
		setMousePointerToNormal((_vm->_championMan->_leaderIndex == kDMChampionNone) ? k0_pointerArrow : k1_pointerHand);
	else
		setPointerToObject(_vm->_objectMan->_objectIconForMousePointer);
}

void EventManager::showMouse() {
	if (_hideMousePointerRequestCount++ == 0)
		CursorMan.showMouse(true);
}

void EventManager::hideMouse() {
	if (_hideMousePointerRequestCount-- == 1)
		CursorMan.showMouse(false);
}

bool EventManager::isMouseButtonDown(MouseButton button) {
	return (button != kDMMouseButtonNone) ? (_mouseButtonStatus & button) : (_mouseButtonStatus == 0);
}

void EventManager::setMousePos(Common::Point pos) {
	g_system->warpMouse(pos.x, pos.y);
}

Common::EventType EventManager::processInput(Common::Event *grabKey, Common::Event *grabMouseClick) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN: {
			if (event.kbdRepeat)
				break;

			if (grabKey) {
				*grabKey = event;
				return event.type;
			}

			if (_primaryKeyboardInput) {
				KeyboardInput *input = _primaryKeyboardInput;
				while (input->_commandToIssue != kDMCommandNone) {
					if ((input->_key == event.kbd.keycode) && (input->_modifiers == (event.kbd.flags & input->_modifiers))) {
						processPendingClick(); // possible fix to BUG0_73
						_commandQueue.push(Command(Common::Point(-1, -1), input->_commandToIssue));
						break;
					}
					input++;
				}
			}

			if (_secondaryKeyboardInput) {
				KeyboardInput *input = _secondaryKeyboardInput;
				while (input->_commandToIssue != kDMCommandNone) {
					if ((input->_key == event.kbd.keycode) && (input->_modifiers == (event.kbd.flags & input->_modifiers))) {
						processPendingClick(); // possible fix to BUG0_73
						_commandQueue.push(Command(Common::Point(-1, -1), input->_commandToIssue));
						break;
					}
					input++;
				}
			}
			break;
		}
		case Common::EVENT_MOUSEMOVE:
			if (!_ignoreMouseMovements)
				_mousePos = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN: {
			MouseButton button = (event.type == Common::EVENT_LBUTTONDOWN) ? kDMMouseButtonLeft : kDMMouseButtonRight;
			_mouseButtonStatus |= button;
			if (grabMouseClick) {
				*grabMouseClick = event;
				return event.type;
			}
			_pendingClickPresent = true;
			_pendingClickPos = _mousePos;
			_pendingClickButton = button;
			break;
		}
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP: {
			MouseButton button = (event.type == Common::EVENT_LBUTTONDOWN) ? kDMMouseButtonLeft : kDMMouseButtonRight;
			_mouseButtonStatus &= ~button;
			resetPressingEyeOrMouth();
			break;
		}
		case Common::EVENT_QUIT:
			_vm->_engineShouldQuit = true;
			break;
		default:
			break;
		}
	}
	if (_ignoreMouseMovements)
		setMousePos(_mousePos);
	return Common::EVENT_INVALID;
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
	if (commandType == kDMCommandNone)
		commandType = getCommandTypeFromMouseInput(_secondaryMouseInput, mousePos, button);

	if (commandType != kDMCommandNone)
		_commandQueue.push(Command(mousePos, commandType));

	_isCommandQueueLocked = false;
}

CommandType EventManager::getCommandTypeFromMouseInput(MouseInput *input, Common::Point mousePos, MouseButton button) {
	if (!input)
		return kDMCommandNone;

	CommandType commandType = kDMCommandNone;
	while ((commandType = input->_commandTypeToIssue) != kDMCommandNone) {
		if (input->_hitbox.isPointInside(mousePos) && input->_button == button)
			break;
		input++;
	}
	return commandType;
}

void EventManager::processCommandQueue() {
	static KeyboardInput *primaryKeyboardInputBackup;
	static KeyboardInput *secondaryKeyboardInputBackup;
	static MouseInput *primaryMouseInputBackup;
	static MouseInput *secondaryMouseInputBackup;

	DisplayMan &displMan = *_vm->_displayMan;
	TextMan &txtMan = *_vm->_textMan;
	InventoryMan &inventory = *_vm->_inventoryMan;

	_isCommandQueueLocked = true;
	if (_commandQueue.empty()) { /* If the command queue is empty */
		_isCommandQueueLocked = false;
		processPendingClick();
		return;
	}

	Command cmd = _commandQueue.pop();
	CommandType cmdType = cmd._type;
	if ((cmdType >= kDMCommandMoveForward) && (cmdType <= kDMCommandMoveLeft) && (_vm->_disabledMovementTicks || (_vm->_projectileDisableMovementTicks && (_vm->_lastProjectileDisabledMovementDirection == (_vm->normalizeModulo4(_vm->_dungeonMan->_partyDir + cmdType - kDMCommandMoveForward)))))) { /* If movement is disabled */
		_isCommandQueueLocked = false;
		processPendingClick();
		return;
	}

	int16 commandX = cmd._pos.x;
	int16 commandY = cmd._pos.y;
	_isCommandQueueLocked = false;
	processPendingClick();
	if ((cmdType == kDMCommandTurnRight) || (cmdType == kDMCommandTurnLeft)) {
		commandTurnParty(cmdType);
		return;
	}

	if ((cmdType >= kDMCommandMoveForward) && (cmdType <= kDMCommandMoveLeft)) {
		commandMoveParty(cmdType);
		return;
	}

	if ((cmdType >= kDMCommandClickInChampion0StatusBox) && (cmdType <= kDMCommandClickInChampion3StatusBox)) {
		int16 championIdx = cmdType - kDMCommandClickInChampion0StatusBox;
		if ((championIdx < _vm->_championMan->_partyChampionCount) && !_vm->_championMan->_candidateChampionOrdinal)
			commandProcessTypes12to27_clickInChampionStatusBox(championIdx, commandX, commandY);

		return;
	}

	if ((cmdType >= kDMCommandClickOnChamptionIconTopLeft) && (cmdType <= kDMCommandClickOnChamptionIconLowerLeft)) {
		mouseProcessCommands125To128_clickOnChampionIcon(cmdType - kDMCommandClickOnChamptionIconTopLeft);

		return;
	}

	if ((cmdType >= kDMCommandClickOnSlotBoxInventoryReadyHand) && (cmdType < (kDMCommandClickOnSlotBoxChest8 + 1))) {
		if (_vm->_championMan->_leaderIndex != kDMChampionNone)
			_vm->_championMan->clickOnSlotBox(cmdType - kDMCommandClickOnSlotBoxChampion0StatusBoxReadyHand);

		return;
	}

	if ((cmdType >= kDMCommandToggleInventoryChampion0) && (cmdType <= kDMCommandCloseInventory)) {
		if (cmdType == kDMCommandCloseInventory) {
			delete _vm->_saveThumbnail;
			_vm->_saveThumbnail = nullptr;
		} else if (!_vm->_saveThumbnail) {
			_vm->_saveThumbnail = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
			Graphics::saveThumbnail(*_vm->_saveThumbnail);
		}

		int16 championIndex = cmdType - kDMCommandToggleInventoryChampion0;
		if (((championIndex == kDMChampionCloseInventory) || (championIndex < _vm->_championMan->_partyChampionCount)) && !_vm->_championMan->_candidateChampionOrdinal)
			inventory.toggleInventory((ChampionIndex)championIndex);

		return;
	}

	if (cmdType == kDMCommandToggleInventoryLeader) {
		if (_vm->_championMan->_leaderIndex != kDMChampionNone)
			inventory.toggleInventory(_vm->_championMan->_leaderIndex);

		return;
	}

	if (cmdType == kDMCommandClickInSpellArea) {
		if ((!_vm->_championMan->_candidateChampionOrdinal) && (_vm->_championMan->_magicCasterChampionIndex != kDMChampionNone))
			commandProcessType100_clickInSpellArea(commandX, commandY);

		return;
	}

	if (cmdType == kDMCommandClickInActionArea) {
		if (!_vm->_championMan->_candidateChampionOrdinal)
			commandProcessType111To115_ClickInActionArea(commandX, commandY);

		return;
	}

	if (cmdType == kDMCommandClickOnMouth) {
		inventory.clickOnMouth();
		return;
	}

	if (cmdType == kDMCommandClickOnEye) {
		inventory.clickOnEye();
		return;
	}

	if (cmdType == kDMCommandClickInDungeonView) {
		commandProcessType80ClickInDungeonView(commandX, commandY);
		return;
	}
	if (cmdType == kDMCommandClickInPanel) {
		commandProcess81ClickInPanel(commandX, commandY);
		return;
	}

	if (_vm->_pressingEye || _vm->_pressingMouth)
		return;

	if (cmdType == kDMCommandSleep) {
		if (!_vm->_championMan->_candidateChampionOrdinal) {
			if (inventory._inventoryChampionOrdinal)
				inventory.toggleInventory(kDMChampionCloseInventory);

			_vm->_menuMan->drawDisabledMenu();
			_vm->_championMan->_partyIsSleeping = true;
			drawSleepScreen();
			displMan.drawViewport(k2_viewportAsBeforeSleepOrFreezeGame);
			_vm->_waitForInputMaxVerticalBlankCount = 0;
			_primaryMouseInput = _primaryMouseInputPartySleeping;
			_secondaryMouseInput = 0;
			_primaryKeyboardInput = _primaryKeyboardInputPartySleeping;
			_secondaryKeyboardInput = nullptr;
			discardAllInput();
		}
		return;
	}

	if (cmdType == kDMCommandWakeUp) {
		_vm->_championMan->wakeUp();
		return;
	}

	if (cmdType == kDMCommandSaveGame) {
		if ((_vm->_championMan->_partyChampionCount > 0) && !_vm->_championMan->_candidateChampionOrdinal)
			_vm->saveGame();

		return;
	}

	if (cmdType == kDMCommandFreezeGame) {
		_vm->_gameTimeTicking = false;
		_vm->_menuMan->drawDisabledMenu();
		displMan.fillBitmap(displMan._bitmapViewport, kDMColorBlack, 112, 136);

		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY:
			txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, 81, 69, kDMColorCyan, kDMColorBlack,
												 "GAME FROZEN", k136_heightViewport);
			break;
		case Common::DE_DEU:
			txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, 66, 69, kDMColorCyan, kDMColorBlack,
												 "SPIEL ANGEHALTEN", k136_heightViewport);
			break;
		case Common::FR_FRA:
			txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, 84, 69, kDMColorCyan, kDMColorBlack,
												 "JEU BLOQUE", k136_heightViewport);
			break;
		}
		displMan.drawViewport(k2_viewportAsBeforeSleepOrFreezeGame);
		primaryMouseInputBackup = _primaryMouseInput;
		secondaryMouseInputBackup = _secondaryMouseInput;
		primaryKeyboardInputBackup = _primaryKeyboardInput;
		secondaryKeyboardInputBackup = _secondaryKeyboardInput;
		_primaryMouseInput = _primaryMouseInputFrozenGame;
		_secondaryMouseInput = 0;
		_primaryKeyboardInput = _primaryKeyboardInputFrozenGame;
		_secondaryKeyboardInput = nullptr;
		discardAllInput();
		return;
	}

	if (cmdType == kDMCommandUnfreezeGame) {
		_vm->_gameTimeTicking = true;
		_vm->_menuMan->drawEnabledMenus();
		_primaryMouseInput = primaryMouseInputBackup;
		_secondaryMouseInput = secondaryMouseInputBackup;
		_primaryKeyboardInput = primaryKeyboardInputBackup;
		_secondaryKeyboardInput = secondaryKeyboardInputBackup;
		discardAllInput();
		return;
	}

	if (cmdType == kDMCommandEntranceEnterDungeon) {
		_vm->_gameMode = kDMModeLoadDungeon;
		return;
	}

	if (cmdType == kDMCommandEntranceResume) {
		_vm->_gameMode = kDMModeLoadSavedGame;
		return;
	}

	if (cmdType == kDMCommandEntranceDrawCredits) {
		_vm->entranceDrawCredits();
		return;
	}

	if ((cmdType >= kDMCommandClickOnDialogChoice1) && (cmdType <= kDMCommandClickOnDialogChoice4)) {
		_vm->_dialog->_selectedDialogChoice = cmdType - (kDMCommandClickOnDialogChoice1 - 1);
		return;
	}

	if (cmdType == kDMCommandRestartGame)
		_vm->_restartGameRequest = true;
}

void EventManager::commandTurnParty(CommandType cmdType) {
	_vm->_stopWaitingForPlayerInput = true;
	if (cmdType == kDMCommandTurnLeft)
		commandHighlightBoxEnable(234, 261, 125, 145);
	else
		commandHighlightBoxEnable(291, 318, 125, 145);

	uint16 partySquare = _vm->_dungeonMan->getSquare(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY).toByte();
	if (Square(partySquare).getType() == kDMElementTypeStairs) {
		commandTakeStairs(getFlag(partySquare, kDMSquareMaskStairsUp));
		return;
	}

	_vm->_moveSens->processThingAdditionOrRemoval(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, _vm->_thingParty, true, false);
	_vm->_championMan->setPartyDirection(_vm->normalizeModulo4(_vm->_dungeonMan->_partyDir + ((cmdType == kDMCommandTurnRight) ? 1 : 3)));
	_vm->_moveSens->processThingAdditionOrRemoval(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, _vm->_thingParty, true, true);
}

void EventManager::commandMoveParty(CommandType cmdType) {
	static Box boxMovementArrows[4] = { // @ G0463_as_Graphic561_Box_MovementArrows
		/* { X1, X2, Y1, Y2 } */
		Box(263, 289, 125, 145),   /* Forward */
		Box(291, 318, 147, 167),   /* Right */
		Box(263, 289, 147, 167),   /* Backward */
		Box(234, 261, 147, 167)    /* Left */
	};

	static int16 movementArrowToStepForwardCount[4] = { // @ G0465_ai_Graphic561_MovementArrowToStepForwardCount
		1,   /* Forward */
		0,   /* Right */
		-1,  /* Backward */
		0    /* Left */
	};

	static int16 movementArrowToSepRightCount[4] = { // @ G0466_ai_Graphic561_MovementArrowToStepRightCount
		0,    /* Forward */
		1,    /* Right */
		0,    /* Backward */
		-1    /* Left */
	};

	_vm->_stopWaitingForPlayerInput = true;
	Champion *championsPtr = _vm->_championMan->_champions;
	for (uint16 idx = kDMChampionFirst; idx < _vm->_championMan->_partyChampionCount; idx++) {
		_vm->_championMan->decrementStamina(idx, ((championsPtr->_load * 3) / _vm->_championMan->getMaximumLoad(championsPtr)) + 1); /* BUG0_50 When a champion is brought back to life at a Vi Altar, his current stamina is lower than what it was before dying. Each time the party moves the current stamina of all champions is decreased, including for dead champions, by an amount that depends on the current load of the champion. For a dead champion the load before he died is used */
		championsPtr++;
	}
	uint16 movementArrowIdx = cmdType - kDMCommandMoveForward;
	Box *highlightBox = &boxMovementArrows[movementArrowIdx];
	commandHighlightBoxEnable(highlightBox->_rect.left, highlightBox->_rect.right, highlightBox->_rect.top, highlightBox->_rect.bottom);
	int16 partyMapX = _vm->_dungeonMan->_partyMapX;
	int16 partyMapY = _vm->_dungeonMan->_partyMapY;

	Square curSquare = _vm->_dungeonMan->getSquare(partyMapX, partyMapY);
	bool isStairsSquare = (curSquare.getType() == kDMElementTypeStairs);
	if (isStairsSquare && (movementArrowIdx == 2)) { /* If moving backward while in stairs */
		commandTakeStairs(getFlag(curSquare.toByte(), kDMSquareMaskStairsUp));
		return;
	}

	_vm->_dungeonMan->mapCoordsAfterRelMovement(_vm->_dungeonMan->_partyDir, movementArrowToStepForwardCount[movementArrowIdx], movementArrowToSepRightCount[movementArrowIdx], partyMapX, partyMapY);
	curSquare = _vm->_dungeonMan->getSquare(partyMapX, partyMapY);

	bool isMovementBlocked = false;
	ElementType partySquareType = curSquare.getType();
	switch (partySquareType){
	case kDMElementTypeWall:
		isMovementBlocked = true;
		break;
	case kDMElementTypeStairs: {
		_vm->_moveSens->getMoveResult(_vm->_thingParty, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, kDMMapXNotOnASquare, 0);
		_vm->_dungeonMan->_partyMapX = partyMapX;
		_vm->_dungeonMan->_partyMapY = partyMapY;
		byte stairState = curSquare.toByte();
		commandTakeStairs(getFlag(stairState, kDMSquareMaskStairsUp));
		return;
		}
	case kDMElementTypeDoor: {
		byte doorState = curSquare.getDoorState();
		isMovementBlocked = (doorState != kDMDoorStateOpen) && (doorState != kDMDoorStateOneFourth) && (doorState != kDMDoorStateDestroyed);
		}
		break;
	case kDMElementTypeFakeWall: {
		byte wallState = curSquare.toByte();
		isMovementBlocked = (!getFlag(wallState, kDMSquareMaskFakeWallOpen) && !getFlag(wallState, kDMSquareMaskFakeWallImaginary));
		}
		break;
	default:
		break;
	}

	if (_vm->_championMan->_partyChampionCount) {
		if (isMovementBlocked) {
			movementArrowIdx += (_vm->_dungeonMan->_partyDir + 2);
			int16 firstDamagedChampionIndex = _vm->_championMan->getTargetChampionIndex(partyMapX, partyMapY, _vm->normalizeModulo4(movementArrowIdx));
			int16 secondDamagedChampionIndex = _vm->_championMan->getTargetChampionIndex(partyMapX, partyMapY, _vm->turnDirRight(movementArrowIdx));
			int16 damage = _vm->_championMan->addPendingDamageAndWounds_getDamage(firstDamagedChampionIndex, 1, kDMWoundTorso | kDMWoundLegs, kDMAttackTypeSelf);
			if (firstDamagedChampionIndex != secondDamagedChampionIndex)
				damage |= _vm->_championMan->addPendingDamageAndWounds_getDamage(secondDamagedChampionIndex, 1, kDMWoundTorso | kDMWoundLegs, kDMAttackTypeSelf);

			if (damage)
				_vm->_sound->requestPlay(kDMSoundIndexPartyDamaged, partyMapX, partyMapY, kDMSoundModePlayImmediately);
		} else {
			isMovementBlocked = (_vm->_groupMan->groupGetThing(partyMapX, partyMapY) != _vm->_thingEndOfList);
			if (isMovementBlocked)
				_vm->_groupMan->processEvents29to41(partyMapX, partyMapY, kDMEventTypeCreateReactionPartyIsAdjacent, 0);
		}
	}

	// DEBUG CODE: check for Console flag
	if (isMovementBlocked && !_vm->_console->_debugNoclip) {
		discardAllInput();
		_vm->_stopWaitingForPlayerInput = false;
		return;
	}

	if (isStairsSquare)
		_vm->_moveSens->getMoveResult(_vm->_thingParty, kDMMapXNotOnASquare, 0, partyMapX, partyMapY);
	else
		_vm->_moveSens->getMoveResult(_vm->_thingParty, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, partyMapX, partyMapY);

	uint16 disabledMovtTicks = 1;
	championsPtr = _vm->_championMan->_champions;
	for (uint16 idx = kDMChampionFirst; idx < _vm->_championMan->_partyChampionCount; idx++) {
		if (championsPtr->_currHealth)
			disabledMovtTicks = MAX((int32)disabledMovtTicks, (int32)_vm->_championMan->getMovementTicks(championsPtr));

		championsPtr++;
	}
	_vm->_disabledMovementTicks = disabledMovtTicks;
	_vm->_projectileDisableMovementTicks = 0;
}

bool EventManager::isLeaderHandObjThrown(int16 posX, int16 posY) {
#define k0_sideLeft 0 // @ C0_SIDE_LEFT
#define k1_sideRight 1 // @ C0_SIDE_LEFT

	if ((posY < 47) || (posY > 102))
		return false;

	bool objectThrownFl;
	if (posX <= 111) {
		if (_vm->_dungeonMan->_squareAheadElement == kDMElementTypeDoorFront) {
			if (posX < 64)
				return false;
		} else if (posX < 32)
			return false;

		// Strangerke: Only present in CSB2.1... But it fixes a bug so we keep it
		objectThrownFl = _vm->_championMan->isLeaderHandObjectThrown(k0_sideLeft);
	} else {
		if (_vm->_dungeonMan->_squareAheadElement == kDMElementTypeDoorFront) {
			if (posX > 163)
				return false;
		} else if (posX > 191)
			return false;

		objectThrownFl = _vm->_championMan->isLeaderHandObjectThrown(k1_sideRight);
	}

	if (objectThrownFl)
		_vm->_stopWaitingForPlayerInput = true;

	return objectThrownFl;
}

void EventManager::setMousePointerFromSpriteData(byte *mouseSprite) {
	byte bitmap[16 * 18];
	memset(bitmap, 0, sizeof(bitmap));
	for (int16 imgPart = 1; imgPart < 3; ++imgPart) {
		for (byte *line = mouseSprite + 72 * imgPart, *pixel = bitmap;
			 line < mouseSprite + 72 * (imgPart + 1);
			 line += 4) {

			uint16 words[2];
			words[0] = READ_BE_UINT16(line);
			words[1] = READ_BE_UINT16(line + 2);
			for (int16 i = 15; i >= 0; --i, ++pixel) {
				uint16 val = (((words[0] >> i) & 1) | (((words[1] >> i) & 1) << 1)) << (imgPart & 0x2);
				if (val)
					*pixel = val + 8;
			}
		}
	}

	CursorMan.replaceCursor(bitmap, 16, 18, 0, 0, 0);
}

void EventManager::commandSetLeader(ChampionIndex champIndex) {
	ChampionMan &cm = *_vm->_championMan;
	ChampionIndex leaderIndex;

	if ((cm._leaderIndex == champIndex) || ((champIndex != kDMChampionNone) && !cm._champions[champIndex]._currHealth))
		return;

	if (cm._leaderIndex != kDMChampionNone) {
		leaderIndex = cm._leaderIndex;
		cm._champions[leaderIndex].setAttributeFlag(kDMAttributeLoad, true);
		cm._champions[leaderIndex].setAttributeFlag(kDMAttributeNameTitle, true);
		cm._champions[leaderIndex]._load -= _vm->_dungeonMan->getObjectWeight(cm._leaderHandObject);
		cm._leaderIndex = kDMChampionNone;
		cm.drawChampionState(leaderIndex);
	}
	if (champIndex == kDMChampionNone) {
		cm._leaderIndex = kDMChampionNone;
		return;
	}
	cm._leaderIndex = champIndex;
	Champion *champion = &cm._champions[cm._leaderIndex];
	champion->_dir = _vm->_dungeonMan->_partyDir;
	cm._champions[champIndex]._load += _vm->_dungeonMan->getObjectWeight(cm._leaderHandObject);
	if (_vm->indexToOrdinal(champIndex) != cm._candidateChampionOrdinal) {
		champion->setAttributeFlag(kDMAttributeIcon, true);
		champion->setAttributeFlag(kDMAttributeNameTitle, true);
		cm.drawChampionState(champIndex);
	}
}

void EventManager::commandProcessType80ClickInDungeonViewTouchFrontWall() {
	int16 mapX = _vm->_dungeonMan->_partyMapX + _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir];
	int16 mapY = _vm->_dungeonMan->_partyMapY + _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];

	if ((mapX >= 0) && (mapX < _vm->_dungeonMan->_currMapWidth)
		&& (mapY >= 0) && (mapY < _vm->_dungeonMan->_currMapHeight))
		_vm->_stopWaitingForPlayerInput = _vm->_moveSens->sensorIsTriggeredByClickOnWall(mapX, mapY, _vm->returnOppositeDir(_vm->_dungeonMan->_partyDir));
}

void EventManager::commandProcessType80ClickInDungeonView(int16 posX, int16 posY) {
	Box boxObjectPiles[4] = { // @ G0462_as_Graphic561_Box_ObjectPiles
		/* { X1, X2, Y1, Y2 } */
		Box(24, 111, 148, 168),   /* Front left */
		Box(112, 199, 148, 168),  /* Front right */
		Box(112, 183, 122, 147),  /* Back right */
		Box(40, 111, 122, 147)    /* Back left */
	};

	if (_vm->_dungeonMan->_squareAheadElement == kDMElementTypeDoorFront) {
		if (_vm->_championMan->_leaderIndex == kDMChampionNone)
			return;

		int16 mapX = _vm->_dungeonMan->_partyMapX + _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir];
		int16 mapY = _vm->_dungeonMan->_partyMapY + _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];

		if (_vm->_championMan->_leaderEmptyHanded) {
			Junk *junkPtr = (Junk*)_vm->_dungeonMan->getSquareFirstThingData(mapX, mapY);
			if ((((Door*)junkPtr)->hasButton()) && _vm->_dungeonMan->_dungeonViewClickableBoxes[kDMViewCellDoorButtonOrWallOrn].isPointInside(posX, posY - 33)) {
				_vm->_stopWaitingForPlayerInput = true;
				_vm->_sound->requestPlay(kDMSoundIndexSwitch, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, kDMSoundModePlayIfPrioritized);
				_vm->_moveSens->addEvent(kDMEventTypeDoor, mapX, mapY, kDMCellNorthWest, kDMSensorEffectToggle, _vm->_gameTime + 1);
				return;
			}
		} else if (isLeaderHandObjThrown(posX, posY))
			return;
	}

	if (_vm->_championMan->_leaderEmptyHanded) {
		for (uint16 currViewCell = kDMViewCellFronLeft; currViewCell < kDMViewCellDoorButtonOrWallOrn + 1; currViewCell++) {
			if (_vm->_dungeonMan->_dungeonViewClickableBoxes[currViewCell].isPointInside(posX, posY - 33)) {
				if (currViewCell == kDMViewCellDoorButtonOrWallOrn) {
					if (!_vm->_dungeonMan->_isFacingAlcove)
						commandProcessType80ClickInDungeonViewTouchFrontWall();
				} else
					processType80_clickInDungeonView_grabLeaderHandObject(currViewCell);

				return;
			}
		}
	} else {
		Thing thingHandObject = _vm->_championMan->_leaderHandObject;
		Junk *junkPtr = (Junk*)_vm->_dungeonMan->getThingData(thingHandObject);
		if (_vm->_dungeonMan->_squareAheadElement == kDMElementTypeWall) {
			for (uint16 currViewCell = kDMViewCellFronLeft; currViewCell < kDMViewCellFrontRight + 1; currViewCell++) {
				if (boxObjectPiles[currViewCell].isPointInside(posX, posY)) {
					clickInDungeonViewDropLeaderHandObject(currViewCell);
					return;
				}
			}
			if (_vm->_dungeonMan->_dungeonViewClickableBoxes[kDMViewCellDoorButtonOrWallOrn].isPointInside(posX, posY - 33)) {
				if (_vm->_dungeonMan->_isFacingAlcove)
					clickInDungeonViewDropLeaderHandObject(kDMViewCellAlcove);
				else {
					if (_vm->_dungeonMan->_isFacingFountain) {
						uint16 iconIdx = _vm->_objectMan->getIconIndex(thingHandObject);
						uint16 weight = _vm->_dungeonMan->getObjectWeight(thingHandObject);
						if ((iconIdx >= kDMIconIndiceJunkWater) && (iconIdx <= kDMIconIndiceJunkWaterSkin))
							junkPtr->setChargeCount(3); /* Full */
						else if (iconIdx == kDMIconIndicePotionEmptyFlask)
							((Potion*)junkPtr)->setType(kDMPotionTypeWaterFlask);
						else {
							commandProcessType80ClickInDungeonViewTouchFrontWall();
							return;
						}
						_vm->_championMan->drawChangedObjectIcons();
						_vm->_championMan->_champions[_vm->_championMan->_leaderIndex]._load += _vm->_dungeonMan->getObjectWeight(thingHandObject) - weight;
					}
					commandProcessType80ClickInDungeonViewTouchFrontWall();
				}
			}
		} else {
			if (isLeaderHandObjThrown(posX, posY))
				return;

			for (uint16 currViewCell = kDMViewCellFronLeft; currViewCell < kDMViewCellBackLeft + 1; currViewCell++) {
				if (boxObjectPiles[currViewCell].isPointInside(posX, posY)) {
					clickInDungeonViewDropLeaderHandObject(currViewCell);
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
	DungeonMan &dunMan = *_vm->_dungeonMan;
	TextMan &txtMan = *_vm->_textMan;

	uint16 championIndex = champMan._partyChampionCount - 1;
	Champion *champ = &champMan._champions[championIndex];
	if (commandType == kDMCommandClickInPanelCancel) {
		invMan.toggleInventory(kDMChampionCloseInventory);
		champMan._candidateChampionOrdinal = _vm->indexToOrdinal(kDMChampionNone);
		if (champMan._partyChampionCount == 1) {
			commandSetLeader(kDMChampionNone);
		}
		champMan._partyChampionCount--;
		Box box;
		box._rect.top = 0;
		box._rect.bottom = 28;
		box._rect.left = championIndex * kDMChampionStatusBoxSpacing;
		box._rect.right = box._rect.left + 66;
		dispMan._useByteBoxCoordinates = false;
		dispMan.fillScreenBox(box, kDMColorBlack);
		dispMan.fillScreenBox(_vm->_championMan->_boxChampionIcons[champMan.getChampionIconIndex(champ->_cell, dunMan._partyDir) * 2], kDMColorBlack);
		_vm->_menuMan->drawEnabledMenus();
		showMouse();
		return;
	}

	champMan._candidateChampionOrdinal = _vm->indexToOrdinal(kDMChampionNone);
	int16 mapX = dunMan._partyMapX + _vm->_dirIntoStepCountEast[dunMan._partyDir];
	int16 mapY = dunMan._partyMapY + _vm->_dirIntoStepCountNorth[dunMan._partyDir];

	for (uint16 slotIndex = kDMSlotReadyHand; slotIndex < kDMSlotChest1; slotIndex++) {
		Thing thing = champ->getSlot((ChampionSlot)slotIndex);
		if (thing != _vm->_thingNone) {
			_vm->_dungeonMan->unlinkThingFromList(thing, Thing(0), mapX, mapY);
		}
	}
	Thing thing = dunMan.getSquareFirstThing(mapX, mapY);
	for (;;) { // infinite
		if (thing.getType() == kDMThingTypeSensor) {
			((Sensor*)dunMan.getThingData(thing))->setTypeDisabled();
			break;
		}
		thing = dunMan.getNextThing(thing);
	}

	if (commandType == kDMCommandClickInPanelReincarnate) {
		champMan.renameChampion(champ);
		if (_vm->_engineShouldQuit)
			return;
		champ->resetSkillsToZero();

		for (uint16 i = 0; i < 12; i++) {
			uint16 statIndex = _vm->getRandomNumber(7);
			champ->getStatistic((ChampionStatType)statIndex, kDMStatCurrent)++; // returns reference
			champ->getStatistic((ChampionStatType)statIndex, kDMStatMaximum)++; // returns reference
		}
	}

	if (champMan._partyChampionCount == 1) {
		_vm->_projexpl->_lastPartyMovementTime = _vm->_gameTime;
		commandSetLeader(kDMChampionFirst);
		_vm->_menuMan->setMagicCasterAndDrawSpellArea(kDMChampionFirst);
	} else
		_vm->_menuMan->drawSpellAreaControls(champMan._magicCasterChampionIndex);

	txtMan.printLineFeed();
	Color champColor = _vm->_championMan->_championColor[championIndex];
	txtMan.printMessage(champColor, champ->_name);

	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		txtMan.printMessage(champColor, (commandType == kDMCommandClickInPanelResurrect) ? " RESURRECTED." : " REINCARNATED.");
		break;
	case Common::DE_DEU:
		txtMan.printMessage(champColor, (commandType == kDMCommandClickInPanelResurrect) ? " VOM TODE ERWECKT." : " REINKARNIERT.");
		break;
	case Common::FR_FRA:
		txtMan.printMessage(champColor, (commandType == kDMCommandClickInPanelResurrect) ? " RESSUSCITE." : " REINCARNE.");
		break;
	}

	invMan.toggleInventory(kDMChampionCloseInventory);
	_vm->_menuMan->drawEnabledMenus();
	setMousePointerToNormal((_vm->_championMan->_leaderIndex == kDMChampionNone) ? k0_pointerArrow : k1_pointerHand);
}

void EventManager::commandProcess81ClickInPanel(int16 x, int16 y) {
	ChampionMan &champMan = *_vm->_championMan;
	InventoryMan &invMan = *_vm->_inventoryMan;

	CommandType commandType;
	switch (invMan._panelContent) {
	case kDMPanelContentChest:
		if (champMan._leaderIndex == kDMChampionNone) // if no leader
			return;
		commandType = getCommandTypeFromMouseInput(_mouseInputPanelChest, Common::Point(x, y), kDMMouseButtonLeft);
		if (commandType != kDMCommandNone)
			champMan.clickOnSlotBox(commandType - kDMCommandClickOnSlotBoxChampion0StatusBoxReadyHand);
		break;
	case kDMPanelContentResurrectReincarnate:
		if (!champMan._leaderEmptyHanded)
			break;
		commandType = getCommandTypeFromMouseInput(_mouseInputPanelResurrectReincarnateCancel, Common::Point(x, y), kDMMouseButtonLeft);
		if (commandType != kDMCommandNone)
			commandProcessCommands160To162ClickInResurrectReincarnatePanel(commandType);
		break;
	default:
		break;
	}
}

void EventManager::processType80_clickInDungeonView_grabLeaderHandObject(uint16 viewCell) {
	if (_vm->_championMan->_leaderIndex == kDMChampionNone)
		return;

	int16 mapX = _vm->_dungeonMan->_partyMapX;
	int16 mapY = _vm->_dungeonMan->_partyMapY;
	if (viewCell >= kDMViewCellBackRight) {
		mapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir];
		mapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];
		Thing groupThing = _vm->_groupMan->groupGetThing(mapX, mapY);
		if ((groupThing != _vm->_thingEndOfList) &&
			!_vm->_moveSens->isLevitating(groupThing) &&
			_vm->_groupMan->getCreatureOrdinalInCell((Group*)_vm->_dungeonMan->getThingData(groupThing), _vm->normalizeModulo4(viewCell + _vm->_dungeonMan->_partyDir))) {
			return; /* It is not possible to grab an object on floor if there is a non levitating creature on its cell */
		}
	}

	Thing topPileThing = _vm->_dungeonMan->_pileTopObject[viewCell];
	if (_vm->_objectMan->getIconIndex(topPileThing) != kDMIconIndiceNone) {
		_vm->_moveSens->getMoveResult(topPileThing, mapX, mapY, kDMMapXNotOnASquare, 0);
		_vm->_championMan->putObjectInLeaderHand(topPileThing, true);
	}

	_vm->_stopWaitingForPlayerInput = true;
}

void EventManager::clickInDungeonViewDropLeaderHandObject(uint16 viewCell) {
	if (_vm->_championMan->_leaderIndex == kDMChampionNone)
		return;

	int16 mapX = _vm->_dungeonMan->_partyMapX;
	int16 mapY = _vm->_dungeonMan->_partyMapY;
	bool droppingIntoAnAlcove = (viewCell == kDMViewCellAlcove);
	if (droppingIntoAnAlcove)
		viewCell = kDMViewCellBackRight;

	if (viewCell > kDMViewCellFrontRight) {
		mapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir];
		mapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];
	}

	uint16 currCell = _vm->normalizeModulo4(_vm->_dungeonMan->_partyDir + viewCell);
	Thing removedThing = _vm->_championMan->getObjectRemovedFromLeaderHand();
	_vm->_moveSens->getMoveResult(_vm->thingWithNewCell(removedThing, currCell), kDMMapXNotOnASquare, 0, mapX, mapY);
	if (droppingIntoAnAlcove && _vm->_dungeonMan->_isFacingViAltar && (_vm->_objectMan->getIconIndex(removedThing) == kDMIconIndiceJunkChampionBones)) {
		Junk *removedJunk = (Junk*)_vm->_dungeonMan->getThingData(removedThing);
		TimelineEvent newEvent;
		newEvent._mapTime = _vm->setMapAndTime(_vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + 1);
		newEvent._type = kDMEventTypeViAltarRebirth;
		newEvent._priority = removedJunk->getChargeCount();
		newEvent._Bu._location._mapX = mapX;
		newEvent._Bu._location._mapY = mapY;
		newEvent._Cu.A._cell = currCell;
		newEvent._Cu.A._effect = kDMSensorEffectToggle;
		_vm->_timeline->addEventGetEventIndex(&newEvent);
	}
	_vm->_stopWaitingForPlayerInput = true;
}

bool EventManager::hasPendingClick(Common::Point& point, MouseButton button) {
	if (_pendingClickButton && button == _pendingClickButton)
		point = _pendingClickPos;

	return _pendingClickPresent;
}

void EventManager::drawSleepScreen() {
	DisplayMan &displMan = *_vm->_displayMan;
	TextMan &txtMan = *_vm->_textMan;

	displMan.fillBitmap(displMan._bitmapViewport, kDMColorBlack, 112, 136);
	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, 93, 69, kDMColorCyan, kDMColorBlack, "WAKE UP", k136_heightViewport);
		break;
	case Common::DE_DEU:
		txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, 96, 69, kDMColorCyan, kDMColorBlack, "WECKEN", k136_heightViewport);
		break;
	case Common::FR_FRA:
		txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, 72, 69, kDMColorCyan, kDMColorBlack, "REVEILLEZ-VOUS", k136_heightViewport);
		break;
	}
}

void EventManager::discardAllInput() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event) && !_vm->_engineShouldQuit) {
		if (event.type == Common::EVENT_QUIT)
			_vm->_engineShouldQuit = true;
	}
	_commandQueue.clear();
}

void EventManager::commandTakeStairs(bool stairsGoDown) {
	_vm->_moveSens->getMoveResult(_vm->_thingParty, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, kDMMapXNotOnASquare, 0);
	_vm->_newPartyMapIndex = _vm->_dungeonMan->getLocationAfterLevelChange(_vm->_dungeonMan->_partyMapIndex, stairsGoDown ? -1 : 1, &_vm->_dungeonMan->_partyMapX, &_vm->_dungeonMan->_partyMapY);
	_vm->_dungeonMan->setCurrentMap(_vm->_newPartyMapIndex);
	_vm->_championMan->setPartyDirection(_vm->_dungeonMan->getStairsExitDirection(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY));
	_vm->_dungeonMan->setCurrentMap(_vm->_dungeonMan->_partyMapIndex);
}

void EventManager::commandProcessTypes12to27_clickInChampionStatusBox(uint16 champIndex, int16 posX, int16 posY) {
	if (_vm->indexToOrdinal(champIndex) == _vm->_inventoryMan->_inventoryChampionOrdinal) {
		commandSetLeader((ChampionIndex)champIndex);
	} else {
		uint16 commandType = getCommandTypeFromMouseInput(_mouseInputChampionNamesHands, Common::Point(posX, posY), kDMMouseButtonLeft);
		if ((commandType >= kDMCommandSetLeaderChampion0) && (commandType <= kDMCommandSetLeaderChampion3))
			commandSetLeader((ChampionIndex)(commandType - kDMCommandSetLeaderChampion0));
		else if ((commandType >= kDMCommandClickOnSlotBoxChampion0StatusBoxReadyHand) && (commandType <= kDMCommandClickOnSlotBoxChampion3StatusBoxActionHand))
			_vm->_championMan->clickOnSlotBox(commandType - kDMCommandClickOnSlotBoxChampion0StatusBoxReadyHand);
	}
}

void EventManager::mouseProcessCommands125To128_clickOnChampionIcon(uint16 champIconIndex) {
	static Box championIconShadowBox = Box(2, 20, 2, 15);
	static Box championIconBox = Box(0, 18, 0, 13);
	static byte mousePointerIconShadowBox[16] = {0, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 0, 120, 120, 120};

	DisplayMan &displMan = *_vm->_displayMan;

	_preventBuildPointerScreenArea = true;
	if (!_useChampionIconOrdinalAsMousePointerBitmap) {
		if (_vm->_championMan->getIndexInCell(_vm->normalizeModulo4(champIconIndex + _vm->_dungeonMan->_partyDir)) == kDMChampionNone) {
			_preventBuildPointerScreenArea = false;
			return;
		}
		_mousePointerBitmapUpdated = true;
		_useChampionIconOrdinalAsMousePointerBitmap = true;
		displMan._useByteBoxCoordinates = false;
		byte *tmpBitmap = _mousePointerTempBuffer;
		memset(tmpBitmap, 0, 32 * 18);
		Box *curChampionIconBox = &_vm->_championMan->_boxChampionIcons[champIconIndex];

		displMan.blitToBitmap(displMan._bitmapScreen, tmpBitmap, championIconShadowBox, curChampionIconBox->_rect.left, curChampionIconBox->_rect.top, k160_byteWidthScreen, k16_byteWidth, kDMColorBlack, 200, 18);
		displMan.blitToBitmapShrinkWithPalChange(tmpBitmap, _mousePointerOriginalColorsChampionIcon, 32, 18, 32, 18, mousePointerIconShadowBox);
		displMan.blitToBitmap(displMan._bitmapScreen, _mousePointerOriginalColorsChampionIcon, championIconBox, curChampionIconBox->_rect.left, curChampionIconBox->_rect.top, k160_byteWidthScreen, k16_byteWidth, kDMColorBlack, 200, 18);
		displMan.fillScreenBox(*curChampionIconBox, kDMColorBlack);
		_useChampionIconOrdinalAsMousePointerBitmap = _vm->indexToOrdinal(champIconIndex);
	} else {
		_mousePointerBitmapUpdated = true;
		uint16 championIconIndex = _vm->ordinalToIndex(_useChampionIconOrdinalAsMousePointerBitmap);
		_useChampionIconOrdinalAsMousePointerBitmap = _vm->indexToOrdinal(kDMChampionNone);
		int16 championCellIndex = _vm->_championMan->getIndexInCell(_vm->normalizeModulo4(championIconIndex + _vm->_dungeonMan->_partyDir));
		if (championIconIndex == champIconIndex) {
			setFlag(_vm->_championMan->_champions[championCellIndex]._attributes, kDMAttributeIcon);
			_vm->_championMan->drawChampionState((ChampionIndex)championCellIndex);
		} else {
			int16 championIndex = _vm->_championMan->getIndexInCell(_vm->normalizeModulo4(champIconIndex + _vm->_dungeonMan->_partyDir));
			if (championIndex >= 0) {
				_vm->_championMan->_champions[championIndex]._cell = (ViewCell)_vm->normalizeModulo4(championIconIndex + _vm->_dungeonMan->_partyDir);
				setFlag(_vm->_championMan->_champions[championIndex]._attributes, kDMAttributeIcon);
				_vm->_championMan->drawChampionState((ChampionIndex)championIndex);
			} else
				displMan.fillScreenBox(_vm->_championMan->_boxChampionIcons[championIconIndex], kDMColorBlack);

			_vm->_championMan->_champions[championCellIndex]._cell = (ViewCell)_vm->normalizeModulo4(champIconIndex + _vm->_dungeonMan->_partyDir);
			setFlag(_vm->_championMan->_champions[championCellIndex]._attributes, kDMAttributeIcon);
			_vm->_championMan->drawChampionState((ChampionIndex)championCellIndex);
		}
	}
	_preventBuildPointerScreenArea = false;
	buildpointerScreenArea(_mousePos.x, _mousePos.y);
}

void EventManager::commandProcessType100_clickInSpellArea(uint16 posX, uint16 posY) {
	ChampionIndex championIndex = kDMChampionNone;
	if (posY <= 48) {
		switch (_vm->_championMan->_magicCasterChampionIndex) {
		case 0:
			if ((posX >= 280) && (posX <= 291))
				championIndex = kDMChampionSecond;
			else if ((posX >= 294) && (posX <= 305))
				championIndex = kDMChampionThird;
			else if (posX >= 308)
				championIndex = kDMChampionFourth;

			break;
		case 1:
			if ((posX >= 233) && (posX <= 244))
				championIndex = kDMChampionFirst;
			else if ((posX >= 294) && (posX <= 305))
				championIndex = kDMChampionThird;
			else if (posX >= 308)
				championIndex = kDMChampionFourth;

			break;
		case 2:
			if ((posX >= 233) && (posX <= 244))
				championIndex = kDMChampionFirst;
			else if ((posX >= 247) && (posX <= 258))
				championIndex = kDMChampionSecond;
			else if (posX >= 308)
				championIndex = kDMChampionFourth;

			break;
		case 3:
			if ((posX >= 247) && (posX <= 258))
				championIndex = kDMChampionSecond;
			else if ((posX >= 261) && (posX <= 272))
				championIndex = kDMChampionThird;
			else if (posX <= 244)
				championIndex = kDMChampionFirst;
			break;
		default:
			break;
		}

		if ((championIndex != kDMChampionNone) && (championIndex < _vm->_championMan->_partyChampionCount))
			_vm->_menuMan->setMagicCasterAndDrawSpellArea(championIndex);

		return;
	}

	CommandType newCommand = getCommandTypeFromMouseInput(_mouseInputSpellArea, Common::Point(posX, posY), kDMMouseButtonLeft);
	if (newCommand != kDMCommandNone)
		commandProcessTypes101To108_clickInSpellSymbolsArea(newCommand);
}

void EventManager::commandProcessTypes101To108_clickInSpellSymbolsArea(CommandType cmdType) {
	static Box spellSymbolsAndDelete[7] = {
		/* { X1, X2, Y1, Y2 } */
		Box(235, 247, 51, 61),   /* Symbol 1 */
		Box(249, 261, 51, 61),   /* Symbol 2 */
		Box(263, 275, 51, 61),   /* Symbol 3 */
		Box(277, 289, 51, 61),   /* Symbol 4 */
		Box(291, 303, 51, 61),   /* Symbol 5 */
		Box(305, 317, 51, 61),   /* Symbol 6 */
		Box(305, 318, 63, 73)    /* Delete */
	};

	if (cmdType == kDMCommandClickInSpeallAreaCastSpell) {
		if (_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex]._symbols[0] == '\0')
			return;

		commandHighlightBoxEnable(234, 303, 63, 73);
		_vm->_stopWaitingForPlayerInput = _vm->_menuMan->getClickOnSpellCastResult();
		return;
	}

	uint16 symbolIndex = cmdType - kDMCommandClickInSpellAreaSymbol1;
	Box *highlightBox = &spellSymbolsAndDelete[symbolIndex];
	commandHighlightBoxEnable(highlightBox->_rect.left, highlightBox->_rect.right, highlightBox->_rect.top, highlightBox->_rect.bottom);
	_vm->delay(1);
	highlightBoxDisable();

	if (symbolIndex < 6)
		_vm->_menuMan->addChampionSymbol(symbolIndex);
	else
		_vm->_menuMan->deleteChampionSymbol();
}

void EventManager::commandProcessType111To115_ClickInActionArea(int16 posX, int16 posY) {
	if (_vm->_championMan->_actingChampionOrdinal) {
		uint16 mouseCommand = getCommandTypeFromMouseInput(_mouseInputActionAreaNames, Common::Point(posX, posY), kDMMouseButtonLeft);
		if (mouseCommand != kDMCommandNone) {
			if (mouseCommand == kDMCommandClickInActionAreaPass) {
				commandHighlightBoxEnable(285, 319, 77, 83);
				_vm->_menuMan->didClickTriggerAction(-1);
			} else if ((mouseCommand - kDMCommandClickInActionAreaPass) <= _vm->_menuMan->_actionCount) {
				if (mouseCommand == kDMCommandClickInActionAreaAction0)
					commandHighlightBoxEnable(234, 318, 86, 96);
				else if (mouseCommand == kDMCommandClickInActionAreaAction1)
					commandHighlightBoxEnable(234, 318, 98, 108);
				else
					commandHighlightBoxEnable(234, 318, 110, 120);

				_vm->_stopWaitingForPlayerInput = _vm->_menuMan->didClickTriggerAction(mouseCommand - kDMCommandClickInActionAreaAction0);
			}
		}
	} else if (_vm->_menuMan->_actionAreaContainsIcons) {
		uint16 mouseCommand = getCommandTypeFromMouseInput(_mouseInputActionAreaIcons, Common::Point(posX, posY), kDMMouseButtonLeft);
		if (mouseCommand != kDMCommandNone) {
			mouseCommand -= kDMCommandClickInActionAreaChampion0Action;
			if (mouseCommand < _vm->_championMan->_partyChampionCount)
				_vm->_menuMan->processCommands116To119_setActingChampion(mouseCommand);
		}
	}
}

void EventManager::resetPressingEyeOrMouth() {
	if (_vm->_pressingEye) {
		_ignoreMouseMovements = false;
		_vm->_stopPressingEye = true;
	}
	if (_vm->_pressingMouth) {
		_ignoreMouseMovements = false;
		_vm->_stopPressingMouth = true;
	}
}

void EventManager::waitForMouseOrKeyActivity() {
	discardAllInput();
	Common::Event event;
	DisplayMan &displMan = *_vm->_displayMan;
	while (true) {
		if (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				_vm->_engineShouldQuit = true;
			case Common::EVENT_KEYDOWN: // Intentional fall through
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
				return;
			default:
				break;
			}
		}
		_vm->delay(1);
		displMan.updateScreen();
	}
}

void EventManager::commandHighlightBoxEnable(int16 x1, int16 x2, int16 y1, int16 y2) {
	_highlightScreenBox = Box(x1, x2, y1, y2);
	highlightScreenBox(x1, x2, y1, y2);
	_highlightBoxEnabled = true;
}

void EventManager::highlightBoxDisable() {
	if (_highlightBoxEnabled == true) {
		highlightScreenBox(_highlightScreenBox._rect.left, _highlightScreenBox._rect.right, _highlightScreenBox._rect.top, _highlightScreenBox._rect.bottom);
		_highlightBoxEnabled = false;
	}
}

} // end of namespace DM
