/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* This file covers both Kernal.GS and Driver.GS.
 * This is because most of Driver.GS is hardware specific,
 * and what is not (the slightly abstracted aspects), is
 * directly connected to kernal, and might as well be
 * considered part of the same process.
 */

#include "graphics/paletteman.h"

#include "immortal/immortal.h"

namespace Immortal {

/*
 *
 * -----                          -----
 * ----- Screen Drawing Functions -----
 * -----                          -----
 *
 */

void ImmortalEngine::drawUniv() {
	// This is where the entire screen actually comes together
	_myViewPortX = _viewPortX & 0xFFFE;
	_myViewPortY = _viewPortY;

	_num2DrawItems = 0;

	_myUnivPointX = !(_myViewPortX & (kChrW - 1)) + kViewPortSpX;
	_myUnivPointY = !(_myViewPortY & (kChrH - 1)) + kViewPortSpY;

	//makeMyCNM();
	//drawBGRND();                          // Draw floor parts of leftmask rightmask and maskers
	//addRows();                                // Add rows to drawitem array
	//addSprites();                         // Add all active sprites that are in the viewport, into a list that will be sorted by priority
	//sortDrawItems();                      // Sort said items
	//drawItems();                          // Draw the items over the background
}

void ImmortalEngine::copyToScreen() {
	/* copyRectToSurface will apply the screenbuffer to the ScummVM surface.
	 * We want to do 320 bytes per scanline, at location (0,0), with a
	 * size of 320x200.
	 */
	_mainSurface->copyRectToSurface(_screenBuff, kResH, 0, 0, kResH, kResV);

	if (_draw == 1) {
		g_system->copyRectToScreen((byte *)_mainSurface->getPixels(), kResH, 0, 0, kResH, kResV);
		g_system->updateScreen();
	}
}

void ImmortalEngine::clearScreen() {
	// Fill the visible screen with black pixels by drawing a rectangle

	//rect(32, 20, 256, 128, 0)
	// This is just temporary, until rect() is implemented
	for (int y = 0; y < 128; y++) {
		for (int x = 0; x < 256; x++) {
			_screenBuff[((y + 20) * kResH) + (x + 32)] = 0;
		}
	}

	_penX = kTextLeft;
	_penY = kTextTop;

	if ((_dontResetColors & kMaskLow) == 0) {
		useNormal();
	}

	// Make sure it takes effect right away
	copyToScreen();
}

// These functions are not yet implemented
void ImmortalEngine::mungeBM() {}
void ImmortalEngine::blit() {}
void ImmortalEngine::blit40() {}
void ImmortalEngine::sBlit() {}
void ImmortalEngine::scroll() {}
void ImmortalEngine::makeMyCNM() {}
// -----

void ImmortalEngine::drawIcon(int img) {
	superSprite(&_dataSprites[kObject], ((kObjectWidth / 2) + kScreenLeft) + _penX, _penY + (kObjectY + kScreenTop), img, kScreenBMW, _screenBuff, 0, 200);
	_penY += kObjectHeight;
}

void ImmortalEngine::addRows() {
	// I'm not really sure how this works yet
	int i = _num2DrawItems;
	_tPriority[i] = !(!(_myViewPortY & (kChrH - 1)) + _myViewPortY);

	for (int j = 0; j != kViewPortCH + 4; j++, i++) {
		_tIndex[i] = (j << 5) | 0x8000;
		_tPriority[i] = _tPriority[i] - kChrH;
	}
	_num2DrawItems = i;
}

void ImmortalEngine::addSprite(uint16 vpX, uint16 vpY, SpriteName s, int img, uint16 x, uint16 y, uint16 p) {
	debug("adding sprite...");
	if (_numSprites != kMaxSprites) {
		if (x >= (kResH + kMaxSpriteLeft)) {
			x |= kMaskHigh;                         // Make it negative
		}

		_sprites[_numSprites]._x = (x << 1) + vpX;

		if (y >= (kMaxSpriteAbove + kResV)) {
			y |= kMaskHigh;
		}

		_sprites[_numSprites]._y = (y << 1) + vpY;

		if (p >= 0x80) {
			p |= kMaskHigh;
		}

		_sprites[_numSprites]._priority = ((p + y) ^ 0xFFFF) + 1;

		_sprites[_numSprites]._image = img;
		_sprites[_numSprites]._dSprite = &_dataSprites[s];
		_sprites[_numSprites]._on = 1;
		_numSprites += 1;
		debug("sprite added");
	} else {
		debug("Max sprites reached beeeeeep!!");
	}
}

void ImmortalEngine::addSprites() {
	// My goodness this routine is gross
	int tmpNum = _num2DrawItems;
	for (int i = 0; i < kMaxSprites; i++) {
		// If the sprite is active
		/* TODO
		 * This is commented out for testing until the issue with the function is resolved
		 */
		if (/*_sprites[i]._on*/0 == 1) {
			// If sprite X is an odd number???
			if ((_sprites[i]._x & 1) != 0) {
				debug("not good! BRK");
				return;
			}

			int tmpx = (_sprites[i]._x - kMaxSpriteW) - _myViewPortX;
			if (tmpx < 0) {
				if (tmpx + (kMaxSpriteW * 2) < 0) {
					continue;
				}
			} else if (tmpx >= kViewPortW) {
				continue;
			}

			int tmpy = (_sprites[i]._y - kMaxSpriteH) - _myViewPortY;
			if (tmpy < 0) {
				if (tmpy + (kMaxSpriteH * 2) < 0) {
					continue;
				}
			} else if (tmpy >= kViewPortH) {
				continue;
			}

			DataSprite *tempD = _sprites[i]._dSprite;
			//debug("what sprite is this: %d %d %d", i, _sprites[i]._image, _sprites[i]._dSprite->_images.size());
			Image *tempImg = &(tempD->_images[_sprites[i]._image]);
			int sx = ((_sprites[i]._x + tempImg->_deltaX) - tempD->_cenX) - _myViewPortX;
			int sy = ((_sprites[i]._y + tempImg->_deltaY) - tempD->_cenY) - _myViewPortY;

			if (sx >= 0) {
				if (sx >= kViewPortW) {
					continue;
				}
			} else if ((sx + tempImg->_rectW) <= 0) {
				continue;
			}

			if (sy >= 0) {
				if (sy >= kViewPortH) {
					continue;
				}
			} else if ((sy + tempImg->_rectH) <= 0) {
				continue;
			}

			// Sprite is actually in viewport, we can now enter it in the sorting array
			_tIndex[_num2DrawItems] = i;
			_tPriority[_num2DrawItems] = _sprites[i]._priority;
			tmpNum++;
			if (tmpNum == kMaxDrawItems) {
				break;
			}
		}
	}
	_num2DrawItems = tmpNum;
}

void ImmortalEngine::sortDrawItems() {
	/* Just an implementation of bubble sort.
	 * Sorting largest to smallest entry, by simply
	 * swapping every two entries if they are not in order.
	 */

	int top = _num2DrawItems;
	bool bailout;

	do {
		// Assume that the list is sorted
		bailout = true;
		for (int i = 1; i < top; i++) {
			if (_tPriority[i] > _tPriority[i - 1]) {
				uint16 tmp = _tPriority[i];
				_tPriority[i] = _tPriority[i - 1];
				_tPriority[i - 1] = tmp;

				// List was not sorted yet, therefor we need to check it again
				bailout = false;
			}
		}
		/* After every pass, the smallest entry is at the end of the array, so we move
		 * the end marker back by one
		 */
		top--;
	} while (bailout == false);
}

void ImmortalEngine::drawBGRND() {
	// 'tmp' is y, 'cmp' is x

	uint16 pointX = _myUnivPointX;
	uint16 pointY = _myUnivPointY;

	for (int y = kViewPortCH + 1, y2 = 0; y != 0; y--, y2++) {
		for (int x = 0; x < (kViewPortCW + 1); x += (kViewPortCW + 1)) {
			uint16 BTS = _myModLCNM[y2][x];

			if (kIsBackground[BTS] != 0) {
				// Low Floor value, draw tile as background
				drawSolid(_myCNM[y2][x], pointX, pointY);

			} else if (kChrMask[BTS] >= 0x8000) {
				// Right Mask, draw upper left hand corner (ULHC) of floor
				drawULHC(_myCNM[y2][x], pointX, pointY);

			} else if (kChrMask[BTS] != 0) {
				// Left Mask, draw upper right hand corner (UPHC) of floor
				drawURHC(_myCNM[y2][x], pointX, pointY);
			}
			pointX += kChrW;                                    // This (and the H version) could be added to the for loop iterator arugment
		}
		pointX -= (kChrW * (kViewPortCW + 1));                  // They could have also just done pointX = _myUnivPointX
		pointY += kChrH;
	}
}

void ImmortalEngine::drawItems() {
	for (int i = 0; i < (kViewPortCW + 1); i++) {
		_columnIndex[i] = 0;
	}

	for (int i = 0; i < (kViewPortCW + 1); i++) {
		_columnTop[i] = _myUnivPointY;
	}

	_columnX[0] = _myUnivPointX;
	for (int i = 1; i < (kViewPortCW + 1); i++) {
		_columnX[i] = _myUnivPointX + kChrW;
	}

	// This is truly horrible, I should double check that this is the intended logic
	int n = 0;
	uint16 rowY = 0;
	do {
		uint16 index = _tIndex[n];
		if (index >= 0x8000) {                              // If negative, it's a row to draw
			// rowY is (I think) the position of the start of the scroll window within the tile data
			rowY = (index & 0x7FFF) + _myUnivPointY;

			// The background is a matrix of rows and columns, so for each column, we draw each row tile
			for (int i = 0; (i < (kViewPortCW + 1)); i++) {
				//draw the column of rows
				while (_columnIndex[i] < ((kViewPortCW + 1) * (kViewPortCH + 1))) {

					uint16 k = _myModLCNM[i][_columnIndex[i]];
					// ******* This is just so that the array can be indexed right now, will remove when myModLCNM is actually useable
					k = 0;
					// *****************************
					if ((rowY - kChrDy[k]) < _columnTop[i]) {
						break;
					}
					if (kIsBackground[k] == 0) {
						// If it's a background tile, we already drew it (why is it in here then??)
						if (kChrMask[k] >= 0x8000) {
							// Right Mask, draw lower right hand corner (LRHC)
							drawLRHC(_myCNM[i][_columnIndex[i]], _columnTop[i], _columnX[i]);

						} else if (kChrMask[k] == 0) {
							// Floor or cover, draw the whole CHR
							drawSolid(_myCNM[i][_columnIndex[i]], _columnTop[i], _columnX[i]);

						} else {
							// Left Mask, draw lower left hand corner (LLHC)
							drawLLHC(_myCNM[i][_columnIndex[i]], _columnTop[i], _columnX[i]);
						}
					}
					_columnTop[i] += kChrH;
					_columnIndex[i] += (kViewPortCW + 1);
				}
			}

		} else {
			// If positive, it's a sprite
			uint16 x = (_sprites[index]._x - _myViewPortX) + kVSX;
			uint16 y = (_sprites[index]._y - _myViewPortY) + kVSY;
			superSprite(_sprites[index]._dSprite, x, y, _sprites[index]._image, kVSBMW, _screenBuff, kMySuperTop, kMySuperBottom);
		}
		n++;
	} while (n != _num2DrawItems);
}

void ImmortalEngine::backspace() {
	// Just moves the drawing position back by a char, and then draws an empty rect there
	_penX -= 8;
	//rect(_penX + 32, 40, 8, 16, 0);

	// The Y is hardcoded here presumably because it's only used for the certificate
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 8; x++) {
			_screenBuff[((y + 40) * kResH) + (x + (_penX + 32))] = 0;
		}
	}
}

