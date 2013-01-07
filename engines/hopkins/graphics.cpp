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
 */

#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/decoders/pcx.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

GraphicsManager::GraphicsManager() {
	_lockCounter = 0;
	SDL_MODEYES = false;
	XSCREEN = YSCREEN = 0;
	WinScan = 0;
	PAL_PIXELS = NULL;
	_lineNbr = 0;
	_videoPtr = NULL;
	_scrollOffset = 0;
	SCROLL = 0;
	_largeScreenFl = false;
	OLD_SCROLL = 0;

	_lineNbr2 = 0;
	Agr_x = Agr_y = 0;
	Agr_Flag_x = Agr_Flag_y = 0;
	_fadeDefaultSpeed = 15;
	FADE_LINUX = 0;
	_skipVideoLockFl = false;
	no_scroll = 0;
	min_x = 0;
	min_y = 20;
	max_x = SCREEN_WIDTH * 2;
	max_y = SCREEN_HEIGHT - 20;
	clip_x = clip_y = 0;
	clip_x1 = clip_y1 = 0;
	clip_flag = false;
	Red_x = Red_y = 0;
	Red = 0;
	_width = 0;
	Compteur_y = 0;
	spec_largeur = 0;

	Common::fill(&SD_PIXELS[0], &SD_PIXELS[PALETTE_SIZE * 2], 0);
	Common::fill(&_colorTable[0], &_colorTable[PALETTE_EXT_BLOCK_SIZE], 0);
	Common::fill(&_palette[0], &_palette[PALETTE_EXT_BLOCK_SIZE], 0);
	Common::fill(&_oldPalette[0], &_oldPalette[PALETTE_EXT_BLOCK_SIZE], 0);
}

GraphicsManager::~GraphicsManager() {
	_vm->_globals.freeMemory(_vesaScreen);
	_vm->_globals.freeMemory(_vesaBuffer);
}

void GraphicsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;

	if (_vm->getIsDemo()) {
		if (_vm->getPlatform() == Common::kPlatformLinux)
		// CHECKME: Should be 0?
			MANU_SCROLL = true;
		else
			MANU_SCROLL = false;
		_scrollSpeed = 16;
	} else {
		MANU_SCROLL = false;
		_scrollSpeed = 32;
	}
}

