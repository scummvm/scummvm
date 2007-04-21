/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "parallaction/menu.h"
#include "parallaction/disk.h"
#include "parallaction/music.h"
#include "parallaction/graphics.h"
#include "parallaction/parallaction.h"


namespace Parallaction {

const char *introMsg1[] = {
	"INSERISCI IL CODICE",
	"ENTREZ CODE",
	"ENTER CODE",
	"GIB DEN KODE EIN"
};

const char *introMsg2[] = {
	"CODICE ERRATO",
	"CODE ERRONE",
	"WRONG CODE",
	"GIB DEN KODE EIN"
};

const char *introMsg3[] = {
	"PRESS LEFT MOUSE BUTTON",
	"TO SEE INTRO",
	"PRESS RIGHT MOUSE BUTTON",
	"TO START"
};

const char *newGameMsg[] = {
	"NUOVO GIOCO",
	"NEUF JEU",
	"NEW GAME",
	"NEUES SPIEL"
};

const char *loadGameMsg[] = {
	"GIOCO SALVATO",
	"JEU SAUVE'",
	"SAVED GAME",
	"SPIEL GESPEICHERT"
};


#define BLOCK_WIDTH 	16
#define BLOCK_HEIGHT	24

#define BLOCK_X 		112
#define BLOCK_Y 		130

#define BLOCK_SELECTION_X		  (BLOCK_X-1)
#define BLOCK_SELECTION_Y		  (BLOCK_Y-1)

#define BLOCK_X_OFFSET	(BLOCK_WIDTH+1)
#define BLOCK_Y_OFFSET	9

//	destination slots for code blocks
//
#define SLOT_X			61
#define SLOT_Y			64
#define SLOT_WIDTH		(BLOCK_WIDTH+2)


static uint16 _dinoKey[] = { 5, 3, 6, 1, 4, 7 }; //
static uint16 _donnaKey[] = { 0, 2, 8, 5, 5, 1 };
static uint16 _doughKey[] = { 1, 7 ,7, 2, 2, 6 };


Menu::Menu(Parallaction *vm) {
	_vm = vm;
}

Menu::~Menu() {

}


void Menu::start() {

	_vm->_disk->selectArchive((_vm->getPlatform() == Common::kPlatformPC) ? "disk1" : "disk0");

	_vm->_gfx->_proportionalFont = false;

	_vm->_disk->loadSlide("intro");
	_vm->_gfx->setPalette(_vm->_gfx->_palette);
	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
	g_system->delayMillis(2000);

	_vm->_disk->loadSlide("minintro");
	_vm->_gfx->setPalette(_vm->_gfx->_palette);
	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
	g_system->delayMillis(2000);

	if (_vm->getPlatform() == Common::kPlatformPC) {
		_vm->_gfx->setFont(kFontMenu);

		_vm->_disk->loadSlide("lingua");
		_vm->_gfx->setPalette(_vm->_gfx->_palette);
		_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

		_vm->_gfx->displayString(60, 30, "SELECT LANGUAGE");

		_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);
		_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBit2);
		_language = chooseLanguage();

		_vm->_disk->setLanguage(_language);

		if (selectGame() == 0) {
			newGame();
		}
	} else {
		_vm->_disk->setLanguage(1);
	}

	return;
}

void Menu::newGame() {

	const char **v14 = introMsg3;

	_vm->_disk->loadScenery("test", NULL);
	_vm->_gfx->setPalette(_vm->_gfx->_palette);
	_vm->_gfx->swapBuffers();

	uint16 _ax = (SCREEN_WIDTH - _vm->_gfx->getStringWidth(v14[0])) / 2;
	_vm->_gfx->displayString(_ax, 50, v14[0]);

	_ax = (SCREEN_WIDTH - _vm->_gfx->getStringWidth(v14[1])) / 2;
	_vm->_gfx->displayString(_ax, 70, v14[1]);

	_ax = (SCREEN_WIDTH - _vm->_gfx->getStringWidth(v14[2])) / 2;
	_vm->_gfx->displayString(_ax, 100, v14[2]);

	_ax = (SCREEN_WIDTH - _vm->_gfx->getStringWidth(v14[3])) / 2;
	_vm->_gfx->displayString(_ax, 120, v14[3]);

	_vm->_gfx->updateScreen();
	_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);


	_mouseButtons = kMouseNone;

	for (; _mouseButtons != kMouseLeftUp; ) {
		_vm->updateInput();
		if (_mouseButtons == kMouseRightUp) break;
	}

	if (_mouseButtons != kMouseRightUp)
		return;    // show intro

	selectCharacter();

	char *v4 = strchr(_vm->_location._name, '.') + 1;
	strcpy(_vm->_characterName, v4);

	return; // start game
}

uint16 Menu::chooseLanguage() {

	_vm->changeCursor(kCursorArrow);

	do {
		_vm->updateInput();
		_vm->_gfx->swapBuffers();

		if (_mouseButtons == kMouseLeftUp) {
			for (uint16 _si = 0; _si < 4; _si++) {

				if (80 + _si*49 >= _vm->_mousePos.x) continue;
				if (110 - _si*25 >= _vm->_mousePos.y) continue;

				if (128 + _si*49 <= _vm->_mousePos.x) continue;
				if (180 - _si*25 <=_vm->_mousePos.y) continue;

//				beep();
				return _si;
			}
		}

		_vm->waitTime( 1 );

	} while (true);

	// never reached !!!
	return 0;
}



