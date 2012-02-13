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


#if !defined(ENABLE_EOB)
#include "kyra/screen.h"
#endif

#ifdef ENABLE_EOB

#include "kyra/eobcommon.h"
#include "kyra/resource.h"

#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"

namespace Kyra {

Screen_EoB::Screen_EoB(EoBCoreEngine *vm, OSystem *system) : Screen(vm, system, _screenDimTable, _screenDimTableCount) {
	_shapeFadeMode[0] = _shapeFadeMode[1] = 0;
	_shapeFadeInternal = 0;
	_fadeData = 0;
	_fadeDataIndex = 0;
	_dsX1 = _dsX2 = _dsY1 = _dsY2 = 0;
	_gfxX = _gfxY = 0;
	_gfxCol = 0;
	_dsTempPage = 0;
	_dsDiv = 0;
	_dsRem = 0;
	_dsScaleTmp = 0;
	_gfxMaxY = 0;
	_egaColorMap = 0;
	_egaPixelValueTable = 0;
	_cgaMappingDefault = 0;
	_cgaDitheringTables[0] = _cgaDitheringTables[1] = 0;
	_useHiResEGADithering = false;
}

Screen_EoB::~Screen_EoB() {
	delete[] _fadeData;
	delete[] _dsTempPage;
	delete[] _egaColorMap;
	delete[] _egaPixelValueTable;
	delete[] _cgaDitheringTables[0];
	delete[] _cgaDitheringTables[1];
	delete[] _cgaDrawCharDitheringTable;
}

bool Screen_EoB::init() {
	if (Screen::init()) {
		int temp;
		_gfxMaxY = _vm->staticres()->loadRawData(kEoBBaseExpObjectY, temp);
		
		if (_renderMode != Common::kRenderCGA && _renderMode != Common::kRenderEGA)
			_fadeData = _vm->resource()->fileData("FADING.DAT", 0);

		if (!_fadeData) {
			_fadeData = new uint8[0x700];
			memset(_fadeData, 0, 0x700);
			if (_renderMode != Common::kRenderCGA && _renderMode != Common::kRenderEGA) {
				uint8 *pal = _vm->resource()->fileData("PALETTE1.PAL", 0);
				for (int i = 0; i < 7; i++)
					createFadeTable(pal, &_fadeData[i << 8], 18, (i + 1) * 36);
				delete[] pal;
			}
		}

		_dsTempPage = new uint8[6000];
		if (_renderMode == Common::kRenderEGA) {
			_egaColorMap = new uint8[256];
			_egaPixelValueTable = new uint8[256];
			for (int i = 0; i < 256; i++) {
				_egaColorMap[i] = i & 0x0f;
				_egaPixelValueTable[i] = i & 0x0f;
			}

			if (_vm->game() == GI_EOB2)
				_useHiResEGADithering = true;
		} else if (_renderMode == Common::kRenderCGA) {
			_cgaMappingDefault = _vm->staticres()->loadRawData(kEoB1CgaMappingDefault, temp);
			_cgaDitheringTables[0] = new uint16[256];
			memset(_cgaDitheringTables[0], 0, 256 * sizeof(uint16));
			_cgaDitheringTables[1] = new uint16[256];
			memset(_cgaDitheringTables[1], 0, 256 * sizeof(uint16));
			_cgaDrawCharDitheringTable = new uint16[256];
			memset(_cgaDrawCharDitheringTable, 0, 256 * sizeof(uint16));
			
			static const uint bits[] = { 0, 3, 12, 15 };
			for (int i = 0; i < 256; i++)
				WRITE_BE_UINT16(&_cgaDrawCharDitheringTable[i], (bits[i & 3] << 8) | (bits[(i >> 2) & 3] << 12) | (bits[(i >> 4) & 3] << 0) | (bits[(i >> 6) & 3] << 4));
		}

		return true;
	}
	return false;
}

void Screen_EoB::setClearScreenDim(int dim) {
	setScreenDim(dim);
	clearCurDim();
}

void Screen_EoB::clearCurDim() {
	fillRect(_curDim->sx << 3, _curDim->sy, ((_curDim->sx + _curDim->w) << 3) - 1, (_curDim->sy + _curDim->h) - 1, _curDim->unkA);
}

void Screen_EoB::setMouseCursor(int x, int y, const byte *shape) {
	if (!shape)
		return;
	int mouseW = shape[2] << 3;
	int mouseH = shape[3];
	uint8 *cursor = new uint8[mouseW * mouseH];
	fillRect(0, 0, mouseW, mouseH, _cursorColorKey, 8);
	drawShape(8, shape, 0, 0, 0);
	CursorMan.showMouse(false);
	copyRegionToBuffer(8, 0, 0, mouseW, mouseH, cursor);
	CursorMan.replaceCursor(cursor, mouseW, mouseH, x, y, _cursorColorKey);
	if (isMouseVisible())
		CursorMan.showMouse(true);
	delete[] cursor;

	// makes sure that the cursor is drawn
	// we do not use Screen::updateScreen here
	// so we can be sure that changes to page 0
	// are NOT updated on the real screen here
	_system->updateScreen();
}

void Screen_EoB::loadFileDataToPage(Common::SeekableReadStream *s, int pageNum, uint32 size) {
	s->read(_pagePtrs[pageNum], size);
}

void Screen_EoB::printShadedText(const char *string, int x, int y, int col1, int col2) {
	printText(string, x - 1, y, 12, col2);
	printText(string, x, y + 1, 12, 0);
	printText(string, x - 1, y + 1, 12, 0);
	printText(string, x, y, col1, 0);
}

void Screen_EoB::loadShapeSetBitmap(const char *file, int tempPage, int destPage) {
	loadEoBBitmap(file, _cgaMappingDefault, tempPage, destPage, -1);
	_curPage = 2;
}

void Screen_EoB::loadEoBBitmap(const char *file, const uint8 *cgaMapping, int tempPage, int destPage, int convertToPage) {
	const char *filePattern = (_vm->game() == GI_EOB1 && (_renderMode == Common::kRenderEGA || _renderMode == Common::kRenderCGA)) ? "%s.EGA" : "%s.CPS";
	Common::String tmp = Common::String::format(filePattern, file);
	Common::SeekableReadStream *s = _vm->resource()->createReadStream(tmp);
	bool loadAlternative = false;
	if (s) {
		// This additional check is necessary since some localized versions of EOB II seem to contain invalid (size zero) cps files
		if (s->size())
			loadBitmap(tmp.c_str(), tempPage, destPage, 0);
		else
			loadAlternative = true;

		delete s;
	} else {
		loadAlternative = true;
	}

	if (loadAlternative) {
		if (_vm->game() == GI_EOB1) {
			tmp.insertChar('1', tmp.size() - 4);
			loadBitmap(tmp.c_str(), tempPage, destPage, 0);
		} else {
			tmp.setChar('X', 0);
			s = _vm->resource()->createReadStream(tmp);
			if (!s)
				error("Screen_EoB::loadEoBBitmap(): Failed to load file '%s'", file);
			s->seek(768);
			loadFileDataToPage(s, destPage, 64000);
			delete s;
		}
	}

	if (convertToPage == -1) {
		return;
	} else if (convertToPage == 2 && _renderMode == Common::kRenderCGA) {
		convertPage(destPage, 4, cgaMapping);
		copyRegion(0, 0, 0, 0, 320, 200, 4, 2, Screen::CR_NO_P_CHECK);
	} else if (convertToPage == 0) {
		convertPage(destPage, 2, cgaMapping);
		copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
	} else {
		convertPage(destPage, convertToPage, cgaMapping);
	}
}

void Screen_EoB::convertPage(int srcPage, int dstPage, const uint8 *cgaMapping) {
	uint8 *src = getPagePtr(srcPage);
	uint8 *dst = getPagePtr(dstPage);
	if (src == dst)
		return;

	if (_renderMode == Common::kRenderCGA) {
		generateCGADitheringTables(cgaMapping);
		uint16 *d = (uint16*)dst;
		uint8 tblSwitch = 0;	
		for (int height = SCREEN_H; height; height--) {
			const uint16 *table = _cgaDitheringTables[(tblSwitch++) & 1];
			for (int width = SCREEN_W / 2; width; width--) {
				*d++ = table[((src[0] & 0x0f) << 4) | (src[1] & 0x0f)];
				src += 2;
			}
		}

		if (dstPage == 0 || dstPage == 1)
			_forceFullUpdate = true;

	} else if (_renderMode == Common::kRenderEGA) {
		uint32 len = SCREEN_W * SCREEN_H;
		if (_useHiResEGADithering) {
			while (len--) {
				uint8 col = _egaColorMap[*src++] & 0x0f;
				*dst++ = col;

				/*for (int i = 4; i; i--) {
					uint8 col = _egaColorMap[*src++];

				}*/
			}
		} else {
			while (len--)
				*dst++ = *src++ & 0x0f;
		}

		if (dstPage == 0 || dstPage == 1)
			_forceFullUpdate = true;
	} else {
		copyPage(srcPage, dstPage);
	}
}

void Screen_EoB::setScreenPalette(const Palette &pal) {
	if (_renderMode == Common::kRenderEGA && _egaColorMap && pal.getNumColors() != 16) {
		const uint8 *src = pal.getData();
		uint8 *dst = _egaColorMap;

		for (int i = 256; i; i--) {
			uint8 r = *src++;
			uint8 g = *src++;
			uint8 b = *src++;

			uint8 col = 0;
			uint16 min = 11907;

			for (int ii = 256; ii; ii--) {
				const uint8 *palEntry = _egaMatchTable + (ii - 1) * 3;
				if (*palEntry == 0xff)
					continue;

				int e_r = palEntry[0];
				e_r -= r;
				int e_g = palEntry[1];
				e_g -= g;
				int e_b = palEntry[2];
				e_b -= b;

				int s = (e_r * e_r) + (e_g * e_g) + (e_b * e_b);

				if (s < min) {
					min = s;
					col = ii - 1;
				}
			}
			*dst++ = col;
		}

		memset(_egaPixelValueTable, 0, 256);
		for (int i = 0; i < 256; i++)
			_egaPixelValueTable[_egaColorMap[i]] = i;

	} else if (_renderMode == Common::kRenderEGA) {
		_screenPalette->copy(pal);
		_system->getPaletteManager()->setPalette(_screenPalette->getData(), 0, _screenPalette->getNumColors());

	} else if (_renderMode != Common::kRenderCGA) {
		Screen::setScreenPalette(pal);
	}
}

uint8 *Screen_EoB::encodeShape(uint16 x, uint16 y, uint16 w, uint16 h, bool no4bitEncoding, const uint8 *cgaMapping) {
	uint8 *shp = 0;
	uint16 shapesize = 0;

	uint8 *srcPage = getPagePtr(_curPage | 1) + y * 320 + (x << 3);
	uint8 *src = srcPage;

	if (no4bitEncoding) {
		uint16 h1 = h;
		while (h1--) {
			uint8 *lineEnd = src + (w << 3);
			do {
				if (!*src++) {
					shapesize++;
					uint8 *startZeroPos = src;
					while (src != lineEnd && *src == 0)
						src++;

					uint16 numZero = src - startZeroPos + 1;
					if (numZero >> 8)
						shapesize += 2;
				}
				shapesize++;
			} while (src != lineEnd);

			srcPage += 320;
			src = srcPage;
		}

		shapesize += 4;

		shp = new uint8[shapesize];
		memset(shp, 0, shapesize);
		uint8 *dst = shp;

		*dst++ = 0;
		*dst++ = (h & 0xff);
		*dst++ = (w & 0xff);
		*dst++ = (h & 0xff);

		srcPage = getPagePtr(_curPage | 1) + y * 320 + (x << 3);
		src = srcPage;

		h1 = h;
		while (h1--) {
			uint8 *lineEnd = src + (w << 3);
			do {
				uint8 val = *src++;
				if (!val) {
					*dst++ = 0;
					uint8 *startZeroPos = src;

					while (src != lineEnd && *src == 0)
						src++;

					uint16 numZero = src - startZeroPos + 1;
					if (numZero >> 8) {
						*dst++ = 255;
						*dst++ = 0;
						numZero -= 255;
					}
					val = numZero & 0xff;
				}
				*dst++ = val;
			} while (src != lineEnd);

			srcPage += 320;
			src = srcPage;
		}

	} else {
		uint8 nib = 0, col = 0;
		uint8 *colorMap = new uint8[0x100];
		memset(colorMap, 0xff, 0x100);

		shapesize = h * (w << 2) + 20;
		shp = new uint8[shapesize];
		memset(shp, 0, shapesize);
		uint8 *dst = shp;

		*dst++ = 1;
		*dst++ = (h & 0xff);
		*dst++ = (w & 0xff);
		*dst++ = (h & 0xff);
		memset(dst, 0xff, 0x10);

		uint8 *pal = dst;
		dst += 0x10;

		srcPage = getPagePtr(_curPage | 1) + y * 320 + (x << 3);
		src = srcPage;
		nib = col = 0;

		uint16 h1 = h;
		while (h1--) {
			uint16 w1 = w << 3;
			while (w1--) {
				uint8 s = *src++;
				uint8 c = colorMap[s];
				if (c == 0xff) {
					if (col < 0x10) {
						*pal++ = s;
						c = colorMap[s] = col++;
						if (!col)
							c = 0;
					} else {
						c = 0;
					}
				}

				if (++nib & 1) {
					*dst = c << 4;
				} else {
					*dst++ |= c;
				}
			}
			srcPage += 320;
			src = srcPage;
		}
		delete [] colorMap;
	}

	return shp;
}

void Screen_EoB::drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...) {
	uint8 *dst = getPagePtr(pageNum);
	const uint8 *src = shapeData;

	if (!src)
		return;

	va_list args;
	va_start(args, flags);
	uint8 *ovl = (flags & 2) ? va_arg(args, uint8 *) : 0;
	va_end(args);

	if (sd != -1) {
		const ScreenDim *dm = getScreenDim(sd);
		setShapeFrame(dm->sx, dm->sy, dm->sx + dm->w, dm->sy + dm->h);
		x += (_dsX1 << 3);
		y += _dsY1;
	}

	dst += (_dsX1 << 3);
	int16 dX = x - (_dsX1 << 3);
	int16 dY = y;
	int16 dW = _dsX2 - _dsX1;
	uint8 flag = *src++;

	uint16 dH = *src++;
	uint16 width = (*src++) << 3;
	src++;

	int rX = x;
	int rY = y;
	int rW = width + 8;
	int rH = dH;

	uint16 w2 = width;
	int d = dY - _dsY1;

	int pixelStep = (flags & 1) ? -1 : 1;

	if (flag) {
		const uint8 *pal = ovl ? ovl : src;
		src += 16;

		if (d < 0) {
			d = -d;
			if (d >= dH)
				return;
			src += (d * (width >> 1));
			d = dY + dH - _dsY1;
			if (d >= 0) {
				dH = d;
				dY = _dsY1;
				d = _dsY2 - dY;
			}
		} else {
			d = _dsY2 - dY;
		}

		if (d < 1)
			return;

		if (d < dH)
			dH = d;

		int16 cnt1 = 0;
		int16 cnt2 = 0;
		int16 dXbit1 = dX & 1;

		if (dX < 0) {
			width += dX;
			d = -dX;
			if (flags & 1)
				src -= (d >> 1);
			else
				src += (d >> 1);

			if (d >= w2)
				return;

			dX = 0;
			cnt1++;
		}

		d = (dW << 3) - dX;

		if (d < 1)
			return;

		if (d < width) {
			width = d;
			cnt2++;
		}

		dst += (dY * 320 + dX);

		if (pageNum == 0 || pageNum == 1)
			addDirtyRect(rX, rY, rW, rH);

		int w3 = w2;
		dY = 320 - width;
		width >>= 1;
		w2 >>= 1;
		if (flags & 1)
			src += (w2 - 1);

		int16 w1shr = width;

		if (cnt1 && (dXbit1 & 1)) {
			w1shr++;
			w2++;
			if (!cnt2)
				dY += 2;
		}

		if (cnt2 && (dXbit1 & 1))
			w1shr++;

		int lineSrcStep = (w2 - w1shr);
		if (flags & 1)
			lineSrcStep = w3 - lineSrcStep;

		while (dH--) {
			int16 hpos = width;
			uint8 col = 0;
			uint8 b = 0;
			uint8 nextloop = 0;

			if (cnt1 && dXbit1) {
				if (!hpos)
					return;
				b = *src;
				src += pixelStep;
				nextloop = 2;
			} else {
				nextloop = hpos ? 1 : 3;
			}

			while (nextloop) {
				switch (nextloop) {
				case 1:
					b = *src;
					src += pixelStep;
					col = pal[(flags & 1) ? (b & 0x0f) : (b >> 4)];
					if (col)
						drawShapeSetPixel(dst, col);
					dst++;

				case 2:
					col = pal[(flags & 1) ? (b >> 4) : (b & 0x0f)];

					if (!col) {
						nextloop = 4;
						break;
					}

					drawShapeSetPixel(dst++, col);
					nextloop = --hpos ? 1 : 3;
					break;

				case 3:
					if (cnt2 && dXbit1) {
						col = pal[(flags & 1) ? (*src & 0x0f) : (*src >> 4)];
						src += pixelStep;
						if (col)
							drawShapeSetPixel(dst, col);
						dst++;
					}

					src += lineSrcStep;
					dst += dY;
					nextloop = 0;
					break;

				case 4:
					dst++;
					nextloop = --hpos ? 1 : 3;
					break;
				}
			}
		}
	} else {
		uint16 marginLeft = 0;
		uint16 marginRight = 0;

		if (d < 0) {
			dH += d;
			if (dH <= 0)
				return;
			d = -d;

			for (int i = 0; i < d; i++) {
				marginLeft = width;
				for (int ii = 0; ii < marginLeft; ii++) {
					if (!*src++)
						marginLeft = marginLeft + 1 - *src++;
				}
			}
			dY = _dsY1;
		}

		d = _dsY2 - dY;

		if (d < 1)
			return;

		if (d < dH)
			dH = d;

		marginLeft = 0;

		if (dX < 0) {
			width += dX;
			marginLeft = -dX;

			if (marginLeft >= w2)
				return;

			dX = 0;
		}

		marginRight = 0;
		d = (dW << 3) - dX;

		if (d < 1)
			return;

		if (d < width) {
			width = d;
			marginRight = w2 - marginLeft - width;
		}

		dst += (dY * 320 + dX);
		uint8 *dstL = dst;

		if (pageNum == 0 || pageNum == 1)
			addDirtyRect(rX, rY, rW, rH);

		while (dH--) {
			int16 xpos = (int16) marginLeft;

			if (flags & 1) {
				for (int i = 0; i < w2; i++) {
					if (*src++ == 0) {
						i += (*src - 1);
						src += (*src - 1);
					}
				}
				src--;
			}
			const uint8 *src2 = src;

			if (xpos) {
				do {
					uint8 val = (flags & 1) ? *(src - 1) : *src;
					while (val && xpos) {
						src += pixelStep;
						xpos--;
						val = (flags & 1) ? *(src - 1) : *src;
					}

					val = (flags & 1) ? *(src - 1) : *src;
					if (!val) {
						src += pixelStep;
						uint8 bt = (flags & 1) ? src[1] : src[0];
						src += pixelStep;
						xpos = xpos - bt;
					}
				} while (xpos > 0);
			}

			dst -= xpos;
			xpos += width;

			while (xpos > 0) {
				uint8 c = *src;
				uint8 m = (flags & 1) ? *(src - 1) : c;
				src += pixelStep;

				if (m) {
					drawShapeSetPixel(dst++, c);
					xpos--;
				} else {
					uint8 len = (flags & 1) ? src[1] : src[0];
					dst += len;
					xpos -= len;
					src += pixelStep;
				}
			}
			xpos += marginRight;

			if (xpos) {
				do {
					uint8 val = (flags & 1) ? *(src - 1) : *src;
					while (val && xpos) {
						src += pixelStep;
						xpos--;
						val = (flags & 1) ? *(src - 1) : *src;
					}

					val = (flags & 1) ? *(src - 1) : *src;
					if (!val) {
						src += pixelStep;
						uint8 bt = (flags & 1) ? src[1] : src[0];
						src += pixelStep;
						xpos = xpos - bt;
					}
				} while (xpos > 0);
			}

			dstL += 320;
			dst = dstL;
			if (flags & 1)
				src = src2 + 1;
		}
	}
}

