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

#ifndef ILLUSIONS_DUCKMAN_MENUSYSTEM_DUCKMAN_H
#define ILLUSIONS_DUCKMAN_MENUSYSTEM_DUCKMAN_H

#include "illusions/menusystem.h"

namespace Illusions {

enum {
	kDuckmanMainMenu,
	kDuckmanLoadGameMenu,
	kDuckmanOptionsMenu,
	kDuckmanPauseMenu,
	kDuckmanDebugMenu,
	kDuckmanDebugPauseMenu,
	kDuckmanAddRemoveInventoryMenu,
	kDuckmanQueryQuitMenu,
	kDuckmanQueryRestartMenu,
	kDuckmanSaveCompleteMenu,
	kDuckmanLastMenuIndex
};

class IllusionsEngine_Duckman;

class DuckmanMenuSystem : public BaseMenuSystem {
public:
	DuckmanMenuSystem(IllusionsEngine_Duckman *vm);
	~DuckmanMenuSystem();
	void runMenu(MenuChoiceOffsets menuChoiceOffsets, int16 *menuChoiceOffset,
		uint32 menuId, uint32 duration, uint timeOutMenuChoiceIndex, uint32 menuCallerThreadId);
public://protected:
	IllusionsEngine_Duckman *_vm;
	BaseMenu *_menus[kDuckmanLastMenuIndex];
	void clearMenus();
	void freeMenus();
	BaseMenu *getMenuById(int menuId);
	BaseMenu *createMenuById(int menuId);
	BaseMenu *createMainMenu();
	BaseMenu *createLoadGameMenu();
	BaseMenu *createOptionsMenu();
	BaseMenu *createPauseMenu();
	BaseMenu *createQueryRestartMenu();
	BaseMenu *createQueryQuitMenu();
	BaseMenu *createSaveCompleteMenu();
	BaseMenu *createDebugMenu();
	BaseMenu *createDebugPauseMenu();
	BaseMenu *createAddRemoveInventoryMenu();
	int convertRootMenuId(uint32 menuId);
	virtual bool initMenuCursor();
	virtual int getGameState();
	virtual void setGameState(int gameState);
	virtual void setMenuCursorNum(int cursorNum);
};

class MenuActionInventoryAddRemove : public BaseMenuAction {
public:
	MenuActionInventoryAddRemove(BaseMenuSystem *menuSystem, IllusionsEngine_Duckman *vm, uint choiceIndex);
	virtual void execute();
protected:
	IllusionsEngine_Duckman *_vm;
	int _choiceIndex;
};

} // End of namespace Illusions

#endif // ILLUSIONS_DUCKMAN_MENUSYSTEM_DUCKMAN_H
