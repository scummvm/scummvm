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

void GfxSurface::decode(const byte *pSrc) {
	_width = _height = 0;
	_var1 = *pSrc++;
	_entryCount = *pSrc++;
	pSrc += 2;

	if (!_var1)
		pSrc += 30;

	// First run through the data to calculate starting offsets
	const byte *p = pSrc;
	int xOffset = 0xffff, yOffset = 0xffff;

	for (int idx = 0; idx < _entryCount; ++idx) {
		_xp = READ_BE_UINT16(p + 4);
		if (_xp < xOffset)
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

	byte table4140[256];
	byte table7110[256];

	// Main processing loop
	do {
		_var4 = READ_BE_UINT16(pSrc);
		_var6 = READ_BE_UINT16(pSrc + 2);
		_xp = READ_BE_UINT16(pSrc + 4) - _xOffset;
		_yp = READ_BE_UINT16(pSrc + 6) - _yOffset;
		pSrc += 8;

		_varC = READ_BE_UINT16(pSrc);
		_xSize = READ_BE_UINT16(pSrc + 2) + 1;
		_ySize = READ_BE_UINT16(pSrc + 4) + 1;
		majTtxTty();

		_var15 = _var18 = 0;

		int decomIndex = 0;
		if (_varC >> 8) {
			// Build up reference table
			int tableOffset = 0;

			if (_varC & 1) {
				do {
					_var12 = desanalyse(pSrc);
					_var14 = desanalyse(pSrc);

					int savedVar15 = _var15;
					int savedVar18 = _var18;
					
					do {
						const byte *pTemp = pSrc;
						_var15 = savedVar15;
						_var18 = savedVar18;

						assert(_var14 < 256);
						for (int idx = 0; idx < _var14; ++idx, ++tableOffset)
							table4140[tableOffset] = suiv(pTemp);
					} while (--_var12 > 0);
				} while (_var18 < (_var4 - 1));

			} else {
				assert(_var4 < 256);
				for (int idx = 0; idx < _var4; ++idx)
					table4140[idx] = suiv(pSrc);
			}

			if (_var15) {
				++pSrc;
				_var15 = 0;
			}
			if ((_var4 + _var6) & 1)
				++pSrc;

			tableOffset = 0;
			_var18 = 0;

			if (_varC & 2) {
				do {
					_var12 = desanalyse(pSrc);
					_var18 = _var14 = desanalyse(pSrc);
					if (_var15 & 1) {
						++pSrc;
						++_var18;
						_var15 = 0;
					}

					const byte *pStart = pSrc;
					do {
						pSrc = pStart;
						for (int idx = 0; idx < _var14; ++idx) {
							table7110[tableOffset++] = *pSrc++;
						}
					} while (--_var12 > 0);
				} while (_var18 < (_var6 - 1));
			} else {
				assert(_var6 < 256);
				for (int idx = 0; idx < _var4; ++idx)
					table7110[idx] = *pSrc++;
			}

			if (_var15)
				++pSrc;

			pSrcStart = pSrc;
			pDest = &outputBuffer[_yp * SCREEN_WIDTH + _xp];
			pSrc = &table7110[0];
			_var1A = _var18 = _var15 = 0;
			decomIndex = _varC;
		}

		// Main decompression switch
		switch (decomIndex) {
		case 0:
			// Draw rect at pos
			pDest = &outputBuffer[_yp * SCREEN_WIDTH + _xp];
			pSrcStart = pSrc;
			INCR_TAIX;
			
			for (int yCtr = 0; yCtr < _ySize; ++yCtr, pDest += SCREEN_WIDTH) {
				byte *pDestLine = pDest;
				for (int xCtr = 0; xCtr < _xSize; ++xCtr) {
					*pDestLine++ = suiv(pSrc);
				}
			}

			pSrc = pSrcStart + _var4 + ((_var4 & 1) ? 1 : 0);
			break;

		case 1:
			// Draw rect alternating left to right, right to left
			INCR_TAIX;
			for (int yCtr = 0; yCtr < _ySize; ++yCtr) {
				if (yCtr % 2) {
					for (int xCtr = 0; xCtr < _xSize; ++xCtr) {
						*pDest++ = suiv(pSrc);
					}
				} else {
					for (int xCtr = 0; xCtr < _xSize; ++xCtr) {
						*--pDest = suiv(pSrc);
					}
				}
				pDest += SCREEN_WIDTH;
			}
			break;

		case 2:
			// Draw rect alternating top to bottom, bottom to top
			INCR_TAIX;
			for (int xCtr = 0; xCtr < _xSize; ++xCtr) {
				if (xCtr % 2) {
					for (int yCtr = 0; yCtr < _ySize; ++yCtr, pDest += SCREEN_WIDTH) {
						*pDest = suiv(pSrc);
					}
				} else {
					for (int yCtr = 0; yCtr < _ySize; ++yCtr, pDest -= SCREEN_WIDTH) {
						*pDest = suiv(pSrc);
					}
				}
				++pDest;
			}
			break;

		case 3:
			// Draw horizontal line?
			_var1C = 2;
			horizontal(pSrc, pDest);
			break;
		}

		pSrc = pSrcStart;
	} while (--_entryCount > 0);
}

void GfxSurface::majTtxTty() {
	if (!_yp)
		_width += _xSize;

	if (!_xp)
		_height += _ySize;
}

byte GfxSurface::suiv(const byte *&pSrc) {
	int v = *pSrc;
	if (_var15) {
		++pSrc;
		++_var18;
		_var15 = 0;
		return v & 0xf;
	} else {
		_var15 = ~(v >> 8);
		return v >> 4;
	}
}

int GfxSurface::desanalyse(const byte *&pSrc) {
	int total = 0;
	int v = suiv(pSrc);
	if (!v) {
		int v2;
		do {
			v2 = suiv(pSrc);
		} while (v2 == 15);

		total *= 15;
		v = suiv(pSrc);
	}

	total += v;
	return total;
}

void GfxSurface::horizontal(const byte *&pSrc, byte *&pDest) {

}

} // End of namespace Mortevielle