const uint8 *Screen_EoB::scaleShape(const uint8 *shapeData, int steps) {
	setShapeFadeMode(1, steps ? true : false);

	while (shapeData && steps--)
		shapeData = scaleShapeStep(shapeData);

	return shapeData;
}

const uint8 *Screen_EoB::scaleShapeStep(const uint8 *shp) {
	uint8 *d = _dsTempPage;
	*d++ = *shp++;

	uint16 h = (*shp++) + 1;
	d[0] = d[2] = (h << 1) / 3;
	d++;

	uint16 w = *shp++;
	uint16 w2 = w << 2;
	uint16 t = ((w << 1) % 3) ? 1 : 0;
	*d++ = ((w << 1) / 3) + t;

	shp++;
	d++;

	int i = 0;
	while (i < 16) {
		if (!shp[i]) {
			i = -i;
			break;
		}
		i++;
	}

	if (i >= 0)
		i = 0;
	else
		i = -i;

	_dsScaleTmp = (i << 4) | (i & 0x0f);
	for (int ii = 0; ii < 16; ii++)
		*d++ = *shp++;

	_dsDiv = w2 / 3;
	_dsRem = w2 % 3;

	while (--h) {
		scaleShapeProcessLine(d, shp);
		if (!--h)
			break;
		scaleShapeProcessLine(d, shp);
		if (!--h)
			break;
		shp += w2;
	}

	return (const uint8 *)_dsTempPage;
}

