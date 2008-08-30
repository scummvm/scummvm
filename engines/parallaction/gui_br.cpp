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

#include "common/system.h"

#include "parallaction/gui.h"
#include "parallaction/input.h"
#include "parallaction/parallaction.h"

namespace Parallaction {

class SplashInputState_BR : public MenuInputState {
protected:
	Common::String _slideName;
	uint32 _timeOut;
	Common::String _nextState;
	uint32	_startTime;
	Palette blackPal;
	Palette pal;

	Parallaction_br *_vm;
	int _fadeSteps;

public:
	SplashInputState_BR(Parallaction_br *vm, const Common::String &name, MenuInputHelper *helper) : MenuInputState(name, helper), _vm(vm)  {
	}

	virtual MenuInputState* run() {
		if (_fadeSteps > 0) {
			pal.fadeTo(blackPal, 1);
			_vm->_gfx->setPalette(pal);
			_fadeSteps--;
			return this;
		}

		if (_fadeSteps == 0) {
			_vm->freeBackground();
			return _helper->getState(_nextState);
		}

		uint32 curTime = _vm->_system->getMillis();
		if (curTime - _startTime > _timeOut) {
			_fadeSteps = 64;
			pal.clone(_vm->_gfx->_backgroundInfo->palette);
		}
		return this;
	}

	virtual void enter() {
		_vm->_gfx->clearScreen();
		_vm->showSlide(_slideName.c_str(), CENTER_LABEL_HORIZONTAL, CENTER_LABEL_VERTICAL);
		_vm->_input->setMouseState(MOUSE_DISABLED);

		_startTime = g_system->getMillis();
		_fadeSteps = -1;
	}
};

class SplashInputState0_BR : public SplashInputState_BR {

public:
	SplashInputState0_BR(Parallaction_br *vm, MenuInputHelper *helper) : SplashInputState_BR(vm, "intro0", helper)  {
		_slideName = "dyna";
		_timeOut = 600;
		_nextState = "intro1";
	}
};

class SplashInputState1_BR : public SplashInputState_BR {

public:
	SplashInputState1_BR(Parallaction_br *vm, MenuInputHelper *helper) : SplashInputState_BR(vm, "intro1", helper) {
		_slideName = "core";
		_timeOut = 600;
		_nextState = "mainmenu";
	}
};

class MainMenuInputState_BR : public MenuInputState {
	Parallaction_br *_vm;

	#define MENUITEMS_X			250
	#define MENUITEMS_Y			200

	#define MENUITEM_WIDTH		190
	#define MENUITEM_HEIGHT		18

	Frames* renderMenuItem(const char *text) {
		// this builds a surface containing two copies of the text.
		// one is in normal color, the other is inverted.
		// the two 'frames' are used to display selected/unselected menu items

		Graphics::Surface *surf = new Graphics::Surface;
		surf->create(MENUITEM_WIDTH, MENUITEM_HEIGHT*2, 1);

		// build first frame to be displayed when item is not selected
		if (_vm->getPlatform() == Common::kPlatformPC) {
			_vm->_menuFont->setColor(0);
		} else {
			_vm->_menuFont->setColor(7);
		}
		_vm->_menuFont->drawString((byte*)surf->getBasePtr(5, 2), MENUITEM_WIDTH, text);

		// build second frame to be displayed when item is selected
		_vm->_menuFont->drawString((byte*)surf->getBasePtr(5, 2 + MENUITEM_HEIGHT), MENUITEM_WIDTH, text);
		byte *s = (byte*)surf->getBasePtr(0, MENUITEM_HEIGHT);
		for (int i = 0; i < surf->w * MENUITEM_HEIGHT; i++) {
			*s++ ^= 0xD;
		}

		// wrap the surface into the suitable Frames adapter
		return new SurfaceToMultiFrames(2, MENUITEM_WIDTH, MENUITEM_HEIGHT, surf);
	}

