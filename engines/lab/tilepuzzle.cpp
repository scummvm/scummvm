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

#include "lab/lab.h"
#include "gui/message.h"

namespace Lab {

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

void LabEngine::initTilePuzzle() {
	for (int i = 0; i < 16; i++)
		_tiles[i] = nullptr;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			_curTile[i][j] = INIT_TILE[i][j];
	}

	for (int i = 0; i < 6; i++)
		_combination[i] = 0;
}

/*****************************************************************************/
/* Processes mouse clicks and changes the combination.                       */
/*****************************************************************************/
void LabEngine::mouseTile(Common::Point pos) {
	int x = _utils->vgaUnscaleX(pos.x);
	int y = _utils->vgaUnscaleY(pos.y);

	if ((x < 101) || (y < 26))
		return;

	x = (x - 101) / 30;
	y = (y -  26) / 25;

	if ((x < 4) && (y < 4))
		changeTile(x, y);
}

/*****************************************************************************/
/* Changes the combination number of one of the slots                        */
/*****************************************************************************/
void LabEngine::changeTile(uint16 col, uint16 row) {
	int16 scrolltype = -1;

	if (row > 0) {
		if (_curTile[col] [row - 1] == 0) {
			_curTile[col] [row - 1] = _curTile[col] [row];
			_curTile[col] [row] = 0;
			scrolltype = DOWNSCROLL;
		}
	}

	if (col > 0) {
		if (_curTile[col - 1] [row] == 0) {
			_curTile[col - 1] [row] = _curTile[col] [row];
			_curTile[col] [row] = 0;
			scrolltype = RIGHTSCROLL;
		}
	}

	if (row < 3) {
		if (_curTile[col] [row + 1] == 0) {
			_curTile[col] [row + 1] = _curTile[col] [row];
			_curTile[col] [row] = 0;
			scrolltype = UPSCROLL;
		}
	}

	if (col < 3) {
		if (_curTile[col + 1] [row] == 0) {
			_curTile[col + 1] [row] = _curTile[col] [row];
			_curTile[col] [row] = 0;
			scrolltype = LEFTSCROLL;
		}
	}

	if (scrolltype != -1) {
		doTileScroll(col, row, scrolltype);

		if (getFeatures() & GF_WINDOWS_TRIAL) {
			GUI::MessageDialog trialMessage("This puzzle is not available in the trial version of the game");
			trialMessage.runModal();
			return;
		}

		bool check = true;
		row = 0;
		col = 0;

		while (row < 4) {
			while (col < 4) {
				check = check && (_curTile[row] [col] == SOLUTION[row] [col]);
				col++;
			}

			row++;
			col = 0;
		}

		if (check) {
			_conditions->inclElement(BRICKOPEN);  /* unlocked combination */
			_anim->_doBlack = true;
			_graphics->readPict("p:Up/BDOpen", true);
		}
	}
}

/*****************************************************************************/
/* Processes mouse clicks and changes the combination.                       */
/*****************************************************************************/
void LabEngine::mouseCombination(Common::Point pos) {
	uint16 number;

	int x = _utils->vgaUnscaleX(pos.x);
	int y = _utils->vgaUnscaleY(pos.y);

	if ((y >= 63) && (y <= 99)) {
		if ((x >= 44) && (x < 83))
			number = 0;
		else if (x < 127)
			number = 1;
		else if (x < 165)
			number = 2;
		else if (x < 210)
			number = 3;
		else if (x < 245)
			number = 4;
		else if (x < 286)
			number = 5;
		else
			return;

		changeCombination(number);
	}
}

/*****************************************************************************/
/* Draws the images of the combination lock to the display bitmap.           */
/*****************************************************************************/
void LabEngine::doTile(bool showsolution) {
	uint16 row = 0, col = 0, rowm, colm, num;
	int16 rows, cols;

	if (showsolution) {
		rowm = _utils->vgaScaleY(23);
		colm = _utils->vgaScaleX(27);

		rows = _utils->vgaScaleY(31);
		cols = _utils->vgaScaleX(105);
	} else {
		_graphics->setAPen(0);
		_graphics->rectFillScaled(97, 22, 220, 126);

		rowm = _utils->vgaScaleY(25);
		colm = _utils->vgaScaleX(30);

		rows = _utils->vgaScaleY(25);
		cols = _utils->vgaScaleX(100);
	}

	while (row < 4) {
		while (col < 4) {
			if (showsolution)
				num = SOLUTION[col] [row];
			else
				num = _curTile[col] [row];

			if (showsolution || num)
				_tiles[num]->drawImage(cols + (col * colm), rows + (row * rowm));

			col++;
		}

		row++;
		col = 0;
	}
}

/*****************************************************************************/
/* Reads in a backdrop picture.                                              */
/*****************************************************************************/
void LabEngine::showTile(const char *filename, bool showsolution) {
	uint16 start = showsolution ? 0 : 1;

	_anim->_doBlack = true;
	_anim->_noPalChange = true;
	_graphics->readPict(filename, true);
	_anim->_noPalChange = false;
	_graphics->blackScreen();

	Common::File *tileFile = tileFile = _resource->openDataFile(showsolution ? "P:TileSolution" : "P:Tile");

	for (uint16 curBit = start; curBit < 16; curBit++)
		_tiles[curBit] = new Image(tileFile);

	delete tileFile;

	doTile(showsolution);
	_graphics->setPalette(_anim->_diffPalette, 256);
}