void Screen_EoB::replaceShapePalette(uint8 *shp, const uint8 *pal) {
	if (*shp != 1)
		return;
	shp += 4;
	memcpy(shp, pal, 16);
}

void Screen_EoB::applyShapeOverlay(uint8 *shp, int ovlIndex) {
	if (*shp != 1)
		return;
	shp += 4;
	uint8 *ovl = getFadeTable(ovlIndex);
	for (int i = 0; i < 16; i++)
		shp[i] = ovl[shp[i]];
}

void Screen_EoB::setShapeFrame(int x1, int y1, int x2, int y2) {
	_dsX1 = x1;
	_dsY1 = y1;
	_dsX2 = x2;
	_dsY2 = y2;
}

void Screen_EoB::setShapeFadeMode(uint8 i, bool b) {
	if (!i || i == 1)
		_shapeFadeMode[i] = b;
}

void Screen_EoB::setGfxParameters(int x, int y, int col) {
	_gfxX = x;
	_gfxY = y;
	_gfxCol = col;
}

void Screen_EoB::drawExplosion(int scale, int radius, int numElements, int stepSize, int aspectRatio, const uint8 *colorTable, int colorTableSize) {
	int ymin = 0;
	int ymax = _gfxMaxY[scale];
	int xmin = -100;
	int xmax = 276;

	if (scale)
		--scale;

	hideMouse();

	const ScreenDim *dm = getScreenDim(5);
	int rX1 = dm->sx << 3;
	int rY1 = dm->sy;
	int rX2 = rX1 + (dm->w << 3);
	int rY2 = rY1 + dm->h - 1;

	int16 gx2 = _gfxX;
	int16 gy2 = _gfxY;

	int16 *ptr2 = (int16 *)_dsTempPage;
	int16 *ptr3 = (int16 *)&_dsTempPage[300];
	int16 *ptr4 = (int16 *)&_dsTempPage[600];
	int16 *ptr5 = (int16 *)&_dsTempPage[900];
	int16 *ptr6 = (int16 *)&_dsTempPage[1200];
	int16 *ptr7 = (int16 *)&_dsTempPage[1500];
	int16 *ptr8 = (int16 *)&_dsTempPage[1800];

	if (numElements > 150)
		numElements = 150;

	for (int i = 0; i < numElements; i++) {
		ptr2[i] = ptr3[i] = 0;
		ptr4[i] = _vm->_rnd.getRandomNumberRng(0, radius) - (radius >> 1);
		ptr5[i] = _vm->_rnd.getRandomNumberRng(0, radius) - (radius >> 1) - (radius >> (8 - aspectRatio));
		ptr7[i] = _vm->_rnd.getRandomNumberRng(1024 / stepSize, 2048 / stepSize);
		ptr8[i] = scale << 8;
	}

	for (int l = 2; l;) {
		if (l != 2) {
			for (int i = numElements - 1; i >= 0; i--) {
				int16 px = ((ptr2[i] >> 6) >> scale) + gx2;
				int16 py = ((ptr3[i] >> 6) >> scale) + gy2;
				if (py > ymax)
					py = ymax;
				if (posWithinRect(px, py, rX1, rY1, rX2, rY2))
					setPagePixel(0, px, py, ptr6[i]);
			}
		}

		l = 0;

		for (int i = 0; i < numElements; i++) {
			uint32 end = _system->getMillis() + 1;
			if (ptr4[i] <= 0)
				ptr4[i]++;
			else
				ptr4[i]--;
			ptr2[i] += ptr4[i];
			ptr5[i] += 5;
			ptr3[i] += ptr5[i];
			ptr8[i] += ptr7[i];

			int16 px = ((ptr2[i] >> 6) >> scale) + gx2;
			int16 py = ((ptr3[i] >> 6) >> scale) + gy2;
			if (py >= ymax || py < ymin)
				ptr5[i] = -(ptr5[i] >> 1);
			if (px >= xmax || px < xmin)
				ptr4[i] = -(ptr4[i] >> 1);

			if (py > ymax)
				py = ymax;

			int pxVal1 = 0;
			if (posWithinRect(px, py, 0, 0, 319, 199)) {
				pxVal1 = getPagePixel(2, px, py);
				ptr6[i] = getPagePixel(0, px, py);
			}

			assert((ptr8[i] >> 8) < colorTableSize);
			int pxVal2 = colorTable[ptr8[i] >> 8];
			if (pxVal2) {
				l = 1;
				if (pxVal1 == _gfxCol && posWithinRect(px, py, rX1, rY1, rX2, rY2)) {
					setPagePixel(0, px, py, pxVal2);
					if (i % 5 == 0)  {
						updateScreen();
						uint32 cur = _system->getMillis();
						if (end > cur)
							_system->delayMillis(end - cur);
					}
				}
			} else {
				ptr7[i] = 0;
			}
		}
	}

	showMouse();
}

