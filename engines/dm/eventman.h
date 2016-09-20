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

#include "dm/dm.h"
#include "dm/gfx.h"
#include "dm/champion.h"

namespace DM {

enum MouseButton {
	kDMMouseButtonNone = 0, // present only because of typesafety
	kDMMouseButtonLeft = 1,
	kDMMouseButtonRight = 2
};

enum CommandType {
	kDMCommandNone = 0, // @ C000_COMMAND_NONE
	kDMCommandTurnLeft = 1, // @ C001_COMMAND_TURN_LEFT
	kDMCommandTurnRight = 2, // @ C002_COMMAND_TURN_RIGHT
	kDMCommandMoveForward = 3, // @ C003_COMMAND_MOVE_FORWARD
	kDMCommandMoveRight = 4, // @ C004_COMMAND_MOVE_RIGHT
	kDMCommandMoveBackward = 5, // @ C005_COMMAND_MOVE_BACKWARD
	kDMCommandMoveLeft = 6, // @ C006_COMMAND_MOVE_LEFT
	kDMCommandToggleInventoryChampion0 = 7, // @ C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0
	kDMCommandToggleInventoryChampion1 = 8, // @ C008_COMMAND_TOGGLE_INVENTORY_CHAMPION_1
	kDMCommandToggleInventoryChampion2 = 9, // @ C009_COMMAND_TOGGLE_INVENTORY_CHAMPION_2
	kDMCommandToggleInventoryChampion3 = 10, // @ C010_COMMAND_TOGGLE_INVENTORY_CHAMPION_3
	kDMCommandCloseInventory = 11, // @ C011_COMMAND_CLOSE_INVENTORY
	kDMCommandClickInChampion0StatusBox = 12, // @ C012_COMMAND_CLICK_IN_CHAMPION_0_STATUS_BOX
	kDMCommandClickInChampion1StatusBox = 13, // @ C013_COMMAND_CLICK_IN_CHAMPION_1_STATUS_BOX
	kDMCommandClickInChampion2StatusBox = 14, // @ C014_COMMAND_CLICK_IN_CHAMPION_2_STATUS_BOX
	kDMCommandClickInChampion3StatusBox = 15, // @ C015_COMMAND_CLICK_IN_CHAMPION_3_STATUS_BOX
	kDMCommandSetLeaderChampion0 = 16, // @ C016_COMMAND_SET_LEADER_CHAMPION_0
	kDMCommandSetLeaderChampion1 = 17, // @ C017_COMMAND_SET_LEADER_CHAMPION_1
	kDMCommandSetLeaderChampion2 = 18, // @ C018_COMMAND_SET_LEADER_CHAMPION_2
	kDMCommandSetLeaderChampion3 = 19, // @ C019_COMMAND_SET_LEADER_CHAMPION_3
	kDMCommandClickOnSlotBoxChampion0StatusBoxReadyHand = 20, // @ C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND
	kDMCommandClickOnSlotBoxChampion0StatusBoxActionHand = 21, // @ C021_COMMAND_CLICK_ON_SLOT_BOX_01_CHAMPION_0_STATUS_BOX_ACTION_HAND
	kDMCommandClickOnSlotBoxChampion1StatusBoxReadyHand = 22, // @ C022_COMMAND_CLICK_ON_SLOT_BOX_02_CHAMPION_1_STATUS_BOX_READY_HAND
	kDMCommandClickOnSlotBoxChampion1StatusBoxActionHand = 23, // @ C023_COMMAND_CLICK_ON_SLOT_BOX_03_CHAMPION_1_STATUS_BOX_ACTION_HAND
	kDMCommandClickOnSlotBoxChampion2StatusBoxReadyHand = 24, // @ C024_COMMAND_CLICK_ON_SLOT_BOX_04_CHAMPION_2_STATUS_BOX_READY_HAND
	kDMCommandClickOnSlotBoxChampion2StatusBoxActionHand = 25, // @ C025_COMMAND_CLICK_ON_SLOT_BOX_05_CHAMPION_2_STATUS_BOX_ACTION_HAND
	kDMCommandClickOnSlotBoxChampion3StatusBoxReadyHand = 26, // @ C026_COMMAND_CLICK_ON_SLOT_BOX_06_CHAMPION_3_STATUS_BOX_READY_HAND
	kDMCommandClickOnSlotBoxChampion3StatusBoxActionHand = 27, // @ C027_COMMAND_CLICK_ON_SLOT_BOX_07_CHAMPION_3_STATUS_BOX_ACTION_HAND
	kDMCommandClickOnSlotBoxInventoryReadyHand = 28, // @ C028_COMMAND_CLICK_ON_SLOT_BOX_08_INVENTORY_READY_HAND
	kDMCommandClickOnSlotBoxInventoryActionHand = 29, // @ C029_COMMAND_CLICK_ON_SLOT_BOX_09_INVENTORY_ACTION_HAND
	kDMCommandClickOnSlotBoxInventoryHead = 30, // @ C030_COMMAND_CLICK_ON_SLOT_BOX_10_INVENTORY_HEAD
	kDMCommandClickOnSlotBoxInventoryTorso = 31, // @ C031_COMMAND_CLICK_ON_SLOT_BOX_11_INVENTORY_TORSO
	kDMCommandClickOnSlotBoxInventoryLegs = 32, // @ C032_COMMAND_CLICK_ON_SLOT_BOX_12_INVENTORY_LEGS
	kDMCommandClickOnSlotBoxInventoryFeet = 33, // @ C033_COMMAND_CLICK_ON_SLOT_BOX_13_INVENTORY_FEET
	kDMCommandClickOnSlotBoxInventoryPouch2 = 34, // @ C034_COMMAND_CLICK_ON_SLOT_BOX_14_INVENTORY_POUCH_2
	kDMCommandClickOnSlotBoxInventoryQuiverLine2_1 = 35, // @ C035_COMMAND_CLICK_ON_SLOT_BOX_15_INVENTORY_QUIVER_LINE2_1
	kDMCommandClickOnSlotBoxInventoryQuiverLine1_2 = 36, // @ C036_COMMAND_CLICK_ON_SLOT_BOX_16_INVENTORY_QUIVER_LINE1_2
	kDMCommandClickOnSlotBoxInventoryQuiverLine2_2 = 37, // @ C037_COMMAND_CLICK_ON_SLOT_BOX_17_INVENTORY_QUIVER_LINE2_2
	kDMCommandClickOnSlotBoxInventoryNeck = 38, // @ C038_COMMAND_CLICK_ON_SLOT_BOX_18_INVENTORY_NECK
	kDMCommandClickOnSlotBoxInventoryPouch1 = 39, // @ C039_COMMAND_CLICK_ON_SLOT_BOX_19_INVENTORY_POUCH_1
	kDMCommandClickOnSlotBoxInventoryQuiverLine1_1 = 40, // @ C040_COMMAND_CLICK_ON_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1
	kDMCommandClickOnSlotBoxInventoryBackpackLine1_1 = 41, // @ C041_COMMAND_CLICK_ON_SLOT_BOX_21_INVENTORY_BACKPACK_LINE1_1
	kDMCommandClickOnSlotBoxInventoryBackpackLine2_2 = 42, // @ C042_COMMAND_CLICK_ON_SLOT_BOX_22_INVENTORY_BACKPACK_LINE2_2
	kDMCommandClickOnSlotBoxInventoryBackpackLine2_3 = 43, // @ C043_COMMAND_CLICK_ON_SLOT_BOX_23_INVENTORY_BACKPACK_LINE2_3
	kDMCommandClickOnSlotBoxInventoryBackpackLine2_4 = 44, // @ C044_COMMAND_CLICK_ON_SLOT_BOX_24_INVENTORY_BACKPACK_LINE2_4
	kDMCommandClickOnSlotBoxInventoryBackpackLine2_5 = 45, // @ C045_COMMAND_CLICK_ON_SLOT_BOX_25_INVENTORY_BACKPACK_LINE2_5
	kDMCommandClickOnSlotBoxInventoryBackpackLine2_6 = 46, // @ C046_COMMAND_CLICK_ON_SLOT_BOX_26_INVENTORY_BACKPACK_LINE2_6
	kDMCommandClickOnSlotBoxInventoryBackpackLine2_7 = 47, // @ C047_COMMAND_CLICK_ON_SLOT_BOX_27_INVENTORY_BACKPACK_LINE2_7
	kDMCommandClickOnSlotBoxInventoryBackpackLine2_8 = 48, // @ C048_COMMAND_CLICK_ON_SLOT_BOX_28_INVENTORY_BACKPACK_LINE2_8
	kDMCommandClickOnSlotBoxInventoryBackpackLine2_9 = 49, // @ C049_COMMAND_CLICK_ON_SLOT_BOX_29_INVENTORY_BACKPACK_LINE2_9
	kDMCommandClickOnSlotBoxInventoryBackpackLine1_2 = 50, // @ C050_COMMAND_CLICK_ON_SLOT_BOX_30_INVENTORY_BACKPACK_LINE1_2
	kDMCommandClickOnSlotBoxInventoryBackpackLine1_3 = 51, // @ C051_COMMAND_CLICK_ON_SLOT_BOX_31_INVENTORY_BACKPACK_LINE1_3
	kDMCommandClickOnSlotBoxInventoryBackpackLine1_4 = 52, // @ C052_COMMAND_CLICK_ON_SLOT_BOX_32_INVENTORY_BACKPACK_LINE1_4
	kDMCommandClickOnSlotBoxInventoryBackpackLine1_5 = 53, // @ C053_COMMAND_CLICK_ON_SLOT_BOX_33_INVENTORY_BACKPACK_LINE1_5
	kDMCommandClickOnSlotBoxInventoryBackpackLine1_6 = 54, // @ C054_COMMAND_CLICK_ON_SLOT_BOX_34_INVENTORY_BACKPACK_LINE1_6
	kDMCommandClickOnSlotBoxInventoryBackpackLine1_7 = 55, // @ C055_COMMAND_CLICK_ON_SLOT_BOX_35_INVENTORY_BACKPACK_LINE1_7
	kDMCommandClickOnSlotBoxInventoryBackpackLine1_8 = 56, // @ C056_COMMAND_CLICK_ON_SLOT_BOX_36_INVENTORY_BACKPACK_LINE1_8
	kDMCommandClickOnSlotBoxInventoryBackpackLine1_9 = 57, // @ C057_COMMAND_CLICK_ON_SLOT_BOX_37_INVENTORY_BACKPACK_LINE1_9
	kDMCommandClickOnSlotBoxChest1 = 58, // @ C058_COMMAND_CLICK_ON_SLOT_BOX_38_CHEST_1
	kDMCommandClickOnSlotBoxChest2 = 59, // @ C059_COMMAND_CLICK_ON_SLOT_BOX_39_CHEST_2
	kDMCommandClickOnSlotBoxChest3 = 60, // @ C060_COMMAND_CLICK_ON_SLOT_BOX_40_CHEST_3
	kDMCommandClickOnSlotBoxChest4 = 61, // @ C061_COMMAND_CLICK_ON_SLOT_BOX_41_CHEST_4
	kDMCommandClickOnSlotBoxChest5 = 62, // @ C062_COMMAND_CLICK_ON_SLOT_BOX_42_CHEST_5
	kDMCommandClickOnSlotBoxChest6 = 63, // @ C063_COMMAND_CLICK_ON_SLOT_BOX_43_CHEST_6
	kDMCommandClickOnSlotBoxChest7 = 64, // @ C064_COMMAND_CLICK_ON_SLOT_BOX_44_CHEST_7
	kDMCommandClickOnSlotBoxChest8 = 65, // @ C065_COMMAND_CLICK_ON_SLOT_BOX_45_CHEST_8
	kDMCommandClickOnMouth = 70, // @ C070_COMMAND_CLICK_ON_MOUTH
	kDMCommandClickOnEye = 71, // @ C071_COMMAND_CLICK_ON_EYE
	kDMCommandClickInDungeonView = 80, // @ C080_COMMAND_CLICK_IN_DUNGEON_VIEW
	kDMCommandClickInPanel = 81, // @ C081_COMMAND_CLICK_IN_PANEL
	kDMCommandToggleInventoryLeader = 83, // @ C083_COMMAND_TOGGLE_INVENTORY_LEADER
	kDMCommandClickInSpellArea = 100, // @ C100_COMMAND_CLICK_IN_SPELL_AREA
	kDMCommandClickInSpellAreaSymbol1 = 101, // @ C101_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_1
	kDMCommandClickInSpellAreaSymbol2 = 102, // @ C102_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_2
	kDMCommandClickInSpellAreaSymbol3 = 103, // @ C103_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_3
	kDMCommandClickInSpellAreaSymbol4 = 104, // @ C104_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_4
	kDMCommandClickInSpellAreaSymbol5 = 105, // @ C105_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_5
	kDMCommandClickInSpellAreaSymbol6 = 106, // @ C106_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_6
	kDMCommandClickInSpellAreaRecantSymbol = 107, // @ C107_COMMAND_CLICK_IN_SPELL_AREA_RECANT_SYMBOL
	kDMCommandClickInSpeallAreaCastSpell = 108, // @ C108_COMMAND_CLICK_IN_SPELL_AREA_CAST_SPELL
	kDMCommandClickInActionArea = 111, // @ C111_COMMAND_CLICK_IN_ACTION_AREA
	kDMCommandClickInActionAreaPass = 112, // @ C112_COMMAND_CLICK_IN_ACTION_AREA_PASS
	kDMCommandClickInActionAreaAction0 = 113, // @ C113_COMMAND_CLICK_IN_ACTION_AREA_ACTION_0
	kDMCommandClickInActionAreaAction1 = 114, // @ C114_COMMAND_CLICK_IN_ACTION_AREA_ACTION_1
	kDMCommandClickInActionAreaAction2 = 115, // @ C115_COMMAND_CLICK_IN_ACTION_AREA_ACTION_2
	kDMCommandClickInActionAreaChampion0Action = 116, // @ C116_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_0_ACTION
	kDMCommandClickInActionAreaChampion1Action = 117, // @ C117_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_1_ACTION
	kDMCommandClickInActionAreaChampion2Action = 118, // @ C118_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_2_ACTION
	kDMCommandClickInActionAreaChampion3Action = 119, // @ C119_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_3_ACTION
	kDMCommandClickOnChamptionIconTopLeft = 125, // @ C125_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_LEFT
	kDMCommandClickOnChamptionIconTopRight = 126, // @ C126_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_RIGHT
	kDMCommandClickOnChamptionIconLowerRight = 127, // @ C127_COMMAND_CLICK_ON_CHAMPION_ICON_LOWER_RIGHT
	kDMCommandClickOnChamptionIconLowerLeft = 128, // @ C128_COMMAND_CLICK_ON_CHAMPION_ICON_LOWER_LEFT
	kDMCommandSaveGame = 140, // @ C140_COMMAND_SAVE_GAME
	kDMCommandSleep = 145, // @ C145_COMMAND_SLEEP
	kDMCommandWakeUp = 146, // @ C146_COMMAND_WAKE_UP
	kDMCommandFreezeGame = 147, // @ C147_COMMAND_FREEZE_GAME
	kDMCommandUnfreezeGame = 148, // @ C148_COMMAND_UNFREEZE_GAME
	kDMCommandClickInPanelResurrect = 160, // @ C160_COMMAND_CLICK_IN_PANEL_RESURRECT
	kDMCommandClickInPanelReincarnate = 161, // @ C161_COMMAND_CLICK_IN_PANEL_REINCARNATE
	kDMCommandClickInPanelCancel = 162, // @ C162_COMMAND_CLICK_IN_PANEL_CANCEL
	kDMCommandEntranceEnterDungeon = 200, // @ C200_COMMAND_ENTRANCE_ENTER_DUNGEON
	kDMCommandEntranceResume = 201, // @ C201_COMMAND_ENTRANCE_RESUME /* Versions 1.x and 2.x command */
	kDMCommandEntranceDrawCredits = 202, // @ C202_COMMAND_ENTRANCE_DRAW_CREDITS /* Versions 1.x and 2.x command */
	kDMCommandClickOnDialogChoice1 = 210, // @ C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1
	kDMCommandClickOnDialogChoice2 = 211, // @ C211_COMMAND_CLICK_ON_DIALOG_CHOICE_2
	kDMCommandClickOnDialogChoice3 = 212, // @ C212_COMMAND_CLICK_ON_DIALOG_CHOICE_3
	kDMCommandClickOnDialogChoice4 = 213, // @ C213_COMMAND_CLICK_ON_DIALOG_CHOICE_4
	kDMCommandRestartGame = 215 // @ C215_COMMAND_RESTART_GAME
};

class Command {
public:
	Common::Point _pos;
	CommandType _type;

