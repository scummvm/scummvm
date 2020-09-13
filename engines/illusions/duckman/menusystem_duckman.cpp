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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/illusions.h"
#include "illusions/actor.h"
#include "illusions/sound.h"
#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/menusystem_duckman.h"
#include "illusions/resources/scriptresource.h"

namespace Illusions {

// DuckmanMenuSystem

DuckmanMenuSystem::DuckmanMenuSystem(IllusionsEngine_Duckman *vm)
	: BaseMenuSystem(vm), _vm(vm) {
	clearMenus();
}

DuckmanMenuSystem::~DuckmanMenuSystem() {
	freeMenus();
}

void DuckmanMenuSystem::runMenu(MenuChoiceOffsets menuChoiceOffsets, int16 *menuChoiceOffset,
	uint32 menuId, uint32 duration, uint timeOutMenuChoiceIndex, uint32 menuCallerThreadId) {

	debug(0, "DuckmanMenuSystem::runMenu(%08X)", menuId);

	setTimeOutDuration(duration, timeOutMenuChoiceIndex);
	setMenuCallerThreadId(menuCallerThreadId);
	setMenuChoiceOffsets(menuChoiceOffsets, menuChoiceOffset);

	int rootMenuId = convertRootMenuId(menuId | 0x180000);
	BaseMenu *rootMenu = getMenuById(rootMenuId);
	openMenu(rootMenu);

}

void DuckmanMenuSystem::clearMenus() {
	for (int i = 0; i < kDuckmanLastMenuIndex; ++i) {
		_menus[i] = 0;
	}
}

void DuckmanMenuSystem::freeMenus() {
	for (int i = 0; i < kDuckmanLastMenuIndex; ++i) {
		delete _menus[i];
	}
}

BaseMenu *DuckmanMenuSystem::getMenuById(int menuId) {
	if (!_menus[menuId])
		_menus[menuId] = createMenuById(menuId);
	return _menus[menuId];
}

BaseMenu *DuckmanMenuSystem::createMenuById(int menuId) {
	switch (menuId) {
	case kDuckmanMainMenu:
		return createMainMenu();
	case kDuckmanMainMenuDemo:
		return createMainMenuDemo();
	case kDuckmanPauseMenu:
		return createPauseMenu();
	case kDuckmanQueryRestartMenu:
		return createQueryRestartMenu();
	case kDuckmanQueryQuitMenu:
		return createQueryQuitMenu();
	case kDuckmanSaveCompleteMenu:
		return createSaveCompleteMenu();
	case kDuckmanOptionsMenu:
		return createOptionsMenu();
	case kDuckmanDebugPauseMenu:
		return createDebugPauseMenu();
	case kDuckmanAddRemoveInventoryMenu:
		return createAddRemoveInventoryMenu();
	case kDuckmanLoadGameFailedMenu:
		return createLoadGameFailedMenu();
	default:
		error("DuckmanMenuSystem::createMenuById() Invalid menu id %d", menuId);
	}
}

BaseMenu *DuckmanMenuSystem::createMainMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 0);

	if (_vm->getGameLanguage() != Common::RU_RUS) {
		menu->addMenuItem(new MenuItem("Start New Game", new MenuActionReturnChoice(this, 11)));
		menu->addMenuItem(new MenuItem("Load Saved Game", new MenuActionLoadGame(this, 1)));
		menu->addMenuItem(new MenuItem("Options", new MenuActionEnterMenu(this, kDuckmanOptionsMenu)));
		menu->addMenuItem(new MenuItem("Quit Game", new MenuActionEnterQueryMenu(this, kDuckmanQueryQuitMenu, 12)));
	} else {
		menu->addMenuItem(new MenuItem("3AHOBO      ", new MenuActionReturnChoice(this, 11)));
		menu->addMenuItem(new MenuItem("B6IHECEM ", new MenuActionLoadGame(this, 1)));
		menu->addMenuItem(new MenuItem("YCTAH .", new MenuActionEnterMenu(this, kDuckmanOptionsMenu)));
		menu->addMenuItem(new MenuItem("B6IXOD   ", new MenuActionEnterQueryMenu(this, kDuckmanQueryQuitMenu, 12)));
	}

	return menu;
}

