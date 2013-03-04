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

#include "hopkins/graphics.h"

#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"

#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/decoders/pcx.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"

namespace Hopkins {

GraphicsManager::GraphicsManager() {
	_lockCounter = 0;
	_initGraphicsFl = false;
	_screenWidth = _screenHeight = 0;
	WinScan = 0;
	PAL_PIXELS = NULL;
	_lineNbr = 0;
	_videoPtr = NULL;
	_scrollOffset = 0;
	_scrollPosX = 0;
	_largeScreenFl = false;
	_oldScrollPosX = 0;
	NBBLOC = 0;

	_lineNbr2 = 0;
	Agr_x = Agr_y = 0;
	Agr_Flag_x = Agr_Flag_y = false;
	_fadeDefaultSpeed = 15;
	FADE_LINUX = 0;
	_skipVideoLockFl = false;
	_scrollStatus = 0;
	_minX = 0;
	_minY = 20;
	_maxX = SCREEN_WIDTH * 2;
	_maxY = SCREEN_HEIGHT - 20;
	_posXClipped = _posYClipped = 0;
	clip_x1 = clip_y1 = 0;
	_clipFl = false;
	Red_x = Red_y = 0;
	Red = 0;
	_width = 0;
	_specialWidth = 0;

	Common::fill(&SD_PIXELS[0], &SD_PIXELS[PALETTE_SIZE * 2], 0);
	Common::fill(&_colorTable[0], &_colorTable[PALETTE_EXT_BLOCK_SIZE], 0);
	Common::fill(&_palette[0], &_palette[PALETTE_EXT_BLOCK_SIZE], 0);
	Common::fill(&_oldPalette[0], &_oldPalette[PALETTE_EXT_BLOCK_SIZE], 0);

	for (int i = 0; i < 250; ++i)
		Common::fill((byte *)&BLOC[i], (byte *)&BLOC[i] + sizeof(BlocItem), 0);

}

GraphicsManager::~GraphicsManager() {
	_vm->_globals.freeMemory(_vesaScreen);
	_vm->_globals.freeMemory(_vesaBuffer);
}

void GraphicsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;

