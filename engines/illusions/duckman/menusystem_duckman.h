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

enum SliderActionType {
	SFX,
	MUSIC,
	VOICE,
	TEXT_DURATION
};

enum {
	kDuckmanMainMenu,
	kDuckmanMainMenuDemo,
	kDuckmanLoadGameMenu,
	kDuckmanLoadGameFailedMenu,
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
class MenuActionUpdateSlider;

class DuckmanMenuSystem : public BaseMenuSystem {
public:
	DuckmanMenuSystem(IllusionsEngine_Duckman *vm);
	~DuckmanMenuSystem() override;
	void runMenu(MenuChoiceOffsets menuChoiceOffsets, int16 *menuChoiceOffset,
		uint32 menuId, uint32 duration, uint timeOutMenuChoiceIndex, uint32 menuCallerThreadId);
public://protected:
	IllusionsEngine_Duckman *_vm;
	BaseMenu *_menus[kDuckmanLastMenuIndex];
	void clearMenus();
	void freeMenus();
	BaseMenu *getMenuById(int menuId) override;
	BaseMenu *createMenuById(int menuId);
	BaseMenu *createMainMenu();
	BaseMenu *createMainMenuDemo();
	BaseMenu *createLoadGameMenu();
	BaseMenu *createLoadGameFailedMenu();
	BaseMenu *createOptionsMenu();
	BaseMenu *createPauseMenu();
	BaseMenu *createQueryRestartMenu();
	BaseMenu *createQueryQuitMenu();
	BaseMenu *createSaveCompleteMenu();
	BaseMenu *createDebugMenu();
	BaseMenu *createDebugPauseMenu();
	BaseMenu *createAddRemoveInventoryMenu();
	int convertRootMenuId(uint32 menuId);
	bool initMenuCursor() override;
	int getGameState() override;
	void setGameState(int gameState) override;
	void setMenuCursorNum(int cursorNum) override;
	void playSoundEffect(int sfxId) override;
private:
	MenuItem *createOptionsSliderMenuItem(MenuActionUpdateSlider **action, const Common::String &text,
										  SliderActionType type, BaseMenu *baseMenu);
	};

class MenuActionInventoryAddRemove : public BaseMenuAction {
public:
	MenuActionInventoryAddRemove(BaseMenuSystem *menuSystem, IllusionsEngine_Duckman *vm, uint choiceIndex);
	void execute() override;
protected:
	IllusionsEngine_Duckman *_vm;
	int _choiceIndex;
};

class MenuActionUpdateSlider : public BaseMenuAction {
public:
	MenuActionUpdateSlider(BaseMenuSystem *menuSystem, BaseMenu *baseMenu, SliderActionType type, IllusionsEngine_Duckman *vm);
	void setMenuItem(MenuItem *menuItem) {
		_menuItem = menuItem;
	}

	void execute() override;
	void setSliderValue(uint8 newValue);
protected:
	IllusionsEngine_Duckman *_vm;
	SliderActionType _type;
	MenuItem *_menuItem;
	BaseMenu *menu;
	int calcNewSliderValue(int newOffset);
};

class MenuActionResetOptionSliders : public BaseMenuAction {
public:
	MenuActionResetOptionSliders(BaseMenuSystem *menuSystem,
								 MenuActionUpdateSlider *sfxSlider,
								 MenuActionUpdateSlider *musicSlider,
								 MenuActionUpdateSlider *speechSlider,
								 MenuActionUpdateSlider *textDurationSlider
	);
	void execute() override;
protected:
	MenuActionUpdateSlider *_sfxSlider;
	MenuActionUpdateSlider *_musicSlider;
	MenuActionUpdateSlider *_speechSlider;
	MenuActionUpdateSlider *_textDurationSlider;
};

} // End of namespace Illusions

#endif // ILLUSIONS_DUCKMAN_MENUSYSTEM_DUCKMAN_H