BaseMenu *DuckmanMenuSystem::createMainMenuDemo() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 0);
	if (_vm->getGameLanguage() != Common::RU_RUS) {
		menu->addMenuItem(new MenuItem("Start New Game", new MenuActionReturnChoice(this, 2)));
		menu->addMenuItem(new MenuItem("Load Saved Game", new MenuActionLoadGame(this, 1)));
		menu->addMenuItem(new MenuItem("Options", new MenuActionEnterMenu(this, kDuckmanOptionsMenu)));
		menu->addMenuItem(new MenuItem("Quit Game", new MenuActionEnterQueryMenu(this, kDuckmanQueryQuitMenu, 3)));
	} else {
		menu->addMenuItem(new MenuItem("3AHOBO      ", new MenuActionReturnChoice(this, 2)));
		menu->addMenuItem(new MenuItem("B6IHECEM ", new MenuActionLoadGame(this, 1)));
		menu->addMenuItem(new MenuItem("YCTAH .", new MenuActionEnterMenu(this, kDuckmanOptionsMenu)));
		menu->addMenuItem(new MenuItem("B6IXOD   ", new MenuActionEnterQueryMenu(this, kDuckmanQueryQuitMenu, 3)));
	}
	return menu;
}

BaseMenu *DuckmanMenuSystem::createLoadGameMenu() {
	return 0; // TODO
}

BaseMenu *DuckmanMenuSystem::createLoadGameFailedMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 0);
	menu->addText("Load Game Failed");
	menu->addText("-------------------");
	menu->addMenuItem(new MenuItem("Continue", new MenuActionReturnChoice(this, 1)));
	return menu;
}


MenuItem *DuckmanMenuSystem::createOptionsSliderMenuItem(MenuActionUpdateSlider **action, const Common::String &text, SliderActionType type, BaseMenu *baseMenu) {
	int sliderValue = 0;
	char sliderText[] = "{~~~~~~~~~~~~~~~~}";
	switch (type) {
		case SFX : sliderValue = _vm->_soundMan->getSfxVolume()/(256/15); break;
		case MUSIC : sliderValue = _vm->_soundMan->getMusicVolume()/(256/15); break;
		case VOICE : sliderValue = _vm->_soundMan->getSpeechVolume()/(256/15); break;
		case TEXT_DURATION : sliderValue = _vm->getSubtitleDuration()/(256/15); break;
		default: break;
	}

	sliderText[sliderValue + 1] = '|';

	*action = new MenuActionUpdateSlider(this, baseMenu, type, _vm);
	MenuItem *menuItem = new MenuItem(text + sliderText, *action);
	(*action)->setMenuItem(menuItem);
	return menuItem;
}

BaseMenu *DuckmanMenuSystem::createOptionsMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 6);
	MenuActionUpdateSlider *sfxSlider;
	MenuActionUpdateSlider *musicSlider;
	MenuActionUpdateSlider *speechSlider;
	MenuActionUpdateSlider *textDurationSlider;

	if (_vm->getGameLanguage() != Common::RU_RUS) {
		menu->addText("              GAME OPTIONS             @@@@");
		menu->addText("--------------------------------------");

		menu->addMenuItem(createOptionsSliderMenuItem(&sfxSlider, "SFX Volume     @@", SFX, menu));
		menu->addMenuItem(createOptionsSliderMenuItem(&musicSlider, "Music Volume  @@@", MUSIC, menu));
		menu->addMenuItem(createOptionsSliderMenuItem(&speechSlider, "Speech Volume ", VOICE, menu));
		menu->addMenuItem(createOptionsSliderMenuItem(&textDurationSlider, "Text Duration @@@", TEXT_DURATION, menu));

		menu->addMenuItem(new MenuItem("Restore Defaults", new MenuActionResetOptionSliders(this, sfxSlider, musicSlider, speechSlider, textDurationSlider)));

		menu->addMenuItem(new MenuItem("Back", new MenuActionLeaveMenu(this)));
	} else {
		menu->addText("              YCTAHOBKA   ");
		menu->addText("--------------------------------------");

		menu->addMenuItem(createOptionsSliderMenuItem(&sfxSlider, "3BYK           @@", SFX, menu));
		menu->addMenuItem(createOptionsSliderMenuItem(&musicSlider, "MY36IKA       @@@", MUSIC, menu));
		menu->addMenuItem(createOptionsSliderMenuItem(&speechSlider, "6A3AP         ", VOICE, menu));
		menu->addMenuItem(createOptionsSliderMenuItem(&textDurationSlider, "TEKCT         @@@", TEXT_DURATION, menu));

		menu->addMenuItem(new MenuItem("Restore Defaults", new MenuActionResetOptionSliders(this, sfxSlider, musicSlider, speechSlider, textDurationSlider)));

		menu->addMenuItem(new MenuItem("Back", new MenuActionLeaveMenu(this)));
	}
	return menu;
}

