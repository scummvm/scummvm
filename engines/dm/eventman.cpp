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
#include "timeline.h"
#include "projexpl.h"
#include "text.h"
#include "group.h"
#include "dialog.h"
#include "sounds.h"
#include <mortevielle/saveload.h>


namespace DM {

void EventManager::initArrays() {
	KeyboardInput primaryKeyboardInputInterface[7] = { // @ G0458_as_Graphic561_PrimaryKeyboardInput_Interface
		/* { Command, Code } */
		KeyboardInput(k7_CommandToggleInventoryChampion_0, Common::KEYCODE_F1, 0), /* F1 (<CSI>1~) Atari ST: Code = 0x3B00 */
		KeyboardInput(k8_CommandToggleInventoryChampion_1, Common::KEYCODE_F2, 0), /* F2 (<CSI>2~) Atari ST: Code = 0x3C00 */
		KeyboardInput(k9_CommandToggleInventoryChampion_2, Common::KEYCODE_F3, 0), /* F3 (<CSI>3~) Atari ST: Code = 0x3D00 */
		KeyboardInput(k10_CommandToggleInventoryChampion_3, Common::KEYCODE_F4, 0), /* F4 (<CSI>4~) Atari ST: Code = 0x3E00 */
		KeyboardInput(k140_CommandSaveGame, Common::KEYCODE_s, Common::KBD_CTRL), /* CTRL-S       Atari ST: Code = 0x0013 */
		KeyboardInput(k147_CommandFreezeGame, Common::KEYCODE_ESCAPE, 0), /* Esc (0x1B)   Atari ST: Code = 0x001B */
		KeyboardInput(k0_CommandNone, Common::KEYCODE_INVALID, 0)
	};

	KeyboardInput secondaryKeyboardInputMovement[19] = { // @ G0459_as_Graphic561_SecondaryKeyboardInput_Movement
		/* { Command, Code } */
		KeyboardInput(k1_CommandTurnLeft, Common::KEYCODE_KP4, 0), /* Numeric pad 4 Atari ST: Code = 0x5200 */
		KeyboardInput(k3_CommandMoveForward, Common::KEYCODE_KP5, 0), /* Numeric pad 5 Atari ST: Code = 0x4800 */
		KeyboardInput(k2_CommandTurnRight, Common::KEYCODE_KP6, 0), /* Numeric pad 6 Atari ST: Code = 0x4700 */
		KeyboardInput(k6_CommandMoveLeft, Common::KEYCODE_KP1, 0), /* Numeric pad 1 Atari ST: Code = 0x4B00 */
		KeyboardInput(k5_CommandMoveBackward, Common::KEYCODE_KP2, 0), /* Numeric pad 2 Atari ST: Code = 0x5000 */
		KeyboardInput(k4_CommandMoveRight, Common::KEYCODE_KP3, 0), /* Numeric pad 3 Atari ST: Code = 0x4D00. Remaining entries below not present */
		KeyboardInput(k3_CommandMoveForward, Common::KEYCODE_w, 0), /* Up Arrow (<CSI>A) */ /*Differs for testing convenience*/
		KeyboardInput(k3_CommandMoveForward, Common::KEYCODE_w, Common::KBD_SHIFT), /* Shift Up Arrow (<CSI>T) */ /*Differs for testing convenience*/
		KeyboardInput(k6_CommandMoveLeft, Common::KEYCODE_a, 0), /* Backward Arrow (<CSI>D) */ /*Differs for testing convenience*/
		KeyboardInput(k6_CommandMoveLeft, Common::KEYCODE_a, Common::KBD_SHIFT), /* Shift Forward Arrow (<CSI> A) */ /*Differs for testing convenience*/
		KeyboardInput(k4_CommandMoveRight, Common::KEYCODE_d, 0), /* Forward Arrow (<CSI>C) */ /*Differs for testing convenience*/
		KeyboardInput(k4_CommandMoveRight, Common::KEYCODE_d, Common::KBD_SHIFT), /* Shift Backward Arrow (<CSI> @) */ /*Differs for testing convenience*/
		KeyboardInput(k5_CommandMoveBackward, Common::KEYCODE_s, 0), /* Down arrow (<CSI>B) */ /*Differs for testing convenience*/
		KeyboardInput(k5_CommandMoveBackward, Common::KEYCODE_s, Common::KBD_SHIFT), /* Shift Down arrow (<CSI>S) */ /*Differs for testing convenience*/
		KeyboardInput(k1_CommandTurnLeft, Common::KEYCODE_q, 0), /* Del (0x7F) */ /*Differs for testing convenience*/
		KeyboardInput(k1_CommandTurnLeft, Common::KEYCODE_q, Common::KBD_SHIFT), /* Shift Del (0x7F) */ /*Differs for testing convenience*/
		KeyboardInput(k2_CommandTurnRight, Common::KEYCODE_e, 0), /* Help (<CSI>?~) */ /*Differs for testing convenience*/
		KeyboardInput(k2_CommandTurnRight, Common::KEYCODE_e, Common::KBD_SHIFT), /* Shift Help (<CSI>?~) */ /*Differs for testing convenience*/
		KeyboardInput(k0_CommandNone, Common::KEYCODE_INVALID, 0)
	};
	KeyboardInput primaryKeyboardInputPartySleeping[3] = { // @ G0460_as_Graphic561_PrimaryKeyboardInput_PartySleeping
		/* { Command, Code } */
		KeyboardInput(k146_CommandWakeUp, Common::KEYCODE_RETURN, 0), /* Return */
		KeyboardInput(k147_CommandFreezeGame, Common::KEYCODE_ESCAPE, 0), /* Esc */
		KeyboardInput(k0_CommandNone, Common::KEYCODE_INVALID, 0)
	};
	KeyboardInput primaryKeyboardInputFrozenGame[2] = { // @ G0461_as_Graphic561_PrimaryKeyboardInput_FrozenGame
		/* { Command, Code } */
		KeyboardInput(k148_CommandUnfreezeGame, Common::KEYCODE_ESCAPE, 0), /* Esc */
		KeyboardInput(k0_CommandNone, Common::KEYCODE_INVALID, 0)
	};
	MouseInput primaryMouseInputEntrance[4] = { // @ G0445_as_Graphic561_PrimaryMouseInput_Entrance[4]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k200_CommandEntranceEnterDungeon, 244, 298,  45,  58, k1_LeftMouseButton),
		// Strangerke - C201_COMMAND_ENTRANCE_RESUME isn't present in the demo
		MouseInput(k201_CommandEntranceResume,        244, 298,  76,  93, k1_LeftMouseButton),
		MouseInput(k202_CommandEntranceDrawCredits,  248, 293, 187, 199, k1_LeftMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputRestartGame[2] = { // @ G0446_as_Graphic561_PrimaryMouseInput_RestartGame[2]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k215_CommandRestartGame, 103, 217, 145, 159, k1_LeftMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputInterface[20] = { // @ G0447_as_Graphic561_PrimaryMouseInput_Interface[20]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k12_CommandClickInChampion_0_StatusBox,       0,  42,   0,  28, k1_LeftMouseButton),
		MouseInput(k13_CommandClickInChampion_1_StatusBox,      69, 111,   0,  28, k1_LeftMouseButton),
		MouseInput(k14_CommandClickInChampion_2_StatusBox,     138, 180,   0,  28, k1_LeftMouseButton),
		MouseInput(k15_CommandClickInChampion_3_StatusBox,     207, 249,   0,  28, k1_LeftMouseButton),
		MouseInput(k125_CommandClickOnChamptionIcon_Top_Left,    274, 299,   0,  13, k1_LeftMouseButton),
		MouseInput(k126_CommandClickOnChamptionIcon_Top_Right,   301, 319,   0,  13, k1_LeftMouseButton),
		MouseInput(k127_CommandClickOnChamptionIcon_Lower_Right, 301, 319,  15,  28, k1_LeftMouseButton),
		MouseInput(k128_CommandClickOnChamptionIcon_Lower_Left,  274, 299,  15,  28, k1_LeftMouseButton),
		MouseInput(k7_CommandToggleInventoryChampion_0,         43,  66,   0,  28, k1_LeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 =  44. swapped with 4 next entries */
		MouseInput(k8_CommandToggleInventoryChampion_1,        112, 135,   0,  28, k1_LeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 113. swapped with 4 next entries */
		MouseInput(k9_CommandToggleInventoryChampion_2,        181, 204,   0,  28, k1_LeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 182. swapped with 4 next entries */
		MouseInput(k10_CommandToggleInventoryChampion_3,        250, 273,   0,  28, k1_LeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 251. swapped with 4 next entries */
		MouseInput(k7_CommandToggleInventoryChampion_0,          0,  66,   0,  28, k2_RightMouseButton), /* Atari ST: swapped with 4 previous entries */
		MouseInput(k8_CommandToggleInventoryChampion_1,         69, 135,   0,  28, k2_RightMouseButton), /* Atari ST: swapped with 4 previous entries */
		MouseInput(k9_CommandToggleInventoryChampion_2,        138, 204,   0,  28, k2_RightMouseButton), /* Atari ST: swapped with 4 previous entries */
		MouseInput(k10_CommandToggleInventoryChampion_3,        207, 273,   0,  28, k2_RightMouseButton), /* Atari ST: swapped with 4 previous entries */
		MouseInput(k100_CommandClickInSpellArea,                233, 319,  42,  73, k1_LeftMouseButton),
		MouseInput(k111_CommandClickInActionArea,               233, 319,  77, 121, k1_LeftMouseButton),
		MouseInput(k147_CommandFreezeGame,                          0,   1, 198, 199, k1_LeftMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput secondaryMouseInputMovement[9] = { // @ G0448_as_Graphic561_SecondaryMouseInput_Movement[9]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k1_CommandTurnLeft,             234, 261, 125, 145, k1_LeftMouseButton),
		MouseInput(k3_CommandMoveForward,          263, 289, 125, 145, k1_LeftMouseButton),
		MouseInput(k2_CommandTurnRight,            291, 318, 125, 145, k1_LeftMouseButton),
		MouseInput(k6_CommandMoveLeft,             234, 261, 147, 167, k1_LeftMouseButton),
		MouseInput(k5_CommandMoveBackward,         263, 289, 147, 167, k1_LeftMouseButton),
		MouseInput(k4_CommandMoveRight,            291, 318, 147, 167, k1_LeftMouseButton),
		MouseInput(k80_CommandClickInDungeonView,   0, 223,  33, 168, k1_LeftMouseButton),
		MouseInput(k83_CommandToggleInventoryLeader, 0, 319,  33, 199, k2_RightMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput secondaryMouseInputChampionInventory[38] = { // @ G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory[38]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k11_CommandCloseInventory,                                   0, 319,   0, 199, k2_RightMouseButton),
		MouseInput(k140_CommandSaveGame,                                       174, 182,  36,  44, k1_LeftMouseButton),
		MouseInput(k145_CommandSleep,                                           188, 204,  36,  44, k1_LeftMouseButton),
		MouseInput(k11_CommandCloseInventory,                                 210, 218,  36,  44, k1_LeftMouseButton),
		MouseInput(k28_CommandClickOnSlotBoxInventoryReadyHand ,        6,  21,  86, 101, k1_LeftMouseButton),
		MouseInput(k29_CommandClickOnSlotBoxInventoryActionHand,       62,  77,  86, 101, k1_LeftMouseButton),
		MouseInput(k30_CommandClickOnSlotBoxInventoryHead,              34,  49,  59,  74, k1_LeftMouseButton),
		MouseInput(k31_CommandClickOnSlotBoxInventoryTorso,             34,  49,  79,  94, k1_LeftMouseButton),
		MouseInput(k32_CommandClickOnSlotBoxInventoryLegs,              34,  49,  99, 114, k1_LeftMouseButton),
		MouseInput(k33_CommandClickOnSlotBoxInventoryFeet,              34,  49, 119, 134, k1_LeftMouseButton),
		MouseInput(k34_CommandClickOnSlotBoxInventoryPouch_2,            6,  21, 123, 138, k1_LeftMouseButton),
		MouseInput(k70_CommandClickOnMouth,                                   56,  71,  46,  61, k1_LeftMouseButton),
		MouseInput(k71_CommandClickOnEye,                                     12,  27,  46,  61, k1_LeftMouseButton),
		MouseInput(k35_CommandClickOnSlotBoxInventoryQuiverLine_2_1,    79,  94, 106, 121, k1_LeftMouseButton),
		MouseInput(k36_CommandClickOnSlotBoxInventoryQuiverLine_1_2,    62,  77, 123, 138, k1_LeftMouseButton),
		MouseInput(k37_CommandClickOnSlotBoxInventoryQuiverLine_2_2,    79,  94, 123, 138, k1_LeftMouseButton),
		MouseInput(k38_CommandClickOnSlotBoxInventoryNeck,               6,  21,  66,  81, k1_LeftMouseButton),
		MouseInput(k39_CommandClickOnSlotBoxInventoryPouch_1,            6,  21, 106, 121, k1_LeftMouseButton),
		MouseInput(k40_CommandClickOnSlotBoxInventoryQuiverLine_1_1,    62,  77, 106, 121, k1_LeftMouseButton),
		MouseInput(k41_CommandClickOnSlotBoxInventoryBackpackLine_1_1,  66,  81,  66,  81, k1_LeftMouseButton),
		MouseInput(k42_CommandClickOnSlotBoxInventoryBackpackLine_2_2,  83,  98,  49,  64, k1_LeftMouseButton),
		MouseInput(k43_CommandClickOnSlotBoxInventoryBackpackLine_2_3, 100, 115,  49,  64, k1_LeftMouseButton),
		MouseInput(k44_CommandClickOnSlotBoxInventoryBackpackLine_2_4, 117, 132,  49,  64, k1_LeftMouseButton),
		MouseInput(k45_CommandClickOnSlotBoxInventoryBackpackLine_2_5, 134, 149,  49,  64, k1_LeftMouseButton),
		MouseInput(k46_CommandClickOnSlotBoxInventoryBackpackLine_2_6, 151, 166,  49,  64, k1_LeftMouseButton),
		MouseInput(k47_CommandClickOnSlotBoxInventoryBackpackLine_2_7, 168, 183,  49,  64, k1_LeftMouseButton),
		MouseInput(k48_CommandClickOnSlotBoxInventoryBackpackLine_2_8, 185, 200,  49,  64, k1_LeftMouseButton),
		MouseInput(k49_CommandClickOnSlotBoxInventoryBackpackLine_2_9, 202, 217,  49,  64, k1_LeftMouseButton),
		MouseInput(k50_CommandClickOnSlotBoxInventoryBackpackLine_1_2,  83,  98,  66,  81, k1_LeftMouseButton),
		MouseInput(k51_CommandClickOnSlotBoxInventoryBackpackLine_1_3, 100, 115,  66,  81, k1_LeftMouseButton),
		MouseInput(k52_CommandClickOnSlotBoxInventoryBackpackLine_1_4, 117, 132,  66,  81, k1_LeftMouseButton),
		MouseInput(k53_CommandClickOnSlotBoxInventoryBackpackLine_1_5, 134, 149,  66,  81, k1_LeftMouseButton),
		MouseInput(k54_CommandClickOnSlotBoxInventoryBackpackLine_1_6, 151, 166,  66,  81, k1_LeftMouseButton),
		MouseInput(k55_CommandClickOnSlotBoxInventoryBackpackLine_1_7, 168, 183,  66,  81, k1_LeftMouseButton),
		MouseInput(k56_CommandClickOnSlotBoxInventoryBackpackLine_1_8, 185, 200,  66,  81, k1_LeftMouseButton),
		MouseInput(k57_CommandClickOnSlotBoxInventoryBackpackLine_1_9, 202, 217,  66,  81, k1_LeftMouseButton),
		MouseInput(k81_CommandClickInPanel,                                   96, 223,  83, 167, k1_LeftMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputPartySleeping[3] = { // @ G0450_as_Graphic561_PrimaryMouseInput_PartySleeping[3]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k146_CommandWakeUp, 0, 223, 33, 168, k1_LeftMouseButton),
		MouseInput(k146_CommandWakeUp, 0, 223, 33, 168, k2_RightMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputFrozenGame[3] = { // @ G0451_as_Graphic561_PrimaryMouseInput_FrozenGame[3]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k148_CommandUnfreezeGame, 0, 319, 0, 199, k1_LeftMouseButton),
		MouseInput(k148_CommandUnfreezeGame, 0, 319, 0, 199, k2_RightMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput mouseInputActionAreaNames[5] = { // @ G0452_as_Graphic561_MouseInput_ActionAreaNames[5]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k112_CommandClickInActionAreaPass,     285, 318,  77,  83, k1_LeftMouseButton),
		MouseInput(k113_CommandClickInActionAreaAction_0, 234, 318,  86,  96, k1_LeftMouseButton),
		MouseInput(k114_CommandClickInActionAreaAction_1, 234, 318,  98, 108, k1_LeftMouseButton),
		MouseInput(k115_CommandClickInActionAreaAction_2, 234, 318, 110, 120, k1_LeftMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput mouseInputActionAreaIcons[5] = { // @ G0453_as_Graphic561_MouseInput_ActionAreaIcons[5]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k116_CommandClickInActionAreaChampion_0_Action, 233, 252, 86, 120, k1_LeftMouseButton),
		MouseInput(k117_CommandClickInActionAreaChampion_1_Action, 255, 274, 86, 120, k1_LeftMouseButton),
		MouseInput(k118_CommandClickInActionAreaChampion_2_Action, 277, 296, 86, 120, k1_LeftMouseButton),
		MouseInput(k119_CommandClickInActionAreaChampion_3_Action, 299, 318, 86, 120, k1_LeftMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput mouseInputSpellArea[9] = { // @ G0454_as_Graphic561_MouseInput_SpellArea[9]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k101_CommandClickInSpellAreaSymbol_1,      235, 247, 51, 61, k1_LeftMouseButton),
		MouseInput(k102_CommandClickInSpellAreaSymbol_2,      249, 261, 51, 61, k1_LeftMouseButton),
		MouseInput(k103_CommandClickInSpellAreaSymbol_3,      263, 275, 51, 61, k1_LeftMouseButton),
		MouseInput(k104_CommandClickInSpellAreaSymbol_4,      277, 289, 51, 61, k1_LeftMouseButton),
		MouseInput(k105_CommandClickInSpellAreaSymbol_5,      291, 303, 51, 61, k1_LeftMouseButton),
		MouseInput(k106_CommandClickInSpellAreaSymbol_6,      305, 317, 51, 61, k1_LeftMouseButton),
		MouseInput(k108_CommandClickInSpeallAreaCastSpell,    234, 303, 63, 73, k1_LeftMouseButton),
		MouseInput(k107_CommandClickInSpellAreaRecantSymbol, 305, 318, 63, 73, k1_LeftMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput mouseInputChampionNamesHands[13] = { // @ G0455_as_Graphic561_MouseInput_ChampionNamesHands[13]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k16_CommandSetLeaderChampion_0,                                    0,  42,  0,  6, k1_LeftMouseButton),
		MouseInput(k17_CommandSetLeaderChampion_1,                                   69, 111,  0,  6, k1_LeftMouseButton),
		MouseInput(k18_CommandSetLeaderChampion_2,                                  138, 180,  0,  6, k1_LeftMouseButton),
		MouseInput(k19_CommandSetLeaderChampion_3,                                  207, 249,  0,  6, k1_LeftMouseButton),
		MouseInput(k20_CommandClickOnSlotBoxChampion_0_StatusBoxReadyHand,    4,  19, 10, 25, k1_LeftMouseButton),
		MouseInput(k21_CommandClickOnSlotBoxChampion_0_StatusBoxActionHand,  24,  39, 10, 25, k1_LeftMouseButton),
		MouseInput(k22_CommandClickOnSlotBoxChampion_1_StatusBoxReadyHand,   73,  88, 10, 25, k1_LeftMouseButton),
		MouseInput(k23_CommandClickOnSlotBoxChampion_1_StatusBoxActionHand,  93, 108, 10, 25, k1_LeftMouseButton),
		MouseInput(k24_CommandClickOnSlotBoxChampion_2_StatusBoxReadyHand,  142, 157, 10, 25, k1_LeftMouseButton),
		MouseInput(k25_CommandClickOnSlotBoxChampion_2_StatusBoxActionHand, 162, 177, 10, 25, k1_LeftMouseButton),
		MouseInput(k26_CommandClickOnSlotBoxChampion_3_StatusBoxReadyHand,  211, 226, 10, 25, k1_LeftMouseButton),
		MouseInput(k27_CommandClickOnSlotBoxChampion_3_StatusBoxActionHand, 231, 246, 10, 25, k1_LeftMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput mouseInputPanelChest[9] = { // @ G0456_as_Graphic561_MouseInput_PanelChest[9]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k58_CommandClickOnSlotBoxChest_1, 117, 132,  92, 107, k1_LeftMouseButton),
		MouseInput(k59_CommandClickOnSlotBoxChest_2, 106, 121, 109, 124, k1_LeftMouseButton),
		MouseInput(k60_CommandClickOnSlotBoxChest_3, 111, 126, 126, 141, k1_LeftMouseButton),
		MouseInput(k61_CommandClickOnSlotBoxChest_4, 128, 143, 131, 146, k1_LeftMouseButton),
		MouseInput(k62_CommandClickOnSlotBoxChest_5, 145, 160, 134, 149, k1_LeftMouseButton),
		MouseInput(k63_CommandClickOnSlotBoxChest_6, 162, 177, 136, 151, k1_LeftMouseButton),
		MouseInput(k64_CommandClickOnSlotBoxChest_7, 179, 194, 137, 152, k1_LeftMouseButton),
		MouseInput(k65_CommandClickOnSlotBoxChest_8, 196, 211, 138, 153, k1_LeftMouseButton),
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput mouseInputPanelResurrectReincarnateCancel[4] = { // @ G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel[4]
		/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
		MouseInput(k160_CommandClickInPanelResurrect,   108, 158,  90, 138, k1_LeftMouseButton), /* Atari ST: Box = 104, 158,  86, 142 */
		MouseInput(k161_CommandClickInPanelReincarnate, 161, 211,  90, 138, k1_LeftMouseButton), /* Atari ST: Box = 163, 217,  86, 142 */
		MouseInput(k162_CommandClickInPanelCancel,      108, 211, 141, 153, k1_LeftMouseButton), /* Atari ST: Box = 104, 217, 146, 156 */
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputViewportDialog1Choice[2] = { // @ G0471_as_Graphic561_PrimaryMouseInput_ViewportDialog1Choice[2]
		MouseInput(k210_CommandClickOnDialogChoice_1, 16, 207, 138, 152, k1_LeftMouseButton), /* Bottom button */
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputViewportDialog2Choices[3] = { // @ G0472_as_Graphic561_PrimaryMouseInput_ViewportDialog2Choices[3]
		MouseInput(k210_CommandClickOnDialogChoice_1, 16, 207, 101, 115, k1_LeftMouseButton), /* Top button */
		MouseInput(k211_CommandClickOnDialogChoice_2, 16, 207, 138, 152, k1_LeftMouseButton), /* Bottom button */
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputViewportDialog3Choices[4] = { // @ G0473_as_Graphic561_PrimaryMouseInput_ViewportDialog3Choices[4]
		MouseInput(k210_CommandClickOnDialogChoice_1,  16, 207, 101, 115, k1_LeftMouseButton), /* Top button */
		MouseInput(k211_CommandClickOnDialogChoice_2,  16, 101, 138, 152, k1_LeftMouseButton), /* Lower left button */
		MouseInput(k212_CommandClickOnDialogChoice_3, 123, 207, 138, 152, k1_LeftMouseButton), /* Lower right button */
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputViewportDialog4Choices[5] = { // @ G0474_as_Graphic561_PrimaryMouseInput_ViewportDialog4Choices[5]
		MouseInput(k210_CommandClickOnDialogChoice_1,  16, 101, 101, 115, k1_LeftMouseButton), /* Top left button */
		MouseInput(k211_CommandClickOnDialogChoice_2, 123, 207, 101, 115, k1_LeftMouseButton), /* Top right button */
		MouseInput(k212_CommandClickOnDialogChoice_3,  16, 101, 138, 152, k1_LeftMouseButton), /* Lower left button */
		MouseInput(k213_CommandClickOnDialogChoice_4, 123, 207, 138, 152, k1_LeftMouseButton), /* Lower right button */
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputScreenDialog1Choice[2] = { // @ G0475_as_Graphic561_PrimaryMouseInput_ScreenDialog1Choice[2]
		MouseInput(k210_CommandClickOnDialogChoice_1, 63, 254, 138, 152, k1_LeftMouseButton), /* Bottom button */
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputScreenDialog2Choices[3] = { // @ G0476_as_Graphic561_PrimaryMouseInput_ScreenDialog2Choices[3]
		MouseInput(k210_CommandClickOnDialogChoice_1, 63, 254, 101, 115, k1_LeftMouseButton), /* Top button */
		MouseInput(k211_CommandClickOnDialogChoice_2, 63, 254, 138, 152, k1_LeftMouseButton), /* Bottom button */
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputScreenDialog3Choices[4] = { // @ G0477_as_Graphic561_PrimaryMouseInput_ScreenDialog3Choices[4]
		MouseInput(k210_CommandClickOnDialogChoice_1,  63, 254, 101, 115, k1_LeftMouseButton), /* Top button */
		MouseInput(k211_CommandClickOnDialogChoice_2,  63, 148, 138, 152, k1_LeftMouseButton), /* Lower left button */
		MouseInput(k212_CommandClickOnDialogChoice_3, 170, 254, 138, 152, k1_LeftMouseButton), /* Lower right button */
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
	};
	MouseInput primaryMouseInputScreenDialog4Choices[5] = { // @ G0478_as_Graphic561_PrimaryMouseInput_ScreenDialog4Choices[5]
		MouseInput(k210_CommandClickOnDialogChoice_1,  63, 148, 101, 115, k1_LeftMouseButton), /* Top left button */
		MouseInput(k211_CommandClickOnDialogChoice_2, 170, 254, 101, 115, k1_LeftMouseButton), /* Top right button */
		MouseInput(k212_CommandClickOnDialogChoice_3,  63, 148, 138, 152, k1_LeftMouseButton), /* Lower left button */
		MouseInput(k213_CommandClickOnDialogChoice_4, 170, 254, 138, 152, k1_LeftMouseButton), /* Lower right button */
		MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)
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
		_mouseInputChampionNamesHands[13] = mouseInputChampionNamesHands[i];

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
	_pendingClickButton = k0_NoneMouseButton;
	_useObjectAsMousePointerBitmap = false;
	_useHandAsMousePointerBitmap = false;
	_preventBuildPointerScreenArea = false;
	_primaryKeyboardInput = nullptr;
	_secondaryKeyboardInput = nullptr;
	_ignoreMouseMovements = false;
	warning(false, "_g587_hideMousePointerRequestCount should start with value 1");
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

void EventManager::setPointerToObject(byte* bitmap) {
	static byte palChangesMousepointerOjbectIconShadow[16] = {120, 120, 120, 120, 120, 120, 120, 120,
		120, 120, 120, 120, 0, 120, 120, 120}; // @ K0027_auc_PaletteChanges_MousePointerObjectIconShadow
	static byte palChangesMousePointerIcon[16] = {120, 10, 20, 30, 40, 50, 60, 70, 80, 90,
		100, 110, 0, 130, 140, 150}; // @ G0044_auc_Graphic562_PaletteChanges_MousePointerIcon
	static Box boxMousePointerObjectShadow(2, 17, 2, 17); // @ G0619_s_Box_MousePointer_ObjectShadow 
	static Box boxMousePointerObject(0, 15, 0, 15); // @ G0620_s_Box_MousePointer_Object 

	_preventBuildPointerScreenArea = true;
	_useObjectAsMousePointerBitmap = true;
	_useHandAsMousePointerBitmap = false;
	_mousePointerBitmapUpdated = true;
	_vm->_displayMan->_useByteBoxCoordinates = true;
	byte *L0051_puc_Bitmap = _mousePointerOriginalColorsObject;
	memset(L0051_puc_Bitmap, 0, 32 * 18);

	_vm->_displayMan->blitToBitmapShrinkWithPalChange(bitmap, _mousePointerTempBuffer, 16, 16, 16, 16, palChangesMousepointerOjbectIconShadow);
	_vm->_displayMan->blitToBitmap(_mousePointerTempBuffer, L0051_puc_Bitmap, boxMousePointerObjectShadow, 0, 0, 8, 16, kM1_ColorNoTransparency, 16, 18);
	_vm->_displayMan->blitToBitmapShrinkWithPalChange(bitmap, _mousePointerTempBuffer, 16, 16, 16, 16, palChangesMousePointerIcon);
	_vm->_displayMan->blitToBitmap(_mousePointerTempBuffer, L0051_puc_Bitmap, boxMousePointerObject, 0, 0, 8, 16, k0_ColorBlack, 16, 18);

	_preventBuildPointerScreenArea = false;
	buildpointerScreenArea(_mousePos.x, _mousePos.y);
}

void EventManager::mouseDropChampionIcon() {
	_preventBuildPointerScreenArea = true;
	uint16 championIconIndex = _vm->ordinalToIndex(_useChampionIconOrdinalAsMousePointerBitmap);
	_useChampionIconOrdinalAsMousePointerBitmap = _vm->indexToOrdinal(kM1_ChampionNone);
	_mousePointerBitmapUpdated = true;
	bool useByteBoxCoordinatesBackup = _vm->_displayMan->_useByteBoxCoordinates;
	_vm->_displayMan->blitToScreen(_mousePointerOriginalColorsChampionIcon, &_vm->_championMan->_boxChampionIcons[championIconIndex << 2], 16, k12_ColorDarkestGray, 18);
	_vm->_displayMan->_useByteBoxCoordinates = useByteBoxCoordinatesBackup;
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
			if (championIdx == _vm->_inventoryMan->_g432_inventoryChampionOrdinal)
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
		}
	}
	_previousMousePointerType = _mousePointerType;
	_preventBuildPointerScreenArea = false;
}

void EventManager::setMousePointer() {
	if (_vm->_championMan->_leaderEmptyHanded)
		setMousePointerToNormal((_vm->_championMan->_leaderIndex == kM1_ChampionNone) ? k0_pointerArrow : k1_pointerHand);
	else
		setPointerToObject(_vm->_objectMan->_g412_objectIconForMousePointer);
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
	return (button != k0_NoneMouseButton) ? (_mouseButtonStatus & button) : (_mouseButtonStatus == 0);
}

void EventManager::setMousePos(Common::Point pos) {
	_vm->_system->warpMouse(pos.x, pos.y);
}

Common::EventType EventManager::processInput(Common::Event *grabKey, Common::Event *grabMouseClick) {
	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN: {
			if (event.synthetic)
				break;

			if (event.kbd.keycode == Common::KEYCODE_d && event.kbd.hasFlags(Common::KBD_CTRL)) {
				_vm->_console->attach();
				return Common::EVENT_INVALID;
			}

			if (grabKey) {
				*grabKey = event;
				return event.type;
			}

			if (_primaryKeyboardInput) {
				KeyboardInput *input = _primaryKeyboardInput;
				while (input->_commandToIssue != k0_CommandNone) {
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
				while (input->_commandToIssue != k0_CommandNone) {
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
			MouseButton button = (event.type == Common::EVENT_LBUTTONDOWN) ? k1_LeftMouseButton : k2_RightMouseButton;
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
			MouseButton button = (event.type == Common::EVENT_LBUTTONDOWN) ? k1_LeftMouseButton : k2_RightMouseButton;
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
	if (commandType == k0_CommandNone)
		commandType = getCommandTypeFromMouseInput(_secondaryMouseInput, mousePos, button);

	if (commandType != k0_CommandNone)
		_commandQueue.push(Command(mousePos, commandType));

	_isCommandQueueLocked = false;
}

CommandType EventManager::getCommandTypeFromMouseInput(MouseInput *input, Common::Point mousePos, MouseButton button) {
	if (!input)
		return k0_CommandNone;

	CommandType commandType = k0_CommandNone;
	while ((commandType = input->_commandTypeToIssue) != k0_CommandNone) {
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

	_isCommandQueueLocked = true;
	if (_commandQueue.empty()) { /* If the command queue is empty */
		_isCommandQueueLocked = false;
		processPendingClick();
		return;
	}

	Command cmd = _commandQueue.pop();
	CommandType cmdType = cmd._type;
	if ((cmdType >= k3_CommandMoveForward) && (cmdType <= k6_CommandMoveLeft) && (_vm->_disabledMovementTicks || (_vm->_projectileDisableMovementTicks && (_vm->_lastProjectileDisabledMovementDirection == (normalizeModulo4(_vm->_dungeonMan->_partyDir + cmdType - k3_CommandMoveForward)))))) { /* If movement is disabled */
		_isCommandQueueLocked = false;
		processPendingClick();
		return;
	}

	int16 commandX = cmd._pos.x;
	int16 commandY = cmd._pos.y;
	_isCommandQueueLocked = false;
	processPendingClick();
	if ((cmdType == k2_CommandTurnRight) || (cmdType == k1_CommandTurnLeft)) {
		commandTurnParty(cmdType);
		return;
	}

	if ((cmdType >= k3_CommandMoveForward) && (cmdType <= k6_CommandMoveLeft)) {
		commandMoveParty(cmdType);
		return;
	}

	if ((cmdType >= k12_CommandClickInChampion_0_StatusBox) && (cmdType <= k15_CommandClickInChampion_3_StatusBox)) {
		int16 championIdx = cmdType - k12_CommandClickInChampion_0_StatusBox;
		if ((championIdx < _vm->_championMan->_partyChampionCount) && !_vm->_championMan->_candidateChampionOrdinal)
			commandProcessTypes12to27_clickInChampionStatusBox(championIdx, commandX, commandY);

		return;
	}

	if ((cmdType >= k125_CommandClickOnChamptionIcon_Top_Left) && (cmdType <= k128_CommandClickOnChamptionIcon_Lower_Left)) {
		mouseProcessCommands125To128_clickOnChampionIcon(cmdType - k125_CommandClickOnChamptionIcon_Top_Left);

		return;
	}

	if ((cmdType >= k28_CommandClickOnSlotBoxInventoryReadyHand) && (cmdType < (k65_CommandClickOnSlotBoxChest_8 + 1))) {
		if (_vm->_championMan->_leaderIndex != kM1_ChampionNone)
			_vm->_championMan->clickOnSlotBox(cmdType - k20_CommandClickOnSlotBoxChampion_0_StatusBoxReadyHand);

		return;
	}

	if ((cmdType >= k7_CommandToggleInventoryChampion_0) && (cmdType <= k11_CommandCloseInventory)) {
		if (cmdType == k11_CommandCloseInventory) {
			delete _vm->_saveThumbnail;
			_vm->_saveThumbnail = nullptr;
		} else if (!_vm->_saveThumbnail) {
			_vm->_saveThumbnail = new Common::MemoryWriteStreamDynamic();
			Graphics::saveThumbnail(*_vm->_saveThumbnail);
		}

		int16 championIndex = cmdType - k7_CommandToggleInventoryChampion_0;
		if (((championIndex == k4_ChampionCloseInventory) || (championIndex < _vm->_championMan->_partyChampionCount)) && !_vm->_championMan->_candidateChampionOrdinal)
			_vm->_inventoryMan->f355_toggleInventory((ChampionIndex)championIndex);

		return;
	}

	if (cmdType == k83_CommandToggleInventoryLeader) {
		if (_vm->_championMan->_leaderIndex != kM1_ChampionNone)
			_vm->_inventoryMan->f355_toggleInventory(_vm->_championMan->_leaderIndex);

		return;
	}

	if (cmdType == k100_CommandClickInSpellArea) {
		if ((!_vm->_championMan->_candidateChampionOrdinal) && (_vm->_championMan->_magicCasterChampionIndex != kM1_ChampionNone))
			commandProcessType100_clickInSpellArea(commandX, commandY);

		return;
	}

	if (cmdType == k111_CommandClickInActionArea) {
		if (!_vm->_championMan->_candidateChampionOrdinal)
			commandProcessType111To115_ClickInActionArea(commandX, commandY);

		return;
	}

	if (cmdType == k70_CommandClickOnMouth) {
		_vm->_inventoryMan->f349_processCommand70_clickOnMouth();
		return;
	}

	if (cmdType == k71_CommandClickOnEye) {
		_vm->_inventoryMan->f352_processCommand71_clickOnEye();
		return;
	}

	if (cmdType == k80_CommandClickInDungeonView) {
		commandProcessType80ClickInDungeonView(commandX, commandY);
		return;
	}
	if (cmdType == k81_CommandClickInPanel) {
		commandProcess81ClickInPanel(commandX, commandY);
		return;
	}

	if (_vm->_pressingEye || _vm->_pressingMouth)
		return;

	if (cmdType == k145_CommandSleep) {
		if (!_vm->_championMan->_candidateChampionOrdinal) {
			if (_vm->_inventoryMan->_g432_inventoryChampionOrdinal)
				_vm->_inventoryMan->f355_toggleInventory(k4_ChampionCloseInventory);

			_vm->_menuMan->f456_drawDisabledMenu();
			_vm->_championMan->_partyIsSleeping = true;
			drawSleepScreen();
			_vm->_displayMan->drawViewport(k2_viewportAsBeforeSleepOrFreezeGame);
			_vm->_waitForInputMaxVerticalBlankCount = 0;
			_primaryMouseInput = _primaryMouseInputPartySleeping;
			_secondaryMouseInput = 0;
			_primaryKeyboardInput = _primaryKeyboardInputPartySleeping;
			_secondaryKeyboardInput = nullptr;
			discardAllInput();
		}
		return;
	}

	if (cmdType == k146_CommandWakeUp) {
		_vm->_championMan->wakeUp();
		return;
	}

	if (cmdType == k140_CommandSaveGame) {
		if ((_vm->_championMan->_partyChampionCount > 0) && !_vm->_championMan->_candidateChampionOrdinal)
			_vm->saveGame();

		return;
	}

	if (cmdType == k147_CommandFreezeGame) {
		_vm->_gameTimeTicking = false;
		_vm->_menuMan->f456_drawDisabledMenu();
		_vm->_displayMan->fillBitmap(_vm->_displayMan->_bitmapViewport, k0_ColorBlack, 112, 136);

		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY:
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, 81, 69, k4_ColorCyan, k0_ColorBlack,
												 "GAME FROZEN", k136_heightViewport);
			break;
		case Common::DE_DEU:
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, 66, 69, k4_ColorCyan, k0_ColorBlack,
												 "SPIEL ANGEHALTEN", k136_heightViewport);
			break;
		case Common::FR_FRA:
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, 84, 69, k4_ColorCyan, k0_ColorBlack,
												 "JEU BLOQUE", k136_heightViewport);
			break;
		}
		_vm->_displayMan->drawViewport(k2_viewportAsBeforeSleepOrFreezeGame);
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

	if (cmdType == k148_CommandUnfreezeGame) {
		_vm->_gameTimeTicking = true;
		_vm->_menuMan->f457_drawEnabledMenus();
		_primaryMouseInput = primaryMouseInputBackup;
		_secondaryMouseInput = secondaryMouseInputBackup;
		_primaryKeyboardInput = primaryKeyboardInputBackup;
		_secondaryKeyboardInput = secondaryKeyboardInputBackup;
		discardAllInput();
		return;
	}

	if (cmdType == k200_CommandEntranceEnterDungeon) {
		_vm->_newGameFl = k1_modeLoadDungeon;
		return;
	}

	if (cmdType == k201_CommandEntranceResume) {
		_vm->_newGameFl = k0_modeLoadSavedGame;
		return;
	}

	if (cmdType == k202_CommandEntranceDrawCredits) {
		_vm->entranceDrawCredits();
		return;
	}

	if ((cmdType >= k210_CommandClickOnDialogChoice_1) && (cmdType <= k213_CommandClickOnDialogChoice_4)) {
		_vm->_dialog->_selectedDialogChoice = cmdType - (k210_CommandClickOnDialogChoice_1 - 1);
		return;
	}

	if (cmdType == k215_CommandRestartGame)
		_vm->_restartGameRequest = true;
}

void EventManager::commandTurnParty(CommandType cmdType) {
	_vm->_stopWaitingForPlayerInput = true;
	if (cmdType == k1_CommandTurnLeft)
		commandHighlightBoxEnable(234, 261, 125, 145);
	else
		commandHighlightBoxEnable(291, 318, 125, 145);

	uint16 partySquare = _vm->_dungeonMan->getSquare(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY).toByte();
	if (Square(partySquare).getType() == k3_StairsElemType) {
		commandTakeStairs(getFlag(partySquare, k0x0004_StairsUp));
		return;
	}

	_vm->_moveSens->f276_sensorProcessThingAdditionOrRemoval(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, Thing::_party, true, false);
	_vm->_championMan->setPartyDirection(normalizeModulo4(_vm->_dungeonMan->_partyDir + ((cmdType == k2_CommandTurnRight) ? 1 : 3)));
	_vm->_moveSens->f276_sensorProcessThingAdditionOrRemoval(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, Thing::_party, true, true);
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
	for (uint16 idx = k0_ChampionFirst; idx < _vm->_championMan->_partyChampionCount; idx++) {
		_vm->_championMan->decrementStamina(idx, ((championsPtr->_load * 3) / _vm->_championMan->getMaximumLoad(championsPtr)) + 1); /* BUG0_50 When a champion is brought back to life at a Vi Altar, his current stamina is lower than what it was before dying. Each time the party moves the current stamina of all champions is decreased, including for dead champions, by an amount that depends on the current load of the champion. For a dead champion the load before he died is used */
		championsPtr++;
	}
	uint16 movementArrowIdx = cmdType - k3_CommandMoveForward;
	Box *highlightBox = &boxMovementArrows[movementArrowIdx];
	commandHighlightBoxEnable(highlightBox->_x1, highlightBox->_x2, highlightBox->_y1, highlightBox->_y2);
	int16 partyMapX = _vm->_dungeonMan->_partyMapX;
	int16 partyMapY = _vm->_dungeonMan->_partyMapY;
	uint16 AL1115_ui_Square = _vm->_dungeonMan->getSquare(partyMapX, partyMapY).toByte();
	bool isStairsSquare = (Square(AL1115_ui_Square).getType() == k3_StairsElemType);
	if (isStairsSquare && (movementArrowIdx == 2)) { /* If moving backward while in stairs */
		commandTakeStairs(getFlag(AL1115_ui_Square, k0x0004_StairsUp));
		return;
	}
	_vm->_dungeonMan->mapCoordsAfterRelMovement(_vm->_dungeonMan->_partyDir, movementArrowToStepForwardCount[movementArrowIdx], movementArrowToSepRightCount[movementArrowIdx], partyMapX, partyMapY);
	int16 partySquareType = Square(AL1115_ui_Square = _vm->_dungeonMan->getSquare(partyMapX, partyMapY).toByte()).getType();
	if (partySquareType == k3_ElementTypeStairs) {
		_vm->_moveSens->f267_getMoveResult(Thing::_party, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, kM1_MapXNotOnASquare, 0);
		_vm->_dungeonMan->_partyMapX = partyMapX;
		_vm->_dungeonMan->_partyMapY = partyMapY;
		commandTakeStairs(getFlag(AL1115_ui_Square, k0x0004_StairsUp));
		return;
	}

	bool isMovementBlocked = false;
	if (partySquareType == k0_ElementTypeWall)
		isMovementBlocked = true;
	else if (partySquareType == k4_DoorElemType) {
		byte doorState = Square(AL1115_ui_Square).getDoorState();
		isMovementBlocked = (doorState != k0_doorState_OPEN) && (doorState != k1_doorState_FOURTH) && (doorState != k5_doorState_DESTROYED);
	} else if (partySquareType == k6_ElementTypeFakeWall)
		isMovementBlocked = (!getFlag(AL1115_ui_Square, k0x0004_FakeWallOpen) && !getFlag(AL1115_ui_Square, k0x0001_FakeWallImaginary));

	if (_vm->_championMan->_partyChampionCount) {
		if (isMovementBlocked) {
			movementArrowIdx += (_vm->_dungeonMan->_partyDir + 2);
			int16 L1124_i_FirstDamagedChampionIndex = _vm->_championMan->getTargetChampionIndex(partyMapX, partyMapY, normalizeModulo4(movementArrowIdx));
			int16 L1125_i_SecondDamagedChampionIndex = _vm->_championMan->getTargetChampionIndex(partyMapX, partyMapY, returnNextVal(movementArrowIdx));
			int16 damage = _vm->_championMan->addPendingDamageAndWounds_getDamage(L1124_i_FirstDamagedChampionIndex, 1, k0x0008_ChampionWoundTorso | k0x0010_ChampionWoundLegs, k2_attackType_SELF);
			if (L1124_i_FirstDamagedChampionIndex != L1125_i_SecondDamagedChampionIndex)
				damage |= _vm->_championMan->addPendingDamageAndWounds_getDamage(L1125_i_SecondDamagedChampionIndex, 1, k0x0008_ChampionWoundTorso | k0x0010_ChampionWoundLegs, k2_attackType_SELF);

			if (damage)
				_vm->_sound->f064_SOUND_RequestPlay_CPSD(k18_soundPARTY_DAMAGED, partyMapX, partyMapY, k0_soundModePlayImmediately);
		} else {
			isMovementBlocked = (_vm->_groupMan->groupGetThing(partyMapX, partyMapY) != Thing::_endOfList);
			if (isMovementBlocked)
				_vm->_groupMan->processEvents29to41(partyMapX, partyMapY, kM1_TMEventTypeCreateReactionEvent31ParyIsAdjacent, 0);
		}
	}

	// DEBUG CODE: check for Console flag
	if (isMovementBlocked && !_vm->_console->_debugNoclip) {
		discardAllInput();
		_vm->_stopWaitingForPlayerInput = false;
		return;
	}

	if (isStairsSquare)
		_vm->_moveSens->f267_getMoveResult(Thing::_party, kM1_MapXNotOnASquare, 0, partyMapX, partyMapY);
	else
		_vm->_moveSens->f267_getMoveResult(Thing::_party, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, partyMapX, partyMapY);

	uint16 disabledMovtTicks = 1;
	championsPtr = _vm->_championMan->_champions;
	for (uint16 idx = k0_ChampionFirst; idx < _vm->_championMan->_partyChampionCount; idx++) {
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
		if (_vm->_dungeonMan->_squareAheadElement == k17_ElementTypeDoorFront) {
			if (posX < 64)
				return false;
		} else if (posX < 32)
			return false;

		// Strangerke: Only present in CSB2.1... But it fixes a bug so we keep it
		objectThrownFl = _vm->_championMan->isLeaderHandObjectThrown(k0_sideLeft);
	} else {
		if (_vm->_dungeonMan->_squareAheadElement == k17_ElementTypeDoorFront) {
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

void EventManager::setMousePointerFromSpriteData(byte* mouseSprite) {
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

	if ((cm._leaderIndex == champIndex) || ((champIndex != kM1_ChampionNone) && !cm._champions[champIndex]._currHealth))
		return;

	if (cm._leaderIndex != kM1_ChampionNone) {
		leaderIndex = cm._leaderIndex;
		cm._champions[leaderIndex].setAttributeFlag(k0x0200_ChampionAttributeLoad, true);
		cm._champions[leaderIndex].setAttributeFlag(k0x0080_ChampionAttributeNameTitle, true);
		cm._champions[leaderIndex]._load -= _vm->_dungeonMan->getObjectWeight(cm._leaderHandObject);
		cm._leaderIndex = kM1_ChampionNone;
		cm.drawChampionState(leaderIndex);
	}
	if (champIndex == kM1_ChampionNone) {
		cm._leaderIndex = kM1_ChampionNone;
		return;
	}
	cm._leaderIndex = champIndex;
	Champion *champion = &cm._champions[cm._leaderIndex];
	champion->_dir = _vm->_dungeonMan->_partyDir;
	cm._champions[champIndex]._load += _vm->_dungeonMan->getObjectWeight(cm._leaderHandObject);
	if (_vm->indexToOrdinal(champIndex) != cm._candidateChampionOrdinal) {
		champion->setAttributeFlag(k0x0400_ChampionAttributeIcon, true);
		champion->setAttributeFlag(k0x0080_ChampionAttributeNameTitle, true);
		cm.drawChampionState(champIndex);
	}
}

void EventManager::commandProcessType80ClickInDungeonViewTouchFrontWall() {
	uint16 mapX = _vm->_dungeonMan->_partyMapX + _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir];
	uint16 mapY = _vm->_dungeonMan->_partyMapY + _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];

	if ((mapX >= 0) && (mapX < _vm->_dungeonMan->_currMapWidth)
		&& (mapY >= 0) && (mapY < _vm->_dungeonMan->_currMapHeight))
		_vm->_stopWaitingForPlayerInput = _vm->_moveSens->f275_sensorIsTriggeredByClickOnWall(mapX, mapY, returnOppositeDir(_vm->_dungeonMan->_partyDir));
}

void EventManager::commandProcessType80ClickInDungeonView(int16 posX, int16 posY) {
	Box boxObjectPiles[4] = { // @ G0462_as_Graphic561_Box_ObjectPiles
		/* { X1, X2, Y1, Y2 } */
		Box(24, 111, 148, 168),   /* Front left */
		Box(112, 199, 148, 168),  /* Front right */
		Box(112, 183, 122, 147),  /* Back right */
		Box(40, 111, 122, 147)    /* Back left */
	};

	if (_vm->_dungeonMan->_squareAheadElement == k17_ElementTypeDoorFront) {
		if (_vm->_championMan->_leaderIndex == kM1_ChampionNone)
			return;

		int16 L1155_i_MapX = _vm->_dungeonMan->_partyMapX + _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir];
		int16 L1156_i_MapY = _vm->_dungeonMan->_partyMapY + _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];

		if (_vm->_championMan->_leaderEmptyHanded) {
			Junk *junkPtr = (Junk*)_vm->_dungeonMan->getSquareFirstThingData(L1155_i_MapX, L1156_i_MapY);
			if ((((Door*)junkPtr)->hasButton()) && _vm->_dungeonMan->_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn].isPointInside(posX, posY - 33)) {
				_vm->_stopWaitingForPlayerInput = true;
				_vm->_sound->f064_SOUND_RequestPlay_CPSD(k01_soundSWITCH, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k1_soundModePlayIfPrioritized);
				_vm->_moveSens->f268_addEvent(k10_TMEventTypeDoor, L1155_i_MapX, L1156_i_MapY, 0, k2_SensorEffToggle, _vm->_gameTime + 1);
				return;
			}
		} else if (isLeaderHandObjThrown(posX, posY))
			return;
	}

	if (_vm->_championMan->_leaderEmptyHanded) {
		for (uint16 currViewCell = k0_ViewCellFronLeft; currViewCell < k5_ViewCellDoorButtonOrWallOrn + 1; currViewCell++) {
			if (_vm->_dungeonMan->_dungeonViewClickableBoxes[currViewCell].isPointInside(posX, posY - 33)) {
				if (currViewCell == k5_ViewCellDoorButtonOrWallOrn) {
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
		if (_vm->_dungeonMan->_squareAheadElement == k0_ElementTypeWall) {
			for (uint16 currViewCell = k0_ViewCellFronLeft; currViewCell < k1_ViewCellFrontRight + 1; currViewCell++) {
				if (boxObjectPiles[currViewCell].isPointInside(posX, posY)) {
					processType80_clickInDungeonViewDropLeaderHandObject(currViewCell);
					return;
				}
			}
			if (_vm->_dungeonMan->_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn].isPointInside(posX, posY - 33)) {
				if (_vm->_dungeonMan->_isFacingAlcove)
					processType80_clickInDungeonViewDropLeaderHandObject(k4_ViewCellAlcove);
				else {
					if (_vm->_dungeonMan->_isFacingFountain) {
						uint16 iconIdx = _vm->_objectMan->f33_getIconIndex(thingHandObject);
						uint16 weight = _vm->_dungeonMan->getObjectWeight(thingHandObject);
						if ((iconIdx >= k8_IconIndiceJunkWater) && (iconIdx <= k9_IconIndiceJunkWaterSkin))
							junkPtr->setChargeCount(3); /* Full */
						else if (iconIdx == k195_IconIndicePotionEmptyFlask)
							((Potion*)junkPtr)->setType(k15_PotionTypeWaterFlask);
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

			for (uint16 currViewCell = k0_ViewCellFronLeft; currViewCell < k3_ViewCellBackLeft + 1; currViewCell++) {
				if (boxObjectPiles[currViewCell].isPointInside(posX, posY)) {
					processType80_clickInDungeonViewDropLeaderHandObject(currViewCell);
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

	uint16 championIndex = champMan._partyChampionCount - 1;
	Champion *champ = &champMan._champions[championIndex];
	if (commandType == k162_CommandClickInPanelCancel) {
		invMan.f355_toggleInventory(k4_ChampionCloseInventory);
		champMan._candidateChampionOrdinal = _vm->indexToOrdinal(kM1_ChampionNone);
		if (champMan._partyChampionCount == 1) {
			commandSetLeader(kM1_ChampionNone);
		}
		champMan._partyChampionCount--;
		Box box;
		box._y1 = 0;
		box._y2 = 28;
		box._x1 = championIndex * k69_ChampionStatusBoxSpacing;
		box._x2 = box._x1 + 66;
		dispMan._useByteBoxCoordinates = false;
		dispMan.fillScreenBox(box, k0_ColorBlack);
		dispMan.fillScreenBox(_vm->_championMan->_boxChampionIcons[champMan.getChampionIconIndex(champ->_cell, dunMan._partyDir) * 2], k0_ColorBlack);
		_vm->_menuMan->f457_drawEnabledMenus();
		showMouse();
		return;
	}

	champMan._candidateChampionOrdinal = _vm->indexToOrdinal(kM1_ChampionNone);
	int16 mapX = dunMan._partyMapX + _vm->_dirIntoStepCountEast[dunMan._partyDir];
	int16 mapY = dunMan._partyMapY + _vm->_dirIntoStepCountNorth[dunMan._partyDir];

	for (uint16 slotIndex = k0_ChampionSlotReadyHand; slotIndex < k30_ChampionSlotChest_1; slotIndex++) {
		Thing thing = champ->getSlot((ChampionSlot)slotIndex);
		if (thing != Thing::_none) {
			_vm->_dungeonMan->unlinkThingFromList(thing, Thing(0), mapX, mapY);
		}
	}
	Thing thing = dunMan.getSquareFirstThing(mapX, mapY);
	for (;;) { // infinite
		if (thing.getType() == k3_SensorThingType) {
			((Sensor*)dunMan.getThingData(thing))->setTypeDisabled();
			break;
		}
		thing = dunMan.getNextThing(thing);
	}

	if (commandType == k161_CommandClickInPanelReincarnate) {
		champMan.renameChampion(champ);
		if (_vm->_engineShouldQuit)
			return;
		champ->resetSkillsToZero();

		for (uint16 i = 0; i < 12; i++) {
			uint16 statIndex = _vm->getRandomNumber(7);
			champ->getStatistic((ChampionStatisticType)statIndex, k1_ChampionStatCurrent)++; // returns reference
			champ->getStatistic((ChampionStatisticType)statIndex, k0_ChampionStatMaximum)++; // returns reference
		}
	}

	if (champMan._partyChampionCount == 1) {
		_vm->_projexpl->_g362_lastPartyMovementTime = _vm->_gameTime;
		commandSetLeader(k0_ChampionFirst);
		_vm->_menuMan->f394_setMagicCasterAndDrawSpellArea(k0_ChampionFirst);
	} else
		_vm->_menuMan->f393_drawSpellAreaControls(champMan._magicCasterChampionIndex);

	_vm->_textMan->f51_messageAreaPrintLineFeed();
	Color champColor = _vm->_championMan->_championColor[championIndex];
	_vm->_textMan->f47_messageAreaPrintMessage(champColor, champ->_name);

	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		_vm->_textMan->f47_messageAreaPrintMessage(champColor, (commandType == k160_CommandClickInPanelResurrect) ? " RESURRECTED." : " REINCARNATED.");
		break;
	case Common::DE_DEU:
		_vm->_textMan->f47_messageAreaPrintMessage(champColor, (commandType == k160_CommandClickInPanelResurrect) ? " VOM TODE ERWECKT." : " REINKARNIERT.");
		break;
	case Common::FR_FRA:
		_vm->_textMan->f47_messageAreaPrintMessage(champColor, (commandType == k160_CommandClickInPanelResurrect) ? " RESSUSCITE." : " REINCARNE.");
		break;
	}

	invMan.f355_toggleInventory(k4_ChampionCloseInventory);
	_vm->_menuMan->f457_drawEnabledMenus();
	setMousePointerToNormal((_vm->_championMan->_leaderIndex == kM1_ChampionNone) ? k0_pointerArrow : k1_pointerHand);
}

void EventManager::commandProcess81ClickInPanel(int16 x, int16 y) {
	ChampionMan &champMan = *_vm->_championMan;
	InventoryMan &invMan = *_vm->_inventoryMan;

	CommandType commandType;
	switch (invMan._g424_panelContent) {
	case k4_PanelContentChest:
		if (champMan._leaderIndex == kM1_ChampionNone) // if no leader
			return;
		commandType = getCommandTypeFromMouseInput(_mouseInputPanelChest, Common::Point(x, y), k1_LeftMouseButton);
		if (commandType != k0_CommandNone)
			_vm->_championMan->clickOnSlotBox(commandType - k20_CommandClickOnSlotBoxChampion_0_StatusBoxReadyHand);
		break;
	case k5_PanelContentResurrectReincarnate:
		if (!champMan._leaderEmptyHanded)
			break;
		commandType = getCommandTypeFromMouseInput(_mouseInputPanelResurrectReincarnateCancel, Common::Point(x, y), k1_LeftMouseButton);
		if (commandType != k0_CommandNone)
			commandProcessCommands160To162ClickInResurrectReincarnatePanel(commandType);
		break;
	default:
		break;
	}
}

void EventManager::processType80_clickInDungeonView_grabLeaderHandObject(uint16 viewCell) {
	if (_vm->_championMan->_leaderIndex == kM1_ChampionNone)
		return;

	int16 mapX = _vm->_dungeonMan->_partyMapX;
	int16 mapY = _vm->_dungeonMan->_partyMapY;
	if (viewCell >= k2_ViewCellBackRight) {
		mapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir], mapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];
		Thing groupThing = _vm->_groupMan->groupGetThing(mapX, mapY);
		if ((groupThing != Thing::_endOfList) &&
			!_vm->_moveSens->f264_isLevitating(groupThing) &&
			_vm->_groupMan->getCreatureOrdinalInCell((Group*)_vm->_dungeonMan->getThingData(groupThing), normalizeModulo4(viewCell + _vm->_dungeonMan->_partyDir))) {
			return; /* It is not possible to grab an object on floor if there is a non levitating creature on its cell */
		}
	}

	Thing topPileThing = _vm->_dungeonMan->_pileTopObject[viewCell];
	if (_vm->_objectMan->f33_getIconIndex(topPileThing) != kM1_IconIndiceNone) {
		_vm->_moveSens->f267_getMoveResult(topPileThing, mapX, mapY, kM1_MapXNotOnASquare, 0);
		_vm->_championMan->putObjectInLeaderHand(topPileThing, true);
	}

	_vm->_stopWaitingForPlayerInput = true;
}

void EventManager::processType80_clickInDungeonViewDropLeaderHandObject(uint16 viewCell) {
	if (_vm->_championMan->_leaderIndex == kM1_ChampionNone)
		return;

	int16 mapX = _vm->_dungeonMan->_partyMapX;
	int16 mapY = _vm->_dungeonMan->_partyMapY;
	bool droppingIntoAnAlcove = (viewCell == k4_ViewCellAlcove);
	if (droppingIntoAnAlcove)
		viewCell = k2_ViewCellBackRight;

	if (viewCell > k1_ViewCellFrontRight)
		mapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir], mapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];

	uint16 currCell = normalizeModulo4(_vm->_dungeonMan->_partyDir + viewCell);
	Thing removedThing = _vm->_championMan->getObjectRemovedFromLeaderHand();
	_vm->_moveSens->f267_getMoveResult(thingWithNewCell(removedThing, currCell), kM1_MapXNotOnASquare, 0, mapX, mapY);
	if (droppingIntoAnAlcove && _vm->_dungeonMan->_isFacingViAltar && (_vm->_objectMan->f33_getIconIndex(removedThing) == k147_IconIndiceJunkChampionBones)) {
		Junk *removedJunk = (Junk*)_vm->_dungeonMan->getThingData(removedThing);
		TimelineEvent newEvent;
		setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + 1);
		newEvent._type = k13_TMEventTypeViAltarRebirth;
		newEvent._priority = removedJunk->getChargeCount();
		newEvent._B._location._mapX = mapX;
		newEvent._B._location._mapY = mapY;
		newEvent._C.A._cell = currCell;
		newEvent._C.A._effect = k2_SensorEffToggle;
		_vm->_timeline->f238_addEventGetEventIndex(&newEvent);
	}
	_vm->_stopWaitingForPlayerInput = true;
}

bool EventManager::hasPendingClick(Common::Point& point, MouseButton button) {
	if (_pendingClickButton && button == _pendingClickButton)
		point = _pendingClickPos;

	return _pendingClickPresent;
}

void EventManager::drawSleepScreen() {
	_vm->_displayMan->fillBitmap(_vm->_displayMan->_bitmapViewport, k0_ColorBlack, 112, 136);
	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, 93, 69, k4_ColorCyan, k0_ColorBlack, "WAKE UP", k136_heightViewport);
		break;
	case Common::DE_DEU:
		_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, 96, 69, k4_ColorCyan, k0_ColorBlack, "WECKEN", k136_heightViewport);
		break;
	case Common::FR_FRA:
		_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, 72, 69, k4_ColorCyan, k0_ColorBlack, "REVEILLEZ-VOUS", k136_heightViewport);
		break;
	}
}

void EventManager::discardAllInput() {
	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event) && !_vm->_engineShouldQuit) {
		if (event.type == Common::EVENT_QUIT)
			_vm->_engineShouldQuit = true;
	}
	_commandQueue.clear();
}

void EventManager::commandTakeStairs(bool stairsGoDown) {
	_vm->_moveSens->f267_getMoveResult(Thing::_party, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, kM1_MapXNotOnASquare, 0);
	_vm->_newPartyMapIndex = _vm->_dungeonMan->getLocationAfterLevelChange(_vm->_dungeonMan->_partyMapIndex, stairsGoDown ? -1 : 1, &_vm->_dungeonMan->_partyMapX, &_vm->_dungeonMan->_partyMapY);
	_vm->_dungeonMan->setCurrentMap(_vm->_newPartyMapIndex);
	_vm->_championMan->setPartyDirection(_vm->_dungeonMan->getStairsExitDirection(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY));
	_vm->_dungeonMan->setCurrentMap(_vm->_dungeonMan->_partyMapIndex);
}

void EventManager::commandProcessTypes12to27_clickInChampionStatusBox(uint16 champIndex, int16 posX, int16 posY) {
	if (_vm->indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
		commandSetLeader((ChampionIndex)champIndex);
	} else {
		uint16 commandType = getCommandTypeFromMouseInput(_mouseInputChampionNamesHands, Common::Point(posX, posY), k1_LeftMouseButton);
		if ((commandType >= k16_CommandSetLeaderChampion_0) && (commandType <= k19_CommandSetLeaderChampion_3))
			commandSetLeader((ChampionIndex)(commandType - k16_CommandSetLeaderChampion_0));
		else if ((commandType >= k20_CommandClickOnSlotBoxChampion_0_StatusBoxReadyHand) && (commandType <= k27_CommandClickOnSlotBoxChampion_3_StatusBoxActionHand))
			_vm->_championMan->clickOnSlotBox(commandType - k20_CommandClickOnSlotBoxChampion_0_StatusBoxReadyHand);
	}
}

void EventManager::mouseProcessCommands125To128_clickOnChampionIcon(uint16 champIconIndex) {
	static Box championIconShadowBox = Box(2, 20, 2, 15);
	static Box championIconBox = Box(0, 18, 0, 13);
	static byte mousePointerIconShadowBox[16] = {0, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 0, 120, 120, 120};

	_preventBuildPointerScreenArea = true;
	if (!_useChampionIconOrdinalAsMousePointerBitmap) {
		if (_vm->_championMan->getIndexInCell(normalizeModulo4(champIconIndex + _vm->_dungeonMan->_partyDir)) == kM1_ChampionNone) {
			_preventBuildPointerScreenArea = false;
			return;
		}
		_mousePointerBitmapUpdated = true;
		_useChampionIconOrdinalAsMousePointerBitmap = true;
		_vm->_displayMan->_useByteBoxCoordinates = false;
		byte *tmpBitmap = _mousePointerTempBuffer;
		memset(tmpBitmap, 0, 32 * 18);
		Box *curChampionIconBox = &_vm->_championMan->_boxChampionIcons[champIconIndex];

		_vm->_displayMan->blitToBitmap(_vm->_displayMan->_bitmapScreen, tmpBitmap, championIconShadowBox, curChampionIconBox->_x1, curChampionIconBox->_y1, k160_byteWidthScreen, k16_byteWidth, k0_ColorBlack, 200, 18);
		_vm->_displayMan->blitToBitmapShrinkWithPalChange(tmpBitmap, _mousePointerOriginalColorsChampionIcon, 32, 18, 32, 18, mousePointerIconShadowBox);
		_vm->_displayMan->blitToBitmap(_vm->_displayMan->_bitmapScreen, _mousePointerOriginalColorsChampionIcon, championIconBox, curChampionIconBox->_x1, curChampionIconBox->_y1, k160_byteWidthScreen, k16_byteWidth, k0_ColorBlack, 200, 18);
		_vm->_displayMan->fillScreenBox(*curChampionIconBox, k0_ColorBlack);
		_useChampionIconOrdinalAsMousePointerBitmap = _vm->indexToOrdinal(champIconIndex);
	} else {
		_mousePointerBitmapUpdated = true;
		uint16 championIconIndex = _vm->ordinalToIndex(_useChampionIconOrdinalAsMousePointerBitmap);
		_useChampionIconOrdinalAsMousePointerBitmap = _vm->indexToOrdinal(kM1_ChampionNone);
		int16 championCellIndex = _vm->_championMan->getIndexInCell(normalizeModulo4(championIconIndex + _vm->_dungeonMan->_partyDir));
		if (championIconIndex == champIconIndex) {
			setFlag(_vm->_championMan->_champions[championCellIndex]._attributes, k0x0400_ChampionAttributeIcon);
			_vm->_championMan->drawChampionState((ChampionIndex)championCellIndex);
		} else {
			int16 championIndex = _vm->_championMan->getIndexInCell(normalizeModulo4(champIconIndex + _vm->_dungeonMan->_partyDir));
			if (championIndex >= 0) {
				_vm->_championMan->_champions[championIndex]._cell = (ViewCell)normalizeModulo4(championIconIndex + _vm->_dungeonMan->_partyDir);
				setFlag(_vm->_championMan->_champions[championIndex]._attributes, k0x0400_ChampionAttributeIcon);
				_vm->_championMan->drawChampionState((ChampionIndex)championIndex);
			} else
				_vm->_displayMan->fillScreenBox(_vm->_championMan->_boxChampionIcons[championIconIndex], k0_ColorBlack);

			_vm->_championMan->_champions[championCellIndex]._cell = (ViewCell)normalizeModulo4(champIconIndex + _vm->_dungeonMan->_partyDir);
			setFlag(_vm->_championMan->_champions[championCellIndex]._attributes, k0x0400_ChampionAttributeIcon);
			_vm->_championMan->drawChampionState((ChampionIndex)championCellIndex);
		}
	}
	_preventBuildPointerScreenArea = false;
	buildpointerScreenArea(_mousePos.x, _mousePos.y);
}

void EventManager::commandProcessType100_clickInSpellArea(uint16 posX, uint16 posY) {
	int16 championIndex = kM1_ChampionNone;
	if (posY <= 48) {
		switch (_vm->_championMan->_magicCasterChampionIndex) {
		case 0:
			if ((posX >= 280) && (posX <= 291))
				championIndex = 1;
			else if ((posX >= 294) && (posX <= 305))
				championIndex = 2;
			else if (posX >= 308)
				championIndex = 3;

			break;
		case 1:
			if ((posX >= 233) && (posX <= 244))
				championIndex = 0;
			else if ((posX >= 294) && (posX <= 305))
				championIndex = 2;
			else if (posX >= 308)
				championIndex = 3;

			break;
		case 2:
			if ((posX >= 233) && (posX <= 244))
				championIndex = 0;
			else if ((posX >= 247) && (posX <= 258))
				championIndex = 1;
			else if (posX >= 308)
				championIndex = 3;

			break;
		case 3:
			if ((posX >= 247) && (posX <= 258))
				championIndex = 1;
			else if ((posX >= 261) && (posX <= 272))
				championIndex = 2;
			else if (posX <= 244)
				championIndex = 0;
			break;
		default:
			break;
		}

		if ((championIndex != kM1_ChampionNone) && (championIndex < _vm->_championMan->_partyChampionCount))
			_vm->_menuMan->f394_setMagicCasterAndDrawSpellArea(championIndex);

		return;
	}

	CommandType newCommand = getCommandTypeFromMouseInput(_mouseInputSpellArea, Common::Point(posX, posY), k1_LeftMouseButton);
	if (newCommand != k0_CommandNone)
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
		Box(305, 318, 63, 73)}; /* Delete */

	if (cmdType == k108_CommandClickInSpeallAreaCastSpell) {
		if (_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex]._symbols[0] == '\0')
			return;

		commandHighlightBoxEnable(234, 303, 63, 73);
		_vm->_stopWaitingForPlayerInput = _vm->_menuMan->f408_getClickOnSpellCastResult();
		return;
	}

	uint16 symbolIndex = cmdType - k101_CommandClickInSpellAreaSymbol_1;
	Box *highlightBox = &spellSymbolsAndDelete[symbolIndex];
	commandHighlightBoxEnable(highlightBox->_x1, highlightBox->_x2, highlightBox->_y1, highlightBox->_y2);
	_vm->delay(1);
	highlightBoxDisable();

	if (symbolIndex < 6)
		_vm->_menuMan->f399_addChampionSymbol(symbolIndex);
	else
		_vm->_menuMan->f400_deleteChampionSymbol();
}

void EventManager::commandProcessType111To115_ClickInActionArea(int16 posX, int16 posY) {
	if (_vm->_championMan->_actingChampionOrdinal) {
		uint16 mouseCommand = getCommandTypeFromMouseInput(_mouseInputActionAreaNames, Common::Point(posX, posY), k1_LeftMouseButton);
		if (mouseCommand != k0_CommandNone) {
			if (mouseCommand == k112_CommandClickInActionAreaPass) {
				commandHighlightBoxEnable(285, 319, 77, 83);
				_vm->_menuMan->f391_didClickTriggerAction(-1);
			} else if ((mouseCommand - k112_CommandClickInActionAreaPass) <= _vm->_menuMan->_g507_actionCount) {
				if (mouseCommand == k113_CommandClickInActionAreaAction_0)
					commandHighlightBoxEnable(234, 318, 86, 96);
				else if (mouseCommand == k114_CommandClickInActionAreaAction_1)
					commandHighlightBoxEnable(234, 318, 98, 108);
				else
					commandHighlightBoxEnable(234, 318, 110, 120);

				_vm->_stopWaitingForPlayerInput = _vm->_menuMan->f391_didClickTriggerAction(mouseCommand - k113_CommandClickInActionAreaAction_0);
			}
		}
	} else if (_vm->_menuMan->_g509_actionAreaContainsIcons) {
		uint16 mouseCommand = getCommandTypeFromMouseInput(_mouseInputActionAreaIcons, Common::Point(posX, posY), k1_LeftMouseButton);
		if (mouseCommand != k0_CommandNone) {
			mouseCommand -= k116_CommandClickInActionAreaChampion_0_Action;
			if (mouseCommand < _vm->_championMan->_partyChampionCount)
				_vm->_menuMan->f389_processCommands116To119_setActingChampion(mouseCommand);
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
	while (true) {
		if (_vm->_system->getEventManager()->pollEvent(event)) {
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
		_vm->_displayMan->updateScreen();
	}
}

void EventManager::commandHighlightBoxEnable(int16 x1, int16 x2, int16 y1, int16 y2) {
	_highlightScreenBox = Box(x1, x2, y1, y2);
	highlightScreenBox(x1, x2, y1, y2);
	_highlightBoxEnabled = true;
}

void EventManager::highlightBoxDisable() {
	if (_highlightBoxEnabled == true) {
		highlightScreenBox(_highlightScreenBox._x1, _highlightScreenBox._x2, _highlightScreenBox._y1, _highlightScreenBox._y2);
		_highlightBoxEnabled = false;
	}
}

} // end of namespace DM
