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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
* GNU General Public License for more details.

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

namespace Avalanche {

const int8 GhostRoom::kAdjustment[5] = { 7, 0, 7, 7, 7 };
const byte GhostRoom::kWaveOrder[5] = { 5, 1, 2, 3, 4 };
const byte GhostRoom::kGlerkFade[26] = { 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 3, 3, 3, 2, 2, 1 };
const byte GhostRoom::kGreldetFade[18] = { 1, 2, 3, 4, 5, 6, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1 };

GhostRoom::GhostRoom(AvalancheEngine *vm) {
	_vm = vm;

	_glerkStage = 0;
	_aarghCount = 0;
	_batX = _batY = 0;
	_batCount = 0;
	_greldetX = _greldetY = 0;
	_greldetCount = 0;
	_redGreldet = false;
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

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 6; j++)
			_greldet[j][i].free();
}

void GhostRoom::wait(uint16 howLong) {
	warning("STUB: wait()");
}

void GhostRoom::doBat() {
	warning("STUB: doBat()");
}

void GhostRoom::bigGreenEyes(byte how) {
	warning("STUB: bigGreenEyes()");
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
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 2; j++)
			for (int y = 0; y < 66; y++)
				for (int x = 0; x < 26; x++)
					_ghost[i][j][y][x] = 0;

	// Read in the pictures of the ghost.
	for (int i = 0; i < 5; i++) {
		ChunkBlock cb = readChunkBlock(file);
		for (int j = 0; j < 2; j++)
			for (int y = 0; y <= cb._height; y++)
				file.read(_ghost[i][j][y], cb._width / 8);
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
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 4; j++)
			for (int y = 0; y < 35; y++)
				for (int x = 0; x < 9; x++)
					_glerk[i][j][y][x] = 0;

	// Read in the pictures of the "glerk".
	for (int i = 0; i < 6; i++) {
		ChunkBlock cb = readChunkBlock(file);
		for (int j = 0; j < 4; j++)
			for (int y = 0; y <= cb._height; y++)
				file.read(_glerk[i][j][y], cb._width / 8);
	}

	for (int i = 0; i < 6; i++)
		_aargh[i] = _vm->_graphics->ghostLoadPicture(file, _aarghWhere[i]);

	for (int i = 0; i < 5; i++)
		_greenEyes[i] = _vm->_graphics->ghostLoadPicture(file, dummyCoord);

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 6; j++)
			_greldet[j][i] = _vm->_graphics->ghostLoadPicture(file, dummyCoord);

	file.close();
}

void GhostRoom::run() {
	_vm->_graphics->saveScreen();
	_vm->fadeOut();
	_vm->fadeIn();
	_vm->_graphics->drawFilledRectangle(Common::Rect(0, 0, 640, 200), kColorBlack); // Black out the whole screen.

	loadPictures();
	
	warning("STUB: run()");
}

} // End of namespace Avalanche