void Screen_EoB::drawVortex(int numElements, int radius, int stepSize, int, int disorder, const uint8 *colorTable, int colorTableSize) {
	int16 *xCoords = (int16 *)_dsTempPage;
	int16 *yCoords = (int16 *)&_dsTempPage[300];
	int16 *xMod = (int16 *)&_dsTempPage[600];
	int16 *yMod = (int16 *)&_dsTempPage[900];
	int16 *pixBackup = (int16 *)&_dsTempPage[1200];
	int16 *colTableStep = (int16 *)&_dsTempPage[1500];
	int16 *colTableIndex = (int16 *)&_dsTempPage[1800];
	int16 *pixDelay = (int16 *)&_dsTempPage[2100];

	hideMouse();
	int cp = _curPage;

	if (numElements > 150)
		numElements = 150;

	int cx = 88;
	int cy = 48;
	radius <<= 6;

	for (int i = 0; i < numElements; i++) {
		int16 v38 = _vm->_rnd.getRandomNumberRng(radius >> 2, radius);
		int16 stepSum = 0;
		int16 sqsum = 0;
		while (sqsum < v38) {
			stepSum += stepSize;
			sqsum += stepSum;
		}

		switch (_vm->_rnd.getRandomNumber(255) & 3) {
		case 0:
			xCoords[i] = 32;
			yCoords[i] = sqsum;
			xMod[i] = stepSum;
			yMod[i] = 0;
			break;

		case 1:
			xCoords[i] = sqsum;
			yCoords[i] = 32;
			xMod[i] = 0;
			yMod[i] = stepSum;
			break;

		case 2:
			xCoords[i] = 32;
			yCoords[i] = -sqsum;
			xMod[i] = stepSum;
			yMod[i] = 0;
			break;

		default:
			xCoords[i] = -sqsum;
			yCoords[i] = 32;
			xMod[i] = 0;
			yMod[i] = stepSum;
			break;
		}

		if (_vm->_rnd.getRandomBit()) {
			xMod[i] *= -1;
			yMod[i] *= -1;
		}

		colTableStep[i] = _vm->_rnd.getRandomNumberRng(1024 / disorder, 2048 / disorder);
		colTableIndex[i] = 0;
		pixDelay[i] = _vm->_rnd.getRandomNumberRng(0, disorder >> 2);
	}

	int d = 0;
	for (int i = 2; i;) {
		if (i != 2) {
			for (int ii = numElements - 1; ii >= 0; ii--) {
				int16 px = CLIP((xCoords[ii] >> 6) + cx, 0, SCREEN_W - 1);
				int16 py = CLIP((yCoords[ii] >> 6) + cy, 0, SCREEN_H - 1);
				setPagePixel(0, px, py, pixBackup[ii]);
			}
		}

		i = 0;
		int r = (stepSize >> 1) + (stepSize >> 2) + (stepSize >> 3);
		uint32 nextDelay = _system->getMillis() + 1;

		for (int ii = 0; ii < numElements; ii++) {
			if (pixDelay[ii] == 0) {
				if (xCoords[ii] > 0) {
					xMod[ii] -= ((xMod[ii] > 0) ? stepSize : r);
				} else {
					xMod[ii] += ((xMod[ii] < 0) ? stepSize : r);
				}

				if (yCoords[ii] > 0) {
					yMod[ii] -= ((yMod[ii] > 0) ? stepSize : r);
				} else {
					yMod[ii] += ((yMod[ii] < 0) ? stepSize : r);
				}

				xCoords[ii] += xMod[ii];
				yCoords[ii] += yMod[ii];
				colTableIndex[ii] += colTableStep[ii];

			} else {
				pixDelay[ii]--;
			}

			int16 px = CLIP((xCoords[ii] >> 6) + cx, 0, SCREEN_W - 1);
			int16 py = CLIP((yCoords[ii] >> 6) + cy, 0, SCREEN_H - 1);

			uint8 tc1 = ((disorder >> 2) <= d) ? getPagePixel(2, px, py) : 0;
			pixBackup[ii] = getPagePixel(0, px, py);
			uint8 tblIndex = CLIP(colTableIndex[ii] >> 8, 0, colorTableSize - 1);
			uint8 tc2 = colorTable[tblIndex];

			if (tc2) {
				i = 1;
				if (tc1 == _gfxCol && !pixDelay[ii]) {
					setPagePixel(0, px, py, tc2);
					if (ii % 15 == 0)  {
						updateScreen();
						uint32 cur = _system->getMillis();
						if (nextDelay > cur)
							_system->delayMillis(nextDelay - cur);
						nextDelay += 1;
					}
				}
			} else {
				colTableStep[ii] = 0;
			}
		}
		d++;
	}

	_curPage = cp;
	showMouse();
}

