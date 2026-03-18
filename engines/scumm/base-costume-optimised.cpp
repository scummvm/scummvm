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


#include "scumm/base-costume.h"
#include "scumm/util.h"

namespace Scumm {

#ifdef SCUMM_OPTIMISED_CODE
byte BaseCostumeRenderer::paintCelByleRLECommon(
	int xMoveCur,
	int yMoveCur,
	int numColors,
	int scaletableSize,
	bool amiOrPcEngCost,
	bool c64Cost,
	ByleRLEData &compData,
	bool &decode) {

	bool actorIsScaled;
	int i, j;
	int linesToSkip = 0, trailingLinesToSkip = 0, startScaleIndexX, startScaleIndexY;
	Common::Rect rect;
	int step;
	byte drawFlag = 1;

	// Setup color decoding variables
	if (numColors == 32) {
		compData.mask = 7;
		compData.shr = 3;
	} else if (numColors == 64) {
		compData.mask = 3;
		compData.shr = 2;
	} else {
		compData.mask = 15;
		compData.shr = 4;
	}

	actorIsScaled = (_scaleX != 0xFF) || (_scaleY != 0xFF);

	compData.boundsRect.left = 0;
	compData.boundsRect.top = 0;
	compData.boundsRect.right = _out.w;
	compData.boundsRect.bottom = _out.h;

	if (actorIsScaled) {
		/* Scale direction */
		compData.scaleXStep = -1;
		if (xMoveCur < 0) {
			xMoveCur = -xMoveCur;
			compData.scaleXStep = -compData.scaleXStep;
		}

		if (_drawActorToRight) {
			/* Adjust X position */
			startScaleIndexX = j = (scaletableSize - xMoveCur) & compData.scaleIndexMask;
			for (i = 0; i < xMoveCur; i++) {
				if (compData.scaleTable[j++ & compData.scaleIndexMask] < _scaleX)
					compData.x -= compData.scaleXStep;
			}

			rect.left = rect.right = compData.x;

			j = startScaleIndexX;
			for (i = 0; i < _width; i++) {
				if (rect.right < compData.boundsRect.left) {
					linesToSkip++;
				} else if (rect.right >= compData.boundsRect.right) {
					trailingLinesToSkip++;
				}
				if (compData.scaleTable[j++ & compData.scaleIndexMask] < _scaleX)
					rect.right++;
			}
			startScaleIndexX += linesToSkip;
		} else {
			/* No mirror */
			/* Adjust X position */
			startScaleIndexX = j = (scaletableSize + xMoveCur) & compData.scaleIndexMask;
			for (i = 0; i < xMoveCur; i++) {
				if (compData.scaleTable[j-- & compData.scaleIndexMask] < _scaleX)
					compData.x += compData.scaleXStep;
			}

			rect.left = rect.right = compData.x;

			j = startScaleIndexX;
			for (i = 0; i < _width; i++) {
				if (rect.left >= compData.boundsRect.right) {
					linesToSkip++;
				} else if (rect.left < compData.boundsRect.left) {
					trailingLinesToSkip++;
				}
				if (compData.scaleTable[j-- & compData.scaleIndexMask] < _scaleX)
					rect.left--;
			}
			startScaleIndexX -= linesToSkip;
		}

		step = -1;
		if (yMoveCur < 0) {
			yMoveCur = -yMoveCur;
			step = -step;
		}

		startScaleIndexY = j = (scaletableSize - yMoveCur) & compData.scaleIndexMask;
		for (i = 0; i < yMoveCur; i++) {
			if (compData.scaleTable[j++ & compData.scaleIndexMask] < _scaleY)
				compData.y -= step;
		}

		rect.top = rect.bottom = compData.y;

		j = startScaleIndexY;
		for (i = 0; i < _height; i++) {
			if (compData.scaleTable[j++ & compData.scaleIndexMask] < _scaleY)
				rect.bottom++;
		}
	} else {
		if (!_drawActorToRight)
			xMoveCur = -xMoveCur;

		compData.x += xMoveCur;
		compData.y += yMoveCur;

		if (_drawActorToRight) {
			rect.left = compData.x;
			rect.right = compData.x + _width;

			linesToSkip = compData.boundsRect.left - compData.x;
			trailingLinesToSkip = rect.right - compData.boundsRect.right;
		} else {
			rect.left = compData.x - _width;
			rect.right = compData.x;

			linesToSkip = rect.right - compData.boundsRect.right + 1;
			if (c64Cost)
				trailingLinesToSkip = (compData.boundsRect.left - 8) - rect.left;
			else
				trailingLinesToSkip = (compData.boundsRect.left - 1) - rect.left;
		}

		rect.top = compData.y;
		rect.bottom = rect.top + _height;

		startScaleIndexX = scaletableSize;
		startScaleIndexY = scaletableSize;
	}

	compData.scaleXIndex = startScaleIndexX;
	compData.scaleYIndex = startScaleIndexY;
	compData.skipWidth = _width;
	compData.scaleXStep = _drawActorToRight ? 1 : -1;

	// All the important 'rect' values. scale sequence = 'compData.scaleTable[i] < _scaleX' result)
	//
	// rendering dir | scaled | scale sequence | drawn columns | dirty columns          | ideal fix
	// --------------+--------+----------------+---------------+------------------------+----------------
	// left-to-right | no     | N/A            | 10, 11, 12    | [10, 13) = 10, 11, 12  | none needed
	// left-to-right | yes    | T, T, T        | 10, 11, 12    | [10, 13) = 10, 11, 12  | none needed
	// left-to-right | yes    | T, F, T        | 10, 11        | [10, 12) = 10, 11      | none needed
	// left-to-right | yes    | T, T, F        | 10, 11, 12    | [10, 12) = 10, 11      | right++
	// right-to-left | no     | N/A            | 10,  9,  8    | [ 7, 10) =  9,  8,  7  | left++, right++
	// right-to-left | yes    | T, T, T        | 10,  9,  8    | [ 7, 10) =  9,  8,  7  | left++, right++
	// right-to-left | yes    | T, F, T        | 10,  9        | [ 8, 10) =  9,  8      | left++, right++
	// right-to-left | yes    | T, T, F        | 10,  9,  8    | [ 8, 10) =  9,  8      | right++
	//
	// Considering how complex would be to handle all left/right adjustments precisely, go with what the old
	// costume renderer did, just add +1 to the right. That sometimes extends the dirty rect by one or two
	// columns but definitely fixes all edge cases with zero effort.
	Common::Rect dirtyRect = rect;
	if (_akosRendering) {
		// ClassicCostumeRenderer::markAsDirty already does that
		dirtyRect.right++;
	}

	markAsDirty(dirtyRect, compData, decode);
	if (!decode)
		return 0;

	if (rect.top >= compData.boundsRect.bottom || rect.bottom <= compData.boundsRect.top) {
		decode = false;
		return 0;
	}

	if (rect.left >= compData.boundsRect.right || rect.right <= compData.boundsRect.left) {
		decode = false;
		return 0;
	}

	compData.repLen = 0;

	if (linesToSkip > 0) {
		if (!amiOrPcEngCost && !c64Cost) {
			compData.skipWidth -= linesToSkip;
			skipCelLines(compData, linesToSkip);
			compData.x = _drawActorToRight ? compData.boundsRect.left : (compData.boundsRect.right - 1);
		}
	}

	if (trailingLinesToSkip > 0) {
		compData.skipWidth -= trailingLinesToSkip;
	}

	if (linesToSkip <= 0 && trailingLinesToSkip <= 0) {
		drawFlag = 2;
	}

	if (compData.skipWidth <= 0) {
		decode = false;
		return 0;
	}

	if (rect.left < compData.boundsRect.left)
		rect.left = compData.boundsRect.left;

	if (rect.top < compData.boundsRect.top)
		rect.top = compData.boundsRect.top;

	if (rect.top > compData.boundsRect.bottom)
		rect.top = compData.boundsRect.bottom;

	if (rect.bottom > compData.boundsRect.bottom)
		rect.bottom = compData.boundsRect.bottom;

	if (_drawTop > rect.top)
		_drawTop = rect.top;
	if (_drawBottom < rect.bottom)
		_drawBottom = rect.bottom;

	if (!_akosRendering && (_height + rect.top >= 256)) {
		decode = false;
		return 2;
	}

	compData.destPtr = (byte *)_out.getBasePtr(compData.x, compData.y);

	return drawFlag;
}

enum class ShadowMode : int {
	Mode0,
	Mode1,
	Mode3,
	Classic
};

void ByleRLEDecode_Mode0(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX, /* unused */
	const byte _scaleY, /* unused */
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable, /* unused */
	const uint16 *_palette) {

	BaseCostumeRenderer::ByleRLEData &compData = *pcompData;

	const byte *src = _srcPtr;

	uint16 len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	uint16 height = _height;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	uint16 batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			if (color) {
				do {
					if (!(*mask & maskbit))
						*dst = _palette[color];
					dst  += pitch;
					mask += _numStrips;
				} while (--batch);
			} else {
				dst  += batch * pitch;
				mask += batch * _numStrips;
			}

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;

				compData.x += compData.scaleXStep;
				maskbit = revBitMask(compData.x & 7);
				compData.destPtr += compData.scaleXStep;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}

void ByleRLEDecode_Mode1(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX, /* unused */
	const byte _scaleY, /* unused */
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette) {

	BaseCostumeRenderer::ByleRLEData &compData = *pcompData;

	const byte *src = _srcPtr;

	uint16 len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	int lastColumnX = -1;
	uint16 height = _height;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	uint16 batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			if (color) {
				do {
					if (!(*mask & maskbit)) {
						uint16 pcolor = _palette[color];
						if (pcolor == 13 && _shadowTable) {
							if (lastColumnX != compData.x)
								*dst = _shadowTable[*dst];
						} else {
							*dst = pcolor;
						}
					}
					dst  += pitch;
					mask += _numStrips;
				} while (--batch);
			} else {
				dst  += batch * pitch;
				mask += batch * _numStrips;
			}

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;

				lastColumnX = compData.x;

				compData.x += compData.scaleXStep;
				maskbit = revBitMask(compData.x & 7);
				compData.destPtr += compData.scaleXStep;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}

void ByleRLEDecode_Mode3(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX, /* unused */
	const byte _scaleY, /* unused */
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette) {

	BaseCostumeRenderer::ByleRLEData &compData = *pcompData;

	const byte *src = _srcPtr;

	uint16 len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	int lastColumnX = -1;
	uint16 height = _height;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	uint16 batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			if (color) {
				do {
					if (!(*mask & maskbit)) {
						uint16 pcolor = _palette[color];
						if (pcolor < 8) {
							if (lastColumnX != compData.x) {
								pcolor = (pcolor << 8) + *dst;
								*dst = _shadowTable[pcolor];
							}
						} else {
							*dst = pcolor;
						}
					}
					dst  += pitch;
					mask += _numStrips;
				} while (--batch);
			} else {
				dst  += batch * pitch;
				mask += batch * _numStrips;
			}

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;

				lastColumnX = compData.x;

				compData.x += compData.scaleXStep;
				maskbit = revBitMask(compData.x & 7);
				compData.destPtr += compData.scaleXStep;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}

void ByleRLEDecode_Classic(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX, /* unused */
	const byte _scaleY, /* unused */
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette /* unused */) {

	BaseCostumeRenderer::ByleRLEData &compData = *pcompData;

	const byte *src = _srcPtr;

	uint16 len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	int lastColumnX = -1;
	uint16 height = _height;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	uint16 batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			if (color) {
				do {
					if (!(*mask & maskbit)) {
						if (lastColumnX != compData.x)
							*dst = _shadowTable[*dst];
					}
					dst  += pitch;
					mask += _numStrips;
				} while (--batch);
			} else {
				dst  += batch * pitch;
				mask += batch * _numStrips;
			}

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;

				lastColumnX = compData.x;

				compData.x += compData.scaleXStep;
				maskbit = revBitMask(compData.x & 7);
				compData.destPtr += compData.scaleXStep;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}

void ByleRLEDecode_Scaled_Mode0(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable, /* unused */
	const uint16 *_palette) {

	BaseCostumeRenderer::ByleRLEData &compData = *pcompData;

	const byte *src = _srcPtr;

	byte len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	uint16 height = _height;
	int scaleIndexY = compData.scaleYIndex;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	byte batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? (byte)height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			do {
				if (compData.scaleTable[scaleIndexY++] < _scaleY) {
					if (color) {
						if (!(*mask & maskbit))
							*dst = _palette[color];
					}
					dst += pitch;
					mask += _numStrips;
				}
			} while (--batch);

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;

				scaleIndexY = compData.scaleYIndex;

				if (compData.scaleTable[compData.scaleXIndex] < _scaleX) {
					compData.x += compData.scaleXStep;
					maskbit = revBitMask(compData.x & 7);
					compData.destPtr += compData.scaleXStep;
				}

				compData.scaleXIndex += compData.scaleXStep;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}

void ByleRLEDecode_Scaled_Mode0_SMask(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable, /* unused */
	const uint16 *_palette) {

	BaseCostumeRenderer::ByleRLEData &compData = *pcompData;

	const byte *src = _srcPtr;

	byte len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	uint16 height = _height;
	int scaleIndexY = compData.scaleYIndex;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	byte batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? (byte)height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			do {
				if (compData.scaleTable[scaleIndexY++ & compData.scaleIndexMask] < _scaleY) {
					if (color) {
						if (!(*mask & maskbit))
							*dst = _palette[color];
					}
					dst += pitch;
					mask += _numStrips;
				}
			} while (--batch);

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;

				scaleIndexY = compData.scaleYIndex;

				if (compData.scaleTable[compData.scaleXIndex] < _scaleX) {
					compData.x += compData.scaleXStep;
					maskbit = revBitMask(compData.x & 7);
					compData.destPtr += compData.scaleXStep;
				}

				compData.scaleXIndex = (compData.scaleXIndex + compData.scaleXStep) & compData.scaleIndexMask;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}

void ByleRLEDecode_Scaled_Mode1(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette) {

	BaseCostumeRenderer::ByleRLEData &compData = *pcompData;

	const byte *src = _srcPtr;

	byte len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	int lastColumnX = -1;
	uint16 height = _height;
	int scaleIndexY = compData.scaleYIndex;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	byte batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? (byte)height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			do {
				if (compData.scaleTable[scaleIndexY++] < _scaleY) {
					if (color) {
						if (!(*mask & maskbit)) {
							uint16 pcolor;

							pcolor = _palette[color];
							if (pcolor == 13 && _shadowTable) {
								if (lastColumnX != compData.x)
									*dst = _shadowTable[*dst];
							} else {
								*dst = pcolor;
							}
						}
					}
					dst += pitch;
					mask += _numStrips;
				}
			} while (--batch);

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;

				scaleIndexY = compData.scaleYIndex;
				lastColumnX = compData.x;

				if (compData.scaleTable[compData.scaleXIndex] < _scaleX) {
					compData.x += compData.scaleXStep;
					maskbit = revBitMask(compData.x & 7);
					compData.destPtr += compData.scaleXStep;
				}

				compData.scaleXIndex += compData.scaleXStep;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}

void ByleRLEDecode_Scaled_Mode1_SMask(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette) {

	BaseCostumeRenderer::ByleRLEData &compData = *pcompData;

	const byte *src = _srcPtr;

	byte len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	int lastColumnX = -1;
	uint16 height = _height;
	int scaleIndexY = compData.scaleYIndex;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	byte batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? (byte)height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			do {
				if (compData.scaleTable[scaleIndexY++ & compData.scaleIndexMask] < _scaleY) {
					if (color) {
						if (!(*mask & maskbit)) {
							uint16 pcolor;

							pcolor = _palette[color];
							if (pcolor == 13 && _shadowTable) {
								if (lastColumnX != compData.x)
									*dst = _shadowTable[*dst];
							} else {
								*dst = pcolor;
							}
						}
					}
					dst += pitch;
					mask += _numStrips;
				}
			} while (--batch);

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;

				scaleIndexY = compData.scaleYIndex;
				lastColumnX = compData.x;

				if (compData.scaleTable[compData.scaleXIndex] < _scaleX) {
					compData.x += compData.scaleXStep;
					maskbit = revBitMask(compData.x & 7);
					compData.destPtr += compData.scaleXStep;
				}

				compData.scaleXIndex = (compData.scaleXIndex + compData.scaleXStep) & compData.scaleIndexMask;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}

void ByleRLEDecode_Scaled_Mode3(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette) {

	BaseCostumeRenderer::ByleRLEData &compData = *pcompData;

	const byte *src = _srcPtr;

	byte len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	int lastColumnX = -1;
	uint16 height = _height;
	int scaleIndexY = compData.scaleYIndex;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	byte batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? (byte)height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			do {
				if (compData.scaleTable[scaleIndexY++] < _scaleY) {
					if (color) {
						if (!(*mask & maskbit)) {
							uint16 pcolor;

							pcolor = _palette[color];
							if (pcolor < 8) {
								if (lastColumnX != compData.x) {
									pcolor = (pcolor << 8) + *dst;
									*dst = _shadowTable[pcolor];
								}
							} else {
								*dst = pcolor;
							}
						}
					}
					dst += pitch;
					mask += _numStrips;
				}
			} while (--batch);

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;

				scaleIndexY = compData.scaleYIndex;
				lastColumnX = compData.x;

				if (compData.scaleTable[compData.scaleXIndex] < _scaleX) {
					compData.x += compData.scaleXStep;
					maskbit = revBitMask(compData.x & 7);
					compData.destPtr += compData.scaleXStep;
				}

				compData.scaleXIndex += compData.scaleXStep;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}

void ByleRLEDecode_Scaled_Classic_SMask(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette /* unused */) {

	BaseCostumeRenderer::ByleRLEData &compData = *pcompData;

	const byte *src = _srcPtr;

	byte len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	int lastColumnX = -1;
	uint16 height = _height;
	int scaleIndexY = compData.scaleYIndex;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	byte batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? (byte)height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			do {
				if (compData.scaleTable[scaleIndexY++ & compData.scaleIndexMask] < _scaleY) {
					if (color) {
						if (!(*mask & maskbit)) {
							if (lastColumnX != compData.x)
								*dst = _shadowTable[*dst];
						}
					}
					dst += pitch;
					mask += _numStrips;
				}
			} while (--batch);

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;

				scaleIndexY = compData.scaleYIndex;
				lastColumnX = compData.x;

				if (compData.scaleTable[compData.scaleXIndex] < _scaleX) {
					compData.x += compData.scaleXStep;
					maskbit = revBitMask(compData.x & 7);
					compData.destPtr += compData.scaleXStep;
				}

				compData.scaleXIndex = (compData.scaleXIndex + compData.scaleXStep) & compData.scaleIndexMask;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}

typedef void (*ByleRLEDecodeFunc)(
	BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette);

static const ByleRLEDecodeFunc byleRLEDecodeNoScaleTable[4] = {
	ByleRLEDecode_Mode0,    // 0: Mode0
	ByleRLEDecode_Mode1,    // 1: Mode1
	ByleRLEDecode_Mode3,    // 2: Mode3
	ByleRLEDecode_Classic,  // 3: Classic
};

static const ByleRLEDecodeFunc byleRLEDecodeScaledTable[8] = {
	ByleRLEDecode_Scaled_Mode0,         // 0: Mode0,   no scaleIndexMask
	ByleRLEDecode_Scaled_Mode0_SMask,   // 1: Mode0,   scaleIndexMask
	ByleRLEDecode_Scaled_Mode1,         // 2: Mode1,   no scaleIndexMask
	ByleRLEDecode_Scaled_Mode1_SMask,   // 3: Mode1,   scaleIndexMask
	ByleRLEDecode_Scaled_Mode3,         // 4: Mode3,   no scaleIndexMask
	nullptr,                            // 5: Mode3,   scaleIndexMask (COMI's Mode3 always uses bigCostumeScaleTable)
	nullptr,                            // 6: Classic, no scaleIndexMask (_shadowMode & 0x20 always uses smallCostumeScaleTable)
	ByleRLEDecode_Scaled_Classic_SMask, // 7: Classic, scaleIndexMask
};

void BaseCostumeRenderer::byleRLEDecodeFast(ByleRLEData &compData) {
	ShadowMode shadowMode = ShadowMode::Mode0;
	if (!_akosRendering) {
		if (_shadowMode & 0x20)
			shadowMode = ShadowMode::Classic;
		else
			shadowMode = ShadowMode::Mode1;
	} else {
		if (_shadowMode == 1)
			shadowMode = ShadowMode::Mode1;
		else if (_shadowMode == 3)
			shadowMode = ShadowMode::Mode3;
	}

	if (compData.y >= compData.boundsRect.top && compData.y + _height <= compData.boundsRect.bottom) {
		const int scaled = (_scaleX != 255 || _scaleY != 255);
		if (!scaled) {
			byleRLEDecodeNoScaleTable[static_cast<int>(shadowMode)](
				&compData,
				_scaleX,
				_scaleY,
				_height,
				_out.pitch,
				_numStrips,
				_srcPtr,
				_shadowTable,
				_palette);
		} else {
			const int useScaleIndexMask = compData.scaleIndexMask != -1;

			byleRLEDecodeScaledTable[(static_cast<int>(shadowMode) << 1) | useScaleIndexMask](
				&compData,
				_scaleX,
				_scaleY,
				_height,
				_out.pitch,
				_numStrips,
				_srcPtr,
				_shadowTable,
				_palette);
		}
		return;
	}

	const byte *src = _srcPtr;

	byte len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	byte *dst = compData.destPtr;
	int lastColumnX = -1;
	int y = compData.y;
	uint16 height = _height;
	int scaleIndexY = compData.scaleYIndex;
	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

	byte batch;
	if (len) {
		--len;
		goto StartPos;
	}

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			batch = height < len ? (byte)height : len;
			len -= batch;
			height -= batch;

			assert(compData.x >= compData.boundsRect.left && compData.x < compData.boundsRect.right);

			do {
				if (_scaleY == 255 || compData.scaleTable[scaleIndexY++ & compData.scaleIndexMask] < _scaleY) {
					if (color) {
						const bool masked = (y < compData.boundsRect.top || y >= compData.boundsRect.bottom)
							|| (*mask & maskbit);

						if (!masked) {
							uint16 pcolor;

							switch(shadowMode) {
							case ShadowMode::Mode0:
								*dst = _palette[color];
								break;

							case ShadowMode::Classic:
								if (lastColumnX != compData.x)
									*dst = _shadowTable[*dst];
								break;

							case ShadowMode::Mode1:
								pcolor = _palette[color];
								if (pcolor == 13 && _shadowTable) {
									if (lastColumnX != compData.x)
										*dst = _shadowTable[*dst];
								} else {
									*dst = pcolor;
								}
								break;

							case ShadowMode::Mode3:
								pcolor = _palette[color];
								if (pcolor < 8) {
									if (lastColumnX != compData.x) {
										pcolor = (pcolor << 8) + *dst;
										*dst = _shadowTable[pcolor];
									}
								} else {
									*dst = pcolor;
								}
								break;
							}
						}
					}
					dst += _out.pitch;
					mask += _numStrips;
					y++;
				}
			} while (--batch);

			if (height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;
				y = compData.y;

				scaleIndexY = compData.scaleYIndex;
				lastColumnX = compData.x;

				if (_scaleX == 255 || compData.scaleTable[compData.scaleXIndex] < _scaleX) {
					compData.x += compData.scaleXStep;
					maskbit = revBitMask(compData.x & 7);
					compData.destPtr += compData.scaleXStep;
				}

				compData.scaleXIndex = (compData.scaleXIndex + compData.scaleXStep) & compData.scaleIndexMask;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (len > 0);
	} while (true);
}
#endif

} // End of namespace Scumm