/*****************************************************************************/
/* Does the scrolling for the tiles on the tile puzzle.                      */
/*****************************************************************************/
void LabEngine::doTileScroll(uint16 col, uint16 row, uint16 scrolltype) {
	int16 dX = 0, dY = 0, dx = 0, dy = 0, sx = 0, sy = 0;
	uint16 last = 0, x1, y1;

	if (scrolltype == LEFTSCROLL) {
		dX = _utils->vgaScaleX(5);
		sx = _utils->vgaScaleX(5);
		last = 6;
	} else if (scrolltype == RIGHTSCROLL) {
		dX = _utils->vgaScaleX(-5);
		dx = _utils->vgaScaleX(-5);
		sx = _utils->vgaScaleX(5);
		last = 6;
	} else if (scrolltype == UPSCROLL) {
		dY = _utils->vgaScaleY(5);
		sy = _utils->vgaScaleY(5);
		last = 5;
	} else if (scrolltype == DOWNSCROLL) {
		dY = _utils->vgaScaleY(-5);
		dy = _utils->vgaScaleY(-5);
		sy = _utils->vgaScaleY(5);
		last = 5;
	}

	sx += _utils->svgaCord(2);

	x1 = _utils->vgaScaleX(100) + (col * _utils->vgaScaleX(30)) + dx;
	y1 = _utils->vgaScaleY(25) + (row * _utils->vgaScaleY(25)) + dy;

	for (uint16 i = 0; i < last; i++) {
		waitTOF();
		scrollRaster(dX, dY, x1, y1, x1 + _utils->vgaScaleX(28) + sx, y1 + _utils->vgaScaleY(23) + sy);
		x1 += dX;
		y1 += dY;
	}
}

/*****************************************************************************/
/* Changes the combination number of one of the slots                        */
/*****************************************************************************/
void LabEngine::changeCombination(uint16 number) {
	static const int solution[6] = { 0, 4, 0, 8, 7, 2 };

	Image display;
	uint16 combnum;
	bool unlocked = true;

	if (_combination[number] < 9)
		(_combination[number])++;
	else
		_combination[number] = 0;

	combnum = _combination[number];

	display._imageData = _graphics->getCurrentDrawingBuffer();
	display._width     = _graphics->_screenWidth;
	display._height    = _graphics->_screenHeight;

	for (uint16 i = 1; i <= (_numberImages[combnum]->_height / 2); i++) {
		if (_isHiRes) {
			if (i & 1)
				waitTOF();
		} else
			waitTOF();

		display._imageData = _graphics->getCurrentDrawingBuffer();
		_graphics->scrollDisplayY(2, _utils->vgaScaleX(COMBINATION_X[number]), _utils->vgaScaleY(65), _utils->vgaScaleX(COMBINATION_X[number]) + (_numberImages[combnum])->_width - 1, _utils->vgaScaleY(65) + (_numberImages[combnum])->_height);
		_numberImages[combnum]->blitBitmap(0, (_numberImages[combnum])->_height - (2 * i), &(display), _utils->vgaScaleX(COMBINATION_X[number]), _utils->vgaScaleY(65), (_numberImages[combnum])->_width, 2, false);
	}

	for (uint16 i = 0; i < 6; i++)
		unlocked = (_combination[i] == solution[i]) && unlocked;

	if (unlocked)
		_conditions->inclElement(COMBINATIONUNLOCKED);
	else
		_conditions->exclElement(COMBINATIONUNLOCKED);
}

void LabEngine::scrollRaster(int16 dx, int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	if (dx)
		_graphics->scrollDisplayX(dx, x1, y1, x2, y2);

	if (dy)
		_graphics->scrollDisplayY(dy, x1, y1, x2, y2);
}

/*****************************************************************************/
/* Draws the images of the combination lock to the display bitmap.           */
/*****************************************************************************/
void LabEngine::doCombination() {
	for (uint16 i = 0; i <= 5; i++)
		_numberImages[_combination[i]]->drawImage(_utils->vgaScaleX(COMBINATION_X[i]), _utils->vgaScaleY(65));
}

/*****************************************************************************/
/* Reads in a backdrop picture.                                              */
/*****************************************************************************/
void LabEngine::showCombination(const char *filename) {
	_anim->_doBlack = true;
	_anim->_noPalChange = true;
	_graphics->readPict(filename, true);
	_anim->_noPalChange = false;

	_graphics->blackScreen();

	Common::File *numFile = _resource->openDataFile("P:Numbers");

	for (uint16 CurBit = 0; CurBit < 10; CurBit++)
		_numberImages[CurBit] = new Image(numFile);

	delete numFile;

	doCombination();

	_graphics->setPalette(_anim->_diffPalette, 256);
}

} // End of namespace Lab
