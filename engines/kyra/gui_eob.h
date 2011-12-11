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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#ifndef KYRA_GUI_EOB_H
#define KYRA_GUI_EOB_H

#include "kyra/gui.h"

#ifdef ENABLE_EOB

namespace Kyra {

class DarkMoonEngine;
class Screen_Eob;

class GUI_Eob : public GUI {
	friend class EobCoreEngine;
	friend class CharacterGenerator;
public:
	GUI_Eob(EobCoreEngine *vm);

	void initStaticData() {}

	// button specific
	void processButton(Button *button);
	int processButtonList(Button *buttonList, uint16 inputFlags, int8 mouseWheel);

	int redrawShadedButtonCallback(Button *button) { return 0; }
	int redrawButtonCallback(Button *button) { return 0; }

	void setupMenu(int sd, int maxItem, const char *const *strings, int32 menuItemsMask, int unk, int lineSpacing);
	int handleMenu(int sd, const char *const *strings, void *b, int32 menuItemsMask, int unk);
	int getMenuItem(int index, int32 menuItemsMask, int unk);
	void menuFlashSelection(const char *str, int x, int y, int color1, int color2, int color3);

	int getTextInput(char *dest, int x, int y, int destMaxLen, int textColor1, int textColor2, int cursorColor);

	//int runMenu(Menu &menu);

	// utilities for thumbnail creation
	void createScreenThumbnail(Graphics::Surface &dst) {}

private:
	void initMenuItemsMask(int menuId, int maxItem, int32 menuItemsMask, int unk);

	//void backupPage0();
	//void restorePage0();

	//void setupSavegameNames(Menu &menu, int num);
	//void printMenuText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 flags);

	//int getMenuCenterStringX(const char *str, int x1, int x2);

	int getInput();

	Button *getButtonListData() { return _menuButtons; }
	Button *getScrollUpButton() { return &_scrollUpButton; }
	Button *getScrollDownButton() { return &_scrollDownButton; }

	Button::Callback getScrollUpButtonHandler() const { return _scrollUpFunctor; }
	Button::Callback getScrollDownButtonHandler() const { return _scrollDownFunctor; }

	uint8 defaultColor1() const { return 0xFE; }
	uint8 defaultColor2() const { return 0x00; }

	const char *getMenuTitle(const Menu &menu) { return 0; }
	const char *getMenuItemTitle(const MenuItem &menuItem) { return 0; }
	const char *getMenuItemLabel(const MenuItem &menuItem) { return 0; }

	Button _menuButtons[10];
	Button _scrollUpButton;
	Button _scrollDownButton;
	//Menu _mainMenu, _gameOptions, _audioOptions, _choiceMenu, _loadMenu, _saveMenu, _deleteMenu, _savenameMenu, _deathMenu;
	//Menu *_currentMenu, *_lastMenu, *_newMenu;
	//int _menuResult;
	//char *_saveDescription;

	EobCoreEngine *_vm;
	Screen_Eob *_screen;

	bool _pressFlag;

	Button *_specialProcessButton;
	Button *_backupButtonList;
	uint16 _flagsMouseLeft;
	uint16 _flagsMouseRight;
	uint16 _flagsModifier;
	uint16 _progress;
	uint16 _prcButtonUnk3; /// ALWAYS 1?? REMOVE ??
	uint16 _cflag;

	Button::Callback _scrollUpFunctor;
	Button::Callback _scrollDownFunctor;

	int _menuLineSpacing;
	int _menuUnk1;
	int _menuLastInFlags;

	int _menuCur;
	int _menuNumItems;
};

}	// End of namespace Kyra

#endif // ENABLE_EOB

#endif

#endif // ENABLE_EOB || ENABLE_LOL