	Command(Common::Point position, CommandType commandType) : _pos(position), _type(commandType) {}
}; // @ COMMAND


class MouseInput {
public:
	CommandType _commandTypeToIssue;
	Box _hitbox;
	MouseButton _button;

	MouseInput(CommandType type, uint16 x1, uint16 x2, uint16 y1, uint16 y2, MouseButton mouseButton)
		: _commandTypeToIssue(type), _hitbox(x1, x2 + 1, y1, y2 + 1), _button(mouseButton) {}
	MouseInput()
		: _commandTypeToIssue(kDMCommandNone), _hitbox(0, 1, 0, 1), _button(kDMMouseButtonNone) {}
}; // @ MOUSE_INPUT

class KeyboardInput {
public:
	CommandType _commandToIssue;
	Common::KeyCode _key;
	byte _modifiers;

	KeyboardInput(CommandType command, Common::KeyCode keycode, byte modifierFlags) : _commandToIssue(command), _key(keycode), _modifiers(modifierFlags) {}
	KeyboardInput() : _commandToIssue(kDMCommandNone), _key(Common::KEYCODE_ESCAPE), _modifiers(0) {}
}; // @ KEYBOARD_INPUT

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

	bool _pendingClickPresent; // G0436_B_PendingClickPresent
	Common::Point _pendingClickPos; // @ G0437_i_PendingClickX, G0438_i_PendingClickY
	MouseButton _pendingClickButton; // @ G0439_i_PendingClickButtonsStatus
	bool _useObjectAsMousePointerBitmap; // @ G0600_B_UseObjectAsMousePointerBitmap
	bool _useHandAsMousePointerBitmap; // @ G0601_B_UseHandAsMousePointerBitmap
	bool _preventBuildPointerScreenArea; // @ K0100_B_PreventBuildPointerScreenArea
	byte *_mousePointerOriginalColorsObject; // @ G0615_puc_Bitmap_MousePointerOriginalColorsObject
	byte *_mousePointerOriginalColorsChampionIcon; // @ G0613_puc_Bitmap_MousePointerOriginalColorsChampionIcon
	byte *_mousePointerTempBuffer; // @ K0190_puc_Bitmap_MousePointerTemporaryBuffer
	int16 _mousePointerType; // @ K0104_i_MousePointerType
	int16 _previousMousePointerType; // @ K0105_i_PreviousMousePointerType
	uint16 _mouseButtonStatus;// @ G0588_i_MouseButtonsStatus

// this doesn't seem to be used anywhere at all
	bool _isCommandQueueLocked; // @ G0435_B_CommandQueueLocked
	Common::Queue<Command> _commandQueue;

