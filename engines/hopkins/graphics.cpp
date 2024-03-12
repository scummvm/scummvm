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

#include "hopkins/graphics.h"

#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"

#include "common/system.h"
#include "image/pcx.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"

namespace Hopkins {

GraphicsManager::GraphicsManager(HopkinsEngine *vm) {
	_vm = vm;

	_lockCounter = 0;
	_initGraphicsFl = false;
	_screenWidth = _screenHeight = 0;
	_screenLineSize = 0;
	_palettePixels = nullptr;
	_lineNbr = 0;
	_videoPtr = nullptr;
	_scrollOffset = 0;
	_scrollPosX = 0;
	_largeScreenFl = false;
	_oldScrollPosX = 0;
	_backBuffer = nullptr;
	_frontBuffer = nullptr;
	_screenBuffer = nullptr;
	_backupScreen = nullptr;
	_showDirtyRects = false;

	_lineNbr2 = 0;
	_enlargedX = _enlargedY = 0;
	_enlargedXFl = _enlargedYFl = false;
	_fadeDefaultSpeed = 15;
	_fadingFl = false;
	_skipVideoLockFl = false;
	_scrollStatus = 0;
	_minX = 0;
	_minY = 20;
	_maxX = SCREEN_WIDTH * 2;
	_maxY = SCREEN_HEIGHT - 20;
	_posXClipped = _posYClipped = 0;
	_clipX1 = _clipY1 = 0;
	_clipFl = false;
	_reduceX = _reducedY = 0;
	_zoomOutFactor = 0;
	_width = 0;
	_specialWidth = 0;
	_showZones = false;
	_showLines = false;

	Common::fill(&_paletteBuffer[0], &_paletteBuffer[PALETTE_SIZE * 2], 0);
	Common::fill(&_colorTable[0], &_colorTable[PALETTE_EXT_BLOCK_SIZE], 0);
	Common::fill(&_palette[0], &_palette[PALETTE_EXT_BLOCK_SIZE], 0);
	Common::fill(&_oldPalette[0], &_oldPalette[PALETTE_EXT_BLOCK_SIZE], 0);

	if (_vm->getIsDemo()) {
		if (_vm->getPlatform() == Common::kPlatformLinux)
			// CHECKME: Should be false?
			_manualScroll = true;
		else
			_manualScroll = false;
		_scrollSpeed = 16;
	} else {
		_manualScroll = false;
		_scrollSpeed = 32;
	}

	_noFadingFl = false;
}

GraphicsManager::~GraphicsManager() {
	_vm->_globals->freeMemory(_backBuffer);
	_vm->_globals->freeMemory(_frontBuffer);
	_vm->_globals->freeMemory(_screenBuffer);
	_vm->_globals->freeMemory(_backupScreen);
}

void GraphicsManager::setGraphicalMode(int width, int height) {
	if (!_initGraphicsFl) {
		Graphics::PixelFormat pixelFormat16(2, 5, 6, 5, 0, 11, 5, 0, 0);
		initGraphics(width, height, &pixelFormat16);

		// Init surfaces
		_backBuffer = _vm->_globals->allocMemory(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);
		_frontBuffer = _vm->_globals->allocMemory(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);
		_screenBuffer = _vm->_globals->allocMemory(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

		_videoPtr = nullptr;
		_screenWidth = width;
		_screenHeight = height;

		_screenLineSize = SCREEN_WIDTH * 2;
		_palettePixels = _paletteBuffer;
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
	if (!_skipVideoLockFl) {
		if (_lockCounter++ == 0) {
			_videoPtr = _screenBuffer;
			_screenLineSize = SCREEN_WIDTH * 2;
		}
	}
}

/**
 * (try to) Unlock Screen
 */
void GraphicsManager::unlockScreen() {
	assert(_videoPtr);
	if (--_lockCounter == 0) {
		_videoPtr = nullptr;
	}
}

/**
 * Clear Screen
 */
void GraphicsManager::clearScreen() {
	lockScreen();
	assert(_videoPtr);

	Common::fill(_screenBuffer, _screenBuffer + _screenLineSize * _screenHeight, 0);
	addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	unlockScreen();
}

void GraphicsManager::clearVesaScreen() {
	Common::fill(_backBuffer, _backBuffer + _screenLineSize * _screenHeight, 0);
	Common::fill(_frontBuffer, _frontBuffer + _screenLineSize * _screenHeight, 0);
	addDirtyRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
}

/**
 * Load Image
 */
void GraphicsManager::loadImage(const Common::Path &file) {
	Common::Path filename(file);
	filename.appendInPlace(".PCX");
	loadScreen(filename);
	initColorTable(165, 170, _palette);
}

/**
 * Load VGA Image
 */
void GraphicsManager::loadVgaImage(const Common::Path &file) {
	setScreenWidth(SCREEN_WIDTH);
	clearScreen();
	loadPCX320(_backBuffer, file, _palette);
	memcpy(_frontBuffer, _backBuffer, 64000);
	setScreenWidth(320);
	_maxX = 320;

	copy16bFromSurfaceScaleX2(_frontBuffer);
	addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	fadeInBreakout();
}

/**
 * Load Screen
 */
void GraphicsManager::loadScreen(const Common::Path &file) {
	Common::File f;
	assert(!_videoPtr);

	bool flag = true;
	bool fileFoundFl = false;
	_vm->_fileIO->searchCat(file, RES_PIC, fileFoundFl);
	if (!fileFoundFl) {
		if (!f.open(file))
			error("loadScreen - %s", file.toString().c_str());

		f.seek(0, SEEK_END);
		f.close();
		flag = false;
	}

	scrollScreen(0);
	loadPCX640(_backBuffer, file, _palette, flag);

	_scrollPosX = 0;
	_oldScrollPosX = 0;
	clearPalette();

	if (!_largeScreenFl) {
		setScreenWidth(SCREEN_WIDTH);
		_maxX = SCREEN_WIDTH;
		clearScreen();

		display8BitRect(_backBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	} else {
		setScreenWidth(SCREEN_WIDTH * 2);
		_maxX = SCREEN_WIDTH * 2;
		clearScreen();

		if (_manualScroll)
			display8BitRect(_backBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	}

	memcpy(_frontBuffer, _backBuffer, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);
}

void GraphicsManager::initColorTable(int minIndex, int maxIndex, byte *palette) {
	for (int idx = 0; idx < 256; ++idx)
		_colorTable[idx] = idx;

	translateSurface(_colorTable, palette, 256, minIndex, maxIndex);

	for (int idx = 0; idx < 256; ++idx) {
		byte v = _colorTable[idx];
		if (v > 27 || !v)
			_colorTable[idx] = 0;
	}

	_colorTable[0] = 1;
}

/**
 * Scroll Screen
 */
void GraphicsManager::scrollScreen(int amount) {
	int result = CLIP(amount, 0, SCREEN_WIDTH);
	_vm->_events->_startPos.x = result;
	_scrollOffset = result;
	_scrollPosX = result;
}

void GraphicsManager::translateSurface(byte *destP, const byte *srcP, int count, int minThreshold, int maxThreshold) {
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

void GraphicsManager::fillSurface(byte *surface, byte *col, int size) {
	byte dataVal;

	byte *dataP = surface;
	for (int count = size - 1; count; count--){
		dataVal = *dataP;
		*dataP = col[dataVal];
		dataP++;
	}
}

void GraphicsManager::loadPCX640(byte *surface, const Common::Path &file, byte *palette, bool typeFlag) {
	Common::File f;
	Image::PCXDecoder pcxDecoder;

	// Clear the passed surface
	memset(surface, 0, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

	if (typeFlag) {
		// Load PCX from within the PIC resource
		if (!f.open("PIC.RES"))
			error("Error opening PIC.RES.");
		f.seek(_vm->_fileIO->_catalogPos);
	} else {
		// Load stand alone PCX file
		if (!f.open(file))
		  error("Error opening PCX %s.", file.toString().c_str());
	}

	// Decode the PCX
	if (!pcxDecoder.loadStream(f))
		error("Error decoding PCX %s", file.toString().c_str());

	const Graphics::Surface *s = pcxDecoder.getSurface();

	// Copy out the dimensions and pixels of the decoded surface
	_largeScreenFl = s->w > SCREEN_WIDTH;
	Common::copy((const byte *)s->getPixels(), (const byte *)s->getBasePtr(0, s->h), surface);

	// Copy out the palette
	const byte *palSrc = pcxDecoder.getPalette();
	Common::copy((const byte *)palSrc, (const byte *)palSrc + PALETTE_BLOCK_SIZE, palette);

	f.close();
}

void GraphicsManager::loadPCX320(byte *surface, const Common::Path &file, byte *palette) {
	Common::File f;
	if (!f.open(file))
		error("File not found - %s", file.toString().c_str());

	size_t filesize = f.size();

	f.read(surface, 128);
	int imageSize = filesize - 896;
	byte *ptr = _vm->_globals->allocMemory(65024);
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

	_vm->_globals->freeMemory(ptr);
}

// Clear Palette
void GraphicsManager::clearPalette() {
	// As weird as it sounds, this is what the original Linux executable does,
	// and not a full array clear.
	_paletteBuffer[0] = 0;
}

void GraphicsManager::setScreenWidth(int pitch) {
	_lineNbr = _lineNbr2 = pitch;
}

/**
 * Copies data from a 8-bit palette surface into the 16-bit screen
 */
void GraphicsManager::display8BitRect(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	lockScreen();

	assert(_videoPtr);
	const byte *srcP = xs + _lineNbr2 * ys + surface;
	byte *destP = (byte *)_videoPtr + destX * 2 + _screenLineSize * destY;

	for (int yp = 0; yp < height; ++yp) {
		// Copy over the line, using the source pixels as lookups into the pixels palette
		const byte *lineSrcP = srcP;
		byte *lineDestP = destP;

		for (int xp = 0; xp < width; ++xp) {
			lineDestP[0] = _palettePixels[lineSrcP[0] * 2];
			lineDestP[1] = _palettePixels[(lineSrcP[0] * 2) + 1];
			lineDestP += 2;
			lineSrcP++;
		}
		// Move to the start of the next line
		srcP += _lineNbr2;
		destP += _screenLineSize;
	}

	unlockScreen();
	addRefreshRect(destX, destY, destX + width, destY + height);
}

void GraphicsManager::displayScaled8BitRect(const byte *surface, int xp, int yp, int width, int height, int destX, int destY) {
	const byte *palette;
	byte *loopDestP;
	const byte *loopSrcP;
	int yCtr;

	assert(_videoPtr);
	const byte *srcP = surface + xp + 320 * yp;
	byte *destP = (byte *)_videoPtr + 30 * _screenLineSize + destX + destX + destX + destX + _screenLineSize * 2 * destY;
	int yCount = height;
	int xCount = width;

	do {
		yCtr = yCount;
		int xCtr = xCount;
		loopSrcP = srcP;
		loopDestP = destP;
		int savedXCount = xCount;
		palette = _palettePixels;

		do {
			destP[0] = destP[2] = destP[_screenLineSize] = destP[_screenLineSize + 2] = palette[2 * srcP[0]];
			destP[1] = destP[3] = destP[_screenLineSize + 1] = destP[_screenLineSize + 3] = palette[(2 * srcP[0]) + 1];
			++srcP;
			destP += 4;
			--xCtr;
		} while (xCtr);

		xCount = savedXCount;
		destP = loopDestP + _screenLineSize * 2;
		srcP = loopSrcP + 320;
		yCount = yCtr - 1;
	} while (yCtr != 1);

	addRefreshRect(destX, destY, destX + width, destY + width);
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
		display8BitRect(surface, _vm->_events->_startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		updateScreen();

		// Added a delay in order to see the fading
		_vm->_events->delay(20);
	}

	// Set the final palette
	setPaletteVGA256(palette);

	// Refresh the screen
	display8BitRect(surface, _vm->_events->_startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	updateScreen();
}

/**
 * Fade out. the step number is determine by parameter.
 */
void GraphicsManager::fadeOut(const byte *palette, int step, const byte *surface) {
	byte palData[PALETTE_BLOCK_SIZE];
	if ((step > 1) && (palette) && (!_vm->_events->_escKeyFl)) {
		int fadeStep = step;
		for (int fadeIndex = 0; fadeIndex < fadeStep; fadeIndex++) {
			for (int palOffset = 0; palOffset < PALETTE_BLOCK_SIZE; palOffset += 3) {
				palData[palOffset + 0] = (fadeStep - fadeIndex - 1) * palette[palOffset + 0] / (fadeStep - 1);
				palData[palOffset + 1] = (fadeStep - fadeIndex - 1) * palette[palOffset + 1] / (fadeStep - 1);
				palData[palOffset + 2] = (fadeStep - fadeIndex - 1) * palette[palOffset + 2] / (fadeStep - 1);
			}

			setPaletteVGA256(palData);
			display8BitRect(surface, _vm->_events->_startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			updateScreen();

			_vm->_events->delay(20);
		}
	}

	// No initial palette, or end of fading
	for (int i = 0; i < PALETTE_BLOCK_SIZE; i++)
		palData[i] = 0;

	setPaletteVGA256(palData);
	display8BitRect(surface, _vm->_events->_startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);

	updateScreen();
}

/**
 * Short fade in. The step number is 1, the default step number is also set to 1.
 */
void GraphicsManager::fadeInShort() {
	_fadeDefaultSpeed = 1;
	fadeIn(_palette, 1, (const byte *)_frontBuffer);
}

/**
 * Short fade out. The step number is 1, the default step number is also set to 1.
 */
void GraphicsManager::fadeOutShort() {
	_fadeDefaultSpeed = 1;
	fadeOut(_palette, 1, (const byte *)_frontBuffer);
}

/**
 * Long fade in. The step number is 20, the default step number is also set to 15.
 */
void GraphicsManager::fadeInLong() {
	_fadeDefaultSpeed = 15;
	fadeIn(_palette, 20, (const byte *)_frontBuffer);
}

/**
 * Long fade out. The step number is 20, the default step number is also set to 15.
 */
void GraphicsManager::fadeOutLong() {
	_fadeDefaultSpeed = 15;
	fadeOut(_palette, 20, (const byte *)_frontBuffer);
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
	copy16bFromSurfaceScaleX2(_frontBuffer);
	updateScreen();
}

/**
 * Fade out used by for the breakout mini-game
 */
void GraphicsManager::fadeOutBreakout() {
	byte palette[PALETTE_EXT_BLOCK_SIZE];

	memset(palette, 0, PALETTE_EXT_BLOCK_SIZE);
	setPaletteVGA256(palette);
	copy16bFromSurfaceScaleX2(_frontBuffer);
	updateScreen();
}

void GraphicsManager::setPaletteVGA256(const byte *palette) {
	changePalette(palette);
}

void GraphicsManager::setPaletteVGA256WithRefresh(const byte *palette, const byte *surface) {
	changePalette(palette);
	display8BitRect(surface, _vm->_events->_startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	updateScreen();
}

void GraphicsManager::setColorPercentage(int palIndex, int r, int g, int b) {
	int palOffset = 3 * palIndex;
	_palette[palOffset] = 255 * r / 100;
	_palette[palOffset + 1] = 255 * g / 100;
	_palette[palOffset + 2] = 255 * b / 100;
}

void GraphicsManager::setColorPercentage2(int palIndex, int r, int g, int b) {
	int rv = 255 * r / 100;
	int gv = 255 * g / 100;
	int bv = 255 * b / 100;

	int palOffset = 3 * palIndex;
	_palette[palOffset] = rv;
	_palette[palOffset + 1] = gv;
	_palette[palOffset + 2] = bv;

	WRITE_UINT16(&_paletteBuffer[2 * palIndex], mapRGB(rv, gv, bv));
}

void GraphicsManager::changePalette(const byte *palette) {
	const byte *srcP = &palette[0];
	for (int idx = 0; idx < PALETTE_SIZE; ++idx, srcP += 3) {
		WRITE_UINT16(&_paletteBuffer[2 * idx], mapRGB(srcP[0], srcP[1], srcP[2]));
	}
}

uint16 GraphicsManager::mapRGB(byte r, byte g, byte b) {
	Graphics::PixelFormat format = g_system->getScreenFormat();

	return (r >> format.rLoss) << format.rShift
			| (g >> format.gLoss) << format.gShift
			| (b >> format.bLoss) << format.bShift;
}

void GraphicsManager::updateScreen() {
	// Display any aras of the screen that need refreshing
	displayDirtyRects();
	displayRefreshRects();

	// Extra checks for debug information
	if (_showZones)
		displayZones();

	if (_showLines)
		displayLines();

	// Update the screen
	g_system->updateScreen();
	debugC(1, kDebugGraphics, "updateScreen()");
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

	lockScreen();
	assert(_videoPtr);

	for (;;) {
		byte srcByte = srcP[0];
		if (srcByte >= 222) {
			if (srcByte == kByteStop)
				break;

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
			break;
		}

		if (srcByte > 210) {
			if (srcByte == 211) {
				int pixelCount = srcP[1];
				int pixelIndex = srcP[2];
				byte *destP = (byte *)_videoPtr + destOffset * 2;
				destOffset += pixelCount;

				while (pixelCount--) {
					destP[0] = _palettePixels[2 * pixelIndex];
					destP[1] = _palettePixels[(2 * pixelIndex) + 1];
					destP += 2;
				}

				srcP += 3;
			} else {
				int pixelCount = srcByte - 211;
				int pixelIndex = srcP[1];
				byte *destP = (byte *)_videoPtr + destOffset * 2;
				destOffset += pixelCount;

				while (pixelCount--) {
					destP[0] = _palettePixels[2 * pixelIndex];
					destP[1] = _palettePixels[(2 * pixelIndex) + 1];
					destP += 2;
				}

				srcP += 2;
			}
		} else {
			byte *destP = (byte *)_videoPtr + destOffset * 2;
			destP[0] = _palettePixels[2 * srcByte];
			destP[1] = _palettePixels[(2 * srcByte) + 1];
			++srcP;
			++destOffset;
		}
	}
	unlockScreen();
}

void GraphicsManager::copyVideoVbe16a(const byte *srcData) {
	byte srcByte;
	int destOffset = 0;
	const byte *srcP = srcData;

	lockScreen();
	for (;;) {
		srcByte = srcP[0];
		if (srcByte == kByteStop)
			break;
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

		WRITE_LE_UINT16((byte *)_videoPtr + destOffset * 2, READ_LE_UINT16(_palettePixels + 2 * srcByte));
		++srcP;
		++destOffset;
	}
	unlockScreen();
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
void GraphicsManager::drawVesaSprite(byte *surface, const byte *spriteData, int xp, int yp, int spriteIndex) {
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
	_clipX1 = width;
	if ((xp + width) <= _minX + 300)
		return;
	if (xp < _minX + 300) {
		_posXClipped = _minX + 300 - xp;
		_clipFl = true;
	}

	// Clip Y
	_clipY1 = height;
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

		_clipX1 = xAmount - 10;
		_clipFl = true;
	}

	// Clip Y1
	if (yp >= _maxY + 300)
		return;
	if (yp + height > _maxY + 300) {
		int yAmount = height + 10 - (yp + height - (_maxY + 300));
		if (yAmount <= 10)
			return;

		// _clipY1 is always positive thanks to the previous check
		_clipY1 = yAmount - 10;
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
		for (int yc = 0; yc < _clipY1; ++yc, destP += _lineNbr2) {
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
						if (_posYClipped == 0 && xc >= _posXClipped && xc < _clipX1)
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
		if (_vm->_animMan->_animBqe[idx]._enabledFl)
			_vm->_objectsMan->hideBob(idx);
	}

	_vm->_events->refreshScreenAndEvents();
	_vm->_events->refreshScreenAndEvents();

	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_animMan->_animBqe[idx]._enabledFl)
			_vm->_objectsMan->resetBob(idx);
	}

	for (int idx = 1; idx < 36; ++idx) {
		_vm->_objectsMan->_lockedAnims[idx]._enableFl = false;
	}

	for (int idx = 1; idx <= 20; ++idx) {
		_vm->_animMan->_animBqe[idx]._enabledFl = false;
	}
}

void GraphicsManager::displayAllBob() {
	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_animMan->_animBqe[idx]._enabledFl)
			_vm->_objectsMan->displayBob(idx);
	}
}

void GraphicsManager::resetDirtyRects() {
	_dirtyRects.clear();
}

void GraphicsManager::resetRefreshRects() {
	_refreshRects.clear();
}

// Add a game area dirty rectangle
void GraphicsManager::addDirtyRect(int x1, int y1, int x2, int y2) {
	x1 = CLIP(x1, _minX, _maxX);
	y1 = CLIP(y1, _minY, _maxY);
	x2 = CLIP(x2, _minX, _maxX);
	y2 = CLIP(y2, _minY, _maxY);

	if ((x2 > x1) && (y2 > y1))
		addRectToArray(_dirtyRects, Common::Rect(x1, y1, x2, y2));
}

// Add a refresh rect
void GraphicsManager::addRefreshRect(int x1, int y1, int x2, int y2) {
	x1 = MAX(x1, 0);
	y1 = MAX(y1, 0);
	x2 = MIN(x2, SCREEN_WIDTH);
	y2 = MIN(y2, SCREEN_HEIGHT);

	if ((x2 > x1) && (y2 > y1))
		addRectToArray(_refreshRects, Common::Rect(x1, y1, x2, y2));
}

void GraphicsManager::addRectToArray(Common::Array<Common::Rect> &rects, const Common::Rect &newRect) {
	// Scan for an intersection with existing rects
	uint rectIndex;
	for (rectIndex = 0; rectIndex < rects.size(); ++rectIndex) {
		Common::Rect &r = rects[rectIndex];

		if (r.intersects(newRect)) {
			// Rect either intersects or is completely inside existing one, so extend existing one as necessary
			r.extend(newRect);
			break;
		}
	}
	if (rectIndex == rects.size()) {
		// Rect not intersecting any existing one, so add it in
		assert(rects.size() < DIRTY_RECTS_SIZE);
		rects.push_back(newRect);
	}

	// Take care of merging the existing rect list. This is done as a separate check even if
	// a previous extending above has been done, since the merging of the new rect above may
	// result in further rects now able to be merged

	for (int srcIndex = rects.size() - 1; srcIndex > 0; --srcIndex) {
		const Common::Rect &srcRect = rects[srcIndex];

		// Loop through all the other rects to see if it intersects them
		for (int destIndex = srcIndex - 1; destIndex >= 0; --destIndex) {
			if (rects[destIndex].intersects(srcRect)) {
				// Found an intersection, so extend the found one, and delete the original
				rects[destIndex].extend(srcRect);
				rects.remove_at(srcIndex);
				break;
			}
		}
	}
}

// Draw any game dirty rects onto the screen intermediate surface
void GraphicsManager::displayDirtyRects() {
	if (_dirtyRects.size() == 0)
		return;

	lockScreen();

	// Refresh the entire screen
	for (uint idx = 0; idx < _dirtyRects.size(); ++idx) {
		Common::Rect &r = _dirtyRects[idx];
		Common::Rect dstRect;

		if (_vm->_events->_breakoutFl) {
			displayScaled8BitRect(_frontBuffer, r.left, r.top, r.right - r.left, r.bottom - r.top, r.left, r.top);
			dstRect.left = r.left * 2;
			dstRect.top = r.top * 2 + 30;
			dstRect.setWidth((r.right - r.left) * 2);
			dstRect.setHeight((r.bottom - r.top) * 2);
		} else if (r.right > _vm->_events->_startPos.x && r.left < _vm->_events->_startPos.x + SCREEN_WIDTH) {
			r.left = MAX<int16>(r.left, _vm->_events->_startPos.x);
			r.right = MIN<int16>(r.right, (int16)_vm->_events->_startPos.x + SCREEN_WIDTH);

			display8BitRect(_frontBuffer, r.left, r.top, r.right - r.left, r.bottom - r.top, r.left - _vm->_events->_startPos.x, r.top);

			dstRect.left = r.left - _vm->_events->_startPos.x;
			dstRect.top = r.top;
			dstRect.setWidth(r.right - r.left);
			dstRect.setHeight(r.bottom - r.top);
		}

		// If it's a valid rect, then add it to the list of areas to refresh on the screen
		if (dstRect.isValidRect() && dstRect.width() > 0 && dstRect.height() > 0)
			addRectToArray(_refreshRects, dstRect);
	}

	unlockScreen();
	resetDirtyRects();
}

void GraphicsManager::displayRefreshRects() {
	debugC(1, kDebugGraphics, "displayRefreshRects() start");
	Graphics::Surface *screenSurface = nullptr;
	if (_showDirtyRects) {
		screenSurface = g_system->lockScreen();
		g_system->copyRectToScreen(_screenBuffer, _screenLineSize, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		debugC(1, kDebugGraphics, "\tcopyRectToScreen(_screenBuffer, %d, %d, %d, %d, %d) - Full Blit", _screenLineSize, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	// Loop through copying over any  specified rects to the screen
	for (uint idx = 0; idx < _refreshRects.size(); ++idx) {
		const Common::Rect &r = _refreshRects[idx];

		byte *srcP = _screenBuffer + _screenLineSize * r.top + (r.left * 2);
		g_system->copyRectToScreen(srcP, _screenLineSize, r.left, r.top, r.width(), r.height());
		debugC(1, kDebugGraphics, "\tcopyRectToScreen(_screenBuffer[%d][%d], %d, %d, %d, %d, %d) - Rect Blit", (r.left * 2), (_screenLineSize * r.top), _screenLineSize, r.left, r.top, r.width(), r.height());

		if (_showDirtyRects)
			screenSurface->frameRect(r, 0xffffff);
	}

	if (_showDirtyRects)
		g_system->unlockScreen();

	resetRefreshRects();
	debugC(1, kDebugGraphics, "displayRefreshRects() end");
}

/**
 * Display any zones for the current room
 */
void GraphicsManager::displayZones() {
	Graphics::Surface *screenSurface = g_system->lockScreen();

	for (int bobZoneId = 0; bobZoneId <= 48; bobZoneId++) {
		int bobId = _vm->_linesMan->_bobZone[bobZoneId];
		if (bobId) {
			// Get the rectangle for the zone
			Common::Rect r(_vm->_objectsMan->_bob[bobId]._oldX, _vm->_objectsMan->_bob[bobId]._oldY,
				_vm->_objectsMan->_bob[bobId]._oldX + _vm->_objectsMan->_bob[bobId]._oldWidth,
				_vm->_objectsMan->_bob[bobId]._oldY + _vm->_objectsMan->_bob[bobId]._oldHeight);

			displayDebugRect(screenSurface, r, 0xff0000);
		}
	}

	for (int squareZoneId = 0; squareZoneId <= 99; squareZoneId++) {
		if (_vm->_linesMan->_zone[squareZoneId]._enabledFl && _vm->_linesMan->_squareZone[squareZoneId]._enabledFl) {
			Common::Rect r(_vm->_linesMan->_squareZone[squareZoneId]._left, _vm->_linesMan->_squareZone[squareZoneId]._top,
				_vm->_linesMan->_squareZone[squareZoneId]._right, _vm->_linesMan->_squareZone[squareZoneId]._bottom);

			displayDebugRect(screenSurface, r, 0x00ff00);
		}
	}

	g_system->unlockScreen();
}

/**
 * Display any zones for the current room
 */
void GraphicsManager::displayLines() {
	Graphics::Surface *screenSurface = g_system->lockScreen();

	for (int lineIndex = 0; lineIndex < _vm->_linesMan->_linesNumb; lineIndex++) {
		int i = 0;
		do {
			int x = _vm->_linesMan->_lineItem[lineIndex]._lineData[i] - _scrollPosX;
			int y = _vm->_linesMan->_lineItem[lineIndex]._lineData[i+1];
			if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
				WRITE_UINT16(screenSurface->getBasePtr(x, y), 0xffff);
			}
			i += 2;
		}
		while(_vm->_linesMan->_lineItem[lineIndex]._lineData[i] != -1);
	}

	g_system->unlockScreen();
}


void GraphicsManager::displayDebugRect(Graphics::Surface *surface, const Common::Rect &srcRect, uint32 color) {
	Common::Rect r = srcRect;

	// Move for scrolling offset and adjust to crop on-screen
	r.translate(-_scrollPosX, 0);
	r.left = MAX(r.left, (int16)0);
	r.top = MAX(r.top, (int16)0);
	r.right = MIN(r.right, (int16)SCREEN_WIDTH);
	r.bottom = MIN(r.bottom, (int16)SCREEN_HEIGHT);

	// If there's an on-screen portion, display it
	if (r.isValidRect())
		surface->frameRect(r, color);
}

/**
 * Fast Display of either a compressed or vesa sprite
 */
void GraphicsManager::fastDisplay(const byte *spriteData, int xp, int yp, int spriteIndex, bool addSegment) {
	int width = _vm->_objectsMan->getWidth(spriteData, spriteIndex);
	int height = _vm->_objectsMan->getHeight(spriteData, spriteIndex);

	if (*spriteData == 78) {
		drawCompressedSprite(_backBuffer, spriteData, xp + 300, yp + 300, spriteIndex, 0, 0, false);
		drawCompressedSprite(_frontBuffer, spriteData, xp + 300, yp + 300, spriteIndex, 0, 0, false);
	} else {
		drawVesaSprite(_frontBuffer, spriteData, xp + 300, yp + 300, spriteIndex);
		drawVesaSprite(_backBuffer, spriteData, xp + 300, yp + 300, spriteIndex);
	}
	if (addSegment)
		addDirtyRect(xp, yp, xp + width, yp + height);
}

void GraphicsManager::fastDisplay2(const byte *objectData, int xp, int yp, int idx, bool addSegment) {
	int width = _vm->_objectsMan->getWidth(objectData, idx);
	int height = _vm->_objectsMan->getHeight(objectData, idx);
	if (*objectData == 78) {
		drawCompressedSprite(_backBuffer, objectData, xp + 300, yp + 300, idx, 0, 0, false);
		drawCompressedSprite(_frontBuffer, objectData, xp + 300, yp + 300, idx, 0, 0, false);
	} else {
		drawVesaSprite(_frontBuffer, objectData, xp + 300, yp + 300, idx);
		drawVesaSprite(_backBuffer, objectData, xp + 300, yp + 300, idx);
	}
	if (addSegment)
		addDirtyRect(xp, yp, xp + width, yp + height);
}

/**
 * Copy from surface to video buffer, scale 2x.
 */
void GraphicsManager::copy16bFromSurfaceScaleX2(const byte *surface) {
	byte *palPtr;
	int curPixel;

	lockScreen();

	assert(_videoPtr);
	const byte *curSurface = surface;
	byte *destPtr = 30 * _screenLineSize + (byte *)_videoPtr;
	for (int y = 200; y; y--) {
		byte *oldDestPtr = destPtr;
		for (int x = 320; x; x--) {
			curPixel = 2 * *curSurface;
			palPtr = _palettePixels + curPixel;
			destPtr[0] = destPtr[2] = destPtr[_screenLineSize] = destPtr[_screenLineSize + 2] = palPtr[0];
			destPtr[1] = destPtr[3] = destPtr[_screenLineSize + 1] = destPtr[_screenLineSize + 3] = palPtr[1];
			++curSurface;
			destPtr += 4;
		}
		destPtr = _screenLineSize * 2 + oldDestPtr;
	}

	unlockScreen();
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
int GraphicsManager::zoomIn(int val, int percentage) {
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
void GraphicsManager::drawCompressedSprite(byte *surface, const byte *srcData, int xp300, int yp300, int frameIndex, int zoom1, int zoom2, bool flipFl) {
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
	_clipX1 = 0;
	_clipY1 = 0;
	if ((xp300 <= _minX) || (yp300 <= _minY) || (xp300 >= _maxX + 300) || (yp300 >= _maxY + 300))
		return;

	// Clipped values are greater or equal to zero, thanks to the previous test
	_clipX1 = _maxX + 300 - xp300;
	_clipY1 = _maxY + 300 - yp300;

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
		_enlargedX = 0;
		_enlargedY = 0;
		_enlargedYFl = false;
		_enlargedXFl = false;
		_width = spriteWidth;
		int zoomedWidth = zoomIn(spriteWidth, zoom2);
		int zoomedHeight = zoomIn(spriteHeight1, zoom2);
		if (flipFl) {
			byte *clippedDestP = zoomedWidth + dest1P;
			if (_posYClipped) {
				if (_posYClipped < 0 || _posYClipped >= zoomedHeight)
					return;
				int hiddenHeight = 0;
				while (zoomIn(++hiddenHeight, zoom2) < _posYClipped)
					;
				spritePixelsP += _width * hiddenHeight;
				clippedDestP += _lineNbr2 * _posYClipped;
				zoomedHeight -= _posYClipped;
			}
			if (zoomedHeight > _clipY1)
				zoomedHeight = _clipY1;
			if (_posXClipped) {
				if (_posXClipped >= zoomedWidth)
					return;
				zoomedWidth -= _posXClipped;
			}
			if (zoomedWidth > _clipX1) {
				int clippedZoomedWidth = zoomedWidth - _clipX1;
				clippedDestP -= clippedZoomedWidth;
				int closestWidth = 0;
				while (zoomIn(++closestWidth, zoom2) < clippedZoomedWidth)
					;
				spritePixelsP += closestWidth;
				zoomedWidth = _clipX1;
			}
			int curHeight;
			do {
				for (;;) {
					curHeight = zoomedHeight;
					byte *oldDestP = clippedDestP;
					const byte *oldSpritePixelsP = spritePixelsP;
					_enlargedXFl = false;
					_enlargedX = 0;
					for (int i = zoomedWidth; i; _enlargedXFl = false, i--) {
						for (;;) {
							if (*spritePixelsP)
								*clippedDestP = *spritePixelsP;
							--clippedDestP;
							++spritePixelsP;
							if (!_enlargedXFl)
								_enlargedX += zoom2;
							if (_enlargedX >= 0 && _enlargedX < 100)
								break;
							_enlargedX -= 100;
							--spritePixelsP;
							_enlargedXFl = true;
							--i;
							if (!i)
								break;
						}
					}
					spritePixelsP = _width + oldSpritePixelsP;
					clippedDestP = _lineNbr2 + oldDestP;
					if (!_enlargedYFl)
						_enlargedY += zoom2;
					if (_enlargedY  >= 0 && _enlargedY < 100)
						break;
					_enlargedY -= 100;
					spritePixelsP = oldSpritePixelsP;
					_enlargedYFl = true;
					zoomedHeight = curHeight - 1;
					if (curHeight == 1)
						return;
				}
				_enlargedYFl = false;
				zoomedHeight = curHeight - 1;
			} while (curHeight != 1);
		} else {
			if (_posYClipped) {
				if (_posYClipped >= zoomedHeight)
					return;
				int closerHeight = 0;
				while (zoomIn(++closerHeight, zoom2) < _posYClipped)
					;
				spritePixelsP += _width * closerHeight;
				dest1P += _lineNbr2 * _posYClipped;
				zoomedHeight -= _posYClipped;
			}
			if (zoomedHeight > _clipY1)
				zoomedHeight = _clipY1;
			if (_posXClipped) {
				if (_posXClipped >= zoomedWidth)
					return;
				int closerWidth = 0;
				while (zoomIn(++closerWidth, zoom2) < _posXClipped)
					;
				spritePixelsP += closerWidth;
				dest1P += _posXClipped;
				zoomedWidth = zoomedWidth - _posXClipped;
			}
			if (zoomedWidth > _clipX1)
				zoomedWidth = _clipX1;

			int curHeight;
			do {
				for (;;) {
					curHeight = zoomedHeight;
					byte *oldDest1P = dest1P;
					const byte *oldSpritePixelsP = spritePixelsP;
					_enlargedXFl = false;
					_enlargedX = 0;
					for (int i = zoomedWidth; i; _enlargedXFl = false, i--) {
						for (;;) {
							if (*spritePixelsP)
								*dest1P = *spritePixelsP;
							++dest1P;
							++spritePixelsP;
							if (!_enlargedXFl)
								_enlargedX += zoom2;
							if (_enlargedX >= 0 && _enlargedX < 100)
								break;
							_enlargedX -= 100;
							--spritePixelsP;
							_enlargedXFl = true;
							--i;
							if (!i)
								break;
						}
					}
					spritePixelsP = _width + oldSpritePixelsP;
					dest1P = _lineNbr2 + oldDest1P;
					if (!_enlargedYFl)
						_enlargedY += zoom2;
					if (_enlargedY >= 0 && _enlargedY < 100)
						break;
					_enlargedY -= 100;
					spritePixelsP = oldSpritePixelsP;
					_enlargedYFl = true;
					zoomedHeight = curHeight - 1;
					if (curHeight == 1)
						return;
				}
				_enlargedYFl = false;
				zoomedHeight = curHeight - 1;
			} while (curHeight != 1);
		}
	} else if (zoom1) {
		_reduceX = 0;
		_reducedY = 0;
		_width = spriteWidth;
		_zoomOutFactor = zoom1;
		if (zoom1 < 100) {
			int zoomedSpriteWidth = zoomOut(spriteWidth, _zoomOutFactor);
			if (flipFl) {
				byte *curDestP = zoomedSpriteWidth + dest1P;
				do {
					byte *oldDestP = curDestP;
					_reducedY += _zoomOutFactor;
					if (_reducedY >= 0 && _reducedY < 100) {
						_reduceX = 0;
						int curWidth = zoomedSpriteWidth;
						for (int i = _width; i; i--) {
							_reduceX += _zoomOutFactor;
							if (_reduceX >= 0 && _reduceX < 100) {
								if (curWidth >= _posXClipped && curWidth < _clipX1 && *spritePixelsP)
									*curDestP = *spritePixelsP;
								--curDestP;
								++spritePixelsP;
								--curWidth;
							} else {
								_reduceX -= 100;
								++spritePixelsP;
							}
						}
						curDestP = _lineNbr2 + oldDestP;
					} else {
						_reducedY -= 100;
						spritePixelsP += _width;
					}
					--spriteHeight2;
				} while (spriteHeight2);
			} else {
				do {
					int oldSpriteHeight = spriteHeight2;
					byte *oldDest1P = dest1P;
					_reducedY += _zoomOutFactor;
					if (_reducedY >= 0 && _reducedY < 100) {
						_reduceX = 0;
						int curX = 0;
						for (int i = _width; i; i--) {
							_reduceX += _zoomOutFactor;
							if (_reduceX >= 0 && _reduceX < 100) {
								if (curX >= _posXClipped && curX < _clipX1 && *spritePixelsP)
									*dest1P = *spritePixelsP;
								++dest1P;
								++spritePixelsP;
								++curX;
							} else {
								_reduceX -= 100;
								++spritePixelsP;
							}
						}
						spriteHeight2 = oldSpriteHeight;
						dest1P = _lineNbr2 + oldDest1P;
					} else {
						_reducedY -= 100;
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
			if (spriteHeight1 > _clipY1)
				spriteHeight1 = _clipY1;

			if (_posXClipped >= spriteWidth)
				return;
			spriteWidth -= _posXClipped;

			if (spriteWidth > _clipX1) {
				int clippedWidth = spriteWidth - _clipX1;
				spritePixelsP += clippedWidth;
				dest2P -= clippedWidth;
				spriteWidth = _clipX1;
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
			if (spriteHeight1 > _clipY1)
				spriteHeight1 = _clipY1;
			if (_posXClipped) {
				if (_posXClipped >= spriteWidth)
					return;
				spritePixelsP += _posXClipped;
				dest1P += _posXClipped;
				spriteWidth -= _posXClipped;
			}
			if (spriteWidth > _clipX1)
				spriteWidth = _clipX1;
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
		copyRect(surface, left, top, croppedWidth, croppedHeight, destSurface, destX, destY);
		addDirtyRect(left, top, left + croppedWidth, top + height2);
	}
}

void GraphicsManager::copyRect(const byte *srcSurface, int x1, int y1, uint16 width, int height, byte *destSurface, int destX, int destY) {
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
void GraphicsManager::displayFont(byte *surface, const byte *spriteData, int xp, int yp, int characterIndex, int color) {
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
					destByte = color;
				*destP = destByte;
			}

			++destP;
			++spritePixelsP;
		}
		destP = _lineNbr2 + destLineP;
		spriteHeight = yCtr - 1;
	} while (yCtr != 1);
}

void GraphicsManager::initScreen(const Common::Path &file, int mode, bool initializeScreen) {
	Common::Path filename(file);
	filename.appendInPlace(".ini");
	bool fileFoundFl = false;

	byte *ptr = _vm->_fileIO->searchCat(filename, RES_INI, fileFoundFl);

	if (!fileFoundFl) {
		ptr = _vm->_fileIO->loadFile(filename);
	}

	if (!mode) {
		filename = file;
		filename.appendInPlace(".spr");
		_vm->_globals->_levelSpriteBuf = _vm->_globals->freeMemory(_vm->_globals->_levelSpriteBuf);
		if (initializeScreen) {
			fileFoundFl = false;
			_vm->_globals->_levelSpriteBuf = _vm->_fileIO->searchCat(filename, RES_SLI, fileFoundFl);
			if (!fileFoundFl) {
				_vm->_globals->_levelSpriteBuf = _vm->_fileIO->loadFile(filename);
			} else {
				_vm->_globals->_levelSpriteBuf = _vm->_fileIO->loadFile("RES_SLI.RES");
			}
		}
	}
	if (READ_BE_UINT24(ptr) != MKTAG24('I', 'N', 'I')) {
		error("Invalid INI File %s", file.toString().c_str());
	} else {
		bool doneFlag = false;
		int dataOffset = 1;

		do {
			int dataVal1 = _vm->_script->handleOpcode(ptr + 20 * dataOffset);
			if (dataVal1 == -1 || _vm->shouldQuit())
				return;

			if (dataVal1 == 2)
				dataOffset =  _vm->_script->handleGoto((ptr + 20 * dataOffset));
			if (dataVal1 == 3)
				dataOffset =  _vm->_script->handleIf(ptr, dataOffset);
			if (dataOffset == -1)
				error("Error, defective IFF");
			if (dataVal1 == 1 || dataVal1 == 4)
				++dataOffset;
			if (!dataVal1 || dataVal1 == 5)
				doneFlag = true;
		} while (!doneFlag);
	}
	_vm->_globals->freeMemory(ptr);
	_vm->_globals->_answerBuffer = _vm->_globals->freeMemory(_vm->_globals->_answerBuffer);

	filename = file;
	filename.appendInPlace(".rep");
	fileFoundFl = false;
	byte *dataP = _vm->_fileIO->searchCat(filename, RES_REP, fileFoundFl);
	if (!fileFoundFl)
		dataP = _vm->_fileIO->loadFile(filename);

	_vm->_globals->_answerBuffer = dataP;
	_vm->_objectsMan->_forceZoneFl = true;
	_vm->_objectsMan->_changeVerbFl = false;
}

void GraphicsManager::displayScreen(bool initPalette) {
	if (initPalette)
		initColorTable(50, 65, _palette);

	if (_lineNbr == SCREEN_WIDTH)
		fillSurface(_frontBuffer, _colorTable, SCREEN_WIDTH * SCREEN_HEIGHT);
	else if (_lineNbr == (SCREEN_WIDTH * 2))
		fillSurface(_frontBuffer, _colorTable, SCREEN_WIDTH * SCREEN_HEIGHT * 2);

	display8BitRect(_frontBuffer, _vm->_events->_startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	memcpy(_backBuffer, _frontBuffer, 614399);
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
	_zoomOutFactor = zoom;
	_width = width;
	_reduceX = 0;
	_reducedY = 0;
	if (zoom < 100) {
		for (int yCtr = 0; yCtr < height; ++yCtr, srcP += _lineNbr2) {
			_reducedY += _zoomOutFactor;
			if (_reducedY < 100) {
				_reduceX = 0;
				const byte *lineSrcP = srcP;

				for (int xCtr = 0; xCtr < _width; ++xCtr) {
					_reduceX += _zoomOutFactor;
					if (_reduceX < 100) {
						*destP++ = *lineSrcP++;
					} else {
						_reduceX -= 100;
						++lineSrcP;
					}
				}
			} else {
				_reducedY -= 100;
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

/**
 * Backup the current screen
 */
void GraphicsManager::backupScreen() {
	// Allocate a new data block for the screen, if necessary
	if (_vm->_graphicsMan->_backupScreen == nullptr)
		_vm->_graphicsMan->_backupScreen = _vm->_globals->allocMemory(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

	// Backup the screen
	Common::copy(_vm->_graphicsMan->_backBuffer, _vm->_graphicsMan->_backBuffer +
		SCREEN_WIDTH * 2 * SCREEN_HEIGHT, _vm->_graphicsMan->_backupScreen);
}

/**
 * Restore a previously backed up screen
 */
void GraphicsManager::restoreScreen() {
	assert(_vm->_graphicsMan->_backupScreen);

	// Restore the screen and free the buffer
	Common::copy(_vm->_graphicsMan->_backupScreen, _vm->_graphicsMan->_backupScreen +
		SCREEN_WIDTH * 2 * SCREEN_HEIGHT, _vm->_graphicsMan->_backBuffer);
	_vm->_globals->freeMemory(_vm->_graphicsMan->_backupScreen);
	_backupScreen = nullptr;
}

} // End of namespace Hopkins
