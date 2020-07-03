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
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/file.h"
#include "common/translation.h"

#include "gui/message.h"

#include "lab/lab.h"
#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/image.h"
#include "lab/labsets.h"
#include "lab/resource.h"
#include "lab/speciallocks.h"
#include "lab/utils.h"

namespace Lab {

#define BRICKOPEN      115
#define COMBINATIONUNLOCKED  130

enum TileScroll {
	kScrollLeft = 1,
	kScrollRight = 2,
	kScrollUp = 3,
	kScrollDown = 4
};

const uint16 INIT_TILE[4][4] = {
	{ 1, 5, 9, 13 },
	{ 2, 6, 10, 14 },
	{ 3, 7, 11, 15 },
	{ 4, 8, 12, 0 }
};

const uint16 SOLUTION[4][4] = {
	{ 7, 1, 8, 3 },
	{ 2, 11, 15, 4 },
	{ 9, 5, 14, 6 },
	{ 10, 13, 12, 0 }
};

const int COMBINATION_X[6] = { 45, 83, 129, 166, 211, 248 };

SpecialLocks::SpecialLocks(LabEngine *vm) : _vm(vm) {
	for (int i = 0; i < 16; i++)
		_tiles[i] = nullptr;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			_curTile[i][j] = INIT_TILE[i][j];
	}

	for (int i = 0; i < 6; i++)
		_combination[i] = 0;

	for (int i = 0; i < 10; i++)
		_numberImages[i] = nullptr;
}

SpecialLocks::~SpecialLocks() {
	for (int i = 0; i < 16; i++)
		delete _tiles[i];

	for (int imgIdx = 0; imgIdx < 10; imgIdx++) {
		delete _numberImages[imgIdx];
		_numberImages[imgIdx] = nullptr;
	}
}

void SpecialLocks::tileClick(Common::Point pos) {
	Common::Point realPos = _vm->_utils->vgaUnscale(pos);

	if ((realPos.x < 101) || (realPos.y < 26))
		return;

	int tileX = (realPos.x - 101) / 30;
	int tileY = (realPos.y -  26) / 25;

	if ((tileX < 4) && (tileY < 4))
		changeTile(tileX, tileY);
}

void SpecialLocks::changeTile(uint16 col, uint16 row) {
	int16 scrolltype = -1;

	if (row > 0) {
		if (_curTile[col][row - 1] == 0) {
			_curTile[col][row - 1] = _curTile[col][row];
			_curTile[col][row] = 0;
			scrolltype = kScrollDown;
		}
	}

	if (col > 0) {
		if (_curTile[col - 1][row] == 0) {
			_curTile[col - 1][row] = _curTile[col][row];
			_curTile[col][row] = 0;
			scrolltype = kScrollRight;
		}
	}

	if (row < 3) {
		if (_curTile[col][row + 1] == 0) {
			_curTile[col][row + 1] = _curTile[col][row];
			_curTile[col][row] = 0;
			scrolltype = kScrollUp;
		}
	}

	if (col < 3) {
		if (_curTile[col + 1][row] == 0) {
			_curTile[col + 1][row] = _curTile[col][row];
			_curTile[col][row] = 0;
			scrolltype = kScrollLeft;
		}
	}

	if (scrolltype != -1) {
		if (_vm->getFeatures() & GF_WINDOWS_TRIAL) {
			GUI::MessageDialog trialMessage(_("This puzzle is not available in the trial version of the game"));
			trialMessage.runModal();
			return;
		}

		doTileScroll(col, row, scrolltype);
		bool check = true;
		row = 0;
		col = 0;

		while (row < 4) {
			while (col < 4) {
				check &= (_curTile[row][col] == SOLUTION[row][col]);
				col++;
			}

			row++;
			col = 0;
		}

		if (check) {
			// unlocked combination
			_vm->_conditions->inclElement(BRICKOPEN);
			_vm->_anim->_doBlack = true;
			_vm->_graphics->readPict("p:Up/BDOpen");
		}
	}
}