void Screen_EoB::fadeTextColor(Palette *pal, int color1, int rate) {
	uint8 *col = pal->getData();

	for (bool loop = true; loop;) {
		loop = true;
		uint32 end = _system->getMillis() + _vm->tickLength();

		loop = false;
		for (int ii = 0; ii < 3; ii++) {
			uint8 c = col[color1 * 3 + ii];
			if (c > rate) {
				col[color1 * 3 + ii] -= rate;
				loop = true;
			} else if (c) {
				col[color1 * 3 + ii] = 0;
				loop = true;
			}
		}

		if (loop) {
			setScreenPalette(*pal);
			updateScreen();
			uint32 cur = _system->getMillis();
			if (end > cur)
				_system->delayMillis(end - cur);
		}
	}
}

bool Screen_EoB::delayedFadePalStep(Palette *fadePal, Palette *destPal, int rate) {
	bool res = false;

	uint8 *s = fadePal->getData();
	uint8 *d = destPal->getData();

	for (int i = 0; i < 765; i++) {
		int fadeVal = *s++;
		int dstCur = *d;
		int diff = ABS(fadeVal - dstCur);

		if (diff == 0) {
			d++;
			continue;
		}

		res = true;
		diff = MIN(diff, rate);

		if (dstCur < fadeVal)
			*d += diff;
		else
			*d -= diff;
		d++;
	}

	return res;
}

