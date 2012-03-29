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
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "common/endian.h"
#include "common/system.h"
#include "graphics/palette.h"
#include "mortevielle/graphics.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"

namespace Mortevielle {

/*-------------------------------------------------------------------------*
 * Palette Manager
 *
 *-------------------------------------------------------------------------*/

/**
 * Set palette entries from the 64 colour available EGA palette
 */
void PaletteManager::setPalette(const int *palette, uint idx, uint size) {
	assert((idx + size) <= 16);

	// Build up the EGA palette
	byte egaPalette[64 * 3];

	byte *p = &egaPalette[0];
	for (int i = 0; i < 64; ++i) {
		*p++ = (i >> 2 & 1) * 0xaa + (i >> 5 & 1) * 0x55;
		*p++ = (i >> 1 & 1) * 0xaa + (i >> 4 & 1) * 0x55;
		*p++ = (i      & 1) * 0xaa + (i >> 3 & 1) * 0x55;
	}

	// Loop through setting palette colours based on the passed indexes
	for (; size > 0; --size, ++idx) {
		int palIndex = palette[idx];
		assert(palIndex < 64);

		const byte *pRgb = (const byte *)&egaPalette[palIndex * 3];
		g_system->getPaletteManager()->setPalette(pRgb, idx, 1);
	}
}

/**
 * Set the default EGA palette
 */
void PaletteManager::setDefaultPalette() {
	int defaultPalette[16] = { 0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63 };
	setPalette(defaultPalette, 0, 16);
}

/*-------------------------------------------------------------------------*
 * Image decoding
 *
 * The code in this section is responsible for decoding image resources.
 * Images are broken down into rectangular sections, which can use one
 * of 18 different encoding methods.
 *-------------------------------------------------------------------------*/

#define INCR_TAIX { if (_xSize & 1) ++_xSize; }
#define DEFAULT_WIDTH (SCREEN_WIDTH / 2)
#define BUFFER_SIZE 40000

void GfxSurface::decode(const byte *pSrc) {
	_width = _height = 0;
	// If no transparency, use invalid (for EGA) palette index of 16. Otherwise get index to use
	_transparency = (*pSrc == 0) ? 16 : *(pSrc + 2);
	bool offsetFlag = *pSrc++ == 0;
	int entryCount = *pSrc++;
	pSrc += 2;

	if (offsetFlag)
		pSrc += 30;

	// First run through the data to calculate starting offsets
	const byte *p = pSrc;
	_offset.x = _offset.y = 999;

	assert(entryCount > 0);
	for (int idx = 0; idx < entryCount; ++idx) {
		_xp = READ_BE_UINT16(p + 4);
		if (_xp < _offset.x)
			_offset.x = _xp;

		_yp = READ_BE_UINT16(p + 6);
		if (_yp < _offset.y)
			_offset.y = _yp;

		// Move to next entry
		int size = READ_BE_UINT16(p) + READ_BE_UINT16(p + 2);
		if ((size % 2) == 1)
			++size;

		p += size + 14;
	}

	// Temporary output buffer
	byte outputBuffer[65536];
	Common::fill(&outputBuffer[0], &outputBuffer[65536], _transparency);

	byte *pDest = &outputBuffer[0];
	const byte *pSrcStart = pSrc;
	const byte *pLookup = NULL;

	byte lookupTable[BUFFER_SIZE];
	byte srcBuffer[BUFFER_SIZE];

	// Main processing loop
	for (int entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
		int lookupBytes = READ_BE_UINT16(pSrc);
		int srcSize = READ_BE_UINT16(pSrc + 2);
		_xp = READ_BE_UINT16(pSrc + 4) - _offset.x;
		_yp = READ_BE_UINT16(pSrc + 6) - _offset.y;
		assert((_xp >= 0) && (_yp >= 0) && (_xp < SCREEN_WIDTH) && (_yp < SCREEN_ORIG_HEIGHT));
		pSrc += 8;

		int decomCode = READ_BE_UINT16(pSrc);
		_xSize = READ_BE_UINT16(pSrc + 2) + 1;
		_ySize = READ_BE_UINT16(pSrc + 4) + 1;
		majTtxTty();

		pSrc += 6;
		pDest = &outputBuffer[0];

		_lookupIndex = 0;
		_nibbleFlag = false;

		int decomIndex = 0;
		if (decomCode >> 8) {
			// Build up reference table
			int tableOffset = 0;

			if (decomCode & 1) {
				// Handle decompression of the pattern lookup table
				do {
					int outerCount = desanalyse(pSrc);
					int innerCount = desanalyse(pSrc);

					const byte *pSrcSaved = pSrc;
					bool savedNibbleFlag = _nibbleFlag;
					int savedLookupIndex = _lookupIndex;

					do {
						pSrc = pSrcSaved;
						_nibbleFlag = savedNibbleFlag;
						_lookupIndex = savedLookupIndex;

						for (int idx = 0; idx < innerCount; ++idx, ++tableOffset) {
							assert(tableOffset < BUFFER_SIZE);
							lookupTable[tableOffset] = suiv(pSrc);
						}
					} while (--outerCount > 0);
				} while (_lookupIndex < (lookupBytes - 1));

			} else {
				assert(lookupBytes < BUFFER_SIZE);
				for (int idx = 0; idx < (lookupBytes * 2); ++idx)
					lookupTable[idx] = suiv(pSrc);
			}

			if (_nibbleFlag) {
				++pSrc;
				_nibbleFlag = false;
			}
			if ((lookupBytes + srcSize) & 1)
				++pSrc;

			tableOffset = 0;
			_lookupIndex = 0;

			if (decomCode & 2) {
				// Handle decompression of the temporary source buffer
				do {
					int outerCount = desanalyse(pSrc);
					int innerCount = desanalyse(pSrc);
					_lookupIndex += innerCount;

					if (_nibbleFlag) {
						++pSrc;
						++_lookupIndex;
						_nibbleFlag = false;
					}

					const byte *pStart = pSrc;
					do {
						pSrc = pStart;
						for (int idx = 0; idx < innerCount; ++idx) {
							assert(tableOffset < BUFFER_SIZE);
							srcBuffer[tableOffset++] = *pSrc++;
						}
					} while (--outerCount > 0);
				} while (_lookupIndex < (srcSize - 1));
			} else {
				assert(srcSize < BUFFER_SIZE);
				for (int idx = 0; idx < srcSize; ++idx)
					srcBuffer[idx] = *pSrc++;
			}

			if (_nibbleFlag)
				++pSrc;

			// Switch over to using the decompressed source and lookup buffers
			pSrcStart = pSrc;
			pDest = &outputBuffer[_yp * DEFAULT_WIDTH + _xp];
			pSrc = &srcBuffer[0];
			pLookup = &lookupTable[0] - 1;

			_lookupValue = _lookupIndex = 0;
			_nibbleFlag = false;
			decomIndex = decomCode >> 8;
		}

		// Main decompression switch
		switch (decomIndex) {
		case 0:
			// Draw rect at pos
			pDest = &outputBuffer[_yp * DEFAULT_WIDTH + _xp];
			pSrcStart = pSrc;
			INCR_TAIX;

			for (int yCtr = 0; yCtr < _ySize; ++yCtr, pDest += DEFAULT_WIDTH) {
				byte *pDestLine = pDest;
				for (int xCtr = 0; xCtr < _xSize; ++xCtr) {
					*pDestLine++ = suiv(pSrc);
				}
			}

			pSrcStart += lookupBytes + ((lookupBytes & 1) ? 1 : 0);
			break;

		case 1:
			// Draw rect using horizontal lines alternating left to right, then right to left
			INCR_TAIX;
			for (int yCtr = 0; yCtr < _ySize; ++yCtr) {
				if ((yCtr % 2) == 0) {
					for (int xCtr = 0; xCtr < _xSize; ++xCtr) {
						*pDest++ = csuiv(pSrc, pLookup);
					}
				} else {
					for (int xCtr = 0; xCtr < _xSize; ++xCtr) {
						*--pDest = csuiv(pSrc, pLookup);
					}
				}
				pDest += DEFAULT_WIDTH;
			}
			break;

		case 2:
			// Draw rect alternating top to bottom, bottom to top
			for (int xCtr = 0; xCtr < _xSize; ++xCtr) {
				if ((xCtr % 2) == 0) {
					for (int yCtr = 0; yCtr < _ySize; ++yCtr, pDest += DEFAULT_WIDTH) {
						*pDest = csuiv(pSrc, pLookup);
					}
				} else {
					for (int yCtr = 0; yCtr < _ySize; ++yCtr) {
						pDest -= DEFAULT_WIDTH;
						*pDest = csuiv(pSrc, pLookup);
					}
				}
				++pDest;
			}
			break;

		case 3:
			// Draw horizontal area?
			_thickness = 2;
			horizontal(pSrc, pDest, pLookup);
			break;

		case 4:
			// Draw vertical area?
			_thickness = 2;
			vertical(pSrc, pDest, pLookup);
			break;

		case 5:
			_thickness = 3;
			horizontal(pSrc, pDest, pLookup);
			break;

		case 6:
			_thickness = 4;
			vertical(pSrc, pDest, pLookup);
			break;

		case 7:
			// Draw rect using horizontal lines left to right
			INCR_TAIX;
			for (int yCtr = 0; yCtr < _ySize; ++yCtr, pDest += DEFAULT_WIDTH) {
				byte *pDestLine = pDest;
				for (int xCtr = 0; xCtr < _xSize; ++xCtr)
					*pDestLine++ = csuiv(pSrc, pLookup);
			}
			break;

		case 8:
			// Draw box
			for (int xCtr = 0; xCtr < _xSize; ++xCtr, ++pDest) {
				byte *pDestLine = pDest;
				for (int yCtr = 0; yCtr < _ySize; ++yCtr, pDestLine += DEFAULT_WIDTH)
					*pDestLine = csuiv(pSrc, pLookup);
			}
			break;

		case 9:
			_thickness = 4;
			horizontal(pSrc, pDest, pLookup);
			break;

		case 10:
			_thickness = 6;
			horizontal(pSrc, pDest, pLookup);
			break;

		case 11:
			decom11(pSrc, pDest, pLookup);
			break;

		case 12:
			INCR_TAIX;
			_thickness = _xInc = 1;
			_yInc = DEFAULT_WIDTH;
			_yEnd = _ySize;
			_xEnd = _xSize;
			diag(pSrc, pDest, pLookup);
			break;

		case 13:
			INCR_TAIX;
			_thickness = _xSize;
			_yInc = 1;
			_yEnd = _xSize;
			_xInc = DEFAULT_WIDTH;
			_xEnd = _ySize;
			diag(pSrc, pDest, pLookup);
			break;

		case 14:
			_thickness = _yInc = 1;
			_yEnd = _xSize;
			_xInc = DEFAULT_WIDTH;
			_xEnd = _ySize;
			diag(pSrc, pDest, pLookup);
			break;

		case 15:
			INCR_TAIX;
			_thickness = 2;
			_yInc = DEFAULT_WIDTH;
			_yEnd = _ySize;
			_xInc = 1;
			_xEnd = _xSize;
			diag(pSrc, pDest, pLookup);
			break;

		case 16:
			_thickness = 3;
			_yInc = 1;
			_yEnd = _xSize;
			_xInc = DEFAULT_WIDTH;
			_xEnd = _ySize;
			diag(pSrc, pDest, pLookup);
			break;

		case 17:
			INCR_TAIX;
			_thickness = 3;
			_yInc = DEFAULT_WIDTH;
			_yEnd = _ySize;
			_xInc = 1;
			_xEnd = _xSize;
			diag(pSrc, pDest, pLookup);
			break;

		case 18:
			INCR_TAIX;
			_thickness = 5;
			_yInc = DEFAULT_WIDTH;
			_yEnd = _ySize;
			_xInc = 1;
			_xEnd = _xSize;
			diag(pSrc, pDest, pLookup);
			break;

		default:
			error("Unknown decompression block type %d", decomIndex);
		}

		pSrc = pSrcStart;
		debugC(2, kMortevielleGraphics, "Decoding image block %d position %d,%d size %d,%d method %d",
			entryIndex + 1, _xp, _yp, _width, _height, decomIndex);
	}

	// At this point, the outputBuffer has the data for the image. Initialise the surface
	// with the calculated size, and copy the lines to the surface
	create(_width, _height, Graphics::PixelFormat::createFormatCLUT8());

	for (int yCtr = 0; yCtr < _height; ++yCtr) {
		const byte *copySrc = &outputBuffer[yCtr * DEFAULT_WIDTH];
		byte *copyDest = (byte *)getBasePtr(0, yCtr);

		Common::copy(copySrc, copySrc + _width, copyDest);
	}
}

void GfxSurface::majTtxTty() {
	if (!_yp)
		_width += _xSize;

	if (!_xp)
		_height += _ySize;
}

byte GfxSurface::suiv(const byte *&pSrc) {
	int v = *pSrc;
	if (_nibbleFlag) {
		++pSrc;
		++_lookupIndex;
		_nibbleFlag = false;
		return v & 0xf;
	} else {
		_nibbleFlag = !_nibbleFlag;
		return v >> 4;
	}
}

byte GfxSurface::csuiv(const byte *&pSrc, const byte *&pLookup) {
	assert(pLookup);

	while (!_lookupValue) {
		int v;
		do {
			v = suiv(pSrc) & 0xff;
			_lookupValue += v;
		} while (v == 0xf);
		++pLookup;
	}

	--_lookupValue;
	return *pLookup;
}

int GfxSurface::desanalyse(const byte *&pSrc) {
	int total = 0;
	int v = suiv(pSrc);
	if (v == 15) {
		int v2;
		do {
			v2 = suiv(pSrc);
			total += v2;
		} while (v2 == 15);

		total *= 15;
		v = suiv(pSrc);
	}

	total += v;
	return total;
}

void GfxSurface::horizontal(const byte *&pSrc, byte *&pDest, const byte *&pLookup) {
	INCR_TAIX;
	byte *pDestEnd = pDest + (_ySize - 1) * DEFAULT_WIDTH + _xSize;

	for (;;) {
		// If position is past end point, then skip this line
		if (((_thickness - 1) * DEFAULT_WIDTH) + pDest >= pDestEnd) {
			if (--_thickness == 0)
				break;
			continue;
		}

		bool continueFlag = false;
		do {
			for (int xIndex = 0; xIndex < _xSize; ++xIndex) {
				if ((xIndex % 2) == 0) {
					if (xIndex != 0)
						++pDest;

					// Write out vertical slice top to bottom
					for (int yIndex = 0; yIndex < _thickness; ++yIndex, pDest += DEFAULT_WIDTH)
						*pDest = csuiv(pSrc, pLookup);

					++pDest;
				} else {
					// Write out vertical slice bottom to top
					for (int yIndex = 0; yIndex < _thickness; ++yIndex) {
						pDest -= DEFAULT_WIDTH;
						*pDest = csuiv(pSrc, pLookup);
					}
				}
			}

			if ((_xSize % 2) == 0) {
				int blockSize = _thickness * DEFAULT_WIDTH;
				pDest += blockSize;
				blockSize -= DEFAULT_WIDTH;

				if (pDestEnd < (pDest + blockSize)) {
					do {
						if (--_thickness == 0)
							return;
					} while ((pDest + (_thickness - 1) * DEFAULT_WIDTH) >= pDestEnd);
				}
			} else {
				while ((pDest + (_thickness - 1) * DEFAULT_WIDTH) >= pDestEnd) {
					if (--_thickness == 0)
						return;
				}
			}

			for (int xIndex = 0; xIndex < _xSize; ++xIndex, --pDest) {
				if ((xIndex % 2) == 0) {
					// Write out vertical slice top to bottom
					for (int yIndex = 0; yIndex < _thickness; ++yIndex, pDest += DEFAULT_WIDTH)
						*pDest = csuiv(pSrc, pLookup);
				} else {
					// Write out vertical slice top to bottom
					for (int yIndex = 0; yIndex < _thickness; ++yIndex) {
						pDest -= DEFAULT_WIDTH;
						*pDest = csuiv(pSrc, pLookup);
					}
				}
			}

			if ((_xSize % 2) == 1) {
				++pDest;

				if ((pDest + (_thickness - 1) * DEFAULT_WIDTH) < pDestEnd) {
					continueFlag = true;
					break;
				}
			} else {
				pDest += _thickness * DEFAULT_WIDTH + 1;
				continueFlag = true;
				break;
			}

			++pDest;
		} while (((_thickness - 1) * DEFAULT_WIDTH + pDest) < pDestEnd);

		if (continueFlag)
			continue;

		// Move to next line
		if (--_thickness == 0)
			break;
	}
}

void GfxSurface::vertical(const byte *&pSrc, byte *&pDest, const byte *&pLookup) {
	int drawIndex = 0;

	for (;;) {
		// Reduce thickness as necessary
		while ((drawIndex + _thickness) > _xSize) {
			if (--_thickness == 0)
				return;
		}

		// Loop
		for (int yCtr = 0; yCtr < _ySize; ++yCtr) {
			if ((yCtr % 2) == 0) {
				if (yCtr > 0)
					pDest += DEFAULT_WIDTH;

				drawIndex += _thickness;
				for (int xCtr = 0; xCtr < _thickness; ++xCtr)
					*pDest++ = csuiv(pSrc, pLookup);
			} else {
				pDest += DEFAULT_WIDTH;
				drawIndex -= _thickness;
				for (int xCtr = 0; xCtr < _thickness; ++xCtr)
					*--pDest = csuiv(pSrc, pLookup);
			}
		}
		if ((_ySize % 2) == 0) {
			pDest += _thickness;
			drawIndex += _thickness;
		}

		while (_xSize < (drawIndex + _thickness)) {
			if (--_thickness == 0)
				return;
		}

		// Loop
		for (int yCtr = 0; yCtr < _ySize; ++yCtr) {
			if ((yCtr % 2) == 0) {
				if (yCtr > 0)
					pDest -= DEFAULT_WIDTH;

				drawIndex += _thickness;

				for (int xCtr = 0; xCtr < _thickness; ++xCtr)
					*pDest++ = csuiv(pSrc, pLookup);
			} else {
				pDest -= DEFAULT_WIDTH;
				drawIndex -= _thickness;

				for (int xCtr = 0; xCtr < _thickness; ++xCtr)
					*--pDest = csuiv(pSrc, pLookup);
			}
		}
		if ((_ySize % 2) == 0) {
			pDest += _thickness;
			drawIndex += _thickness;
		}
	}
}

void GfxSurface::decom11(const byte *&pSrc, byte *&pDest, const byte *&pLookup) {
	int yPos = 0, drawIndex = 0;
	_yInc = DEFAULT_WIDTH;
	_xInc = -1;
	--_xSize;
	--_ySize;

	int areaNum = 0;
	while (areaNum != -1) {
		switch (areaNum) {
		case 0:
			*pDest = csuiv(pSrc, pLookup);
			areaNum = 1;
			break;

		case 1:
			increments(pDest);

			if (!drawIndex) {
				NIH();
				NIV();

				if (yPos == _ySize) {
					increments(pDest);
					++drawIndex;
				} else {
					++yPos;
				}

				*++pDest = csuiv(pSrc, pLookup);
				areaNum = 2;
			} else if (yPos != _ySize) {
				++yPos;
				--drawIndex;
				areaNum = 0;
			} else {
				NIH();
				NIV();
				increments(pDest);
				++drawIndex;

				*++pDest = csuiv(pSrc, pLookup);

				if (drawIndex == _xSize) {
					areaNum = -1;
				} else {
					areaNum = 2;
				}
			}
			break;

		case 2:
			increments(pDest);

			if (!yPos) {
				NIH();
				NIV();

				if (drawIndex == _xSize) {
					increments(pDest);
					++yPos;
				} else {
					++drawIndex;
				}

				pDest += DEFAULT_WIDTH;
				areaNum = 0;
			} else if (drawIndex != _xSize) {
				++drawIndex;
				--yPos;

				*pDest = csuiv(pSrc, pLookup);
				areaNum = 2;
			} else {
				pDest += DEFAULT_WIDTH;
				++yPos;
				NIH();
				NIV();
				increments(pDest);

				*pDest = csuiv(pSrc, pLookup);

				if (yPos == _ySize)
					areaNum = -1;
				else
					areaNum = 1;
			}
			break;
		}
	}
}

void GfxSurface::diag(const byte *&pSrc, byte *&pDest, const byte *&pLookup) {
	int diagIndex = 0, drawIndex = 0;
	--_xEnd;

	while (!TFP(diagIndex)) {
		for (;;) {
			NIH();
			for (int idx = 0; idx <= _thickness; ++idx) {
				*pDest = csuiv(pSrc, pLookup);
				NIH();
				increments(pDest);
			}

			NIV();
			pDest += _yInc;

			for (int idx = 0; idx <= _thickness; ++idx) {
				*pDest = csuiv(pSrc, pLookup);
				NIH();
				increments(pDest);
			}

			NIH();
			NIV();
			increments(pDest);

			++drawIndex;
			if (_xEnd < (drawIndex + 1)) {
				TF1(pDest, diagIndex);
				break;
			}

			pDest += _xInc;
			++drawIndex;
			if (_xEnd < (drawIndex + 1)) {
				TF2(pSrc, pDest, pLookup, diagIndex);
				break;
			}
		}

		if (TFP(diagIndex))
			break;

		for (;;) {
			for (int idx = 0; idx <= _thickness; ++idx) {
				*pDest = csuiv(pSrc, pLookup);
				NIH();
				increments(pDest);
			}

			NIV();
			pDest += _yInc;

			for (int idx = 0; idx <= _thickness; ++idx) {
				*pDest = csuiv(pSrc, pLookup);
				NIH();
				increments(pDest);
			}

			NIH();
			NIV();
			increments(pDest);

			if (--drawIndex == 0) {
				TF1(pDest, diagIndex);
				NIH();
				break;
			} else {
				pDest += _xInc;

				if (--drawIndex == 0) {
					TF2(pSrc, pDest, pLookup, diagIndex);
					NIH();
					break;
				}
			}

			NIH();
		}
	}
}


void GfxSurface::increments(byte *&pDest) {
	pDest += _xInc + _yInc;
}

void GfxSurface::NIH() {
	_xInc = -_xInc;
}

void GfxSurface::NIV() {
	_yInc = -_yInc;
}

bool GfxSurface::TFP(int v) {
	int diff = _yEnd - v;
	if (!diff)
		// Time to finish loop in outer method
		return true;

	if (diff < (_thickness + 1))
		_thickness = diff - 1;

	return false;
}

void GfxSurface::TF1(byte *&pDest, int &v) {
	v += _thickness + 1;
	pDest += (_thickness + 1) * _yInc;
}

void GfxSurface::TF2(const byte *&pSrc, byte *&pDest, const byte *&pLookup, int &v) {
	v += _thickness + 1;

	for (int idx = 0; idx <= _thickness; ++idx) {
		*pDest = csuiv(pSrc, pLookup);
		pDest += _yInc;
	}
}

/*-------------------------------------------------------------------------*/

GfxSurface::~GfxSurface() {
	free();
}

/*-------------------------------------------------------------------------*
 * Screen surface
 *-------------------------------------------------------------------------*/

/**
 * Called to populate the font data from the passed file
 */
void ScreenSurface::readFontData(Common::File &f, int dataSize) {
	assert(dataSize == (FONT_NUM_CHARS * FONT_HEIGHT));
	f.read(_fontData, FONT_NUM_CHARS * FONT_HEIGHT);
}

/**
 * Returns a graphics surface representing a subset of the screen. The affected area
 * is also marked as dirty
 */
Graphics::Surface ScreenSurface::lockArea(const Common::Rect &bounds) {
	_dirtyRects.push_back(bounds);

	Graphics::Surface s;
	s.format = format;
	s.pixels = getBasePtr(bounds.left, bounds.top);
	s.pitch = pitch;
	s.w = bounds.width();
	s.h = bounds.height();

	return s;
}

/**
 * Updates the affected areas of the surface to the underlying physical screen
 */
void ScreenSurface::updateScreen() {
	// Iterate through copying dirty areas to the screen
	for (Common::List<Common::Rect>::iterator i = _dirtyRects.begin(); i != _dirtyRects.end(); ++i) {
		Common::Rect r = *i;
		g_system->copyRectToScreen((const byte *)getBasePtr(r.left, r.top), pitch,
			r.left, r.top, r.width(), r.height());
	}
	_dirtyRects.clear();

	// Update the screen
	g_system->updateScreen();
}

/**
 * Draws a decoded picture on the screen
 * @remarks		- Because the ScummVM surface is using a double height 640x400 surface to
 *		simulate the original 640x400 surface, all Y values have to be doubled.
 *		- Image resources are stored at 320x200, so when drawn onto the screen a single pixel
 *		from the source image is drawn using the two pixels at the given index in the palette map
 *		- Because the original game supported 320 width resolutions, the X coordinate
 *		also needs to be doubled for EGA mode
 */
void ScreenSurface::drawPicture(GfxSurface &surface, int x, int y) {
	// Adjust the draw position by the draw offset
	x += surface._offset.x;
	y += surface._offset.y;

	// Lock the affected area of the surface to write to
	Graphics::Surface destSurface = lockArea(Common::Rect(x * 2, y * 2,
		(x + surface.w) * 2, (y + surface.h) * 2));

	// Get a lookup for the palette mapping
	const byte *paletteMap = &g_vm->_mem[0x7000 * 16 + 2];

	// Loop through writing
	for (int yp = 0; yp < surface.h; ++yp) {
		if (((y + yp) < 0) || ((y + yp) >= 200))
			continue;

		const byte *pSrc = (const byte *)surface.getBasePtr(0, yp);
		byte *pDest = (byte *)destSurface.getBasePtr(0, yp * 2);

		for (int xp = 0; xp < surface.w; ++xp, ++pSrc) {
			if (*pSrc == surface._transparency) {
				// Transparent point, so skip pixels
				pDest += 2;
			} else {
				// Draw the pixel using the specified index in the palette map
				*pDest = paletteMap[*pSrc * 2];
				*(pDest + SCREEN_WIDTH) = paletteMap[*pSrc * 2];
				++pDest;

				// Use the secondary mapping value to draw the secondary column pixel
				*pDest = paletteMap[*pSrc * 2 + 1];
				*(pDest + SCREEN_WIDTH) = paletteMap[*pSrc * 2 + 1];
				++pDest;
			}
		}
	}
}

/**
 * Copys a given surface to the given position
 */
void ScreenSurface::copyFrom(Graphics::Surface &src, int x, int y) {
	Graphics::Surface destSurface = lockArea(Common::Rect(x, y, x + src.w, y + src.h));

	// Loop through writing
	for (int yp = 0; yp < src.h; ++yp) {
		if (((y + yp) < 0) || ((y + yp) >= SCREEN_HEIGHT))
			continue;

		const byte *pSrc = (const byte *)src.getBasePtr(0, yp);
		byte *pDest = (byte *)getBasePtr(0, yp);
		Common::copy(pSrc, pSrc + src.w, pDest);
	}
}

/**
 * Draws a character at the specified co-ordinates
 * @remarks		Because the ScummVM surface is using a double height 640x400 surface to
 *		simulate the original 640x400 surface, all Y values have to be doubled
 */
void ScreenSurface::writeCharacter(const Common::Point &pt, unsigned char ch, int palIndex) {
	Graphics::Surface destSurface = lockArea(Common::Rect(pt.x, pt.y * 2,
		pt.x + FONT_WIDTH, (pt.y + FONT_HEIGHT) * 2));

	// Get the start of the character to use
	assert((ch >= ' ') && (ch <= (unsigned char)(32 + FONT_NUM_CHARS)));
	const byte *charData = &_fontData[((int)ch - 32) * FONT_HEIGHT];

	// Loop through decoding each character's data
	for (int yp = 0; yp < FONT_HEIGHT; ++yp) {
		byte *lineP = (byte *)destSurface.getBasePtr(0, yp * 2);
		byte byteVal = *charData++;

		for (int xp = 0; xp < FONT_WIDTH; ++xp, ++lineP, byteVal <<= 1) {
			if (byteVal & 0x80) {
				*lineP = palIndex;
				*(lineP + SCREEN_WIDTH) = palIndex;
			}
		}
	}
}

/**
 * Draws a box at the specified position and size
 * @remarks		Because the ScummVM surface is using a double height 640x400 surface to
 *		simulate the original 640x400 surface, all Y values have to be doubled
 */
void ScreenSurface::drawBox(int x, int y, int dx, int dy, int col) {
	if (g_vm->_res == 1) {
		x = (uint)x >> 1;
		dx = (uint)dx >> 1;
	}

	Graphics::Surface destSurface = lockArea(Common::Rect(x, y * 2, x + dx, (y + dy) * 2));

	destSurface.hLine(0, 0, dx, col);
	destSurface.hLine(0, 1, dx, col);
	destSurface.hLine(0, destSurface.h - 1, dx, col);
	destSurface.hLine(0, destSurface.h - 2, dx, col);
	destSurface.vLine(0, 2, destSurface.h - 3, col);
	destSurface.vLine(1, 2, destSurface.h - 3, col);
	destSurface.vLine(dx - 1, 2, destSurface.h - 3, col);
	destSurface.vLine(dx - 2, 2, destSurface.h - 3, col);
}

/**
 * Fills an area with the specified colour
 * @remarks		Because the ScummVM surface is using a double height 640x400 surface to
 *		simulate the original 640x400 surface, all Y values have to be doubled
 */
void ScreenSurface::fillRect(int colour, const Common::Rect &bounds) {
	Graphics::Surface destSurface = lockArea(Common::Rect(bounds.left, bounds.top * 2,
		bounds.right, bounds.bottom * 2));

	// Fill the area
	destSurface.fillRect(Common::Rect(0, 0, destSurface.w, destSurface.h), colour);
}

/**
 * Clears the screen
 */
void ScreenSurface::clearScreen() {
	Graphics::Surface destSurface = lockArea(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	destSurface.fillRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
}

/**
 * Sets a single pixel at the specified co-ordinates
 * @remarks		Because the ScummVM surface is using a double height 640x400 surface to
 *		simulate the original 640x400 surface, all Y values have to be doubled
 */
void ScreenSurface::setPixel(const Common::Point &pt, int palIndex) {
	assert((pt.x >= 0) && (pt.y >= 0) && (pt.x <= SCREEN_WIDTH) && (pt.y <= SCREEN_ORIG_HEIGHT));
	Graphics::Surface destSurface = lockArea(Common::Rect(pt.x, pt.y * 2, pt.x + 1, (pt.y + 1) * 2));

	byte *destP = (byte *)destSurface.pixels;
	*destP = palIndex;
	*(destP + SCREEN_WIDTH) = palIndex;
}

/**
 * Write out a string
 * @remarks	Originally called 'writeg'
 */
void ScreenSurface::drawString(const Common::String &l, int command) {
	int i, x;
	Common::Point pt;
	int cecr = 0;

	if (l == "")
		return;

	g_vm->_mouse.hideMouse();
	pt = _textPos;

	if (g_vm->_res == 2)
		i = 6;
	else
		i = 10;
	x = pt.x + i * l.size();

	switch (command) {
	case 1:
	case 3: {
		cecr = 0;
		g_vm->_screenSurface.fillRect(15, Common::Rect(pt.x, pt.y, x, pt.y + 7));
		}
		break;
	case 4:
		cecr = 0;
		break;
	case 5:
		cecr = 15;
		break;
	case 0:
	case 2: {
		cecr = 15;
		g_vm->_screenSurface.fillRect(0, Common::Rect(pt.x, pt.y, x, pt.y + 7));
		}
		break;
	default:
		break;
	}

	pt.x += 1;
	pt.y += 1;
	for (x = 1; (x <= (int)l.size()) && (l[x - 1] != 0); ++x) {
		g_vm->_screenSurface.writeCharacter(Common::Point(pt.x, pt.y), ord(l[x - 1]), cecr);
		pt.x += i;
	}
	g_vm->_mouse.showMouse();
}

/**
 * Gets the width in pixels of the specified string
 */
int ScreenSurface::getStringWidth(const Common::String &s) {
	int charWidth = (g_vm->_res == 2) ? 6 : 10;

	return s.size() * charWidth;
}

void ScreenSurface::drawLine(int x, int y, int xx, int yy, int coul) {
	int step, i;
	float a, b;
	float xr, yr, xro, yro;

	xr = x;
	yr = y;
	xro = xx;
	yro = yy;

	if (abs(y - yy) > abs(x - xx)) {
		a = (float)((x - xx)) / (y - yy);
		b = (yr * xro - yro * xr) / (y - yy);
		i = y;
		if (y > yy)
			step = -1;
		else
			step = 1;
		do {
			g_vm->_screenSurface.setPixel(Common::Point(abs((int)(a * i + b)), i), coul);
			i += step;
		} while (i != yy);
	} else {
		a = (float)((y - yy)) / (x - xx);
		b = ((yro * xr) - (yr * xro)) / (x - xx);
		i = x;
		if (x > xx)
			step = -1;
		else
			step = 1;
		do {
			g_vm->_screenSurface.setPixel(Common::Point(i, abs((int)(a * i + b))), coul);
			i = i + step;
		} while (i != xx);
	}
}

/**
 * Draw plain rectangle
 * @remarks	Originally called 'paint_rect'
 */
void ScreenSurface::drawRectangle(int x, int y, int dx, int dy) {
	int co;

	if (g_vm->_currGraphicalDevice == MODE_CGA)
		co = 3;
	else
		co = 11;
	g_vm->_screenSurface.fillRect(co, Common::Rect(x, y, x + dx, y + dy));
}


} // End of namespace Mortevielle