	if (_vm->getIsDemo()) {
		if (_vm->getPlatform() == Common::kPlatformLinux)
		// CHECKME: Should be false?
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
	if (!_initGraphicsFl) {
		Graphics::PixelFormat pixelFormat16(2, 5, 6, 5, 0, 11, 5, 0, 0);
		initGraphics(width, height, true, &pixelFormat16);

		// Init surfaces
		_vesaScreen = _vm->_globals.allocMemory(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);
		_vesaBuffer = _vm->_globals.allocMemory(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

		_videoPtr = NULL;
		_screenWidth = width;
		_screenHeight = height;

		// Clear the screen pitch. This will be set on the first lockScreen call
		WinScan = 0;

		PAL_PIXELS = SD_PIXELS;
		_lineNbr = width;

		_initGraphicsFl = true;
	} else {
		error("setGraphicalMode called multiple times");
	}
}

/**
 * (try to) Lock Screen
 */
void GraphicsManager::lockScreen() {
	if (_skipVideoLockFl)
		return;

	if (_lockCounter++ == 0) {
		_videoPtr = g_system->lockScreen();
		if (WinScan == 0)
			WinScan = _videoPtr->pitch;
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
	_videoPtr->fillRect(Common::Rect(0, 0, _screenWidth, _screenHeight), 0);
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
	setScreenWidth(SCREEN_WIDTH);
	lockScreen();
	clearScreen();
	unlockScreen();
	loadPCX320(_vesaScreen, file, _palette);
	memcpy(_vesaBuffer, _vesaScreen, 64000);
	setScreenWidth(320);
	_maxX = 320;

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
	if (_vm->_fileManager.searchCat(file, RES_PIC) == g_PTRNUL) {
		if (!f.open(file))
			error("loadScreen - %s", file.c_str());

		f.seek(0, SEEK_END);
		f.close();
		flag = false;
	}

	scrollScreen(0);
	loadPCX640(_vesaScreen, file, _palette, flag);

	_scrollPosX = 0;
	_oldScrollPosX = 0;
	clearPalette();

	if (!_largeScreenFl) {
		setScreenWidth(SCREEN_WIDTH);
		_maxX = SCREEN_WIDTH;
		lockScreen();
		clearScreen();
		m_scroll16(_vesaScreen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		unlockScreen();
	} else {
		setScreenWidth(SCREEN_WIDTH * 2);
		_maxX = SCREEN_WIDTH * 2;
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
	_scrollPosX = result;
}

void GraphicsManager::Trans_bloc(byte *destP, const byte *srcP, int count, int minThreshold, int maxThreshold) {
	byte *destPosP = destP;
	for (int idx = 0; idx < count; ++idx) {
		int palIndex = *destPosP;
		int srcOffset = 3 * palIndex;
		int col1 = srcP[srcOffset] + srcP[srcOffset + 1] + srcP[srcOffset + 2];

		for (int idx2 = 0; idx2 < 38; ++idx2) {
			srcOffset = 3 * idx2;
			int col2 = srcP[srcOffset] + srcP[srcOffset + 1] + srcP[srcOffset + 2];

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

void GraphicsManager::loadPCX640(byte *surface, const Common::String &file, byte *palette, bool typeFlag) {
	Common::File f;
	Graphics::PCXDecoder pcxDecoder;

	// Clear the passed surface
	memset(surface, 0, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

	if (typeFlag) {
		// Load PCX from within the PIC resource
		if (!f.open("PIC.RES"))
			error("Error opening PIC.RES.");
		f.seek(_vm->_globals._catalogPos);
	} else {
		// Load stand alone PCX file
		if (!f.open(file))
		  error("Error opening PCX %s.", file.c_str());
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

void GraphicsManager::loadPCX320(byte *surface, const Common::String &file, byte *palette) {
	Common::File f;
	if (!f.open(file))
		error("File not found - %s", file.c_str());

	size_t filesize = f.size();

	f.read(surface, 128);
	int imageSize = filesize - 896;
	byte *ptr = _vm->_globals.allocMemory(65024);
	size_t curBufSize;
	int imageNumb;
	int imageDataSize;
	if (imageSize >= 64000) {
		imageNumb = imageSize / 64000 + 1;
		imageDataSize = abs(64000 * (imageSize / 64000) - imageSize);
		f.read(ptr, 64000);
		curBufSize = 64000;
	} else {
		imageNumb = 1;
		imageDataSize = imageSize;
		f.read(ptr, imageSize);
		curBufSize = imageSize;
	}
	imageNumb--;
	size_t curByteIdx = 0;
	for (int i = 0; i < 64000; i++) {
		if (curByteIdx == curBufSize) {
			curByteIdx = 0;
			--imageNumb;
			curBufSize = 64000;
			if (!imageNumb)
				curBufSize = imageDataSize;
			f.read(ptr, curBufSize);
		}
		byte curByte = ptr[curByteIdx++];
		if (curByte > 192) {
			int repeatCount = curByte - 192;
			if (curByteIdx == curBufSize) {
				curByteIdx = 0;
				--imageNumb;
				curBufSize = 64000;
				if (imageNumb == 1)
					curBufSize = imageDataSize;
				f.read(ptr, curBufSize);
			}
			curByte = ptr[curByteIdx++];
			for (; repeatCount; repeatCount--)
				surface[i++] = curByte;

			--i;
		} else {
			surface[i] = curByte;
		}
	}

	f.seek(filesize - 768);
	f.read(palette, 768);
	f.close();

	_vm->_globals.freeMemory(ptr);
}

// Clear Palette
// CHECKME: Some versions of the game don't include it, some contains nothing more than 
// than a loop doing nothing, some others just map the last value. While debugging, it
// seems that this function is called once the palette is already cleared, so it would be useless
// This code could most likely be removed.
void GraphicsManager::clearPalette() {
	uint16 col0 = mapRGB(0, 0, 0);
	for (int i = 0; i < 512; i += 2)
		WRITE_LE_UINT16(&SD_PIXELS[i], col0);
}

void GraphicsManager::setScreenWidth(int pitch) {
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
	int xCtr;
	const byte *palette;
	int yCtr;
	const byte *srcCopyP;
	byte *destCopyP;

	assert(_videoPtr);
	const byte *srcP = xs + _lineNbr2 * ys + surface;
	byte *destP = (byte *)_videoPtr->pixels + destX + destX + WinScan * destY;
	int yNext = height;
	Agr_x = 0;
	Agr_y = 0;
	Agr_Flag_y = false;

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
				if (Agr_x >= 100) {
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
			if (Agr_Flag_y)
				break;

			if (Agr_y >= 0 && Agr_y < 100)
				break;

			Agr_y -= 100;
			Agr_Flag_y = true;
		}

		Agr_Flag_y = false;
		srcP = _lineNbr2 + srcCopyP;
		yNext = yCtr - 1;
	} while (yCtr != 1);
}

void GraphicsManager::Copy_Vga16(const byte *surface, int xp, int yp, int width, int height, int destX, int destY) {
	int xCtr;
	const byte *palette;
	int savedXCount;
	byte *loopDestP;
	const byte *loopSrcP;
	int yCtr;

	assert(_videoPtr);
	const byte *srcP = surface + xp + 320 * yp;
	byte *destP = (byte *)_videoPtr->pixels + 30 * WinScan + destX + destX + destX + destX + WinScan * 2 * destY;
	int yCount = height;
	int xCount = width;

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
		updateScreen();

		// Added a delay in order to see the fading
		_vm->_eventsManager.delay(20);
	}

	// Set the final palette
	setPaletteVGA256(palette);

	// Refresh the screen
	m_scroll16(surface, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	updateScreen();
}

/** 
 * Fade out. the step number is determine by parameter.
 */
void GraphicsManager::fadeOut(const byte *palette, int step, const byte *surface) {
	byte palData[PALETTE_BLOCK_SIZE];
	if ((step > 1) && (palette) && (!_vm->_eventsManager._escKeyFl)) {
		int fadeStep = step;
		for (int fadeIndex = 0; fadeIndex < fadeStep; fadeIndex++) {
			for (int palOffset = 0; palOffset < PALETTE_BLOCK_SIZE; palOffset += 3) {
				palData[palOffset + 0] = (fadeStep - fadeIndex - 1) * palette[palOffset + 0] / (fadeStep - 1);
				palData[palOffset + 1] = (fadeStep - fadeIndex - 1) * palette[palOffset + 1] / (fadeStep - 1);
				palData[palOffset + 2] = (fadeStep - fadeIndex - 1) * palette[palOffset + 2] / (fadeStep - 1);
			}

			setPaletteVGA256(palData);
			m_scroll16(surface, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			updateScreen();

			_vm->_eventsManager.delay(20);
		}
	}

	// No initial palette, or end of fading
	for (int i = 0; i < PALETTE_BLOCK_SIZE; i++)
		palData[i] = 0;

	setPaletteVGA256(palData);
	m_scroll16(surface, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	return updateScreen();
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
	updateScreen();
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
	updateScreen();
}

void GraphicsManager::setPaletteVGA256(const byte *palette) {
	changePalette(palette);
}

void GraphicsManager::setPaletteVGA256WithRefresh(const byte *palette, const byte *surface) {
	changePalette(palette);
	m_scroll16(surface, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	updateScreen();
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

void GraphicsManager::updateScreen() {
	// TODO: Is this okay here?
	g_system->updateScreen();
}

void GraphicsManager::copyWinscanVbe3(const byte *srcData, byte *destSurface) {
	byte srcByte;
	byte destLen1;
	byte *destSlice1P;
	byte destLen2;
	byte *destSlice2P;

	int rleValue = 0;
	int destOffset = 0;
	const byte *srcP = srcData;
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

void GraphicsManager::copyVideoVbe16(const byte *srcData) {
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

void GraphicsManager::copyVideoVbe16a(const byte *srcData) {
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

void GraphicsManager::copySurfaceRect(const byte *srcSurface, byte *destSurface, int xs, int ys, int width, int height) {
	const byte *srcP;
	byte *destP;
	int rowCount;
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
			for (int i = width >> 1; i; --i) {
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

/**
 * Draws a sprite onto the screen
 * @param surface		Destination surface
 * @param spriteData	The raw data for a sprite set
 * @param xp			X co-ordinate. For some reason, starts from 300 = first column
 * @param yp			Y co-ordinate. FOr some reason, starts from 300 = top row
 * @param spriteIndex	Index of the sprite to draw
 */
void GraphicsManager::Sprite_Vesa(byte *surface, const byte *spriteData, int xp, int yp, int spriteIndex) {
	// Get a pointer to the start of the desired sprite
	const byte *spriteP = spriteData + 3;
	for (int i = spriteIndex; i; --i)
		spriteP += READ_LE_UINT32(spriteP) + 16;

	_posXClipped = 0;
	_posYClipped = 0;
	_clipFl = false;

	spriteP += 4;
	int width = READ_LE_UINT16(spriteP);
	spriteP += 2;
	int height = READ_LE_UINT16(spriteP);

	// Clip X
	clip_x1 = width;
	if ((xp + width) <= _minX + 300)
		return;
	if (xp < _minX + 300) {
		_posXClipped = _minX + 300 - xp;
		_clipFl = true;
	}

	// Clip Y
	clip_y1 = height;
	if (yp <= 0)
		return;
	if (yp < _minY + 300) {
		_posYClipped = _minY + 300 - yp;
		_clipFl = true;
	}

	// Clip X1
	if (xp >= _maxX + 300)
		return;
	if (xp + width > _maxX + 300) {
		int xAmount = width + 10 - (xp + width - (_maxX + 300));
		if (xAmount <= 10)
			return;

		clip_x1 = xAmount - 10;
		_clipFl = true;
	}

	// Clip Y1
	if (yp >= _maxY + 300)
		return;
	if (yp + height > _maxY + 300) {
		int yAmount = height + 10 - (yp + height - (_maxY + 300));
		if (yAmount <= 10)
			return;

		// clip_y1 is always positive thanks to the previous check
		clip_y1 = yAmount - 10;
		_clipFl = true;
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
	if (_clipFl) {
		// Clipped version
		for (int yc = 0; yc < clip_y1; ++yc, destP += _lineNbr2) {
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
						if (_posYClipped == 0 && xc >= _posXClipped && xc < clip_x1)
							*tempDestP = *spriteP;
					}
				} else {
					// Skip over bytes
					tempDestP += width;
					xc += width;
				}
			}

			if (_posYClipped > 0)
				--_posYClipped;
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

void GraphicsManager::endDisplayBob() {
	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_globals._animBqe[idx]._enabledFl)
			_vm->_objectsManager.hideBob(idx);
	}

	_vm->_eventsManager.refreshScreenAndEvents();
	_vm->_eventsManager.refreshScreenAndEvents();

	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_globals._animBqe[idx]._enabledFl)
			_vm->_objectsManager.resetBob(idx);
	}

	for (int idx = 1; idx <= 29; ++idx) {
		_vm->_globals._lockedAnims[idx]._enableFl = false;
	}

	for (int idx = 1; idx <= 20; ++idx) {
		_vm->_globals._animBqe[idx]._enabledFl = false;
	}
}

void GraphicsManager::displayAllBob() {
	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_globals._animBqe[idx]._enabledFl)
			_vm->_objectsManager.displayBob(idx);
	}
}

void GraphicsManager::resetVesaSegment() {
	for (int idx = 0; idx <= NBBLOC; idx++)
		BLOC[idx]._activeFl = false;

	NBBLOC = 0;
}

// Add VESA Segment
void GraphicsManager::addVesaSegment(int x1, int y1, int x2, int y2) {
	int tempX = x1;
	bool addFlag = true;
	if (x2 > _maxX)
		x2 = _maxX;
	if (y2 > _maxY)
		y2 = _maxY;
	if (x1 < _minX)
		tempX = _minX;
	if (y1 < _minY)
		y1 = _minY;

	for (int blocIndex = 0; blocIndex <= NBBLOC; blocIndex++) {
		BlocItem &bloc = BLOC[blocIndex];
		if (bloc._activeFl && tempX >= bloc._x1 && x2 <= bloc._x2 && y1 >= bloc._y1 && y2 <= bloc._y2)
			addFlag = false;
	};

	if (addFlag) {
		assert(NBBLOC < 250);
		BlocItem &bloc = BLOC[++NBBLOC];

		bloc._activeFl = true;
		bloc._x1 = tempX;
		bloc._x2 = x2;
		bloc._y1 = y1;
		bloc._y2 = y2;
	}
}

// Display VESA Segment
void GraphicsManager::displayVesaSegment() {
	if (NBBLOC == 0)
		return;

	lockScreen();

	for (int idx = 1; idx <= NBBLOC; ++idx) {
		BlocItem &bloc = BLOC[idx];
		Common::Rect &dstRect = dstrect[idx - 1];
		if (!bloc._activeFl)
			continue;

		if (_vm->_eventsManager._breakoutFl) {
			Copy_Vga16(_vesaBuffer, bloc._x1, bloc._y1, bloc._x2 - bloc._x1, bloc._y2 - bloc._y1, bloc._x1, bloc._y1);
			dstRect.left = bloc._x1 * 2;
			dstRect.top = bloc._y1 * 2 + 30;
			dstRect.setWidth((bloc._x2 - bloc._x1) * 2);
			dstRect.setHeight((bloc._y2 - bloc._y1) * 2);
		} else if (bloc._x2 > _vm->_eventsManager._startPos.x && bloc._x1 < _vm->_eventsManager._startPos.x + SCREEN_WIDTH) {
			if (bloc._x1 < _vm->_eventsManager._startPos.x)
				bloc._x1 = _vm->_eventsManager._startPos.x;
			if (bloc._x2 > _vm->_eventsManager._startPos.x + SCREEN_WIDTH)
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

		BLOC[idx]._activeFl = false;
	}

	NBBLOC = 0;
	unlockScreen();
}

void GraphicsManager::AFFICHE_SPEEDVGA(const byte *objectData, int xp, int yp, int idx, bool addSegment) {
	int width = _vm->_objectsManager.getWidth(objectData, idx);
	int height = _vm->_objectsManager.getHeight(objectData, idx);
	if (*objectData == 78) {
		Affiche_Perfect(_vesaScreen, objectData, xp + 300, yp + 300, idx, 0, 0, false);
		Affiche_Perfect(_vesaBuffer, objectData, xp + 300, yp + 300, idx, 0, 0, false);
	} else {
		Sprite_Vesa(_vesaBuffer, objectData, xp + 300, yp + 300, idx);
		Sprite_Vesa(_vesaScreen, objectData, xp + 300, yp + 300, idx);
	}
	if (addSegment)
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

void GraphicsManager::restoreSurfaceRect(byte *destSurface, const byte *src, int xp, int yp, int width, int height) {
	int yCtr;

	byte *destP = xp + _lineNbr2 * yp + destSurface;
	int yNext = height;
	const byte *srcP = src;
	do {
		yCtr = yNext;
		if (width & 1) {
			memcpy(destP, srcP, width);
			srcP += width;
			destP += width;
		} else if (width & 2) {
			for (int i = width >> 1; i; --i) {
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
int GraphicsManager::zoomIn(int val, int percentage ) {
	if (val)
		val += percentage * (long int)val / 100;

	return val;
}

/**
 * Compute the value of a parameter minus a given percentage
 */
int GraphicsManager::zoomOut(int val, int percentage) {
	if (val)
		val -= percentage * (long int)val / 100;

	return val;
}

// Display 'Perfect?'
void GraphicsManager::Affiche_Perfect(byte *surface, const byte *srcData, int xp300, int yp300, int frameIndex, int zoom1, int zoom2, bool flipFl) {
	const byte *spriteStartP = srcData + 3;
	for (int i = frameIndex; i; --i)
		spriteStartP += READ_LE_UINT32(spriteStartP) + 16;

	const byte *spriteSizeP = spriteStartP + 4;
	int spriteWidth = READ_LE_INT16(spriteSizeP);
	spriteSizeP += 2;
	int spriteHeight2 = READ_LE_INT16(spriteSizeP);
	int spriteHeight1 = spriteHeight2;
	const byte *spritePixelsP = spriteSizeP + 10;
	_posXClipped = 0;
	_posYClipped = 0;
	clip_x1 = 0;
	clip_y1 = 0;
	if ((xp300 <= _minX) || (yp300 <= _minY) || (xp300 >= _maxX + 300) || 	(yp300 >= _maxY + 300))
		return;

	// Clipped values are greater or equal to zero, thanks to the previous test
	clip_x1 = _maxX + 300 - xp300;
	clip_y1 = _maxY + 300 - yp300;

	// _minX is never negative, and should be always 0
	// The previous check insures that xp300 it's always greater to it
	// After this check, posXClipped is always positive
	if (xp300 < _minX + 300)
		_posXClipped = _minX + 300 - xp300;

	// Ditto.
	if (yp300 < _minY + 300)
		_posYClipped = _minY + 300 - yp300;

	byte *dest1P = xp300 + _lineNbr2 * (yp300 - 300) - 300 + surface;
	if (zoom2) {
		Agr_x = 0;
		Agr_y = 0;
		Agr_Flag_y = false;
		Agr_Flag_x = false;
		_width = spriteWidth;
		int zoomedWidth = zoomIn(spriteWidth, zoom2);
		int zoomedHeight = zoomIn(spriteHeight1, zoom2);
		if (flipFl) {
			byte *v29 = zoomedWidth + dest1P;
			if (_posYClipped) {
				if (_posYClipped < 0 || _posYClipped >= zoomedHeight)
					return;
				int v30 = 0;
				while (zoomIn(++v30, zoom2) < _posYClipped)
					;
				spritePixelsP += _width * v30;
				v29 += _lineNbr2 * _posYClipped;
				zoomedHeight -= _posYClipped;
			}
			if (zoomedHeight > clip_y1)
				zoomedHeight = clip_y1;
			if (_posXClipped) {
				if (_posXClipped >= zoomedWidth)
					return;
				zoomedWidth -= _posXClipped;
			}
			if (zoomedWidth > clip_x1) {
				int v32 = zoomedWidth - clip_x1;
				v29 -= v32;
				int v33 = 0;
				while (zoomIn(++v33, zoom2) < v32)
					;
				spritePixelsP += v33;
				zoomedWidth = clip_x1;
			}
			int v63;
			do {
				for (;;) {
					v63 = zoomedHeight;
					byte *v53 = v29;
					const byte *oldSpritePixelsP = spritePixelsP;
					Agr_Flag_x = false;
					Agr_x = 0;
					for (int v35 = zoomedWidth; v35; Agr_Flag_x = false, v35--) {
						for (;;) {
							if (*spritePixelsP)
								*v29 = *spritePixelsP;
							--v29;
							++spritePixelsP;
							if (!Agr_Flag_x)
								Agr_x += zoom2;
							if (Agr_x >= 0 && Agr_x < 100)
								break;
							Agr_x -= 100;
							--spritePixelsP;
							Agr_Flag_x = true;
							--v35;
							if (!v35)
								break;
						}
					}
					spritePixelsP = _width + oldSpritePixelsP;
					v29 = _lineNbr2 + v53;
					if (!Agr_Flag_y)
						Agr_y += zoom2;
					if ((uint16)Agr_y < 100)
						break;
					Agr_y -= 100;
					spritePixelsP = oldSpritePixelsP;
					Agr_Flag_y = true;
					zoomedHeight = v63 - 1;
					if (v63 == 1)
						return;
				}
				Agr_Flag_y = false;
				zoomedHeight = v63 - 1;
			} while (v63 != 1);
		} else {
			if (_posYClipped) {
				if (_posYClipped >= zoomedHeight)
					return;
				int v23 = 0;
				while (zoomIn(++v23, zoom2) < _posYClipped)
					;
				spritePixelsP += _width * v23;
				dest1P += _lineNbr2 * _posYClipped;
				zoomedHeight -= _posYClipped;
			}
			if (zoomedHeight > clip_y1)
				zoomedHeight = clip_y1;
			if (_posXClipped) {
				if (_posXClipped >= zoomedWidth)
					return;
				int v26 = 0;
				while (zoomIn(++v26, zoom2) < _posXClipped)
					;
				spritePixelsP += v26;
				dest1P += _posXClipped;
				zoomedWidth = zoomedWidth - _posXClipped;
			}
			if (zoomedWidth > clip_x1)
				zoomedWidth = clip_x1;

			int v60;
			do {
				for (;;) {
					v60 = zoomedHeight;
					byte *oldDest1P = dest1P;
					const byte *oldSpritePixelsP = spritePixelsP;
					Agr_Flag_x = false;
					Agr_x = 0;
					for (int v28 = zoomedWidth; v28; Agr_Flag_x = false, v28--) {
						for (;;) {
							if (*spritePixelsP)
								*dest1P = *spritePixelsP;
							++dest1P;
							++spritePixelsP;
							if (!Agr_Flag_x)
								Agr_x += zoom2;
							if ((uint16)Agr_x < 100)
								break;
							Agr_x -= 100;
							--spritePixelsP;
							Agr_Flag_x = true;
							--v28;
							if (!v28)
								break;
						}
					}
					spritePixelsP = _width + oldSpritePixelsP;
					dest1P = _lineNbr2 + oldDest1P;
					if (!Agr_Flag_y)
						Agr_y += zoom2;
					if ((uint16)Agr_y < 100)
						break;
					Agr_y -= 100;
					spritePixelsP = oldSpritePixelsP;
					Agr_Flag_y = true;
					zoomedHeight = v60 - 1;
					if (v60 == 1)
						return;
				}
				Agr_Flag_y = false;
				zoomedHeight = v60 - 1;
			} while (v60 != 1);
		}
	} else if (zoom1) {
		Red_x = 0;
		Red_y = 0;
		_width = spriteWidth;
		Red = zoom1;
		if (zoom1 < 100) {
			int v37 = zoomOut(spriteWidth, Red);
			if (flipFl) {
				byte *v40 = v37 + dest1P;
				do {
					int v65 = spriteHeight2;
					byte *v55 = v40;
					Red_y += Red;
					if ((uint16)Red_y < 100) {
						Red_x = 0;
						int v42 = v37;
						for (int v41 = _width; v41; v41--) {
							Red_x += Red;
							if ((uint16)Red_x < 100) {
								if (v42 >= _posXClipped && v42 < clip_x1 && *spritePixelsP)
									*v40 = *spritePixelsP;
								--v40;
								++spritePixelsP;
								--v42;
							} else {
								Red_x -= 100;
								++spritePixelsP;
							}
						}
						spriteHeight2 = v65;
						v40 = _lineNbr2 + v55;
					} else {
						Red_y -= 100;
						spritePixelsP += _width;
					}
					--spriteHeight2;
				} while (spriteHeight2);
			} else {
				do {
					int oldSpriteHeight = spriteHeight2;
					byte *oldDest1P = dest1P;
					Red_y += Red;
					if ((uint16)Red_y < 100) {
						Red_x = 0;
						int v39 = 0;
						for (int i = _width; i; i--) {
							Red_x += Red;
							if ((uint16)Red_x < 100) {
								if (v39 >= _posXClipped && v39 < clip_x1 && *spritePixelsP)
									*dest1P = *spritePixelsP;
								++dest1P;
								++spritePixelsP;
								++v39;
							} else {
								Red_x -= 100;
								++spritePixelsP;
							}
						}
						spriteHeight2 = oldSpriteHeight;
						dest1P = _lineNbr2 + oldDest1P;
					} else {
						Red_y -= 100;
						spritePixelsP += _width;
					}
					--spriteHeight2;
				} while (spriteHeight2);
			}
		}
	} else {
		_width = spriteWidth;
		if (flipFl) {
			byte *dest2P = spriteWidth + dest1P;
			_specialWidth = spriteWidth;
			if (_posYClipped) {
				if (_posYClipped >= spriteHeight1 || spriteHeight1 < 0)
					return;
				spritePixelsP += spriteWidth * _posYClipped;
				dest2P += _lineNbr2 * _posYClipped;
				spriteHeight1 -= _posYClipped;
			}
			if (spriteHeight1 > clip_y1)
				spriteHeight1 = clip_y1;

			if (_posXClipped >= spriteWidth)
				return;
			spriteWidth -= _posXClipped;

			if (spriteWidth > clip_x1) {
				int clippedWidth = spriteWidth - clip_x1;
				spritePixelsP += clippedWidth;
				dest2P -= clippedWidth;
				spriteWidth = clip_x1;
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
				spritePixelsP = _specialWidth + spritePixelsCopy2P;
				dest2P = _lineNbr2 + destCopy2P;
				spriteHeight1 = yCtr2 - 1;
			} while (yCtr2 != 1);
		} else {
			_specialWidth = spriteWidth;
			if (_posYClipped) {
				if (_posYClipped >= spriteHeight1 || spriteHeight1 < 0)
					return;
				spritePixelsP += spriteWidth * _posYClipped;
				dest1P += _lineNbr2 * _posYClipped;
				spriteHeight1 -= _posYClipped;
			}
			if (spriteHeight1 > clip_y1)
				spriteHeight1 = clip_y1;
			if (_posXClipped) {
				if (_posXClipped >= spriteWidth)
					return;
				spritePixelsP += _posXClipped;
				dest1P += _posXClipped;
				spriteWidth -= _posXClipped;
			}
			if (spriteWidth > clip_x1)
				spriteWidth = clip_x1;
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
				spritePixelsP = _specialWidth + spritePixelsCopyP;
				dest1P = _lineNbr2 + dest1CopyP;
				spriteHeight1 = yCtr1 - 1;
			} while (yCtr1 != 1);
		}
	}
}

/**
 * Fast Display
 */
void GraphicsManager::fastDisplay(const byte *spriteData, int xp, int yp, int spriteIndex, bool addSegment) {
	int width = _vm->_objectsManager.getWidth(spriteData, spriteIndex);
	int height = _vm->_objectsManager.getHeight(spriteData, spriteIndex);

	if (*spriteData == 78) {
		Affiche_Perfect(_vesaScreen, spriteData, xp + 300, yp + 300, spriteIndex, 0, 0, false);
		Affiche_Perfect(_vesaBuffer, spriteData, xp + 300, yp + 300, spriteIndex, 0, 0, false);
	} else {
		Sprite_Vesa(_vesaBuffer, spriteData, xp + 300, yp + 300, spriteIndex);
		Sprite_Vesa(_vesaScreen, spriteData, xp + 300, yp + 300, spriteIndex);
	}
	if (addSegment)
		addVesaSegment(xp, yp, xp + width, yp + height);
}

void GraphicsManager::copySurface(const byte *surface, int x1, int y1, int width, int height, byte *destSurface, int destX, int destY) {
	int left = x1;
	int top = y1;
	int croppedWidth = width;
	int croppedHeight = height;

	if (x1 < _minX) {
		croppedWidth = width - (_minX - x1);
		left = _minX;
	}
	if (y1 < _minY) {
		croppedHeight = height - (_minY - y1);
		top = _minY;
	}

	if (top + croppedHeight > _maxY)
		croppedHeight = _maxY - top;
	if (left + croppedWidth > _maxX)
		croppedWidth = _maxX - left;

	if (croppedWidth > 0 && croppedHeight > 0) {
		int height2 = croppedHeight;
		Copy_Mem(surface, left, top, croppedWidth, croppedHeight, destSurface, destX, destY);
		addVesaSegment(left, top, left + croppedWidth, top + height2);
	}
}

void GraphicsManager::Copy_Mem(const byte *srcSurface, int x1, int y1, uint16 width, int height, byte *destSurface, int destX, int destY) {
	const byte *srcP = x1 + _lineNbr2 * y1 + srcSurface;
	byte *destP = destX + _lineNbr2 * destY + destSurface;
	int yp = height;
	int yCurrent;
	do {
		yCurrent = yp;
		memcpy(destP, srcP, 4 * (width >> 2));
		const byte *src2P = (srcP + 4 * (width >> 2));
		byte *dest2P = (destP + 4 * (width >> 2));
		int pitch = width - 4 * (width >> 2);
		memcpy(dest2P, src2P, pitch);
		destP = (dest2P + pitch + _lineNbr2 - width);
		srcP = (src2P + pitch + _lineNbr2 - width);
		yp = yCurrent - 1;
	} while (yCurrent != 1);
}

// Display Font
void GraphicsManager::displayFont(byte *surface, const byte *spriteData, int xp, int yp, int characterIndex, int colour) {
	const byte *spriteDataP = spriteData + 3;
	for (int i = characterIndex; i; --i)
		spriteDataP += READ_LE_UINT32(spriteDataP) + 16;

	int spriteWidth = 0;
	int spriteHeight = 0;
	const byte *spriteSizeP = spriteDataP + 4;
	spriteWidth = READ_LE_INT16(spriteSizeP);
	spriteSizeP += 2;
	spriteHeight = READ_LE_INT16(spriteSizeP);
	const byte *spritePixelsP = spriteSizeP + 10;
	byte *destP = surface + xp + _lineNbr2 * yp;
	_width = spriteWidth;

	int yCtr;
	do {
		yCtr = spriteHeight;
		byte *destLineP = destP;
		for (int xCtr = spriteWidth; xCtr; xCtr--) {
			byte destByte = *spritePixelsP;
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

void GraphicsManager::initScreen(const Common::String &file, int mode, bool initializeScreen) {
	Common::String filename = file + ".ini";
	byte *ptr = _vm->_fileManager.searchCat(filename, RES_INI);

	if (ptr == g_PTRNUL) {
		ptr = _vm->_fileManager.loadFile(filename);
	}
	if (!mode) {
		filename = file + ".spr";
		_vm->_globals.SPRITE_ECRAN = _vm->_globals.freeMemory(_vm->_globals.SPRITE_ECRAN);
		if (initializeScreen) {
			_vm->_globals.SPRITE_ECRAN = _vm->_fileManager.searchCat(filename, RES_SLI);
			if (_vm->_globals.SPRITE_ECRAN) {
				_vm->_globals.SPRITE_ECRAN = _vm->_fileManager.loadFile(filename);
			} else {
				_vm->_globals.SPRITE_ECRAN = _vm->_fileManager.loadFile("RES_SLI.RES");
			}
		}
	}
	if (READ_BE_UINT24(ptr) != MKTAG24('I', 'N', 'I')) {
		error("Invalid INI File %s", file.c_str());
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
	_vm->_globals._answerBuffer = _vm->_globals.freeMemory(_vm->_globals._answerBuffer);

	filename = file + ".rep";
	byte *dataP = _vm->_fileManager.searchCat(filename, RES_REP);
	if (dataP == g_PTRNUL)
		dataP = _vm->_fileManager.loadFile(filename);

	_vm->_globals._answerBuffer = dataP;
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
	updateScreen();
}

void GraphicsManager::copyWinscanVbe(const byte *src, byte *dest) {
	int destOffset = 0;
	const byte *srcPtr = src;
	for (;;) {
		byte byteVal = *srcPtr;
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

// Reduce Screen
void GraphicsManager::reduceScreenPart(const byte *srcSurface, byte *destSurface, int xp, int yp, int width, int height, int zoom) {
	const byte *srcP = xp + _lineNbr2 * yp + srcSurface;
	byte *destP = destSurface;
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
void GraphicsManager::drawHorizontalLine(byte *surface, int xp, int yp, uint16 width, byte col) {
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