int Screen_EoB::getRectSize(int w, int h) {
	return w * h;
}

void Screen_EoB::setFadeTableIndex(int index) {
	_fadeDataIndex = (CLIP(index, 0, 7) << 8);
}

void Screen_EoB::createFadeTable(uint8 *palData, uint8 *dst, uint8 rootColor, uint8 weight) {
	if (!palData)
		return;

	uint8 *src = palData + 3 * rootColor;
	uint8 r = *src++;
	uint8 g = *src++;
	uint8 b = *src;
	uint8 tr, tg, tb;
	src = palData + 3;

	*dst++ = 0;
	weight >>= 1;

	for (uint8 i = 1; i; i++) {
		uint16 tmp = (uint16)((*src - r) * weight) << 1;
		tr = *src++ - ((tmp >> 8) & 0xff);
		tmp = (uint16)((*src - g) * weight) << 1;
		tg = *src++ - ((tmp >> 8) & 0xff);
		tmp = (uint16)((*src - b) * weight) << 1;
		tb = *src++ - ((tmp >> 8) & 0xff);

		uint8 *d = palData + 3;
		uint16 v = 0xffff;
		uint8 col = rootColor;

		for (uint8 ii = 1; ii; ii++) {
			int a = *d++ - tr;
			int t = a * a;
			a = *d++ - tg;
			t += (a * a);
			a = *d++ - tb;
			t += (a * a);

			if (t <= v && (ii == rootColor || ii != i)) {
				v = t;
				col = ii ;
			}
		}
		*dst++ = col;
	}
}

uint8 *Screen_EoB::getFadeTable(int index) {
	return (index >= 0 && index < 5) ? &_fadeData[index << 8] : 0;
}

void Screen_EoB::drawShapeSetPixel(uint8 *dst, uint8 c) {
	if (_shapeFadeMode[0]) {
		if (_shapeFadeMode[1]) {
			c = *dst;
		} else {
			_shapeFadeInternal &= 7;
			c = *(dst + _shapeFadeInternal++);
		}
	}

	if (_shapeFadeMode[1]) {
		uint8 cnt = _shapeFadeMode[1];
		while (cnt--)
			c = _fadeData[_fadeDataIndex + c];
	}

	*dst = c;
}