void ImmortalEngine::printByte(int b) {
	int hundreds = 0;
	int tens = 0;

	while ((b - 100) >= 0) {
		hundreds++;
		b -= 100;
	}

	if (hundreds > 0) {
		printChr(char (hundreds + '0'));
	}

	while ((b - 10) >= 0) {
		tens++;
		b -= 10;
	}

	if (tens > 0) {
		printChr(char (tens + '0'));
	}

	printChr(char (b + '0'));
}

void ImmortalEngine::printChr(char c) {
	// This draws a character from the font sprite table, indexed as an ascii char, using superSprite
	c &= kMaskASCII;                // Grab just the non-extended ascii part

	if (c == ' ') {
		_penX += 8;                 // A space just moves the position on the screen to draw ahead by the size of a space
		return;
	}

	// Why is single quote done twice?
	if (c == 0x27) {
		_penX -= 2;
	}

	switch (c) {
	case 'm':
	case 'w':
	case 'M':
	case 'W':
		_penX += 8;
	// fall through
	default:
		break;
	}

	if ((((c >= 'A') && (c <= 'Z'))) || ((c == kGaugeOn) || (c == kGaugeOff))) {
		_penX += 8;
	}

	switch (c) {
	case 'i':
		_penX -= 3;
		break;
	case 'j':
	// fall through
	case 't':
		_penX -= 2;
		break;
	case 'l':
		_penX -= 4;
	// fall through
	default:
		break;
	}

	uint16 x = _penX + kScreenLeft;
	if (x < _dataSprites[kFont]._cenX) {
		return;
	}

	uint16 y = _penY + kScreenTop;
	if (y < _dataSprites[kFont]._cenY) {
		return;
	}

	superSprite(&_dataSprites[kFont], x, y, (int) c, kScreenBMW, _screenBuff, kSuperTop, kSuperBottom);

	// Back tick quote
	if (c == 0x27) {
		_penX -= 2;
	}

	// If the letter was a captial T, the next letter should be a little closer
	if (c == 'T') {
		_penX -= 2;
	}

	_penX += 8;
}