uint16 Menu::selectGame() {
//	  printf("selectGame()\n");


	_vm->_disk->loadSlide("restore");
	_vm->_gfx->setPalette(_vm->_gfx->_palette);
	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBit2);


	uint16 _si = 0;
	uint16 _di = 3;

	_vm->updateInput();
	while (_mouseButtons != kMouseLeftUp) {

		_vm->updateInput();
		_vm->_gfx->swapBuffers();
		_vm->waitTime( 1 );

		_si = 0;
		if (_vm->_mousePos.x > 160)
			_si = 1;

		if (_si == _di) continue;

		_di = _si;
		_vm->_gfx->copyScreen(Gfx::kBit2, Gfx::kBitFront);

		if (_si != 0) {
			// load a game
			_vm->_gfx->displayString(60, 30, loadGameMsg[_language]);
		} else {
			// new game
			_vm->_gfx->displayString(60, 30, newGameMsg[_language]);
		}

		_vm->_gfx->updateScreen();
		_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);

	}

	if (_si == 0) return 0; // new game

	// load game

	strcpy(_vm->_location._name, "fogne");
	strcpy(_vm->_characterName, "dough");

	_vm->loadGame();

	return 1;  // load game
}


//
//	character selection and protection
//
void Menu::selectCharacter() {

	uint16 _di = 0;
	bool askPassword = true;

	uint16 _donna_points = 0;
	uint16 _dino_points = 0;
	uint16 _dough_points = 0;

	StaticCnv v14;

	v14._data0 = (byte*)malloc(BLOCK_WIDTH*BLOCK_HEIGHT);
	v14._width = BLOCK_WIDTH;
	v14._height = BLOCK_HEIGHT;

	_vm->changeCursor(kCursorArrow);
	_vm->_midiPlayer->stop();

	_vm->_gfx->_proportionalFont = false;
	_vm->_gfx->setFont(kFontMenu);

	_vm->_disk->selectArchive((_vm->getPlatform() == Common::kPlatformPC) ? "disk1" : "disk0");

	_vm->_disk->loadSlide("password");	// loads background into kBitBack buffer

	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);	//
	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBit2);		//
	_vm->_gfx->setPalette(_vm->_gfx->_palette);

	while (askPassword == true) {

		askPassword = false;
		_di = 0;

		_vm->_gfx->displayString(60, 30, introMsg1[_language]);			// displays message
		_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);

		while (_di < 6) {

			_mouseButtons = kMouseNone;
			do {
				_vm->updateInput();
				_vm->_gfx->swapBuffers();
				_vm->waitTime(1);
			} while (_mouseButtons != kMouseLeftUp);	// waits for left click

			for (uint16 _si = 0; _si < 9; _si++) {

				Common::Rect r(
					_si * BLOCK_X_OFFSET + BLOCK_SELECTION_X,
					BLOCK_SELECTION_Y - _si * BLOCK_Y_OFFSET,
					(_si + 1) * BLOCK_X_OFFSET + BLOCK_SELECTION_X,
					BLOCK_SELECTION_Y + BLOCK_HEIGHT - _si * BLOCK_Y_OFFSET
				);

				if (!r.contains(_vm->_mousePos)) continue;

				r.setWidth(BLOCK_WIDTH);
				r.setHeight(BLOCK_HEIGHT);
				r.moveTo(_si * BLOCK_X_OFFSET + BLOCK_X, BLOCK_Y - _si * BLOCK_Y_OFFSET);
				_vm->_gfx->grabRect(v14._data0, r, Gfx::kBitFront, BLOCK_WIDTH);

				_vm->_gfx->flatBlitCnv(&v14, _di * SLOT_WIDTH + SLOT_X, SLOT_Y, Gfx::kBitBack);
				_vm->_gfx->flatBlitCnv(&v14, _di * SLOT_WIDTH + SLOT_X, SLOT_Y, Gfx::kBitFront);

//				beep();

				if (_dinoKey[_di] == _si)
					_dino_points++;  // dino
				if (_donnaKey[_di] == _si)
					_donna_points++;  // donna
				if (_doughKey[_di] == _si)
					_dough_points++;  // dough

				_di++;
			}

			askPassword = (_dino_points < 6 && _donna_points < 6 && _dough_points < 6);
		}

		if (askPassword == false) break;

		_vm->_gfx->copyScreen(Gfx::kBit2, Gfx::kBitFront);
		_vm->_gfx->displayString(60, 30, introMsg2[_language]);
		_vm->_gfx->updateScreen();

		g_system->delayMillis(2000);

		_vm->_gfx->copyScreen(Gfx::kBit2, Gfx::kBitFront);
	}


	if (_dino_points > _donna_points && _dino_points > _dough_points) {
		sprintf(_vm->_location._name, "test.%s", _dinoName);
	} else {
		if (_donna_points > _dino_points && _donna_points > _dough_points) {
			sprintf(_vm->_location._name, "test.%s", _donnaName);
		} else {
			sprintf(_vm->_location._name, "test.%s", _doughName);
		}
	}

	_vm->_gfx->setBlackPalette();

	_engineFlags |= kEngineChangeLocation;

	free(v14._data0);

	return;

}


} // namespace Parallaction
