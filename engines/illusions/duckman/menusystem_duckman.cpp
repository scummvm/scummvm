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
#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/menusystem_duckman.h"

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
	for (int i = 0; i < kDuckmanLastMenuIndex; ++i)
		_menus[i] = 0;
}

void DuckmanMenuSystem::freeMenus() {
	for (int i = 0; i < kDuckmanLastMenuIndex; ++i)
		delete _menus[i];
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
	default:
		error("DuckmanMenuSystem::createMenuById() Invalid menu id %d", menuId);
	}
}

BaseMenu *DuckmanMenuSystem::createMainMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 0);
	menu->addMenuItem(new MenuItem("Start New Game", new MenuActionReturnChoice(this, 11)));
	menu->addMenuItem(new MenuItem("Load Saved Game", new MenuActionLoadGame(this, 1)));
	// TODO menu->addMenuItem(new MenuItem("Options", new MenuActionEnterMenu(this, kDuckmanOptionsMenu)));
	menu->addMenuItem(new MenuItem("Quit Game", new MenuActionEnterQueryMenu(this, kDuckmanQueryQuitMenu, 12)));
	return menu;
}

BaseMenu *DuckmanMenuSystem::createLoadGameMenu() {
	return 0; // TODO
}

BaseMenu *DuckmanMenuSystem::createOptionsMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 1);
	menu->addText("              GAME OPTIONS");
	menu->addText("--------------------------------------");
	menu->addMenuItem(new MenuItem("SFX Volume     @@{~~~~~~~~~~~~|~~~}", new MenuActionReturnChoice(this, 21)));
	menu->addMenuItem(new MenuItem("Music Volume  @@@{~~~~~~~~~~~~|~~~}", new MenuActionReturnChoice(this, 21)));
	menu->addMenuItem(new MenuItem("Speech Volume {~~~~~~~~~~~~|~~~}", new MenuActionReturnChoice(this, 21)));
	menu->addMenuItem(new MenuItem("Text Duration @@@{~~~~~~~~~~~~|~~~}", new MenuActionReturnChoice(this, 21)));
	menu->addMenuItem(new MenuItem("Restore Defaults", new MenuActionReturnChoice(this, 21)));
	menu->addMenuItem(new MenuItem("Back", new MenuActionLeaveMenu(this)));
	return menu;
}

BaseMenu *DuckmanMenuSystem::createPauseMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 1);
	menu->addText("   Game Paused");
	menu->addText("--------------------");
	menu->addMenuItem(new MenuItem("Resume", new MenuActionReturnChoice(this, 21)));
	menu->addMenuItem(new MenuItem("Load Game", new MenuActionLoadGame(this, 1)));
	menu->addMenuItem(new MenuItem("Save Game", new MenuActionSaveGame(this, 11)));
	menu->addMenuItem(new MenuItem("Restart Game", new MenuActionEnterQueryMenu(this, kDuckmanQueryRestartMenu, 2)));
	menu->addMenuItem(new MenuItem("Options", new MenuActionEnterMenu(this, kDuckmanOptionsMenu)));
	menu->addMenuItem(new MenuItem("Quit Game", new MenuActionEnterQueryMenu(this, kDuckmanQueryQuitMenu, 23)));
	return menu;
}

BaseMenu *DuckmanMenuSystem::createQueryRestartMenu() {
	return 0; // TODO
}

BaseMenu *DuckmanMenuSystem::createQueryQuitMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 2);
	menu->addText("Do you really want to quit?");
	menu->addText("-------------------------------");
	menu->addMenuItem(new MenuItem("Yes, I'm outta here", new MenuActionReturnChoice(this, getQueryConfirmationChoiceIndex())));
	menu->addMenuItem(new MenuItem("No, just kidding", new MenuActionLeaveMenu(this)));
	return menu;
}

BaseMenu *DuckmanMenuSystem::createSaveCompleteMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 1);
	menu->addText("Game Saved");
	menu->addText("-------------");
	menu->addMenuItem(new MenuItem("Continue", new MenuActionReturnChoice(this, 1)));
	return menu;
}

int DuckmanMenuSystem::convertRootMenuId(uint32 menuId) {
	switch (menuId) {
	case 0x180001:
		return kDuckmanMainMenu;
	case 0x180002:
		return kDuckmanPauseMenu;
	case 0x180005:
		return kDuckmanSaveCompleteMenu;
	/*
	case 0x180006: // save game failed menu
	case 0x180007: // load game failed menu
	*/
	/* TODO CHECKME Another pause menu?
	case 0x180008:
		menuData = &g_menuDataPause;
	*/
	default:
		error("DuckmanMenuSystem() Menu ID %08X not found", menuId);
	}
}

bool DuckmanMenuSystem::initMenuCursor() {
	bool cursorInitialVisibleFlag = false;
	Control *cursorControl = _vm->getObjectControl(0x40004);
	if (cursorControl) {
		if (cursorControl->_flags & 1)
			cursorInitialVisibleFlag = false;
		cursorControl->appearActor();
	} else {
		Common::Point pos = _vm->getNamedPointPosition(0x70001);
		_vm->_controls->placeActor(0x50001, pos, 0x60001, 0x40004, 0);
		cursorControl = _vm->getObjectControl(0x40004);
	}
	return cursorInitialVisibleFlag;
}

int DuckmanMenuSystem::getGameState() {
	return _vm->_cursor._gameState;
}

void DuckmanMenuSystem::setMenuCursorNum(int cursorNum) {
	Control *mouseCursor = _vm->getObjectControl(0x40004);
	_vm->setCursorActorIndex(5, cursorNum, 0);
	mouseCursor->startSequenceActor(0x60001, 2, 0);
}

void DuckmanMenuSystem::setGameState(int gameState) {
	_vm->_cursor._gameState = gameState;
}

} // End of namespace Illusions
