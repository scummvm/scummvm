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

/*
* This code is based on the original source code of Lord Avalot d'Argent version 1.3.
* Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
*/

#include "avalanche/avalanche.h"
#include "avalanche/ghostroom.h"

#include "common/random.h"
#include "common/system.h"

namespace Avalanche {

const int8 GhostRoom::kAdjustment[5] = { 7, 0, 7, 7, 7 };
const byte GhostRoom::kWaveOrder[5] = { 4, 0, 1, 2, 3 };
const byte GhostRoom::kGlerkFade[26] = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1, 0 };
const byte GhostRoom::kGreldetFade[18] = { 0, 1, 2, 3, 4, 5, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0 };

GhostRoom::GhostRoom(AvalancheEngine *vm) {
	_vm = vm;

	_glerkStage = 0;
	_batX = 0;
	_batY = 0;
	_batCount = 0;
	_aarghCount = 0;
	_greldetX = _greldetY = 0;
	_greldetCount = 0;
	_redGreldet = false;
	_wasLoaded = false;

	_ghost = nullptr;
	_glerk = nullptr;
}

GhostRoom::~GhostRoom() {
	for (int i = 0; i < 2; i++)
		_eyes[i].free();

	_exclamation.free();

	for (int i = 0; i < 3; i++)
		_bat[i].free();

	for (int i = 0; i < 6; i++)
		_aargh[i].free();

	for (int i = 0; i < 5; i++)
		_greenEyes[i].free();

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 6; j++)
			_greldet[j][i].free();
	}

	if (_wasLoaded) {
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 2; j++) {
				for (int y = 0; y < 66; y++) {
					delete[] _ghost[i][j][y];
				}
				delete[] _ghost[i][j];
			}
			delete[] _ghost[i];
		}
		delete[] _ghost;

		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 4; j++) {
				for (int y = 0; y < 35; y++) {
					delete[] _glerk[i][j][y];
				}
				delete[] _glerk[i][j];
			}
			delete[] _glerk[i];
		}
		delete[] _glerk;
	}
}

void GhostRoom::wait(uint16 howLong) {
	for (int i = 0; i < howLong; i++) {
		Common::Event event;
		_vm->getEvent(event);
		if (event.type == Common::EVENT_KEYDOWN)
			_vm->_sound->playNote(6177, 1);
		_vm->_system->delayMillis(1);
	}
}

void GhostRoom::doBat() {
	_batCount++;

	int8 dx = 0;
	int8 iy = 0;
	byte batImage = 0;
	if ((_batCount % 2) == 1) {
		if ((1 <= _batCount) && (_batCount <= 90)) {
			dx = 2;
			iy = 1;
			batImage = 0;
		} else if ((91 <= _batCount) && (_batCount <= 240)) {
			dx = 1;
			iy = 1;
			batImage = 1;
		} else if((241 <= _batCount) && (_batCount <= 260)) {
			dx = 1;
			iy = 4;
			batImage = 2;
		}

		if ((_batCount == 91) || (_batCount == 241)) // When the bat changes, blank out the old one.
			_vm->_graphics->drawFilledRectangle(Common::Rect(_batX + _bat[batImage].w, _batY, _batX + _bat[batImage - 1].w, _batY + _bat[batImage - 1].h), kColorBlack);

		_vm->_graphics->drawFilledRectangle(Common::Rect(_batX, _batY, _batX + _bat[batImage].w, _batY + iy), kColorBlack);
		_vm->_graphics->drawFilledRectangle(Common::Rect(_batX + _bat[batImage].w - dx, _batY, _batX + _bat[batImage].w, _batY + _bat[batImage].h), kColorBlack);

		_batX -= dx;
		_batY++;
		_vm->_graphics->ghostDrawPicture(_bat[batImage], _batX, _batY);
	}
}

void GhostRoom::bigGreenEyes(byte how) {
	_vm->_graphics->ghostDrawPicture(_greenEyes[how], 330, 103);
	_vm->_graphics->ghostDrawPicture(_greenEyes[how], 376, 103);
	_vm->_graphics->refreshScreen();
}

ChunkBlock GhostRoom::readChunkBlock(Common::File &file) {
	ChunkBlock cb;
	cb._flavour = (Flavour)file.readByte();
	cb._x = file.readSint16LE();
	cb._y = file.readSint16LE();
	cb._width = file.readSint16LE();
	cb._height = file.readSint16LE();
	cb._size = file.readSint32LE();
	return cb;
}

