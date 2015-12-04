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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/scummsys.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/rect.h"

#include "engines/dialogs.h"
#include "engines/engine.h"
#include "engines/util.h"
#include "gui/message.h"
#include "engines/advancedDetector.h"

#include "lab/lab.h"
#include "lab/labfun.h"
#include "lab/resource.h"
#include "lab/anim.h"
#include "lab/graphics.h"

namespace Lab {

LabEngine *g_lab;

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

LabEngine::LabEngine(OSystem *syst, const ADGameDescription *gameDesc)
 : Engine(syst), _gameDescription(gameDesc), _extraGameFeatures(0) {
	g_lab = this;

	_screenWidth = 320;
	_screenHeight = 200;

	_currentDisplayBuffer = 0;
	_displayBuffer = 0;

	_lastWaitTOFTicks = 0;

	_isHiRes = false;
	_roomNum = -1;
	for (int i = 0; i < MAX_CRUMBS; i++) {
		_breadCrumbs[i]._roomNum = 0;
		_breadCrumbs[i]._direction = NORTH;
	}

	_numCrumbs = 0;
	_droppingCrumbs = false;
	_followingCrumbs = false;
	_followCrumbsFast = false;
	_isCrumbTurning = false;
	_isCrumbWaiting = false;
	_crumbSecs = 0;
	_crumbMicros = 0;

	_event = nullptr;
	_resource = nullptr;
	_music = nullptr;
	_anim = nullptr;
	_graphics = nullptr;

	_lastTooLong = false;
	_interfaceOff = false;
	_alternate = false;

	for (int i = 0; i < 20; i++)
		_moveImages[i] = nullptr;

	for (int i = 0; i < 10; i++) {
		_invImages[i] = nullptr;
		Images[i] = nullptr;
	}

	_moveGadgetList = nullptr;
	_invGadgetList = nullptr;
	_curFileName = nullptr;
	_nextFileName = nullptr;
	_newFileName = nullptr;

	_moveGadgetList = 0;
	_invGadgetList = 0;

	_curFileName = " ";
	_msgFont = 0;

	_inventory = 0;

	for (int i = 0; i < 16; i++)
		Tiles[i] = nullptr;

	for (int i= 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			CurTile[i][j] = INIT_TILE[i][j];
	}

	for (int i = 0; i < 6; i++)
		combination[i] = 0;

	//const Common::FSNode gameDataDir(ConfMan.get("path"));
	//SearchMan.addSubDirectoryMatching(gameDataDir, "game");
	//SearchMan.addSubDirectoryMatching(gameDataDir, "game/pict");
	//SearchMan.addSubDirectoryMatching(gameDataDir, "game/spict");
	//SearchMan.addSubDirectoryMatching(gameDataDir, "music");
}

LabEngine::~LabEngine() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

	delete _event;
	delete _resource;
	delete _music;
	delete _anim;
	delete _graphics;

	for (int i = 0; i < 16; i++)
		delete Tiles[i];
}

Common::Error LabEngine::run() {
	if (getFeatures() & GF_LOWRES)
		initGraphics(320, 200, false);
	else
		initGraphics(640, 480, true);

	_event = new EventManager(this);
	_resource = new Resource(this);
	_music = new Music(this);
	_anim = new Anim(this);
	_graphics = new DisplayMan(this);

	if (getPlatform() == Common::kPlatformWindows) {
		// Check if this is the Wyrmkeep trial
		Common::File roomFile;
		bool knownVersion = true;
		bool roomFileOpened = roomFile.open("game/rooms/48");

		if (!roomFileOpened)
			knownVersion = false;
		else if (roomFile.size() != 892)
			knownVersion = false;
		else {
			roomFile.seek(352);
			byte checkByte = roomFile.readByte();
			if (checkByte == 0x00) {
				// Full Windows version
			} else if (checkByte == 0x80) {
				// Wyrmkeep trial version
				_extraGameFeatures = GF_WINDOWS_TRIAL;

				GUI::MessageDialog trialMessage("This is a trial Windows version of the game. To play the full version, you will need to use the original interpreter and purchase a key from Wyrmkeep");
				trialMessage.runModal();
			} else {
				knownVersion = false;
			}

			roomFile.close();

			if (!knownVersion) {
				warning("Unknown Windows version found, please report this version to the ScummVM team");
				return Common::kNoGameDataFoundError;
			}
		}
	}

	go();

	return Common::kNoError;
}