	void commandTurnParty(CommandType cmdType); // @ F0365_COMMAND_ProcessTypes1To2_TurnParty
	void commandMoveParty(CommandType cmdType); // @ F0366_COMMAND_ProcessTypes3To6_MoveParty
	bool isLeaderHandObjThrown(int16 posX, int16 posY); // @ F0375_COMMAND_ProcessType80_ClickInDungeonView_IsLeaderHandObjectThrown
	void setMousePointerFromSpriteData(byte *mouseSprite);

	Box _highlightScreenBox; // @ G0336_i_HighlightBoxX1
public:
	explicit EventManager(DMEngine *vm);
	~EventManager();

	MouseInput *_primaryMouseInput;// @ G0441_ps_PrimaryMouseInput
	MouseInput *_secondaryMouseInput;// @ G0442_ps_SecondaryMouseInput
	bool _mousePointerBitmapUpdated; // @ G0598_B_MousePointerBitmapUpdated
	bool _refreshMousePointerInMainLoop; // @ G0326_B_RefreshMousePointerInMainLoop
	bool _highlightBoxEnabled; // @ G0341_B_HighlightBoxEnabled
	uint16 _useChampionIconOrdinalAsMousePointerBitmap; // @ G0599_ui_UseChampionIconOrdinalAsMousePointerBitmap
	KeyboardInput *_primaryKeyboardInput; // @ G0443_ps_PrimaryKeyboardInput
	KeyboardInput *_secondaryKeyboardInput; // @ G0444_ps_SecondaryKeyboardInput
	bool _ignoreMouseMovements;// @ G0597_B_IgnoreMouseMovements
	int16 _hideMousePointerRequestCount; // @ G0587_i_HideMousePointerRequestCount

