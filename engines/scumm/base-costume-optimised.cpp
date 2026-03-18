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
	int linesToSkip = 0, startScaleIndexX, startScaleIndexY;
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
			compData.scaleXStep = 1;
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
				if (rect.right < 0) {
					linesToSkip++;
					startScaleIndexX = j;
				}
				if (compData.scaleTable[j++ & compData.scaleIndexMask] < _scaleX)
					rect.right++;
			}
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
					startScaleIndexX = j;
					linesToSkip++;
				}
				if (compData.scaleTable[j-- & compData.scaleIndexMask] < _scaleX)
					rect.left--;
			}
		}

		if (linesToSkip)
			linesToSkip--;

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
		} else {
			rect.left = compData.x - _width;
			rect.right = compData.x;
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

	markAsDirty(rect, compData, decode);
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

	if (_drawActorToRight) {
		if (!actorIsScaled)
			linesToSkip = compData.boundsRect.left - compData.x;
		if (linesToSkip > 0) {
			if (!amiOrPcEngCost && !c64Cost) {
				compData.skipWidth -= linesToSkip;
				skipCelLines(compData, linesToSkip);
				compData.x = compData.boundsRect.left;
			}
		} else {
			linesToSkip = rect.right - compData.boundsRect.right;
			if (linesToSkip <= 0) {
				drawFlag = 2;
			} else {
				compData.skipWidth -= linesToSkip;
			}
		}
	} else {
		if (!actorIsScaled) {
			if (_akosRendering)
				linesToSkip = rect.right - compData.boundsRect.right + 1;
			else
				linesToSkip = rect.right - compData.boundsRect.right;
		}
		if (linesToSkip > 0) {
			if (!amiOrPcEngCost && !c64Cost) {
				compData.skipWidth -= linesToSkip;
				skipCelLines(compData, linesToSkip);
				compData.x = compData.boundsRect.right - 1;
			}
		} else {
			// V1 games uses 8 x 8 pixels for actors
			if (c64Cost)
				linesToSkip = (compData.boundsRect.left - 8) - rect.left;
			else
				linesToSkip = (compData.boundsRect.left - 1) - rect.left;
			if (linesToSkip <= 0)
				drawFlag = 2;
			else
				compData.skipWidth -= linesToSkip;
		}
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

void BaseCostumeRenderer::byleRLEDecodeFast(ByleRLEData &compData) {
	const byte *src = _srcPtr;
	byte *dst = compData.destPtr;

	byte len = compData.repLen;
	uint16 color = compData.repColor;

	// reset every column
	int lastColumnX = -1;
	int y = compData.y;
	uint16 height = _height;
	int scaleIndexY = compData.scaleYIndex;

	byte maskbit = revBitMask(compData.x & 7);
	const byte *mask = compData.maskPtr + compData.x / 8;

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

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> compData.shr;
		len &= compData.mask;
		if (!len)
			len = *src++;

		do {
			if (_scaleY == 255 || compData.scaleTable[scaleIndexY++ & compData.scaleIndexMask] < _scaleY) {
				if (color) {
					const bool masked = (y < compData.boundsRect.top || y >= compData.boundsRect.bottom)
					|| (compData.x < compData.boundsRect.left || compData.x >= compData.boundsRect.right)
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
			if (--height == 0) {
				if (--compData.skipWidth == 0)
					return;
				height = _height;
				y = compData.y;

				scaleIndexY = compData.scaleYIndex;
				lastColumnX = compData.x;

				if (_scaleX == 255 || compData.scaleTable[compData.scaleXIndex] < _scaleX) {
					compData.x += compData.scaleXStep;
					if (compData.x < compData.boundsRect.left || compData.x >= compData.boundsRect.right)
						return;
					maskbit = revBitMask(compData.x & 7);
					compData.destPtr += compData.scaleXStep;
				}

				compData.scaleXIndex = (compData.scaleXIndex + compData.scaleXStep) & compData.scaleIndexMask;

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (--len);
	} while (true);
}
#endif

} // End of namespace Scumm
