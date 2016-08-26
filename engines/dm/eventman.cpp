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

Box g462_BoxObjectPiles[4] = { // @ G0462_as_Graphic561_Box_ObjectPiles
	/* { X1, X2, Y1, Y2 } */
	Box(24, 111, 148, 168),   /* Front left */
	Box(112, 199, 148, 168),   /* Front right */
	Box(112, 183, 122, 147),   /* Back right */
	Box(40, 111, 122, 147)}; /* Back left */

MouseInput g445_PrimaryMouseInput_Entrance[4] = { // @ G0445_as_Graphic561_PrimaryMouseInput_Entrance[4]
												  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k200_CommandEntranceEnterDungeon, 244, 298,  45,  58, k1_LeftMouseButton),
	// Strangerke - C201_COMMAND_ENTRANCE_RESUME isn't present in the demo
	MouseInput(k201_CommandEntranceResume,        244, 298,  76,  93, k1_LeftMouseButton),
	MouseInput(k202_CommandEntranceDrawCredits,  248, 293, 187, 199, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g446_PrimaryMouseInput_RestartGame[2] = { // @ G0446_as_Graphic561_PrimaryMouseInput_RestartGame[2]
													 /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k215_CommandRestartGame, 103, 217, 145, 159, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g447_PrimaryMouseInput_Interface[20] = { // @ G0447_as_Graphic561_PrimaryMouseInput_Interface[20]
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
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g448_SecondaryMouseInput_Movement[9] = { // @ G0448_as_Graphic561_SecondaryMouseInput_Movement[9]
													/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k1_CommandTurnLeft,             234, 261, 125, 145, k1_LeftMouseButton),
	MouseInput(k3_CommandMoveForward,          263, 289, 125, 145, k1_LeftMouseButton),
	MouseInput(k2_CommandTurnRight,            291, 318, 125, 145, k1_LeftMouseButton),
	MouseInput(k6_CommandMoveLeft,             234, 261, 147, 167, k1_LeftMouseButton),
	MouseInput(k5_CommandMoveBackward,         263, 289, 147, 167, k1_LeftMouseButton),
	MouseInput(k4_CommandMoveRight,            291, 318, 147, 167, k1_LeftMouseButton),
	MouseInput(k80_CommandClickInDungeonView,   0, 223,  33, 168, k1_LeftMouseButton),
	MouseInput(k83_CommandToggleInventoryLeader, 0, 319,  33, 199, k2_RightMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g449_SecondaryMouseInput_ChampionInventory[38] = { // @ G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory[38]
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
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g450_PrimaryMouseInput_PartySleeping[3] = { // @ G0450_as_Graphic561_PrimaryMouseInput_PartySleeping[3]
													   /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k146_CommandWakeUp, 0, 223, 33, 168, k1_LeftMouseButton),
	MouseInput(k146_CommandWakeUp, 0, 223, 33, 168, k2_RightMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g451_PrimaryMouseInput_FrozenGame[3] = { // @ G0451_as_Graphic561_PrimaryMouseInput_FrozenGame[3]
													/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k148_CommandUnfreezeGame, 0, 319, 0, 199, k1_LeftMouseButton),
	MouseInput(k148_CommandUnfreezeGame, 0, 319, 0, 199, k2_RightMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g452_MouseInput_ActionAreaNames[5] = { // @ G0452_as_Graphic561_MouseInput_ActionAreaNames[5]
												  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k112_CommandClickInActionAreaPass,     285, 318,  77,  83, k1_LeftMouseButton),
	MouseInput(k113_CommandClickInActionAreaAction_0, 234, 318,  86,  96, k1_LeftMouseButton),
	MouseInput(k114_CommandClickInActionAreaAction_1, 234, 318,  98, 108, k1_LeftMouseButton),
	MouseInput(k115_CommandClickInActionAreaAction_2, 234, 318, 110, 120, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g453_MouseInput_ActionAreaIcons[5] = { // @ G0453_as_Graphic561_MouseInput_ActionAreaIcons[5]
												  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k116_CommandClickInActionAreaChampion_0_Action, 233, 252, 86, 120, k1_LeftMouseButton),
	MouseInput(k117_CommandClickInActionAreaChampion_1_Action, 255, 274, 86, 120, k1_LeftMouseButton),
	MouseInput(k118_CommandClickInActionAreaChampion_2_Action, 277, 296, 86, 120, k1_LeftMouseButton),
	MouseInput(k119_CommandClickInActionAreaChampion_3_Action, 299, 318, 86, 120, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g454_MouseInput_SpellArea[9] = { // @ G0454_as_Graphic561_MouseInput_SpellArea[9]
											/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k101_CommandClickInSpellAreaSymbol_1,      235, 247, 51, 61, k1_LeftMouseButton),
	MouseInput(k102_CommandClickInSpellAreaSymbol_2,      249, 261, 51, 61, k1_LeftMouseButton),
	MouseInput(k103_CommandClickInSpellAreaSymbol_3,      263, 275, 51, 61, k1_LeftMouseButton),
	MouseInput(k104_CommandClickInSpellAreaSymbol_4,      277, 289, 51, 61, k1_LeftMouseButton),
	MouseInput(k105_CommandClickInSpellAreaSymbol_5,      291, 303, 51, 61, k1_LeftMouseButton),
	MouseInput(k106_CommandClickInSpellAreaSymbol_6,      305, 317, 51, 61, k1_LeftMouseButton),
	MouseInput(k108_CommandClickInSpeallAreaCastSpell,    234, 303, 63, 73, k1_LeftMouseButton),
	MouseInput(k107_CommandClickInSpellAreaRecantSymbol, 305, 318, 63, 73, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g455_MouseInput_ChampionNamesHands[13] = { // @ G0455_as_Graphic561_MouseInput_ChampionNamesHands[13]
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
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g456_MouseInput_PanelChest[9] = { // @ G0456_as_Graphic561_MouseInput_PanelChest[9]
											 /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k58_CommandClickOnSlotBoxChest_1, 117, 132,  92, 107, k1_LeftMouseButton),
	MouseInput(k59_CommandClickOnSlotBoxChest_2, 106, 121, 109, 124, k1_LeftMouseButton),
	MouseInput(k60_CommandClickOnSlotBoxChest_3, 111, 126, 126, 141, k1_LeftMouseButton),
	MouseInput(k61_CommandClickOnSlotBoxChest_4, 128, 143, 131, 146, k1_LeftMouseButton),
	MouseInput(k62_CommandClickOnSlotBoxChest_5, 145, 160, 134, 149, k1_LeftMouseButton),
	MouseInput(k63_CommandClickOnSlotBoxChest_6, 162, 177, 136, 151, k1_LeftMouseButton),
	MouseInput(k64_CommandClickOnSlotBoxChest_7, 179, 194, 137, 152, k1_LeftMouseButton),
	MouseInput(k65_CommandClickOnSlotBoxChest_8, 196, 211, 138, 153, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g457_MouseInput_PanelResurrectReincarnateCancel[4] = { // @ G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel[4]
																  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k160_CommandClickInPanelResurrect,   108, 158,  90, 138, k1_LeftMouseButton), /* Atari ST: Box = 104, 158,  86, 142 */
	MouseInput(k161_CommandClickInPanelReincarnate, 161, 211,  90, 138, k1_LeftMouseButton), /* Atari ST: Box = 163, 217,  86, 142 */
	MouseInput(k162_CommandClickInPanelCancel,      108, 211, 141, 153, k1_LeftMouseButton), /* Atari ST: Box = 104, 217, 146, 156 */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};


MouseInput g471_PrimaryMouseInput_ViewportDialog1Choice[2] = { // @ G0471_as_Graphic561_PrimaryMouseInput_ViewportDialog1Choice[2]
	MouseInput(k210_CommandClickOnDialogChoice_1, 16, 207, 138, 152, k1_LeftMouseButton), /* Bottom button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g472_PrimaryMouseInput_ViewportDialog2Choices[3] = { // @ G0472_as_Graphic561_PrimaryMouseInput_ViewportDialog2Choices[3]
	MouseInput(k210_CommandClickOnDialogChoice_1, 16, 207, 101, 115, k1_LeftMouseButton), /* Top button */
	MouseInput(k211_CommandClickOnDialogChoice_2, 16, 207, 138, 152, k1_LeftMouseButton), /* Bottom button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g473_PrimaryMouseInput_ViewportDialog3Choices[4] = { // @ G0473_as_Graphic561_PrimaryMouseInput_ViewportDialog3Choices[4]
	MouseInput(k210_CommandClickOnDialogChoice_1,  16, 207, 101, 115, k1_LeftMouseButton), /* Top button */
	MouseInput(k211_CommandClickOnDialogChoice_2,  16, 101, 138, 152, k1_LeftMouseButton), /* Lower left button */
	MouseInput(k212_CommandClickOnDialogChoice_3, 123, 207, 138, 152, k1_LeftMouseButton), /* Lower right button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g474_PrimaryMouseInput_ViewportDialog4Choices[5] = { // @ G0474_as_Graphic561_PrimaryMouseInput_ViewportDialog4Choices[5]
	MouseInput(k210_CommandClickOnDialogChoice_1,  16, 101, 101, 115, k1_LeftMouseButton), /* Top left button */
	MouseInput(k211_CommandClickOnDialogChoice_2, 123, 207, 101, 115, k1_LeftMouseButton), /* Top right button */
	MouseInput(k212_CommandClickOnDialogChoice_3,  16, 101, 138, 152, k1_LeftMouseButton), /* Lower left button */
	MouseInput(k213_CommandClickOnDialogChoice_4, 123, 207, 138, 152, k1_LeftMouseButton), /* Lower right button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g475_PrimaryMouseInput_ScreenDialog1Choice[2] = { // @ G0475_as_Graphic561_PrimaryMouseInput_ScreenDialog1Choice[2]
	MouseInput(k210_CommandClickOnDialogChoice_1, 63, 254, 138, 152, k1_LeftMouseButton), /* Bottom button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g476_PrimaryMouseInput_ScreenDialog2Choices[3] = { // @ G0476_as_Graphic561_PrimaryMouseInput_ScreenDialog2Choices[3]
	MouseInput(k210_CommandClickOnDialogChoice_1, 63, 254, 101, 115, k1_LeftMouseButton), /* Top button */
	MouseInput(k211_CommandClickOnDialogChoice_2, 63, 254, 138, 152, k1_LeftMouseButton), /* Bottom button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g477_PrimaryMouseInput_ScreenDialog3Choices[4] = { // @ G0477_as_Graphic561_PrimaryMouseInput_ScreenDialog3Choices[4]
	MouseInput(k210_CommandClickOnDialogChoice_1,  63, 254, 101, 115, k1_LeftMouseButton), /* Top button */
	MouseInput(k211_CommandClickOnDialogChoice_2,  63, 148, 138, 152, k1_LeftMouseButton), /* Lower left button */
	MouseInput(k212_CommandClickOnDialogChoice_3, 170, 254, 138, 152, k1_LeftMouseButton), /* Lower right button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g478_PrimaryMouseInput_ScreenDialog4Choices[5] = { // @ G0478_as_Graphic561_PrimaryMouseInput_ScreenDialog4Choices[5]
	MouseInput(k210_CommandClickOnDialogChoice_1,  63, 148, 101, 115, k1_LeftMouseButton), /* Top left button */
	MouseInput(k211_CommandClickOnDialogChoice_2, 170, 254, 101, 115, k1_LeftMouseButton), /* Top right button */
	MouseInput(k212_CommandClickOnDialogChoice_3,  63, 148, 138, 152, k1_LeftMouseButton), /* Lower left button */
	MouseInput(k213_CommandClickOnDialogChoice_4, 170, 254, 138, 152, k1_LeftMouseButton), /* Lower right button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};

MouseInput* g480_PrimaryMouseInput_DialogSets[2][4] = { // @ G0480_aaps_PrimaryMouseInput_DialogSets
	{g471_PrimaryMouseInput_ViewportDialog1Choice,
	g472_PrimaryMouseInput_ViewportDialog2Choices,
	g473_PrimaryMouseInput_ViewportDialog3Choices,
	g474_PrimaryMouseInput_ViewportDialog4Choices},
	{g475_PrimaryMouseInput_ScreenDialog1Choice,
	g476_PrimaryMouseInput_ScreenDialog2Choices,
	g477_PrimaryMouseInput_ScreenDialog3Choices,
	g478_PrimaryMouseInput_ScreenDialog4Choices},};


EventManager::EventManager(DMEngine *vm) : _vm(vm) {
	_g441_primaryMouseInput = nullptr;
	_g442_secondaryMouseInput = nullptr;

	_g436_pendingClickPresent = false;
	_g435_isCommandQueueLocked = true;
	_g598_mousePointerBitmapUpdated = false;
	_g326_refreshMousePointerInMainLoop = false;
	_g341_highlightBoxEnabled = false;

	_dummyMapIndex = 0;
	_g439_pendingClickButton = k0_NoneMouseButton;
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

	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		// DUMMY CODE: case EVENT_KEYDOWN, only for testing
		case Common::EVENT_KEYDOWN:
			if (event.synthetic)
				break;

			switch (event.kbd.keycode) {
			case Common::KEYCODE_w:
				dungeonMan.f150_mapCoordsAfterRelMovement(dungeonMan._g308_partyDir, 1, 0, dungeonMan._g306_partyMapX, dungeonMan._g307_partyMapY);
				break;
			case Common::KEYCODE_a:
				dungeonMan.f150_mapCoordsAfterRelMovement(dungeonMan._g308_partyDir, 0, -1, dungeonMan._g306_partyMapX, dungeonMan._g307_partyMapY);
				break;
			case Common::KEYCODE_s:
				dungeonMan.f150_mapCoordsAfterRelMovement(dungeonMan._g308_partyDir, -1, 0, dungeonMan._g306_partyMapX, dungeonMan._g307_partyMapY);
				break;
			case Common::KEYCODE_d:
				dungeonMan.f150_mapCoordsAfterRelMovement(dungeonMan._g308_partyDir, 0, 1, dungeonMan._g306_partyMapX, dungeonMan._g307_partyMapY);
				break;
			case Common::KEYCODE_q:
				turnDirLeft(dungeonMan._g308_partyDir);
				break;
			case Common::KEYCODE_e:
				turnDirRight(dungeonMan._g308_partyDir);
				break;
			case Common::KEYCODE_UP:
				if (_dummyMapIndex < 13)
					dungeonMan.f174_setCurrentMapAndPartyMap(++_dummyMapIndex);
				break;
			case Common::KEYCODE_DOWN:
				if (_dummyMapIndex > 0)
					dungeonMan.f174_setCurrentMapAndPartyMap(--_dummyMapIndex);
				break;
			default:
				break;
			}
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			_g436_pendingClickPresent = true;
			_g437_pendingClickPos = _mousePos;
			_g439_pendingClickButton = (event.type == Common::EVENT_LBUTTONDOWN) ? k1_LeftMouseButton : k2_RightMouseButton;
			break;
		default:
			break;
		}
	}
}

void EventManager::f360_processPendingClick() {
	if (_g436_pendingClickPresent) {
		_g436_pendingClickPresent = false;
		f359_processClick(_g437_pendingClickPos, _g439_pendingClickButton);
	}
}

void EventManager::f359_processClick(Common::Point mousePos, MouseButton button) {
	CommandType commandType;

	commandType = f358_getCommandTypeFromMouseInput(_g441_primaryMouseInput, mousePos, button);
	if (commandType == k0_CommandNone)
		commandType = f358_getCommandTypeFromMouseInput(_g442_secondaryMouseInput, mousePos, button);

	if (commandType != k0_CommandNone)
		_commandQueue.push(Command(mousePos, commandType));

	_g435_isCommandQueueLocked = false;
}

CommandType EventManager::f358_getCommandTypeFromMouseInput(MouseInput *input, Common::Point mousePos, MouseButton button) {
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


void EventManager::f380_processCommandQueue() {
	_g435_isCommandQueueLocked = true;
	if (_commandQueue.empty()) {
		_g435_isCommandQueueLocked = false;
		f360_processPendingClick();
		return;
	}

	Command cmd = _commandQueue.pop();

	int16 commandX = cmd._pos.x;
	int16 commandY = cmd._pos.y;

	_g435_isCommandQueueLocked = false;
	f360_processPendingClick();

	if ((cmd._type == k2_CommandTurnRight) || (cmd._type == k1_CommandTurnLeft)) {
		f365_commandTurnParty(cmd._type);
		return;
	}

	if ((cmd._type >= k3_CommandMoveForward) && (cmd._type <= k6_CommandMoveLeft)) {
		f366_commandMoveParty(cmd._type);
		return;
	}

	if (cmd._type == k80_CommandClickInDungeonView) {
		f377_commandProcessType80ClickInDungeonView(commandX, commandY);
	}
	if (cmd._type == k81_CommandClickInPanel) {
		f378_commandProcess81ClickInPanel(commandX, commandY);
	}

	// MISSING CODE: the rest of the function
}

void EventManager::f365_commandTurnParty(CommandType cmdType) {
	_vm->_g321_stopWaitingForPlayerInput = true;

	// MISSING CODE: highlight turn left/right buttons

	// MISSING CODE: processing stairs

	// MISSING CODE: process sensors

	// DUMMY CODE: should call F0284_CHAMPION_SetPartyDirection instead
	direction &partyDir = _vm->_dungeonMan->_g308_partyDir;
	(cmdType == k1_CommandTurnLeft) ? turnDirLeft(partyDir) : turnDirRight(partyDir);

	// MISSING CODE: process sensors
}

void EventManager::f366_commandMoveParty(CommandType cmdType) {
	_vm->_g321_stopWaitingForPlayerInput = true;

	// MISSING CODE: Lots of code

	// DUMMY CODE:
	DungeonMan &dungeonMan = *_vm->_dungeonMan;

	switch (cmdType) {
	case k3_CommandMoveForward:
		dungeonMan.f150_mapCoordsAfterRelMovement(dungeonMan._g308_partyDir, 1, 0, dungeonMan._g306_partyMapX, dungeonMan._g307_partyMapY);
		break;
	case k6_CommandMoveLeft:
		dungeonMan.f150_mapCoordsAfterRelMovement(dungeonMan._g308_partyDir, 0, -1, dungeonMan._g306_partyMapX, dungeonMan._g307_partyMapY);
		break;
	case k5_CommandMoveBackward:
		dungeonMan.f150_mapCoordsAfterRelMovement(dungeonMan._g308_partyDir, -1, 0, dungeonMan._g306_partyMapX, dungeonMan._g307_partyMapY);
		break;
	case k4_CommandMoveRight:
		dungeonMan.f150_mapCoordsAfterRelMovement(dungeonMan._g308_partyDir, 0, 1, dungeonMan._g306_partyMapX, dungeonMan._g307_partyMapY);
		break;
	default:
		break;
	}

	// MISSING CODE: Lots of code
}

void EventManager::f368_commandSetLeader(ChampionIndex champIndex) {
	ChampionMan &cm = *_vm->_championMan;
	ChampionIndex leaderIndex;

	if ((cm._g411_leaderIndex == champIndex) || ((champIndex != kM1_ChampionNone) && !cm._gK71_champions[champIndex]._currHealth))
		return;

	if (cm._g411_leaderIndex != kM1_ChampionNone) {
		leaderIndex = cm._g411_leaderIndex;
		cm._gK71_champions[leaderIndex].setAttributeFlag(k0x0200_ChampionAttributeLoad, true);
		cm._gK71_champions[leaderIndex].setAttributeFlag(k0x0080_ChampionAttributeNameTitle, true);
		cm._gK71_champions[leaderIndex]._load -= _vm->_dungeonMan->f140_getObjectWeight(cm._g414_leaderHandObject);
		cm._g411_leaderIndex = kM1_ChampionNone;
		cm.f292_drawChampionState(leaderIndex);
	}
	if (champIndex == kM1_ChampionNone) {
		cm._g411_leaderIndex = kM1_ChampionNone;
		return;
	}
	cm._g411_leaderIndex = champIndex;
	Champion *champion = &cm._gK71_champions[cm._g411_leaderIndex];
	champion->_dir = _vm->_dungeonMan->_g308_partyDir;
	cm._gK71_champions[champIndex]._load += _vm->_dungeonMan->f140_getObjectWeight(cm._g414_leaderHandObject);
	if (_vm->M0_indexToOrdinal(champIndex) != cm._g299_candidateChampionOrdinal) {
		champion->setAttributeFlag(k0x0400_ChampionAttributeIcon, true);
		champion->setAttributeFlag(k0x0080_ChampionAttributeNameTitle, true);
		cm.f292_drawChampionState(champIndex);
	}
}

void EventManager::f372_commandProcessType80ClickInDungeonViewTouchFrontWall() {
	DungeonMan &dunMan = *_vm->_dungeonMan;
	int16 mapX = dunMan._g306_partyMapX + _vm->_dirIntoStepCountEast[dunMan._g308_partyDir];
	int16 mapY = dunMan._g307_partyMapY + _vm->_dirIntoStepCountNorth[dunMan._g308_partyDir];
	if ((mapX >= 0) && (mapX < dunMan._g273_currMapWidth) && (mapY >= 0) && (mapY < dunMan._g274_currMapHeight)) {
		_vm->_g321_stopWaitingForPlayerInput = _vm->_movsens->f275_sensorIsTriggeredByClickOnWall(mapX, mapY, returnOppositeDir(dunMan._g308_partyDir));
	}
}

void EventManager::f377_commandProcessType80ClickInDungeonView(int16 posX, int16 posY) {
	DungeonMan &dunMan = *_vm->_dungeonMan;
	ChampionMan &champMan = *_vm->_championMan;

	if (dunMan._g285_squareAheadElement == k17_ElementTypeDoorFront) {
		if (champMan._g411_leaderIndex == kM1_ChampionNone)
			return;

		if (champMan._g415_leaderEmptyHanded) {
			int16 mapX = dunMan._g306_partyMapX + _vm->_dirIntoStepCountEast[dunMan._g308_partyDir];
			int16 mapY = dunMan._g307_partyMapY + _vm->_dirIntoStepCountNorth[dunMan._g308_partyDir];

			if (Door(dunMan.f157_getSquareFirstThingData(mapX, mapY)).hasButton() &&
				dunMan._g291_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn].isPointInside(Common::Point(posX, posY - 33))) {
				_vm->_g321_stopWaitingForPlayerInput = true;
				warning("MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
				warning("MISSING CODE: F0268_SENSOR_AddEvent");
				return;
			}

			warning("MISSING CODE: F0375_COMMAND_ProcessType80_ClickInDungeonView_IsLeaderHandObjectThrown in elseif condition");
		}
	}

	if (champMan._g415_leaderEmptyHanded) {
		for (int16 viewCell = k0_ViewCellFronLeft; viewCell <= k5_ViewCellDoorButtonOrWallOrn; viewCell++) {
			if (dunMan._g291_dungeonViewClickableBoxes[viewCell].isPointInside(Common::Point(posX, posY - 33))) {
				if (viewCell == k5_ViewCellDoorButtonOrWallOrn) {
					if (!dunMan._g286_isFacingAlcove) {
						f372_commandProcessType80ClickInDungeonViewTouchFrontWall();
					}
				} else {
					warning("MISSING CODE: F0373_COMMAND_ProcessType80_ClickInDungeonView_GrabLeaderHandObject");
				}
				return;
			}
		}
	} else {
		Thing thing = champMan._g414_leaderHandObject;
		uint16 *rawThingPointer = dunMan.f156_getThingData(thing);
		if (dunMan._g285_squareAheadElement == k0_ElementTypeWall) {
			for (int16 viewCell = k0_ViewCellFronLeft; viewCell <= k1_ViewCellFrontRight; ++viewCell) {
				if (g462_BoxObjectPiles[viewCell].isPointInside(Common::Point(posX, posY))) {
					warning("F0374_COMMAND_ProcessType80_ClickInDungeonView_DropLeaderHandObject");
					return;
				}
			}

			if (dunMan._g291_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn].isPointInside(Common::Point(posX, posY - 33))) {
				if (dunMan._g286_isFacingAlcove) {
					warning("MISSING CODE: F0374_COMMAND_ProcessType80_ClickInDungeonView_DropLeaderHandObject");
				} else {
					if (dunMan._g288_isFacingFountain) {
						uint16 iconIndex = _vm->_objectMan->f33_getIconIndex(thing);
						int16 weight = dunMan.f140_getObjectWeight(thing);
						if ((iconIndex >= k8_IconIndiceJunkWater) && (iconIndex <= k9_IconIndiceJunkWaterSkin)) {
							((Junk*)rawThingPointer)->setChargeCount(3);
						} else if (iconIndex == k195_IconIndicePotionEmptyFlask) {
							((Potion*)rawThingPointer)->setType(k15_PotionTypeWaterFlask);
						} else {
							goto T0377019;
						}
						champMan.f296_drawChangedObjectIcons();
						champMan._gK71_champions[champMan._g411_leaderIndex]._load += dunMan.f140_getObjectWeight(thing) - weight;
					}
T0377019:
					f372_commandProcessType80ClickInDungeonViewTouchFrontWall();
				}
			}
		} else {
			warning("MISSING CODE: F0375_COMMAND_ProcessType80_ClickInDungeonView_IsLeaderHandObjectThrown in if branch");
			for (int16 viewCell = k0_ViewCellFronLeft; viewCell <= k3_ViewCellBackLeft; viewCell++) {
				if (g462_BoxObjectPiles[viewCell].isPointInside(Common::Point(posX, posY))) {
					warning("MISSING CODE: F0374_COMMAND_ProcessType80_ClickInDungeonView_DropLeaderHandObject");
					return;
				}
			}
		}
	}
}

void EventManager::f282_commandProcessCommands160To162ClickInResurrectReincarnatePanel(CommandType commandType) {
	ChampionMan &champMan = *_vm->_championMan;
	InventoryMan &invMan = *_vm->_inventoryMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	DungeonMan &dunMan = *_vm->_dungeonMan;

	uint16 championIndex = champMan._g305_partyChampionCount - 1;
	Champion *champ = &champMan._gK71_champions[championIndex];
	if (commandType == k162_CommandClickInPanelCancel) {
		invMan.f355_toggleInventory(k4_ChampionCloseInventory);
		champMan._g299_candidateChampionOrdinal = _vm->M0_indexToOrdinal(kM1_ChampionNone);
		if (champMan._g305_partyChampionCount == 1) {
			f368_commandSetLeader(kM1_ChampionNone);
		}
		champMan._g305_partyChampionCount--;
		Box box;
		box._y1 = 0;
		box._y2 = 28 + 1;
		box._x1 = championIndex * k69_ChampionStatusBoxSpacing;
		box._x2 = box._x1 + 66 + 1;
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.D24_clearScreenBox(k0_ColorBlack, box);
		dispMan.D24_clearScreenBox(k0_ColorBlack, g54_BoxChampionIcons[champMan.M26_championIconIndex(champ->_cell, dunMan._g308_partyDir) * 2]);
		warning("F0457_START_DrawEnabledMenus_CPSF");
		warning("F0078_MOUSE_ShowPointer");
		return;
	}

	champMan._g299_candidateChampionOrdinal = _vm->M0_indexToOrdinal(kM1_ChampionNone);
	int16 mapX = dunMan._g306_partyMapX + _vm->_dirIntoStepCountEast[dunMan._g308_partyDir];
	int16 mapY = dunMan._g307_partyMapY + _vm->_dirIntoStepCountNorth[dunMan._g308_partyDir];

	for (uint16 slotIndex = k0_ChampionSlotReadyHand; slotIndex < k30_ChampionSlotChest_1; slotIndex++) {
		Thing thing = champ->getSlot((ChampionSlot)slotIndex);
		if (thing != Thing::_none) {
			warning("MISSING CODE: F0164_DUNGEON_UnlinkThingFromList");
		}
	}
	Thing thing = dunMan.f161_getSquareFirstThing(mapX, mapY);
	for (;;) { // infinite
		if (thing.getType() == k3_SensorThingType) {
			((Sensor*)dunMan.f156_getThingData(thing))->setTypeDisabled();
			break;
		}
		thing = dunMan.f159_getNextThing(thing);
	}

	if (commandType == k161_CommandClickInPanelReincarnate) {
		champMan.f281_renameChampion(champ);
		champ->resetSkillsToZero();

		for (uint16 i = 0; i < 12; i++) {
			uint16 statIndex = _vm->_rnd->getRandomNumber(7);
			champ->getStatistic((ChampionStatisticType)statIndex, k1_ChampionStatCurrent)++; // returns reference
			champ->getStatistic((ChampionStatisticType)statIndex, k0_ChampionStatMaximum)++; // returns reference
		}
	}

	if (champMan._g305_partyChampionCount == 1) {
		warning("MISSING CODE: setting time, G0362_l_LastPartyMovementTime , G0313_ul_GameTime");
		f368_commandSetLeader(k0_ChampionFirst);
		_vm->_menuMan->f394_setMagicCasterAndDrawSpellArea(k0_ChampionFirst);
	} else {
		_vm->_menuMan->f393_drawSpellAreaControls(champMan._g514_magicCasterChampionIndex);
	}

	warning("MISSING CODE: F0051_TEXT_MESSAGEAREA_PrintLineFeed");
	Color champColor = g46_ChampionColor[championIndex]; // unreferenced because of missing code
	warning("MISSING CODE: F0047_TEXT_MESSAGEAREA_PrintMessage");
	warning("MISSING CODE: F0047_TEXT_MESSAGEAREA_PrintMessage");

	invMan.f355_toggleInventory(k4_ChampionCloseInventory);
	warning("MISSING CODE: F0457_START_DrawEnabledMenus_CPSF");
	warning("MISSING CODE: F0067_MOUSE_SetPointerToNormal");
}

void EventManager::f378_commandProcess81ClickInPanel(int16 x, int16 y) {
	ChampionMan &champMan = *_vm->_championMan;
	InventoryMan &invMan = *_vm->_inventoryMan;

	CommandType commandType;
	switch (invMan._g424_panelContent) {
	case k4_PanelContentChest:
		if (champMan._g411_leaderIndex == kM1_ChampionNone) // if no leader
			return;
		commandType = f358_getCommandTypeFromMouseInput(g456_MouseInput_PanelChest, Common::Point(x, y), k1_LeftMouseButton);
		if (commandType != k0_CommandNone)
			warning("MISSING CODE: F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox");
		break;
	case k5_PanelContentResurrectReincarnate:
		if (!champMan._g415_leaderEmptyHanded)
			break;
		commandType = f358_getCommandTypeFromMouseInput(g457_MouseInput_PanelResurrectReincarnateCancel, Common::Point(x, y), k1_LeftMouseButton);
		if (commandType != k0_CommandNone)
			f282_commandProcessCommands160To162ClickInResurrectReincarnatePanel(commandType);
		break;
	default:
		break;
	}
}

bool EventManager::f360_hasPendingClick(Common::Point& point, MouseButton button) {
	if (_g439_pendingClickButton && button == _g439_pendingClickButton)
		point = _g437_pendingClickPos;

	return _g436_pendingClickPresent;
}

} // end of namespace DM
