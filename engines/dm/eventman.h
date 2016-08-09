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

#ifndef DM_EVENTMAN_H
#define DM_EVENTMAN_H

#include "common/events.h"
#include "common/queue.h"
#include "common/array.h"

#include "dm.h"
#include "gfx.h"
#include "champion.h"


namespace DM {



enum MouseButton {
	k0_NoneMouseButton = 0, // present only because of typesafety
	k1_LeftMouseButton = 1,
	k2_RightMouseButton = 2
};

enum CommandType {
	k0_CommandNone = 0, // @ C000_COMMAND_NONE
	k1_CommandTurnLeft = 1, // @ C001_COMMAND_TURN_LEFT
	k2_CommandTurnRight = 2, // @ C002_COMMAND_TURN_RIGHT
	k3_CommandMoveForward = 3, // @ C003_COMMAND_MOVE_FORWARD
	k4_CommandMoveRight = 4, // @ C004_COMMAND_MOVE_RIGHT
	k5_CommandMoveBackward = 5, // @ C005_COMMAND_MOVE_BACKWARD
	k6_CommandMoveLeft = 6, // @ C006_COMMAND_MOVE_LEFT
	k7_CommandToggleInventoryChampion_0 = 7, // @ C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0
	k8_CommandToggleInventoryChampion_1 = 8, // @ C008_COMMAND_TOGGLE_INVENTORY_CHAMPION_1
	k9_CommandToggleInventoryChampion_2 = 9, // @ C009_COMMAND_TOGGLE_INVENTORY_CHAMPION_2
	k10_CommandToggleInventoryChampion_3 = 10, // @ C010_COMMAND_TOGGLE_INVENTORY_CHAMPION_3
	k11_CommandCloseInventory = 11, // @ C011_COMMAND_CLOSE_INVENTORY
	k12_CommandClickInChampion_0_StatusBox = 12, // @ C012_COMMAND_CLICK_IN_CHAMPION_0_STATUS_BOX
	k13_CommandClickInChampion_1_StatusBox = 13, // @ C013_COMMAND_CLICK_IN_CHAMPION_1_STATUS_BOX
	k14_CommandClickInChampion_2_StatusBox = 14, // @ C014_COMMAND_CLICK_IN_CHAMPION_2_STATUS_BOX
	k15_CommandClickInChampion_3_StatusBox = 15, // @ C015_COMMAND_CLICK_IN_CHAMPION_3_STATUS_BOX
	k16_CommandSetLeaderChampion_0 = 16, // @ C016_COMMAND_SET_LEADER_CHAMPION_0
	k17_CommandSetLeaderChampion_1 = 17, // @ C017_COMMAND_SET_LEADER_CHAMPION_1
	k18_CommandSetLeaderChampion_2 = 18, // @ C018_COMMAND_SET_LEADER_CHAMPION_2
	k19_CommandSetLeaderChampion_3 = 19, // @ C019_COMMAND_SET_LEADER_CHAMPION_3
	k20_CommandClickOnSlotBoxChampion_0_StatusBoxReadyHand = 20, // @ C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND
	k21_CommandClickOnSlotBoxChampion_0_StatusBoxActionHand = 21, // @ C021_COMMAND_CLICK_ON_SLOT_BOX_01_CHAMPION_0_STATUS_BOX_ACTION_HAND
	k22_CommandClickOnSlotBoxChampion_1_StatusBoxReadyHand = 22, // @ C022_COMMAND_CLICK_ON_SLOT_BOX_02_CHAMPION_1_STATUS_BOX_READY_HAND
	k23_CommandClickOnSlotBoxChampion_1_StatusBoxActionHand = 23, // @ C023_COMMAND_CLICK_ON_SLOT_BOX_03_CHAMPION_1_STATUS_BOX_ACTION_HAND
	k24_CommandClickOnSlotBoxChampion_2_StatusBoxReadyHand = 24, // @ C024_COMMAND_CLICK_ON_SLOT_BOX_04_CHAMPION_2_STATUS_BOX_READY_HAND
	k25_CommandClickOnSlotBoxChampion_2_StatusBoxActionHand = 25, // @ C025_COMMAND_CLICK_ON_SLOT_BOX_05_CHAMPION_2_STATUS_BOX_ACTION_HAND
	k26_CommandClickOnSlotBoxChampion_3_StatusBoxReadyHand = 26, // @ C026_COMMAND_CLICK_ON_SLOT_BOX_06_CHAMPION_3_STATUS_BOX_READY_HAND
	k27_CommandClickOnSlotBoxChampion_3_StatusBoxActionHand = 27, // @ C027_COMMAND_CLICK_ON_SLOT_BOX_07_CHAMPION_3_STATUS_BOX_ACTION_HAND
	k28_CommandClickOnSlotBoxInventoryReadyHand = 28, // @ C028_COMMAND_CLICK_ON_SLOT_BOX_08_INVENTORY_READY_HAND
	k29_CommandClickOnSlotBoxInventoryActionHand = 29, // @ C029_COMMAND_CLICK_ON_SLOT_BOX_09_INVENTORY_ACTION_HAND
	k30_CommandClickOnSlotBoxInventoryHead = 30, // @ C030_COMMAND_CLICK_ON_SLOT_BOX_10_INVENTORY_HEAD
	k31_CommandClickOnSlotBoxInventoryTorso = 31, // @ C031_COMMAND_CLICK_ON_SLOT_BOX_11_INVENTORY_TORSO
	k32_CommandClickOnSlotBoxInventoryLegs = 32, // @ C032_COMMAND_CLICK_ON_SLOT_BOX_12_INVENTORY_LEGS
	k33_CommandClickOnSlotBoxInventoryFeet = 33, // @ C033_COMMAND_CLICK_ON_SLOT_BOX_13_INVENTORY_FEET
	k34_CommandClickOnSlotBoxInventoryPouch_2 = 34, // @ C034_COMMAND_CLICK_ON_SLOT_BOX_14_INVENTORY_POUCH_2
	k35_CommandClickOnSlotBoxInventoryQuiverLine_2_1 = 35, // @ C035_COMMAND_CLICK_ON_SLOT_BOX_15_INVENTORY_QUIVER_LINE2_1
	k36_CommandClickOnSlotBoxInventoryQuiverLine_1_2 = 36, // @ C036_COMMAND_CLICK_ON_SLOT_BOX_16_INVENTORY_QUIVER_LINE1_2
	k37_CommandClickOnSlotBoxInventoryQuiverLine_2_2 = 37, // @ C037_COMMAND_CLICK_ON_SLOT_BOX_17_INVENTORY_QUIVER_LINE2_2
	k38_CommandClickOnSlotBoxInventoryNeck = 38, // @ C038_COMMAND_CLICK_ON_SLOT_BOX_18_INVENTORY_NECK
	k39_CommandClickOnSlotBoxInventoryPouch_1 = 39, // @ C039_COMMAND_CLICK_ON_SLOT_BOX_19_INVENTORY_POUCH_1
	k40_CommandClickOnSlotBoxInventoryQuiverLine_1_1 = 40, // @ C040_COMMAND_CLICK_ON_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1
	k41_CommandClickOnSlotBoxInventoryBackpackLine_1_1 = 41, // @ C041_COMMAND_CLICK_ON_SLOT_BOX_21_INVENTORY_BACKPACK_LINE1_1
	k42_CommandClickOnSlotBoxInventoryBackpackLine_2_2 = 42, // @ C042_COMMAND_CLICK_ON_SLOT_BOX_22_INVENTORY_BACKPACK_LINE2_2
	k43_CommandClickOnSlotBoxInventoryBackpackLine_2_3 = 43, // @ C043_COMMAND_CLICK_ON_SLOT_BOX_23_INVENTORY_BACKPACK_LINE2_3
	k44_CommandClickOnSlotBoxInventoryBackpackLine_2_4 = 44, // @ C044_COMMAND_CLICK_ON_SLOT_BOX_24_INVENTORY_BACKPACK_LINE2_4
	k45_CommandClickOnSlotBoxInventoryBackpackLine_2_5 = 45, // @ C045_COMMAND_CLICK_ON_SLOT_BOX_25_INVENTORY_BACKPACK_LINE2_5
	k46_CommandClickOnSlotBoxInventoryBackpackLine_2_6 = 46, // @ C046_COMMAND_CLICK_ON_SLOT_BOX_26_INVENTORY_BACKPACK_LINE2_6
	k47_CommandClickOnSlotBoxInventoryBackpackLine_2_7 = 47, // @ C047_COMMAND_CLICK_ON_SLOT_BOX_27_INVENTORY_BACKPACK_LINE2_7
	k48_CommandClickOnSlotBoxInventoryBackpackLine_2_8 = 48, // @ C048_COMMAND_CLICK_ON_SLOT_BOX_28_INVENTORY_BACKPACK_LINE2_8
	k49_CommandClickOnSlotBoxInventoryBackpackLine_2_9 = 49, // @ C049_COMMAND_CLICK_ON_SLOT_BOX_29_INVENTORY_BACKPACK_LINE2_9
	k50_CommandClickOnSlotBoxInventoryBackpackLine_1_2 = 50, // @ C050_COMMAND_CLICK_ON_SLOT_BOX_30_INVENTORY_BACKPACK_LINE1_2
	k51_CommandClickOnSlotBoxInventoryBackpackLine_1_3 = 51, // @ C051_COMMAND_CLICK_ON_SLOT_BOX_31_INVENTORY_BACKPACK_LINE1_3
	k52_CommandClickOnSlotBoxInventoryBackpackLine_1_4 = 52, // @ C052_COMMAND_CLICK_ON_SLOT_BOX_32_INVENTORY_BACKPACK_LINE1_4
	k53_CommandClickOnSlotBoxInventoryBackpackLine_1_5 = 53, // @ C053_COMMAND_CLICK_ON_SLOT_BOX_33_INVENTORY_BACKPACK_LINE1_5
	k54_CommandClickOnSlotBoxInventoryBackpackLine_1_6 = 54, // @ C054_COMMAND_CLICK_ON_SLOT_BOX_34_INVENTORY_BACKPACK_LINE1_6
	k55_CommandClickOnSlotBoxInventoryBackpackLine_1_7 = 55, // @ C055_COMMAND_CLICK_ON_SLOT_BOX_35_INVENTORY_BACKPACK_LINE1_7
	k56_CommandClickOnSlotBoxInventoryBackpackLine_1_8 = 56, // @ C056_COMMAND_CLICK_ON_SLOT_BOX_36_INVENTORY_BACKPACK_LINE1_8
	k57_CommandClickOnSlotBoxInventoryBackpackLine_1_9 = 57, // @ C057_COMMAND_CLICK_ON_SLOT_BOX_37_INVENTORY_BACKPACK_LINE1_9
	k58_CommandClickOnSlotBoxChest_1 = 58, // @ C058_COMMAND_CLICK_ON_SLOT_BOX_38_CHEST_1
	k59_CommandClickOnSlotBoxChest_2 = 59, // @ C059_COMMAND_CLICK_ON_SLOT_BOX_39_CHEST_2
	k60_CommandClickOnSlotBoxChest_3 = 60, // @ C060_COMMAND_CLICK_ON_SLOT_BOX_40_CHEST_3
	k61_CommandClickOnSlotBoxChest_4 = 61, // @ C061_COMMAND_CLICK_ON_SLOT_BOX_41_CHEST_4
	k62_CommandClickOnSlotBoxChest_5 = 62, // @ C062_COMMAND_CLICK_ON_SLOT_BOX_42_CHEST_5
	k63_CommandClickOnSlotBoxChest_6 = 63, // @ C063_COMMAND_CLICK_ON_SLOT_BOX_43_CHEST_6
	k64_CommandClickOnSlotBoxChest_7 = 64, // @ C064_COMMAND_CLICK_ON_SLOT_BOX_44_CHEST_7
	k65_CommandClickOnSlotBoxChest_8 = 65, // @ C065_COMMAND_CLICK_ON_SLOT_BOX_45_CHEST_8
	k70_CommandClickOnMouth = 70, // @ C070_COMMAND_CLICK_ON_MOUTH
	k71_CommandClickOnEye = 71, // @ C071_COMMAND_CLICK_ON_EYE
	k80_CommandClickInDungeonView = 80, // @ C080_COMMAND_CLICK_IN_DUNGEON_VIEW
	k81_CommandClickInPanel = 81, // @ C081_COMMAND_CLICK_IN_PANEL
	k83_CommandToggleInventoryLeader = 83, // @ C083_COMMAND_TOGGLE_INVENTORY_LEADER
	k100_CommandClickInSpellArea = 100, // @ C100_COMMAND_CLICK_IN_SPELL_AREA
	k101_CommandClickInSpellAreaSymbol_1 = 101, // @ C101_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_1
	k102_CommandClickInSpellAreaSymbol_2 = 102, // @ C102_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_2
	k103_CommandClickInSpellAreaSymbol_3 = 103, // @ C103_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_3
	k104_CommandClickInSpellAreaSymbol_4 = 104, // @ C104_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_4
	k105_CommandClickInSpellAreaSymbol_5 = 105, // @ C105_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_5
	k106_CommandClickInSpellAreaSymbol_6 = 106, // @ C106_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_6
	k107_CommandClickInSpellAreaRecantSymbol = 107, // @ C107_COMMAND_CLICK_IN_SPELL_AREA_RECANT_SYMBOL
	k108_CommandClickInSpeallAreaCastSpell = 108, // @ C108_COMMAND_CLICK_IN_SPELL_AREA_CAST_SPELL
	k111_CommandClickInActionArea = 111, // @ C111_COMMAND_CLICK_IN_ACTION_AREA
	k112_CommandClickInActionAreaPass = 112, // @ C112_COMMAND_CLICK_IN_ACTION_AREA_PASS
	k113_CommandClickInActionAreaAction_0 = 113, // @ C113_COMMAND_CLICK_IN_ACTION_AREA_ACTION_0
	k114_CommandClickInActionAreaAction_1 = 114, // @ C114_COMMAND_CLICK_IN_ACTION_AREA_ACTION_1
	k115_CommandClickInActionAreaAction_2 = 115, // @ C115_COMMAND_CLICK_IN_ACTION_AREA_ACTION_2
	k116_CommandClickInActionAreaChampion_0_Action = 116, // @ C116_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_0_ACTION
	k117_CommandClickInActionAreaChampion_1_Action = 117, // @ C117_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_1_ACTION
	k118_CommandClickInActionAreaChampion_2_Action = 118, // @ C118_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_2_ACTION
	k119_CommandClickInActionAreaChampion_3_Action = 119, // @ C119_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_3_ACTION
	k125_CommandClickOnChamptionIcon_Top_Left = 125, // @ C125_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_LEFT
	k126_CommandClickOnChamptionIcon_Top_Right = 126, // @ C126_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_RIGHT
	k127_CommandClickOnChamptionIcon_Lower_Right = 127, // @ C127_COMMAND_CLICK_ON_CHAMPION_ICON_LOWER_RIGHT
	k128_CommandClickOnChamptionIcon_Lower_Left = 128, // @ C128_COMMAND_CLICK_ON_CHAMPION_ICON_LOWER_LEFT
	k140_CommandSaveGame = 140, // @ C140_COMMAND_SAVE_GAME
	k145_CommandSleep = 145, // @ C145_COMMAND_SLEEP
	k146_CommandWakeUp = 146, // @ C146_COMMAND_WAKE_UP
	k147_CommandFreezeGame = 147, // @ C147_COMMAND_FREEZE_GAME
	k148_CommandUnfreezeGame = 148, // @ C148_COMMAND_UNFREEZE_GAME
	k160_CommandClickInPanelResurrect = 160, // @ C160_COMMAND_CLICK_IN_PANEL_RESURRECT
	k161_CommandClickInPanelReincarnate = 161, // @ C161_COMMAND_CLICK_IN_PANEL_REINCARNATE
	k162_CommandClickInPanelCancel = 162, // @ C162_COMMAND_CLICK_IN_PANEL_CANCEL
	k200_CommandEntranceEnterDungeon = 200, // @ C200_COMMAND_ENTRANCE_ENTER_DUNGEON
	k201_CommandEntranceResume = 201, // @ C201_COMMAND_ENTRANCE_RESUME /* Versions 1.x and 2.x command */
	k202_CommandEntranceDrawCredits = 202, // @ C202_COMMAND_ENTRANCE_DRAW_CREDITS /* Versions 1.x and 2.x command */
	k210_CommandClickOnDialogChoice_1 = 210, // @ C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1
	k211_CommandClickOnDialogChoice_2 = 211, // @ C211_COMMAND_CLICK_ON_DIALOG_CHOICE_2
	k212_CommandClickOnDialogChoice_3 = 212, // @ C212_COMMAND_CLICK_ON_DIALOG_CHOICE_3
	k213_CommandClickOnDialogChoice_4 = 213, // @ C213_COMMAND_CLICK_ON_DIALOG_CHOICE_4
	k215_CommandRestartGame = 215 // @ C215_COMMAND_RESTART_GAME
}; // @ NONE

class Command {
public:
	Common::Point _pos;
	CommandType _type;

