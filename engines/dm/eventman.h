#ifndef DM_EVENTMAN_H
#define DM_EVENTMAN_H

#include "common/events.h"
#include "common/queue.h"
#include "gfx.h"


namespace DM {

enum MouseButton {
	kNoneMouseButton = 0, // present only because of typesafety
	kLeftMouseButton = 1,
	kRightMouseButton = 2
};

enum CommandType {
	kCommandNone = 0, // @ C000_COMMAND_NONE
	kCommandTurnLeft = 1, // @ C001_COMMAND_TURN_LEFT
	kCommandTurnRight = 2, // @ C002_COMMAND_TURN_RIGHT
	kCommandMoveForward = 3, // @ C003_COMMAND_MOVE_FORWARD
	kCommandMoveRight = 4, // @ C004_COMMAND_MOVE_RIGHT
	kCommandMoveBackward = 5, // @ C005_COMMAND_MOVE_BACKWARD
	kCommandMoveLeft = 6, // @ C006_COMMAND_MOVE_LEFT
	kCommandToggleInventoryChampion_0 = 7, // @ C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0
	kCommandToggleInventoryChampion_1 = 8, // @ C008_COMMAND_TOGGLE_INVENTORY_CHAMPION_1
	kCommandToggleInventoryChampion_2 = 9, // @ C009_COMMAND_TOGGLE_INVENTORY_CHAMPION_2
	kCommandToggleInventoryChampion_3 = 10, // @ C010_COMMAND_TOGGLE_INVENTORY_CHAMPION_3
	kCommandCloseInventory = 11, // @ C011_COMMAND_CLOSE_INVENTORY
	kCommandClickInChampion_0_StatusBox = 12, // @ C012_COMMAND_CLICK_IN_CHAMPION_0_STATUS_BOX
	kCommandClickInChampion_1_StatusBox = 13, // @ C013_COMMAND_CLICK_IN_CHAMPION_1_STATUS_BOX
	kCommandClickInChampion_2_StatusBox = 14, // @ C014_COMMAND_CLICK_IN_CHAMPION_2_STATUS_BOX
	kCommandClickInChampion_3_StatusBox = 15, // @ C015_COMMAND_CLICK_IN_CHAMPION_3_STATUS_BOX
	kCommandSetLeaderChampion_0 = 16, // @ C016_COMMAND_SET_LEADER_CHAMPION_0
	kCommandSetLeaderChampion_1 = 17, // @ C017_COMMAND_SET_LEADER_CHAMPION_1
	kCommandSetLeaderChampion_2 = 18, // @ C018_COMMAND_SET_LEADER_CHAMPION_2
	kCommandSetLeaderChampion_3 = 19, // @ C019_COMMAND_SET_LEADER_CHAMPION_3
	kCommandClickOnSlotBoxChampion_0_StatusBoxReadyHand = 20, // @ C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND
	kCommandClickOnSlotBoxChampion_0_StatusBoxActionHand = 21, // @ C021_COMMAND_CLICK_ON_SLOT_BOX_01_CHAMPION_0_STATUS_BOX_ACTION_HAND
	kCommandClickOnSlotBoxChampion_1_StatusBoxReadyHand = 22, // @ C022_COMMAND_CLICK_ON_SLOT_BOX_02_CHAMPION_1_STATUS_BOX_READY_HAND
	kCommandClickOnSlotBoxChampion_1_StatusBoxActionHand = 23, // @ C023_COMMAND_CLICK_ON_SLOT_BOX_03_CHAMPION_1_STATUS_BOX_ACTION_HAND
	kCommandClickOnSlotBoxChampion_2_StatusBoxReadyHand = 24, // @ C024_COMMAND_CLICK_ON_SLOT_BOX_04_CHAMPION_2_STATUS_BOX_READY_HAND
	kCommandClickOnSlotBoxChampion_2_StatusBoxActionHand = 25, // @ C025_COMMAND_CLICK_ON_SLOT_BOX_05_CHAMPION_2_STATUS_BOX_ACTION_HAND
	kCommandClickOnSlotBoxChampion_3_StatusBoxReadyHand = 26, // @ C026_COMMAND_CLICK_ON_SLOT_BOX_06_CHAMPION_3_STATUS_BOX_READY_HAND
	kCommandClickOnSlotBoxChampion_3_StatusBoxActionHand = 27, // @ C027_COMMAND_CLICK_ON_SLOT_BOX_07_CHAMPION_3_STATUS_BOX_ACTION_HAND
	kCommandClickOnSlotBoxInventoryReadyHand = 28, // @ C028_COMMAND_CLICK_ON_SLOT_BOX_08_INVENTORY_READY_HAND
	kCommandClickOnSlotBoxInventoryActionHand = 29, // @ C029_COMMAND_CLICK_ON_SLOT_BOX_09_INVENTORY_ACTION_HAND
	kCommandClickOnSlotBoxInventoryHead = 30, // @ C030_COMMAND_CLICK_ON_SLOT_BOX_10_INVENTORY_HEAD
	kCommandClickOnSlotBoxInventoryTorso = 31, // @ C031_COMMAND_CLICK_ON_SLOT_BOX_11_INVENTORY_TORSO
	kCommandClickOnSlotBoxInventoryLegs = 32, // @ C032_COMMAND_CLICK_ON_SLOT_BOX_12_INVENTORY_LEGS
	kCommandClickOnSlotBoxInventoryFeet = 33, // @ C033_COMMAND_CLICK_ON_SLOT_BOX_13_INVENTORY_FEET
	kCommandClickOnSlotBoxInventoryPouch_2 = 34, // @ C034_COMMAND_CLICK_ON_SLOT_BOX_14_INVENTORY_POUCH_2
	kCommandClickOnSlotBoxInventoryQuiverLine_2_1 = 35, // @ C035_COMMAND_CLICK_ON_SLOT_BOX_15_INVENTORY_QUIVER_LINE2_1
	kCommandClickOnSlotBoxInventoryQuiverLine_1_2 = 36, // @ C036_COMMAND_CLICK_ON_SLOT_BOX_16_INVENTORY_QUIVER_LINE1_2
	kCommandClickOnSlotBoxInventoryQuiverLine_2_2 = 37, // @ C037_COMMAND_CLICK_ON_SLOT_BOX_17_INVENTORY_QUIVER_LINE2_2
	kCommandClickOnSlotBoxInventoryNeck = 38, // @ C038_COMMAND_CLICK_ON_SLOT_BOX_18_INVENTORY_NECK
	kCommandClickOnSlotBoxInventoryPouch_1 = 39, // @ C039_COMMAND_CLICK_ON_SLOT_BOX_19_INVENTORY_POUCH_1
	kCommandClickOnSlotBoxInventoryQuiverLine_1_1 = 40, // @ C040_COMMAND_CLICK_ON_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1
	kCommandClickOnSlotBoxInventoryBackpackLine_1_1 = 41, // @ C041_COMMAND_CLICK_ON_SLOT_BOX_21_INVENTORY_BACKPACK_LINE1_1
	kCommandClickOnSlotBoxInventoryBackpackLine_2_2 = 42, // @ C042_COMMAND_CLICK_ON_SLOT_BOX_22_INVENTORY_BACKPACK_LINE2_2
	kCommandClickOnSlotBoxInventoryBackpackLine_2_3 = 43, // @ C043_COMMAND_CLICK_ON_SLOT_BOX_23_INVENTORY_BACKPACK_LINE2_3
	kCommandClickOnSlotBoxInventoryBackpackLine_2_4 = 44, // @ C044_COMMAND_CLICK_ON_SLOT_BOX_24_INVENTORY_BACKPACK_LINE2_4
	kCommandClickOnSlotBoxInventoryBackpackLine_2_5 = 45, // @ C045_COMMAND_CLICK_ON_SLOT_BOX_25_INVENTORY_BACKPACK_LINE2_5
	kCommandClickOnSlotBoxInventoryBackpackLine_2_6 = 46, // @ C046_COMMAND_CLICK_ON_SLOT_BOX_26_INVENTORY_BACKPACK_LINE2_6
	kCommandClickOnSlotBoxInventoryBackpackLine_2_7 = 47, // @ C047_COMMAND_CLICK_ON_SLOT_BOX_27_INVENTORY_BACKPACK_LINE2_7
	kCommandClickOnSlotBoxInventoryBackpackLine_2_8 = 48, // @ C048_COMMAND_CLICK_ON_SLOT_BOX_28_INVENTORY_BACKPACK_LINE2_8
	kCommandClickOnSlotBoxInventoryBackpackLine_2_9 = 49, // @ C049_COMMAND_CLICK_ON_SLOT_BOX_29_INVENTORY_BACKPACK_LINE2_9
	kCommandClickOnSlotBoxInventoryBackpackLine_1_2 = 50, // @ C050_COMMAND_CLICK_ON_SLOT_BOX_30_INVENTORY_BACKPACK_LINE1_2
	kCommandClickOnSlotBoxInventoryBackpackLine_1_3 = 51, // @ C051_COMMAND_CLICK_ON_SLOT_BOX_31_INVENTORY_BACKPACK_LINE1_3
	kCommandClickOnSlotBoxInventoryBackpackLine_1_4 = 52, // @ C052_COMMAND_CLICK_ON_SLOT_BOX_32_INVENTORY_BACKPACK_LINE1_4
	kCommandClickOnSlotBoxInventoryBackpackLine_1_5 = 53, // @ C053_COMMAND_CLICK_ON_SLOT_BOX_33_INVENTORY_BACKPACK_LINE1_5
	kCommandClickOnSlotBoxInventoryBackpackLine_1_6 = 54, // @ C054_COMMAND_CLICK_ON_SLOT_BOX_34_INVENTORY_BACKPACK_LINE1_6
	kCommandClickOnSlotBoxInventoryBackpackLine_1_7 = 55, // @ C055_COMMAND_CLICK_ON_SLOT_BOX_35_INVENTORY_BACKPACK_LINE1_7
	kCommandClickOnSlotBoxInventoryBackpackLine_1_8 = 56, // @ C056_COMMAND_CLICK_ON_SLOT_BOX_36_INVENTORY_BACKPACK_LINE1_8
	kCommandClickOnSlotBoxInventoryBackpackLine_1_9 = 57, // @ C057_COMMAND_CLICK_ON_SLOT_BOX_37_INVENTORY_BACKPACK_LINE1_9
	kCommandClickOnSlotBoxChest_1 = 58, // @ C058_COMMAND_CLICK_ON_SLOT_BOX_38_CHEST_1
	kCommandClickOnSlotBoxChest_2 = 59, // @ C059_COMMAND_CLICK_ON_SLOT_BOX_39_CHEST_2
	kCommandClickOnSlotBoxChest_3 = 60, // @ C060_COMMAND_CLICK_ON_SLOT_BOX_40_CHEST_3
	kCommandClickOnSlotBoxChest_4 = 61, // @ C061_COMMAND_CLICK_ON_SLOT_BOX_41_CHEST_4
	kCommandClickOnSlotBoxChest_5 = 62, // @ C062_COMMAND_CLICK_ON_SLOT_BOX_42_CHEST_5
	kCommandClickOnSlotBoxChest_6 = 63, // @ C063_COMMAND_CLICK_ON_SLOT_BOX_43_CHEST_6
	kCommandClickOnSlotBoxChest_7 = 64, // @ C064_COMMAND_CLICK_ON_SLOT_BOX_44_CHEST_7
	kCommandClickOnSlotBoxChest_8 = 65, // @ C065_COMMAND_CLICK_ON_SLOT_BOX_45_CHEST_8
	kCommandClickOnMouth = 70, // @ C070_COMMAND_CLICK_ON_MOUTH
	kCommandClickOnEye = 71, // @ C071_COMMAND_CLICK_ON_EYE
	kCommandClickInDungeonView = 80, // @ C080_COMMAND_CLICK_IN_DUNGEON_VIEW
	kCommandClickInPanel = 81, // @ C081_COMMAND_CLICK_IN_PANEL
	kCommandToggleInventoryLeader = 83, // @ C083_COMMAND_TOGGLE_INVENTORY_LEADER
	kCommandClickInSpellArea = 100, // @ C100_COMMAND_CLICK_IN_SPELL_AREA
	kCommandClickInSpellAreaSymbol_1 = 101, // @ C101_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_1
	kCommandClickInSpellAreaSymbol_2 = 102, // @ C102_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_2
	kCommandClickInSpellAreaSymbol_3 = 103, // @ C103_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_3
	kCommandClickInSpellAreaSymbol_4 = 104, // @ C104_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_4
	kCommandClickInSpellAreaSymbol_5 = 105, // @ C105_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_5
	kCommandClickInSpellAreaSymbol_6 = 106, // @ C106_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_6
	kCommandClickInSpellAreaRecantSymbol = 107, // @ C107_COMMAND_CLICK_IN_SPELL_AREA_RECANT_SYMBOL
	kCommandClickInSpeallAreaCastSpell = 108, // @ C108_COMMAND_CLICK_IN_SPELL_AREA_CAST_SPELL
	kCommandClickInActionArea = 111, // @ C111_COMMAND_CLICK_IN_ACTION_AREA
	kCommandClickInActionAreaPass = 112, // @ C112_COMMAND_CLICK_IN_ACTION_AREA_PASS
	kCommandClickInActionAreaAction_0 = 113, // @ C113_COMMAND_CLICK_IN_ACTION_AREA_ACTION_0
	kCommandClickInActionAreaAction_1 = 114, // @ C114_COMMAND_CLICK_IN_ACTION_AREA_ACTION_1
	kCommandClickInActionAreaAction_2 = 115, // @ C115_COMMAND_CLICK_IN_ACTION_AREA_ACTION_2
	kCommandClickInActionAreaChampion_0_Action = 116, // @ C116_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_0_ACTION
	kCommandClickInActionAreaChampion_1_Action = 117, // @ C117_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_1_ACTION
	kCommandClickInActionAreaChampion_2_Action = 118, // @ C118_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_2_ACTION
	kCommandClickInActionAreaChampion_3_Action = 119, // @ C119_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_3_ACTION
	kCommandClickOnChamptionIcon_Top_Left = 125, // @ C125_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_LEFT
	kCommandClickOnChamptionIcon_Top_Right = 126, // @ C126_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_RIGHT
	kCommandClickOnChamptionIcon_Lower_Right = 127, // @ C127_COMMAND_CLICK_ON_CHAMPION_ICON_LOWER_RIGHT
	kCommandClickOnChamptionIcon_Lower_Left = 128, // @ C128_COMMAND_CLICK_ON_CHAMPION_ICON_LOWER_LEFT
	kCommandSaveGame = 140, // @ C140_COMMAND_SAVE_GAME
	kCommandSleep = 145, // @ C145_COMMAND_SLEEP
	kCommandWakeUp = 146, // @ C146_COMMAND_WAKE_UP
	kCommandFreezeGame = 147, // @ C147_COMMAND_FREEZE_GAME
	kCommandUnfreezeGame = 148, // @ C148_COMMAND_UNFREEZE_GAME
	kCommandClickInPanelResurrect = 160, // @ C160_COMMAND_CLICK_IN_PANEL_RESURRECT
	kCommandClickInPanelReincarnate = 161, // @ C161_COMMAND_CLICK_IN_PANEL_REINCARNATE
	kCommandClickInPanelCancel = 162, // @ C162_COMMAND_CLICK_IN_PANEL_CANCEL
	kCommandEntranceEnterDungeon = 200, // @ C200_COMMAND_ENTRANCE_ENTER_DUNGEON
	kCommandEntranceResume = 201, // @ C201_COMMAND_ENTRANCE_RESUME /* Versions 1.x and 2.x command */
	kCommandEntranceDrawCredits = 202, // @ C202_COMMAND_ENTRANCE_DRAW_CREDITS /* Versions 1.x and 2.x command */
	kCommandClickOnDialogChoice_1 = 210, // @ C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1
	kCommandClickOnDialogChoice_2 = 211, // @ C211_COMMAND_CLICK_ON_DIALOG_CHOICE_2
	kCommandClickOnDialogChoice_3 = 212, // @ C212_COMMAND_CLICK_ON_DIALOG_CHOICE_3
	kCommandClickOnDialogChoice_4 = 213, // @ C213_COMMAND_CLICK_ON_DIALOG_CHOICE_4
	kCommandRestartGame = 215 // @ C215_COMMAND_RESTART_GAME
}; // @ NONE

class Command {
public:
	Common::Point pos;
	CommandType type;

