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

	for (int i = 0; i < 5; i++)
		_greenEyes[i] = nullptr;

	_glerk = nullptr;
	_glerkStage = 0;
	_aarghCount = 0;
	_batX = _batY = 0;
	_batCount = 0;
	_greldetX = _greldetY = 0;
	_greldetCount = 0;
	_gb = false;
	_redGreldet = false;
}

GhostRoom::~GhostRoom() {
	for (int i = 0; i < 2; i++)
		_eyes[i].free();
	_exclamation.free();
}

void GhostRoom::plainGrab() {
	warning("STUB: plainGrab()");
}

void GhostRoom::getMeAargh(byte which) {
	warning("STUB: getMeAargh()");
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

ChunkBlockType GhostRoom::readChunkBlock(Common::File &file) {
	ChunkBlockType cb;
	cb._flavour = (FlavourType)file.readByte();
	cb._x = file.readSint16LE();
	cb._y = file.readSint16LE();
	cb._xl = file.readSint16LE();
	cb._yl = file.readSint16LE();
	cb._size = file.readSint32LE();
	return cb;
}

void GhostRoom::run() {
	_vm->_graphics->saveScreen();
	_vm->fadeOut();
	_vm->fadeIn();
	_vm->_graphics->drawFilledRectangle(Common::Rect(0, 0, 640, 200), kColorBlack);

	if (!_file.open("spooky.avd"))
		error("AVALANCHE: Trip: File not found: spooky.avd");

	_file.seek(44);

	// Initializing ghost's array.
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 2; j++)
			for (int y = 0; y < 66; y++)
				for (int x = 0; x < 26; x++)
					_ghost[i][j][y][x] = 0;

	// Reading in the pictures of the ghost.
	for (int i = 0; i < 5; i++) {
		ChunkBlockType cb = readChunkBlock(_file);
		for (int j = 0; j < 2; j++)
			for (uint16 y = 0; y <= cb._yl; y++)
				_file.read(_ghost[i][j][y], cb._xl / 8);
	}

	// Load some smaller pictures.
	for (int i = 0; i < 2; i++)
		_eyes[i] = _vm->_graphics->ghostLoadPicture(_file);
	_exclamation = _vm->_graphics->ghostLoadPicture(_file);

	warning("STUB: run()");
}

} // End of namespace Avalanche