BaseMenu *DuckmanMenuSystem::createPauseMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 1);
	if (_vm->getGameLanguage() != Common::RU_RUS) {
		menu->addText("   Game Paused");
		menu->addText("--------------------");
		menu->addMenuItem(new MenuItem("Resume", new MenuActionReturnChoice(this, 21)));
		menu->addMenuItem(new MenuItem("Load Game", new MenuActionLoadGame(this, 1)));
		menu->addMenuItem(new MenuItem("Save Game", new MenuActionSaveGame(this, 11)));
		menu->addMenuItem(new MenuItem("Restart Game", new MenuActionEnterQueryMenu(this, kDuckmanQueryRestartMenu, 2)));
		menu->addMenuItem(new MenuItem("Options", new MenuActionEnterMenu(this, kDuckmanOptionsMenu)));
		menu->addMenuItem(new MenuItem("Quit Game", new MenuActionEnterQueryMenu(this, kDuckmanQueryQuitMenu, 23)));
	} else {
		menu->addText("   OCTAHOBKA");
		menu->addText("--------------------");
		menu->addMenuItem(new MenuItem("YXHEM ", new MenuActionReturnChoice(this, 21)));
		menu->addMenuItem(new MenuItem("B6IHECEM ", new MenuActionLoadGame(this, 1)));
		menu->addMenuItem(new MenuItem("BHECEM   ", new MenuActionSaveGame(this, 11)));
		menu->addMenuItem(new MenuItem("3AHOBO      ", new MenuActionEnterQueryMenu(this, kDuckmanQueryRestartMenu, 2)));
		menu->addMenuItem(new MenuItem("YCTAH .", new MenuActionEnterMenu(this, kDuckmanOptionsMenu)));
		menu->addMenuItem(new MenuItem("B6IXOD   ", new MenuActionEnterQueryMenu(this, kDuckmanQueryQuitMenu, 23)));
	}
	return menu;
}

BaseMenu *DuckmanMenuSystem::createQueryRestartMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 2);
	if (_vm->getGameLanguage() != Common::RU_RUS) {
		menu->addText("Do you really want to restart?");
		menu->addText("-----------------------------------");
		menu->addMenuItem(new MenuItem("Yes, let's try again", new MenuActionReturnChoice(this, getQueryConfirmationChoiceIndex())));
		menu->addMenuItem(new MenuItem("No, just kidding", new MenuActionLeaveMenu(this)));
	} else {
		menu->addText("TO4HO  3AHOBO  ?              ");
		menu->addText("-----------------------------------");
		menu->addMenuItem(new MenuItem("DA , ECTECTBEHHO   ", new MenuActionReturnChoice(this, getQueryConfirmationChoiceIndex())));
		menu->addMenuItem(new MenuItem("HET , ODHO3HA4HO", new MenuActionLeaveMenu(this)));
	}
	return menu;
}

BaseMenu *DuckmanMenuSystem::createQueryQuitMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 2);
	if (_vm->getGameLanguage() != Common::RU_RUS) {
		menu->addText("Do you really want to quit?");
		menu->addText("-------------------------------");
		menu->addMenuItem(new MenuItem("Yes, I'm outta here", new MenuActionReturnChoice(this, getQueryConfirmationChoiceIndex())));
		menu->addMenuItem(new MenuItem("No, just kidding", new MenuActionLeaveMenu(this)));
	} else {
		menu->addText("TO4HO  HA  B6IXOD  ?       ");
		menu->addText("-------------------------------");
		menu->addMenuItem(new MenuItem("DA , ECTECTBEHHO   ", new MenuActionReturnChoice(this, getQueryConfirmationChoiceIndex())));
		menu->addMenuItem(new MenuItem("HET , ODHO3HA4HO", new MenuActionLeaveMenu(this)));
	}

	return menu;
}

BaseMenu *DuckmanMenuSystem::createSaveCompleteMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 1);
	menu->addText("Game Saved");
	menu->addText("-------------");
	menu->addMenuItem(new MenuItem("Continue", new MenuActionReturnChoice(this, 1)));
	return menu;
}

BaseMenu *DuckmanMenuSystem::createDebugMenu() {
	// TODO
	BaseMenu *menu = new BaseMenu(this, 0x00120002, 0, 0, 0, 17, 1);
	menu->addText("Debug Menu");
	menu->addText("-----------------");
	return menu;
}