	Command(Common::Point position, CommandType commandType): pos(position), type(type) {}
}; // @ COMMAND


class MouseInput {
public:
	CommandType commandTypeToIssue;
	Box hitbox;
	MouseButton button;

	MouseInput(CommandType type, uint16 x1, uint16 x2, uint16 y1, uint16 y2, MouseButton mouseButton)
		: commandTypeToIssue(type), hitbox(x1, x2 + 1, y1, y2 + 1), button(mouseButton) {}
}; // @ MOUSE_INPUT

extern MouseInput gPrimaryMouseInput_Entrance[4]; // @ G0445_as_Graphic561_PrimaryMouseInput_Entrance[4]
extern MouseInput gPrimaryMouseInput_RestartGame[2]; // @ G0446_as_Graphic561_PrimaryMouseInput_RestartGame[2]
extern MouseInput gPrimaryMouseInput_Interface[20]; // @ G0447_as_Graphic561_PrimaryMouseInput_Interface[20]
extern MouseInput gSecondaryMouseInput_Movement[9]; // @ G0448_as_Graphic561_SecondaryMouseInput_Movement[9]
extern MouseInput gSecondaryMouseInput_ChampionInventory[38]; // @ G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory[38]
extern MouseInput gPrimaryMouseInput_PartySleeping[3]; // @ G0450_as_Graphic561_PrimaryMouseInput_PartySleeping[3]
extern MouseInput gPrimaryMouseInput_FrozenGame[3]; // @ G0451_as_Graphic561_PrimaryMouseInput_FrozenGame[3]
extern MouseInput gMouseInput_ActionAreaNames[5]; // @ G0452_as_Graphic561_MouseInput_ActionAreaNames[5]
extern MouseInput gMouseInput_ActionAreaIcons[5]; // @ G0453_as_Graphic561_MouseInput_ActionAreaIcons[5]
extern MouseInput gMouseInput_SpellArea[9]; // @ G0454_as_Graphic561_MouseInput_SpellArea[9]
extern MouseInput gMouseInput_ChampionNamesHands[13]; // @ G0455_as_Graphic561_MouseInput_ChampionNamesHands[13]
extern MouseInput gMouseInput_PanelChest[9]; // @ G0456_as_Graphic561_MouseInput_PanelChest[9]
extern MouseInput gMouseInput_PanelResurrectReincarnateCancel[4]; // @ G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel[4]
extern MouseInput gPrimaryMouseInput_ViewportDialog1Choice[2]; // @ G0471_as_Graphic561_PrimaryMouseInput_ViewportDialog1Choice[2]
extern MouseInput gPrimaryMouseInput_ViewportDialog2Choices[3]; // @ G0472_as_Graphic561_PrimaryMouseInput_ViewportDialog2Choices[3]
extern MouseInput gPrimaryMouseInput_ViewportDialog3Choices[4]; // @ G0473_as_Graphic561_PrimaryMouseInput_ViewportDialog3Choices[4]
extern MouseInput gPrimaryMouseInput_ViewportDialog4Choices[5]; // @ G0474_as_Graphic561_PrimaryMouseInput_ViewportDialog4Choices[5]
extern MouseInput gPrimaryMouseInput_ScreenDialog1Choice[2]; // @ G0475_as_Graphic561_PrimaryMouseInput_ScreenDialog1Choice[2]
extern MouseInput gPrimaryMouseInput_ScreenDialog2Choices[3]; // @ G0476_as_Graphic561_PrimaryMouseInput_ScreenDialog2Choices[3]
extern MouseInput gPrimaryMouseInput_ScreenDialog3Choices[4]; // @ G0477_as_Graphic561_PrimaryMouseInput_ScreenDialog3Choices[4]
extern MouseInput gPrimaryMouseInput_ScreenDialog4Choices[5]; // @ G0478_as_Graphic561_PrimaryMouseInput_ScreenDialog4Choices[5]
extern MouseInput* gPrimaryMouseInput_DialogSets[2][4]; // @ G0480_aaps_PrimaryMouseInput_DialogSets

class KeyboardInput {
public:
	Command commandToIssue;
	Common::KeyCode key;
	byte modifiers;