	void initMouse();
	void setMousePointerToNormal(int16 mousePointer); // @ F0067_MOUSE_SetPointerToNormal
	void setPointerToObject(byte *bitmap); // @ F0068_MOUSE_SetPointerToObject
	void mouseDropChampionIcon(); // @ F0071_MOUSE_DropChampionIcon
	void buildpointerScreenArea(int16 mousePosX, int16 mousePosY); // @ F0073_MOUSE_BuildPointerScreenArea
	void setMousePointer(); // @ F0069_MOUSE_SetPointer
	void showMouse(); // @ F0077_MOUSE_HidePointer_CPSE
	void hideMouse(); // @ F0078_MOUSE_ShowPointer
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
	void processPendingClick(); // @ F0360_COMMAND_ProcessPendingClick
	void processClick(Common::Point mousePos, MouseButton button); // @ F0359_COMMAND_ProcessClick_CPSC
	CommandType getCommandTypeFromMouseInput(MouseInput *input, Common::Point mousePos, MouseButton button); // @ F0358_COMMAND_GetCommandFromMouseInput_CPSC
	void processCommandQueue(); // @ F0380_COMMAND_ProcessQueue_CPSC

	void commandSetLeader(ChampionIndex index); // @ F0368_COMMAND_SetLeader
	void commandProcessType80ClickInDungeonViewTouchFrontWall(); // @ F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWall
	void commandProcessType80ClickInDungeonView(int16 posX, int16 posY); // @ F0377_COMMAND_ProcessType80_ClickInDungeonView
	void commandProcessCommands160To162ClickInResurrectReincarnatePanel(CommandType commandType); // @ F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel
	void commandProcess81ClickInPanel(int16 x, int16 y); // @ F0378_COMMAND_ProcessType81_ClickInPanel
	void processType80_clickInDungeonView_grabLeaderHandObject(uint16 viewCell); // @ F0373_COMMAND_ProcessType80_ClickInDungeonView_GrabLeaderHandObject
	void clickInDungeonViewDropLeaderHandObject(uint16 viewCell); // @ F0374_COMMAND_ProcessType80_ClickInDungeonView_DropLeaderHandObject