BaseMenu *DuckmanMenuSystem::createDebugPauseMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120002, 0, 0, 0, 17, 1);
	menu->addText("Debug Pause Menu");
	menu->addText("-----------------");
	menu->addMenuItem(new MenuItem("Return to Game", new MenuActionReturnChoice(this, 1)));
	menu->addMenuItem(new MenuItem("Add/Remove Inventory", new MenuActionEnterMenu(this, kDuckmanAddRemoveInventoryMenu)));
	// TODO quit to debug menu.
	return menu;
}

typedef struct InventoryMenuItem {
	const char *name;
	uint32 objectId;
	uint32 sequenceId;
	uint32 propertyId;
} InventoryMenuItem;

static const InventoryMenuItem kDebugInventoryItems[21] = {
	{ "Pick-up Book", 262212, 393231, 917519 },
	{ "Bucket and Squeegee", 262314, 393233, 917599 },
	{ "Cardboard Cut Out", 262219, 393264, 917573 },
	{ "Talking Doll", 262209, 393943, 917587 },
	{ "Cookie Fortunes", 262263, 393266, 917520 },
	{ "Garbage Can Lid", 262311, 393259, 917597 },
	{ "Chewing Gum", 262210, 393267, 917522 },
	{ "Ladder", 262155, 393258, 917598 },
	{ "Disco Light", 262342, 393260, 917594 },
	{ "Magazine Cover", 262185, 393261, 917517 },
	{ "Matches", 262159, 393232, 917516 },
	{ "Opera Lessons", 262293, 393731, 917600 },
	{ "Pizza Card", 262239, 393262, 917526 },
	{ "Toilet Plunger", 262282, 393257, 917555 },
	{ "Black Velvet Poster", 262258, 393269, 917527 },
	{ "Red Spray Paint", 262297, 393254, 917531 },
	{ "Remote Control", 262161, 393255, 917595 },
	{ "Sparkplug", 262294, 393256, 917532 },
	{ "Tape Recorder", 262328, 393827, 917584 },
	{ "Wacky Putty", 262228, 393559, 917537 },
	{ "Wrench", 262175, 393422, 917530 }
};

BaseMenu *DuckmanMenuSystem::createAddRemoveInventoryMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120002, 0, 0, 0, 17, 1);
	menu->addText("Add/Remove Inventory");
	menu->addText("-----------------");
	for (int i = 0; i < 21; i++) {
		menu->addMenuItem(new MenuItem(kDebugInventoryItems[i].name, new MenuActionInventoryAddRemove(this, _vm, i)));
	}
	menu->addMenuItem(new MenuItem("Back", new MenuActionLeaveMenu(this)));
	return menu;
}

int DuckmanMenuSystem::convertRootMenuId(uint32 menuId) {
	switch (menuId) {
	case 0x180001:
		return kDuckmanMainMenu;
	case 0x180002:
		return kDuckmanPauseMenu;
	case 0x180003:
		return kDuckmanDebugMenu;
	case 0x180004:
		return kDuckmanDebugPauseMenu;
	case 0x180005:
		return kDuckmanSaveCompleteMenu;
	/*
	case 0x180006: // save game failed menu
	*/
	case 0x180007: // load game failed menu
		return kDuckmanLoadGameFailedMenu;

	case 0x180008:
		return kDuckmanMainMenuDemo;

	default:
		error("DuckmanMenuSystem() Menu ID %08X not found", menuId);
	}
}

bool DuckmanMenuSystem::initMenuCursor() {
	bool cursorInitialVisibleFlag = false;
	Control *cursorControl = _vm->getObjectControl(Illusions::CURSOR_OBJECT_ID);
	if (cursorControl) {
		if (cursorControl->_flags & 1)
			cursorInitialVisibleFlag = false;
		cursorControl->appearActor();
	} else {
		Common::Point pos = _vm->getNamedPointPosition(0x70001);
		_vm->_controls->placeActor(0x50001, pos, 0x60001, Illusions::CURSOR_OBJECT_ID, 0);
		cursorControl = _vm->getObjectControl(Illusions::CURSOR_OBJECT_ID);
	}
	return cursorInitialVisibleFlag;
}

int DuckmanMenuSystem::getGameState() {
	return _vm->_cursor._gameState;
}

void DuckmanMenuSystem::setMenuCursorNum(int cursorNum) {
	Control *mouseCursor = _vm->getObjectControl(Illusions::CURSOR_OBJECT_ID);
	_vm->setCursorActorIndex(5, cursorNum, 0);
	mouseCursor->startSequenceActor(0x60001, 2, 0);
}

void DuckmanMenuSystem::setGameState(int gameState) {
	_vm->_cursor._gameState = gameState;
}