	Command(Common::Point position, CommandType commandType): _pos(position), _type(commandType) {}
}; // @ COMMAND


class MouseInput {
public:
	CommandType _commandTypeToIssue;
	Box _hitbox;
	MouseButton _button;

	MouseInput(CommandType type, uint16 x1, uint16 x2, uint16 y1, uint16 y2, MouseButton mouseButton)
		: _commandTypeToIssue(type), _hitbox(x1, x2 + 1, y1, y2 + 1), _button(mouseButton) {}
}; // @ MOUSE_INPUT

extern MouseInput g445_PrimaryMouseInput_Entrance[4]; // @ G0445_as_Graphic561_PrimaryMouseInput_Entrance[4]
extern MouseInput g446_PrimaryMouseInput_RestartGame[2]; // @ G0446_as_Graphic561_PrimaryMouseInput_RestartGame[2]
extern MouseInput g447_PrimaryMouseInput_Interface[20]; // @ G0447_as_Graphic561_PrimaryMouseInput_Interface[20]
extern MouseInput g448_SecondaryMouseInput_Movement[9]; // @ G0448_as_Graphic561_SecondaryMouseInput_Movement[9]
extern MouseInput g449_SecondaryMouseInput_ChampionInventory[38]; // @ G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory[38]
extern MouseInput g450_PrimaryMouseInput_PartySleeping[3]; // @ G0450_as_Graphic561_PrimaryMouseInput_PartySleeping[3]
extern MouseInput g451_PrimaryMouseInput_FrozenGame[3]; // @ G0451_as_Graphic561_PrimaryMouseInput_FrozenGame[3]
extern MouseInput g452_MouseInput_ActionAreaNames[5]; // @ G0452_as_Graphic561_MouseInput_ActionAreaNames[5]
extern MouseInput g453_MouseInput_ActionAreaIcons[5]; // @ G0453_as_Graphic561_MouseInput_ActionAreaIcons[5]
extern MouseInput g454_MouseInput_SpellArea[9]; // @ G0454_as_Graphic561_MouseInput_SpellArea[9]
extern MouseInput g455_MouseInput_ChampionNamesHands[13]; // @ G0455_as_Graphic561_MouseInput_ChampionNamesHands[13]
extern MouseInput g456_MouseInput_PanelChest[9]; // @ G0456_as_Graphic561_MouseInput_PanelChest[9]
extern MouseInput g457_MouseInput_PanelResurrectReincarnateCancel[4]; // @ G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel[4]
extern MouseInput g471_PrimaryMouseInput_ViewportDialog1Choice[2]; // @ G0471_as_Graphic561_PrimaryMouseInput_ViewportDialog1Choice[2]
extern MouseInput g472_PrimaryMouseInput_ViewportDialog2Choices[3]; // @ G0472_as_Graphic561_PrimaryMouseInput_ViewportDialog2Choices[3]
extern MouseInput g473_PrimaryMouseInput_ViewportDialog3Choices[4]; // @ G0473_as_Graphic561_PrimaryMouseInput_ViewportDialog3Choices[4]
extern MouseInput g474_PrimaryMouseInput_ViewportDialog4Choices[5]; // @ G0474_as_Graphic561_PrimaryMouseInput_ViewportDialog4Choices[5]
extern MouseInput g475_PrimaryMouseInput_ScreenDialog1Choice[2]; // @ G0475_as_Graphic561_PrimaryMouseInput_ScreenDialog1Choice[2]
extern MouseInput g476_PrimaryMouseInput_ScreenDialog2Choices[3]; // @ G0476_as_Graphic561_PrimaryMouseInput_ScreenDialog2Choices[3]
extern MouseInput g477_PrimaryMouseInput_ScreenDialog3Choices[4]; // @ G0477_as_Graphic561_PrimaryMouseInput_ScreenDialog3Choices[4]
extern MouseInput g478_PrimaryMouseInput_ScreenDialog4Choices[5]; // @ G0478_as_Graphic561_PrimaryMouseInput_ScreenDialog4Choices[5]
extern MouseInput* g480_PrimaryMouseInput_DialogSets[2][4]; // @ G0480_aaps_PrimaryMouseInput_DialogSets

class KeyboardInput {
public:
	CommandType _commandToIssue;
	Common::KeyCode _key;
	byte _modifiers;