/*
 *
 * -----            -----
 * ----- Asset Init -----
 * -----            -----
 *
 */

void ImmortalEngine::clearSprites() {
	// Just sets the 'active' flag on all possible sprites to 0
	for (int i = 0; i < kMaxSprites; i++) {
		_sprites[i]._on = 0;
	}
}

void ImmortalEngine::cycleFreeAll() {
	// Sets all cycle indexes to -1, indicating they are available
	for (int i = 0; i < kMaxCycles; i++) {
		_cycles[i]._index = -1;
	}
}

void ImmortalEngine::loadMazeGraphics(int m) {
	char mazeNum = m + '0';
	loadUniv(mazeNum);
	//setColors(_palUniv);
}

int ImmortalEngine::loadUniv(char mazeNum) {
	int lData = 0;
	int lStuff = 0x26;

	// We start by loading the mazeN.CNM file with loadIFF (a little silly since we know this isn't compressed)
	Common::String sCNM = "MAZE" + Common::String(mazeNum) + ".CNM";
	Common::SeekableReadStream *mazeCNM = loadIFF(sCNM);
	if (!mazeCNM) {
		debug("Error, couldn't load maze %d.CNM", mazeNum);
		return -1;
	}
	debug("Size of maze CNM: %ld", mazeCNM->size());

	// The logical CNM contains the contents of mazeN.CNM, with every entry being bitshifted left once
	_logicalCNM = (uint16 *)malloc(mazeCNM->size());
	mazeCNM->seek(0);
	for (int i = 0; i < (mazeCNM->size() / 2); i++) {
		_logicalCNM[i] = mazeCNM->readUint16LE();
	}

	// This is where the source defines the location of the pointers for modCNM, lModCNM, and then the universe properties
	// So in similar fasion, here we will create the struct for universe
	_univ = new Univ();

	// Next we load the mazeN.UNV file, which contains the compressed data for the Univ, CNM, and CBM
	Common::String sUNV = "MAZE" + Common::String(mazeNum) + ".UNV";
	Common::SeekableReadStream *mazeUNV = loadIFF(sUNV);
	if (!mazeUNV) {
		debug("Error, couldn't load maze %d.UNV", mazeNum);
		return -1;
	}
	debug("Size of maze UNV: %ld", mazeUNV->size());

	// This is also where the pointer to CNM is defined, because it is 26 bytes after the pointer to Univ. However for our purposes these are separate

	// After which, we set data length to be the total size of the file
	lData = mazeUNV->size();

	// The first data we need is found at index 20
	mazeUNV->seek(0x20);

	// The view port of the level is longer than it is wide, so there are more columns than rows
	// numCols = rectX / 64 (charW)
	_univ->_rectX    = mazeUNV->readUint16LE() << 1;
	_univ->_numCols  = _univ->_rectX >> 6;
	_univ->_num2Cols = _univ->_numCols << 1;

	// univRectY is mazeUNV[22]
	// numRows = rectY / 32 (charH)
	_univ->_rectY    = mazeUNV->readUint16LE();
	_univ->_numRows  = _univ->_rectY >> 5;
	_univ->_num2Rows = _univ->_numRows << 1;

	// Technically this is done right after decompressing the data, but it is more relevant here for now
	_univ->_num2Cells = _univ->_num2Cols * _univ->_numRows;

	// If there are animations (are there ever?), the univ data is expanded from 26 to include them
	if (mazeUNV->readUint16LE() != 0) {
		debug("there are animations??");
		mazeUNV->seek(0x2C);
		lStuff += mazeUNV->readUint16LE();
	}

	// lData is everything from the .UNV file after the universe properties
	lData -= lStuff;

	// At this point in the source, the data after universe properties is moved to the end of the heap where it can be uncompressed back to the CNM pointer

	// We then uncompress all of that data
	mazeUNV->seek(lStuff);
	_dataBuffer = unCompress((Common::File *)mazeUNV, lData);
	debug("size of uncompressed CNM/CBM data %ld", _dataBuffer->size());

	// Check every entry in the CNM (while we add them). The highest number is the total number of tiles in the file
	_CNM = (uint16 *)malloc(_univ->_num2Cells);
	_univ->_numChrs = 0;
	_dataBuffer->seek(0);

	// The CNM is 0x500 bytes (usually), with each entry being a word, so we need 0x500 / 2
	for (int i = 0; i < _univ->_num2Cells / 2; i++) {
		_CNM[i] = _dataBuffer->readUint16LE();
		if (_CNM[i] >= _univ->_numChrs) {
			_univ->_numChrs = _CNM[i];
		}
	}
	_univ->_numChrs++;							// The 0th tile is still a tile, so inc one more time to account for it
	debug("Number of Chars: %d", _univ->_numChrs);
	_univ->_num2Chrs = _univ->_numChrs << 1;

	// Set the databuffer back to position 0 for now. The remaining data in databuffer is the CBM (we don't really need to do this, but for clarity we will)
	_dataBuffer->seek(0);

	// In the source, this is where we munge the CBM, which is to say that we sort of combine the CBM into the CNM to create routines that draw tiles from their component characters, which are stored in sequence by tile
	int lCNMCBM = mungeCBM(_univ->_num2Chrs);
	
	debug("nchrs %04X, n2cells %04X, univX %04X, univY %04X, cols %04X, rows %04X, lstuff %04X", _univ->_numChrs, _univ->_num2Cells, _univ->_rectX, _univ->_rectY, _univ->_numCols, _univ->_numRows, lStuff);

	// We don't actually want to blister any rooms yet, so we give it a POV of (0,0)
	makeBlisters(0, 0);

	// We return the final size of everything by adding logicalCNM + modCNM + modLogicalCNM + univ + length of expanded CNM/CBM
	return mazeCNM->size() /*+ _modCNM.size() + _modLCNM.size()*/ + lStuff + lCNMCBM;
}

