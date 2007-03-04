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

#include "parallaction/menu.h"
#include "parallaction/disk.h"
#include "parallaction/music.h"
#include "parallaction/inventory.h"
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


Menu::Menu(Parallaction *engine) {
	_engine = engine;
}

Menu::~Menu() {
	//
}


void Menu::start() {

	_vm->_archive.open("disk1");
	_vm->_graphics->_proportionalFont = false;

	_vm->_graphics->setFont("slide");

	loadSlide("intro");
	_vm->_graphics->palUnk0(_palette);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);

	g_system->delayMillis(2000);

	loadSlide("minintro");
	_vm->_graphics->palUnk0(_palette);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);

	g_system->delayMillis(2000);

	loadSlide("lingua");
	_vm->_graphics->palUnk0(_palette);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);

	_vm->_graphics->displayString(60, 30, "SELECT LANGUAGE");

	_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBitBack);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBit2);
	_language = chooseLanguage();

	switch (_language) {
	case 0:
		strcpy(_engine->_languageDir, "it/");
		break;

	case 1:
		strcpy(_engine->_languageDir, "fr/");
		break;

	case 2:
		strcpy(_engine->_languageDir, "en/");
		break;

	case 3:
		strcpy(_engine->_languageDir, "ge/");
		break;
	}

	loadSlide("restore");
	_vm->_graphics->palUnk0(_palette);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);

	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBit2);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBit3);

	if (selectGame() == 0) {
		newGame();
	}

	_vm->_archive.close();

	return;
}

void Menu::newGame() {
//	  printf("newGame()\n");

	const char **v14 = introMsg3;

	loadScenery("test", NULL);
	_vm->_graphics->palUnk0(_palette);
	_vm->_graphics->swapBuffers();

	uint16 _ax = (SCREEN_WIDTH - _vm->_graphics->getStringWidth(v14[0])) / 2;
	_vm->_graphics->displayString(_ax, 50, v14[0]);

	_ax = (SCREEN_WIDTH - _vm->_graphics->getStringWidth(v14[1])) / 2;
	_vm->_graphics->displayString(_ax, 70, v14[1]);

	_ax = (SCREEN_WIDTH - _vm->_graphics->getStringWidth(v14[2])) / 2;
	_vm->_graphics->displayString(_ax, 100, v14[2]);

	_ax = (SCREEN_WIDTH - _vm->_graphics->getStringWidth(v14[3])) / 2;
	_vm->_graphics->displayString(_ax, 120, v14[3]);

	_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBitBack);


	_mouseButtons = kMouseNone;

	for (; _mouseButtons != kMouseLeftUp; ) {
		_vm->updateInput();
		if (_mouseButtons == kMouseRightUp) break;
	}

	if (_mouseButtons != kMouseRightUp)
		return;    // show intro

	_vm->_archive.close();

	selectCharacter();

	char *v4 = strchr(_location, '.') + 1;
	strcpy(_engine->_characterName, v4);

	return; // start game
}

uint16 Menu::chooseLanguage() {

	_engine->changeCursor(kCursorArrow);

	do {
		_engine->updateInput();
		_vm->_graphics->swapBuffers();

		uint16 _di = _mousePos._x;
		uint16 v2 = _mousePos._y;

		if (_mouseButtons == kMouseLeftUp) {
			for (uint16 _si = 0; _si < 4; _si++) {

				if (80 + _si*49 >= _di) continue;
				if (110 - _si*25 >= v2) continue;

				if (128 + _si*49 <= _di) continue;
				if (180 - _si*25 <= v2) continue;

				beep();
				return _si;
			}
		}

		_engine->waitTime( 1 );

	} while (true);

	// never reached !!!
	return 0;
}



