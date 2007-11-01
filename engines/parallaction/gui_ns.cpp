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

#include "parallaction/parallaction.h"
#include "parallaction/sound.h"


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

#define PASSWORD_LEN	6


static uint16 _amigaKeys[][PASSWORD_LEN] = {
	{ 1, 3 ,7, 2, 4, 6 },		// dough
	{ 5, 3, 6, 2, 2, 7 },		// dino
	{ 0, 3, 6, 2, 2, 6 }		// donna
};

static uint16 _pcKeys[][PASSWORD_LEN] = {
	{ 1, 7 ,7, 2, 2, 6 },		// dough
	{ 5, 3, 6, 1, 4, 7 },		// dino
	{ 0, 2, 8, 5, 5, 1 }		// donna
};

enum {
	NEW_GAME,
	LOAD_GAME
};

void Parallaction_ns::guiStart() {

	_disk->selectArchive((getFeatures() & GF_LANG_MULT) ? "disk1" : "disk0");

	guiSplash();

	_gfx->setFont(_menuFont);

	_language = guiChooseLanguage();
	_disk->setLanguage(_language);

	if (getFeatures() & GF_DEMO) {
		strcpy(_location._name, "fognedemo.dough");
		return;
	}

	if (guiSelectGame() == NEW_GAME) {
		guiNewGame();
	}

	return;
}

void Parallaction_ns::guiSplash() {

	showSlide("intro");
	_gfx->updateScreen();
	g_system->delayMillis(2000);

	showSlide("minintro");
	_gfx->updateScreen();
	g_system->delayMillis(2000);

}

void Parallaction_ns::guiNewGame() {

	const char **v14 = introMsg3;

	_disk->selectArchive("disk1");

	setBackground("test", NULL, NULL);

	_gfx->swapBuffers();

	_gfx->displayCenteredString(50, v14[0]);
	_gfx->displayCenteredString(70, v14[1]);
	_gfx->displayCenteredString(100, v14[2]);
	_gfx->displayCenteredString(120, v14[3]);

	showCursor(false);

	_gfx->updateScreen();

	_mouseButtons = kMouseNone;
	do {
		updateInput();
	} while (_mouseButtons != kMouseLeftUp && _mouseButtons != kMouseRightUp);

	showCursor(true);

	if (_mouseButtons != kMouseRightUp) {
		strcpy(_location._name, "fogne.dough");
		return;    // show intro
	}

	guiSelectCharacter();

	return; // start game
}

uint16 Parallaction_ns::guiChooseLanguage() {

	if (getPlatform() == Common::kPlatformAmiga) {
		if (!(getFeatures() & GF_LANG_MULT)) {
			if (getFeatures() & GF_DEMO)
				return 1;		// Amiga Demo supports English
			else
				return 0;		// The only other non multi-lingual version just supports Italian
		}
	}

	// user can choose language in dos version
	showSlide("lingua");
	_gfx->displayString(60, 30, "SELECT LANGUAGE", 1);

	setArrowCursor();

	do {
		updateInput();

		if (_mouseButtons == kMouseLeftUp) {
			for (uint16 _si = 0; _si < 4; _si++) {

				if (80 + _si * 49 >= _mousePos.x) continue;
				if (110 - _si * 25 >= _mousePos.y) continue;

				if (128 + _si * 49 <= _mousePos.x) continue;
				if (180 - _si * 25 <= _mousePos.y) continue;

				beep();

				switch (_si) {
				case 0:
					if (!(getFeatures() & GF_LANG_IT))
						continue;
				case 1:
					if (!(getFeatures() & GF_LANG_FR))
						continue;
				case 2:
					if (!(getFeatures() & GF_LANG_EN))
						continue;
				case 3:
					if (!(getFeatures() & GF_LANG_DE))
						continue;
				}

				return _si;
			}
		}

		g_system->delayMillis(30);
		_gfx->updateScreen();

	} while (true);

	// never reached !!!
	return 0;
}