	bool hasPendingClick(Common::Point &point, MouseButton button); // @ F0360_COMMAND_ProcessPendingClick
	void drawSleepScreen(); // @ F0379_COMMAND_DrawSleepScreen
	void discardAllInput(); // @ F0357_COMMAND_DiscardAllInput
	void commandTakeStairs(bool stairsGoDown);// @ F0364_COMMAND_TakeStairs
	void commandProcessTypes12to27_clickInChampionStatusBox(uint16 champIndex, int16 posX,
																 int16 posY); // @ F0367_COMMAND_ProcessTypes12To27_ClickInChampionStatusBox
	void mouseProcessCommands125To128_clickOnChampionIcon(uint16 champIconIndex); // @ F0070_MOUSE_ProcessCommands125To128_ClickOnChampionIcon
	void commandProcessType100_clickInSpellArea(uint16 posX, uint16 posY); // @ F0370_COMMAND_ProcessType100_ClickInSpellArea
	void commandProcessTypes101To108_clickInSpellSymbolsArea(CommandType cmdType); // @ F0369_COMMAND_ProcessTypes101To108_ClickInSpellSymbolsArea_CPSE
	void commandProcessType111To115_ClickInActionArea(int16 posX, int16 posY); // @ F0371_COMMAND_ProcessType111To115_ClickInActionArea_CPSE
	void resetPressingEyeOrMouth(); // @ F0544_INPUT_ResetPressingEyeOrMouth
	void waitForMouseOrKeyActivity(); // @ F0541_INPUT_WaitForMouseOrKeyboardActivity
	void commandHighlightBoxEnable(int16 x1, int16 x2, int16 y1, int16 y2); // @ F0362_COMMAND_HighlightBoxEnable
	void highlightBoxDisable(); // @ F0363_COMMAND_HighlightBoxDisable
	void highlightScreenBox(int16 x1, int16 x2, int16 y1, int16 y2) { warning("STUB METHOD: highlightScreenBox"); } // @  F0006_MAIN_HighlightScreenBox

