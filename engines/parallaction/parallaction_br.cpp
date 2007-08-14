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

#include "common/stdafx.h"
#include "common/system.h"

#include "parallaction/parallaction.h"
#include "parallaction/sound.h"

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

const char *partNames[] = {
	"PART0",
	"PART1",
	"PART2",
	"PART3",
	"PART4"
};

const char *partFirstLocation[] = {
	"intro",
	"museo",
	"start",
	"bolscoi",
	"treno"
};

int Parallaction_br::init() {

	// Detect game
	if (!detectGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	_screenWidth = 640;
	_screenHeight = 400;

	if (getGameType() == GType_BRA) {
		if (getPlatform() == Common::kPlatformPC) {
			_disk = new DosDisk_br(this);
			_disk->setLanguage(2);					// NOTE: language is now hardcoded to English. Original used command-line parameters.
		} else
			error("unsupported platform for Big Red Adventure");
	} else
		error("unknown game type");

	_soundMan = new DummySoundMan(this);

	initResources();
	initFonts();
	initCursors();

	_part = -1;

	Parallaction::init();

	return 0;
}

Parallaction_br::~Parallaction_br() {
	freeFonts();

	_dinoCursor->free();
	_dougCursor->free();
	_donnaCursor->free();

	delete _dinoCursor;
	delete _dougCursor;
	delete _donnaCursor;

}

void Parallaction_br::callFunction(uint index, void* parm) {
	assert(index < 6);	// magic value 6 is maximum # of callables for Big Red Adventure

	(this->*_callables[index])(parm);
}

int Parallaction_br::go() {
	splash("dyna");
	splash("core");

	// TODO: load progress value from special save game
	_progress = 3;

	while ((_engineFlags & kEngineQuit) == 0) {

//		initCharacter();

		int option = showMenu();
		switch (option) {
		case kMenuQuit:
			_engineFlags |= kEngineQuit;
			break;

		case kMenuLoadGame:
			warning("loadgame not yet implemented");
			break;

		default:
			_part = option;
			_disk->selectArchive(partNames[_part]);
			startPart();
			break;
		}


		runGame();

		freePart();
//		freeLocation();
//		freeCharacter();

	}

	return 0;
}

void Parallaction_br::splash(const char *name) {

	BackgroundInfo info;

	_gfx->clearScreen(Gfx::kBitFront);

	_disk->loadSlide(info, name);
	_gfx->setPalette(info.palette);
	_gfx->flatBlitCnv(&info.bg, (640 - info.width) >> 1, (400 - info.height) >> 1, Gfx::kBitFront);
	_gfx->updateScreen();
	_system->delayMillis(600);

	Palette pal;
	for (uint i = 0; i < 64; i++) {
		info.palette.fadeTo(pal, 1);
		_gfx->setPalette(info.palette);
		_gfx->updateScreen();
		_system->delayMillis(20);
	}
	info.bg.free();

	return;
}

#define MENUITEMS_X			250
#define MENUITEMS_Y			200

#define MENUITEM_WIDTH		190
#define MENUITEM_HEIGHT		18

void Parallaction_br::renderMenuItem(Graphics::Surface &surf, const char *text) {
	surf.create(MENUITEM_WIDTH, MENUITEM_HEIGHT, 1);
	_menuFont->setColor(0);
	_menuFont->drawString((byte*)surf.getBasePtr(5, 2), MENUITEM_WIDTH, text);
}

void Parallaction_br::invertMenuItem(Graphics::Surface &surf) {
	for (int i = 0; i < surf.w * surf.h; i++)
		((byte*)surf.pixels)[i] ^= 0xD;
}

int Parallaction_br::showMenu() {
	// TODO: filter menu entries according to progress in game

	_gfx->clearScreen(Gfx::kBitFront);

	BackgroundInfo info;

	Graphics::Surface	_menuItems[7];

	const char *menuStrings[7] = {
		"SEE INTRO",
		"NEW GAME",
		"SAVED GAME",
		"EXIT TO DOS",
		"PART 2",
		"PART 3",
		"PART 4"
	};

	MenuOptions options[7] = {
		kMenuPart0,
		kMenuPart1,
		kMenuLoadGame,
		kMenuQuit,
		kMenuPart2,
		kMenuPart3,
		kMenuPart4
	};

	_disk->loadSlide(info, "tbra");
	_gfx->setPalette(info.palette);
	_gfx->flatBlitCnv(&info.bg, 20, 50, Gfx::kBitFront);

	int availItems = 4 + _progress;

	for (int i = 0; i < availItems; i++)
		renderMenuItem(_menuItems[i], menuStrings[i]);

	int selectedItem = -1, oldSelectedItem = -2;

	setMousePointer(0);

	while (true) {

		if ((_mouseButtons == kMouseLeftUp) && selectedItem >= 0)
			break;

		updateInput();

		if ((_mousePos.x > MENUITEMS_X) && (_mousePos.x < (MENUITEMS_X+MENUITEM_WIDTH)) && (_mousePos.y > MENUITEMS_Y)) {
			selectedItem = (_mousePos.y - MENUITEMS_Y) / MENUITEM_HEIGHT;

			if (!(selectedItem < availItems))
				selectedItem = -1;
		} else
			selectedItem = -1;


		if (selectedItem != oldSelectedItem) {

			if (selectedItem >= 0 && selectedItem < availItems)
				invertMenuItem(_menuItems[selectedItem]);

			if (oldSelectedItem >= 0 && oldSelectedItem < availItems)
				invertMenuItem(_menuItems[oldSelectedItem]);

			Common::Rect r(MENUITEM_WIDTH, MENUITEM_HEIGHT);

			for (int i = 0; i < availItems; i++) {
				r.moveTo(MENUITEMS_X, MENUITEMS_Y + i * 20);
				_gfx->copyRect(Gfx::kBitFront, r, (byte*)_menuItems[i].pixels, _menuItems[i].pitch);
			}

			oldSelectedItem = selectedItem;
		}

		_gfx->updateScreen();
		_system->delayMillis(20);
	}

	_system->showMouse(false);

	info.bg.free();

	for (int i = 0; i < availItems; i++)
		_menuItems[i].free();

	return options[selectedItem];
}


void Parallaction_br::initFonts() {

	_menuFont = _disk->loadFont("russia");
	_dialogueFont = _disk->loadFont("comic");
	_labelFont = _menuFont;

}

void Parallaction_br::freeFonts() {

	delete _menuFont;
	delete _dialogueFont;

	return;
}

void Parallaction_br::initCursors() {

	_dinoCursor = _disk->loadPointer("pointer1");
	_dougCursor = _disk->loadPointer("pointer2");
	_donnaCursor = _disk->loadPointer("pointer3");

	_mouseArrow = _donnaCursor;

}

void Parallaction_br::setMousePointer(int16 index) {

	_system->setMouseCursor((byte*)_mouseArrow->pixels, _mouseArrow->w, _mouseArrow->h, 0, 0, 0);
	_system->showMouse(true);

}

void Parallaction_br::initPart() {

	_globalTable = _disk->loadTable("global");
	_objectsNames = _disk->loadTable("objects");
	_countersNames = _disk->loadTable("counters");

//	_disk->loadObjects("icone.ico");

}

void Parallaction_br::freePart() {

	delete _globalTable;
	delete _objectsNames;
	delete _countersNames;

}

void Parallaction_br::startPart() {

	initPart();

	strcpy(_location._name, partFirstLocation[_part]);

	parseLocation("common");
	parseLocation(_location._name);

}

void skip(Script* script, const char* endToken) {

	while (scumm_stricmp(_tokens[0], endToken)) {
		fillBuffers(*script, true);
	}

}
#if 0
bool Parallaction_br::parseLocationLine(const char *filename, Script *script) {

	bool parsed = true;
	bool flip = false;
	int nextToken = 0;

	if (!scumm_stricmp(_tokens[0], "LOCATION")) {

		strcpy(_location._name, _tokens[1]);
		_disk->loadScenery(*_backgroundInfo, _location._name, NULL, NULL);
		_gfx->setBackground(&_backgroundInfo->bg);
		_gfx->_palette.clone(_backgroundInfo->palette);
		_gfx->setPalette(_backgroundInfo->palette);

		if (!scumm_stricmp("flip", _tokens[2])) {
			flip = true;
			nextToken = 3;
		} else {
			nextToken = 2;
		}

		if (_tokens[nextToken][0] != '\0') {
			_char._ani._left = atoi(_tokens[nextToken]);
			nextToken++;
			_char._ani._top = atoi(_tokens[nextToken]);
			nextToken++;
		}

		if (_tokens[nextToken][0] != '\0') {
			_char._ani._frame = atoi(_tokens[nextToken]);
		}
	} else
	if (!scumm_stricmp(_tokens[0], "IFCHAR")) {
		skip(script, "ENDIF");
	} else
	if (!scumm_stricmp(_tokens[0], "CHARACTER")) {

	} else
	if (!scumm_stricmp(_tokens[0], "MASK")) {
		_disk->loadScenery(*_backgroundInfo, NULL, _tokens[1], NULL);
		_gfx->setMask(&_backgroundInfo->mask);

		_gfx->_bgLayers[0] = atoi(_tokens[2]);
		_gfx->_bgLayers[1] = atoi(_tokens[3]);
		_gfx->_bgLayers[2] = atoi(_tokens[4]);
	} else
	if (!scumm_stricmp(_tokens[0], "PATH")) {
		_disk->loadScenery(*_backgroundInfo, NULL, NULL, _tokens[1]);
		_pathBuffer = &_backgroundInfo->path;
	} else
	if (!scumm_stricmp(_tokens[0], "DISK")) {
		//
	} else
	if (!scumm_stricmp(_tokens[0], "ESCAPE")) {
		skip(script, "endcommands");
	} else
	if (!scumm_stricmp(_tokens[0], "ZETA")) {
		//
	} else
	if (!scumm_stricmp(_tokens[0], "ZONE")) {
		skip(script, "endzone");
	} else
	if (!scumm_stricmp(_tokens[0], "ANIMATION")) {
		skip(script, "endanimation");
	} else
		parsed = false;

	return parsed;
}
#endif

} // namespace Parallaction