	KeyboardInput(CommandType command, Common::KeyCode keycode, byte modifierFlags) : _commandToIssue(command), _key(keycode), _modifiers(modifierFlags) {}
}; // @ KEYBOARD_INPUT



extern KeyboardInput g458_primaryKeyboardInput_interface[7];
extern KeyboardInput g459_secondaryKeyboardInput_movement[19];
extern KeyboardInput g460_primaryKeyboardInput_partySleeping[3];
extern KeyboardInput g461_primaryKeyboardInput_frozenGame[2];


class DMEngine;

#define k0_pointerArrow 0 // @ C0_POINTER_ARROW 
#define k1_pointerHand 1 // @ C1_POINTER_HAND  

#define k0_pointerTypeArrow 0 // @ C0_POINTER_TYPE_ARROW         
#define k1_pointerTypeObjectIcon 1 // @ C1_POINTER_TYPE_OBJECT_ICON   
#define k2_pointerTypeChampionIcon 2 // @ C2_POINTER_TYPE_CHAMPION_ICON 
#define k3_pointerTypeHand 3 // @ C3_POINTER_TYPE_HAND          
#define k4_pointerTypeAutoselect 4 // @ C4_POINTER_TYPE_AUTOSELECT    




class EventManager {
	DMEngine *_vm;

