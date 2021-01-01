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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef ENABLE_LOL

#ifndef KYRA_GUI_LOL_H
#define KYRA_GUI_LOL_H

#include "kyra/gui/gui_v1.h"

namespace Kyra {
#define GUI_LOL_MENU(menu, a, b, c, d, e, f, g, i) \
	do { \
		const ScreenDim *dim = _screen->getScreenDim(a); \
		menu.x = (dim->sx << 3); \
		menu.y = (dim->sy); \
		menu.width = (dim->w << 3); \
		menu.height = (dim->h); \
		if (_vm->gameFlags().use16ColorMode) { \
		menu.bkgdColor = 0xCC; \
		menu.color1 = 0xFF; \
		menu.color2 = 0xDD; \
		} else { \
		menu.bkgdColor = 225; \
		menu.color1 = 223; \
		menu.color2 = 227; \
		} \
		menu.menuNameId = b; \
		menu.highlightedItem = c; \
		menu.numberOfItems = d; \
		menu.titleX = (dim->sx << 3) + (dim->w << 2); \
		menu.titleY = 6; \
		menu.textColor = _vm->gameFlags().use16ColorMode ? 0xE1 : 254; \
		menu.scrollUpButtonX = e; \
		menu.scrollUpButtonY = f; \
		menu.scrollDownButtonX = g; \
		menu.scrollDownButtonY = i; \
	} while (0)

	#define GUI_LOL_MENU_ITEM(item, a, b, c, d, e, f, g) \
	do { \
		item.enabled = 1; \
		item.itemId = a; \
		item.itemString = ""; \
		item.x = b; \
		item.y = c; \
		item.width = d; \
		item.height = e; \
		item.textColor =  _vm->gameFlags().use16ColorMode ? 0xC1 : 204; \
		item.highlightColor = _vm->gameFlags().use16ColorMode ? 0xE1 : 254; \
		item.titleX = -1; \
		if (_vm->gameFlags().use16ColorMode) { \
		item.bkgdColor = 0xCC; \
		item.color1 = 0xFF; \
		item.color2 = 0xDD; \
		} else { \
		item.bkgdColor = 225; \
		item.color1 = 223; \
		item.color2 = 227; \
		} \
		item.saveSlot = 0; \
		item.labelId = f; \
		item.labelString = 0; \
		item.labelX = 0; \
		item.labelY = 0; \
		item.keyCode = g; \
	} while (0)

class LoLEngine;
class Screen_LoL;

class GUI_LoL : public GUI_v1 {
	friend class LoLEngine;
public:
	GUI_LoL(LoLEngine *vm);

	void initStaticData();

	// button specific
	void processButton(Button *button) override;
	int processButtonList(Button *buttonList, uint16 inputFlags, int8 mouseWheel) override;

	int redrawShadedButtonCallback(Button *button) override;
	int redrawButtonCallback(Button *button) override;

	int runMenu(Menu &menu);

	// utilities for thumbnail creation
	void createScreenThumbnail(Graphics::Surface &dst) override;

private:
	void backupPage0();
	void restorePage0();

	void setupSaveMenuSlots(Menu &menu, int num);

	void printMenuText(const Common::String &str, int x, int y, uint8 c0, uint8 c1, uint8 flags) override;
	int getMenuCenterStringX(const Common::String &str, int x1, int x2) override;

	int getInput();

	int clickedMainMenu(Button *button);
	int clickedLoadMenu(Button *button);
	int clickedSaveMenu(Button *button);
	int clickedDeleteMenu(Button *button);
	int clickedOptionsMenu(Button *button);
	int clickedAudioMenu(Button *button);
	int clickedDeathMenu(Button *button);
	int clickedSavenameMenu(Button *button);
	int clickedChoiceMenu(Button *button);

	int scrollUp(Button *button);
	int scrollDown(Button *button);

	Button *getButtonListData() override { return _menuButtons; }
	Button *getScrollUpButton() override { return &_scrollUpButton; }
	Button *getScrollDownButton() override { return &_scrollDownButton; }


	Button::Callback getScrollUpButtonHandler() const override { return _scrollUpFunctor; }
	Button::Callback getScrollDownButtonHandler() const override { return _scrollDownFunctor; }

	uint8 defaultColor1() const override { return 0xFE; }
	uint8 defaultColor2() const override { return 0x00; }

	Common::String getMenuTitle(const Menu &menu) override;
	Common::String getMenuItemTitle(const MenuItem &menuItem) override;
	Common::String getMenuItemLabel(const MenuItem &menuItem) override;

	Button _menuButtons[10];
	Button _scrollUpButton;
	Button _scrollDownButton;
	Menu _mainMenu, _gameOptions, _audioOptions, _choiceMenu, _loadMenu, _saveMenu, _deleteMenu, _savenameMenu, _deathMenu;
	Menu *_currentMenu, *_lastMenu, *_newMenu;
	int _menuResult;
	char *_saveDescription;

	LoLEngine *_vm;
	Screen_LoL *_screen;

	bool _pressFlag;

	Button *_specialProcessButton;
	Button *_backUpButtonList;
	uint16 _flagsModifier;

	int _savegameOffset;
	int _sliderSfx;

	Button::Callback _scrollUpFunctor;
	Button::Callback _scrollDownFunctor;

	void sortSaveSlots() override;
};

} // End of namespace Kyra

#endif

#endif // ENABLE_LOL