void ImmortalEngine::makeBlisters(int povX, int povY) {
}

void ImmortalEngine::loadSprites() {
	/* This is a bit weird, so I'll explain.
	 * In the source, this routine loads the files onto the heap, and then
	 * goes through a table of sprites in the form file_index, sprite_num, center_x, center_y.
	 * It uses file_index to get a pointer to the start of the file on the heap,
	 * which it then uses to set the center x/y variables in the file itself.
	 * ie. file_pointer[file_index]+((sprite_num<<3)+4) = center_x.
	 * We aren't going to have the sprite properties inside the file data, so instead
	 * we have an array of all game sprites _dataSprites which is indexed
	 * soley by a sprite number now. This also means that a sprite itself has a reference to
	 * a datasprite, instead of the sprite index and separately the file pointer. Datasprite
	 * is what needs the file, so that's where the pointer is. The index isn't used by
	 * the sprite or datasprite themselves, so it isn't a member of either of them.
	 */

	Common::String spriteNames[] = {"MORESPRITES.SPR", "NORLAC.SPR", "POWWOW.SPR", "TURRETS.SPR",
	                                "WORM.SPR", "IANSPRITES.SPR", "LAST.SPR", "DOORSPRITES.SPR",
	                                "GENSPRITES.SPR", "DRAGON.SPR", "MORDAMIR.SPR", "FLAMES.SPR",
	                                "ROPE.SPR", "RESCUE.SPR", "TROLL.SPR", "GOBLIN.SPR", "WIZARDA.SPR",
	                                "WIZARDB.SPR", "ULINDOR.SPR", "SPIDER.SPR", "DRAG.SPR"
	                               };

	// Number of sprites in each file
	int spriteNum[] = {10, 5, 7, 10, 4, 6, 3, 10, 5, 3, 2, 1, 3, 2, 9, 10, 8, 3, 9, 10, 9};

	// Pairs of (x,y) for each sprite
	// Should probably have made this a 2d array, oops
	uint16 centerXY[] = {16, 56, 16, 32, 27, 39, 16, 16, 32, 16, 34, 83, 28, 37, 8, 12, 8, 19, 24, 37,
	/* Norlac      */    46, 18, 40, 0, 8, 13, 32, 48, 32, 40,
	/* Powwow      */    53, 43, 28, 37, 27, 37, 26, 30, 26, 30, 26, 29, 28, 25,
	/* Turrets     */    34, 42, 28, 37, 24, 32, 32, 56, 26, 56, 8, 48, 8, 32, 8, 14, 8, 24, 32, 44,
	/* Worm        */    20, 65, 25, 46, 9, 56, 20, 53,
	/* Iansprites  */    24, 50, 32, 52, 32, 53, 32, 52, 40, 16, 40, 16,
	/* Last        */    32, 56, 24, 32, 24, 36,
	/* Doorsprites */    0, 64, 4, 49, 18, 49, 18, 56, 24, 32, 24, 16, 24, 56, 24, 32, 24, 32, 36, 32,
	/* Gensprites  */    16, 44, 16, 28, 32, 24, 34, 45, 20, 28,
	/* Dragon      */    24, 93, 32, 48, 0, 64,
	/* Mordamir    */    104, 104, 30, 30,
	/* Flames      */    64, 0,
	/* Rope        */    0, 80, 32, 52, 32, 40,
	/* Rescue      */    0, 112, 0, 112,
	/* Troll       */    28, 38, 28, 37, 28, 37, 31, 38, 28, 37, 25, 39, 28, 37, 28, 37, 28, 37,
	/* Goblin      */    28, 38, 30, 38, 26, 37, 30, 38, 26, 37, 26, 37, 26, 37, 26, 37, 26, 36, 44, 32,
	/* Wizarda     */    28, 37, 28, 37, 28, 37, 28, 37, 28, 37, 28, 37, 28, 37, 28, 37,
	/* Wizardb     */    28, 37, 28, 37, 28, 37,
	/* Ulindor     */    42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
	/* Spider      */    64, 44, 64, 44, 64, 44, 64, 44, 64, 44, 64, 44, 64, 44, 64, 44, 64, 44, 64, 44,
	/* Drag        */    19, 36, 19, 36, 19, 36, 19, 36, 19, 36, 19, 36, 19, 36, 19, 36, 19, 36
	};

	// s = current sprite index, f = current file index, n = current number of sprites for this file
	int s = 0;
	for (int f = 0; f < 21; f++) {
		// For every sprite file, open it and get the pointer
		Common::SeekableReadStream *file = loadIFF(spriteNames[f]);

		for (int n = 0; n < (spriteNum[f] * 2); n += 2, s++) {
			// For every data sprite in the file, make a datasprite and initialize it
			DataSprite d;
			initDataSprite(file, &d, n / 2, centerXY[s * 2], centerXY[(s * 2) + 1]);
			_dataSprites[s] = d;
		}
	}
}