	Common::Point _mousePos;
	uint16 _dummyMapIndex;

	bool _g436_pendingClickPresent; // G0436_B_PendingClickPresent
	Common::Point _g437_pendingClickPos; // @ G0437_i_PendingClickX, G0438_i_PendingClickY
	MouseButton _g439_pendingClickButton; // @ G0439_i_PendingClickButtonsStatus
	bool _g600_useObjectAsMousePointerBitmap; // @ G0600_B_UseObjectAsMousePointerBitmap
	bool _g601_useHandAsMousePointerBitmap; // @ G0601_B_UseHandAsMousePointerBitmap
	bool _gK100_preventBuildPointerScreenArea; // @ K0100_B_PreventBuildPointerScreenArea
	byte *_g615_mousePointerOriginalColorsObject; // @ G0615_puc_Bitmap_MousePointerOriginalColorsObject
	byte *_g613_mousePointerOriginalColorsChampionIcon; // @ G0613_puc_Bitmap_MousePointerOriginalColorsChampionIcon
	byte *_gK190_mousePointerTempBuffer; // @ K0190_puc_Bitmap_MousePointerTemporaryBuffer
	int16 _gK104_mousePointerType; // @ K0104_i_MousePointerType
	int16 _gK105_previousMousePointerType; // @ K0105_i_PreviousMousePointerType
	uint16 _g558_mouseButtonStatus;// @ G0588_i_MouseButtonsStatus

// this doesn't seem to be used anywhere at all
	bool _g435_isCommandQueueLocked; // @ G0435_B_CommandQueueLocked
	Common::Queue<Command> _commandQueue;

