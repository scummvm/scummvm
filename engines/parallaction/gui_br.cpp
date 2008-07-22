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


#include "parallaction/input.h"
#include "parallaction/parallaction.h"

namespace Parallaction {

enum MenuOptions {
	kMenuPart0 = 0,
	kMenuPart1 = 1,
	kMenuPart2 = 2,
	kMenuPart3 = 3,
	kMenuPart4 = 4,
	kMenuLoadGame = 5,
	kMenuQuit = 6
};


void Parallaction_br::guiStart() {

	// TODO: load progress value from special save game
	_progress = 3;

	int option = guiShowMenu();
	switch (option) {
	case kMenuQuit:
		_engineFlags |= kEngineQuit;
		break;

	case kMenuLoadGame:
		warning("loadgame not yet implemented");
		break;

	default:
		_part = option;
		_disk->selectArchive(_partNames[_part]);
		startPart();
	}
}

void Parallaction_br::guiSplash(const char *name) {

	_gfx->clearScreen();
	_gfx->setBackground(kBackgroundSlide, name, 0, 0);
	_gfx->_backgroundInfo.x = (_screenWidth - _gfx->_backgroundInfo.width) >> 1;
	_gfx->_backgroundInfo.y = (_screenHeight - _gfx->_backgroundInfo.height) >> 1;
	_gfx->updateScreen();
	_system->delayMillis(600);

	Palette blackPal;
	Palette pal(_gfx->_backgroundInfo.palette);
	for (uint i = 0; i < 64; i++) {
		pal.fadeTo(blackPal, 1);
		_gfx->setPalette(pal);
		_gfx->updateScreen();
		_system->delayMillis(20);
	}

}

#define MENUITEMS_X			250
#define MENUITEMS_Y			200

#define MENUITEM_WIDTH		190
#define MENUITEM_HEIGHT		18

Frames* Parallaction_br::guiRenderMenuItem(const char *text) {
	// this builds a surface containing two copies of the text.
	// one is in normal color, the other is inverted.
	// the two 'frames' are used to display selected/unselected menu items

	Graphics::Surface *surf = new Graphics::Surface;
	surf->create(MENUITEM_WIDTH, MENUITEM_HEIGHT*2, 1);

	// build first frame to be displayed when item is not selected
	if (getPlatform() == Common::kPlatformPC) {
		_menuFont->setColor(0);
	} else {
		_menuFont->setColor(7);
	}
	_menuFont->drawString((byte*)surf->getBasePtr(5, 2), MENUITEM_WIDTH, text);

	// build second frame to be displayed when item is selected
	_menuFont->drawString((byte*)surf->getBasePtr(5, 2 + MENUITEM_HEIGHT), MENUITEM_WIDTH, text);
	byte *s = (byte*)surf->getBasePtr(0, MENUITEM_HEIGHT);
	for (int i = 0; i < surf->w * MENUITEM_HEIGHT; i++) {
		*s++ ^= 0xD;
	}

	// wrap the surface into the suitable Frames adapter
	return new SurfaceToMultiFrames(2, MENUITEM_WIDTH, MENUITEM_HEIGHT, surf);
}


int Parallaction_br::guiShowMenu() {
	// TODO: filter menu entries according to progress in game

	#define NUM_MENULINES	7
	GfxObj *_lines[NUM_MENULINES];

	const char *menuStrings[NUM_MENULINES] = {
		"SEE INTRO",
		"NEW GAME",
		"SAVED GAME",
		"EXIT TO DOS",
		"PART 2",
		"PART 3",
		"PART 4"
	};

	MenuOptions options[NUM_MENULINES] = {
		kMenuPart0,
		kMenuPart1,
		kMenuLoadGame,
		kMenuQuit,
		kMenuPart2,
		kMenuPart3,
		kMenuPart4
	};

	_gfx->clearScreen();
	_gfx->setBackground(kBackgroundSlide, "tbra", 0, 0);
	if (getPlatform() == Common::kPlatformPC) {
		_gfx->_backgroundInfo.x = 20;
		_gfx->_backgroundInfo.y = 50;
	}

	int availItems = 4 + _progress;

	// TODO: keep track of and destroy menu item frames/surfaces

	int i;
	for (i = 0; i < availItems; i++) {
		_lines[i] = new GfxObj(0, guiRenderMenuItem(menuStrings[i]), "MenuItem");
		uint id = _gfx->setItem(_lines[i], MENUITEMS_X, MENUITEMS_Y + MENUITEM_HEIGHT * i, 0xFF);
		_gfx->setItemFrame(id, 0);
	}

	int selectedItem = -1;

	setMousePointer(0);

	uint32 event;
	Common::Point p;
	while (true) {

		_input->readInput();

		event = _input->getLastButtonEvent();
		if ((event == kMouseLeftUp) && selectedItem >= 0)
			break;

		_input->getCursorPos(p);

		if ((p.x > MENUITEMS_X) && (p.x < (MENUITEMS_X+MENUITEM_WIDTH)) && (p.y > MENUITEMS_Y)) {
			selectedItem = (p.y - MENUITEMS_Y) / MENUITEM_HEIGHT;

			if (!(selectedItem < availItems))
				selectedItem = -1;
		} else
			selectedItem = -1;

		for (i = 0; i < availItems; i++) {
			_gfx->setItemFrame(i, selectedItem == i ? 1 : 0);
		}

		_gfx->updateScreen();
		_system->delayMillis(20);
	}

	_system->showMouse(false);
	hideDialogueStuff();

	for (i = 0; i < availItems; i++) {
		delete _lines[i];
	}

	return options[selectedItem];
}

} // namespace Parallaction