void SpecialLocks::combinationClick(Common::Point pos) {
	Common::Point realPos = _vm->_utils->vgaUnscale(pos);

	if (!Common::Rect(44, 63, 285, 99).contains(realPos))
		return;

	uint16 number = 0;
	if (realPos.x < 83)
		number = 0;
	else if (realPos.x < 127)
		number = 1;
	else if (realPos.x < 165)
		number = 2;
	else if (realPos.x < 210)
		number = 3;
	else if (realPos.x < 245)
		number = 4;
	else if (realPos.x < 286)
		number = 5;

	changeCombination(number);
}

void SpecialLocks::doTile(bool showsolution) {
	uint16 row = 0, col = 0, rowm, colm, num;
	int16 rows, cols;

	if (showsolution) {
		rowm = _vm->_utils->vgaScaleY(23);
		colm = _vm->_utils->vgaScaleX(27);

		rows = _vm->_utils->vgaScaleY(31);
		cols = _vm->_utils->vgaScaleX(105);
	} else {
		_vm->_graphics->rectFillScaled(97, 22, 220, 126, 0);

		rowm = _vm->_utils->vgaScaleY(25);
		colm = _vm->_utils->vgaScaleX(30);

		rows = _vm->_utils->vgaScaleY(25);
		cols = _vm->_utils->vgaScaleX(100);
	}

	while (row < 4) {
		while (col < 4) {
			if (showsolution)
				num = SOLUTION[col][row];
			else
				num = _curTile[col][row];

			if (showsolution || num)
				_tiles[num]->drawImage(cols + (col * colm), rows + (row * rowm));

			col++;
		}

		row++;
		col = 0;
	}
}

void SpecialLocks::showTileLock(const Common::String filename, bool showSolution) {
	_vm->_anim->_doBlack = true;
	_vm->_anim->_noPalChange = true;
	_vm->_graphics->readPict(filename);
	_vm->_anim->_noPalChange = false;
	_vm->_graphics->blackScreen();

	Common::File *tileFile;
	if (_vm->getPlatform() == Common::kPlatformDOS)
		tileFile = _vm->_resource->openDataFile(showSolution ? "P:TileSolu" : "P:Tile");
	else
		// Windows and Amiga versions use TileSolution and Tile
		tileFile = _vm->_resource->openDataFile(showSolution ? "P:TileSolution" : "P:Tile");

	int start = showSolution ? 0 : 1;

	for (int curBit = start; curBit < 16; curBit++)
		_tiles[curBit] = new Image(tileFile, _vm);

	delete tileFile;

	doTile(showSolution);
	_vm->_graphics->setPalette(_vm->_anim->_diffPalette, 256);
}

void SpecialLocks::doTileScroll(uint16 col, uint16 row, uint16 scrolltype) {
	int16 dX = 0, dY = 0, dx = 0, dy = 0, sx = 0, sy = 0;
	int last = 0;

	if (scrolltype == kScrollLeft) {
		dX = _vm->_utils->vgaScaleX(5);
		sx = _vm->_utils->vgaScaleX(5);
		last = 6;
	} else if (scrolltype == kScrollRight) {
		dX = _vm->_utils->vgaScaleX(-5);
		dx = _vm->_utils->vgaScaleX(-5);
		sx = _vm->_utils->vgaScaleX(5);
		last = 6;
	} else if (scrolltype == kScrollUp) {
		dY = _vm->_utils->vgaScaleY(5);
		sy = _vm->_utils->vgaScaleY(5);
		last = 5;
	} else if (scrolltype == kScrollDown) {
		dY = _vm->_utils->vgaScaleY(-5);
		dy = _vm->_utils->vgaScaleY(-5);
		sy = _vm->_utils->vgaScaleY(5);
		last = 5;
	}

	sx += _vm->_utils->svgaCord(2);

	uint16 x1 = _vm->_utils->vgaScaleX(100) + (col * _vm->_utils->vgaScaleX(30)) + dx;
	uint16 y1 = _vm->_utils->vgaScaleY(25) + (row * _vm->_utils->vgaScaleY(25)) + dy;

	byte *buffer = new byte[_tiles[1]->_width * _tiles[1]->_height * 2];

	for (int i = 0; i < last; i++) {
		_vm->waitTOF();
		scrollRaster(dX, dY, x1, y1, x1 + _vm->_utils->vgaScaleX(28) + sx, y1 + _vm->_utils->vgaScaleY(23) + sy, buffer);
		x1 += dX;
		y1 += dY;
	}

	delete[] buffer;
}