void GhostRoom::loadPictures() {
	Common::File file;

	if (!file.open("spooky.avd"))
		error("AVALANCHE: GhostRoom: File not found: spooky.avd");

	file.seek(44);

	// Initializing ghost's array.
	_ghost = new byte***[5];
	for (int i = 0; i < 5; i++) {
		_ghost[i] = new byte**[2];
		for (int j = 0; j < 2; j++) {
			_ghost[i][j] = new byte*[66];
			for (int y = 0; y < 66; y++) {
				_ghost[i][j][y] = new byte[26];
				for (int x = 0; x < 26; x++)
					_ghost[i][j][y][x] = 0;
			}
		}
	}

	// Read in the pictures of the ghost.
	for (int i = 0; i < 5; i++) {
		ChunkBlock cb = readChunkBlock(file);
		for (int j = 0; j < 2; j++) {
			for (int y = 0; y <= cb._height; y++)
				file.read(_ghost[i][j][y], cb._width / 8);
		}
	}

	for (int i = 0; i < 2; i++)
		_eyes[i] = _vm->_graphics->ghostLoadPicture(file, dummyCoord);

	_exclamation = _vm->_graphics->ghostLoadPicture(file, dummyCoord);

	// Actually this function not just loads, but also draws the images, but they are part of the background
	// and they are need to be drawn only once.
	_vm->_graphics->ghostDrawBackgroundItems(file);

	for (int i = 0; i < 3; i++)
		_bat[i] = _vm->_graphics->ghostLoadPicture(file, dummyCoord);

	// Initializing glerk's array.
	_glerk = new byte***[6];
	for (int i = 0; i < 6; i++) {
		_glerk[i] = new byte**[4];
		for (int j = 0; j < 4; j++) {
			_glerk[i][j] = new byte*[35];
			for (int y = 0; y < 35; y++) {
				_glerk[i][j][y] = new byte[9];
				for (int x = 0; x < 9; x++)
					_glerk[i][j][y][x] = 0;
			}
		}
	}

	// Read in the pictures of the "glerk".
	for (int i = 0; i < 6; i++) {
		ChunkBlock cb = readChunkBlock(file);
		for (int j = 0; j < 4; j++) {
			for (int y = 0; y <= cb._height; y++)
				file.read(_glerk[i][j][y], cb._width / 8);
		}
	}

	for (int i = 0; i < 6; i++)
		_aargh[i] = _vm->_graphics->ghostLoadPicture(file, _aarghWhere[i]);

	for (int i = 0; i < 5; i++)
		_greenEyes[i] = _vm->_graphics->ghostLoadPicture(file, dummyCoord);

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 6; j++)
			_greldet[j][i] = _vm->_graphics->ghostLoadPicture(file, dummyCoord);
	}

	file.close();
}

