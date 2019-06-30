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
#include "illusions/cursor.h"
#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/bbdou/menusystem_bbdou.h"

namespace Illusions {

// BBDOUMenuSystem

BBDOUMenuSystem::BBDOUMenuSystem(IllusionsEngine_BBDOU *vm)
	: BaseMenuSystem(vm), _vm(vm) {
	clearMenus();
}

BBDOUMenuSystem::~BBDOUMenuSystem() {
	freeMenus();
}

void BBDOUMenuSystem::runMenu(MenuChoiceOffsets menuChoiceOffsets, int16 *menuChoiceOffset,
	uint32 menuId, uint32 duration, uint timeOutMenuChoiceIndex, uint32 menuCallerThreadId) {
	
	debug(0, "BBDOUMenuSystem::runMenu(%08X)", menuId);

	setTimeOutDuration(duration, timeOutMenuChoiceIndex);
	setMenuCallerThreadId(menuCallerThreadId);
	setMenuChoiceOffsets(menuChoiceOffsets, menuChoiceOffset);

	int rootMenuId = convertRootMenuId(menuId);
	BaseMenu *rootMenu = getMenuById(rootMenuId);
	openMenu(rootMenu);

}

void BBDOUMenuSystem::clearMenus() {
	for (int i = 0; i < kBBDOULastMenuIndex; ++i) {
		_menus[i] = 0;
	}
}

void BBDOUMenuSystem::freeMenus() {
	for (int i = 0; i < kBBDOULastMenuIndex; ++i) {
		delete _menus[i];
	}
}

BaseMenu *BBDOUMenuSystem::getMenuById(int menuId) {
	if (!_menus[menuId])
		_menus[menuId] = createMenuById(menuId);
	return _menus[menuId];
}

BaseMenu *BBDOUMenuSystem::createMenuById(int menuId) {
	switch (menuId) {
	case kBBDOUMainMenu:
		return createMainMenu();
	case kBBDOUPauseMenu:
		return createPauseMenu();
	// TODO Other menus
	default:
		error("BBDOUMenuSystem::createMenuById() Invalid menu id %d", menuId);
	}
}

BaseMenu *BBDOUMenuSystem::createMainMenu() {
	return 0; // TODO
}

BaseMenu *BBDOUMenuSystem::createLoadGameMenu() {
	return 0; // TODO
}

BaseMenu *BBDOUMenuSystem::createOptionsMenu() {
	return 0; // TODO
}

BaseMenu *BBDOUMenuSystem::createPauseMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 218, 150, 80, 20, 1);
	menu->addText("   Game Paused");
	menu->addText("-------------------");
	menu->addMenuItem(new MenuItem("Resume", new MenuActionReturnChoice(this, 1)));
	// menu->addMenuItem(new MenuItem("Load Game", new MenuActionLoadGame(this, 1)));
	// TODO menu->addMenuItem(new MenuItem("Save Game", new MenuActionSaveGame(this, 11)));
	// TODO menu->addMenuItem(new MenuItem("Restart Game", new MenuActionEnterQueryMenu(this, kDuckmanQueryRestartMenu, 2)));
	// TODO menu->addMenuItem(new MenuItem("Options", new MenuActionEnterMenu(this, kDuckmanOptionsMenu)));
	// menu->addMenuItem(new MenuItem("Quit Game", new MenuActionEnterQueryMenu(this, kDuckmanQueryQuitMenu, 23)));
	return menu;
}

int BBDOUMenuSystem::convertRootMenuId(uint32 menuId) {
	switch (menuId) {
	case 0x1C0001:
		return kBBDOUMainMenu;
	case 0x1C0002:
		return kBBDOUPauseMenu;
	case 0x1C0006:
		return kBBDOULoadGameMenu;
	case 0x1C0007:
		return kBBDOUSaveGameMenu;
	case 0x1C0008:
		return kBBDOUGameSavedMenu;
	case 0x1C0009:
		return kBBDOUSaveFailedMenu;
	case 0x1C000A:
		return kBBDOULoadFailedMenu;
	/* Unused/unimplemented debug menus
	case 0x1C0003: debugStartMenu
	case 0x1C0004: debugPauseMenu
	case 0x1C0005: unitTestsMenu
	*/
	default:
		error("BBDOUMenuSystem() Menu ID %08X not found", menuId);
	}
}

bool BBDOUMenuSystem::initMenuCursor() {
	bool cursorInitialVisibleFlag = false;
	Control *cursorControl = _vm->getObjectControl(0x40004);
	if (cursorControl) {
		if (cursorControl->_flags & 1) {
			cursorInitialVisibleFlag = false;
		} else {
			cursorInitialVisibleFlag = true;
			cursorControl->appearActor();
		}
	} else {
		Common::Point pos = _vm->getNamedPointPosition(0x70023);
		_vm->_controls->placeActor(0x50001, pos, 0x60001, 0x40004, 0);
		cursorControl = _vm->getObjectControl(0x40004);
	}
	return cursorInitialVisibleFlag;
}

int BBDOUMenuSystem::getGameState() {
	return _vm->_cursor->_status;
}

void BBDOUMenuSystem::setMenuCursorNum(int cursorNum) {
	Control *mouseCursor = _vm->getObjectControl(0x40004);
	_vm->_cursor->setActorIndex(5, cursorNum, 0);
	mouseCursor->startSequenceActor(0x60001, 2, 0);
}

void BBDOUMenuSystem::setGameState(int gameState) {
	_vm->_cursor->_status = gameState;
}

void BBDOUMenuSystem::playSoundEffect(int sfxId) {
	// TODO
}

} // End of namespace Illusions