void Screen_EoB::scaleShapeProcessLine(uint8 *&dst, const uint8 *&src) {
	for (int i = 0; i < _dsDiv; i++) {
		*dst++ = *src++;
		*dst++ = (READ_BE_UINT16(src) >> 4) & 0xff;
		src += 2;
	}

	if (_dsRem == 1) {
		*dst++ = *src++;
		*dst++ = _dsScaleTmp;
	} else if (_dsRem == 2) {
		*dst++ = (src[0] & 0xf0) | (src[1] >> 4);
		src += 2;
		*dst++ = _dsScaleTmp;
		*dst++ = _dsScaleTmp;
		*dst++ = _dsScaleTmp;
	}
}

bool Screen_EoB::posWithinRect(int posX, int posY, int x1, int y1, int x2, int y2) {
	if (posX < x1 || posX > x2 || posY < y1 || posY > y2)
		return false;
	return true;
}

void Screen_EoB::generateCGADitheringTables(const uint8 *mappingData) {
	for (int i = 0; i < 256; i++) {
		WRITE_BE_UINT16(&_cgaDitheringTables[0][i], (mappingData[i >> 4] << 8) | mappingData[(i & 0x0f) + 16]);
		WRITE_BE_UINT16(&_cgaDitheringTables[1][i], (mappingData[(i >> 4) + 16] << 8) | mappingData[i & 0x0f]);
	}
}

const uint8 Screen_EoB::_egaMatchTable[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x1E, 0x00, 0x00, 0x1E, 0x1E, 0x1E, 0x00, 0x00, 0x1E,
	0x00, 0x1E, 0x1E, 0x0F, 0x00, 0x1E, 0x1E, 0x1E, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x2D, 0x0F, 0x2D,
	0x0F, 0x0F, 0x2D, 0x2D, 0x2D, 0x0F, 0x0F, 0x2D, 0x0F, 0x2D, 0x2D, 0x2D, 0x0F, 0x2D, 0x2D, 0x2D,
	0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x2A, 0x00, 0x1E, 0x1E, 0x00, 0x1E, 0x2A, 0x1E, 0x00, 0x1E, 0x1E,
	0x00, 0x2A, 0x1E, 0x0F, 0x1E, 0x1E, 0x1E, 0x2A, 0x0F, 0x0F, 0x21, 0x0F, 0x0F, 0x36, 0x0F, 0x2D,
	0x21, 0x0F, 0x2D, 0x36, 0x2D, 0x0F, 0x21, 0x2D, 0x0F, 0x36, 0x2D, 0x2D, 0x21, 0x2D, 0x2D, 0x36,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x2A, 0x00, 0x00, 0x2A, 0x1E, 0x1E, 0x1E, 0x00, 0x1E,
	0x1E, 0x1E, 0x1E, 0x21, 0x00, 0x1E, 0x2A, 0x1E, 0x0F, 0x21, 0x0F, 0x0F, 0x21, 0x2D, 0x0F, 0x36,
	0x0F, 0x0F, 0x36, 0x2D, 0x2D, 0x21, 0x0F, 0x2D, 0x21, 0x2D, 0x2D, 0x36, 0x0F, 0x2D, 0x36, 0x2D,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x2A, 0x2A, 0x1E, 0x1E, 0x1E, 0x1E,
	0x1E, 0x2A, 0x1E, 0x21, 0x1E, 0x1E, 0x2A, 0x2A, 0x0F, 0x21, 0x21, 0x0F, 0x21, 0x36, 0x0F, 0x36,
	0x21, 0x0F, 0x36, 0x36, 0x2D, 0x21, 0x21, 0x2D, 0x21, 0x36, 0x2D, 0x36, 0x21, 0x2D, 0x36, 0x36,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2A, 0x00, 0x00, 0x2A,
	0x00, 0x1E, 0x2A, 0x0F, 0x00, 0x2A, 0x1E, 0x1E, 0x21, 0x0F, 0x0F, 0x21, 0x0F, 0x2D, 0x21, 0x2D,
	0x0F, 0x21, 0x2D, 0x2D, 0x36, 0x0F, 0x0F, 0x36, 0x0F, 0x2D, 0x36, 0x2D, 0x0F, 0x36, 0x2D, 0x2D,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2A,
	0x00, 0x2A, 0x2A, 0x0F, 0x1E, 0x2A, 0x1E, 0x2A, 0x21, 0x0F, 0x21, 0x21, 0x0F, 0x36, 0x21, 0x2D,
	0x21, 0x21, 0x2D, 0x36, 0x36, 0x0F, 0x21, 0x36, 0x0F, 0x36, 0x36, 0x2D, 0x21, 0x36, 0x2D, 0x36,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0x2A, 0x15, 0x00, 0x2A, 0x21, 0x1E, 0x21, 0x15, 0x0F, 0x21, 0x15, 0x2D, 0x21, 0x2F,
	0x0F, 0x21, 0x2F, 0x2D, 0x36, 0x15, 0x0F, 0x36, 0x15, 0x2D, 0x36, 0x2F, 0x0F, 0x36, 0x2F, 0x2D,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2A, 0x2A, 0x2A, 0x21, 0x21, 0x21, 0x21, 0x21, 0x36, 0x21, 0x36,
	0x21, 0x21, 0x36, 0x36, 0x36, 0x21, 0x21, 0x36, 0x21, 0x36, 0x36, 0x36, 0x21, 0x36, 0x36, 0x36,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x15, 0x15, 0x15, 0x15, 0x15, 0x2F, 0x15, 0x2F,
	0x15, 0x15, 0x2F, 0x2F, 0x2F, 0x15, 0x15, 0x2F, 0x15, 0x2F, 0x2F, 0x2F, 0x15, 0x2F, 0x2F, 0x2F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x15, 0x15, 0x3F, 0x15, 0x2F,
	0x2F, 0x15, 0x2F, 0x3F, 0x2F, 0x15, 0x2F, 0x2F, 0x15, 0x3F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x3F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x15, 0x3F,
	0x15, 0x15, 0x3F, 0x2F, 0x2F, 0x2F, 0x15, 0x2F, 0x2F, 0x2F, 0x2F, 0x3F, 0x15, 0x2F, 0x3F, 0x2F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x15, 0x3F, 0x3F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x3F, 0x2F, 0x3F, 0x2F, 0x2F, 0x3F, 0x3F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x15, 0x15, 0x3F, 0x15, 0x2F, 0x3F, 0x2F, 0x15, 0x3F, 0x2F, 0x2F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x15, 0x3F, 0x3F, 0x2F, 0x2F, 0x3F, 0x2F, 0x3F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x3F, 0x15, 0x3F, 0x3F, 0x2F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x3F, 0x3F
};

