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

struct EobRect16 {
	int16 x1;
	int16 y1;
	uint16 x2;
	uint16 y2;
};

class DarkMoonEngine;
class Screen_Eob;

class GUI_Eob : public GUI_v1 {
	friend class EobCoreEngine;
	friend class CharacterGenerator;
public:
	GUI_Eob(EobCoreEngine *vm);
	virtual ~GUI_Eob();

	void initStaticData() {}

	// button specific
	void processButton(Button *button);
	int processButtonList(Button *buttonList, uint16 inputFlags, int8 mouseWheel);

	// Non button based menu handling (main menu, character generation)
	void simpleMenu_setup(int sd, int maxItem, const char *const *strings, int32 menuItemsMask, int unk, int lineSpacing);
	int simpleMenu_process(int sd, const char *const *strings, void *b, int32 menuItemsMask, int unk);	

	// Button based menus (camp menu, options, save/load)
	void runCampMenu();
	int runLoadMenu(int x, int y);

	void updateBoxFrameHighLight(int box);

	int getTextInput(char *dest, int x, int y, int destMaxLen, int textColor1, int textColor2, int cursorColor);

	// utilities for thumbnail creation
	void createScreenThumbnail(Graphics::Surface &dst) {}

	// unused
	int redrawShadedButtonCallback(Button *button) { return 0; }
	int redrawButtonCallback(Button *button) { return 0; }

private:
	int simpleMenu_getMenuItem(int index, int32 menuItemsMask, int itemOffset);
	void simpleMenu_flashSelection(const char *str, int x, int y, int color1, int color2, int color3);
	void simpleMenu_initMenuItemsMask(int menuId, int maxItem, int32 menuItemsMask, int unk);

	void runSaveMenu();
	void runMemorizePrayMenu(int charIndex, int spellType);
	void scribeScrollDialogue();

	bool confirmDialogue(int id);
	int selectCharacterDialogue(int id);
	void displayTextBox(int id);
	
	Button *initMenu(int id);	
	void drawMenuButton(Button *b, bool clicked, bool highlight, bool noFill);
	void drawMenuButtonBox(int x, int y, int w, int h, bool clicked, bool noFill);
	void memorizePrayMenuPrintString(int spellId, int bookPageIndex, int spellType, bool noFill, bool highLight);
	void updateOptionsStrings();
	const char *getMenuString(int id);

	Button *linkButton(Button *list, Button *newbt);
	void releaseButtons(Button *list);

	Button _scrollUpButton;//////////////////77
	Button _scrollDownButton;

	char **_menuStringsPrefsTemp;

	EobCoreEngine *_vm;
	Screen_Eob *_screen;

	bool _pressFlag;

	Button *_specialProcessButton;
	Button *_backupButtonList;
	uint16 _flagsMouseLeft;
	uint16 _flagsMouseRight;
	uint16 _flagsModifier;
	uint16 _progress;
	uint16 _prcButtonUnk3;
	uint16 _cflag;

	Button::Callback _scrollUpFunctor;
	Button::Callback _scrollDownFunctor;

	int _menuLineSpacing;
	//int _menuUnk1;
	int _menuLastInFlags;

	uint8 _numPages;
	uint8 _numVisPages;
	int8 *_numAssignedSpellsOfType;
	uint32 _clericSpellAvltyFlags;
	uint32 _paladinSpellAvltyFlags;

	int _menuCur;
	int _menuNumItems;
	bool _charSelectRedraw;

	int _updateBoxIndex;
	int _updateBoxColorIndex;
	uint32 _highLightBoxTimer;
	static const EobRect16 _updateBoxFrameHighLights[];

	// unused
	Button *getButtonListData() { return 0; }
	Button *getScrollUpButton() { return &_scrollUpButton; }
	Button *getScrollDownButton() { return &_scrollDownButton; }

	Button::Callback getScrollUpButtonHandler() const { return _scrollUpFunctor; }
	Button::Callback getScrollDownButtonHandler() const { return _scrollDownFunctor; }

	uint8 defaultColor1() const { return 0xFE; }
	uint8 defaultColor2() const { return 0x00; }

	const char *getMenuTitle(const Menu &menu) { return 0; }
	const char *getMenuItemTitle(const MenuItem &menuItem) { return 0; }
	const char *getMenuItemLabel(const MenuItem &menuItem) { return 0; }
};

}	// End of namespace Kyra

#endif // ENABLE_EOB

#endif

#endif // ENABLE_EOB || ENABLE_LOL
