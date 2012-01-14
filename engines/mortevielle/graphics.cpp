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

#include "mortevielle/graphics.h"

namespace Mortevielle {

#define INCR_TAIX { if (_xSize & 1) ++_xSize; }
#define DEFAULT_WIDTH (SCREEN_WIDTH / 2)
#define BUFFER_SIZE 8192

void GfxSurface::decode(const byte *pSrc) {
	_width = _height = 0;
	_var1 = *pSrc++;
	int entryCount = *pSrc++;
	pSrc += 2;

	if (!_var1)
		pSrc += 30;

	// First run through the data to calculate starting offsets
	const byte *p = pSrc;
	_xOffset = _yOffset = 0xffff;

	assert(entryCount > 0);
	for (int idx = 0; idx < entryCount; ++idx) {
		_xp = READ_BE_UINT16(p + 4);
		if (_xp < _xOffset)
			_xOffset = _xp;

		_yp = READ_BE_UINT16(p + 6);
		if (_yp < _yOffset)
			_yOffset = _yp;

		// Move to next entry
		int size = READ_BE_UINT16(p) + READ_BE_UINT16(p + 2);
		if ((size % 2) == 1)
			++size;

		p += size + 14;
	}

	// Temporary output buffer
	byte outputBuffer[65536];
	byte *pDest = &outputBuffer[0];
	const byte *pSrcStart = pSrc;
	const byte *pLookup = NULL;

	byte lookupTable[BUFFER_SIZE];
	byte srcBuffer[BUFFER_SIZE];

	// Main processing loop
	do {
		int lookupBytes = READ_BE_UINT16(pSrc);
		int srcSize = READ_BE_UINT16(pSrc + 2);
		_xp = READ_BE_UINT16(pSrc + 4) - _xOffset;
		_yp = READ_BE_UINT16(pSrc + 6) - _yOffset;
		pSrc += 8;

		int decomCode = READ_BE_UINT16(pSrc);
		_xSize = READ_BE_UINT16(pSrc + 2) + 1;
		_ySize = READ_BE_UINT16(pSrc + 4) + 1;
		majTtxTty();
		
		pSrc += 6;
		pDest = &outputBuffer[0];

		_var18 = 0;
		_nibbleFlag = false;

		int decomIndex = 0;
		if (decomCode >> 8) {
			// Build up reference table
			int tableOffset = 0;

			if (decomCode & 1) {
				// Handle decompression of the pattern lookup table
				do {
					_var12 = desanalyse(pSrc);
					_var14 = desanalyse(pSrc);

					const byte *pSrcSaved = pSrc;
					bool savedNibbleFlag = _nibbleFlag;
					int savedVar18 = _var18;
					
					do {
						pSrc = pSrcSaved;
						_nibbleFlag = savedNibbleFlag;
						_var18 = savedVar18;

						assert(_var14 < 256);
						for (int idx = 0; idx < _var14; ++idx, ++tableOffset) {
							assert(tableOffset < BUFFER_SIZE);
							lookupTable[tableOffset] = suiv(pSrc);
						}
					} while (--_var12 > 0);
				} while (_var18 < (lookupBytes - 1));

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
			_var18 = 0;

			if (decomCode & 2) {
				// Handle decompression of the temporary source buffer
				do {
					_var12 = desanalyse(pSrc);
					_var14 = desanalyse(pSrc);
					_var18 += _var14;

					if (_nibbleFlag) {
						++pSrc;
						++_var18;
						_nibbleFlag = false;
					}

					const byte *pStart = pSrc;
					do {
						pSrc = pStart;
						for (int idx = 0; idx < _var14; ++idx) {
							assert(tableOffset < BUFFER_SIZE);
							srcBuffer[tableOffset++] = *pSrc++;
						}
					} while (--_var12 > 0);
				} while (_var18 < (srcSize - 1));
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

			_lookupValue = _var18 = 0;
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

			pSrc = pSrcStart + lookupBytes + ((lookupBytes & 1) ? 1 : 0);
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
			INCR_TAIX;
			for (int xCtr = 0; xCtr < _xSize; ++xCtr) {
				if ((xCtr % 2) == 0) {
					for (int yCtr = 0; yCtr < _ySize; ++yCtr, pDest += DEFAULT_WIDTH) {
						*pDest = csuiv(pSrc, pLookup);
					}
				} else {
					for (int yCtr = 0; yCtr < _ySize; ++yCtr, pDest -= DEFAULT_WIDTH) {
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
			_thickness = _var22 = 1;
			_var1E = 320;
			_var20 = _ySize;
			_var24 = _xSize;
			diag();
			break;

		case 13:
			INCR_TAIX;
			_thickness = _xSize;
			_var1E = 1;
			_var20 = _xSize;
			_var22 = 320;
			_var24 = _ySize;
			diag();
			break;

		case 14:
			_thickness = _var1E = 1;
			_var20 = _xSize;
			_var22 = 320;
			_var24 = _ySize;
			diag();
			break;

		case 15:
			INCR_TAIX;
			_thickness = 2;
			_var1E = 320;
			_var20 = _ySize;
			_var22 = 1;
			_var24 = _xSize;
			diag();
			break;

		case 16:
			_thickness = 3;
			_var1E = 1;
			_var20 = _xSize;
			_var22 = 320;
			_var24 = _ySize;
			diag();
			break;

		case 17:
			INCR_TAIX;
			_thickness = 3;
			_var1E = 320;
			_var20 = _ySize;
			_var22 = 1;
			_var24 = _xSize;
			diag();
			break;

		case 18:
			INCR_TAIX;
			_thickness = 5;
			_var1E = 320;
			_var20 = _ySize;
			_var22 = 1;
			_var24 = _xSize;
			diag();
			break;
		}

		pSrc = pSrcStart;
	} while (--entryCount > 0);

	// At this point, the outputBuffer has the data for the image. Initialise the surface
	// with the calculated size for the full image, and copy the lines to the surface
	create(_xOffset + _width, _yOffset + _height, Graphics::PixelFormat::createFormatCLUT8());

	for (int yCtr = 0; yCtr < _height; ++yCtr) {
		const byte *copySrc = &outputBuffer[yCtr * DEFAULT_WIDTH];
		byte *copyDest = (byte *)getBasePtr(_xOffset, yCtr + _yOffset);

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
		++_var18;
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
			if (--_thickness == 0) break;
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
					for (int yIndex = 0; yIndex < _thickness; ++yIndex, pDest -= DEFAULT_WIDTH)
						*pDest = csuiv(pSrc, pLookup);
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
					for (int yIndex = 0; yIndex < _thickness; ++yIndex, pDest -= DEFAULT_WIDTH)
						*pDest = csuiv(pSrc, pLookup);
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
//	byte *pDestEnd = pDest + (_ySize - 1) * DEFAULT_WIDTH + _xSize;
	int var28 = 0;

	for (;;) {
		// Reduce thickness as necessary
		while ((var28 + _thickness) >= _xSize) {
			if (--_thickness == 0)
				return;
		}

		// Loop
		for (int idx = 0; idx < _thickness; ++idx) {
			if ((idx % 2) == 0) {
				if (idx > 0)
					pDest -= DEFAULT_WIDTH;

				// Write out horizontal slice left to right
				var28 += _thickness;
				for (int xIndex = 0; xIndex < _thickness; ++xIndex) 
					*pDest++ = suiv(pSrc);
			} else {
				// Write out horizontal slice right to left
				pDest += DEFAULT_WIDTH;
				var28 -= _thickness;
				for (int xIndex = 0; xIndex < _thickness; ++xIndex)
					*pDest-- = csuiv(pSrc, pLookup);
			}
		}
		if ((_thickness % 2) == 0) {
			pDest += _thickness;
			var28 += _thickness;
		}

		// Reduce thickness as necessary
		while ((var28 + _thickness) < _xSize) {
			if (--_thickness == 0)
				return;
		}

		for (int yIndex = 0; yIndex < _ySize; ++yIndex) {
			if ((yIndex % 2) == 0) {
				if (yIndex > 0)
					pDest -= DEFAULT_WIDTH;

				// Draw horizontal slice
				var28 += _thickness;

				for (int xIndex = 0; xIndex < _thickness; ++xIndex)
					*pDest++ = suiv(pSrc);
			} else {
				pDest -= DEFAULT_WIDTH;
				var28 -= _thickness;

				for (int xIndex = 0; xIndex < _thickness; ++xIndex)
					*pDest-- = csuiv(pSrc, pLookup);
			}
		}
	}
}

void GfxSurface::decom11(const byte *&pSrc, byte *&pDest, const byte *&pLookup) {
	int var26 = 0, var28 = 0;
	_var1E = DEFAULT_WIDTH;
	_var22 = -1;
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

			if (!var28) {
				NIH();
				NIV();

				if (var26 == _ySize) {
					increments(pDest);
					++var28;
				} else {
					++var26;
				}

				*++pDest = csuiv(pSrc, pLookup);
				areaNum = 2;
			} else if (var26 != _ySize) {
				++var26;
				--var28;
				areaNum = 0;
			} else {
				NIH();
				NIV();
				increments(pDest);
				++var28;

				*++pDest = csuiv(pSrc, pLookup);

				if (var28 == _xSize) {
					areaNum = -1;
				} else {
					areaNum = 2;
				}
			}
			break;

		case 2:
			increments(pDest);

			if (!var26) {
				NIH();
				NIV();

				if (var28 == _xSize) {
					increments(pDest);
					++var26;
				} else {
					++var28;
				}

				pDest += DEFAULT_WIDTH;
				areaNum = 0;
			} else if (var28 != _xSize) {
				++var28;
				--var26;

				*pDest = csuiv(pSrc, pLookup);
				areaNum = 2;
			} else {
				pDest += DEFAULT_WIDTH;
				++var26;
				NIH();
				NIV();
				increments(pDest);

				*pDest = csuiv(pSrc, pLookup);

				if (var26 == _ySize)
					areaNum = -1;
				else
					areaNum = 1;
			}
			break;
		}
	}
}

void GfxSurface::increments(byte *&pDest) {
	pDest += _var22 + _var1E;
}

void GfxSurface::NIH() {
	_var22 = -_var22;
}

void GfxSurface::NIV() {
	_var1E = -_var1E;
}

void GfxSurface::diag() {
	// The diag method in the original source doesn't seem to have any exit point,
	// which if the case, means the routine may not be used by the game
	error("Non-exitable method diag() called");
}

} // End of namespace Mortevielle