void SpecialLocks::scrollRaster(int16 dx, int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer) {
	if (dx)
		_vm->_graphics->scrollDisplayX(dx, x1, y1, x2, y2, buffer);

	if (dy)
		_vm->_graphics->scrollDisplayY(dy, x1, y1, x2, y2, buffer);
}

void SpecialLocks::changeCombination(uint16 number) {
	const int solution[6] = { 0, 4, 0, 8, 7, 2 };

	Image display(_vm);

	if (_combination[number] < 9)
		(_combination[number])++;
	else
		_combination[number] = 0;

	uint16 combnum = _combination[number];

	display.setData(_vm->_graphics->getCurrentDrawingBuffer(), false);
	display._width = _vm->_graphics->_screenWidth;
	display._height = _vm->_graphics->_screenHeight;

	byte *buffer = new byte[_numberImages[1]->_width * _numberImages[1]->_height * 2];

	for (int i = 1; i <= (_numberImages[combnum]->_height / 2); i++) {
		if (i & 1)
			_vm->waitTOF();

		display.setData(_vm->_graphics->getCurrentDrawingBuffer(), false);
		_vm->_graphics->scrollDisplayY(2, _vm->_utils->vgaScaleX(COMBINATION_X[number]), _vm->_utils->vgaScaleY(65), _vm->_utils->vgaScaleX(COMBINATION_X[number]) + (_numberImages[combnum])->_width - 1, _vm->_utils->vgaScaleY(65) + (_numberImages[combnum])->_height, buffer);
		_numberImages[combnum]->blitBitmap(0, (_numberImages[combnum])->_height - (2 * i), &(display), _vm->_utils->vgaScaleX(COMBINATION_X[number]), _vm->_utils->vgaScaleY(65), (_numberImages[combnum])->_width, 2, false);
	}

	delete[] buffer;

	bool unlocked = true;
	for (int i = 0; i < 6; i++)
		unlocked &= (_combination[i] == solution[i]);

	if (unlocked)
		_vm->_conditions->inclElement(COMBINATIONUNLOCKED);
	else
		_vm->_conditions->exclElement(COMBINATIONUNLOCKED);
}

void SpecialLocks::showCombinationLock(const Common::String filename) {
	_vm->_anim->_doBlack = true;
	_vm->_anim->_noPalChange = true;
	_vm->_graphics->readPict(filename);
	_vm->_anim->_noPalChange = false;

	_vm->_graphics->blackScreen();

	Common::File *numFile = _vm->_resource->openDataFile("P:Numbers");

	for (int i = 0; i < 10; i++) {
		_numberImages[i] = new Image(numFile, _vm);
	}

	delete numFile;

	for (int i = 0; i <= 5; i++)
		_numberImages[_combination[i]]->drawImage(_vm->_utils->vgaScaleX(COMBINATION_X[i]), _vm->_utils->vgaScaleY(65));

	_vm->_graphics->setPalette(_vm->_anim->_diffPalette, 256);
}

void SpecialLocks::save(Common::OutSaveFile *file) {
	// Combination lock
	for (int i = 0; i < 6; i++)
		file->writeByte(_combination[i]);

	// Tiles
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			file->writeUint16LE(_curTile[i][j]);
}

void SpecialLocks::load(Common::InSaveFile *file) {
	// Combination lock
	for (int i = 0; i < 6; i++)
		_combination[i] = file->readByte();

	// Tiles
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			_curTile[i][j] = file->readUint16LE();
}

} // End of namespace Lab