void ImmortalEngine::loadWindow() {
	/* Technically, the source uses loadIFF to just move the window bitmap from
	 * the file straight into the virtual screen buffer. However, since
	 * we will have to extract each pixel from the buffer to use with
	 * Surface anyway, we are doing the extracting in advance, since that is
	 * more or less what is happening at this point in the source. This will
	 * likely be combined with something else in the future.
	 */

	// Initialize the window bitmap
	Common::File f;

	if (f.open("WINDOWS.BM")) {

		/* The byte buffer for the screen (_screenBuff) has one byte for
		 * every pixel, with the resolution of the game being 320x200.
		 * For a bitmap like the window frame, all we need to do is
		 * extract the pixel out of each nyble (half byte) of the data,
		 * by looping over it one row at a time.
		 */

		byte pixel;
		int pos;
		for (int y = 0; y < kResV; y++) {
			for (int x = 0; x < kResH; x += 2) {
				pos = (y * kResH) + x;
				pixel = f.readByte();
				_screenBuff[pos]     = (pixel & kMask8High) >> 4;
				_screenBuff[pos + 1] =  pixel & kMask8Low;
			}
		}

		// Now that the bitmap is processed and stored in a byte buffer, we can close the file
		f.close();

	} else {
		// Should probably give an error here
		debug("oh nose :(");
	}
}