void GraphicsManager::setGraphicalMode(int width, int height) {
	if (!SDL_MODEYES) {
		Graphics::PixelFormat pixelFormat16(2, 5, 6, 5, 0, 11, 5, 0, 0);
		initGraphics(width, height, true, &pixelFormat16);

		// Init surfaces
		_vesaScreen = _vm->_globals.allocMemory(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);
		_vesaBuffer = _vm->_globals.allocMemory(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

		_videoPtr = NULL;
		XSCREEN = width;
		YSCREEN = height;

		WinScan = width * 2; // Refactor me

		PAL_PIXELS = SD_PIXELS;
		_lineNbr = width;

		SDL_MODEYES = true;
	} else {
		error("Called SET_MODE multiple times");
	}
}

/**
 * (try to) Lock Screen
 */
void GraphicsManager::lockScreen() {
	if (!_skipVideoLockFl) {
		if (_lockCounter++ == 0)
			_videoPtr = g_system->lockScreen();
	}
}

/**
 * (try to) Unlock Screen
 */
void GraphicsManager::unlockScreen() {
	assert(_videoPtr);
	if (--_lockCounter == 0) {
		g_system->unlockScreen();
		_videoPtr = NULL;
	}
}

/**
 * Clear Screen
 */
void GraphicsManager::clearScreen() {
	assert(_videoPtr);
	_videoPtr->fillRect(Common::Rect(0, 0, XSCREEN, YSCREEN), 0);
}

/**
 * Load Image
 */
void GraphicsManager::loadImage(const Common::String &file) {
	Common::String filename	= Common::String::format("%s.PCX", file.c_str());
	loadScreen(filename);
	initColorTable(165, 170, _palette);
}

/**
 * Load VGA Image
 */
void GraphicsManager::loadVgaImage(const Common::String &file) {
	SCANLINE(SCREEN_WIDTH);
	lockScreen();
	clearScreen();
	unlockScreen();
	_vm->_fileManager.constructFilename(_vm->_globals.HOPIMAGE, file);
	A_PCX320(_vesaScreen, _vm->_globals._curFilename, _palette);
	memcpy(_vesaBuffer, _vesaScreen, 64000);
	SCANLINE(320);
	max_x = 320;

	lockScreen();
	copy16bFromSurfaceScaleX2(_vesaBuffer);
	unlockScreen();

	fadeInBreakout();
}

/**
 * Load Screen
 */
void GraphicsManager::loadScreen(const Common::String &file) {
	Common::File f;

	bool flag = true;
	if (_vm->_fileManager.searchCat(file, 6) == g_PTRNUL) {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPIMAGE, file);
		if (!f.open(_vm->_globals._curFilename))
			error("loadScreen - %s", file.c_str());

		f.seek(0, SEEK_END);
		f.close();
		flag = false;
	}

	scrollScreen(0);
	A_PCX640_480(_vesaScreen, file, _palette, flag);

	SCROLL = 0;
	OLD_SCROLL = 0;
	clearPalette();

	if (!_largeScreenFl) {
		SCANLINE(SCREEN_WIDTH);
		max_x = SCREEN_WIDTH;
		lockScreen();
		clearScreen();
		m_scroll16(_vesaScreen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		unlockScreen();
	} else {
		SCANLINE(SCREEN_WIDTH * 2);
		max_x = SCREEN_WIDTH * 2;
		lockScreen();
		clearScreen();
		unlockScreen();

		if (MANU_SCROLL) {
			lockScreen();
			m_scroll16(_vesaScreen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			unlockScreen();
		}
	}

	memcpy(_vesaBuffer, _vesaScreen, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);
}

void GraphicsManager::initColorTable(int minIndex, int maxIndex, byte *palette) {
	for (int idx = 0; idx < 256; ++idx)
		_colorTable[idx] = idx;

	Trans_bloc(_colorTable, palette, 256, minIndex, maxIndex);

	for (int idx = 0; idx < 256; ++idx) {
		byte v = _colorTable[idx];
		if (v > 27)
			_colorTable[idx] = 0;
		if (!v)
			_colorTable[idx] = 0;
	}

	_colorTable[0] = 1;
}

/**
 * Scroll Screen
 */
void GraphicsManager::scrollScreen(int amount) {
	int result = CLIP(amount, 0, SCREEN_WIDTH);
	_vm->_eventsManager._startPos.x = result;
	_scrollOffset = result;
	SCROLL = result;
}

void GraphicsManager::Trans_bloc(byte *destP, const byte *srcP, int count, int minThreshold, int maxThreshold) {
	int palIndex;
	int srcOffset;
	int col1, col2;

	byte *destPosP = destP;
	for (int idx = 0; idx < count; ++idx) {
		palIndex = *destPosP;
		srcOffset = 3 * palIndex;
		col1 = srcP[srcOffset] + srcP[srcOffset + 1] + srcP[srcOffset + 2];

		for (int idx2 = 0; idx2 < 38; ++idx2) {
			srcOffset = 3 * idx2;
			col2 = srcP[srcOffset] + srcP[srcOffset + 1] + srcP[srcOffset + 2];

			col2 += minThreshold;
			if (col2 < col1)
				continue;

			col2 -= maxThreshold;
			if (col2 > col1)
				continue;

			*destPosP = (idx2 == 0) ? 1 : idx2;
			break;
		}
		destPosP++;
	}
}

void GraphicsManager::Trans_bloc2(byte *surface, byte *col, int size) {
	byte dataVal;

	byte *dataP = surface;
	for (int count = size - 1; count; count--){
		dataVal = *dataP;
		*dataP = col[dataVal];
		dataP++;
	}
}

void GraphicsManager::A_PCX640_480(byte *surface, const Common::String &file, byte *palette, bool typeFlag) {
	Common::File f;
	Graphics::PCXDecoder pcxDecoder;

	// Clear the passed surface
	memset(surface, 0, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

	if (typeFlag) {
		// Load PCX from within the PIC resource
		_vm->_fileManager.constructFilename(_vm->_globals.HOPIMAGE, "PIC.RES");
		if (!f.open(_vm->_globals._curFilename))
			error("(nom)Erreur en cours de lecture.");
		f.seek(_vm->_globals._catalogPos);

	} else {
		// Load stand alone PCX file
		_vm->_fileManager.constructFilename(_vm->_globals.HOPIMAGE, file);
		if (!f.open(_vm->_globals._curFilename))
		  error("(nom)Erreur en cours de lecture.");
	}

	// Decode the PCX
	if (!pcxDecoder.loadStream(f))
		error("Error decoding PCX %s", file.c_str());

	const Graphics::Surface *s = pcxDecoder.getSurface();

	// Copy out the dimensions and pixels of the decoded surface
	_largeScreenFl = s->w > SCREEN_WIDTH;
	Common::copy((byte *)s->pixels, (byte *)s->pixels + (s->pitch * s->h), surface);

	// Copy out the palette
	const byte *palSrc = pcxDecoder.getPalette();
	Common::copy((const byte *)palSrc, (const byte *)palSrc + PALETTE_BLOCK_SIZE, palette);

	f.close();
}

void GraphicsManager::A_PCX320(byte *surface, const Common::String &file, byte *palette) {
	size_t filesize;
	int v4;
	size_t v5;
	size_t v7;
	byte v9;
	int v10;
	char v12;
	int v15;
	int v16;
	int32 v17;
	byte *ptr;
	Common::File f;

	if (!f.open(file))
		error("File not found - %s", file.c_str());

	filesize = f.size();

	f.read(surface, 128);
	v4 = filesize - 896;
	ptr = _vm->_globals.allocMemory(65024);
	if (v4 >= 64000) {
		v15 = v4 / 64000 + 1;
		v17 = 64000 * (v4 / 64000) - v4;
		if (v17 < 0)
			v17 = -v17;
		f.read(ptr, 64000);
		v5 = 64000;
	} else {
		v15 = 1;
		v17 = v4;
		f.read(ptr, v4);
		v5 = v4;
	}
	v16 = v15 - 1;
	v7 = 0;
	for (int i = 0; i < 64000; i++) {
		if (v7 == v5) {
			v7 = 0;
			--v16;
			v5 = 64000;
			if (!v16)
				v5 = v17;
			f.read(ptr, v5);
		}
		v9 = ptr[v7++];
		if (v9 > 192) {
			v10 = v9 - 192;
			if (v7 == v5) {
				v7 = 0;
				--v16;
				v5 = 64000;
				if (v16 == 1)
					v5 = v17;
				f.read(ptr, v5);
			}
			v12 = ptr[v7++];
			do {
				surface[i++] = v12;
				--v10;
			} while (v10);
			--i;
		} else {
			surface[i] = v9;
		}
	}

	f.seek(filesize - 768);
	f.read(palette, 768);
	f.close();

	_vm->_globals.freeMemory(ptr);
}

// Clear Palette
void GraphicsManager::clearPalette() {
	SD_PIXELS[0] = 0;
}

void GraphicsManager::SCANLINE(int pitch) {
	_lineNbr = _lineNbr2 = pitch;
}

/**
 * Copies data from a 8-bit palette surface into the 16-bit screen
 */
void GraphicsManager::m_scroll16(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	lockScreen();

	assert(_videoPtr);
	const byte *srcP = xs + _lineNbr2 * ys + surface;
	byte *destP = (byte *)_videoPtr->pixels + destX * 2 + WinScan * destY;

	for (int yp = 0; yp < height; ++yp) {
		// Copy over the line, using the source pixels as lookups into the pixels palette
		const byte *lineSrcP = srcP;
		byte *lineDestP = destP;

		for (int xp = 0; xp < width; ++xp) {
			lineDestP[0] = PAL_PIXELS[lineSrcP[0] * 2];
			lineDestP[1] = PAL_PIXELS[(lineSrcP[0] * 2) + 1];
			lineDestP += 2;
			lineSrcP++;
		}
		// Move to the start of the next line
		srcP += _lineNbr2;
		destP += WinScan;
	}

	unlockScreen();
}

// TODO: See if PAL_PIXELS can be converted to a uint16 array
void GraphicsManager::m_scroll16A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	const byte *srcP;
	byte *destP;
	int yNext;
	int xCtr;
	const byte *palette;
	int yCtr;
	const byte *srcCopyP;
	byte *destCopyP;

	assert(_videoPtr);
	srcP = xs + _lineNbr2 * ys + surface;
	destP = (byte *)_videoPtr->pixels + destX + destX + WinScan * destY;
	yNext = height;
	Agr_x = 0;
	Agr_y = 0;
	Agr_Flag_y = 0;

	do {
		for (;;) {
			destCopyP = destP;
			srcCopyP = srcP;
			xCtr = width;
			yCtr = yNext;
			palette = PAL_PIXELS;
			Agr_x = 0;

			do {
				destP[0] = palette[2 * srcP[0]];
				destP[1] = palette[(2 * srcP[0]) + 1];
				destP += 2;
				if ((unsigned int)Agr_x >= 100) {
					Agr_x -= 100;
					destP[0] = palette[2 * srcP[0]];
					destP[1] = palette[(2 * srcP[0]) + 1];
					destP += 2;
				}
				++srcP;
				--xCtr;
			} while (xCtr);

			yNext = yCtr;
			srcP = srcCopyP;
			destP = WinScan + destCopyP;
			if (Agr_Flag_y == 1)
				break;

			if ((unsigned int)Agr_y < 100)
				break;

			Agr_y -= 100;
			Agr_Flag_y = 1;
		}

		Agr_Flag_y = 0;
		srcP = _lineNbr2 + srcCopyP;
		yNext = yCtr - 1;
	} while (yCtr != 1);
}

void GraphicsManager::Copy_Vga16(const byte *surface, int xp, int yp, int width, int height, int destX, int destY) {
	int yCount;
	int xCount;
	int xCtr;
	const byte *palette;
	int savedXCount;
	byte *loopDestP;
	const byte *loopSrcP;
	int yCtr;

	assert(_videoPtr);
	const byte *srcP = surface + xp + 320 * yp;
	byte *destP = (byte *)_videoPtr->pixels + 30 * WinScan + destX + destX + destX + destX + WinScan * 2 * destY;
	yCount = height;
	xCount = width;

	do {
		yCtr = yCount;
		xCtr = xCount;
		loopSrcP = srcP;
		loopDestP = destP;
		savedXCount = xCount;
		palette = PAL_PIXELS;

		do {
			destP[0] = destP[2] = destP[WinScan] = destP[WinScan + 2] = palette[2 * srcP[0]];
			destP[1] = destP[3] = destP[WinScan + 1] = destP[WinScan + 3] = palette[(2 * srcP[0]) + 1];
			++srcP;
			destP += 4;
			--xCtr;
		} while (xCtr);

		xCount = savedXCount;
		destP = loopDestP + WinScan * 2;
		srcP = loopSrcP + 320;
		yCount = yCtr - 1;
	} while (yCtr != 1);
}

/** 
 * Fade in. the step number is determine by parameter.
 */
void GraphicsManager::fadeIn(const byte *palette, int step, const byte *surface) {
	byte palData2[PALETTE_BLOCK_SIZE];
	int fadeStep;
	if (step > 1)
		fadeStep = step;
	else
		fadeStep = 2;
	// Initialize temporary palette
	Common::fill(&palData2[0], &palData2[PALETTE_BLOCK_SIZE], 0);

	// Set current palette to black
	setPaletteVGA256(palData2);

	// Loop through fading in the palette
	for (int fadeIndex = 0; fadeIndex < fadeStep; ++fadeIndex) {
		for (int palOffset = 0; palOffset < PALETTE_BLOCK_SIZE; palOffset += 3) {
			palData2[palOffset + 0] = fadeIndex * palette[palOffset + 0] / (fadeStep - 1);
			palData2[palOffset + 1] = fadeIndex * palette[palOffset + 1] / (fadeStep - 1);
			palData2[palOffset + 2] = fadeIndex * palette[palOffset + 2] / (fadeStep - 1);
		}

		// Set the transition palette and refresh the screen
		setPaletteVGA256(palData2);
		m_scroll16(surface, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		DD_VBL();

		// Added a delay in order to see the fading
		_vm->_eventsManager.delay(20);
	}

	// Set the final palette
	setPaletteVGA256(palette);

	// Refresh the screen
	m_scroll16(surface, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	DD_VBL();
}

/** 
 * Fade out. the step number is determine by parameter.
 */
void GraphicsManager::fadeOut(const byte *palette, int step, const byte *surface) {
	byte palData[PALETTE_BLOCK_SIZE];
	int fadeStep;
	if (step > 1)
		fadeStep = step;
	else
		fadeStep = 2;

	if (palette) {
		for (int fadeIndex = 0; fadeIndex < fadeStep; fadeIndex++) {
			for (int palOffset = 0; palOffset < PALETTE_BLOCK_SIZE; palOffset += 3) {
				palData[palOffset + 0] = (fadeStep - fadeIndex - 1) * palette[palOffset + 0] / (fadeStep - 1);
				palData[palOffset + 1] = (fadeStep - fadeIndex - 1) * palette[palOffset + 1] / (fadeStep - 1);
				palData[palOffset + 2] = (fadeStep - fadeIndex - 1) * palette[palOffset + 2] / (fadeStep - 1);
			}

			setPaletteVGA256(palData);
			m_scroll16(surface, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			DD_VBL();

			_vm->_eventsManager.delay(20);
		}
	}

	// No initial palette, or end of fading
	for (int i = 0; i < PALETTE_BLOCK_SIZE; i++)
		palData[i] = 0;

	setPaletteVGA256(palData);
	m_scroll16(surface, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	return DD_VBL();
}

/** 
 * Short fade in. The step number is 1, the default step number is also set to 1.
 */
void GraphicsManager::fadeInShort() {
	_fadeDefaultSpeed = 1;
	fadeIn(_palette, 1, (const byte *)_vesaBuffer);
}

/** 
 * Short fade out. The step number is 1, the default step number is also set to 1.
 */
void GraphicsManager::fadeOutShort() {
	_fadeDefaultSpeed = 1;
	fadeOut(_palette, 1, (const byte *)_vesaBuffer);
}

/** 
 * Long fade in. The step number is 20, the default step number is also set to 15.
 */
void GraphicsManager::fadeInLong() {
	_fadeDefaultSpeed = 15;
	fadeIn(_palette, 20, (const byte *)_vesaBuffer);
}

/** 
 * Long fade out. The step number is 20, the default step number is also set to 15.
 */
void GraphicsManager::fadeOutLong() {
	_fadeDefaultSpeed = 15;
	fadeOut(_palette, 20, (const byte *)_vesaBuffer);
}

/** 
 * Fade in. The step number used is the default step number.
 */
void GraphicsManager::fadeInDefaultLength(const byte *surface) {
	assert(surface);
	fadeIn(_palette, _fadeDefaultSpeed, surface);
}

/** 
 * Fade out. The step number used is the default step number.
 */
void GraphicsManager::fadeOutDefaultLength(const byte *surface) {
	assert(surface);
	fadeOut(_palette, _fadeDefaultSpeed, surface);
}

/** 
 * Fade in used by for the breakout mini-game
 */
void GraphicsManager::fadeInBreakout() {
	setPaletteVGA256(_palette);
	lockScreen();
	copy16bFromSurfaceScaleX2(_vesaBuffer);
	unlockScreen();
	DD_VBL();
}

/** 
 * Fade out used by for the breakout mini-game
 */
void GraphicsManager::fateOutBreakout() {
	byte palette[PALETTE_EXT_BLOCK_SIZE];

	memset(palette, 0, PALETTE_EXT_BLOCK_SIZE);
	setPaletteVGA256(palette);

	lockScreen();
	copy16bFromSurfaceScaleX2(_vesaBuffer);
	unlockScreen();
	DD_VBL();
}

void GraphicsManager::setPaletteVGA256(const byte *palette) {
	changePalette(palette);
}

void GraphicsManager::setPaletteVGA256WithRefresh(const byte *palette, const byte *surface) {
	changePalette(palette);
	m_scroll16(surface, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	DD_VBL();
}

void GraphicsManager::SETCOLOR3(int palIndex, int r, int g, int b) {
	int palOffset = 3 * palIndex;
	_palette[palOffset] = 255 * r / 100;
	_palette[palOffset + 1] = 255 * g / 100;
	_palette[palOffset + 2] = 255 * b / 100;
}

void GraphicsManager::SETCOLOR4(int palIndex, int r, int g, int b) {
	int rv = 255 * r / 100;
	int gv = 255 * g / 100;
	int bv = 255 * b / 100;

	int palOffset = 3 * palIndex;
	_palette[palOffset] = rv;
	_palette[palOffset + 1] = gv;
	_palette[palOffset + 2] = bv;

	WRITE_LE_UINT16(&SD_PIXELS[2 * palIndex], mapRGB(rv, gv, bv));
}

void GraphicsManager::changePalette(const byte *palette) {
	const byte *srcP = &palette[0];
	for (int idx = 0; idx < PALETTE_SIZE; ++idx, srcP += 3) {
		WRITE_LE_UINT16(&SD_PIXELS[2 * idx], mapRGB(srcP[0], srcP[1], srcP[2]));
	}
}

uint16 GraphicsManager::mapRGB(byte r, byte g, byte b) {
	Graphics::PixelFormat format = g_system->getScreenFormat();

	return (r >> format.rLoss) << format.rShift
			| (g >> format.gLoss) << format.gShift
			| (b >> format.bLoss) << format.bShift;
}

void GraphicsManager::DD_VBL() {
	// TODO: Is this okay here?
	g_system->updateScreen();
}

void GraphicsManager::Copy_WinScan_Vbe3(const byte *srcData, byte *destSurface) {
	int rleValue;
	int destOffset;
	const byte *srcP;
	byte srcByte;
	byte destLen1;
	byte *destSlice1P;
	byte destLen2;
	byte *destSlice2P;

	rleValue = 0;
	destOffset = 0;
	srcP = srcData;
	for (;;) {
		srcByte = srcP[0];
		if (srcByte == kByteStop)
			return;
		if (srcByte == 211) {
			destLen1 = srcP[1];
			rleValue = srcP[2];
			destSlice1P = destOffset + destSurface;
			destOffset += destLen1;
			memset(destSlice1P, rleValue, destLen1);
			srcP += 3;
		} else if (srcByte < 222) {
			if (srcByte > 211) {
				destLen2 = (byte)(srcP[0] + 45);
				rleValue = srcP[1];
				destSlice2P = destOffset + destSurface;
				destOffset += destLen2;
				memset(destSlice2P, rleValue, destLen2);
				srcP += 2;
			} else {
				destSurface[destOffset] = srcByte;
				++srcP;
				++destOffset;
			}
		} else if (srcByte < kSetOffset) {
			destOffset += (byte)(srcP[0] + 35);
			srcP++;
		} else if (srcByte == k8bVal) {
			destOffset += srcP[1];
			srcP += 2;
		} else if (srcByte == k16bVal) {
			destOffset += READ_LE_UINT16(srcP + 1);
			srcP += 3;
		} else {
			destOffset += READ_LE_UINT32(srcP + 1);
			srcP += 5;
		}
	}
}

void GraphicsManager::Copy_Video_Vbe16(const byte *srcData) {
	const byte *srcP = srcData;
	int destOffset = 0;
	assert(_videoPtr);

	for (;;) {
		byte srcByte = srcP[0];
		if (srcByte >= 222) {
			if (srcByte == kByteStop)
				return;
			if (srcByte < kSetOffset) {
				destOffset += srcByte - 221;
				srcByte = *++srcP;
			} else if (srcByte == k8bVal) {
				destOffset += srcP[1];
				srcByte = srcP[2];
				srcP += 2;
			} else if (srcByte == k16bVal) {
				destOffset += READ_LE_UINT16(srcP + 1);
				srcByte = srcP[3];
				srcP += 3;
			} else {
				destOffset += READ_LE_UINT32(srcP + 1);
				srcByte = srcP[5];
				srcP += 5;
			}
		}

		if (destOffset > SCREEN_WIDTH * SCREEN_HEIGHT) {
			warning("HACK: Stopping anim, out of bounds - 0x%x %d", srcByte, destOffset);
			return;
		}

		if (srcByte > 210) {
			if (srcByte == 211) {
				int pixelCount = srcP[1];
				int pixelIndex = srcP[2];
				byte *destP = (byte *)_videoPtr->pixels + destOffset * 2;
				destOffset += pixelCount;

				while (pixelCount--) {
					destP[0] = PAL_PIXELS[2 * pixelIndex];
					destP[1] = PAL_PIXELS[(2 * pixelIndex) + 1];
					destP += 2;
				}

				srcP += 3;
			} else {
				int pixelCount = srcByte - 211;
				int pixelIndex = srcP[1];
				byte *destP = (byte *)_videoPtr->pixels + destOffset * 2;
				destOffset += pixelCount;

				while (pixelCount--) {
					destP[0] = PAL_PIXELS[2 * pixelIndex];
					destP[1] = PAL_PIXELS[(2 * pixelIndex) + 1];
					destP += 2;
				}

				srcP += 2;
			}
		} else {
			byte *destP = (byte *)_videoPtr->pixels + destOffset * 2;
			destP[0] = PAL_PIXELS[2 * srcByte];
			destP[1] = PAL_PIXELS[(2 * srcByte) + 1];
			++srcP;
			++destOffset;
		}
	}
}

void GraphicsManager::Copy_Video_Vbe16a(const byte *srcData) {
	byte srcByte;
	int destOffset = 0;
	const byte *srcP = srcData;

	for (;;) {
		srcByte = srcP[0];
		if (srcByte == kByteStop)
			return;
		if (srcP[0] > kByteStop) {
			if (srcByte == k8bVal) {
				destOffset += srcP[1];
				srcByte = srcP[2];
				srcP += 2;
			} else if (srcByte == k16bVal) {
				destOffset += READ_LE_UINT16(srcP + 1);
				srcByte = srcP[3];
				srcP += 3;
			} else {
				destOffset += READ_LE_UINT32(srcP + 1);
				srcByte = srcP[5];
				srcP += 5;
			}
		}

		WRITE_LE_UINT16((byte *)_videoPtr->pixels + destOffset * 2, READ_LE_UINT16(PAL_PIXELS + 2 * srcByte));
		++srcP;
		++destOffset;
	}
}

void GraphicsManager::Capture_Mem(const byte *srcSurface, byte *destSurface, int xs, int ys, int width, int height) {
	const byte *srcP;
	byte *destP;
	int rowCount;
	int i;
	int rowCount2;

	// TODO: This code in the original is potentially dangerous, as it doesn't clip the area to within
	// the screen, and so thus can read areas outside of the allocated surface buffer
	srcP = xs + _lineNbr2 * ys + srcSurface;
	destP = destSurface;
	rowCount = height;
	do {
		rowCount2 = rowCount;
		if (width & 1) {
			memcpy(destP, srcP, width);
			srcP += width;
			destP += width;
		} else if (width & 2) {
			for (i = width >> 1; i; --i) {
				destP[0] = srcP[0];
				destP[1] = srcP[1];
				srcP += 2;
				destP += 2;
			}
		} else {
			memcpy(destP, srcP, 4 * (width >> 2));
			srcP += 4 * (width >> 2);
			destP += 4 * (width >> 2);
		}
		srcP = _lineNbr2 + srcP - width;
		rowCount = rowCount2 - 1;
	} while (rowCount2 != 1);
}

void GraphicsManager::Sprite_Vesa(byte *surface, const byte *spriteData, int xp, int yp, int spriteIndex) {
	// Get a pointer to the start of the desired sprite
	const byte *spriteP = spriteData + 3;
	for (int i = spriteIndex; i; --i)
		spriteP += READ_LE_UINT32(spriteP) + 16;

	clip_x = 0;
	clip_y = 0;
	clip_flag = false;

	spriteP += 4;
	int width = READ_LE_UINT16(spriteP);
	spriteP += 2;
	int height = READ_LE_UINT16(spriteP);

	// Clip X
	clip_x1 = width;
	if ((xp + width) <= (min_x + 300))
		return;
	if (xp < (min_x + 300)) {
		clip_x = min_x + 300 - xp;
		clip_flag = true;
	}

	// Clip Y
	// TODO: This is weird, but it's that way in the original. Original game bug?
	if ((yp + height) <= height)
		return;
	if (yp < (min_y + 300)) {
		clip_y = min_y + 300 - yp;
		clip_flag = true;
	}

	// Clip X1
	if (xp >= (max_x + 300))
		return;
	if ((xp + width) > (max_x + 300)) {
		int xAmount = width + 10 - (xp + width - (max_x + 300));
		if (xAmount <= 10)
			return;

		clip_x1 = xAmount - 10;
		clip_flag = true;
	}

	// Clip Y1
	if (yp >= (max_y + 300))
		return;
	if ((yp + height) > (max_y + 300)) {
		int yAmount = height + 10 - (yp + height - (max_y + 300));
		if (yAmount <= 10)
			return;

		clip_y1 = yAmount - 10;
		clip_flag = true;
	}

	// Sprite display

	// Set up source
	spriteP += 6;
	int srcOffset = READ_LE_UINT16(spriteP);
	spriteP += 4;
	const byte *srcP = spriteP;
	spriteP += srcOffset;

	// Set up surface destination
	byte *destP = surface + (yp - 300) * _lineNbr2 + (xp - 300);

	// Handling for clipped versus non-clipped
	if (clip_flag) {
		// Clipped version
		for (int yc = 0; yc < height; ++yc, destP += _lineNbr2) {
			byte *tempDestP = destP;
			byte byteVal;
			int xc = 0;

			while ((byteVal = *srcP) != 253) {
				++srcP;
				width = READ_LE_UINT16(srcP);
				srcP += 2;

				if (byteVal == 254) {
					// Copy pixel range
					for (int xv = 0; xv < width; ++xv, ++xc, ++spriteP, ++tempDestP) {
						if (clip_y == 0 && xc >= clip_x && xc < clip_x1)
							*tempDestP = *spriteP;
					}
				} else {
					// Skip over bytes
					tempDestP += width;
					xc += width;
				}
			}

			if (clip_y > 0)
				--clip_y;
			srcP += 3;
		}
	} else {
		// Non-clipped
		for (int yc = 0; yc < height; ++yc, destP += _lineNbr2) {
			byte *tempDestP = destP;
			byte byteVal;

			while ((byteVal = *srcP) != 253) {
				++srcP;
				width = READ_LE_UINT16(srcP);
				srcP += 2;

				if (byteVal == 254) {
					// Copy pixel range
					Common::copy(spriteP, spriteP + width, tempDestP);
					spriteP += width;
				}

				tempDestP += width;
			}

			// Skip over control byte and width
			srcP += 3;
		}
	}
}

void GraphicsManager::FIN_VISU() {
	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_OFF(idx);
	}

	_vm->_eventsManager.VBL();
	_vm->_eventsManager.VBL();

	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_ZERO(idx);
	}

	for (int idx = 1; idx <= 29; ++idx) {
		_vm->_globals._lockedAnims[idx]._enableFl = false;
	}

	for (int idx = 1; idx <= 20; ++idx) {
		_vm->_globals.Bqe_Anim[idx]._enabledFl = false;
	}
}

void GraphicsManager::VISU_ALL() {
	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_VISU(idx);
	}
}

void GraphicsManager::RESET_SEGMENT_VESA() {
	if (_vm->_globals.NBBLOC > 0) {
		for (int idx = 0; idx != _vm->_globals.NBBLOC; idx++)
			_vm->_globals.BLOC[idx]._activeFl = false;

		_vm->_globals.NBBLOC = 0;
	}
}

// Add VESA Segment
void GraphicsManager::addVesaSegment(int x1, int y1, int x2, int y2) {
	int tempX;
	bool addFlag;

	tempX = x1;
	addFlag = true;
	if (x2 > max_x)
		x2 = max_x;
	if (y2 > max_y)
		y2 = max_y;
	if (x1 < min_x)
		tempX = min_x;
	if (y1 < min_y)
		y1 = min_y;

	if (_vm->_globals.NBBLOC > 1) {
		int16 blocIndex = 0;
		do {
			BlocItem &bloc = _vm->_globals.BLOC[blocIndex];

			if (bloc._activeFl && tempX >= bloc._x1 && x2 <= bloc._x2
					&& y1 >= bloc._y1 && y2 <= bloc._y2)
				addFlag = false;
			++blocIndex;
		} while (_vm->_globals.NBBLOC + 1 != blocIndex);
	}

	if (addFlag) {
		assert(_vm->_globals.NBBLOC < 250);
		BlocItem &bloc = _vm->_globals.BLOC[++_vm->_globals.NBBLOC];

		bloc._activeFl = true;
		bloc._x1 = tempX;
		bloc._x2 = x2;
		bloc._y1 = y1;
		bloc._y2 = y2;
	}
}

// Display VESA Segment
void GraphicsManager::displayVesaSegment() {
	if (_vm->_globals.NBBLOC == 0)
		return;

	lockScreen();

	for (int idx = 1; idx <= _vm->_globals.NBBLOC; ++idx) {
		BlocItem &bloc = _vm->_globals.BLOC[idx];
		Common::Rect &dstRect = dstrect[idx - 1];
		if (!bloc._activeFl)
			continue;

		if (_vm->_eventsManager._breakoutFl) {
			Copy_Vga16(_vesaBuffer, bloc._x1, bloc._y1, bloc._x2 - bloc._x1, bloc._y2 - bloc._y1, bloc._x1, bloc._y1);
			dstRect.left = bloc._x1 * 2;
			dstRect.top = bloc._y1 * 2 + 30;
			dstRect.setWidth((bloc._x2 - bloc._x1) * 2);
			dstRect.setHeight((bloc._y2 - bloc._y1) * 2);
		} else if (bloc._x2 > _vm->_eventsManager._startPos.x && bloc._x1 < (_vm->_eventsManager._startPos.x + SCREEN_WIDTH)) {
			if (bloc._x1 < _vm->_eventsManager._startPos.x)
				bloc._x1 = _vm->_eventsManager._startPos.x;
			if (bloc._x2 > (_vm->_eventsManager._startPos.x + SCREEN_WIDTH))
				bloc._x2 = _vm->_eventsManager._startPos.x + SCREEN_WIDTH;

			// WORKAROUND: Original didn't lock the screen for access
			lockScreen();
			m_scroll16(_vesaBuffer, bloc._x1, bloc._y1, bloc._x2 - bloc._x1, bloc._y2 - bloc._y1, bloc._x1 - _vm->_eventsManager._startPos.x, bloc._y1);

			dstRect.left = bloc._x1 - _vm->_eventsManager._startPos.x;
			dstRect.top = bloc._y1;
			dstRect.setWidth(bloc._x2 - bloc._x1);
			dstRect.setHeight(bloc._y2 - bloc._y1);

			unlockScreen();
		}

		_vm->_globals.BLOC[idx]._activeFl = false;
	}

	_vm->_globals.NBBLOC = 0;
	unlockScreen();
}

void GraphicsManager::AFFICHE_SPEEDVGA(const byte *objectData, int xp, int yp, int idx) {
	int height, width;

	width = _vm->_objectsManager.getWidth(objectData, idx);
	height = _vm->_objectsManager.getHeight(objectData, idx);
	if (*objectData == 78) {
		Affiche_Perfect(_vesaScreen, objectData, xp + 300, yp + 300, idx, 0, 0, 0);
		Affiche_Perfect(_vesaBuffer, objectData, xp + 300, yp + 300, idx, 0, 0, 0);
	} else {
		Sprite_Vesa(_vesaBuffer, objectData, xp + 300, yp + 300, idx);
		Sprite_Vesa(_vesaScreen, objectData, xp + 300, yp + 300, idx);
	}
	if (!_vm->_globals.NO_VISU)
		addVesaSegment(xp, yp, xp + width, yp + height);
}

/**
 * Copy from surface to video buffer, scale 2x.
 */
void GraphicsManager::copy16bFromSurfaceScaleX2(const byte *surface) {
	byte *palPtr;
	int curPixel;

	assert(_videoPtr);
	const byte *curSurface = surface;
	byte *destPtr = 30 * WinScan + (byte *)_videoPtr->pixels;
	for (int y = 200; y; y--) {
		byte *oldDestPtr = destPtr;
		for (int x = 320; x; x--) {
			curPixel = 2 * *curSurface;
			palPtr = PAL_PIXELS + curPixel;
			destPtr[0] = destPtr[2] = destPtr[WinScan] = destPtr[WinScan + 2] = palPtr[0];
			destPtr[1] = destPtr[3] = destPtr[WinScan + 1] = destPtr[WinScan + 3] = palPtr[1];
			++curSurface;
			destPtr += 4;
		}
		destPtr = WinScan * 2 + oldDestPtr;
	}
}

void GraphicsManager::Restore_Mem(byte *destSurface, const byte *src, int xp, int yp, int width, int height) {
	byte *destP;
	int yNext;
	const byte *srcP;
	int i;
	int yCtr;

	destP = xp + _lineNbr2 * yp + destSurface;
	yNext = height;
	srcP = src;
	do {
		yCtr = yNext;
		if (width & 1) {
			memcpy(destP, srcP, width);
			srcP += width;
			destP += width;
		} else if (width & 2) {
			for (i = width >> 1; i; --i) {
				destP[0] = srcP[0];
				destP[1] = srcP[1];
				srcP += 2;
				destP += 2;
			}
		} else {
			memcpy(destP, srcP, 4 * (width >> 2));
			srcP += 4 * (width >> 2);
			destP += 4 * (width >> 2);
		}
		destP = _lineNbr2 + destP - width;
		yNext = yCtr - 1;
	} while (yCtr != 1);
}

/**
 * Compute the value of a parameter plus a given percentage
 */
int GraphicsManager::zoomIn( int v, int percentage ) {
	if (v)
		v += percentage * (long int)v / 100;

	return v;
}

/**
 * Compute the value of a parameter minus a given percentage
 */
int GraphicsManager::zoomOut(int v, int percentage) {
	if (v)
		v -= percentage * (long int)v / 100;

	return v;
}

// Display 'Perfect?'
void GraphicsManager::Affiche_Perfect(byte *surface, const byte *srcData, int xp300, int yp300, int frameIndex, int zoom1, int zoom2, int modeFlag) {
	const byte *spriteStartP;
	int i;
	const byte *spriteSizeP;
	const byte *spritePixelsP;
	byte *dest1P;
	byte *dest2P;
	byte *v29;
	byte *v40;
	const byte *v45;
	const byte *v46;

	spriteStartP = srcData + 3;
	for (i = frameIndex; i; --i)
		spriteStartP += READ_LE_UINT32(spriteStartP) + 16;

	spriteSizeP = spriteStartP + 4;
	int spriteWidth = (int16)READ_LE_UINT16(spriteSizeP);
	spriteSizeP += 2;
	int spriteHeight2 = (int16)READ_LE_UINT16(spriteSizeP);
	int spriteHeight1 = spriteHeight2;
	spritePixelsP = spriteSizeP + 10;
	clip_x = 0;
	clip_y = 0;
	clip_x1 = 0;
	clip_y1 = 0;
	if ((xp300 <= min_x) || (yp300 <= min_y) || (xp300 >= max_x + 300) || 	(yp300 >= max_y + 300))
		return;

	if ((uint16)xp300 < (uint16)(min_x + 300))
		clip_x = min_x + 300 - xp300;

	if ((uint16)yp300 < (uint16)(min_y + 300))
		clip_y = min_y + 300 - yp300;

	clip_x1 = max_x + 300 - xp300;
	clip_y1 = max_y + 300 - yp300;
	dest1P = xp300 + _lineNbr2 * (yp300 - 300) - 300 + surface;
	if (zoom2) {
		Compteur_y = 0;
		Agr_x = 0;
		Agr_y = 0;
		Agr_Flag_y = 0;
		Agr_Flag_x = 0;
		_width = spriteWidth;
		int v20 = zoomIn(spriteWidth, zoom2);
		int v22 = zoomIn(spriteHeight1, zoom2);
		if (modeFlag) {
			v29 = v20 + dest1P;
			if (clip_y) {
				if ((uint16)clip_y >= v22)
					return;
				int v30 = 0;
				while (zoomIn(v30 + 1, zoom2) < (uint16)clip_y)
					;
				spritePixelsP += _width * v30;
				v29 += _lineNbr2 * (uint16)clip_y;
				v22 = v22 - (uint16)clip_y;
			}
			if (v22 > (uint16)clip_y1)
				v22 = (uint16)clip_y1;
			if (clip_x) {
				if ((uint16)clip_x >= v20)
					return;
				v20 -= (uint16)clip_x;
			}
			if (v20 > (uint16)clip_x1) {
				int v32 = v20 - (uint16)clip_x1;
				v29 -= v32;
				int v62 = v22;
				int v33 = 0;
				while (zoomIn(v33 + 1, zoom2) < v32)
					;
				int v34 = v33;
				v22 = v62;
				spritePixelsP += v34;
				v20 = (uint16)clip_x1;
			}
			int v63;
			do {
				for (;;) {
					v63 = v22;
					byte *v53 = v29;
					v46 = spritePixelsP;
					Agr_Flag_x = 0;
					Agr_x = 0;
					for (int v35 = v20; v35; v35--) {
						for (;;) {
							if (*spritePixelsP)
								*v29 = *spritePixelsP;
							--v29;
							++spritePixelsP;
							if (!Agr_Flag_x)
								Agr_x = zoom2 + Agr_x;
							if ((uint16)Agr_x < 100)
								break;
							Agr_x = Agr_x - 100;
							--spritePixelsP;
							Agr_Flag_x = 1;
							--v35;
							if (!v35)
								goto R_Aff_Zoom_Larg_Cont1;
						}
						Agr_Flag_x = 0;
					}
R_Aff_Zoom_Larg_Cont1:
					spritePixelsP = _width + v46;
					v29 = _lineNbr2 + v53;
					++Compteur_y;
					if (!Agr_Flag_y)
						Agr_y = zoom2 + Agr_y;
					if ((uint16)Agr_y < 100)
						break;
					Agr_y = Agr_y - 100;
					spritePixelsP = v46;
					Agr_Flag_y = 1;
					v22 = v63 - 1;
					if (v63 == 1)
						return;
				}
				Agr_Flag_y = 0;
				v22 = v63 - 1;
			} while (v63 != 1);
		} else {
			if (clip_y) {
				if ((uint16)clip_y >= v22)
					return;
				int v58 = v22;
				int v49 = v20;
				int v23 = 0;
				int v24 = (uint16)clip_y;
				while (zoomIn(v23 + 1, zoom2) < v24)
					;
				v20 = v49;
				spritePixelsP += _width * v23;
				dest1P += _lineNbr2 * (uint16)clip_y;
				v22 = v58 - (uint16)clip_y;
			}
			if (v22 > (uint16)clip_y1)
				v22 = (uint16)clip_y1;
			if (clip_x) {
				if ((uint16)clip_x >= v20)
					return;
				int v26 = 0;
				while (zoomIn(v26 + 1, zoom2) < (uint16)clip_x)
					;
				spritePixelsP += v26;
				dest1P += (uint16)clip_x;
				v20 = v20 - (uint16)clip_x;
			}
			if (v20 > (uint16)clip_x1)
				v20 = (uint16)clip_x1;

			int v60;
			do {
				for (;;) {
					v60 = v22;
					byte *v51 = dest1P;
					v45 = spritePixelsP;
					int v28 = v20;
					Agr_Flag_x = 0;
					Agr_x = 0;
					do {
						for (;;) {
							if (*spritePixelsP)
								*dest1P = *spritePixelsP;
							++dest1P;
							++spritePixelsP;
							if (!Agr_Flag_x)
								Agr_x = zoom2 + Agr_x;
							if ((uint16)Agr_x < 100)
								break;
							Agr_x = Agr_x - 100;
							--spritePixelsP;
							Agr_Flag_x = 1;
							--v28;
							if (!v28)
								goto Aff_Zoom_Larg_Cont1;
						}
						Agr_Flag_x = 0;
						--v28;
					} while (v28);
Aff_Zoom_Larg_Cont1:
					spritePixelsP = _width + v45;
					dest1P = _lineNbr2 + v51;
					if (!Agr_Flag_y)
						Agr_y = zoom2 + Agr_y;
					if ((uint16)Agr_y < 100)
						break;
					Agr_y = Agr_y - 100;
					spritePixelsP = v45;
					Agr_Flag_y = 1;
					v22 = v60 - 1;
					if (v60 == 1)
						return;
				}
				Agr_Flag_y = 0;
				v22 = v60 - 1;
			} while (v60 != 1);
		}
	} else if (zoom1) {
		Compteur_y = 0;
		Red_x = 0;
		Red_y = 0;
		_width = spriteWidth;
		Red = zoom1;
		if (zoom1 < 100) {
			int v37 = zoomOut(spriteWidth, Red);
			if (modeFlag) {
				v40 = v37 + dest1P;
				do {
					int v65 = spriteHeight2;
					byte *v55 = v40;
					Red_y = Red + Red_y;
					if ((uint16)Red_y < 100) {
						Red_x = 0;
						int v42 = v37;
						for (int v41 = _width; v41; v41--) {
							Red_x = Red + Red_x;
							if ((uint16)Red_x < 100) {
								if (v42 >= clip_x && v42 < clip_x1 && *spritePixelsP)
									*v40 = *spritePixelsP;
								--v40;
								++spritePixelsP;
								--v42;
							} else {
								Red_x = Red_x - 100;
								++spritePixelsP;
							}
						}
						spriteHeight2 = v65;
						v40 = _lineNbr2 + v55;
					} else {
						Red_y = Red_y - 100;
						spritePixelsP += _width;
					}
					--spriteHeight2;
				} while (spriteHeight2);
			} else {
				do {
					int v64 = spriteHeight2;
					byte *v54 = dest1P;
					Red_y = Red + Red_y;
					if ((uint16)Red_y < 100) {
						Red_x = 0;
						int v39 = 0;
						for (int v38 = _width; v38; v38--) {
							Red_x = Red + Red_x;
							if ((uint16)Red_x < 100) {
								if (v39 >= clip_x && v39 < clip_x1 && *spritePixelsP)
									*dest1P = *spritePixelsP;
								++dest1P;
								++spritePixelsP;
								++v39;
							} else {
								Red_x = Red_x - 100;
								++spritePixelsP;
							}
						}
						spriteHeight2 = v64;
						dest1P = _lineNbr2 + v54;
					} else {
						Red_y = Red_y - 100;
						spritePixelsP += _width;
					}
					--spriteHeight2;
				} while (spriteHeight2);
			}
		}
	} else {
		_width = spriteWidth;
		Compteur_y = 0;
		if (modeFlag) {
			dest2P = spriteWidth + dest1P;
			spec_largeur = spriteWidth;
			if (clip_y) {
				if ((uint16)clip_y >= (unsigned int)spriteHeight1)
					return;
				spritePixelsP += spriteWidth * (uint16)clip_y;
				dest2P += _lineNbr2 * (uint16)clip_y;
				spriteHeight1 -= (uint16)clip_y;
			}
			int xLeft = (uint16)clip_y1;
			if (spriteHeight1 > clip_y1)
				spriteHeight1 = clip_y1;
			xLeft = clip_x;
			if (clip_x) {
				if (xLeft >= spriteWidth)
					return;
				spriteWidth -= xLeft;
			}
			if (spriteWidth > (uint16)clip_x1) {
				int clippedWidth = spriteWidth - (uint16)clip_x1;
				spritePixelsP += clippedWidth;
				dest2P -= clippedWidth;
				spriteWidth = (uint16)clip_x1;
			}
			int yCtr2;
			do {
				yCtr2 = spriteHeight1;
				byte *destCopy2P = dest2P;
				const byte *spritePixelsCopy2P = spritePixelsP;
				for (int xCtr2 = spriteWidth; xCtr2; xCtr2--) {
					if (*spritePixelsP)
						*dest2P = *spritePixelsP;
					++spritePixelsP;
					--dest2P;
				}
				spritePixelsP = spec_largeur + spritePixelsCopy2P;
				dest2P = _lineNbr2 + destCopy2P;
				spriteHeight1 = yCtr2 - 1;
			} while (yCtr2 != 1);
		} else {
			spec_largeur = spriteWidth;
			if (clip_y) {
				if ((uint16)clip_y >= (unsigned int)spriteHeight1)
					return;
				spritePixelsP += spriteWidth * (uint16)clip_y;
				dest1P += _lineNbr2 * (uint16)clip_y;
				spriteHeight1 -= (uint16)clip_y;
			}
			if (spriteHeight1 > clip_y1)
				spriteHeight1 = clip_y1;
			if (clip_x) {
				if ((uint16)clip_x >= spriteWidth)
					return;
				spritePixelsP += (uint16)clip_x;
				dest1P += (uint16)clip_x;
				spriteWidth -= (uint16)clip_x;
			}
			if (spriteWidth > (uint16)clip_x1)
				spriteWidth = (uint16)clip_x1;
			int yCtr1;
			do {
				yCtr1 = spriteHeight1;
				byte *dest1CopyP = dest1P;
				const byte *spritePixelsCopyP = spritePixelsP;
				for (int xCtr1 = spriteWidth; xCtr1; xCtr1--) {
					if (*spritePixelsP)
						*dest1P = *spritePixelsP;
					++dest1P;
					++spritePixelsP;
				}
				spritePixelsP = spec_largeur + spritePixelsCopyP;
				dest1P = _lineNbr2 + dest1CopyP;
				spriteHeight1 = yCtr1 - 1;
			} while (yCtr1 != 1);
		}
	}
}

/**
 * Fast Display
 */
void GraphicsManager::fastDisplay(const byte *spriteData, int xp, int yp, int spriteIndex) {
	int width = _vm->_objectsManager.getWidth(spriteData, spriteIndex);
	int height = _vm->_objectsManager.getHeight(spriteData, spriteIndex);

	if (*spriteData == 78) {
		Affiche_Perfect(_vesaScreen, spriteData, xp + 300, yp + 300, spriteIndex, 0, 0, 0);
		Affiche_Perfect(_vesaBuffer, spriteData, xp + 300, yp + 300, spriteIndex, 0, 0, 0);
	} else {
		Sprite_Vesa(_vesaBuffer, spriteData, xp + 300, yp + 300, spriteIndex);
		Sprite_Vesa(_vesaScreen, spriteData, xp + 300, yp + 300, spriteIndex);
	}
	if (!_vm->_globals.NO_VISU)
		addVesaSegment(xp, yp, xp + width, yp + height);
}

void GraphicsManager::SCOPY(const byte *surface, int x1, int y1, int width, int height, byte *destSurface, int destX, int destY) {
	int top;
	int croppedWidth;
	int croppedHeight;
	int xRight;
	int top2;
	int left;

	left = x1;
	top = y1;
	croppedWidth = width;
	croppedHeight = height;

	if (x1 < min_x) {
		croppedWidth = width - (min_x - x1);
		left = min_x;
	}
	if (y1 < min_y) {
		croppedHeight = height - (min_y - y1);
		top = min_y;
	}
	top2 = top;
	if (top + croppedHeight > max_y)
		croppedHeight = max_y - top;
	xRight = left + croppedWidth;
	if (xRight > max_x)
		croppedWidth = max_x - left;

	if (croppedWidth > 0 && croppedHeight > 0) {
		int height2 = croppedHeight;
		Copy_Mem(surface, left, top2, croppedWidth, croppedHeight, destSurface, destX, destY);
		addVesaSegment(left, top2, left + croppedWidth, top2 + height2);
	}
}

void GraphicsManager::Copy_Mem(const byte *srcSurface, int x1, int y1, unsigned int width, int height, byte *destSurface, int destX, int destY) {
	const byte *srcP;
	byte *destP;
	int yp;
	int yCurrent;
	byte *dest2P;
	const byte *src2P;
	unsigned int pitch;

	srcP = x1 + _lineNbr2 * y1 + srcSurface;
	destP = destX + _lineNbr2 * destY + destSurface;
	yp = height;
	do {
		yCurrent = yp;
		memcpy(destP, srcP, 4 * (width >> 2));
		src2P = (srcP + 4 * (width >> 2));
		dest2P = (destP + 4 * (width >> 2));
		pitch = width - 4 * (width >> 2);
		memcpy(dest2P, src2P, pitch);
		destP = (dest2P + pitch + _lineNbr2 - width);
		srcP = (src2P + pitch + _lineNbr2 - width);
		yp = yCurrent - 1;
	} while (yCurrent != 1);
}

// Display Font
void GraphicsManager::displayFont(byte *surface, const byte *spriteData, int xp, int yp,
									int characterIndex, int colour) {
	const byte *spriteDataP;
	int i;
	const byte *spriteSizeP;
	int spriteWidth;
	int spriteHeight;
	const byte *spritePixelsP;
	byte *destP;
	byte destByte;
	byte *destLineP;
	int yCtr;

	spriteDataP = spriteData + 3;
	for (i = characterIndex; i; --i)
		spriteDataP += READ_LE_UINT32(spriteDataP) + 16;

	spriteWidth = 0;
	spriteHeight = 0;
	spriteSizeP = spriteDataP + 4;
	spriteWidth = (int16)READ_LE_UINT16(spriteSizeP);
	spriteSizeP += 2;
	spriteHeight = (int16)READ_LE_UINT16(spriteSizeP);
	spritePixelsP = spriteSizeP + 10;
	destP = surface + xp + _lineNbr2 * yp;
	_width = spriteWidth;

	do {
		yCtr = spriteHeight;
		destLineP = destP;
		for (int xCtr = spriteWidth; xCtr; xCtr--) {
			destByte = *spritePixelsP;
			if (*spritePixelsP) {
				if (destByte == 252)
					destByte = colour;
				*destP = destByte;
			}

			++destP;
			++spritePixelsP;
		}
		destP = _lineNbr2 + destLineP;
		spriteHeight = yCtr - 1;
	} while (yCtr != 1);
}

// Init Screen
void GraphicsManager::INI_ECRAN(const Common::String &file) {
	OPTI_INI(file, 0);
}

// Init Screen 2
void GraphicsManager::INI_ECRAN2(const Common::String &file) {
	OPTI_INI(file, 2);
}

void GraphicsManager::OPTI_INI(const Common::String &file, int mode) {
	Common::String filename = file + ".ini";
	byte *ptr = _vm->_fileManager.searchCat(filename, 1);

	if (ptr == g_PTRNUL) {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, filename);
		ptr = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	}
	if (!mode) {
		filename = file + ".spr";
		_vm->_globals.SPRITE_ECRAN = _vm->_globals.freeMemory(_vm->_globals.SPRITE_ECRAN);
		if (!_vm->_globals.NOSPRECRAN) {
			_vm->_globals.SPRITE_ECRAN = _vm->_fileManager.searchCat(filename, 8);
			if (_vm->_globals.SPRITE_ECRAN) {
				_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, filename);
			} else {
				_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, "RES_SLI.RES");
			}
			_vm->_globals.SPRITE_ECRAN = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
		}
	}
	if (ptr[0] != 'I' || ptr[1] != 'N' || ptr[2] != 'I') {
		error("Error, file not ini");
	} else {
		bool doneFlag = false;
		int dataOffset = 1;

		do {
			int dataVal1 = _vm->_scriptManager.handleOpcode(ptr + 20 * dataOffset);
			if (_vm->shouldQuit())
				return;

			if (dataVal1 == 2)
				dataOffset =  _vm->_scriptManager.handleGoto((ptr + 20 * dataOffset));
			if (dataVal1 == 3)
				dataOffset =  _vm->_scriptManager.handleIf(ptr, dataOffset);
			if (dataOffset == -1)
				error("Error, defective IFF");
			if (dataVal1 == 1 || dataVal1 == 4)
				++dataOffset;
			if (!dataVal1 || dataVal1 == 5)
				doneFlag = true;
		} while (!doneFlag);
	}
	_vm->_globals.freeMemory(ptr);
	if (mode != 1) {
		_vm->_globals.COUCOU = _vm->_globals.freeMemory(_vm->_globals.COUCOU);

		filename = file + ".rep";
		byte *dataP = _vm->_fileManager.searchCat(filename, 2);
		_vm->_globals.COUCOU = dataP;
		if (g_PTRNUL == dataP) {
			_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, filename);
			dataP = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
			_vm->_globals.COUCOU = dataP;
		}
	}
	_vm->_objectsManager._forceZoneFl = true;
	_vm->_objectsManager._changeVerbFl = false;
}

