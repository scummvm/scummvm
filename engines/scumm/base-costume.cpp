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

byte BaseCostumeRenderer::drawCostume(const VirtScreen &vs, int numStrips, const Actor *a, bool drawToBackBuf) {
	int i;
	byte result = 0;

	_out = vs;
	if (drawToBackBuf)
		_out.setPixels(vs.getBackPixels(0, 0));
	else
		_out.setPixels(vs.getPixels(0, 0));

	_actorX += _vm->_virtscr[kMainVirtScreen].xstart & 7;
	_out.w = _out.pitch / _vm->_bytesPerPixel;
	// We do not use getBasePtr here because the offset to pixels never used
	// _vm->_bytesPerPixel, but it seems unclear why.
	_out.setPixels((byte *)_out.getPixels() - (_vm->_virtscr[kMainVirtScreen].xstart & 7));

	_numStrips = numStrips;

	if (_vm->_game.version <= 1) {
		_xMove = 0;
		_yMove = 0;
	} else if (_vm->_game.features & GF_OLD_BUNDLE) {
		_xMove = -72;
		_yMove = -100;
	} else {
		_xMove = _yMove = 0;
	}
	for (i = 0; i < 16; i++)
		result |= drawLimb(a, i);
	return result;
}

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

		if (_mirror) {
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
		if (!_mirror)
			xMoveCur = -xMoveCur;

		compData.x += xMoveCur;
		compData.y += yMoveCur;

		if (_mirror) {
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
	compData.scaleXStep = _mirror ? 1 : -1;

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

	if (_mirror) {
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

	if (compData.skipWidth <= 0 || (_akosRendering && _height <= 0)) {
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

	compData.width = _out.w;
	compData.height = _out.h;
	compData.destPtr = (byte *)_out.getBasePtr(compData.x, compData.y);

	return drawFlag;
}

#ifdef USE_ARM_COSTUME_ASM

#ifndef IPHONE
#define ClassicProc3RendererShadowARM _ClassicProc3RendererShadowARM
#endif

extern "C" int ClassicProc3RendererShadowARM(
	int _scaleY,
	ClassicCostumeRenderer::ByleRLEData *compData,
	int pitch,
	const byte *src,
	int height,
	int _scaleX,
	int _scaleIndexX,
	byte *_shadowTable,
	uint16 _palette[32],
	int32 _numStrips,
	int _scaleIndexY);
#endif

void BaseCostumeRenderer::byleRLEDecode(ByleRLEData &compData, int16 actorHitX, int16 actorHitY, bool *actorHitResult, const uint8 *xmap) {
	const byte *mask, *src;
	byte *dst;
	byte len, maskbit;
	int lastColumnX, y;
	uint16 color, height, pcolor;
	int scaleIndexY;
	bool masked;

#ifdef USE_ARM_COSTUME_ASM
	if ((_shadowMode == 0xff) &&
		(compData.maskPtr != NULL) &&
		(_shadowTable != NULL))
	{
		compData.scaleXIndex = (byte)ClassicProc3RendererShadowARM(
			_scaleY,
			&compData,
			_out.pitch,
			_srcPtr,
			_height,
			_scaleX,
			compData.scaleXIndex,
			_shadowTable,
			_palette,
			_numStrips,
			compData.scaleYIndex);
		return;
	}
#endif /* USE_ARM_COSTUME_ASM */

	lastColumnX = -1;
	y = compData.y;
	src = _srcPtr;
	dst = compData.destPtr;
	len = compData.repLen;
	color = compData.repColor;
	height = _height;

	scaleIndexY = compData.scaleYIndex;
	maskbit = revBitMask(compData.x & 7);
	mask = compData.maskPtr + compData.x / 8;

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
				if (actorHitResult) {
					if (color && y == actorHitY && compData.x == actorHitX) {
						*actorHitResult = true;
						return;
					}
				} else {
					masked = (y < compData.boundsRect.top || y >= compData.boundsRect.bottom)
							 || (compData.x < compData.boundsRect.left || compData.x >= compData.boundsRect.right)
							 || (compData.maskPtr && (*mask & maskbit));
					bool skipColumn = false;

					if (color && !masked) {
						pcolor = _palette[color];
						if (_shadowMode == 0xff) {
							// classic costume, skipColumn = false
							if (pcolor == 13 && _shadowTable)
								pcolor = _shadowTable[*dst];
						} else if (_shadowMode & 0x20) {
							// classic costume, skipColumn = false
							pcolor = _shadowTable[*dst];
						} else if (_shadowMode == 1) {
							if (pcolor == 13) {
								// In shadow mode 1 skipColumn works more or less the same way as in shadow
								// mode 3. It is only ever checked and applied if pcolor is 13.
								skipColumn = (lastColumnX == compData.x);
								pcolor = _shadowTable[*dst];
							}
						} else if (_shadowMode == 2) {
							error("AkosRenderer::byleRLEDecode(): shadowMode 2 not implemented."); // TODO
						} else if (_shadowMode == 3) {
							if (_vm->_game.features & GF_16BIT_COLOR) {
								// I add the column skip here, too, although I don't know whether it always
								// applies. But this is the only way to prevent recursive shading of pixels.
								// This might need more fine tuning...
								skipColumn = (lastColumnX == compData.x);
								uint16 srcColor = (pcolor >> 1) & 0x7DEF;
								uint16 dstColor = (READ_UINT16(dst) >> 1) & 0x7DEF;
								pcolor = srcColor + dstColor;
							} else if (_vm->_game.heversion >= 90) {
								// I add the column skip here, too, although I don't know whether it always
								// applies. But this is the only way to prevent recursive shading of pixels.
								// This might need more fine tuning...
								skipColumn = (lastColumnX == compData.x);
								pcolor = (pcolor << 8) + *dst;
								pcolor = xmap[pcolor];
							} else if (pcolor < 8) {
								// This mode is used in COMI. The column skip only takes place when the shading
								// is actually applied (for pcolor < 8). The skip avoids shading of pixels that
								// already have been shaded.
								skipColumn = (lastColumnX == compData.x);
								pcolor = (pcolor << 8) + *dst;
								pcolor = _shadowTable[pcolor];
							}
						}
						if (!skipColumn) {
							if (_vm->_bytesPerPixel == 2) {
								WRITE_UINT16(dst, pcolor);
							} else {
								*dst = pcolor;
							}
						}
					}
				}
				dst += _out.pitch;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--compData.skipWidth)
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
					compData.destPtr += compData.scaleXStep * _vm->_bytesPerPixel;
				}

				// From MONKEY1 EGA disasm: we only increment by 1.
				// This accurately produces the original wonky scaling
				// for the floppy editions of Monkey Island 1.
				// Also valid for all other v4 games (this code is
				// also in the executable for LOOM CD).
				if (_vm->_game.version == 4) {
					compData.scaleXIndex = (compData.scaleXIndex + 1) & compData.scaleIndexMask;
				} else {
					compData.scaleXIndex = (compData.scaleXIndex + compData.scaleXStep) & compData.scaleIndexMask;
				}

				dst = compData.destPtr;
				mask = compData.maskPtr + compData.x / 8;
			}
		StartPos:;
		} while (--len);
	} while (true);
}

void BaseCostumeRenderer::skipCelLines(ByleRLEData &compData, int num) {
	num *= _height;

	do {
		compData.repLen = *_srcPtr++;
		compData.repColor = compData.repLen >> compData.shr;
		compData.repLen &= compData.mask;

		if (!compData.repLen)
			compData.repLen = *_srcPtr++;

		do {
			if (!--num)
				return;
		} while (--compData.repLen);
	} while (true);
}

bool ScummEngine::isCostumeInUse(int cost) const {
	Actor *a;

	if (_roomResource != 0)
		for (int i = 1; i < _numActors; i++) {
			a = derefActor(i);
			if (a->isInCurrentRoom() && a->_costume == cost)
				return true;
		}

	return false;
}

} // End of namespace Scumm