	void f365_commandTurnParty(CommandType cmdType); // @ F0365_COMMAND_ProcessTypes1To2_TurnParty
	void f366_commandMoveParty(CommandType cmdType); // @ F0366_COMMAND_ProcessTypes3To6_MoveParty
	bool f375_processType80_clickDungeonView_isLeaderHandObjThrown(int16 posX, int16 posY); // @ F0375_COMMAND_ProcessType80_ClickInDungeonView_IsLeaderHandObjectThrown
	void setMousePointerFromSpriteData(byte* mouseSprite);

public:
	explicit EventManager(DMEngine *vm);
	~EventManager();

	MouseInput* _g441_primaryMouseInput;// @ G0441_ps_PrimaryMouseInput
	MouseInput* _g442_secondaryMouseInput;// @ G0442_ps_SecondaryMouseInput
	bool _g598_mousePointerBitmapUpdated; // @ G0598_B_MousePointerBitmapUpdated
	bool _g326_refreshMousePointerInMainLoop; // @ G0326_B_RefreshMousePointerInMainLoop
	bool _g341_highlightBoxEnabled; // @ G0341_B_HighlightBoxEnabled
	uint16 _g599_useChampionIconOrdinalAsMousePointerBitmap; // @ G0599_ui_UseChampionIconOrdinalAsMousePointerBitmap
	KeyboardInput *_g443_primaryKeyboardInput; // @ G0443_ps_PrimaryKeyboardInput
	KeyboardInput *_g444_secondaryKeyboardInput; // @ G0444_ps_SecondaryKeyboardInput
	bool _g597_ignoreMouseMovements;// @ G0597_B_IgnoreMouseMovements
	int16 _g587_hideMousePointerRequestCount; // @ G0587_i_HideMousePointerRequestCount