void DuckmanMenuSystem::playSoundEffect(int sfxId) {
	_vm->playSoundEffect(sfxId);
}

MenuActionInventoryAddRemove::MenuActionInventoryAddRemove(BaseMenuSystem *menuSystem, IllusionsEngine_Duckman *vm, uint choiceIndex)
	: BaseMenuAction(menuSystem), _choiceIndex(choiceIndex), _vm(vm) {
}

void MenuActionInventoryAddRemove::execute() {
	if (_vm->_scriptResource->_properties.get(kDebugInventoryItems[_choiceIndex].propertyId)) {
		if (_vm->_cursor._objectId == kDebugInventoryItems[_choiceIndex].objectId) {
			_vm->stopCursorHoldingObject();
		}
		_vm->_scriptResource->_properties.set(kDebugInventoryItems[_choiceIndex].propertyId, false);
	} else {
		_vm->startCursorHoldingObject(kDebugInventoryItems[_choiceIndex].objectId,
									  kDebugInventoryItems[_choiceIndex].sequenceId);
		_vm->_scriptResource->_properties.set(kDebugInventoryItems[_choiceIndex].propertyId, true);
	}
	_menuSystem->leaveMenu();
}

MenuActionUpdateSlider::MenuActionUpdateSlider(BaseMenuSystem *menuSystem, BaseMenu *baseMenu, SliderActionType type, IllusionsEngine_Duckman *vm)
			: BaseMenuAction(menuSystem), menu(baseMenu), _type(type), _vm(vm) {
	_menuItem = NULL;
}

void MenuActionUpdateSlider::execute() {
	assert(_menuItem);
	Common::String text = _menuItem->getText();
	Common::Point point = _menuItem->getMouseClickPoint();
	int offset = 0;
	_menuSystem->calcMenuItemTextPositionAtPoint(point, offset);
	int newSliderValue = calcNewSliderValue(offset);

	debug(0, "item text: %s, (%d, %d), New slider value: %d", text.c_str(), point.x, point.y, newSliderValue);

	setSliderValue(newSliderValue);
}

int MenuActionUpdateSlider::calcNewSliderValue(int newOffset) {
	Common::String text = _menuItem->getText();
	int newSliderValue = 0;
	int start = 0;
	int end = 0;
	int currentPosition = 0;
	for (uint i = 0; i < text.size(); i++) {
		switch (text[i]) {
			case '{' : start = i; break;
			case '}' : end = i; break;
			case '|' : currentPosition = i; break;
			default: break;
		}
	}

	if (newOffset >= start && newOffset <= end) {
		if (newOffset == start) {
			newSliderValue = 0;
		} else if (newOffset == end) {
			newSliderValue = 15;
		} else {
			newSliderValue = newOffset - (start + 1);
		}
		return newSliderValue;
	}
	return currentPosition - start - 1;
}

void MenuActionUpdateSlider::setSliderValue(uint8 newValue) {
	int start = 0;
	Common::String text = _menuItem->getText();
	for (uint i = 0; i < text.size(); i++) {
		switch (text[i]) {
			case '{' : start = i; break;
			case '|' : text.setChar('~', i); break;
			default: break;
		}
	}

	text.setChar('|', start + newValue + 1);

	_menuItem->setText(text);
	_menuSystem->redrawMenuText(menu);

	switch (_type) {
		case SFX : _vm->_soundMan->setSfxVolume(newValue * (256/15)); break;
		case MUSIC : _vm->_soundMan->setMusicVolume(newValue * (256/15)); break;
		case VOICE : _vm->_soundMan->setSpeechVolume(newValue * (256/15)); break;
		case TEXT_DURATION : _vm->setSubtitleDuration(newValue * (256/15)); break;
		default: break;
	}
}

MenuActionResetOptionSliders::MenuActionResetOptionSliders(BaseMenuSystem *menuSystem,
														   MenuActionUpdateSlider *sfxSlider,
														   MenuActionUpdateSlider *musicSlider,
														   MenuActionUpdateSlider *speechSlider,
														   MenuActionUpdateSlider *textDurationSlider)
: BaseMenuAction(menuSystem), _sfxSlider(sfxSlider), _musicSlider(musicSlider),
  _speechSlider(speechSlider), _textDurationSlider(textDurationSlider) {

}

void MenuActionResetOptionSliders::execute() {
	_sfxSlider->setSliderValue(11);
	_musicSlider->setSliderValue(11);
	_speechSlider->setSliderValue(15);
	_textDurationSlider->setSliderValue(0);
}

} // End of namespace Illusions