void GraphicsManager::NB_SCREEN(bool initPalette) {
	if (initPalette)
		initColorTable(50, 65, _palette);

	if (_lineNbr == SCREEN_WIDTH)
		Trans_bloc2(_vesaBuffer, _colorTable, SCREEN_WIDTH * SCREEN_HEIGHT);
	else if (_lineNbr == (SCREEN_WIDTH * 2))
		Trans_bloc2(_vesaBuffer, _colorTable, SCREEN_WIDTH * SCREEN_HEIGHT * 2);

	lockScreen();
	m_scroll16(_vesaBuffer, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	unlockScreen();

	memcpy(_vesaScreen, _vesaBuffer, 614399);
	DD_VBL();
}

void GraphicsManager::SHOW_PALETTE() {
	setPaletteVGA256(_palette);
}

void GraphicsManager::Copy_WinScan_Vbe(const byte *src, byte *dest) {
	int destOffset;
	const byte *srcPtr;
	byte byteVal;

	destOffset = 0;
	srcPtr = src;
	for (;;) {
		byteVal = *srcPtr;
		if (byteVal == kByteStop)
			return;
		if (*srcPtr > kByteStop) {
			if (byteVal == k8bVal) {
				destOffset += srcPtr[1];
				byteVal = srcPtr[2];
				srcPtr += 2;
			} else if (byteVal == k16bVal) {
				destOffset += READ_LE_UINT16(srcPtr + 1);
				byteVal = srcPtr[3];
				srcPtr += 3;
			} else {
				destOffset += READ_LE_UINT32(srcPtr + 1);
				byteVal = srcPtr[5];
				srcPtr += 5;
			}
		}
		dest[destOffset] = byteVal;
		++srcPtr;
		++destOffset;
	}
}

void GraphicsManager::Copy_Video_Vbe(const byte *src) {
	int destOffset;
	const byte *srcP;
	byte byteVal;

	assert(_videoPtr);
	destOffset = 0;
	srcP = src;
	for (;;) {
		byteVal = *srcP;
		if (*srcP < kByteStop)
			break;
		else {
			if (byteVal == kByteStop)
				return;
			if (byteVal == k8bVal) {
				destOffset += srcP[1];
				byteVal = srcP[2];
				srcP += 2;
			} else if (byteVal == k16bVal) {
				destOffset += READ_LE_UINT16(srcP + 1);
				byteVal = srcP[3];
				srcP += 3;
			} else {
				destOffset += READ_LE_UINT32(srcP + 1);
				byteVal = srcP[5];
				srcP += 5;
			}
		}

		*((byte *)_videoPtr->pixels + destOffset) = byteVal;
		++srcP;
		++destOffset;
	}
}

// Reduce Screen
void GraphicsManager::Reduc_Ecran(const byte *srcSurface, byte *destSurface, int xp, int yp, int width, int height, int zoom) {
	const byte *srcP;
	byte *destP;

	srcP = xp + _lineNbr2 * yp + srcSurface;
	destP = destSurface;
	Red = zoom;
	_width = width;
	Red_x = 0;
	Red_y = 0;
	if (zoom < 100) {
		for (int yCtr = 0; yCtr < height; ++yCtr, srcP += _lineNbr2) {
			Red_y += Red;
			if (Red_y < 100) {
				Red_x = 0;
				const byte *lineSrcP = srcP;

				for (int xCtr = 0; xCtr < _width; ++xCtr) {
					Red_x += Red;
					if (Red_x < 100) {
						*destP++ = *lineSrcP++;
					} else {
						Red_x -= 100;
						++lineSrcP;
					}
				}
			} else {
				Red_y -= 100;
			}
		}
	}
}

/**
 * Draw horizontal line
 */
void GraphicsManager::drawHorizontalLine(byte *surface, int xp, int yp, unsigned int width, byte col) {
	memset(surface + xp + _lineNbr2 * yp, col, width);
}

/**
 * Draw vertical line
 */
void GraphicsManager::drawVerticalLine(byte *surface, int xp, int yp, int height, byte col) {
	byte *destP = surface + xp + _lineNbr2 * yp;

	for (int yCtr = height; yCtr; yCtr--) {
		*destP = col;
		destP += _lineNbr2;
	}
}

} // End of namespace Hopkins