	void initMouse();
	void f67_setMousePointerToNormal(int16 mousePointer); // @ F0067_MOUSE_SetPointerToNormal
	void f68_setPointerToObject(byte *bitmap); // @ F0068_MOUSE_SetPointerToObject
	void f71_mouseDropChampionIcon(); // @ F0071_MOUSE_DropChampionIcon
	void f73_buildpointerScreenArea(int16 mousePosX, int16 mousePosY); // @ F0073_MOUSE_BuildPointerScreenArea
	void f69_setMousePointer(); // @ F0069_MOUSE_SetPointer
	void f78_showMouse(); // @ F0077_MOUSE_HidePointer_CPSE
	void f77_hideMouse(); // @ F0078_MOUSE_ShowPointer
	bool isMouseButtonDown(MouseButton button);

	void setMousePos(Common::Point pos);
	Common::Point getMousePos() { return _mousePos; }
	/**
	* Upon encountering an event type for which the grab parameter is not null, the function
	* will return with the event type, passes the event to the grab desitination and returns without
	* processing the rest of the events into commands accoring to the current keyboard and mouse input.
	* If there are no more events, it returns with Common::EVENT_INVALID.
	*/
	Common::EventType processInput(Common::Event *grabKey = nullptr, Common::Event *grabMouseClick = nullptr);
	void f360_processPendingClick(); // @ F0360_COMMAND_ProcessPendingClick
	void f359_processClick(Common::Point mousePos, MouseButton button); // @ F0359_COMMAND_ProcessClick_CPSC
	CommandType f358_getCommandTypeFromMouseInput(MouseInput *input, Common::Point mousePos, MouseButton button); // @ F0358_COMMAND_GetCommandFromMouseInput_CPSC
	void f380_processCommandQueue(); // @ F0380_COMMAND_ProcessQueue_CPSC