void ImmortalEngine::loadFont() {
	// Initialize the font data sprite
	Common::SeekableReadStream *f = loadIFF("FONT.SPR");
	DataSprite d;

	if (f) {
		initDataSprite(f, &d, 0, 16, 0);
		_dataSprites[kFont] = d;

	} else {
		debug("file doesn't exist!");
	}

}

Common::SeekableReadStream *ImmortalEngine::loadIFF(Common::String fileName) {
	/* Technically the way this works in the source is that it loads the file
	 * to a destination address, and then checks the start of that address, and
	 * if it needs to uncompress, it gives that same address to the uncompress
	 * routine, overwriting the file in it's place. This is of course slightly
	 * different here, for simplicity we are not overwriting the original file
	 * pointer, instead just returning either a compressed or uncompressed
	 * file pointer.
	 */

	Common::File f;
	if (!f.open(Common::Path(fileName))) {
		debug("*surprised pikachu face*");
		return nullptr;
	}

	/* This isn't the most efficient way to do this (could just read a 32bit uint and compare),
	 * but this makes it more obvious what the source was doing. We want to know if the 4 bytes
	 * at file[8] are 'C' 'M' 'P' '0', so this grabs just the ascii bits of those 4 bytes,
	 * allowing us to directly compare it with 'CMP0'.
	 */
	char compSig[] = "CMP0";
	char sig[] = "0000";

	f.seek(8);

	for (int i = 0; i < 4; i++) {
		sig[i] = f.readByte() & kMaskASCII;
	}

	if (strcmp(sig, compSig) == 0) {
		debug("compressed");

		/* The size of the compressed data is stored in the header, but doesn't
		 * account for the FORM part?? Also, **technically** this is a uint32LE,
		 * but the engine itself actually /doesn't/ use it like that. It only
		 * decrements the first word (although it compares against the second half,
		 * as if it is expecting that to be zero? It's a little bizarre).
		 */
		f.seek(6);
		int len = f.readUint16LE() - 4;

		// Compressed files have a 12 byte header before the data
		f.seek(12);
		return unCompress(&f, len);
	}
	// Gotta remember we just moved the cursor around a bunch, need to reset it to read the file
	f.seek(SEEK_SET);

	byte *out = (byte *)malloc(f.size());
	f.read(out, f.size());
	return new Common::MemoryReadStream(out, f.size(), DisposeAfterUse::YES);
}


/*
 *
 * -----                   -----
 * ----- Palette Functions -----
 * -----                   -----
 *
 */

/* Palettes on the Apple IIGS:
 * In High-res mode you have 2 options: 320x200 @ 4bpp or 320x640 @ 2bpp.
 * The Immortal uses the former, giving us 16 colours to use
 * for any given pixel on the screen (ignoring per scanline palettes because
 * The Immortal does not use them). This 16 colour palette is made of 2 byte
 * words containing the RGB components in the form 0RGB.
 *
 * The equivalent palette for ScummVM is a byte stream of up to 256
 * colours composed of 3 bytes each, ending with a transparency byte.
 *
 * Because each colour in the game palette is only a single nyble (4 bits),
 * we also need to multiply the nyble up to the size of a byte (* 16, or << 4).
 */

