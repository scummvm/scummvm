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

#include "common/cosinetables.h"
#include "common/sinetables.h"
#include "common/random.h"
#include "common/memstream.h"
#include "graphics/cursor.h"
#include "graphics/cursorman.h"

#include "hdb/hdb.h"
#include "hdb/ai.h"
#include "hdb/file-manager.h"
#include "hdb/gfx.h"
#include "hdb/input.h"
#include "hdb/mpc.h"
#include "hdb/sound.h"

namespace HDB {

Gfx::Gfx() {
	_tLookupArray = nullptr;
	_starsInfo.active = false;
	_gfxCache = new Common::Array<GfxCache *>;
	_globalSurface.create(g_hdb->_screenWidth, g_hdb->_screenHeight, g_hdb->_format);
	_pointerDisplayable = 1;
	_sines = new Common::SineTable(360);
	_cosines = new Common::CosineTable(360);
	_systemInit = false;

	_numTiles = 0;

	memset(&_fadeInfo, 0, sizeof(_fadeInfo));
	memset(&_snowInfo, 0, sizeof(_snowInfo));
	memset(&_skyTiles, 0, sizeof(_skyTiles));

	_tileSkyStars = 0;
	_tileSkyStarsLeft = 0;
	_tileSkyClouds = 0;
	for (int i = 0; i < 4; ++i) {
		_starField[i] = nullptr;
		_mousePointer[2 * i] = nullptr;
		_mousePointer[(2 * i) + 1] = nullptr;
	}

	_snowflake = nullptr;
	_skyClouds = nullptr;
	_starsInfo.gfx[0] = nullptr;
	_starsInfo.gfx[1] = nullptr;
	_starsInfo.timer = 0;
	_starsInfo.anim = 0;
	_starsInfo.radius = 0;
	_starsInfo.angleSpeed = 0;
	_starsInfo.totalTime = 0;
	_cursorX = 0;
	_cursorY = 0;
	_showCursor = false;
	_fontHeader.type = 0;
	_fontHeader.numChars = 0;
	_fontHeader.height = 0;
	_fontHeader.kerning = 0;
	_fontHeader.leading = 0;
	_fontGfx = 0;
	_eLeft = 0;
	_eRight = 0;
	_eTop = 0;
	_eBottom = 0;
	_currentSky = 0;
	for (int i = 0; i < kNum3DStars; ++i) {
		_stars3D[i].x = 0;
		_stars3D[i].y = 0;
		_stars3D[i].speed = 0;
		_stars3D[i].color = 0;
		_stars3DSlow[i].x = 0;
		_stars3DSlow[i].y = 0;
		_stars3DSlow[i].speed = 0;
		_stars3DSlow[i].color = 0;
	}
}

Gfx::~Gfx() {
	for (uint i = 0; i < _gfxCache->size(); i++) {
		GfxCache *cache = _gfxCache->operator[](i);
		if (cache->status)
			delete cache->picGfx;
		else
			delete cache->tileGfx;
		delete cache;
	}
	delete _gfxCache;
	for (uint i = 0; i < _charInfoBlocks.size(); i++)
		delete _charInfoBlocks[i];
	delete _sines;
	delete _cosines;
	for (int i = 0; i < _fontHeader.numChars; i++)
		_fontSurfaces[i].free();
	_globalSurface.free();
	for (int i = 0; i < _numTiles; i++) {
		delete _tLookupArray[i].tData;
		_tLookupArray[i].tData = nullptr;
	}
	delete[] _tLookupArray;
	for (int i = 0; i < 8; i++)
		delete _mousePointer[i];
	for (int i = 0; i < 4; i++)
		delete _starField[i];
	delete _snowflake;
	delete _skyClouds;
}

static const byte cursorPalette[16 * 3] = {
	0x00, 0x01, 0x00,
	0x09, 0x4E, 0xA2,
	0x42, 0x3D, 0xBE,
	0xFF, 0x00, 0xFD,
	0x00, 0x57, 0xB8,
	0x0C, 0x66, 0xCB,
	0x0D, 0x65, 0xED,
	0x01, 0x6A, 0xDE,
	0x34, 0x59, 0xF1,
	0x26, 0x5E, 0xEE,
	0x00, 0x77, 0xFA,
	0x23, 0x87, 0xFE,
	0x47, 0x94, 0xFA,
	0x69, 0xAF, 0xFC,
	0x9F, 0xCD, 0xFE,
	0xDE, 0xEE, 0xFD
};

static const byte cursorData[] = {
 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  0,  0,  0,  0,  0,  0,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  0, 11, 11, 11, 10, 10, 10, 10,  6,  0,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  0, 13, 13, 12, 12, 11, 10, 10, 10, 10, 10, 10,  8,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  0, 14, 13, 13, 13, 12, 11,  7,  7, 10, 10, 10, 10, 10,  6,  0,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  0, 14, 14, 14, 13, 12,  0,  0,  0,  0,  0,  0,  7, 10, 10, 10, 10,  0,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  0, 14, 15, 14, 13,  0,  0,  3,  3,  3,  3,  3,  3,  0,  0,  7, 10, 10,  6,  0,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  0, 13, 14, 14, 13,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  7,  7,  7,  8,  0,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  0, 13, 14, 13,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  7,  7,  7,  0,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  0, 12, 13, 13,  5,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  7,  7,  7,  8,  0,  3,  3,  3,  3,
 3,  3,  3,  3,  0, 12, 13, 13,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  7,  5,  6,  0,  3,  3,  3,  3,
 3,  3,  3,  3,  0, 12, 12, 12,  0,  3,  3,  3,  3,  3,  3,  0,  0,  3,  3,  3,  3,  3,  3,  0,  5,  5,  5,  0,  3,  3,  3,  3,
 3,  3,  3,  3,  0, 11, 11,  5,  0,  3,  3,  3,  3,  3,  0, 15, 15,  0,  3,  3,  3,  3,  3,  0,  5,  4,  4,  0,  3,  3,  3,  3,
 3,  3,  3,  3,  0, 10, 11,  5,  0,  3,  3,  3,  3,  0, 15, 15, 13, 13,  0,  3,  3,  3,  3,  0,  7,  4,  4,  0,  3,  3,  3,  3,
 3,  3,  3,  3,  0, 10, 11,  5,  0,  3,  3,  3,  3,  0, 15, 13, 13,  7,  0,  3,  3,  3,  3,  0,  7,  4,  4,  0,  3,  3,  3,  3,
 3,  3,  3,  3,  0, 10, 10,  7,  0,  3,  3,  3,  3,  3,  0, 13,  7,  0,  3,  3,  3,  3,  3,  0,  5,  4,  4,  0,  3,  3,  3,  3,
 3,  3,  3,  3,  0, 10, 10,  7,  0,  3,  3,  3,  3,  3,  3,  0,  0,  3,  3,  3,  3,  3,  3,  0,  4,  4,  4,  0,  3,  3,  3,  3,
 3,  3,  3,  3,  0, 10, 10, 10,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  4,  4,  4,  0,  3,  3,  3,  3,
 3,  3,  3,  3,  0,  9, 10, 10,  7,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  5,  1,  1,  2,  0,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  0, 10, 10, 10,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  1,  1,  1,  0,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  0,  8, 10, 10,  7,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  1,  1,  1,  2,  0,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  0,  6, 10,  7,  7,  0,  0,  3,  3,  3,  3,  3,  3,  0,  0,  1,  1,  1,  4,  0,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  0, 10,  7,  7,  7,  7,  0,  0,  0,  0,  0,  0,  5,  1,  1,  1,  1,  0,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  0,  6,  7,  7,  7,  5,  5,  4,  4,  4,  4,  1,  1,  1,  4,  0,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  9,  7,  5,  5,  4,  4,  4,  4,  4,  1,  1,  2,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  0,  9,  7,  4,  4,  4,  4,  4,  2,  0,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  0,  0,  0,  0,  0,  0,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3
};


void Gfx::init() {
	// Set the default cursor pos & char clipping
	setCursor(0, 0);

	if (g_hdb->isPPC()) {
		CursorMan.replaceCursorPalette(cursorPalette, 0, 16);
		CursorMan.replaceCursor(cursorData, 32, 32, 16, 16, 3);
	}

	_eLeft = 0;
	_eRight = g_hdb->_screenWidth;
	_eTop = 0;
	_eBottom = g_hdb->_screenHeight;

	// Need two main memory screen-sized surfaces for screen fading
	_fadeBuffer1.create(g_hdb->_screenWidth, g_hdb->_screenHeight, g_hdb->_format);
	_fadeBuffer2.create(g_hdb->_screenWidth, g_hdb->_screenHeight, g_hdb->_format);

	// Load Game Font
	if (!loadFont(HDB_FONT))
		error("Gfx::init: Couldn't load fonts");

	// Read total number of tiles in game
	_numTiles = g_hdb->_fileMan->getCount("t32_", TYPE_TILE32);
	if (!_numTiles)
		error("Gfx::init: No tiles in game");

	// Setup Tile Lookup Array
	_tLookupArray = new TileLookup[_numTiles];
	Common::Array<const char *> *tileData = g_hdb->_fileMan->findFiles("t32_", TYPE_TILE32);

	assert((uint)_numTiles == tileData->size());

	int index = 0, skyIndex = 0;
	for (; index < _numTiles; index++) {
		_tLookupArray[index].filename = tileData->operator[](index);
		_tLookupArray[index].tData = nullptr;
		_tLookupArray[index].skyIndex = 0;
		_tLookupArray[index].animIndex = index;
		// Check if the loaded Tile is a Sky Tile
		if (strstr(tileData->operator[](index), "sky") && (skyIndex < kMaxSkies)) {
			_tLookupArray[index].skyIndex = skyIndex + 1;
			_skyTiles[skyIndex] = index;
			skyIndex++;
		}
	}

	delete tileData;

	// Add Animating Tile Info
	int found = -1;
	char search[32];
	Common::strlcpy(search, "anim_", 32);
	for (index = 0; index < _numTiles; index++) {
		// IF we have not found a start, look for it
		// ELSE IF we have found a start and are in the middle of an anim group
		// ELSE IF we're in an anim group and have just reached the end
		if (!strncmp(_tLookupArray[index].filename, search, strlen(search)) && found == -1) {
			found = index;
			memset(search, 0, sizeof(search));
			strncpy(search, _tLookupArray[index].filename, strlen(_tLookupArray[index].filename) - 2);
		} else if (!strncmp(_tLookupArray[index].filename, search, strlen(search)) && found >= 0)
			_tLookupArray[index - 1].animIndex = index;
		else if (strncmp(_tLookupArray[index].filename, search, strlen(search)) && found >= 0) {
			_tLookupArray[index - 1].animIndex = found;
			Common::strlcpy(search, "anim_", 32);
			found = -1;
			if (!strncmp(_tLookupArray[index].filename, search, strlen(search)))
				index--;
		}
	}

	// Init Sky Data
	_currentSky = 0;
	_tileSkyStars = getTileIndex(TILE_SKY_STARS);
	_tileSkyStarsLeft = getTileIndex(TILE_SKY_STARS_LEFT_SLOW);
	_tileSkyClouds = getTileIndex(TILE_SKY_CLOUDS); // Not completely sure about this filename.
	_skyClouds = nullptr;

	if (!g_hdb->isPPC()) {
		// Load Mouse Pointer and Display Cursor
		_mousePointer[0] = loadPic(PIC_MOUSE_CURSOR1);
		_mousePointer[1] = loadPic(PIC_MOUSE_CURSOR2);
		_mousePointer[2] = loadPic(PIC_MOUSE_CURSOR3);
		_mousePointer[3] = loadPic(PIC_MOUSE_CURSOR4);
		_mousePointer[4] = loadPic(PIC_MOUSE_CURSOR5);
		_mousePointer[5] = loadPic(PIC_MOUSE_CURSOR6);
		_mousePointer[6] = loadPic(PIC_MOUSE_CURSOR7);
		_mousePointer[7] = loadPic(PIC_MOUSE_CURSOR8);
		_showCursor = true;

		// Load all 4 levels of star colors
		_starField[0] = getPicture(PIC_STAR64);
		_starField[1] = getPicture(PIC_STAR128);
		_starField[2] = getPicture(PIC_STAR192);
		_starField[3] = getPicture(PIC_STAR256);
		_snowflake = getPicture(PIC_SNOWFLAKE);
	} else {
		for (int i = 0; i < 8; i++)
			_mousePointer[i] = nullptr;

		for (int i = 0; i < 4; i++)
			_starField[i] = nullptr;

		_snowflake = nullptr;

		_showCursor = false;
	}

	_systemInit = true;
}

void Gfx::save(Common::OutSaveFile *out) {
	out->writeSint32LE(_currentSky);

	out->writeByte(_fadeInfo.active);
	out->writeByte(_fadeInfo.stayFaded);
	out->writeByte(_fadeInfo.isBlack);
	out->writeSint32LE(_fadeInfo.speed);
	out->writeByte(_fadeInfo.isFadeIn);
	out->writeSint32LE(_fadeInfo.curStep);

	out->writeByte(_snowInfo.active);
	for (int i = 0; i < MAX_SNOW; i++)
		out->writeDoubleLE(_snowInfo.x[i]);
	for (int i = 0; i < MAX_SNOW; i++)
		out->writeDoubleLE(_snowInfo.y[i]);
	for (int i = 0; i < MAX_SNOW; i++)
		out->writeDoubleLE(_snowInfo.yv[i]);
	for (int i = 0; i < MAX_SNOW; i++)
		out->writeSint32LE(_snowInfo.xvindex[i]);
}

void Gfx::loadSaveFile(Common::InSaveFile *in) {
	_currentSky = in->readSint32LE();

	_fadeInfo.active = in->readByte();
	_fadeInfo.stayFaded = in->readByte();
	_fadeInfo.isBlack = in->readByte();
	_fadeInfo.speed = in->readSint32LE();
	_fadeInfo.isFadeIn = in->readByte();
	_fadeInfo.curStep = in->readSint32LE();

	_snowInfo.active = in->readByte();

	for (int i = 0; i < MAX_SNOW; i++)
		_snowInfo.x[i] = in->readDoubleLE();
	for (int i = 0; i < MAX_SNOW; i++)
		_snowInfo.y[i] = in->readDoubleLE();
	for (int i = 0; i < MAX_SNOW; i++)
		_snowInfo.yv[i] = in->readDoubleLE();
	for (int i = 0; i < MAX_SNOW; i++)
		_snowInfo.xvindex[i] = in->readSint32LE();

	setSky(_currentSky);

	if (!g_hdb->isPPC()) {
		turnOffSnow();
		if (_snowInfo.active)
			turnOnSnow();
	}
}

double Gfx::getSin(int index) {
	return _sines->at(index);
}
double Gfx::getCos(int index) {
	return _cosines->at(index);
}

void Gfx::fillScreen(uint32 color) {
	_globalSurface.fillRect(Common::Rect(g_hdb->_screenWidth, g_hdb->_screenHeight), color);
	g_system->fillScreen(color);
}

void Gfx::updateVideo() {
	updateFade();

	if (!g_hdb->_progressGfx)
		return;

	g_hdb->checkProgress();

	int left = g_hdb->_screenWidth / 2 - g_hdb->_progressGfx->_width / 2;

	Common::Rect clip(g_hdb->_progressGfx->getSurface()->getBounds());
	clip.moveTo(left, g_hdb->_progressY);
	clip.clip(_globalSurface.getBounds());
	if (!clip.isEmpty())
		g_system->copyRectToScreen(_globalSurface.getBasePtr(clip.left, clip.top), _globalSurface.pitch, clip.left, clip.top, clip.width(), clip.height());

	g_system->updateScreen();
}

void Gfx::drawPointer() {
	static int anim = 0;
	static uint32 animTime = 0;

	if (animTime < g_system->getMillis()) {
		animTime = g_system->getMillis() + 50;
		anim = (anim + 1) & 7;
	}

	// If pointer is not displayable and we are in game, exit
	if (!_pointerDisplayable && g_hdb->getGameState() == GAME_PLAY)
		return;

	// If we are in game and the cursor should be displayed, draw it
	if (_showCursor || g_hdb->getGameState() != GAME_PLAY) {
		if (g_hdb->isPPC())
			CursorMan.showMouse(true);
		else
			_mousePointer[anim]->drawMasked(g_hdb->_input->getMouseX() - 16, g_hdb->_input->getMouseY() - 16);
	}
}

void Gfx::setPointerState(int value) {
	_pointerDisplayable = value;
}

void Gfx::setFade(bool fadeIn, bool black, int steps) {
	_fadeInfo.isFadeIn = fadeIn;
	_fadeInfo.isBlack = black;

	if (!steps)
		steps = 1;

	_fadeInfo.speed = steps;

	if (fadeIn)
		_fadeInfo.curStep = 0;
	else
		_fadeInfo.curStep = 255;

	_fadeInfo.active = true;
}

void Gfx::updateFade() {
	if (!_fadeInfo.active && !_fadeInfo.stayFaded)
		return;

	debug(7, "updateFade: active: %d stayFaded: %d isBlack: %d speed: %d isFadeIn: %d curStep: %d",
		_fadeInfo.active, _fadeInfo.stayFaded, _fadeInfo.isBlack, _fadeInfo.speed, _fadeInfo.isFadeIn, _fadeInfo.curStep);

	if (g_hdb->isPPC()) {
		if (!_fadeInfo.isBlack) {
			// Black fade
			for (int y = 0; y < g_hdb->_screenHeight; y++) {
				uint16 *ptr = (uint16 *)_fadeBuffer1.getBasePtr(0, y);
				for (int x = 0; x < g_hdb->_screenWidth; x++) {
					uint16 value = *ptr;
					if (value) {
						uint8 r, g, b;
						g_hdb->_format.colorToRGB(value, r, g, b);
						r = (r * _fadeInfo.curStep) >> 8;
						g = (g * _fadeInfo.curStep) >> 8;
						b = (b * _fadeInfo.curStep) >> 8;
						*ptr = g_hdb->_format.RGBToColor(r, g, b);
					}
					ptr++;
				}
			}

		} else {
			// White fade
			for (int y = 0; y < g_hdb->_screenHeight; y++) {
				uint16 *ptr = (uint16 *)_fadeBuffer1.getBasePtr(0, y);
				for (int x = 0; x < g_hdb->_screenWidth; x++) {
					uint16 value = *ptr;

					uint8 r, g, b;
					g_hdb->_format.colorToRGB(value, r, g, b);
					r += (255 - r) * (256 - _fadeInfo.curStep) / 256;
					g += (255 - g) * (256 - _fadeInfo.curStep) / 256;
					b += (255 - b) * (256 - _fadeInfo.curStep) / 256;
					*ptr = g_hdb->_format.RGBToColor(r, g, b);
					ptr++;
				}
			}
		}

		if (_fadeInfo.isFadeIn) {
			if (_fadeInfo.active)
				_fadeInfo.curStep += _fadeInfo.speed;

			if (_fadeInfo.curStep > 255) {
				_fadeInfo.curStep = 255;
				_fadeInfo.active = false;
				_fadeInfo.stayFaded = false;
			}
		} else {
			if (_fadeInfo.active)
				_fadeInfo.curStep -= _fadeInfo.speed;

			if (_fadeInfo.curStep < 1) {
				_fadeInfo.curStep = 0;
				_fadeInfo.active = false;
				_fadeInfo.stayFaded = true;
			}
		}
	} else {
		_fadeBuffer2.blitFrom(_globalSurface);

		static int waitAFrame = 0;

		do {
			// Copy pristine copy of background to modification buffer
			_fadeBuffer1.blitFrom(_fadeBuffer2);

			// do the actual alphablending

			if (!_fadeInfo.isBlack) {
				// Black Fade

				for (int y = 0; y < g_hdb->_screenHeight; y++) {
					uint16 *ptr = (uint16 *)_fadeBuffer1.getBasePtr(0, y);
					for (int x = 0; x < g_hdb->_screenWidth; x++) {
						uint16 value = *ptr;
						if (value) {
							uint8 r, g, b;
							g_hdb->_format.colorToRGB(value, r, g, b);
							r = (r * _fadeInfo.curStep) >> 8;
							g = (g * _fadeInfo.curStep) >> 8;
							b = (b * _fadeInfo.curStep) >> 8;
							*ptr = g_hdb->_format.RGBToColor(r, g, b);
						}
						ptr++;
					}
				}
			} else {
				// White Fade

				for (int y = 0; y < g_hdb->_screenHeight; y++) {
					uint16 *ptr = (uint16 *)_fadeBuffer1.getBasePtr(0, y);
					for (int x = 0; x < g_hdb->_screenWidth; x++) {
						uint16 value = *ptr;
						uint8 r, g, b;
						g_hdb->_format.colorToRGB(value, r, g, b);
						r += (255 - r) * (256 - _fadeInfo.curStep) / 256;
						g += (255 - g) * (256 - _fadeInfo.curStep) / 256;
						b += (255 - b) * (256 - _fadeInfo.curStep) / 256;
						*ptr = g_hdb->_format.RGBToColor(r, g, b);
						ptr++;
					}
				}
			}

			_globalSurface.blitFrom(_fadeBuffer1);
			g_system->copyRectToScreen(_globalSurface.getBasePtr(0, 0), _globalSurface.pitch, 0, 0, _globalSurface.w, _globalSurface.h);

			// step the fading values to the next one and
			// see if we're done yet
			if (_fadeInfo.isFadeIn) {
				if (_fadeInfo.active)
					_fadeInfo.curStep += _fadeInfo.speed;

				if (_fadeInfo.curStep > 255) {
					_fadeInfo.curStep = 255;
					_fadeInfo.active = false;
					_fadeInfo.stayFaded = false;
				}
			} else {
				if (_fadeInfo.active == true)
					_fadeInfo.curStep -= _fadeInfo.speed;

				if (_fadeInfo.curStep < 1) {
					_fadeInfo.curStep = 0;
					_fadeInfo.active = false;
					_fadeInfo.stayFaded = false;
				}
			}

			// make sure we wait one frame at least - some logic in the game
			// doesn't draw the frame immediately
			if (!waitAFrame) {
				waitAFrame++;
				return;
			}

			g_system->updateScreen();
			if (g_hdb->getDebug()) {
				g_hdb->_frames.push_back(g_system->getMillis());
				while (g_hdb->_frames[0] < g_system->getMillis() - 1000)
					g_hdb->_frames.remove_at(0);
			}
			g_system->delayMillis(1000 / kGameFPS);

		} while (_fadeInfo.active);

		waitAFrame = 0;			// reset counter
	}
}

void Gfx::turnOnSnow() {
	_snowInfo.active = true;
	for (int i = 0; i < MAX_SNOW; i++) {
		_snowInfo.x[i] = g_hdb->_rnd->getRandomNumber(g_hdb->_screenWidth - 1);
		_snowInfo.y[i] = g_hdb->_rnd->getRandomNumber(g_hdb->_screenHeight - 1);
		_snowInfo.yv[i] = g_hdb->_rnd->getRandomNumber(2) + 1;
		_snowInfo.xvindex[i] = g_hdb->_rnd->getRandomNumber(MAX_SNOW_XV - 1);
	}
}

Picture *Gfx::loadPic(const char *picName) {
	Common::SeekableReadStream *stream = g_hdb->_fileMan->findFirstData(picName, TYPE_PIC);
	if (!stream)
		return nullptr;

	Picture *pic = new Picture;
	pic->load(stream);
	delete stream;
	return pic;
}

Tile *Gfx::loadTile(const char *tileName) {
	Common::SeekableReadStream *stream = g_hdb->_fileMan->findFirstData(tileName, TYPE_TILE32);
	if (!stream)
		return nullptr;

	Tile *tile = new Tile;
	tile->load(stream);
	delete stream;
	return tile;
}

Tile *Gfx::loadIcon(const char *tileName) {
	Common::SeekableReadStream *stream = g_hdb->_fileMan->findFirstData(tileName, TYPE_ICON32);
	if (!stream)
		return nullptr;

	Tile *tile = new Tile;
	tile->load(stream);
	delete stream;
	return tile;
}

void Gfx::setPixel(int x, int y, uint16 color) {
	if (x < 0 || y < 0 || x >= _globalSurface.w || y >= _globalSurface.h)
		return;

	*(uint16 *)_globalSurface.getBasePtr(x, y) = color;
	g_system->copyRectToScreen(_globalSurface.getBasePtr(x, y), _globalSurface.pitch, x, y, 1, 1);
}

Tile *Gfx::getTile(int index) {
	if (index < 0 || index > _numTiles) {
		if (index != 0xFFFF)
			debug(6, "getTile(%d): wrong index > %d", index, _numTiles);
		return nullptr;
	}
	if (_tLookupArray[index].skyIndex) {
		debug(6, "getTile(%d): sky tile (%d)", index, _tLookupArray[index].skyIndex);
		// We don't draw Sky Tiles, so return nullptr
		return nullptr;
	}

	if (_tLookupArray[index].tData == nullptr) {
		Common::SeekableReadStream *stream = g_hdb->_fileMan->findFirstData(_tLookupArray[index].filename, TYPE_TILE32);
		Tile *tile = new Tile;
		tile->load(stream);
		delete stream;
		_tLookupArray[index].tData = tile;
	}

	return _tLookupArray[index].tData;
}

void Gfx::emptyGfxCaches() {
	// We have plenty of memory, so do not do it
}

void Gfx::cacheTileSequence(int tileIndex, int count) {
	for (int i = tileIndex; i < tileIndex + count; i++)
		getTile(i);
}

int Gfx::getTileIndex(const char *name) {
	if (!name)
		return -1;

	for (int i = 0; i < _numTiles; i++) {
		if (Common::matchString(_tLookupArray[i].filename, name))
			return i;
	}
	return -1;
}

Picture *Gfx::getPicture(const char *name) {
	Common::SeekableReadStream *stream = g_hdb->_fileMan->findFirstData(name, TYPE_PIC);
	if (stream == nullptr)
		return nullptr;

	Picture *picture = new Picture;
	picture->load(stream);
	delete stream;
	return picture;
}

// Returns: true->Tile, false->Pic
bool Gfx::selectGfxType(const char *name) {
	// Check for Pic types
	if (Common::matchString(name, "clubup1"))
		return false;
	if (Common::matchString(name, "clubup2"))
		return false;
	if (Common::matchString(name, "clubup3"))
		return false;
	if (Common::matchString(name, "clubup4"))
		return false;
	if (Common::matchString(name, "clubdown1"))
		return false;
	if (Common::matchString(name, "clubdown2"))
		return false;
	if (Common::matchString(name, "clubdown3"))
		return false;
	if (Common::matchString(name, "clubdown4"))
		return false;
	if (Common::matchString(name, "clubleft1"))
		return false;
	if (Common::matchString(name, "clubleft2"))
		return false;
	if (Common::matchString(name, "clubleft3"))
		return false;
	if (Common::matchString(name, "clubleft4"))
		return false;
	if (Common::matchString(name, "clubright1"))
		return false;
	if (Common::matchString(name, "clubright2"))
		return false;
	if (Common::matchString(name, "clubright3"))
		return false;
	if (Common::matchString(name, "clubright4"))
		return false;
	if (Common::matchString(name, "slug_shot1"))
		return false;
	if (Common::matchString(name, "slug_shot2"))
		return false;
	if (Common::matchString(name, "slug_shot3"))
		return false;
	if (Common::matchString(name, "slug_shot4"))
		return false;

	return true;
}

Tile *Gfx::getTileGfx(const char *name, int32 size) {
	// Try to find graphic
	for (Common::Array<GfxCache *>::iterator it = _gfxCache->begin(); it != _gfxCache->end(); ++it) {
		if (Common::matchString((*it)->name, name)) {
			if ((*it)->loaded == -1) {	// Marked for Deletetion?
				(*it)->loaded = 1;		// Reactivate it
				return (*it)->tileGfx;
			}
		}
	}

	GfxCache *gc = new GfxCache;
	Common::strlcpy(gc->name, name, 32);
	gc->tileGfx = loadTile(name);
	gc->status = false;
	if (size == -1)
		size = g_hdb->_fileMan->getLength(name, TYPE_TILE32);
	gc->size = size;
	gc->loaded = 1;

	_gfxCache->push_back(gc);

	return gc->tileGfx;
}

void Gfx::markGfxCacheFreeable() {
	for (Common::Array<GfxCache *>::iterator it = _gfxCache->begin(); it != _gfxCache->end(); ++it)
		(*it)->loaded = -1;
}

void Gfx::markTileCacheFreeable() {
	// we have plenty of memory, so do not do it
}

Picture *Gfx::getPicGfx(const char *name, int32 size) {
	// Try to find graphic
	for (Common::Array<GfxCache *>::iterator it = _gfxCache->begin(); it != _gfxCache->end(); ++it) {
		if (Common::matchString((*it)->name, name)) {
			if ((*it)->loaded == -1) {	// Marked for Deletetion?
				(*it)->loaded = 1;		// Reactivate it
				return (*it)->picGfx;
			}
		}
	}

	GfxCache *gc = new GfxCache;
	Common::strlcpy(gc->name, name, 32);
	gc->picGfx = loadPic(name);
	gc->status = true;
	if (size == -1)
		size = g_hdb->_fileMan->getLength(name, TYPE_PIC);
	gc->size = size;
	gc->loaded = 1;

	_gfxCache->push_back(gc);

	return gc->picGfx;
}

int Gfx::isSky(int index) {
	if (!index)
		return 0;

	for (int i = 0; i < kMaxSkies; i++) {
		if (_skyTiles[i] == index)
			return i + 1; // The skyTiles are indexed from 1. 0 => No Sky tile
	}

	return 0;
}

void Gfx::setSky(int skyIndex) {
	int tileIndex = _skyTiles[skyIndex - 1];
	_currentSky = skyIndex;

	// Clear memory used by last sky
	if (tileIndex != _tileSkyClouds && _skyClouds) {
		delete _skyClouds;
		_skyClouds = nullptr;
	}

	// Setup current sky
	if (tileIndex == _tileSkyStars)
		setup3DStars();
	else if (tileIndex == _tileSkyStarsLeft)
		setup3DStarsLeft();
	else if (tileIndex == _tileSkyClouds)
		_skyClouds = getPicture(CLOUDY_SKIES);
}

void Gfx::setup3DStars() {
	for (int i = 0; i < kNum3DStars; i++) {
		_stars3D[i].x = g_hdb->_rnd->getRandomNumber(g_hdb->_screenWidth - 1);
		_stars3D[i].y = g_hdb->_rnd->getRandomNumber(g_hdb->_screenHeight - 1);
		_stars3D[i].speed = g_hdb->_rnd->getRandomNumber(255);
		if (g_hdb->isPPC())
			_stars3D[i].color = g_hdb->_format.RGBToColor(_stars3D[i].speed, _stars3D[i].speed, _stars3D[i].speed);
		else {
			_stars3D[i].speed >>= 1;
			_stars3D[i].color = _stars3D[i].speed / 64;
		}
	}
}

void Gfx::setup3DStarsLeft() {
	for (int i = 0; i < kNum3DStars; i++) {
		_stars3DSlow[i].x = g_hdb->_rnd->getRandomNumber(g_hdb->_screenWidth - 1);
		_stars3DSlow[i].y = g_hdb->_rnd->getRandomNumber(g_hdb->_screenHeight - 1);
		_stars3DSlow[i].speed = ((double) (1 + g_hdb->_rnd->getRandomNumber(4))) / 6.0;
		if (g_hdb->isPPC())
			_stars3DSlow[i].color = g_hdb->_format.RGBToColor((int)(_stars3DSlow[i].speed * 250), (int)(_stars3DSlow[i].speed * 250), (int)(_stars3DSlow[i].speed * 250));
		else
			_stars3DSlow[i].color = (int) (_stars3DSlow[i].speed * 4.00);
	}
}

void Gfx::draw3DStars() {
	fillScreen(0);
	for (int i = 0; i < kNum3DStars; i++) {
		if (g_hdb->isPPC()) {
			setPixel((int)_stars3D[i].x, (int)_stars3D[i].y, _stars3D[i].color);
			_stars3D[i].y += (_stars3D[i].speed >> 5);
		} else {
			_starField[_stars3D[i].color]->drawMasked((int)_stars3D[i].x, (int)_stars3D[i].y);
			_stars3D[i].y += (_stars3D[i].speed >> 5) + 1;
		}

		if (_stars3D[i].y > g_hdb->_screenHeight)
			_stars3D[i].y = 0;
	}
}

void Gfx::draw3DStarsLeft() {
	fillScreen(0);
	for (int i = 0; i < kNum3DStars; i++) {
		if (g_hdb->isPPC())
			setPixel((int)_stars3DSlow[i].x, (int)_stars3DSlow[i].y, _stars3DSlow[i].color);
		else
			_starField[_stars3DSlow[i].color]->drawMasked((int)_stars3DSlow[i].x, (int)_stars3DSlow[i].y);
		_stars3DSlow[i].x -= _stars3DSlow[i].speed;
		if (_stars3DSlow[i].x < 0)
			_stars3DSlow[i].x = g_hdb->_screenWidth - 1;
	}
}

void Gfx::drawSky() {
	int tile = _skyTiles[_currentSky - 1];

	if (tile == _tileSkyStars)
		draw3DStars();
	else if (tile == _tileSkyStarsLeft)
		draw3DStarsLeft();
	else if (tile == _tileSkyClouds) {
		static int offset = 0, wait = 0;
		for (int j = -64; j < g_hdb->_screenHeight; j += 64) {
			for (int i = -64; i < g_hdb->_screenWidth; i += 64) {
				if (_skyClouds)
					_skyClouds->draw(i + offset, j + offset);
			}
		}
		wait--;
		if (wait < 1) {
			offset = (offset + 1) & 63;
			wait = 5;
		}
	}
}

static const int snowXVList[13] = {0, -1, -1, -2, -2, -1, 0, 0, 0, -1, -2, -1, 0};

void Gfx::drawSnow() {
	if (_snowInfo.active == false)
		return;

	for (int i = 0; i < MAX_SNOW; i++) {
		if (g_hdb->isPPC()) {
			uint16 color = g_hdb->_format.RGBToColor(160, 160, 160);
			setPixel((int)_snowInfo.x[i] + 1, (int)_snowInfo.y[i], color);
			setPixel((int)_snowInfo.x[i] - 1, (int)_snowInfo.y[i], color);
			setPixel((int)_snowInfo.x[i], (int)_snowInfo.y[i] + 1, color);
			setPixel((int)_snowInfo.x[i], (int)_snowInfo.y[i] - 1, color);
		} else
			_snowflake->drawMasked((int)_snowInfo.x[i], (int)_snowInfo.y[i]);
		_snowInfo.x[i] += snowXVList[_snowInfo.xvindex[i]++];
		_snowInfo.y[i] += _snowInfo.yv[i];
		if (_snowInfo.xvindex[i] == MAX_SNOW_XV)
			_snowInfo.xvindex[i] = 0;
		if (_snowInfo.x[i] < 0)
			_snowInfo.x[i] = g_hdb->_screenWidth - 1;
		if (_snowInfo.y[i] > g_hdb->_screenHeight - 1)
			_snowInfo.y[i] = 0;
	}
}

int Gfx::animateTile(int tileIndex) {
	return _tLookupArray[tileIndex].animIndex;
}

bool Gfx::loadFont(const char *string) {
	Common::SeekableReadStream *stream = g_hdb->_fileMan->findFirstData(string, TYPE_FONT);
	if (!stream)
		return false;

	if (g_hdb->isPPC()) {
		const int32 ulength = g_hdb->_fileMan->getLength(string, TYPE_FONT);
		byte *buffer = (byte *)malloc(ulength);
		stream->read(buffer, ulength);
		Common::MemoryReadStream memoryStream(buffer, ulength, DisposeAfterUse::YES);
		delete stream;

		// Loading _fontHeader
		_fontHeader.type = (int)memoryStream.readUint32LE();
		_fontHeader.numChars = (int)memoryStream.readUint32LE();
		_fontHeader.height = (int)memoryStream.readUint32LE();
		_fontHeader.kerning = (int)memoryStream.readUint32LE();
		_fontHeader.leading = (int)memoryStream.readUint32LE();

		debug(3, "Loaded _fontHeader with following data");
		debug(3, "type: %d", _fontHeader.type);
		debug(3, "numChars: %d", _fontHeader.numChars);
		debug(3, "height: %d", _fontHeader.height);
		debug(3, "kerning: %d", _fontHeader.kerning);
		debug(3, "leading: %d", _fontHeader.leading);

		// Loading _charInfoBlocks & creating character surfaces

		// Position after _fontHeader
		int startPos = memoryStream.pos();
		for (int i = 0; i < _fontHeader.numChars; i++) {
			CharInfo *cInfo = new CharInfo;
			cInfo->width = (int16)memoryStream.readUint32LE();
			cInfo->offset = (int32)memoryStream.readUint32LE();

			debug(3, "Loaded _charInfoBlocks[%d]: width: %d, offset: %d", i, cInfo->width, cInfo->offset);

			// Position after reading cInfo
			int curPos = memoryStream.pos();

			_fontSurfaces[i].create(cInfo->width, _fontHeader.height, g_hdb->_format);

			// Go to character location
			memoryStream.seek(startPos + cInfo->offset);

			for (int x = 0; x < cInfo->width; x++) {
				for (int y = 0; y < _fontHeader.height; y++) {
					int u = x;
					int v = _fontHeader.height - y - 1;
					uint16 *ptr = (uint16 *)_fontSurfaces[i].getBasePtr(u, v);
					*ptr = memoryStream.readUint16LE();
				}
			}

			memoryStream.seek(curPos);

			_charInfoBlocks.push_back(cInfo);
		}

		// Loading _fontGfx
		_fontGfx = memoryStream.readUint16LE();
	} else {
		// Loading _fontHeader
		_fontHeader.type = (int)stream->readUint32LE();
		_fontHeader.numChars = (int)stream->readUint32LE();
		_fontHeader.height = (int)stream->readUint32LE();
		_fontHeader.kerning = (int)stream->readUint32LE();
		_fontHeader.leading = (int)stream->readUint32LE();

		debug(3, "Loaded _fontHeader with following data");
		debug(3, "type: %d", _fontHeader.type);
		debug(3, "numChars: %d", _fontHeader.numChars);
		debug(3, "height: %d", _fontHeader.height);
		debug(3, "kerning: %d", _fontHeader.kerning);
		debug(3, "leading: %d", _fontHeader.leading);

		// Loading _charInfoBlocks & creating character surfaces

		// Position after _fontHeader
		int startPos = stream->pos();
		for (int i = 0; i < _fontHeader.numChars; i++) {
			CharInfo *cInfo = new CharInfo;
			cInfo->width = (int16)stream->readUint32LE();
			cInfo->offset = (int32)stream->readUint32LE();

			debug(3, "Loaded _charInfoBlocks[%d]: width: %d, offset: %d", i, cInfo->width, cInfo->offset);

			// Position after reading cInfo
			int curPos = stream->pos();

			_fontSurfaces[i].create(cInfo->width, _fontHeader.height, g_hdb->_format);

			// Go to character location
			stream->seek(startPos + cInfo->offset);

			for (int y = 0; y < _fontHeader.height; y++) {
				uint16 *ptr = (uint16 *)_fontSurfaces[i].getBasePtr(0, y);
				for (int x = 0; x < cInfo->width; x++) {
					*ptr = stream->readUint16LE();
					ptr++;
				}
			}

			stream->seek(curPos);

			_charInfoBlocks.push_back(cInfo);
		}

		// Loading _fontGfx
		_fontGfx = stream->readUint16LE();
		delete stream;
	}

	return true;
}

void Gfx::drawText(const char *string) {
	if (!_systemInit)
		return;

	if (_cursorX < _eLeft)
		_cursorX = _eLeft;
	if (_cursorY < _eTop)
		_cursorY = _eTop;

	// Word Wrapping
	int width = _eLeft;
	char cr[256];	// Carriage Return Array

	for (int i = 0; i < (int)strlen(string); i++) {
		unsigned char c = string[i];
		width += _charInfoBlocks[c]->width + _fontHeader.kerning + kFontIncrement;
		if (c == ' ')
			width += kFontSpace;

		cr[i] = 0;
		if (c == '\n') {
			cr[i] = 1;
			width = _eLeft;
		} else if (width > _eRight) {
			i--;
			while (string[i] != ' ' && i > 0)
				i--;
			cr[i] = 1;
			width = _eLeft;
		}
	}

	// Draw the characters
	for (int j = 0; j < (int)strlen(string); j++) {
		unsigned char c = string[j];
		if (c == '\n' || cr[j]) {
			_cursorX = _eLeft;
			_cursorY += _fontHeader.height + _fontHeader.leading;
			if (_cursorY + _fontHeader.height > _eBottom)
				_cursorY = _eTop;
			continue;
		}

		width = _charInfoBlocks[c]->width;
		if (c == ' ')
			width = kFontSpace;

		// Blit the character
		_globalSurface.transBlitFrom(_fontSurfaces[c], Common::Point(_cursorX, _cursorY), 0xf81f);

		Common::Rect clip(0, 0, width, _fontHeader.height);
		clip.moveTo(_cursorX, _cursorY);
		clip.clip(_globalSurface.getBounds());
		if (!clip.isEmpty()) {
			g_system->copyRectToScreen(_globalSurface.getBasePtr(clip.left, clip.top), _globalSurface.pitch, clip.left, clip.top, clip.width(), clip.height());
		}

		// Advance the cursor
		_cursorX += width + _fontHeader.kerning + kFontIncrement;
		if (_cursorX > g_hdb->_screenWidth) {
			_cursorX = 0;
			_cursorY += _fontHeader.height + _fontHeader.leading;
			if (_cursorY + _fontHeader.height > g_hdb->_screenHeight)
				_cursorY = 0;
		}
	}
}

// Calculates pixel width of a string
void Gfx::getDimensions(const char *string, int *pixelsWide, int *lines) {
	if (!string) {
		*pixelsWide = kFontSpace;
		*lines = 1;
		return;
	}

	int maxWidth = 0;
	int width = _eLeft;
	int height = 1;

	for (int i = 0; i < (int)strlen(string); i++) {
		unsigned char c = string[i];
		width += _charInfoBlocks[c]->width + _fontHeader.kerning + kFontIncrement;
		if (c == ' ')
			width += kFontSpace;

		if (c == '\n') {
			height++;
			if (width > maxWidth)
				maxWidth = width;
			width = _eLeft;
		} else if (width > _eRight) {
			int oldWidth = width;
			i--;
			while (string[i] != ' ' && i > 0) {
				c = string[i];
				width -= _charInfoBlocks[c]->width + _fontHeader.kerning + kFontIncrement;
				i--;
			}
			if (!i && !g_hdb->isPPC()) {
				maxWidth = oldWidth;
				break;
			}
			height++;
			if (width > maxWidth)
				maxWidth = width;
			width = _eLeft;
		}
	}

	if (width > maxWidth)
		maxWidth = width;

	// If its one line, add 8 pixels
	if (height == 1)
		maxWidth += 8;

	*pixelsWide = maxWidth - _eLeft;
	*lines = height;
}

int Gfx::stringLength(const char *string) {
	int w, h;
	getDimensions(string, &w, &h);
	return w;
}

void Gfx::centerPrint(const char *string) {
	int totalWidth = 0;

	for (int i = 0; i < (int)strlen(string); i++) {
		if (string[i] == ' ')
			totalWidth += kFontSpace;
		else if (string[i] != '\n')
			totalWidth += _charInfoBlocks[string[i]]->width;
	}

	setCursor(g_hdb->_screenWidth / 2 - totalWidth / 2, _cursorY);
	drawText(string);
}

void Gfx::setTextEdges(int left, int right, int top, int bottom) {
	_eLeft = left;
	_eRight = right;
	_eTop = top;
	_eBottom = bottom;
}

void Gfx::getTextEdges(int *left, int *right, int *top, int *bottom) {
	*left = _eLeft;
	*right = _eRight;
	*top = _eTop;
	*bottom = _eBottom;
}

void Gfx::setKernLead(int kern, int lead) {
	_fontHeader.kerning = kern;
	_fontHeader.leading = lead;
}

void Gfx::getKernLead(int *kern, int *lead) {
	*kern = _fontHeader.kerning;
	*lead = _fontHeader.leading;
}

void Gfx::setCursor(int x, int y) {
	_cursorX = x;
	_cursorY = y;
}

void Gfx::getCursor(int *x, int *y) {
	*x = _cursorX;
	*y = _cursorY;
}

void Gfx::turnOnBonusStars(int which) {
	if (!g_hdb->isDemo())
		return;

	_starsInfo.active = true;
	for (int i = 0; i < 10; i++)
		_starsInfo.starAngle[i] = (36 * (i + 1)) - 10;
	if (!_starsInfo.gfx[0]) {
		switch (which) {
		case 0:		// Red Star
			_starsInfo.gfx[0] = loadPic(SECRETSTAR_RED1);
			_starsInfo.gfx[1] = loadPic(SECRETSTAR_RED2);
			break;
		case 1:		// Green Star
			_starsInfo.gfx[0] = loadPic(SECRETSTAR_GREEN1);
			_starsInfo.gfx[1] = loadPic(SECRETSTAR_GREEN2);
			break;
		case 2:		// Blue Star
			_starsInfo.gfx[0] = loadPic(SECRETSTAR_BLUE1);
			_starsInfo.gfx[1] = loadPic(SECRETSTAR_BLUE2);
			break;
		default:
			break;
		}
	}

	_starsInfo.radius = 0;
	_starsInfo.angleSpeed = 25;
	_starsInfo.timer = g_hdb->getTimeSlice() + 500;
	_starsInfo.anim = 0;
	_starsInfo.totalTime = g_hdb->getTimeSlice() + 5000;		// 5 seconds long
	g_hdb->_sound->playSound(SND_MONKEYSTONE_SECRET_STAR);
}

void Gfx::drawBonusStars() {
	if (!_starsInfo.active)
		return;

	if (_starsInfo.timer < g_hdb->getTimeSlice()) {
		_starsInfo.timer = g_hdb->getTimeSlice() + 500;
		_starsInfo.anim = 1 - _starsInfo.anim;
	}

	int w = _starsInfo.gfx[0]->_width / 2;
	int h = _starsInfo.gfx[0]->_height / 2;

	for (int i = 0; i < 10; i++) {
		if (g_hdb->isPPC()) {
			_starsInfo.gfx[_starsInfo.anim]->drawMasked(
				(g_hdb->_screenWidth / 2) + (int)((double)_starsInfo.radius * _cosines->at(_starsInfo.starAngle[i]) - w),
				(g_hdb->_screenHeight / 2) + (int)((double)_starsInfo.radius * _sines->at(_starsInfo.starAngle[i]) - h)
				);
		} else {
			_starsInfo.gfx[_starsInfo.anim]->drawMasked(
				(int)(g_hdb->_screenDrawWidth / 2 + ((float)_starsInfo.radius / 2)) + (int)((double)_starsInfo.radius * _cosines->at(_starsInfo.starAngle[i]) - w),
				(g_hdb->_screenDrawHeight / 2) + (int)((double)_starsInfo.radius * _sines->at(_starsInfo.starAngle[i]) - h)
			);
		}

		int angle = (int)(_starsInfo.starAngle[i] + _starsInfo.angleSpeed);
		if (angle >= 360)
			angle = 0;
		_starsInfo.starAngle[i] = angle;
	}

	_starsInfo.radius++;
	_starsInfo.angleSpeed -= 0.25;
	if (_starsInfo.angleSpeed < 15)
		_starsInfo.angleSpeed = 15;

	// timed out?
	if (_starsInfo.totalTime < g_hdb->getTimeSlice()) {
		_starsInfo.active = false;
		delete _starsInfo.gfx[0];
		delete _starsInfo.gfx[1];
		_starsInfo.gfx[0] = _starsInfo.gfx[1] = 0;
	}
}

void Gfx::drawDebugInfo(Tile *_debugLogo, int fps) {
	_debugLogo->drawMasked(g_hdb->_screenWidth - 32, 0);

	// Draw  FPS
	setCursor(0, 0);
	char buff[64];
	sprintf(buff, "FPS: %d", fps);
	drawText(buff);

	// Draw Player Info
	setCursor(0, 16);

	int x, y;
	g_hdb->_ai->getPlayerXY(&x, &y);
	sprintf(buff, "Player X: %d, Y: %d", x / kTileWidth, y / kTileHeight);
	drawText(buff);

	setCursor(0, 32);
	AIEntity *p = g_hdb->_ai->getPlayer();
	if (p) {
		sprintf(buff, "Player height level: %d", p->level);
		drawText(buff);
	}

	setCursor(0, 48);
	sprintf(buff, "Map Name: %s", g_hdb->getInMapName());
	drawText(buff);

	setCursor(0, 64);
	g_hdb->getActionMode() ? sprintf(buff, "Action Mode") : sprintf(buff, "Puzzle Mode");
	drawText(buff);
}

Picture::Picture() : _width(0), _height(0) {
	_name[0] = 0;
	_surface.create(_width, _height, g_hdb->_format);
}

Picture::~Picture() {
}

Graphics::Surface Picture::load(Common::SeekableReadStream *stream) {
	_width = stream->readUint32LE();
	_height = stream->readUint32LE();
	stream->read(_name, 64);

	debug(8, "Picture: _width: %d, _height: %d", _width, _height);
	debug(8, "Picture: _name: %s", _name);

	if (g_hdb->isPPC()) {
		_surface.create(_width, _height, g_hdb->_format);

		for (int x = 0; x < _width; x++) {
			for (int y = 0; y < _height; y++) {
				int u = x;
				int v = _height - y - 1;
				uint16 *ptr = (uint16 *)_surface.getBasePtr(u, v);
				*ptr = stream->readUint16LE();
			}
		}

	} else {
		_surface.create(_width, _height, g_hdb->_format);
		stream->readUint32LE(); // Skip Win32 Surface

		for (int y = 0; y < _height; y++) {
			uint16 *ptr = (uint16 *)_surface.getBasePtr(0, y);
			for (int x = 0; x < _width; x++) {
				*ptr = stream->readUint16LE();
				ptr++;
			}
		}
	}

	return _surface;
}

int Picture::draw(int x, int y) {
	g_hdb->_gfx->_globalSurface.blitFrom(_surface, Common::Point(x, y));

	Common::Rect clip(_surface.getBounds());
	clip.moveTo(x, y);
	clip.clip(g_hdb->_gfx->_globalSurface.getBounds());
	if (!clip.isEmpty()) {
		g_system->copyRectToScreen(g_hdb->_gfx->_globalSurface.getBasePtr(clip.left, clip.top), g_hdb->_gfx->_globalSurface.pitch, clip.left, clip.top, clip.width(), clip.height());
		return 1;
	}
	return 0;
}

int Picture::drawMasked(int x, int y, int alpha) {
	g_hdb->_gfx->_globalSurface.transBlitFrom(_surface, Common::Point(x, y), 0xf81f, false, 0, alpha & 0xff);

	Common::Rect clip(_surface.getBounds());
	clip.moveTo(x, y);
	clip.clip(g_hdb->_gfx->_globalSurface.getBounds());
	if (!clip.isEmpty()) {
		g_system->copyRectToScreen(g_hdb->_gfx->_globalSurface.getBasePtr(clip.left, clip.top), g_hdb->_gfx->_globalSurface.pitch, clip.left, clip.top, clip.width(), clip.height());
		return 1;
	}
	return 0;
}

Tile::Tile() : _flags(0) {
	_name[0] = 0;
	_surface.create(32, 32, g_hdb->_format);
}

Tile::~Tile() {
}

Graphics::Surface Tile::load(Common::SeekableReadStream *stream) {
	_flags = stream->readUint32LE();
	stream->read(_name, 64);

	_surface.create(32, 32, g_hdb->_format);
	if (g_hdb->isPPC()) {
		for (int y = 0; y < 32; y++) {
			for (int x = 0; x < 32; x++) {
				int u = y;
				int v = 32 - x - 1;
				uint16 *ptr = (uint16 *)_surface.getBasePtr(u, v);
				*ptr = stream->readUint16LE();
			}
		}
	} else {
		stream->readUint32LE(); // Skip Win32 Surface
		for (uint y = 0; y < 32; y++) {
			uint16 *ptr = (uint16 *)_surface.getBasePtr(0, y);
			for (uint x = 0; x < 32; x++) {
				*ptr = stream->readUint16LE();
				ptr++;
			}
		}
	}

	return _surface;
}

int Tile::draw(int x, int y) {
	g_hdb->_gfx->_globalSurface.blitFrom(_surface, Common::Point(x, y));

	Common::Rect clip(_surface.getBounds());
	clip.moveTo(x, y);
	clip.clip(g_hdb->_gfx->_globalSurface.getBounds());
	if (!clip.isEmpty()) {
		g_system->copyRectToScreen(g_hdb->_gfx->_globalSurface.getBasePtr(clip.left, clip.top), g_hdb->_gfx->_globalSurface.pitch, clip.left, clip.top, clip.width(), clip.height());
		return 1;
	}
	return 0;
}

int Tile::drawMasked(int x, int y, int alpha) {
	g_hdb->_gfx->_globalSurface.transBlitFrom(_surface, Common::Point(x, y), 0xf81f, false, 0, alpha & 0xff);

	Common::Rect clip(_surface.getBounds());
	clip.moveTo(x, y);
	clip.clip(g_hdb->_gfx->_globalSurface.getBounds());
	if (!clip.isEmpty()) {
		g_system->copyRectToScreen(g_hdb->_gfx->_globalSurface.getBasePtr(clip.left, clip.top), g_hdb->_gfx->_globalSurface.pitch, clip.left, clip.top, clip.width(), clip.height());
		return 1;
	}
	return 0;
}

}