	KeyboardInput(Command command, Common::KeyCode keycode, byte modifierFlags) : commandToIssue(command), key(keycode), modifiers(modifierFlags) {}
}; // @ KEYBOARD_INPUT

class DMEngine;

class EventManager {
	DMEngine *_vm;

	Common::Point _mousePos;
	uint16 _dummyMapIndex;

	bool _pendingClickPresent; // G0436_B_PendingClickPresent
	Common::Point _pendingClickPos; // @ G0437_i_PendingClickX, G0438_i_PendingClickY
	MouseButton _pendingClickButton; // @ G0439_i_PendingClickButtonsStatus

	bool _isCommandQueueLocked;
	Common::Queue<Command> _commandQueue;
public:
	MouseInput* _primaryMouseInput;// @ G0441_ps_PrimaryMouseInput
	MouseInput* _secondaryMouseInput;// @ G0442_ps_SecondaryMouseInput

	EventManager(DMEngine *vm);
	void initMouse();
	void showMouse(bool visibility);

	void setMousePos(Common::Point pos);
	void processInput();
	void processPendingClick(); // @ F0360_COMMAND_ProcessPendingClick
	void processClick(Common::Point mousePos, MouseButton button); // @	F0359_COMMAND_ProcessClick_CPSC
	CommandType getCommandTypeFromMouseInput(MouseInput *input, Common::Point mousePos, MouseButton button); // @ F0358_COMMAND_GetCommandFromMouseInput_CPSC
};

}


#endif