void ImmortalEngine::loadPalette() {
	// The palettes are stored at a particular location in the disk, this just grabs them
	Common::File d;
	d.open("IMMORTAL.dsk");
	d.seek(kPaletteOffset);

	// Each palette is stored after each other at this kPaletteOffset in the disk
	uint16 *pals[4] = {_palDefault, _palWhite, _palBlack, _palDim};

	// So we can just grab 16 colours at a time and store them to the appropriate palette
	for (int p = 0; p < 4; p++) {
		for (int i = 0; i < 16; i++) {
			pals[p][i] = d.readUint16LE();
		}
	}

	// And now we are done with the file
	d.close();
}

void ImmortalEngine::setColors(uint16 pal[]) {
	// The RGB palette is 3 bytes per entry, and each byte is a colour
	for (int i = 0; i < 16; i++) {

		// The palette gets masked so it can update only specific indexes and uses FFFF to do so. However the check is simply for a negative
		if (pal[i] < kMaskNeg) {

			// Green is already the correct size, being the second nyble (00G0)
			// Red is in the first nyble of the high byte, so it needs to move right by 4 bits (0R00 -> 00R0)
			// Blue is the first nyble of the first byte, so it needs to move left by 4 bits (000B -> 00B0)
			// We also need to repeat the bits so that the colour is the same proportion of 255 as it is of 15
			_palRGB[(i * 3)]     = ((pal[i] & kMaskRed) >> 4) | ((pal[i] & kMaskRed) >> 8);
			_palRGB[(i * 3) + 1] = (pal[i] & kMaskGreen)     | ((pal[i] & kMaskGreen) >> 4);
			_palRGB[(i * 3) + 2] = (pal[i] & kMaskBlue)      | ((pal[i] & kMaskBlue) << 4);
		}
	}
	// Palette index to update first is 0, and there are 16 colours to update
	g_system->getPaletteManager()->setPalette(_palRGB, 0, 16);
	g_system->updateScreen();
}

void ImmortalEngine::fixColors() {
	// Pretty silly that this is done with two separate variables, could just index by one...
	if (_dim == 1) {
		if (_usingNormal == 1) {
			useDim();
		}
	} else {
		if (_usingNormal == 0) {
			useNormal();
		}
	}
}

void ImmortalEngine::pump() {
	// Flashes the screen (except the frame thankfully) white, black, white, black, then clears the screen and goes back to normal
	useWhite();
	g_system->updateScreen();
	Utilities::delay(2);
	useBlack();
	g_system->updateScreen();
	Utilities::delay(2);
	useWhite();
	g_system->updateScreen();
	Utilities::delay(2);
	useBlack();
	g_system->updateScreen();
	clearScreen();
	// Why does it do this instead of setting _dontResetColors for clearScreen() instead?
	useNormal();
}

void ImmortalEngine::fadePal(uint16 pal[], int count, uint16 target[]) {
	/* This will fade the palette used by everything inside the game screen
	 * but will not touch the window frame palette. It essentially takes the
	 * color value nyble, multiplies it by a multiplier, then takes the whole
	 * number result and inserts it into the word at the palette index of the
	 * temporary palette. This could I'm sure, be done with regular multiplication
	 * and division operators, but in case the bits that get dropped are otherwise
	 * kept, this is a direct translation of the bit manipulation sequence.
	 */
	uint16 maskPal[16] = {0xFFFF, 0x0000, 0x0000, 0x0000,
	                      0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	                      0xFFFF, 0xFFFF, 0xFFFF, 0x0000,
	                      0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
	                     };

	uint16 result;
	uint16 temp;

	for (int i = 15; i >= 0; i--) {
		result = maskPal[i];
		if (result == 0) {
			// If the equivalent maskPal entry is 0, then it is a colour we want to fade
			result = pal[i];
			if (result != 0xFFFF) {
				// If we have not reached FFFF in one direction or the other, we keep going

				// Blue = 0RGB -> 000B -> 0Bbb -> bb0B -> 000B
				result = (xba(mult16((result & kMaskFirst), count))) & kMaskFirst;

				// Green = 0RGB -> 00RG -> 000G -> 0Ggg -> gg0G -> 000G -> 00G0 -> 00GB
				temp = mult16(((pal[i] >> 4) & kMaskFirst), count);
				temp = (xba(temp) & kMaskFirst) << 4;
				result = temp | result;

				// Red = 0RGB -> GB0R -> 000R -> 0Rrr -> rr0R -> 000R -> 0R00 -> 0RGB
				temp = xba(pal[i]) & kMaskFirst;
				temp = xba(mult16(temp, count));
				temp = xba(temp & kMaskFirst);
				result = temp | result;
			}
		}
		target[i] = result;
	}
}

void ImmortalEngine::fade(uint16 pal[], int dir, int delay) {
	// This temp palette will have FFFF in it, which will be understood as masks by setColors()
	uint16 target[16];
	uint16 count;

	// Originally used a branch, but this is functionally identical and much cleaner
	count = dir * 256;

	while (count <= 256) {
		fadePal(pal, count, target);
		Utilities::delay8(delay);
		setColors(target);

		// Same as above, it was originally a branch, this does the same thing
		count += (dir == 0) ? 16 : -16;
	}
}