	void f368_commandSetLeader(ChampionIndex index); // @ F0368_COMMAND_SetLeader
	void f372_commandProcessType80ClickInDungeonViewTouchFrontWall(); // @ F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWall
	void f377_commandProcessType80ClickInDungeonView(int16 posX, int16 posY); // @ F0377_COMMAND_ProcessType80_ClickInDungeonView
	void f282_commandProcessCommands160To162ClickInResurrectReincarnatePanel(CommandType commandType); // @ F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel
	void f378_commandProcess81ClickInPanel(int16 x, int16 y); // @ F0378_COMMAND_ProcessType81_ClickInPanel
	void f373_processType80_clickInDungeonView_grabLeaderHandObject(uint16 viewCell); // @ F0373_COMMAND_ProcessType80_ClickInDungeonView_GrabLeaderHandObject
	void f374_processType80_clickInDungeonViewDropLeaderHandObject(uint16 viewCell); // @ F0374_COMMAND_ProcessType80_ClickInDungeonView_DropLeaderHandObject

	bool f360_hasPendingClick(Common::Point &point, MouseButton button); // @ F0360_COMMAND_ProcessPendingClick
	void f379_drawSleepScreen(); // @ F0379_COMMAND_DrawSleepScreen
	void f357_discardAllInput(); // @ F0357_COMMAND_DiscardAllInput
	void f364_commandTakeStairs(bool stairsGoDown);// @ F0364_COMMAND_TakeStairs
	void f367_commandProcessTypes12to27_clickInChampionStatusBox(uint16 champIndex, int16 posX,
																 int16 posY); // @ F0367_COMMAND_ProcessTypes12To27_ClickInChampionStatusBox
	void f70_mouseProcessCommands125To128_clickOnChampionIcon(uint16 champIconIndex); // @ F0070_MOUSE_ProcessCommands125To128_ClickOnChampionIcon
	void f370_commandProcessType100_clickInSpellArea(uint16 posX, uint16 posY); // @ F0370_COMMAND_ProcessType100_ClickInSpellArea
	void f369_commandProcessTypes101To108_clickInSpellSymbolsArea(CommandType cmdType); // @ F0369_COMMAND_ProcessTypes101To108_ClickInSpellSymbolsArea_CPSE
	void f371_commandProcessType111To115_ClickInActionArea(int16 posX, int16 posY); // @ F0371_COMMAND_ProcessType111To115_ClickInActionArea_CPSE
	void f544_resetPressingEyeOrMouth(); // @ F0544_INPUT_ResetPressingEyeOrMouth
	void f541_waitForMouseOrKeyActivity(); // @ F0541_INPUT_WaitForMouseOrKeyboardActivity
	void f362_commandHighlightBoxEnable(int16 x1, int16 x2, int16 y1, int16 y2) { warning(false, "STUB METHOD: f362_commandHighlightBoxEnable"); }// @ F0362_COMMAND_HighlightBoxEnable
	void f363_highlightBoxDisable() { warning(false, "STUB METHOD: f363_highlightBoxDisable"); } // @ F0363_COMMAND_HighlightBoxDisable
};

}

#endif