OldDOSFont::OldDOSFont(Common::RenderMode mode, const uint16 *cgaDitheringTable) : _renderMode(mode), _cgaDitheringTable(cgaDitheringTable) {
	_data = 0;
	_width = _height = _numGlyphs = 0;
	_bitmapOffsets = 0;
}

OldDOSFont::~OldDOSFont() {
	unload();
}

bool OldDOSFont::load(Common::SeekableReadStream &file) {
	unload();

	_data = new uint8[file.size()];
	assert(_data);

	file.read(_data, file.size());
	if (file.err())
		return false;

	if (file.size() - 2 != READ_LE_UINT16(_data))
		return false;

	_width = _data[0x103];
	_height = _data[0x102];
	_numGlyphs = 255;

	_bitmapOffsets = (uint16 *)(_data + 2);

	for (int i = 0; i < _numGlyphs; ++i)
		_bitmapOffsets[i] = READ_LE_UINT16(&_bitmapOffsets[i]);

	return true;
}

int OldDOSFont::getCharWidth(uint16 c) const {
	if (c >= _numGlyphs)
		return 0;
	return _width;
}

void OldDOSFont::drawChar(uint16 c, byte *dst, int pitch) const {
	static const uint8 renderMaskTable6[] = { 0xFC, 0x00, 0x7E, 0x00, 0x3F, 0x00, 0x1F, 0x80, 0x0F, 0xC0, 0x07, 0xE0, 0x03, 0xF0, 0x01, 0xF8 };
	static const uint8 renderMaskTable8[] = { 0xFF, 0x00, 0x7F, 0x80, 0x3F, 0xC0, 0x1F, 0xE0, 0x0F, 0xF0, 0x07, 0xF8, 0x03, 0xFC, 0x01, 0xFE };

	if (_width != 8 && _width != 6)
		error("EOB font rendering not implemented for other font widths than 6 and 8.");

	if (_width == 6) {
		switch (c) {
		case 0x81:
		case 0x9a:
			c = 0x5d;
			break;
		case 0x84:
		case 0x8e:
			c = 0x5b;
			break;
		case 0x94:
		case 0x99:
			c = 0x40;
		case 0xe1:
			// TODO: recheck this: no conversion for 'ß' ?
			break;
		}
	} else if (_width == 8) {
		switch (c) {
		case 0x81:
		case 0x9a:
		case 0x5d:
			c = 0x1d;
			break;
		case 0x84:
		case 0x5b:
			c = 0x1e;
			break;
		case 0x94:
		case 0x40:
			c = 0x1f;
			break;
		case 0x8e:
			c = 0x1b;
			break;
		case 0x99:
			c = 0x1c;
			break;
		case 0xe1:
			c = 0x19;
			break;
		}
	}

	const uint8 *src = &_data[_bitmapOffsets[c]];

	int w = (_width - 1) >> 3;
	pitch -= _width;

	uint8 color1 = _colorMap[1];
	uint8 color2 = _colorMap[0];

	static const uint16 cgaColorMask[] = { 0, 0x5555, 0xAAAA, 0xFFFF };
	uint16 cgaMask1 = cgaColorMask[color1 & 3];
	uint16 cgaMask2 = cgaColorMask[color2 & 3];

	int cH = _height;
	while (cH--) {
		int cW = w;
		uint8 last = 0;
		const uint8 *mtbl = _width == 8 ? renderMaskTable8 : renderMaskTable6;

		if (_renderMode == Common::kRenderCGA) {
			uint8 s = *src++;
			uint8 m = *mtbl++;

			uint8 in = s | last;
			uint16 cmp1 = 0;
			uint16 cmp2 = 0;
			
			if (color1) {
				in &= m;
				cmp1 = _cgaDitheringTable[in];
			}

			if (color2) {
				in = ~in & m;
				cmp2 = _cgaDitheringTable[in];
			}

			uint16 cDst = 0;
			uint8 sh = 14;
			for (int i = 0; i < _width; i++) {
				cDst |= ((dst[i] & 3) << sh);
				sh -= 2;
			}

			uint16 out = (~(cmp1 | cmp2) & cDst) | (cmp1 & cgaMask1) | (cmp2 & cgaMask2);

			sh = 14;
			for (int i = 0; i < _width; i++) {
				*dst++ = (out >> sh) & 3;
				sh -= 2;
			}

			last = s;
		} else {
			for (bool runWidthLoop = true; runWidthLoop;) {
				uint8 s = *src++;
				uint8 m = *mtbl++;

				for (uint8 i = 0x80; i; i >>= 1) {
					if (!(m & i)) {
						runWidthLoop = false;
						break;
					}

					if (_renderMode == Common::kRenderCGA) {
						uint8 in = s | last;
						if (s & i) {
							if (color1)
								*dst = color1;
						} else if (color2) {
							*dst = color2;
						}
						last = s;
					} else {
						if (s & i) {
							if (color1)
								*dst = color1;
						} else if (color2) {
							*dst = color2;
						}
					}
					dst++;
				}

				if (cW)
					cW--;
				else
					runWidthLoop = false;
			}
		}
		dst += pitch;
	}
}

void OldDOSFont::unload() {
	delete[] _data;
	_data = 0;
	_width = _height = _numGlyphs = 0;
	_bitmapOffsets = 0;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