// These two can probably be removed and instead use an enum to declare fadeout/in
void ImmortalEngine::fadeOut(int j) {
	fade(_palDefault, 1, j);
}

void ImmortalEngine::normalFadeOut() {
	fadeOut(15);
}

void ImmortalEngine::slowFadeOut() {
	fadeOut(28);
}

void ImmortalEngine::fadeIn(int j) {
	fade(_palDefault, 0, j);
}

void ImmortalEngine::normalFadeIn() {
	fadeIn(15);
}

// These two can probably be removed since the extra call in C doesn't have the setup needed in ASM
void ImmortalEngine::useBlack() {
	setColors(_palBlack);
}
void ImmortalEngine::useWhite() {
	setColors(_palBlack);
}

void ImmortalEngine::useNormal() {
	setColors(_palDefault);
	_usingNormal = 1;
}

void ImmortalEngine::useDim() {
	setColors(_palDim);
	_usingNormal = 0;
}


/*
 *
 * -----                 -----
 * ----- Input Functions -----
 * -----                 -----
 *
 */

void ImmortalEngine::waitKey() {
	bool wait = true;
	while (wait == true) {
		if (getInput() == true) {
			wait = false;
		}
	}
}

// This was originally in Motives, which is weird since it seems more like an engine level function, so it's in kernal now
void ImmortalEngine::waitClick() {
	bool wait = true;
	while (wait == true) {
		if (getInput() == true) {
			Utilities::delay(25);
			if (buttonPressed() || firePressed()) {
				wait = false;
			}
		}
	}
}

// These functions are not yet implemented
void ImmortalEngine::blit8() {}
void ImmortalEngine::addKeyBuffer() {}
void ImmortalEngine::clearKeyBuff() {}
void ImmortalEngine::userIO() {}
void ImmortalEngine::pollKeys() {}
void ImmortalEngine::noNetwork() {}
bool ImmortalEngine::getInput() {
	return true;
}
// ----

/*
 *
 * -----                       -----
 * ----- Sound/Music Functions -----
 * -----                       -----
 *
 */

void ImmortalEngine::toggleSound() {
	// Interestingly, this does not mute or turn off the sound, it actually pauses it
	_themePaused = !_themePaused;
	fixPause();
}

void ImmortalEngine::fixPause() {
	/* The code for this is a little strange, but the idea is that you have
	 * a level theme, and a combat theme, that can both be active. So first you
	 * pause the level theme, and then you pause the combat theme.
	 * The way it does it is weird though. Here's the logic:
	 * if playing either text or maze song, check if the theme is paused. else, just go ahead and pause.
	 * Same thing for combat song. A little odd.
	 */

	// This is a nasty bit of code isn't it? It's accurate to the source though :D
	switch (_playing) {
	case kSongText:
	// fall through
	case kSongMaze:
		if (_themePaused) {
			musicUnPause(_themeID);
			break;
		}
	// fall through
	default:
		musicPause(_themeID);
		break;
	}

	// Strictly speaking this should probably be a single function called twice, but the source writes out both so I will too
	switch (_playing) {
	case kSongCombat:
		if (_themePaused) {
			musicUnPause(_combatID);
			break;
		}
	// fall through
	default:
		musicPause(_combatID);
		break;
	}

}

// *** These two functions will be in music.cpp, they just aren't implemented yet ***
void ImmortalEngine::musicPause(int sID) {}
void ImmortalEngine::musicUnPause(int sID) {}
// ***

Song ImmortalEngine::getPlaying() {
	// Temporary value
	return kSongMaze;
}

// These functions are not yet implemented
void ImmortalEngine::playMazeSong() {}
void ImmortalEngine::playCombatSong() {}
void ImmortalEngine::playTextSong() {}
// ----

void ImmortalEngine::loadSingles(Common::String songName) {
	debug("%s", songName.c_str());
}

void ImmortalEngine::stopMusic() {
	//musicStop(-1)
	_playing = kSongNothing;
	//stopSound();
}

/*
 *
 * -----                         -----
 * ----- 'Pen' related Functions -----
 * -----                         -----
 *
 */

// This sets the pen to a given x,y point
void ImmortalEngine::setPen(uint16 penX, uint16 penY) {
	_penX = penX & kMaskLow;
	if ((penY & kMaskLow) < 200) {
		_penY = penY & kMaskLow;
	}

	else {
		_penY = penY | kMaskHigh;
	}
}

void ImmortalEngine::center() {
	_penX = ((uint16) 128) - (kObjectWidth / 2);
}

// Reset the X position and move the Y position down by 16 pixels
void ImmortalEngine::carriageReturn() {
	_penY += 16;
	_penX = kTextLeft;
}

} // namespace Immortal