void GhostRoom::run() {
	CursorMan.showMouse(false);
	_vm->_graphics->saveScreen();
	_vm->fadeOut();
	_vm->_graphics->blackOutScreen();
	_vm->fadeIn();

	// Only load the pictures if it's our first time walking into the room.
	// After that we simply use the already loaded images.
	if (!_wasLoaded) {
		loadPictures();
		_wasLoaded = true;
	}

	// Avvy walks over:
	_glerkStage = 0;
	_batX = 277;
	_batY = 40;
	_batCount = 0;

	for (int x = 500; x >= 217; x--) {
		// The floating eyeballs:
		int xBound = x % 30;
		if ((22 <= xBound) && (xBound <= 27)) {
			if (xBound == 27)
				_vm->_graphics->drawFilledRectangle(Common::Rect(x, 135, x + 17, 137), kColorBlack);
			_vm->_graphics->ghostDrawPicture(_eyes[0], x, 136);
			_vm->_graphics->drawDot(x + 16, 137, kColorBlack);
		} else {
			if (xBound == 21)
				_vm->_graphics->drawFilledRectangle(Common::Rect(x, 137, x + 18, 139), kColorBlack);
			_vm->_graphics->ghostDrawPicture(_eyes[0], x, 135);
			_vm->_graphics->drawDot(x + 16, 136, kColorBlack); // Eyes would leave a trail 1 pixel high behind them.
		}

		// Plot the Glerk:
		if ((x % 10) == 0) {
			if (_glerkStage > 25)
				break;

			_vm->_graphics->ghostDrawMonster(_glerk[kGlerkFade[_glerkStage]], 456, 14, kMonsterTypeGlerk);
			_glerkStage++;
		}

		doBat();

		_vm->_graphics->refreshScreen();

		wait(15);
	}

	// Blank out the Glerk's space.
	_vm->_graphics->drawFilledRectangle(Common::Rect(456, 14, 531, 51), kColorBlack);
	_vm->_graphics->refreshScreen();


	// Here comes the descending ghost:
	for (int y = -64; y <= 103; y++) {
		_vm->_graphics->ghostDrawMonster(_ghost[1 + (abs(y / 7) % 2) * 3], 0, y, kMonsterTypeGhost);
		if (y > 0)
			_vm->_graphics->drawFilledRectangle(Common::Rect(0, y - 1, 26 * 8 + 1, y + 1), kColorBlack);
		_vm->_graphics->refreshScreen();

		wait(27);
	}

	// Then it waves:
	_aarghCount = -15;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 5; j++) {
			_vm->_graphics->drawFilledRectangle(Common::Rect(0, 96, 26 * 8, 170), kColorBlack);
			_vm->_graphics->ghostDrawMonster(_ghost[kWaveOrder[j]], 0, 96 + kAdjustment[j], kMonsterTypeGhost);

			_aarghCount++;

			if (_aarghCount >= 0) {
				for (int k = 0; k <= _aarghCount; k++)
					_vm->_graphics->ghostDrawPicture(_aargh[k], _aarghWhere[k].x, _aarghWhere[k].y);
			}

			_vm->_graphics->refreshScreen();

			wait(177);
		}
	}

	// The exclamation mark appears:
	_vm->_graphics->ghostDrawPicture(_exclamation, 246, 127);
	_vm->_graphics->refreshScreen();
	wait(777);

	// Erase "aargh":
	_vm->_graphics->drawFilledRectangle(Common::Rect(172, 78, 348, 112), kColorBlack);
	_vm->_graphics->refreshScreen();

	for (int i = 4; i >= 0; i--) {
		wait(377);
		bigGreenEyes(i);
	}

	// Erase the exclamation mark:
	_vm->_graphics->drawFilledRectangle(Common::Rect(246, 127, 252, 134), kColorBlack);
	_vm->_graphics->refreshScreen();

	// Avvy hurries back:
	_glerkStage = 0;
	_greldetCount = 18;
	_redGreldet = false;

	for (int x = 217; x <= 479; x++) {
		// The floating eyeballs again:
		int xBound = x % 30;
		if ((22 <= xBound) && (xBound <= 27)) {
			if (xBound == 22)
				_vm->_graphics->drawFilledRectangle(Common::Rect(x + 22, 134, x + 39, 138), kColorBlack);
			_vm->_graphics->ghostDrawPicture(_eyes[1], x + 23, 136);
			_vm->_graphics->drawDot(x + 22, 137, kColorBlack);
		} else {
			if (xBound == 28)
				_vm->_graphics->drawFilledRectangle(Common::Rect(x + 22, 135, x + 39, 139), kColorBlack);
			_vm->_graphics->ghostDrawPicture(_eyes[1], x + 23, 135);
			_vm->_graphics->drawDot(x + 22, 136, kColorBlack); // Eyes would leave a trail 1 pixel high behind them.
		}

		// Plot the Green Eyes:
		if ((x % 53) == 5) {
			bigGreenEyes(_glerkStage);
			_glerkStage++;
		}

		// Plot the Greldet:
		if (_greldetCount == 18) {
			_greldetX = _vm->_rnd->getRandomNumber(599);
			_greldetY = _vm->_rnd->getRandomNumber(79);
			_greldetCount = 0;
			_redGreldet = !_redGreldet;
		}

		_vm->_graphics->ghostDrawPicture(_greldet[kGreldetFade[_greldetCount]][_redGreldet], _greldetX, _greldetY);
		_greldetCount++;

		_vm->_graphics->refreshScreen();

		wait(10);
	}

	CursorMan.showMouse(true);

	_vm->fadeOut();
	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();
	_vm->_animation->animLink();
	_vm->fadeIn();
}

} // End of namespace Avalanche
