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

#ifndef KYRA_GUI_H
#define KYRA_GUI_H

#include "kyra/kyra_v1.h"
#include "kyra/screen.h"

#include "common/ptr.h"
#include "common/array.h"
#include "common/func.h"

#include "graphics/surface.h"

namespace Kyra {

#define BUTTON_FUNCTOR(type, x, y) Button::Callback(new Common::Functor1Mem<Button*, int, type>(x, y))

struct Button {
	typedef Common::Functor1<Button*, int> CallbackFunctor;
	typedef Common::SharedPtr<CallbackFunctor> Callback;

	Button() : nextButton(0), index(0), keyCode(0), keyCode2(0), data0Val1(0), data1Val1(0), data2Val1(0), flags(0),
		data0ShapePtr(0), data1ShapePtr(0), data2ShapePtr(0), data0Callback(), data1Callback(), data2Callback(),
		dimTableIndex(0), x(0), y(0), width(0), height(0), data0Val2(0), data0Val3(0), data1Val2(0), data1Val3(0),
		data2Val2(0), data2Val3(0), flags2(0), mouseWheel(0), buttonCallback(), arg(0) {}

	Button *nextButton;
	uint16 index;

	uint16 keyCode;
	uint16 keyCode2;

	byte data0Val1;
	byte data1Val1;
	byte data2Val1;

	uint16 flags;

	const uint8 *data0ShapePtr;
	const uint8 *data1ShapePtr;
	const uint8 *data2ShapePtr;
	Callback data0Callback;
	Callback data1Callback;
	Callback data2Callback;

	uint16 dimTableIndex;

	int16 x, y;
	uint16 width, height;

	uint8 data0Val2;
	uint8 data0Val3;

	uint8 data1Val2;
	uint8 data1Val3;

	uint8 data2Val2;
	uint8 data2Val3;

	uint16 flags2;

	int8 mouseWheel;

	Callback buttonCallback;

	uint16 arg;
};

struct MenuItem {
	bool enabled;

	const char *itemString;
	uint16 itemId;

	int16 x, y;
	uint16 width, height;

	uint8 textColor, highlightColor;

	int16 titleX;

	uint8 color1, color2;
	uint8 bkgdColor;

	Button::Callback callback;

	int16 saveSlot;

	const char *labelString;
	uint16 labelId;
	int16 labelX, labelY;

	uint16 keyCode;
};

struct Menu {
	int16 x, y;
	uint16 width, height;

	uint8 bkgdColor;
	uint8 color1, color2;

	const char *menuNameString;
	uint16 menuNameId;

	uint8 textColor;
	int16 titleX, titleY;

	uint8 highlightedItem;

	uint8 numberOfItems;

	int16 scrollUpButtonX, scrollUpButtonY;
	int16 scrollDownButtonX, scrollDownButtonY;

	MenuItem item[7];
};

class Screen;
class TextDisplayer;

class GUI {
public:
	GUI(KyraEngine_v1 *vm);
	virtual ~GUI() {}

	// button specific
	virtual Button *addButtonToList(Button *list, Button *newButton);

	virtual void processButton(Button *button) = 0;
	virtual int processButtonList(Button *buttonList, uint16 inputFlags, int8 mouseWheel) = 0;

	virtual int redrawShadedButtonCallback(Button *button);
	virtual int redrawButtonCallback(Button *button);

	// menu specific
	virtual void initMenuLayout(Menu &menu);
	void initMenu(Menu &menu);

	void processHighlights(Menu &menu);

	// utilities for thumbnail creation
	virtual void createScreenThumbnail(Graphics::Surface &dst) = 0;

protected:
	KyraEngine_v1 *_vm;
	Screen *_screen;
	TextDisplayer *_text;

	Button *_menuButtonList;
	bool _displayMenu;
	bool _displaySubMenu;
	bool _cancelSubMenu;

	virtual void printMenuText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2, Screen::FontId font=Screen::FID_8_FNT);
	virtual int getMenuCenterStringX(const char *str, int x1, int x2);

	Button::Callback _redrawShadedButtonFunctor;
	Button::Callback _redrawButtonFunctor;

	virtual Button *getButtonListData() = 0;
	virtual Button *getScrollUpButton() = 0;
	virtual Button *getScrollDownButton() = 0;

	virtual Button::Callback getScrollUpButtonHandler() const = 0;
	virtual Button::Callback getScrollDownButtonHandler() const = 0;

	virtual uint8 defaultColor1() const = 0;
	virtual uint8 defaultColor2() const = 0;

	virtual const char *getMenuTitle(const Menu &menu) = 0;
	virtual const char *getMenuItemTitle(const MenuItem &menuItem) = 0;
	virtual const char *getMenuItemLabel(const MenuItem &menuItem) = 0;

	void updateAllMenuButtons();
	void updateMenuButton(Button *button);
	virtual void updateButton(Button *button);

	void redrawText(const Menu &menu);
	void redrawHighlight(const Menu &menu);

	Common::Array<int> _saveSlots;
	void updateSaveList(bool excludeQuickSaves = false);
	int getNextSavegameSlot();

	uint32 _lastScreenUpdate;
	Common::KeyState _keyPressed;
	void checkTextfieldInput();
};

class Movie;

class MainMenu {
public:
	MainMenu(KyraEngine_v1 *vm);
	virtual ~MainMenu() {}

	struct Animation {
		Animation() : anim(0), startFrame(0), endFrame(0), delay(0) {}

		Movie *anim;
		int startFrame;
		int endFrame;
		int delay;
	};

	struct StaticData {
		const char *strings[5];

		uint8 menuTable[7];
		uint8 colorTable[4];

		Screen::FontId font;
		uint8 altColor;
	};

	void init(StaticData data, Animation anim);
	int handle(int dim);
private:
	KyraEngine_v1 *_vm;
	Screen *_screen;
	OSystem *_system;

	StaticData _static;
	struct AnimIntern {
		int curFrame;
		int direction;
	};
	Animation _anim;
	AnimIntern _animIntern;

	uint32 _nextUpdate;

	void updateAnimation();
	void draw(int select);
	void drawBox(int x, int y, int w, int h, int fill);
	bool getInput();

	void printString(const char *string, int x, int y, int col1, int col2, int flags, ...);
};

} // end of namesapce Kyra

#endif

