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

#ifndef ILLUSIONS_BBDOU_MENUSYSTEM_BBDOU_H
#define ILLUSIONS_BBDOU_MENUSYSTEM_BBDOU_H

#include "illusions/menusystem.h"

namespace Illusions {

enum {
	kBBDOUMainMenu,
	kBBDOUPauseMenu,
	kBBDOULoadGameMenu,
	kBBDOUSaveGameMenu,
	kBBDOUGameSavedMenu,
	kBBDOUSaveFailedMenu,
	kBBDOULoadFailedMenu,
	kBBDOULastMenuIndex
};

class IllusionsEngine_BBDOU;

class BBDOUMenuSystem : public BaseMenuSystem {
public:
	BBDOUMenuSystem(IllusionsEngine_BBDOU *vm);
	~BBDOUMenuSystem() override;
	void runMenu(MenuChoiceOffsets menuChoiceOffsets, int16 *menuChoiceOffset,
		uint32 menuId, uint32 duration, uint timeOutMenuChoiceIndex, uint32 menuCallerThreadId);
public://protected:
	IllusionsEngine_BBDOU *_vm;
	BaseMenu *_menus[kBBDOULastMenuIndex];
	void clearMenus();
	void freeMenus();
	BaseMenu *getMenuById(int menuId) override;
	BaseMenu *createMenuById(int menuId);
	BaseMenu *createMainMenu();
	BaseMenu *createPauseMenu();
	BaseMenu *createLoadGameMenu();
	BaseMenu *createSaveGameMenu();
	BaseMenu *createGameSavedMenu();
	BaseMenu *createSaveFailedMenu();
	BaseMenu *createLoadFailedMenu();
	BaseMenu *createOptionsMenu();
	int convertRootMenuId(uint32 menuId);
	bool initMenuCursor() override;
	int getGameState() override;
	void setGameState(int gameState) override;
	void setMenuCursorNum(int cursorNum) override;
	void playSoundEffect(int sfxId) override;
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_MENUSYSTEM_BBDOU_H