uint16 Parallaction_ns::guiSelectGame() {
//	  printf("selectGame()\n");

	showSlide("restore");

	uint16 _si = 0;
	uint16 _di = 3;

	_mouseButtons = kMouseNone;
	while (_mouseButtons != kMouseLeftUp) {

		updateInput();

		_si = (_mousePos.x > 160) ? 1 : 0;

		if (_si != _di) {
			_di = _si;

			_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
			if (_si != 0) {
				// load a game
				_gfx->displayString(60, 30, loadGameMsg[_language], 1);
			} else {
				// new game
				_gfx->displayString(60, 30, newGameMsg[_language], 1);
			}

		}

		g_system->delayMillis(30);
		_gfx->updateScreen();
	}

	if (_si == 0) return NEW_GAME; // new game

	// load game

	// TODO: allow the user to change her mind in this screen, that is
	// don't force her to start at the intro when she closes her load
	// game window without picking a savegame.
	// The 2 strcpy's below act as workaround to prevent crashes for
	// time being.
	strcpy(_location._name, "fogne.dough");
	loadGame();

	return LOAD_GAME;  // load game
}


int Parallaction_ns::guiGetSelectedBlock(const Common::Point &p, Common::Rect &r) {

	for (uint16 _si = 0; _si < 9; _si++) {

		Common::Rect q(
			_si * BLOCK_X_OFFSET + BLOCK_SELECTION_X,
			BLOCK_SELECTION_Y - _si * BLOCK_Y_OFFSET,
			(_si + 1) * BLOCK_X_OFFSET + BLOCK_SELECTION_X,
			BLOCK_SELECTION_Y + BLOCK_HEIGHT - _si * BLOCK_Y_OFFSET
		);

		if (q.contains(p)) {
			r.setWidth(BLOCK_WIDTH);
			r.setHeight(BLOCK_HEIGHT);
			r.moveTo(_si * BLOCK_X_OFFSET + BLOCK_X, BLOCK_Y - _si * BLOCK_Y_OFFSET);
			return _si;
		}

	}

	return -1;
}


//
//	character selection and protection
//
void Parallaction_ns::guiSelectCharacter() {
	debugC(1, kDebugMenu, "Parallaction_ns::guiselectCharacter()");

	Graphics::Surface v14;
	v14.create(BLOCK_WIDTH, BLOCK_HEIGHT, 1);

	setArrowCursor();
	_soundMan->stopMusic();

	_gfx->setFont(_menuFont);

	_disk->selectArchive((getFeatures() & GF_LANG_MULT) ? "disk1" : "disk0");

	showSlide("password");	// loads background into kBitBack buffer


	uint16 (*keys)[PASSWORD_LEN] = (getPlatform() == Common::kPlatformAmiga && (getFeatures() & GF_LANG_MULT)) ? _amigaKeys : _pcKeys;
	uint16 points[3];
	bool matched = false;
	uint16 _di = 0;

	while (true) {

		_di = 0;

		_gfx->displayString(60, 30, introMsg1[_language], 1);			// displays message

		points[0] = 0;
		points[1] = 0;
		points[2] = 0;

		while (_di < PASSWORD_LEN) {

			_mouseButtons = kMouseNone;
			do {
				updateInput();
				g_system->delayMillis(30);
				_gfx->updateScreen();
			} while (_mouseButtons != kMouseLeftUp);	// waits for left click

			Common::Rect r;
			int _si = guiGetSelectedBlock(_mousePos, r);
			if (_si != -1) {
				_gfx->grabRect((byte*)v14.pixels, r, Gfx::kBitFront, BLOCK_WIDTH);
				_gfx->flatBlitCnv(&v14, _di * SLOT_WIDTH + SLOT_X, SLOT_Y, Gfx::kBitFront);
				beep();

				for (int i = 0; i < 3; i++) {
					if (keys[i][_di] == _si) {
						points[i]++;
					}

					if (points[i] == PASSWORD_LEN) {
						matched = true;
					}
				}

				_di++;
			}
		}

		if (matched) {
			break;
		}

		_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
		_gfx->displayString(60, 30, introMsg2[_language], 1);
		_gfx->updateScreen();

		g_system->delayMillis(2000);

		_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
	}

	if (points[1] == PASSWORD_LEN) {
		sprintf(_location._name, "test.%s", _dinoName);
	} else
	if (points[2] == PASSWORD_LEN) {
		sprintf(_location._name, "test.%s", _donnaName);
	} else
	if (points[0] == PASSWORD_LEN) {
		sprintf(_location._name, "test.%s", _doughName);
	}

	_gfx->setBlackPalette();
	_gfx->updateScreen();

	_engineFlags |= kEngineChangeLocation;

	v14.free();

	return;

}


} // namespace Parallaction
