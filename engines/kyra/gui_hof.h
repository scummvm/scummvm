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
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_GUI_HOF_H
#define KYRA_GUI_HOF_H

#include "kyra/gui_v2.h"

namespace Kyra {

class KyraEngine_HoF;
class Screen_HoF;

class GUI_HoF : public GUI_v2 {
friend class KyraEngine_HoF;
public:
	GUI_HoF(KyraEngine_HoF *engine);

	int optionsButton(Button *button);
private:
	void getInput();

	Button _menuButtons[7];
	Button _scrollUpButton;
	Button _scrollDownButton;
	Menu _mainMenu, _gameOptions, _audioOptions, _choiceMenu, _loadMenu, _saveMenu, _savenameMenu, _deathMenu;
	void initStaticData();

	const char *getMenuTitle(const Menu &menu);
	const char *getMenuItemTitle(const MenuItem &menuItem);
	const char *getMenuItemLabel(const MenuItem &menuItem);

	Button *getButtonListData() { return _menuButtons; }

	Button *getScrollUpButton() { return &_scrollUpButton; }
	Button *getScrollDownButton() { return &_scrollDownButton; }

	int scrollUpButton(Button *button);
	int scrollDownButton(Button *button);
	Button::Callback _scrollUpFunctor;
	Button::Callback _scrollDownFunctor;
	Button::Callback getScrollUpButtonHandler() const { return _scrollUpFunctor; }
	Button::Callback getScrollDownButtonHandler() const { return _scrollDownFunctor; }

	Button _sliderButtons[3][4];

	uint8 defaultColor1() const { return 0xCF; }
	uint8 defaultColor2() const { return 0xF8; }

	void renewHighlight(Menu &menu);

	void setupPalette();
	void restorePalette();

	void backUpPage1(uint8 *buffer);
	void restorePage1(const uint8 *buffer);

	void resetState(int item);

	KyraEngine_HoF *_vm;
	Screen_HoF *_screen;

	Menu *_currentMenu;
	bool _isLoadMenu;
	bool _isDeathMenu;
	bool _isSaveMenu;
	bool _isDeleteMenu;
	bool _isChoiceMenu;
	bool _isOptionsMenu;
	bool _madeSave;
	bool _loadedSave;
	bool _restartGame;
	bool _reloadTemporarySave;

	int _savegameOffset;

	void setupSavegameNames(Menu &menu, int num);

	// main menu
	int quitGame(Button *caller);
	int resumeGame(Button *caller);

	// options menu
	int gameOptions(Button *caller);
	int gameOptionsTalkie(Button *caller);
	int quitOptionsMenu(Button *caller);

	int toggleWalkspeed(Button *caller);
	int changeLanguage(Button *caller);
	int toggleText(Button *caller);

	void setupOptionsButtons();

	// audio menu
	int audioOptions(Button *caller);

	Button::Callback _sliderHandlerFunctor;
	int sliderHandler(Button *caller);

	void drawSliderBar(int slider, const uint8 *shape);

	static const int _sliderBarsPosition[];

	// load menu
	bool _noLoadProcess;
	int loadMenu(Button *caller);
	int clickLoadSlot(Button *caller);
	int cancelLoadMenu(Button *caller);

	// save menu
	bool _noSaveProcess;
	int _saveSlot;
	char _saveDescription[0x50];

	int saveMenu(Button *caller);
	int clickSaveSlot(Button *caller);
	int cancelSaveMenu(Button *caller);

	// delete menu
	int _slotToDelete;
	int deleteMenu(Button *caller);

	// savename menu
	bool _finishNameInput, _cancelNameInput;
	Common::KeyState _keyPressed;

	const char *nameInputProcess(char *buffer, int x, int y, uint8 c1, uint8 c2, uint8 c3, int bufferSize);
	int finishSavename(Button *caller);
	int cancelSavename(Button *caller);

	bool checkSavegameDescription(const char *buffer, int size);
	int getCharWidth(uint8 c);
	void checkTextfieldInput();
	void drawTextfieldBlock(int x, int y, uint8 c);

	// choice menu
	bool _choice;

	bool choiceDialog(int name, bool type);
	int choiceYes(Button *caller);
	int choiceNo(Button *caller);

	static const uint16 _menuStringsTalkie[];
	static const uint16 _menuStringsOther[];
};

} // end of namespace Kyra

#endif