	KeyboardInput _primaryKeyboardInputInterface[7]; // @ G0458_as_Graphic561_PrimaryKeyboardInput_Interface
	KeyboardInput _secondaryKeyboardInputMovement[19]; // @ G0459_as_Graphic561_SecondaryKeyboardInput_Movement
	KeyboardInput _primaryKeyboardInputPartySleeping[3]; // @ G0460_as_Graphic561_PrimaryKeyboardInput_PartySleeping
	KeyboardInput _primaryKeyboardInputFrozenGame[2]; // @ G0461_as_Graphic561_PrimaryKeyboardInput_FrozenGame
	MouseInput _primaryMouseInputEntrance[4]; // @ G0445_as_Graphic561_PrimaryMouseInput_Entrance[4]
	MouseInput _primaryMouseInputRestartGame[2]; // @ G0446_as_Graphic561_PrimaryMouseInput_RestartGame[2]
	MouseInput _primaryMouseInputInterface[20]; // @ G0447_as_Graphic561_PrimaryMouseInput_Interface[20]
	MouseInput _secondaryMouseInputMovement[9]; // @ G0448_as_Graphic561_SecondaryMouseInput_Movement[9]
	MouseInput _secondaryMouseInputChampionInventory[38]; // @ G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory[38]
	MouseInput _primaryMouseInputPartySleeping[3]; // @ G0450_as_Graphic561_PrimaryMouseInput_PartySleeping[3]
	MouseInput _primaryMouseInputFrozenGame[3]; // @ G0451_as_Graphic561_PrimaryMouseInput_FrozenGame[3]
	MouseInput _mouseInputActionAreaNames[5]; // @ G0452_as_Graphic561_MouseInput_ActionAreaNames[5]
	MouseInput _mouseInputActionAreaIcons[5]; // @ G0453_as_Graphic561_MouseInput_ActionAreaIcons[5]
	MouseInput _mouseInputSpellArea[9]; // @ G0454_as_Graphic561_MouseInput_SpellArea[9]
	MouseInput _mouseInputChampionNamesHands[13]; // @ G0455_as_Graphic561_MouseInput_ChampionNamesHands[13]
	MouseInput _mouseInputPanelChest[9]; // @ G0456_as_Graphic561_MouseInput_PanelChest[9]
	MouseInput _mouseInputPanelResurrectReincarnateCancel[4]; // @ G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel[4]
	MouseInput _primaryMouseInputViewportDialog1Choice[2]; // @ G0471_as_Graphic561_PrimaryMouseInput_ViewportDialog1Choice[2]
	MouseInput _primaryMouseInputScreenDialog1Choice[2]; // @ G0475_as_Graphic561_PrimaryMouseInput_ScreenDialog1Choice[2]
	MouseInput _primaryMouseInputViewportDialog2Choices[3]; // @ G0472_as_Graphic561_PrimaryMouseInput_ViewportDialog2Choices[3]
	MouseInput _primaryMouseInputScreenDialog2Choices[3]; // @ G0476_as_Graphic561_PrimaryMouseInput_ScreenDialog2Choices[3]
	MouseInput _primaryMouseInputViewportDialog3Choices[4]; // @ G0473_as_Graphic561_PrimaryMouseInput_ViewportDialog3Choices[4]
	MouseInput _primaryMouseInputScreenDialog3Choices[4]; // @ G0477_as_Graphic561_PrimaryMouseInput_ScreenDialog3Choices[4]
	MouseInput _primaryMouseInputViewportDialog4Choices[5]; // @ G0474_as_Graphic561_PrimaryMouseInput_ViewportDialog4Choices[5]
	MouseInput _primaryMouseInputScreenDialog4Choices[5]; // @ G0478_as_Graphic561_PrimaryMouseInput_ScreenDialog4Choices[5]

	MouseInput *_primaryMouseInputDialogSets[2][4]; // @ G0480_aaps_PrimaryMouseInput_DialogSets

	void initArrays();
};

}

#endif
