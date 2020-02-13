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

#ifndef ILLUSIONS_MENUSYSTEM_H
#define ILLUSIONS_MENUSYSTEM_H

#include "illusions/actor.h"
#include "illusions/graphics.h"
#include "illusions/resources/fontresource.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/stack.h"
#include "common/str.h"
#include "graphics/surface.h"

namespace Illusions {

class IllusionsEngine;

class BaseMenuSystem;
class BaseMenuAction;

const uint kMenuTextSize = 4096;

class MenuItem {
public:
	MenuItem(const Common::String text, BaseMenuAction *action);
	~MenuItem();
	void executeAction(const Common::Point &point);
	const Common::String& getText() const { return _text; }
	void setText(const Common::String &text) { _text = text; }
	const Common::Point& getMouseClickPoint() { return _mouseClickPoint; };
protected:
	Common::String _text;
	BaseMenuAction *_action;
	Common::Point _mouseClickPoint;
};

class BaseMenu {
public:
	BaseMenu(BaseMenuSystem *menuSystem, uint32 fontId, byte backgroundColor, byte borderColor, byte textColor, byte fieldE,
		uint defaultMenuItemIndex);
	virtual ~BaseMenu();
	void addText(const Common::String text);
	void addMenuItem(MenuItem *menuItem);
	uint getHeaderLinesCount();
	const Common::String& getHeaderLine(uint index);
	uint getMenuItemsCount();
	MenuItem *getMenuItem(uint index);
	virtual void enterMenu();
public://protected://TODO
	typedef Common::Array<MenuItem*> MenuItems;
	BaseMenuSystem *_menuSystem;
	uint32 _fontId;
	byte _backgroundColor, _borderColor, _textColor, _fieldE;
	uint _field2C18;
	uint _defaultMenuItemIndex;
	Common::Array<Common::String> _text;
	MenuItems _menuItems;
};

class MenuStack : public Common::Stack<BaseMenu*> {
};

typedef Common::Array<int16> MenuChoiceOffsets;

class BaseMenuSystem {
public:
	BaseMenuSystem(IllusionsEngine *vm);
	virtual ~BaseMenuSystem();
	void playSoundEffect13();
	void playSoundEffect14();
	void selectMenuChoiceIndex(uint choiceIndex);
	void leaveMenu();
	void enterSubMenu(BaseMenu *menu);
	void leaveSubMenu();
	void enterSubMenuById(int menuId);
	uint getQueryConfirmationChoiceIndex() const;
	void setQueryConfirmationChoiceIndex(uint queryConfirmationChoiceIndex);
	bool isActive() const { return _isActive; }
	void openMenu(BaseMenu *menu);
	void closeMenu();
	void handleClick(uint menuItemIndex, const Common::Point &mousePos);
	uint drawMenuText(BaseMenu *menu);
	void redrawMenuText(BaseMenu *menu);
	void update(Control *cursorControl);
	void setTimeOutDuration(uint32 duration, uint timeOutMenuChoiceIndex);
	void setMenuCallerThreadId(uint32 menuCallerThreadId);
	void setMenuChoiceOffsets(MenuChoiceOffsets menuChoiceOffsets, int16 *menuChoiceOffset);
	void setSavegameSlotNum(int slotNum);
	void setSavegameDescription(Common::String desc);
	bool calcMenuItemTextPositionAtPoint(Common::Point pt, int &offset);
	virtual bool initMenuCursor() = 0;
	virtual int getGameState() = 0;
	virtual void setGameState(int gameState) = 0;
	virtual void setMenuCursorNum(int cursorNum) = 0;
protected:
	IllusionsEngine *_vm;
	MenuStack _menuStack;

	uint32 _menuCallerThreadId;
	bool _isTimeOutEnabled;
	bool _isTimeOutReached;
	uint32 _timeOutDuration;
	uint _timeOutMenuChoiceIndex;
	uint32 _timeOutStartTime;
	uint32 _timeOutEndTime;

	Common::Point _savedCursorPos;
	bool _cursorInitialVisibleFlag;
	int _savedGameState;
	int _savedCursorActorIndex;
	int _savedCursorSequenceId;

	bool _isActive;

	MenuChoiceOffsets _menuChoiceOffsets;
	int16 *_menuChoiceOffset;

	uint _queryConfirmationChoiceIndex;

	uint _field54;
	uint _menuLinesCount;
	uint _menuItemCount;

	uint _hoveredMenuItemIndex;
	uint _hoveredMenuItemIndex2;
	uint _hoveredMenuItemIndex3;

	BaseMenu *_activeMenu;
	void setMouseCursorToMenuItem(int menuItemIndex);

	void calcMenuItemRect(uint menuItemIndex, WRect &rect);
	bool calcMenuItemMousePos(uint menuItemIndex, Common::Point &pt);
	bool calcMenuItemIndexAtPoint(Common::Point pt, uint &menuItemIndex);
	void setMousePos(Common::Point &mousePos);

	void activateMenu(BaseMenu *menu);

	void updateTimeOut(bool resetTimeOut);

	void initActorHoverBackground();
	void placeActorHoverBackground();
	void updateActorHoverBackground();
	void hideActorHoverBackground();

	void initActorTextColorRect();
	void placeActorTextColorRect();
	void hideActorTextColorRect();

	virtual BaseMenu *getMenuById(int menuId) = 0;
	virtual void playSoundEffect(int sfxId) = 0;
};

class MenuTextBuilder {
public:
	MenuTextBuilder();
	void appendString(const Common::String &value);
	void appendNewLine();
	void finalize();
	uint16 *getText() { return _text; }
protected:
	uint16 _text[kMenuTextSize];
	uint _pos;
};

// Menu actions

class BaseMenuAction {
public:
	BaseMenuAction(BaseMenuSystem *menuSystem);
	virtual ~BaseMenuAction() {}
	virtual void execute() = 0;
protected:
	BaseMenuSystem *_menuSystem;
};

// Type 1: Enter a submenu

class MenuActionEnterMenu : public BaseMenuAction {
public:
	MenuActionEnterMenu(BaseMenuSystem *menuSystem, int menuId);
	void execute() override;
protected:
	int _menuId;
};

// Type 4: Leave a submenu or the whole menu if on the main menu level

class MenuActionLeaveMenu : public BaseMenuAction {
public:
	MenuActionLeaveMenu(BaseMenuSystem *menuSystem);
	void execute() override;
};

// Type 5: Return a menu choice index and exit the menu

class MenuActionReturnChoice : public BaseMenuAction {
public:
	MenuActionReturnChoice(BaseMenuSystem *menuSystem, uint choiceIndex);
	void execute() override;
protected:
	int _choiceIndex;
};

// Type 8: Return a menu choice index and exit the menu after displaying a query message

class MenuActionEnterQueryMenu : public BaseMenuAction {
public:
	MenuActionEnterQueryMenu(BaseMenuSystem *menuSystem, int menuId, uint confirmationChoiceIndex);
	void execute() override;
protected:
	int _menuId;
	uint _confirmationChoiceIndex;
};

class MenuActionLoadGame : public BaseMenuAction {
public:
	MenuActionLoadGame(BaseMenuSystem *menuSystem, uint choiceIndex);
	void execute() override;
protected:
	uint _choiceIndex;
};

class MenuActionSaveGame : public BaseMenuAction {
public:
	MenuActionSaveGame(BaseMenuSystem *menuSystem, uint choiceIndex);
	void execute() override;
protected:
	uint _choiceIndex;
};

} // End of namespace Illusions

#endif // ILLUSIONS_MENUSYSTEM_H