	enum MenuOptions {
		kMenuPart0 = 0,
		kMenuPart1 = 1,
		kMenuPart2 = 2,
		kMenuPart3 = 3,
		kMenuPart4 = 4,
		kMenuLoadGame = 5,
		kMenuQuit = 6
	};

	#define NUM_MENULINES	7
	GfxObj *_lines[NUM_MENULINES];

	static const char *_menuStrings[NUM_MENULINES];
	static const MenuOptions _options[NUM_MENULINES];

	int _availItems;
	int _selection;

	void cleanup() {
		_vm->_system->showMouse(false);
		_vm->hideDialogueStuff();

		for (int i = 0; i < _availItems; i++) {
			delete _lines[i];
		}
	}

	void performChoice(int selectedItem) {
		switch (selectedItem) {
		case kMenuQuit:
			_engineFlags |= kEngineQuit;
			break;

		case kMenuLoadGame:
			warning("loadgame not yet implemented");
			break;

		default:
			_vm->startPart(selectedItem);
		}
	}

public:
	MainMenuInputState_BR(Parallaction_br *vm, MenuInputHelper *helper) : MenuInputState("mainmenu", helper), _vm(vm)  {
	}

	virtual MenuInputState* run() {

		int event = _vm->_input->getLastButtonEvent();
		if ((event == kMouseLeftUp) && _selection >= 0) {
			cleanup();
			performChoice(_options[_selection]);
			return 0;
		}

		Common::Point p;
		_vm->_input->getCursorPos(p);

		if ((p.x > MENUITEMS_X) && (p.x < (MENUITEMS_X+MENUITEM_WIDTH)) && (p.y > MENUITEMS_Y)) {
			_selection = (p.y - MENUITEMS_Y) / MENUITEM_HEIGHT;

			if (!(_selection < _availItems))
				_selection = -1;
		} else
			_selection = -1;

		for (int i = 0; i < _availItems; i++) {
			_vm->_gfx->setItemFrame(i, _selection == i ? 1 : 0);
		}


		return this;
	}

	virtual void enter() {
		_vm->_gfx->clearScreen();
		int x = 0, y = 0;
		if (_vm->getPlatform() == Common::kPlatformPC) {
			x = 20;
			y = 50;
		}
		_vm->showSlide("tbra", x, y);

		// TODO: load progress from savefile
		int progress = 3;
		_availItems = 4 + progress;

		// TODO: keep track of and destroy menu item frames/surfaces
		int i;
		for (i = 0; i < _availItems; i++) {
			_lines[i] = new GfxObj(0, renderMenuItem(_menuStrings[i]), "MenuItem");
			uint id = _vm->_gfx->setItem(_lines[i], MENUITEMS_X, MENUITEMS_Y + MENUITEM_HEIGHT * i, 0xFF);
			_vm->_gfx->setItemFrame(id, 0);
		}
		_selection = -1;
		_vm->_input->setArrowCursor();
		_vm->_input->setMouseState(MOUSE_ENABLED_SHOW);
	}

};

const char *MainMenuInputState_BR::_menuStrings[NUM_MENULINES] = {
	"SEE INTRO",
	"NEW GAME",
	"SAVED GAME",
	"EXIT TO DOS",
	"PART 2",
	"PART 3",
	"PART 4"
};

const MainMenuInputState_BR::MenuOptions MainMenuInputState_BR::_options[NUM_MENULINES] = {
	kMenuPart0,
	kMenuPart1,
	kMenuLoadGame,
	kMenuQuit,
	kMenuPart2,
	kMenuPart3,
	kMenuPart4
};







void Parallaction_br::startGui(bool showSplash) {
	_menuHelper = new MenuInputHelper;

	new MainMenuInputState_BR(this, _menuHelper);

	if (showSplash) {
		new SplashInputState0_BR(this, _menuHelper);
		new SplashInputState1_BR(this, _menuHelper);
		_menuHelper->setState("intro0");
	} else {
		_menuHelper->setState("mainmenu");
	}

	_input->_inputMode = Input::kInputModeMenu;
}




} // namespace Parallaction