Common::String LabEngine::generateSaveFileName(uint slot) {
	return Common::String::format("%s.%03u", _targetName.c_str(), slot);
}

/*****************************************************************************/
/* Converts SVGA cords to VGA if necessary, otherwise returns VGA cords.     */
/*****************************************************************************/
int LabEngine::VGAUnScaleX(int x) {
	if (_isHiRes)
		return (x / 2);
	else
		return x;
}

/*****************************************************************************/
/* Converts SVGA cords to VGA if necessary, otherwise returns VGA cords.     */
/*****************************************************************************/
int LabEngine::VGAUnScaleY(int y) {
	if (_isHiRes)
		return ((y * 5) / 12);
	else
		return y;
}

/*****************************************************************************/
/* Processes mouse clicks and changes the combination.                       */
/*****************************************************************************/
void LabEngine::mouseTile(Common::Point pos) {
	int x = VGAUnScaleX(pos.x);
	int y = VGAUnScaleY(pos.y);

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
		if (CurTile[col] [row - 1] == 0) {
			CurTile[col] [row - 1] = CurTile[col] [row];
			CurTile[col] [row] = 0;
			scrolltype = DOWNSCROLL;
		}
	}

	if (col > 0) {
		if (CurTile[col - 1] [row] == 0) {
			CurTile[col - 1] [row] = CurTile[col] [row];
			CurTile[col] [row] = 0;
			scrolltype = RIGHTSCROLL;
		}
	}

	if (row < 3) {
		if (CurTile[col] [row + 1] == 0) {
			CurTile[col] [row + 1] = CurTile[col] [row];
			CurTile[col] [row] = 0;
			scrolltype = UPSCROLL;
		}
	}

	if (col < 3) {
		if (CurTile[col + 1] [row] == 0) {
			CurTile[col + 1] [row] = CurTile[col] [row];
			CurTile[col] [row] = 0;
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
				check = check && (CurTile[row] [col] == SOLUTION[row] [col]);
				col++;
			}

			row++;
			col = 0;
		}

		if (check) {
			_conditions->inclElement(BRICKOPEN);  /* unlocked combination */
			_anim->_doBlack = true;
			check = _graphics->readPict("p:Up/BDOpen", true);
		}
	}
}

