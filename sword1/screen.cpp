/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "screen.h"
#include "logic.h"
#include "sworddefs.h"
#include "text.h"
#include "resman.h"
#include "objectman.h"
#include "scummsys.h"
#include "common/util.h"
#include "system.h"
#include "menu.h"
#include "sword1.h"
#include "animation.h"

namespace Sword1 {

#define SCROLL_FRACTION 16
#define MAX_SCROLL_DISTANCE 8
#define FADE_UP 1
#define FADE_DOWN -1

Screen::Screen(OSystem *system, ResMan *pResMan, ObjectMan *pObjMan) {
	_system = system;
	_resMan = pResMan;
	_objMan = pObjMan;
	_screenBuf = _screenGrid = NULL;
	_backLength = _foreLength = _sortLength = 0;
	_fadingStep = 0;
}

void Screen::useTextManager(Text *pTextMan) {
	_textMan = pTextMan;
}

int32 Screen::inRange(int32 a, int32 b, int32 c) { // return b(!) so that: a <= b <= c
	return (a > b) ? (a) : ((b < c) ? b : c);
}

void Screen::setScrolling(int16 offsetX, int16 offsetY) {
	if (!Logic::_scriptVars[SCROLL_FLAG])
		return ; // screen is smaller than 640x400 => no need for scrolling

	offsetX = inRange(0, offsetX, Logic::_scriptVars[MAX_SCROLL_OFFSET_X]);
	offsetY = inRange(0, offsetY, Logic::_scriptVars[MAX_SCROLL_OFFSET_Y]);

	if (Logic::_scriptVars[SCROLL_FLAG] == 2) { // first time on this screen - need absolute scroll immediately!
		_oldScrollX = Logic::_scriptVars[SCROLL_OFFSET_X] = (uint32)offsetX;
		_oldScrollY = Logic::_scriptVars[SCROLL_OFFSET_Y] = (uint32)offsetY;
		Logic::_scriptVars[SCROLL_FLAG] = 1;
		_fullRefresh = true;
	} else if (Logic::_scriptVars[SCROLL_FLAG] == 1) {
		// Because parallax layers may be drawn on the old scroll offset, we
		// want a full refresh not only when the scroll offset changes, but
		// also on the frame where they become the same.
		if (_oldScrollX != Logic::_scriptVars[SCROLL_OFFSET_X] || _oldScrollY != Logic::_scriptVars[SCROLL_OFFSET_Y])
			_fullRefresh = true;
		_oldScrollX = Logic::_scriptVars[SCROLL_OFFSET_X];
		_oldScrollY = Logic::_scriptVars[SCROLL_OFFSET_Y];
		int32 distX = inRange(-MAX_SCROLL_DISTANCE, _oldScrollX - offsetX, MAX_SCROLL_DISTANCE);
		int32 distY = inRange(-MAX_SCROLL_DISTANCE, _oldScrollY - offsetY, MAX_SCROLL_DISTANCE);
		if ((distX != 0) || (distY != 0))
			_fullRefresh = true;
		Logic::_scriptVars[SCROLL_OFFSET_X] -= distX;
		Logic::_scriptVars[SCROLL_OFFSET_Y] -= distY;
	}
}

void Screen::fadeDownPalette(void) {
	if (!_isBlack) { // don't fade down twice
		_fadingStep = 15;
		_fadingDirection = FADE_DOWN;
	}
}

void Screen::fadeUpPalette(void) {
	_fadingStep = 1;
	_fadingDirection = FADE_UP;
}

void Screen::fnSetPalette(uint8 start, uint16 length, uint32 id, bool fadeUp) {
	uint8 *palData = (uint8*)_resMan->openFetchRes(id);
	if (start == 0) // force color 0 to black
		palData[0] = palData[1] = palData[2] = 0;
	for (uint32 cnt = 0; cnt < length; cnt++) {
		_targetPalette[(start + cnt) * 4 + 0] = palData[cnt * 3 + 0] << 2;
		_targetPalette[(start + cnt) * 4 + 1] = palData[cnt * 3 + 1] << 2;
		_targetPalette[(start + cnt) * 4 + 2] = palData[cnt * 3 + 2] << 2;
	}
	_resMan->resClose(id);
	_isBlack = false;
	if (fadeUp) {
		_fadingStep = 1;
		_fadingDirection = FADE_UP;
		memset(_currentPalette, 0, 256 * 4);
		_system->setPalette(_currentPalette, 0, 256);
	} else
		_system->setPalette(_targetPalette + 4 * start, start, length);
}

void Screen::fullRefresh(void) {
	_fullRefresh = true;
	_system->setPalette(_targetPalette, 0, 256);
}

bool Screen::stillFading(void) {
	return (_fadingStep != 0);
}

bool Screen::showScrollFrame(void) {
	if ((!_fullRefresh) || Logic::_scriptVars[NEW_PALETTE] || (!Logic::_scriptVars[SCROLL_FLAG]))
		return false; // don't draw an additional frame if we aren't scrolling or have to change the palette
	if ((_oldScrollX == Logic::_scriptVars[SCROLL_OFFSET_X]) &&
		(_oldScrollY == Logic::_scriptVars[SCROLL_OFFSET_Y]))
		return false; // check again if we *really* are scrolling.

	uint16 avgScrlX = (uint16)(_oldScrollX + Logic::_scriptVars[SCROLL_OFFSET_X]) / 2;
	uint16 avgScrlY = (uint16)(_oldScrollY + Logic::_scriptVars[SCROLL_OFFSET_Y]) / 2;

	_system->copy_rect(_screenBuf + avgScrlY * _scrnSizeX + avgScrlX, _scrnSizeX, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);
	_system->updateScreen();
	return true;
}

void Screen::updateScreen(void) {
	if (Logic::_scriptVars[NEW_PALETTE]) {
		_fadingStep = 1;
		_fadingDirection = FADE_UP;
		fnSetPalette(0, 184, _roomDefTable[_currentScreen].palettes[0], true);
		fnSetPalette(184, 72, _roomDefTable[_currentScreen].palettes[1], true);
		Logic::_scriptVars[NEW_PALETTE] = 0;
	}
	if (_fadingStep) {
		fadePalette();
		_system->setPalette(_currentPalette, 0, 256);
	}

	uint16 scrlX = (uint16)Logic::_scriptVars[SCROLL_OFFSET_X];
	uint16 scrlY = (uint16)Logic::_scriptVars[SCROLL_OFFSET_Y];
	if (_fullRefresh) {
		_fullRefresh = false;
		uint16 copyWidth = SCREEN_WIDTH;
		uint16 copyHeight = SCREEN_DEPTH;
		if (scrlX + copyWidth > _scrnSizeX)
			copyWidth = _scrnSizeX - scrlX;
		if (scrlY + copyHeight > _scrnSizeY)
			copyHeight = _scrnSizeY - scrlY;
		_system->copy_rect(_screenBuf + scrlY * _scrnSizeX + scrlX, _scrnSizeX, 0, 40, copyWidth, copyHeight);
	} else {
		// partial screen update only. The screen coordinates probably won't fit to the
		// grid holding the informations on which blocks have to be updated.
		// as the grid will be X pixel higher and Y pixel more to the left, this can be cured
		// by first checking the top border, then the left column and then the remaining (aligned) part.
		uint8 *gridPos = _screenGrid + (scrlX / SCRNGRID_X) + (scrlY / SCRNGRID_Y) * _gridSizeX;
		uint8 *scrnBuf = _screenBuf + scrlY * _scrnSizeX + scrlX;
		uint8 diffX = (uint8)(scrlX % SCRNGRID_X);
		uint8 diffY = (uint8)(scrlY % SCRNGRID_Y);
		uint16 gridW = SCREEN_WIDTH / SCRNGRID_X;
		uint16 gridH = SCREEN_DEPTH / SCRNGRID_Y;
		if (diffY) {
			diffY = SCRNGRID_Y - diffY;
			uint16 cpWidth = 0;
			for (uint16 cntx = 0; cntx < gridW; cntx++) 
				if (gridPos[cntx]) {
					gridPos[cntx] >>= 1;
					cpWidth++;
				} else if (cpWidth) {
					int16 xPos = (cntx - cpWidth) * SCRNGRID_X - diffX;
					if (xPos < 0)
						xPos = 0;
					_system->copy_rect(scrnBuf + xPos, _scrnSizeX, xPos, 40, cpWidth * SCRNGRID_X, diffY);
					cpWidth = 0;
				}
			if (cpWidth) {
				int16 xPos = (gridW - cpWidth) * SCRNGRID_X - diffX;
				if (xPos < 0)
					xPos = 0;
				_system->copy_rect(scrnBuf + xPos, _scrnSizeX, xPos, 40, SCREEN_WIDTH - xPos, diffY);
			}
			scrlY += diffY;
		}
		// okay, y scrolling is compensated. check x now.
		gridPos = _screenGrid + (scrlX / SCRNGRID_X) + (scrlY / SCRNGRID_Y) * _gridSizeX;
		scrnBuf = _screenBuf + scrlY * _scrnSizeX + scrlX;
		if (diffX) {
			diffX = SCRNGRID_X - diffX;
			uint16 cpHeight = 0;
			for (uint16 cnty = 0; cnty < gridH; cnty++) {
				if (*gridPos) {
					*gridPos >>= 1;
					cpHeight++;
				} else if (cpHeight) {
					uint16 yPos = (cnty - cpHeight) * SCRNGRID_Y;
					_system->copy_rect(scrnBuf + yPos * _scrnSizeX, _scrnSizeX, 0, yPos + diffY + 40, diffX, cpHeight * SCRNGRID_Y);
					cpHeight = 0;
				}
				gridPos += _gridSizeX;
			}
			if (cpHeight) {
				uint16 yPos = (gridH - cpHeight) * SCRNGRID_Y;
				_system->copy_rect(scrnBuf + yPos * _scrnSizeX, _scrnSizeX, 0, yPos + diffY + 40, diffX, SCREEN_DEPTH - (yPos + diffY));
			}
			scrlX += diffX;
		}
		// x scroll is compensated, too. check the rest of the screen, now.
		scrnBuf = _screenBuf + scrlY * _scrnSizeX + scrlX;
		gridPos = _screenGrid + (scrlX / SCRNGRID_X) + (scrlY / SCRNGRID_Y) * _gridSizeX;
		for (uint16 cnty = 0; cnty < gridH; cnty++) {
			uint16 cpWidth = 0;
			uint16 cpHeight = SCRNGRID_Y;
			if (cnty == gridH - 1)
				cpHeight = SCRNGRID_Y - diffY;
			for (uint16 cntx = 0; cntx < gridW; cntx++)
				if (gridPos[cntx]) {
					gridPos[cntx] >>= 1;
					cpWidth++;
				} else if (cpWidth) {
					_system->copy_rect(scrnBuf + (cntx - cpWidth) * SCRNGRID_X, _scrnSizeX, (cntx - cpWidth) * SCRNGRID_X + diffX, cnty * SCRNGRID_Y + diffY + 40, cpWidth * SCRNGRID_X, cpHeight);
					cpWidth = 0;
				}
			if (cpWidth) {
				uint16 xPos = (gridW - cpWidth) * SCRNGRID_X;
				_system->copy_rect(scrnBuf + xPos, _scrnSizeX, xPos + diffX, cnty * SCRNGRID_Y + diffY + 40, SCREEN_WIDTH - (xPos + diffX), cpHeight);
			}
			gridPos += _gridSizeX;
			scrnBuf += _scrnSizeX * SCRNGRID_Y;
		}
	}
	_system->updateScreen();
}

void Screen::newScreen(uint32 screen) {
	uint8 cnt;
	// set sizes and scrolling, initialize/load screengrid, force screen refresh
	_currentScreen = screen;
	_scrnSizeX = _roomDefTable[screen].sizeX;
	_scrnSizeY = _roomDefTable[screen].sizeY;
	_gridSizeX = _scrnSizeX / SCRNGRID_X;
	_gridSizeY = _scrnSizeY / SCRNGRID_Y;
	if ((_scrnSizeX % SCRNGRID_X) || (_scrnSizeY % SCRNGRID_Y))
		error("Illegal screensize: %d: %d/%d", screen, _scrnSizeX, _scrnSizeY);
	if ((_scrnSizeX > SCREEN_WIDTH) || (_scrnSizeY > SCREEN_DEPTH)) {
		Logic::_scriptVars[SCROLL_FLAG] = 2;
		Logic::_scriptVars[MAX_SCROLL_OFFSET_X] = _scrnSizeX - SCREEN_WIDTH;
		Logic::_scriptVars[MAX_SCROLL_OFFSET_Y] = _scrnSizeY - SCREEN_DEPTH;
	} else {
		Logic::_scriptVars[SCROLL_FLAG] = 0;
		Logic::_scriptVars[MAX_SCROLL_OFFSET_X] = 0;
		Logic::_scriptVars[MAX_SCROLL_OFFSET_Y] = 0;
		Logic::_scriptVars[SCROLL_OFFSET_X] = 0;
		Logic::_scriptVars[SCROLL_OFFSET_Y] = 0;
	}
	if (_screenBuf)
		free(_screenBuf);
	if (_screenGrid)
		free(_screenGrid);
	_screenBuf = (uint8*)malloc(_scrnSizeX * _scrnSizeY);
	_screenGrid = (uint8*)malloc(_gridSizeX * _gridSizeY);
	memset(_screenGrid, 0, _gridSizeX * _gridSizeY);
	for (cnt = 0; cnt < _roomDefTable[_currentScreen].totalLayers; cnt++) {
		// open and lock all resources, will be closed in quitScreen()
		_layerBlocks[cnt] = (uint8*)_resMan->openFetchRes(_roomDefTable[_currentScreen].layers[cnt]);
		if (cnt > 0)
			_layerBlocks[cnt] += sizeof(Header);
	}
	for (cnt = 0; cnt < _roomDefTable[_currentScreen].totalLayers - 1; cnt++) {
		// there's no grid for the background layer, so it's totalLayers - 1
		_layerGrid[cnt] = (uint16*)_resMan->openFetchRes(_roomDefTable[_currentScreen].grids[cnt]);
 		_layerGrid[cnt] += 14;
	}
	_parallax[0] = _parallax[1] = NULL;
	if (_roomDefTable[_currentScreen].parallax[0])
		_parallax[0] = (uint8*)_resMan->openFetchRes(_roomDefTable[_currentScreen].parallax[0]);
	if (_roomDefTable[_currentScreen].parallax[1])
		_parallax[1] = (uint8*)_resMan->openFetchRes(_roomDefTable[_currentScreen].parallax[1]);

	fnSetPalette(0, 184, _roomDefTable[_currentScreen].palettes[0], SwordEngine::_systemVars.wantFade);
	fnSetPalette(184, 72, _roomDefTable[_currentScreen].palettes[1], SwordEngine::_systemVars.wantFade);
	_fullRefresh = true;
}

void Screen::quitScreen(void) {
	uint8 cnt;
	for (cnt = 0; cnt < _roomDefTable[_currentScreen].totalLayers; cnt++)
		_resMan->resClose(_roomDefTable[_currentScreen].layers[cnt]);
	for (cnt = 0; cnt < _roomDefTable[_currentScreen].totalLayers - 1; cnt++)
		_resMan->resClose(_roomDefTable[_currentScreen].grids[cnt]);
	if (_roomDefTable[_currentScreen].parallax[0])
		_resMan->resClose(_roomDefTable[_currentScreen].parallax[0]);
	if (_roomDefTable[_currentScreen].parallax[1])
		_resMan->resClose(_roomDefTable[_currentScreen].parallax[1]);
}

void Screen::draw(void) {
	uint8 cnt;
	if (_currentScreen == 54) {
		// rm54 has a BACKGROUND parallax layer in parallax[0]
		if (_parallax[0])
			renderParallax(_parallax[0]);
		uint8 *src = _layerBlocks[0];
		uint8 *dest = _screenBuf;
		for (uint16 cnty = 0; cnty < _scrnSizeY; cnty++)
			for (uint16 cntx = 0; cntx < _scrnSizeX; cntx++) {
				if (*src)
					*dest = *src;
				dest++;
				src++;
			}
	} else
		memcpy(_screenBuf, _layerBlocks[0], _scrnSizeX * _scrnSizeY);

	for (cnt = 0; cnt < _backLength; cnt++)
		processImage(_backList[cnt]);

	for (cnt = 0; cnt < _sortLength - 1; cnt++)
		for (uint8 sCnt = 0; sCnt < _sortLength - 1; sCnt++)
			if (_sortList[sCnt].y > _sortList[sCnt + 1].y) {
				SWAP(_sortList[sCnt], _sortList[sCnt + 1]);
			}
	for (cnt = 0; cnt < _sortLength; cnt++)
		processImage(_sortList[cnt].id);

	if ((_currentScreen != 54) && _parallax[0])
		renderParallax(_parallax[0]); // screens other than 54 have FOREGROUND parallax layer in parallax[0]
	if (_parallax[1])
		renderParallax(_parallax[1]);

	for (cnt = 0; cnt < _foreLength; cnt++)
		processImage(_foreList[cnt]);

	_backLength = _sortLength = _foreLength = 0;
}

void Screen::processImage(uint32 id) {
	Object *compact;
	FrameHeader *frameHead;
	int scale;

	compact = _objMan->fetchObject(id);
	if (compact->o_type == TYPE_TEXT)
		frameHead = _textMan->giveSpriteData((uint8)compact->o_target);
	else
		frameHead = _resMan->fetchFrame(_resMan->openFetchRes(compact->o_resource), compact->o_frame);
	
	uint8 *sprData = ((uint8*)frameHead) + sizeof(FrameHeader);

	uint16 spriteX = compact->o_anim_x;
	uint16 spriteY = compact->o_anim_y;
	if (compact->o_status & STAT_SHRINK) {
		scale = (compact->o_scale_a * compact->o_ycoord + compact->o_scale_b) / 256;
		spriteX += ((int16)FROM_LE_16(frameHead->offsetX) * scale) / 256;
		spriteY += ((int16)FROM_LE_16(frameHead->offsetY) * scale) / 256;
	} else {
		scale = 256;
		spriteX += (int16)FROM_LE_16(frameHead->offsetX);
		spriteY += (int16)FROM_LE_16(frameHead->offsetY);
	}
	if (scale > 512)
		debug(1, "compact %d is oversized: scale = %d", id, scale);

	uint8 *tonyBuf = NULL;
	if (frameHead->runTimeComp[3] == '7') { // RLE7 encoded?
		decompressRLE7(sprData, FROM_LE_32(frameHead->compSize), _rleBuffer);
		sprData = _rleBuffer;
	} else if (frameHead->runTimeComp[3] == '0') { // RLE0 encoded?
		decompressRLE0(sprData, FROM_LE_32(frameHead->compSize), _rleBuffer);
		sprData = _rleBuffer;
	} else if (frameHead->runTimeComp[1] == 'I') { // new type
		tonyBuf = (uint8*)malloc(FROM_LE_16(frameHead->width) * FROM_LE_16(frameHead->height));
		decompressTony(sprData, FROM_LE_32(frameHead->compSize), tonyBuf);
		sprData = tonyBuf;
	}

	uint16 sprSizeX, sprSizeY;
	if (compact->o_status & STAT_SHRINK) {
		sprSizeX = (scale * FROM_LE_16(frameHead->width)) / 256;
		sprSizeY = (scale * FROM_LE_16(frameHead->height)) / 256;
		fastShrink(sprData, FROM_LE_16(frameHead->width), FROM_LE_16(frameHead->height), scale, _shrinkBuffer);
		sprData = _shrinkBuffer;
	} else {
		sprSizeX = FROM_LE_16(frameHead->width);
		sprSizeY = FROM_LE_16(frameHead->height);
	}
	if (!(compact->o_status & STAT_OVERRIDE)) {
		//mouse size linked to exact size & coordinates of sprite box - shrink friendly
		if ((frameHead->offsetX) || (frameHead->offsetY)) {
			//for megas the mouse area is reduced to account for sprite not
			//filling the box size is reduced to 1/2 width, 4/5 height
			compact->o_mouse_x1 = spriteX + sprSizeX / 4;
			compact->o_mouse_x2 = spriteX + (3 * sprSizeX) / 4;
			compact->o_mouse_y1 = spriteY + sprSizeY / 10;
			compact->o_mouse_y2 = spriteY + (9 * sprSizeY) / 10;
		} else {
			compact->o_mouse_x1 = spriteX;
			compact->o_mouse_x2 = spriteX + sprSizeX;
			compact->o_mouse_y1 = spriteY;
			compact->o_mouse_y2 = spriteY + sprSizeY;
		}
	}
	uint16 sprPitch = sprSizeX;
	uint16 incr;
	spriteClipAndSet(&spriteX, &spriteY, &sprSizeX, &sprSizeY, &incr);
	if ((sprSizeX > 0) && (sprSizeY > 0)) {
		drawSprite(sprData + incr, spriteX, spriteY, sprSizeX, sprSizeY, sprPitch);
		if (!(compact->o_status&STAT_FORE))
			verticalMask(spriteX, spriteY, sprSizeX, sprSizeY);
	}
	if (compact->o_type != TYPE_TEXT)
		_resMan->resClose(compact->o_resource);
	if (tonyBuf)
		free(tonyBuf);
}

void Screen::verticalMask(uint16 x, uint16 y, uint16 bWidth, uint16 bHeight) {
	if (_roomDefTable[_currentScreen].totalLayers <= 1)
		return;

	bWidth = (bWidth + (x & (SCRNGRID_X - 1)) + (SCRNGRID_X - 1)) / SCRNGRID_X;
	bHeight = (bHeight + (y & (SCRNGRID_Y - 1)) + (SCRNGRID_Y - 1)) / SCRNGRID_Y;

	x /= SCRNGRID_X;
	y /= SCRNGRID_Y;
	if (x + bWidth > _gridSizeX)
		bWidth = _gridSizeX - x;
	if (y + bHeight > _gridSizeY)
		bHeight = _gridSizeY - y;

	uint16 gridY = y + SCREEN_TOP_EDGE / SCRNGRID_Y; // imaginary screen on top
	gridY += bHeight - 1; // we start from the bottom edge
	uint16 gridX = x + SCREEN_LEFT_EDGE / SCRNGRID_X; // imaginary screen left
	uint16 lGridSizeX = _gridSizeX + 2 * (SCREEN_LEFT_EDGE / SCRNGRID_X); // width of the grid for the imaginary screen

	for (uint16 blkx = 0; blkx < bWidth; blkx++) {
		uint16 level = 0;
		while ((level < _roomDefTable[_currentScreen].totalLayers - 1) && 
			(!_layerGrid[level][gridX + blkx + gridY * lGridSizeX]))
			level++;
		if (level < _roomDefTable[_currentScreen].totalLayers - 1) {
			uint16 *grid = _layerGrid[level] + gridX + blkx + gridY * lGridSizeX;
			for (int16 blky = bHeight - 1; blky >= 0; blky--) {
				if (*grid) {
					uint8 *blkData = _layerBlocks[level + 1] + (READ_LE_UINT16(grid) - 1) * 128;
					blitBlockClear(x + blkx, y + blky, blkData);
				} else 
					break;
				grid -= lGridSizeX;
			}
		}
	}
}

void Screen::blitBlockClear(uint16 x, uint16 y, uint8 *data) {
	uint8 *dest = _screenBuf + (y * SCRNGRID_Y) * _scrnSizeX + (x * SCRNGRID_X);
	for (uint8 cnty = 0; cnty < SCRNGRID_Y; cnty++) {
		for (uint8 cntx = 0; cntx < SCRNGRID_X; cntx++)
			if (data[cntx])
				dest[cntx] = data[cntx];
		data += SCRNGRID_X;
		dest += _scrnSizeX;
	}
}

void Screen::renderParallax(uint8 *data) {
	ParallaxHeader *header = (ParallaxHeader*)data;
	uint32 *lineIndexes = (uint32*)(data + sizeof(ParallaxHeader));
	assert((FROM_LE_16(header->sizeX) >= SCREEN_WIDTH) && (FROM_LE_16(header->sizeY) >= SCREEN_DEPTH));

	//double scrlfx, scrlfy;
	uint16 paraScrlX, paraScrlY;
	uint16 scrnScrlX, scrnScrlY;
	uint16 scrnWidth, scrnHeight;

	// we have to render more than the visible screen part for displaying scroll frames
	scrnScrlX = MIN((uint32)_oldScrollX, Logic::_scriptVars[SCROLL_OFFSET_X]);
	scrnWidth = SCREEN_WIDTH + ABS((int32)_oldScrollX - (int32)Logic::_scriptVars[SCROLL_OFFSET_X]);
	scrnScrlY = MIN((uint32)_oldScrollY, Logic::_scriptVars[SCROLL_OFFSET_Y]);
	scrnHeight = SCREEN_DEPTH + ABS((int32)_oldScrollY - (int32)Logic::_scriptVars[SCROLL_OFFSET_Y]);

	if (_scrnSizeX != SCREEN_WIDTH) {
		double scrlfx = (FROM_LE_16(header->sizeX) - SCREEN_WIDTH) / ((double)(_scrnSizeX - SCREEN_WIDTH));
		paraScrlX = (uint16)(scrnScrlX * scrlfx);
	} else
		paraScrlX = 0;

	if (_scrnSizeY != SCREEN_DEPTH) {
		double scrlfy = (FROM_LE_16(header->sizeY) - SCREEN_DEPTH) / ((double)(_scrnSizeY - SCREEN_DEPTH));
		paraScrlY = (uint16)(scrnScrlY * scrlfy);
	} else
		paraScrlY = 0;
	
	for (uint16 cnty = 0; cnty < scrnHeight; cnty++) {
		uint8 *src = data + READ_LE_UINT32(lineIndexes + cnty + paraScrlY);
		uint8 *dest = _screenBuf + scrnScrlX + (cnty + scrnScrlY) * _scrnSizeX;
		uint16 remain = paraScrlX;
		uint16 xPos = 0;
		bool copyFirst = false;
		while (remain) { // skip past the first part of the parallax to get to the right scrolling position
			uint8 doSkip = *src++;
			if (doSkip <= remain)
				remain -= doSkip;
			else {
				xPos = doSkip - remain;
				dest += xPos;
				remain = 0;
			}
			if (remain) {
				uint8 doCopy = *src++;
				if (doCopy <= remain) {
					remain -= doCopy;
					src += doCopy;
				} else {
					uint16 remCopy = doCopy - remain;
					memcpy(dest, src + remain, remCopy);
					dest += remCopy;
					src += doCopy;
					xPos = remCopy;
					remain = 0;
				}
			} else
				copyFirst = true;
		}
		while (xPos < scrnWidth) {
			if (!copyFirst) {
				if (uint8 skip = *src++) {
					dest += skip;
					xPos += skip;
				}
			} else
				copyFirst = false;
			if (xPos < scrnWidth) {
				if (uint8 doCopy = *src++) {
					if (xPos + doCopy > scrnWidth)
						doCopy = scrnWidth - xPos;
					memcpy(dest, src, doCopy);
					dest += doCopy;
					xPos += doCopy;
					src += doCopy;
				}
			}
		}
	}
}

void Screen::drawSprite(uint8 *sprData, uint16 sprX, uint16 sprY, uint16 sprWidth, uint16 sprHeight, uint16 sprPitch) {
	uint8 *dest = _screenBuf + (sprY * _scrnSizeX) + sprX;
	for (uint16 cnty = 0; cnty < sprHeight; cnty++) {
		for (uint16 cntx = 0; cntx < sprWidth; cntx++)
			if (sprData[cntx])
				dest[cntx] = sprData[cntx];
		sprData += sprPitch;
		dest += _scrnSizeX;
	}
}

// nearest neighbor filter:
void Screen::fastShrink(uint8 *src, uint32 width, uint32 height, uint32 scale, uint8 *dest) {
	uint32 resHeight = (height * scale) >> 8;
	uint32 resWidth = (width * scale) >> 8;
	uint32 step = 0x10000 / scale;
	uint8 columnTab[160];
	uint32 res = step >> 1;
	for (uint16 cnt = 0; cnt < resWidth; cnt++) {
		columnTab[cnt] = (uint8)(res >> 8);
		res += step;
	}

	uint32 newRow = step >> 1;
	uint32 oldRow = 0;

	uint8 *destPos = dest;
	uint16 lnCnt;
	for (lnCnt = 0; lnCnt < resHeight; lnCnt++) {
		while (oldRow < (newRow >> 8)) {
			oldRow++;
			src += width;
		}
		for (uint16 colCnt = 0; colCnt < resWidth; colCnt++) {
			*destPos++ = src[columnTab[colCnt]];
		}
		newRow += step;
	}
	// scaled, now stipple shadows if there are any
	for (lnCnt = 0; lnCnt < resHeight; lnCnt++) {
		uint16 xCnt = lnCnt & 1;
		destPos = dest + lnCnt * resWidth + (lnCnt & 1);
		while (xCnt < resWidth) {
			if (*destPos == 200)
				*destPos = 0;
			destPos += 2;
			xCnt += 2;
		}
	}
}

void Screen::addToGraphicList(uint8 listId, uint32 objId) {
	if (listId == 0) {
		_foreList[_foreLength++] = objId;
		if (_foreLength > MAX_FORE)
			error("foreList exceeded!");
	}
	if (listId == 1) {
		Object *cpt = _objMan->fetchObject(objId);
		_sortList[_sortLength].id = objId;
		_sortList[_sortLength].y = cpt->o_anim_y; // gives feet coords if boxed mega, otherwise top of sprite box
		if (!(cpt->o_status & STAT_SHRINK)) {     // not a boxed mega using shrinking
			Header *frameRaw = (Header*)_resMan->openFetchRes(cpt->o_resource);
			FrameHeader *frameHead = _resMan->fetchFrame(frameRaw, cpt->o_frame);
			_sortList[_sortLength].y += FROM_LE_16(frameHead->height) - 1; // now pointing to base of sprite
			_resMan->resClose(cpt->o_resource);
		}
		_sortLength++;
		if (_sortLength > MAX_SORT)
			error("sortList exceeded!");
	}
	if (listId == 2) {
		_backList[_backLength++] = objId;
		if (_backLength > MAX_BACK)
			error("backList exceeded!");
	}
}

void Screen::decompressTony(uint8 *src, uint32 compSize, uint8 *dest) {
	uint8 *endOfData = src + compSize;
	while (src < endOfData) {
		uint8 numFlat = *src++;
		if (numFlat) {
			memset(dest, *src, numFlat);
			src++;
			dest += numFlat;
		}
		if (src < endOfData) {
			uint8 numNoFlat = *src++;
			memcpy(dest, src, numNoFlat);
			src += numNoFlat;
			dest += numNoFlat;
		}
	}
}

void Screen::decompressRLE7(uint8 *src, uint32 compSize, uint8 *dest) {
	uint8 *compBufEnd = src + compSize;
	while (src < compBufEnd) {
		uint8 code = *src++;
		if ((code > 127) || (code == 0))
			*dest++ = code;
		else {
			code++;
			memset(dest, *src++, code);
			dest += code;
		}
	}
}

void Screen::decompressRLE0(uint8 *src, uint32 compSize, uint8 *dest) {
	uint8 *srcBufEnd = src + compSize;
	while (src < srcBufEnd) {
		uint8 color = *src++;
		if (color) {
			*dest++ = color;
		} else {
			uint8 skip = *src++;
			memset(dest, 0, skip);
			dest += skip;
		}
	}
}

void Screen::fadePalette(void) {
	if (_fadingStep == 16)
		memcpy(_currentPalette, _targetPalette, 256 * 4);
	else if ((_fadingStep == 1) && (_fadingDirection == FADE_DOWN)) {
		memset(_currentPalette, 0, 4 * 256);
	} else
		for (uint16 cnt = 0; cnt < 256 * 4; cnt++)
			_currentPalette[cnt] = (_targetPalette[cnt] * _fadingStep) >> 4;

	_fadingStep += _fadingDirection;
	if (_fadingStep == 17) {
		_fadingStep = 0;
		_isBlack = false;
	} else if (_fadingStep == 0)
		_isBlack = true;
}

void Screen::fnSetParallax(uint32 screen, uint32 resId) {
	if ((screen == _currentScreen) && (resId != _roomDefTable[screen].parallax[0]))
		warning("fnSetParallax: setting parallax for current room!!");
	_roomDefTable[screen].parallax[0] = resId;
}

void Screen::spriteClipAndSet(uint16 *pSprX, uint16 *pSprY, uint16 *pSprWidth, uint16 *pSprHeight, uint16 *incr) {
	int16 sprX = *pSprX - SCREEN_LEFT_EDGE;
	int16 sprY = *pSprY - SCREEN_TOP_EDGE;
	int16 sprW = *pSprWidth;
	int16 sprH = *pSprHeight;
	
	if (sprY < 0) {
		*incr = (uint16)((-sprY) * sprW);
		sprH += sprY;
		sprY = 0;
	} else
		*incr = 0;
	if (sprX < 0) {
		*incr -= sprX;
		sprW += sprX;
		sprX = 0;
	}
	
	if (sprY + sprH > _scrnSizeY)
		sprH = _scrnSizeY - sprY;
	if (sprX + sprW > _scrnSizeX)
		sprW = _scrnSizeX - sprX;

	if (sprH < 0)
		*pSprHeight = 0;
	else
		*pSprHeight = (uint16)sprH;
	if (sprW < 0)
		*pSprWidth = 0;
	else
		*pSprWidth = (uint16)sprW;
	*pSprX = (uint16)sprX;
	*pSprY = (uint16)sprY;

	if (*pSprWidth && *pSprHeight) {
		// sprite will be drawn, so mark it in the grid buffer (we don't need to keep
		// track of changed blocks if we're going to do a full refresh, anyways.
		uint16 gridH = (*pSprHeight + (sprY & (SCRNGRID_Y - 1)) + (SCRNGRID_Y - 1)) / SCRNGRID_Y;
		uint16 gridW = (*pSprWidth +  (sprX & (SCRNGRID_X - 1)) + (SCRNGRID_X - 1)) / SCRNGRID_X;
		uint16 gridX = sprX / SCRNGRID_X;
		uint16 gridY = sprY / SCRNGRID_Y;
		uint8 *gridBuf = _screenGrid + gridX + gridY * _gridSizeX;
		if (gridX + gridW > _gridSizeX)
			gridW = _gridSizeX - gridX;
		if (gridY + gridH > _gridSizeY)
			gridH = _gridSizeY - gridY;

		for (uint16 cnty = 0; cnty < gridH; cnty++) {
			for (uint16 cntx = 0; cntx < gridW; cntx++)
				gridBuf[cntx] = 2;
			gridBuf += _gridSizeX;
		}
	}
}

void Screen::fnFlash(uint8 color) {
	warning("stub: Screen::fnFlash(%d)", color);
}

// ------------------- Menu screen interface ---------------------------

void Screen::showFrame(uint16 x, uint16 y, uint32 resId, uint32 frameNo, const byte *fadeMask, int8 fadeStatus) {
	uint8 frame[40 * 40];
	int i, j;

	memset(frame, 199, sizeof(frame));	// Dark gray background

	if (resId != 0xffffffff) {
		FrameHeader *frameHead = _resMan->fetchFrame(_resMan->openFetchRes(resId), frameNo);
		uint8 *frameData = ((uint8*)frameHead) + sizeof(FrameHeader);

		for (i = 0; i < FROM_LE_16(frameHead->height); i++) {
			for (j = 0; j < FROM_LE_16(frameHead->height); j++) {
				frame[(i + 4) * 40 + j + 2] = frameData[i * FROM_LE_16(frameHead->width) + j];
			}
		}

		_resMan->resClose(resId);
	}

	if (fadeMask) {
		for (i = 0; i < 40; i++) {
			for (j = 0; j < 40; j++) {
				if (fadeMask[((i % 8) * 8) + (j % 8)] >= fadeStatus)
					frame[i * 40 + j] = 0;
			}
		}
	}

	_system->copy_rect(frame, 40, x, y, 40, 40);
}

// ------------------- router debugging code --------------------------------

void Screen::vline(uint16 x, uint16 y1, uint16 y2) {
	for (uint16 cnty = y1; cnty <= y2; cnty++)
		_screenBuf[x + _scrnSizeX * cnty] = 0;
}

void Screen::hline(uint16 x1, uint16 x2, uint16 y) {
	for (uint16 cntx = x1; cntx <= x2; cntx++)
		_screenBuf[y * _scrnSizeX + cntx] = 0;
}

void Screen::bsubline_1(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int x, y, ddx, ddy, e;
	ddx = ABS(x2 - x1);
	ddy = ABS(y2 - y1) << 1;
	e = ddx - ddy;
	ddx <<= 1;

	if (x1 > x2) {
		uint16 tmp;
		tmp = x1; x1 = x2; x2 = tmp;
		tmp = y1; y1 = y2; y2 = tmp;
	}

	for (x = x1, y = y1; x <= x2; x++) {
		_screenBuf[y * _scrnSizeX + x] = 0;
		if (e < 0) {
			y++;
			e += ddx - ddy;
		} else {
			e -= ddy;
		}
	}
}

void Screen::bsubline_2(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int x, y, ddx, ddy, e;
	ddx = ABS(x2 - x1) << 1;
	ddy = ABS(y2 - y1);
	e = ddy - ddx;
	ddy <<= 1;

	if (y1 > y2) {
		uint16 tmp;
		tmp = x1; x1 = x2; x2 = tmp;
		tmp = y1; y1 = y2; y2 = tmp;
	}

	for (y = y1, x = x1; y <= y2; y++) {
		_screenBuf[y * _scrnSizeX + x] = 0;
		if (e < 0) {
			x++;
			e += ddy - ddx;
		} else {
			e -= ddx;
		}
	}
}

void Screen::bsubline_3(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int x, y, ddx, ddy, e;
	ddx = ABS(x1 - x2) << 1;
	ddy = ABS(y2 - y1);
	e = ddy - ddx;
	ddy <<= 1;

	if (y1 > y2) {
		uint16 tmp;
		tmp = x1; x1 = x2; x2 = tmp;
		tmp = y1; y1 = y2; y2 = tmp;
	}

	for (y = y1, x = x1; y <= y2; y++) {
		_screenBuf[y * _scrnSizeX + x] = 0;
		if (e < 0) {
			x--;
			e += ddy - ddx;
		} else {
			e -= ddx;
		}
	}
}

void Screen::bsubline_4(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int x, y, ddx, ddy, e;
	ddy = ABS(y2 - y1) << 1;
	ddx = ABS(x1 - x2);
	e = ddx - ddy;
	ddx <<= 1;

	if (x1 > x2) {
		uint16 tmp;
		tmp = x1; x1 = x2; x2 = tmp;
		tmp = y1; y1 = y2; y2 = tmp;
	}

	for (x = x1, y = y1; x <= x2; x++) {
		_screenBuf[y * _scrnSizeX + x] = 0;
		if (e < 0) {
			y--;
			e += ddx - ddy;
		} else {
			e -= ddy;
		}
	}
}

void Screen::drawLine(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	if ((x1 == x2) && (y1 == y2)) {
		_screenBuf[x1 + y1 * _scrnSizeX] = 0;
	}
	if (x1 == x2) {
		vline(x1, MIN(y1, y2), MAX(y1, y2));
		return;
	}

	if (y1 == y2) {
		hline(MIN(x1, x2), MAX(x1, x2), y1);
		return;
	}

	float k = float(y2 - y1) / float(x2 - x1);

	if ((k >= 0) && (k <= 1)) {
		bsubline_1(x1, y1, x2, y2);
	} else if (k > 1) {
		bsubline_2(x1, y1, x2, y2);
	} else if ((k < 0) && (k >= -1)) {
		bsubline_4(x1, y1, x2, y2);
	} else {
		bsubline_3(x1, y1, x2, y2);
	}
}

#ifdef BACKEND_8BIT
void Screen::plotYUV(byte *lut, int width, int height, byte *const *dat) {

	byte * buf = (uint8*)malloc(width * height);

	int x, y;

	int ypos = 0;
	int cpos = 0;
	int linepos = 0;

	for (y = 0; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			int i = ((((dat[2][cpos] + ROUNDADD) >> SHIFT) * (BITDEPTH+1)) + ((dat[1][cpos] + ROUNDADD)>>SHIFT)) * (BITDEPTH+1);
			cpos++;

			buf[linepos          ] = lut[i + ((dat[0][        ypos  ] + ROUNDADD) >> SHIFT)];
			buf[width + linepos++] = lut[i + ((dat[0][width + ypos++] + ROUNDADD) >> SHIFT)];
			buf[linepos          ] = lut[i + ((dat[0][        ypos  ] + ROUNDADD) >> SHIFT)];
			buf[width + linepos++] = lut[i + ((dat[0][width + ypos++] + ROUNDADD) >> SHIFT)];
		}
		linepos += (2 * width - width);
		ypos += width;
	}

	_system->copy_rect(buf, width, (640-width)/2, (480-height)/2, width, height);
	_system->updateScreen();

	free(buf);

}
#endif



} // End of namespace Sword1