uint16 Menu::selectGame() {
//	  printf("selectGame()\n");

	uint16 _si = 0;
	uint16 _di = 3;

	_engine->updateInput();
	while (_mouseButtons != kMouseLeftUp) {

		_engine->updateInput();
		_vm->_graphics->swapBuffers();
		_engine->waitTime( 1 );

		_si = 0;
		if (_mousePos._x > 160)
			_si = 1;

		if (_si == _di) continue;

		_di = _si;
		_vm->_graphics->copyScreen(Graphics::kBit3, Graphics::kBitFront);

		if (_si != 0) {
			// load a game
			_vm->_graphics->displayString(60, 30, loadGameMsg[_language]);
		} else {
			// new game
			_vm->_graphics->displayString(60, 30, newGameMsg[_language]);
		}

		_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBit2);
		_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBitBack);

	}

	if (_si == 0) return 0; // new game

	// load game

	strcpy(_location, "fogne");
	strcpy(_engine->_characterName, "dough");

	_vm->loadGame();
	_vm->_archive.close();

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

	v14._data0 = (byte*)malloc(33);
	v14._data2 = v14._data0;
	v14._width = BLOCK_WIDTH;
	v14._height = BLOCK_HEIGHT;

	_engine->changeCursor(kCursorArrow);
	_vm->_midiPlayer->stop();
	_vm->_graphics->_proportionalFont = false;

	_vm->_graphics->setFont("slide");
	_vm->_archive.open("disk1");

	loadSlide("password");
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBit2);
	_vm->_graphics->palUnk0(_palette);

	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBit3);

	while (askPassword == true) {

		askPassword = false;
		_di = 0;
		_vm->_graphics->displayString(60, 30, introMsg1[_language]);
		_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBit2);
		_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBitBack);
		_mouseButtons = kMouseNone;

		while (_di < 6) {
			do {
				_engine->updateInput();
				//job_eraseMouse();
				//job_drawMouse();
				_vm->_graphics->swapBuffers();
				_engine->waitTime(1);

			} while (_mouseButtons != kMouseLeftUp);


			_mouseButtons = kMouseNone;
			uint16 x = _mousePos._x;
			uint16 y = _mousePos._y;

			uint16 _si = 0;

			while (_si < 9) {

				if ((_si * BLOCK_X_OFFSET + BLOCK_SELECTION_X < x) &&
					((_si + 1) * BLOCK_X_OFFSET + BLOCK_SELECTION_X > x) &&
					(BLOCK_SELECTION_Y - _si * BLOCK_Y_OFFSET < y) &&
					(BLOCK_SELECTION_Y + BLOCK_HEIGHT - _si * BLOCK_Y_OFFSET > y)) {

					_vm->_graphics->backupCnvBackground(&v14, _si * BLOCK_X_OFFSET + BLOCK_X, BLOCK_Y - _si * BLOCK_Y_OFFSET);

					_vm->_graphics->flatBlitCnv(&v14, _di * SLOT_WIDTH + SLOT_X, SLOT_Y, Graphics::kBitBack, 0);
					_vm->_graphics->flatBlitCnv(&v14, _di * SLOT_WIDTH + SLOT_X, SLOT_Y, Graphics::kBitFront, 0);
					_vm->_graphics->flatBlitCnv(&v14, _di * SLOT_WIDTH + SLOT_X, SLOT_Y, Graphics::kBit2, 0);

					beep();

					if (_dinoKey[_di] == _si) {
						_dino_points++;  // dino
					} else
					if (_donnaKey[_di] == _si) {
						_donna_points++;  // donna
					} else
					if (_doughKey[_di] == _si) {
						_dough_points++;  // dough
					} else
						askPassword = true;

					_di++;
				}

				_si++;

			}
		}

		if (askPassword == false) continue;

		_vm->_graphics->copyScreen(Graphics::kBit3, Graphics::kBitFront);
		_vm->_graphics->displayString(60, 30, introMsg1[_language]);

		g_system->delayMillis(2000);

		_vm->_graphics->copyScreen(Graphics::kBit3, Graphics::kBitFront);
	}


	strcpy(_location, "test");
	if (_dino_points > _donna_points && _dino_points > _dough_points) {
		strcat(_location, ".dino");
	} else {
		if (_donna_points > _dino_points && _donna_points > _dough_points) {
			strcat(_location, ".donna");
		} else {
			strcat(_location, ".dough");
		}
	}

	byte palette[PALETTE_SIZE];
	_vm->_graphics->getBlackPalette(palette);
	_vm->_graphics->setPalette(palette);

	_engineFlags |= kEngineChangeLocation;
	_vm->_archive.close();

	free(v14._data0);

	refreshInventory(_vm->_characterName);

	return;

}


} // namespace Parallaction