/*****************************************************************************/
/* Processes mouse clicks and changes the combination.                       */
/*****************************************************************************/
void LabEngine::mouseCombination(Common::Point pos) {
	uint16 number;

	int x = VGAUnScaleX(pos.x);
	int y = VGAUnScaleY(pos.y);

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
		rowm = _graphics->VGAScaleY(23);
		colm = _graphics->VGAScaleX(27);

		rows = _graphics->VGAScaleY(31);
		cols = _graphics->VGAScaleX(105);
	} else {
		_graphics->setAPen(0);
		_graphics->rectFill(_graphics->VGAScaleX(97), _graphics->VGAScaleY(22), _graphics->VGAScaleX(220), _graphics->VGAScaleY(126));

		rowm = _graphics->VGAScaleY(25);
		colm = _graphics->VGAScaleX(30);

		rows = _graphics->VGAScaleY(25);
		cols = _graphics->VGAScaleX(100);
	}

	while (row < 4) {
		while (col < 4) {
			if (showsolution)
				num = SOLUTION[col] [row];
			else
				num = CurTile[col] [row];

			if (showsolution || num)
				Tiles[num]->drawImage(cols + (col * colm), rows + (row * rowm));

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

	resetBuffer();
	_anim->_doBlack = true;
	_anim->_noPalChange = true;
	_graphics->readPict(filename, true);
	_anim->_noPalChange = false;
	_graphics->blackScreen();

	Common::File *tileFile = tileFile = _resource->openDataFile(showsolution ? "P:TileSolution" : "P:Tile");

	for (uint16 curBit = start; curBit < 16; curBit++)
		Tiles[curBit] = new Image(tileFile);

	delete tileFile;

	allocFile((void **)&_tempScrollData, Tiles[1]->_width * Tiles[1]->_height * 2L, "tempdata");

	doTile(showsolution);
	setPalette(_anim->_diffPalette, 256);
}

/*****************************************************************************/
/* Does the scrolling for the tiles on the tile puzzle.                      */
/*****************************************************************************/
void LabEngine::doTileScroll(uint16 col, uint16 row, uint16 scrolltype) {
	int16 dX = 0, dY = 0, dx = 0, dy = 0, sx = 0, sy = 0;
	uint16 last = 0, x1, y1;

	if (scrolltype == LEFTSCROLL) {
		dX = _graphics->VGAScaleX(5);
		sx = _graphics->VGAScaleX(5);
		last = 6;
	} else if (scrolltype == RIGHTSCROLL) {
		dX = _graphics->VGAScaleX(-5);
		dx = _graphics->VGAScaleX(-5);
		sx = _graphics->VGAScaleX(5);
		last = 6;
	} else if (scrolltype == UPSCROLL) {
		dY = _graphics->VGAScaleY(5);
		sy = _graphics->VGAScaleY(5);
		last = 5;
	} else if (scrolltype == DOWNSCROLL) {
		dY = _graphics->VGAScaleY(-5);
		dy = _graphics->VGAScaleY(-5);
		sy = _graphics->VGAScaleY(5);
		last = 5;
	}

	sx += _graphics->SVGACord(2);

	x1 = _graphics->VGAScaleX(100) + (col * _graphics->VGAScaleX(30)) + dx;
	y1 = _graphics->VGAScaleY(25) + (row * _graphics->VGAScaleY(25)) + dy;

	for (uint16 i = 0; i < last; i++) {
		waitTOF();
		scrollRaster(dX, dY, x1, y1, x1 + _graphics->VGAScaleX(28) + sx, y1 + _graphics->VGAScaleY(23) + sy);
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

	if (combination[number] < 9)
		(combination[number])++;
	else
		combination[number] = 0;

	combnum = combination[number];

	display._imageData = getCurrentDrawingBuffer();
	display._width     = _screenWidth;
	display._height    = _screenHeight;

	for (uint16 i = 1; i <= (Images[combnum]->_height / 2); i++) {
		if (_isHiRes) {
			if (i & 1)
				waitTOF();
		} else
			waitTOF();

		display._imageData = getCurrentDrawingBuffer();

		scrollDisplayY(2, _graphics->VGAScaleX(COMBINATION_X[number]), _graphics->VGAScaleY(65), _graphics->VGAScaleX(COMBINATION_X[number]) + (Images[combnum])->_width - 1, _graphics->VGAScaleY(65) + (Images[combnum])->_height);

		Images[combnum]->bltBitMap(0, (Images[combnum])->_height - (2 * i), &(display), _graphics->VGAScaleX(COMBINATION_X[number]), _graphics->VGAScaleY(65), (Images[combnum])->_width, 2);
	}

	for (uint16 i = 0; i < 6; i++)
		unlocked = (combination[i] == solution[i]) && unlocked;

	if (unlocked)
		_conditions->inclElement(COMBINATIONUNLOCKED);
	else
		_conditions->exclElement(COMBINATIONUNLOCKED);
}

void LabEngine::scrollRaster(int16 dx, int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	if (dx)
		scrollDisplayX(dx, x1, y1, x2, y2);

	if (dy)
		scrollDisplayY(dy, x1, y1, x2, y2);
}

/*****************************************************************************/
/* Draws the images of the combination lock to the display bitmap.           */
/*****************************************************************************/
void LabEngine::doCombination() {
	for (uint16 i = 0; i <= 5; i++)
		Images[combination[i]]->drawImage(_graphics->VGAScaleX(COMBINATION_X[i]), _graphics->VGAScaleY(65));
}

/*****************************************************************************/
/* Reads in a backdrop picture.                                              */
/*****************************************************************************/
void LabEngine::showCombination(const char *filename) {
	resetBuffer();
	_anim->_doBlack = true;
	_anim->_noPalChange = true;
	_graphics->readPict(filename, true);
	_anim->_noPalChange = false;

	_graphics->blackScreen();

	Common::File *numFile = _resource->openDataFile("P:Numbers");

	for (uint16 CurBit = 0; CurBit < 10; CurBit++)
		Images[CurBit] = new Image(numFile);

	delete numFile;

	allocFile((void **)&_tempScrollData, Images[0]->_width * Images[0]->_height * 2L, "tempdata");

	doCombination();

	setPalette(_anim->_diffPalette, 256);
}

} // End of namespace Lab
