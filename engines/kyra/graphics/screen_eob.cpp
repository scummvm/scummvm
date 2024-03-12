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


#if !defined(ENABLE_EOB)
#include "kyra/graphics/screen.h"
#endif

#ifdef ENABLE_EOB

#include "kyra/engine/eobcommon.h"
#include "kyra/resource/resource.h"
#include "kyra/engine/util.h"
#include "kyra/graphics/screen_eob_segacd.h"

#include "common/system.h"
#include "common/memstream.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "graphics/sjis.h"
#include "graphics/fonts/dosfont.h"

#define EXPLOSION_ANIM_DURATION 500
#define VORTEX_ANIM_DURATION 750

namespace Kyra {
namespace {
void naive_memcpy(byte *dest, const byte *src, int len) {
	while(len--)
		*dest++ = *src++;
}

struct eob2ChineseLZInStream {
	Common::SeekableReadStream *srcStream;
	int numBits;
	uint16 bits;

	int getBit() {
		if (numBits == 0) {
			numBits = 16;
			bits = getShort();
		}
		numBits--;
		int val = bits & 1;
		bits >>= 1;
		return val;
	}

	byte getByte() {
		return srcStream->readByte();
	}

	uint16 getShort() {
		return srcStream->readUint16LE();
	}

	eob2ChineseLZInStream(Common::SeekableReadStream *src) : srcStream(src), numBits(0), bits(0) {}
};
}

Screen_EoB::Screen_EoB(EoBCoreEngine *vm, OSystem *system) : Screen(vm, system, vm->gameFlags().lang == Common::Language::ZH_TWN ? _screenDimTableZH : _screenDimTableIntl, _screenDimTableCount), _cursorColorKey16Bit(0x8000) {
	_dsBackgroundFading = false;
	_dsShapeFadingLevel = 0;
	_dsBackgroundFadingXOffs = 0;
	_dsShapeFadingTable = 0;
	_dsX1 = _dsX2 = _dsY1 = _dsY2 = 0;
	_gfxX = _gfxY = 0;
	_gfxCol = 0;
	_dsTempPage = 0;
	_shpBuffer = _convertHiColorBuffer = 0;
	_dsDiv = 0;
	_dsRem = 0;
	_dsScaleTrans = 0;
	_cgaScaleTable = 0;
	_gfxMaxY = 0;
	_egaDitheringTable = 0;
	_egaDitheringTempPage = 0;
	_cgaMappingDefault = 0;
	_cgaDitheringTables[0] = _cgaDitheringTables[1] = 0;
	_useHiResEGADithering = false;
	_cyclePalette = 0;
	_cpsFilePattern = "%s.";
	_activePalCycle = 0;
	_segaRenderer = 0;
	_segaAnimator = 0;
	_segaCustomPalettes = 0;
	_palFaders = 0;
	_defaultRenderBuffer = 0;
	_defaultRenderBufferSize = 0;
	_specialColorReplace = false;
	memset(_segaCurPalette, 0, sizeof(_segaCurPalette));
}

Screen_EoB::~Screen_EoB() {
	delete[] _dsTempPage;
	delete[] _shpBuffer;
	delete[] _convertHiColorBuffer;
	delete[] _cgaScaleTable;
	delete[] _egaDitheringTable;
	delete[] _egaDitheringTempPage;
	delete[] _cgaDitheringTables[0];
	delete[] _cgaDitheringTables[1];
	delete[] _cyclePalette;
	delete[] _segaCustomPalettes;
	delete[] _palFaders;
	delete[] _defaultRenderBuffer;
	delete _segaRenderer;
	delete _segaAnimator;
}

bool Screen_EoB::init() {
	if (Screen::init()) {
		int temp;
		_gfxMaxY = _vm->staticres()->loadRawData(kEoBBaseExpObjectY, temp);
		_dsTempPage = new uint8[12000];

		if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
			_shpBuffer = new uint8[SCREEN_H * SCREEN_W];
			_convertHiColorBuffer = new uint8[SCREEN_H * SCREEN_W];
			enableHiColorMode(true);
			setFontStyles(FID_SJIS_FNT, Font::kStyleFat);
			_fonts[FID_SJIS_LARGE_FNT] = new SJISFontLarge(_sjisFontShared);
		} else if (_vm->gameFlags().platform == Common::kPlatformPC98) {
			if (_vm->game() == GI_EOB1) {
				_fonts[FID_SJIS_FNT] = new SJISFontEoB1PC98(_sjisFontShared, /*12,*/ _vm->staticres()->loadRawDataBe16(kEoB1Ascii2SjisTable1, temp), _vm->staticres()->loadRawDataBe16(kEoB1Ascii2SjisTable2, temp));
			} else {
				const char *const *tbl = _vm->staticres()->loadStrings(kEoB2Ascii2SjisTables2, temp);
				assert(temp > 1);
				_fonts[FID_SJIS_FNT] = new SJISFontEoB2PC98(_sjisFontShared, /*12,*/ tbl[0], tbl[1]);
			}
		}

		if (_vm->gameFlags().useHiRes && _renderMode == Common::kRenderEGA) {
			_useHiResEGADithering = true;
			_egaDitheringTable = new uint8[256];
			_egaDitheringTempPage = new uint8[SCREEN_W * 2 * SCREEN_H * 2];
			for (int i = 0; i < 256; i++)
				_egaDitheringTable[i] = i & 0x0F;
		} else if (_renderMode == Common::kRenderCGA) {
			_cgaMappingDefault = _vm->staticres()->loadRawData(kEoB1CgaMappingDefault, temp);
			_cgaDitheringTables[0] = new uint16[256]();
			_cgaDitheringTables[1] = new uint16[256]();

			_cgaScaleTable = new uint8[256]();
			for (int i = 0; i < 256; i++)
				_cgaScaleTable[i] = ((i & 0xF0) >> 2) | (i & 0x03);
		} else if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
			sega_initGraphics();
			_segaCustomPalettes = new uint16[128]();
			_palFaders = new PaletteFader[4];
			_defaultRenderBufferSize = SCREEN_W * _screenHeight;
			_defaultRenderBuffer = new uint8[_defaultRenderBufferSize]();
			sega_setTextBuffer(0, 0);
		}

		_useShapeShading = (_bytesPerPixel != 2 && !_isAmiga && !_isSegaCD && !_use16ColorMode && _renderMode != Common::kRenderCGA && _renderMode != Common::kRenderEGA) || _useHiResEGADithering;

		static const char *cpsExt[] = { "CPS", "EGA", "SHP", "BIN" };
		int ci = 0;
		if (_vm->game() == GI_EOB1) {
			if (_vm->gameFlags().platform == Common::kPlatformPC98) {
				_cyclePalette = new uint8[48]();
				ci = 3;
			} else if (_renderMode == Common::kRenderEGA || _renderMode == Common::kRenderCGA) {
				ci = 1;
			}
		} else if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
			ci = 2;
		}
		_cpsFilePattern += cpsExt[ci];

		if (_vm->game() == GI_EOB2 && _vm->gameFlags().lang == Common::Language::ZH_TWN) {
			Common::File f;
			if (!f.open("ceob.pat"))
				return false;
			_big5.reset(new Graphics::Big5Font());
			_big5->loadPrefixedRaw(f, 14);
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
	static const uint8 amigaColorMap[16] = { 0x00, 0x06, 0x1d, 0x1b, 0x1a, 0x17, 0x18, 0x0e, 0x19, 0x1c, 0x1c, 0x1e, 0x13, 0x0a, 0x11, 0x1f };
	if (_curDim)
		fillRect(_curDim->sx << 3, _curDim->sy, ((_curDim->sx + _curDim->w) << 3) - 1, (_curDim->sy + _curDim->h) - 1, _isAmiga ? amigaColorMap[_curDim->col2] : _use16ColorMode ? 0 : _curDim->col2);
}

void Screen_EoB::clearCurDimOvl(int pageNum) {
	if (pageNum > 1 || !_useOverlays || !_curDim)
		return;
	addDirtyRect(_curDim->sx << 3, _curDim->sy, _curDim->w << 3, _curDim->h);
	clearOverlayRect(pageNum, _curDim->sx << 3, _curDim->sy, _curDim->w << 3, _curDim->h);
}

void Screen_EoB::setMouseCursor(int x, int y, const byte *shape) {
	setMouseCursor(x, y, shape, 0);
}

void Screen_EoB::setMouseCursor(int x, int y, const byte *shape, const uint8 *ovl) {
	if (!shape)
		return;

	int mouseW = (shape[2] << 3);
	int mouseH = (shape[3]);
	int colorKey = (_renderMode == Common::kRenderCGA) ? 0 : (_bytesPerPixel == 2 ? _cursorColorKey16Bit : _cursorColorKey);

	int scaleFactor = _vm->gameFlags().useHiRes ? 2 : 1;
	int bpp = _useHiColorScreen ? 2 : 1;

	uint8 *cursor = new uint8[mouseW * scaleFactor * bpp * mouseH * scaleFactor];

	if (_bytesPerPixel == 2) {
		for (int s = mouseW * scaleFactor * bpp * mouseH * scaleFactor; s; s -= 2)
			*(uint16*)(cursor + s - 2) = colorKey;
	} else {
		// We don't use fillRect here to make sure that the color key 0xFF doesn't get converted into EGA color
		memset(cursor, colorKey, mouseW * scaleFactor * bpp * mouseH * scaleFactor);
	}

	copyBlockToPage(6, 0, 0, mouseW * scaleFactor, mouseH * scaleFactor, cursor);
	drawShape(6, shape, 0, 0, 0, 2, ovl);
	CursorMan.showMouse(false);

	if (_useHiResEGADithering)
		ditherRect(getCPagePtr(6), cursor, mouseW * scaleFactor, mouseW, mouseH, colorKey);
	else if (_useHiColorScreen)
		scale2x<uint16, uint32>(cursor, mouseW * scaleFactor, getCPagePtr(6), SCREEN_W, mouseW, mouseH);
	else if (_vm->gameFlags().useHiRes)
		scale2x<uint8, uint16>(cursor, mouseW * scaleFactor, getCPagePtr(6), SCREEN_W, mouseW, mouseH);
	else
		copyRegionToBuffer(6, 0, 0, mouseW, mouseH, cursor);

	// Mouse cursor post processing for EOB II Amiga
	if (_dualPaletteModeSplitY) {
		int len = mouseW * mouseH;
		while (--len > -1)
			cursor[len] |= 0x20;
	}

	// Mouse cursor post processing for CGA mode. Unlike the original (which uses drawShape for the mouse cursor)
	// the cursor manager cannot know whether a pixel value of 0 is supposed to be black or transparent. Thus, we
	// go over the transparency mask again and turn the black pixels to color 4.
	if (_renderMode == Common::kRenderCGA) {
		const uint8 *maskTbl = shape + 4 + ((mouseW * mouseH) >> 2);
		uint8 *dst = cursor;
		uint8 trans = 0;
		uint8 shift = 6;

		uint16 mH = mouseH;
		while (mH--) {
			uint16 mW = mouseW;
			while (mW--) {
				if (shift == 6)
					trans = *maskTbl++;
				if (!*dst && !((trans >> shift) & 3))
					*dst = 4;
				dst++;
				shift = (shift - 2) & 7;
			}
		}
	}

	// Convert color key to 16 bit after drawing the mouse cursor.
	// The cursor has been converted to 16 bit in scale2x().
	colorKey = _16bitConversionPalette ? _16bitConversionPalette[colorKey] : colorKey;
	Graphics::PixelFormat pixelFormat = _system->getScreenFormat();

	CursorMan.replaceCursor(cursor, mouseW * scaleFactor, mouseH * scaleFactor, x * scaleFactor, y * scaleFactor, colorKey, false, &pixelFormat);
	if (isMouseVisible())
		CursorMan.showMouse(true);
	delete[] cursor;

	// makes sure that the cursor is drawn
	// we do not use Screen::updateScreen here
	// so we can be sure that changes to page 0
	// are NOT updated on the real screen here
	updateBackendScreen(true);
}

void Screen_EoB::loadFileDataToPage(Common::SeekableReadStream *s, int pageNum, uint32 size) {
	s->read(_pagePtrs[pageNum], size);
}

void Screen_EoB::printShadedText(const char *string, int x, int y, int col1, int col2, int shadowCol, int pitch) {
	if (_isSegaCD && shadowCol) {
		printText(string, x + 1, y + 1, shadowCol, 0, pitch);
	} else if (!_isSegaCD && !_use16ColorMode && !_useHiColorScreen && (_fonts[_currentFont]->getType() == Font::kASCII || _fonts[_currentFont]->getType() == Font::kBIG5)) {
		printText(string, x - 1, y, shadowCol, col2);
		printText(string, x, y + 1, shadowCol, 0);
		printText(string, x - 1, y + 1, shadowCol, 0);
	} else if (!_isSegaCD && col2) {
		fillRect(x, y, x + getTextWidth(string) - 1, y + getFontHeight() - 1, col2);
	}

	if (_vm->gameFlags().use16ColorMode)
		setFontStyles(_currentFont, Font::kStyleLeftShadow);

	printText(string, x, y, col1, 0, pitch);

	if (_vm->gameFlags().use16ColorMode)
		setFontStyles(_currentFont, Font::kStyleNone);
}

void Screen_EoB::loadShapeSetBitmap(const char *file, int tempPage, int destPage) {
	loadEoBBitmap(file, _cgaMappingDefault, tempPage, destPage, -1);
	_curPage = 2;
}

void Screen_EoB::eob2ChineseLZUncompress(byte *dest, uint32 destSize, Common::SeekableReadStream *src) {
	src->skip(6);
	eob2ChineseLZInStream in(src);
	int lzOffset, lzLen;
	byte *destPtr = dest;
	byte *destEnd = dest + destSize;

	while (1) {
		if (in.getBit()) {
			assert(destPtr < destEnd);
			*destPtr++ = in.getByte();
			continue;
		}

		if (!in.getBit()) {
			lzLen = in.getBit() << 1;
			lzLen |= in.getBit();
			lzOffset = 0x100 - in.getByte();
		} else {
			uint16 lzPair = in.getShort();
			lzOffset = 0x2000 - (lzPair >> 3);
			lzLen = lzPair & 7;
			if (lzLen == 0) {
				lzLen = in.getByte();
				if (lzLen == 0) {
					return;
				}
			}
		}
		lzLen += 2;
		assert(destPtr < destEnd);
		assert(destPtr + lzLen < destEnd);
		assert(destPtr - lzOffset >= dest);
		naive_memcpy(destPtr, destPtr - lzOffset, lzLen);
		destPtr += lzLen;
	}
}

void Screen_EoB::loadChineseEOB2LZBitmap(Common::SeekableReadStream *s, int pageNum, uint32 size) {
	eob2ChineseLZUncompress(_pagePtrs[pageNum], size, s);
}

void Screen_EoB::loadBitmap(const char *filename, int tempPage, int dstPage, Palette *pal, bool skip) {
	if (_vm->game() == GI_EOB2 && _vm->gameFlags().lang == Common::Language::ZH_TWN && scumm_stricmp(filename, "menu.cps") == 0) {
		uint32 palSize;
		uint8 *palData = _vm->resource()->fileData("menu.col", &palSize);
		if (!palData) {
			warning("couldn't load bitmap: '%s'", filename);
			return;
		}
		loadPalette(palData, *pal, palSize);
		delete[] palData;

		Common::ScopedPtr<Common::SeekableReadStream> srcStream(_vm->resource()->createReadStream(filename));

		if (!srcStream) {
			warning("couldn't load bitmap: '%s'", filename);
			return;
		}

		uint8 *dstData = getPagePtr(dstPage);
		memset(dstData, 0, _screenPageSize);
		if (dstPage == 0 || tempPage == 0)
			_forceFullUpdate = true;

		eob2ChineseLZUncompress(dstData, _screenPageSize, srcStream.get());
		return;
	}

	if (!scumm_stricmp(filename + strlen(filename) - 3, "BIN")) {
		Common::SeekableReadStream *str = _vm->resource()->createReadStream(filename);
		if (!str)
			error("Screen_EoB::loadBitmap(): Failed to load file '%s'", filename);
		str->skip(2);
		uint16 imgSize = str->readUint16LE();
		assert(imgSize == str->size() - 4);
		uint8 *buf = new uint8[MAX<uint16>(imgSize, SCREEN_W * SCREEN_H)];
		str->read(buf, imgSize);
		delete str;

		decodeBIN(buf, _pagePtrs[dstPage], imgSize);
		if (!skip)
			decodePC98PlanarBitmap(_pagePtrs[dstPage], buf, SCREEN_W * SCREEN_H);

		delete[] buf;
	} else {
		Screen::loadBitmap(filename, tempPage, dstPage, pal);
	}

	if (_isAmiga && !skip) {
		Common::SeekableReadStream *str = _vm->resource()->createReadStream(filename);
		str->skip(4);
		uint32 imgSize = str->readUint32LE();

		if (_vm->game() == GI_EOB1 && (dstPage == 3 || dstPage == 4) && imgSize == 40064) {
			// Yay, this is where EOB1 Amiga hides the palette data
			loadPalette(_pagePtrs[dstPage] + 40000, *_palettes[0], 64);
			_palettes[0]->fill(0, 1, 0);
		} else if (_vm->game() == GI_EOB2) {
			uint16 palSize = str->readUint16LE();
			// EOB II Amiga CPS files may contain more than one palette (each one 64 bytes,
			// one after the other). We load them all...
			if (pal && palSize) {
				for (int i = 1; i <= palSize >> 6; ++i)
					_palettes[i]->loadAmigaPalette(*str, 0, 32);
			}
		}

		Screen::convertAmigaGfx(getPagePtr(dstPage), 320, 200);
		delete str;
	}
}

void Screen_EoB::loadEoBBitmap(const char *file, const uint8 *cgaMapping, int tempPage, int destPage, int convertToPage) {
	Common::String tmp = Common::String::format(_cpsFilePattern.c_str(), file);
	Common::SeekableReadStream *s = _vm->resource()->createReadStream(Common::Path(tmp));
	bool loadAlternative = false;

	if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		if (!s)
			error("Screen_EoB::loadEoBBitmap(): Failed to load file '%s'", file);
		s->read(_shpBuffer, s->size());
		decodeSHP(_shpBuffer, destPage);

	} else if (s) {
		// This additional check is necessary since some localized versions of EOB II seem to contain invalid (size zero) cps files
		if (s->size() == 0) {
			loadAlternative = true;

		// This check is due to EOB II Amiga German. That version simply checks
		// for certain file names which aren't actual CPS files. These files use
		// a different format and compression type. I check the header size
		// info to identify these.
		} else if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
			// Tolerance for size mismatches up to 2 bytes is needed in some cases
			if ((((s->readUint16LE()) + 5) & ~3) != (((s->size()) + 3) & ~3))
				loadAlternative = true;
		}

		if (!loadAlternative)
			loadBitmap(tmp.c_str(), tempPage, destPage, _vm->gameFlags().platform == Common::kPlatformAmiga ? _palettes[0] : 0);

	} else {
		loadAlternative = true;
	}

	delete s;

	if (loadAlternative) {
		if (_vm->game() == GI_EOB1) {
			tmp.insertChar('1', tmp.size() - 4);
			loadBitmap(tmp.c_str(), tempPage, destPage, 0);

		} else if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
			loadSpecialAmigaCPS(tmp.c_str(), destPage, true);

		} else {
			tmp.setChar('X', 0);
			s = _vm->resource()->createReadStream(Common::Path(tmp));

			if (!s)
				error("Screen_EoB::loadEoBBitmap(): Failed to load file '%s'", file);

			s->seek(768);
			loadFileDataToPage(s, destPage, 64000);
			delete s;
		}
	}

	if (convertToPage == -1)
		return;

	if (_16bitPalette)
		convertToHiColor(destPage);

	if (convertToPage == 2 && _renderMode == Common::kRenderCGA) {
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
		if (cgaMapping)
			generateCGADitheringTables(cgaMapping);

		uint16 *d = (uint16 *)dst;
		uint8 tblSwitch = 0;
		for (int height = SCREEN_H; height; height--) {
			const uint16 *table = _cgaDitheringTables[(tblSwitch++) & 1];
			for (int width = SCREEN_W / 2; width; width--) {
				WRITE_LE_UINT16(d++, table[((src[1] & 0x0F) << 4) | (src[0] & 0x0F)]);
				src += 2;
			}
		}
	} else if (_renderMode == Common::kRenderEGA && !_useHiResEGADithering) {
		uint32 len = SCREEN_W * SCREEN_H;
		while (len--)
			*dst++ = *src++ & 0x0F;
	} else {
		copyPage(srcPage, dstPage);
	}

	if (dstPage == 0 || dstPage == 1)
		_forceFullUpdate = true;
}

void Screen_EoB::setScreenPalette(const Palette &pal) {
	if (_bytesPerPixel == 2) {
		for (int i = 0; i < 4; i++)
			createFadeTable16bit((const uint16*)(pal.getData()), &_16bitPalette[i * 256], 0, i * 85);
	} else if (_useHiResEGADithering && pal.getNumColors() != 16) {
		generateEGADitheringTable(pal);
	} else if (_isSegaCD || (_renderMode == Common::kRenderEGA && pal.getNumColors() == 16)) {
		_paletteChanged = true;
		_screenPalette->copy(pal);
		_system->getPaletteManager()->setPalette(_screenPalette->getData(), 0, _screenPalette->getNumColors());
	} else if (_renderMode != Common::kRenderCGA && _renderMode != Common::kRenderEGA) {
		Screen::setScreenPalette(pal);
	}
}

void Screen_EoB::getRealPalette(int num, uint8 *dst) {
	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderEGA) {
		const uint8 *pal = _screenPalette->getData();
		for (int i = 0; i < 16; ++i) {
			dst[0] = (pal[0] << 2) | (pal[0] & 3);
			dst[1] = (pal[1] << 2) | (pal[1] & 3);
			dst[2] = (pal[2] << 2) | (pal[2] & 3);
			dst += 3;
			pal += 3;
		}
	} else {
		Screen::getRealPalette(num, dst);
	}
}

uint8 *Screen_EoB::encodeShape(uint16 x, uint16 y, uint16 w, uint16 h, bool encode8bit, const uint8 *cgaMapping) {
	uint8 *shp = 0;
	uint16 shapesize = 0;

	uint8 *srcLineStart = getPagePtr(_curPage | 1) + y * 320 + (x << 3);
	uint8 *src = srcLineStart;

	if (_use16ColorMode || (_renderMode == Common::kRenderEGA && !_useHiResEGADithering))
		encode8bit = false;

	if (_bytesPerPixel == 2 && encode8bit) {
		shapesize = h * (w << 3) + 4;
		shp = new uint8[shapesize]();
		uint8 *dst = shp;

		*dst++ = 0;
		*dst++ = (h & 0xFF);
		*dst++ = (w & 0xFF);
		*dst++ = (h & 0xFF);

		w <<= 3;

		for (int i = 0; i < h; ++i) {
			memcpy(dst, src, w);
			srcLineStart += SCREEN_W;
			src = srcLineStart;
			dst += w;
		}
	} else if (_renderMode == Common::kRenderCGA) {
		if (cgaMapping)
			generateCGADitheringTables(cgaMapping);
		shapesize = h * (w << 2) + 4;
		shp = new uint8[shapesize]();
		uint8 *dst = shp;

		*dst++ = 4;
		*dst++ = (h & 0xFF);
		*dst++ = (w & 0xFF);
		*dst++ = (h & 0xFF);

		uint8 *dst2 = dst + (h * (w << 1));

		uint8 tblSwitch = 0;
		uint16 h1 = h;
		while (h1--) {
			uint16 w1 = w << 1;
			const uint16 *table = _cgaDitheringTables[(tblSwitch++) & 1];

			while (w1--) {
				uint16 p0 = table[((src[1] & 0x0F) << 4) | (src[0] & 0x0F)];
				uint16 p1 = table[((src[3] & 0x0F) << 4) | (src[2] & 0x0F)];

				*dst++ = ((p0 & 0x0003) << 6) | ((p0 & 0x0300) >> 4) | ((p1 & 0x0003) << 2) | ((p1 & 0x0300) >> 8);

				uint8 msk = 0;
				for (int i = 0; i < 4; i++) {
					if (!src[3 - i])
						msk |= (3 << (i << 1));
				}
				*dst2++ = msk;
				src += 4;
			}
			srcLineStart += SCREEN_W;
			src = srcLineStart;
		}

	} else if (encode8bit) {
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

			srcLineStart += SCREEN_W;
			src = srcLineStart;
		}

		shapesize += 4;

		shp = new uint8[shapesize]();
		uint8 *dst = shp;

		*dst++ = 1;
		*dst++ = (h & 0xFF);
		*dst++ = (w & 0xFF);
		*dst++ = (h & 0xFF);

		srcLineStart = getPagePtr(_curPage | 1) + y * 320 + (x << 3);
		src = srcLineStart;

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
					val = numZero & 0xFF;
				}
				*dst++ = val;
			} while (src != lineEnd);

			srcLineStart += SCREEN_W;
			src = srcLineStart;
		}

	} else {
		uint8 nib = 0, col = 0;
		uint8 *colorMap = 0;

		if (_renderMode != Common::kRenderEGA || _useHiResEGADithering) {
			colorMap = new uint8[0x100];
			memset(colorMap, 0xFF, 0x100);
		}

		shapesize = h * (w << 2) + 20;
		shp = new uint8[shapesize]();
		uint8 *dst = shp;

		*dst++ = 2;
		*dst++ = (h & 0xFF);
		*dst++ = (w & 0xFF);
		*dst++ = (h & 0xFF);

		if (_renderMode != Common::kRenderEGA || _useHiResEGADithering) {
			memset(dst, 0xFF, 0x10);
		} else {
			for (int i = 0; i < 16; i++)
				dst[i] = i;
		}

		uint8 *pal = dst;
		dst += 16;
		nib = col = 0;

		uint16 h1 = h;
		while (h1--) {
			uint16 w1 = w << 3;
			while (w1--) {
				uint8 s = *src++;
				uint8 c = s & 0x0F;
				if (colorMap) {
					c = colorMap[s];
					if (c == 0xFF) {
						if (col < 0x10) {
							*pal++ = s;
							c = colorMap[s] = col++;
						} else {
							c = 0;
						}
					}
				}

				if (++nib & 1)
					*dst = c << 4;
				else
					*dst++ |= c;
			}
			srcLineStart += SCREEN_W;
			src = srcLineStart;
		}
		delete[] colorMap;
	}

	return shp;
}

void Screen_EoB::drawT1Shape(uint8 pageNum, const byte *t1data, int x, int y, int sd) {
	const byte *src = t1data;
	int width = READ_LE_UINT16(t1data);
	int height = READ_LE_UINT16(t1data + 2);
	src += 4;

	const ScreenDim *dm = getScreenDim(sd);
	setShapeFrame(dm->sx, dm->sy, dm->sx + dm->w, dm->sy + dm->h);
	int fx = dm->sx << 3;
	int fy = dm->sy;
	int fw = dm->w << 3;
	int fh = dm->h;

	int rX = fx + x;
	int rY = fy + y;
	int rW = (fx + fw) - rX;
	int rH = (fy + fh) - rY;
	int dX = 0, dY = 0;

	if (rX < 0) {
		dX = -rX;
		rX = 0;
	}

	if (rY < 0) {
		dY = -rY;
		rY = 0;
	}

	if (dX >= width || dY >= height)
		return;

	if (rW > width - dX)
		rW = width - dX;
	if (rH > height - rY)
		rH = height - dY;
	if (rW <= 0 || rH <= 0)
		return;

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(rX, rY, rW, rH);

	int dH = rH;
	uint8 *dstL = getPagePtr(pageNum) + rY * _bytesPerPixel * SCREEN_W;
	src += dY * width;

	while (dH--) {
		const uint8 *src2 = src + dX;
		uint8 *dst = dstL + rX * _bytesPerPixel;

		for (int i = 0; i < rW; i++, src2++, dst += _bytesPerPixel)
			if (*src2)
				drawShapeSetPixel(dst, *src2);

		dstL += SCREEN_W * _bytesPerPixel;
		src += width;
	}
}

void Screen_EoB::drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...) {
	uint8 *dst = getPagePtr(pageNum);
	const uint8 *src = shapeData;

	if (!src)
		return;

	if (sd != -1) {
		const ScreenDim *dm = getScreenDim(sd);
		setShapeFrame(dm->sx, dm->sy, dm->sx + dm->w, dm->sy + dm->h);
		x += (_dsX1 << 3);
		y += _dsY1;
	}

	uint8 *ovl = 0;

	va_list args;
	va_start(args, flags);
	if (flags & 2) {
		ovl = va_arg(args, uint8 *);
		_dsBackgroundFadingXOffs = x;
	}
	va_end(args);

	dst += (_dsX1 << (2 + _bytesPerPixel));
	int16 dX = x - (_dsX1 << 3);
	int16 dY = y;
	int16 dW = _dsX2 - _dsX1;

	uint8 pixelsPerByte = *src++;
	uint16 dH = *src++;
	uint16 width = (*src++) << 3;
	uint16 transOffset = (pixelsPerByte == 4) ? (dH * width) >> 2 : 0;
	src++;

	int rX = x;
	int rY = y;
	int rW = width + 8;
	int rH = dH;

	uint16 w2 = width;
	int d = dY - _dsY1;

	int pixelStep = (flags & 1) ? -1 : 1;

	if (pixelsPerByte < 2)  {
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

		dst += (dY * SCREEN_W * _bytesPerPixel + dX * _bytesPerPixel);
		uint8 *dstL = dst;

		if (pageNum == 0 || pageNum == 1)
			addDirtyRect(rX, rY, rW, rH);

		while (dH--) {
			int16 xpos = (int16) marginLeft;

			if (flags & 1) {
				if (pixelsPerByte == 1) {
					for (int i = 0; i < w2; i++) {
						if (*src++ == 0) {
							i += (*src - 1);
							src += (*src - 1);
						}
					}
				} else {
					src += w2;
				}
				src--;
			}
			const uint8 *src2 = src;

			if (xpos) {
				if (pixelsPerByte == 1) {
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
				} else {
					src += (xpos * pixelStep);
					xpos = 0;
				}
			}

			dst -= xpos * _bytesPerPixel;
			xpos += width;

			while (xpos > 0) {
				uint8 c = *src;
				uint8 m = (flags & 1) ? *(src - 1) : c;
				src += pixelStep;

				if (m) {
					drawShapeSetPixel(dst, c);
					dst += _bytesPerPixel;
					xpos--;
				} else if (pixelsPerByte) {
					uint8 len = (flags & 1) ? src[1] : src[0];
					dst += len * _bytesPerPixel;
					xpos -= len;
					src += pixelStep;
				} else {
					dst += _bytesPerPixel;
					xpos--;
				}
			}
			xpos += marginRight;

			if (xpos) {
				do {
					if (pixelsPerByte == 1) {
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
					} else {
						src += (xpos * pixelStep);
						xpos = 0;
					}
				} while (xpos > 0);
			}

			dstL += SCREEN_W * _bytesPerPixel;
			dst = dstL;
			if (flags & 1)
				src = src2 + 1;
		}
	} else {
		const uint8 *pal = 0;
		uint8 cgaPal[4];
		memset(cgaPal, 0, 4);

		if (pixelsPerByte == 2) {
			pal = ovl ? ovl : src;
			src += 16;
		} else {
			static const uint8 cgaDefOvl[] = { 0x00, 0x55, 0xAA, 0xFF };
			pal = ovl ? ovl : cgaDefOvl;
			for (int i = 0; i < 4; i++)
				cgaPal[i] = pal[i] & 3;
			pal = cgaPal;
		}

		if (d < 0) {
			d = -d;
			if (d >= dH)
				return;
			src += (d * (width / pixelsPerByte));
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

		bool trimL = false;
		uint8 dXbitAlign = dX & (pixelsPerByte - 1);

		if (dX < 0) {
			width += dX;
			d = -dX;
			if (flags & 1)
				src -= (d / pixelsPerByte);
			else
				src += (d / pixelsPerByte);

			if (d >= w2)
				return;

			dX = 0;
			trimL = true;
		}

		d = (dW << 3) - dX;

		if (d < 1)
			return;

		if (d < width)
			width = d;

		dst += (dY * SCREEN_W * _bytesPerPixel + dX * _bytesPerPixel);

		if (pageNum == 0 || pageNum == 1)
			addDirtyRect(rX, rY, rW, rH);

		int pitch = SCREEN_W - width;
		int16 lineSrcStep = (w2 - width) / pixelsPerByte;
		uint8 lineSrcStepRemainder = (w2 - width) % pixelsPerByte;

		w2 /= pixelsPerByte;
		if (flags & 1)
			src += (w2 - 1);

		uint8 pixelPacking = 8 / pixelsPerByte;
		uint8 pixelPackingMask = 0;

		for (int i = 0; i < pixelPacking; i++)
			pixelPackingMask |= (1 << i);

		if (trimL && (dXbitAlign > lineSrcStepRemainder))
			lineSrcStep--;

		uint8 bitShDef = 8 - pixelPacking;
		if (flags & 1) {
			lineSrcStep = (w2 << 1) - lineSrcStep;
			bitShDef = 0;
		}

		uint8 bitShLineStart = bitShDef;
		if (trimL)
			bitShLineStart -= (dXbitAlign * pixelStep * pixelPacking);

		while (dH--) {
			int16 wd = width;
			uint8 in = 0;
			uint8 trans = 0;
			uint8 shift = bitShLineStart;
			uint8 shSwtch = bitShLineStart;

			while (wd--) {
				if (shift == shSwtch) {
					in = *src;
					trans = src[transOffset];
					src += pixelStep;
					shSwtch = bitShDef;
				}
				uint8 col = (pixelsPerByte == 2) ? pal[(in >> shift) & pixelPackingMask] : (*dst & ((trans >> shift) & (pixelPackingMask))) | pal[(in >> shift) & pixelPackingMask];
				if (col || pixelsPerByte == 4)
					drawShapeSetPixel(dst, col);
				dst += _bytesPerPixel;
				shift = ((shift - (pixelStep * pixelPacking)) & 7);
			}
			src += lineSrcStep;
			dst += (pitch * _bytesPerPixel);
		}
	}
}

const uint8 *Screen_EoB::scaleShape(const uint8 *shapeData, int steps) {
	setShapeFadingLevel(steps);

	while (shapeData && steps--)
		shapeData = scaleShapeStep(shapeData);

	return shapeData;
}

const uint8 *Screen_EoB::scaleShapeStep(const uint8 *shp) {
	uint8 *dst = (shp != _dsTempPage) ? _dsTempPage : _dsTempPage + 6000;
	uint8 *d = dst;
	uint8 pixelsPerByte = *d++ = *shp++;
	assert(pixelsPerByte > 1);

	uint16 h = shp[0] + 1;
	d[0] = d[2] = (h << 1) / 3;

	uint16 w = shp[1];
	uint16 w2 = (w << 3) / pixelsPerByte;
	uint16 t = ((w << 1) % 3) ? 1 : 0;
	d[1] = ((w << 1) / 3) + t;

	uint32 transOffsetSrc = (pixelsPerByte == 4) ? (shp[0] * shp[1]) << 1 : 0;
	uint32 transOffsetDst = (pixelsPerByte == 4) ? (d[0] * d[1]) << 1 : 0;
	shp += 3;
	d += 3;

	if (pixelsPerByte == 2) {
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

		_dsScaleTrans = (i << 4) | (i & 0x0F);
		for (int ii = 0; ii < 16; ii++)
			*d++ = *shp++;
	}

	_dsDiv = w2 / 3;
	_dsRem = w2 % 3;

	while (--h) {
		if (pixelsPerByte == 2)
			scaleShapeProcessLine4Bit(d, shp);
		else
			scaleShapeProcessLine2Bit(d, shp, transOffsetDst, transOffsetSrc);
		if (!--h)
			break;
		if (pixelsPerByte == 2)
			scaleShapeProcessLine4Bit(d, shp);
		else
			scaleShapeProcessLine2Bit(d, shp, transOffsetDst, transOffsetSrc);
		if (!--h)
			break;
		shp += w2;
	}

	return (const uint8 *)dst;
}

const uint8 *Screen_EoB::generateShapeOverlay(const uint8 *shp, const uint8 *fadingTable) {
	if (*shp != 2)
		return 0;

	if (_bytesPerPixel == 2) {
		setFadeTable(fadingTable);
		setShapeFadingLevel(1);
		return 0;
	}

	shp += 4;
	for (int i = 0; i < 16; i++)
		_shapeOverlay[i] = fadingTable[shp[i]];
	return _shapeOverlay;
}

void Screen_EoB::setShapeFrame(int x1, int y1, int x2, int y2) {
	_dsX1 = x1;
	_dsY1 = y1;
	_dsX2 = x2;
	_dsY2 = y2;
}

void Screen_EoB::enableShapeBackgroundFading(bool enable) {
	_dsBackgroundFading = enable;
}

void Screen_EoB::setShapeFadingLevel(int level) {
	_dsShapeFadingLevel = level;
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

	uint32 playSpeedDelay = EXPLOSION_ANIM_DURATION << 3;
	uint32 frameDelay = (1000 << 8) / 60;
	uint32 playSpeedTimer = 0;
	uint32 frameTimer = frameDelay;
	uint32 start = _system->getMillis();

	for (int l = 2; l;) {
		if (l != 2) {
			for (int i = numElements - 1; i >= 0; i--) {
				int16 px = ((ptr2[i] >> 6) >> scale) + gx2;
				int16 py = ((ptr3[i] >> 6) >> scale) + gy2;
				if (py > ymax)
					py = ymax;
				if (posWithinRect(px, py, rX1, rY1, rX2, rY2)) {
					if (_bytesPerPixel == 2)
						setPagePixel16bit(0, px, py, ptr6[i]);
					else
						setPagePixel(0, px, py, ptr6[i]);
				}

				playSpeedTimer += playSpeedDelay;
				uint32 ct = _system->getMillis();
				if (ct >= (start + (frameTimer >> 8))) {
					updateScreen();
					uint32 diff = (_system->getMillis() - ct) << 8;
					if ((int32)diff > 0 && diff > frameDelay)
						start += ((diff - frameDelay) >> 8);
					frameTimer += frameDelay;
				}
				if (_system->getMillis() < (start + (playSpeedTimer >> 15)))
					_vm->delayUntil(start + (playSpeedTimer >> 15));
			}
		}

		l = 0;

		for (int i = 0; i < numElements; i++) {
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
				if (pxVal1 == _gfxCol && posWithinRect(px, py, rX1, rY1, rX2, rY2))
					setPagePixel(0, px, py, pxVal2);
			} else {
				ptr7[i] = 0;
			}

			playSpeedTimer += playSpeedDelay;
			uint32 ct = _system->getMillis();
			if (ct >= (start + (frameTimer >> 8))) {
				updateScreen();
				uint32 diff = (_system->getMillis() - ct) << 8;
				if ((int32)diff > 0 && diff > frameDelay)
					start += ((diff - frameDelay) >> 8);
				frameTimer += frameDelay;
			}
			if (_system->getMillis() < (start + (playSpeedTimer >> 15)))
				_vm->delayUntil(start + (playSpeedTimer >> 15));
		}
	}

	updateScreen();
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

	uint32 playSpeedDelay = (VORTEX_ANIM_DURATION << 8) / numElements;
	uint32 frameDelay = (1000 << 8) / 60;
	uint32 playSpeedTimer = 0;
	uint32 frameTimer = frameDelay;
	uint32 start = _system->getMillis();

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
				if (_bytesPerPixel == 2)
					setPagePixel16bit(0, px, py, pixBackup[ii]);
				else
					setPagePixel(0, px, py, pixBackup[ii]);

				playSpeedTimer += playSpeedDelay;
				uint32 ct = _system->getMillis();
				if (ct >= (start + (frameTimer >> 8))) {
					updateScreen();
					uint32 diff = (_system->getMillis() - ct) << 8;
					if ((int32)diff > 0 && diff > frameDelay)
						start += ((diff - frameDelay) >> 8);
					frameTimer += frameDelay;
				}
				if (_system->getMillis() < (start + (playSpeedTimer >> 16)))
					_vm->delayUntil(start + (playSpeedTimer >> 16));
			}
		}

		i = 0;
		int r = (stepSize >> 1) + (stepSize >> 2) + (stepSize >> 3);

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
				if (tc1 == _gfxCol && !pixDelay[ii])
					setPagePixel(0, px, py, tc2);
			} else {
				colTableStep[ii] = 0;
			}

			playSpeedTimer += playSpeedDelay;
			uint32 ct = _system->getMillis();
			if (ct >= (start + (frameTimer >> 8))) {
				updateScreen();
				uint32 diff = (_system->getMillis() - ct) << 8;
				if ((int32)diff > 0 && diff > frameDelay)
					start += ((diff - frameDelay) >> 8);
				frameTimer += frameDelay;
			}
			if (_system->getMillis() < (start + (playSpeedTimer >> 16)))
				_vm->delayUntil(start + (playSpeedTimer >> 16));
		}
		d++;
	}

	_curPage = cp;
	updateScreen();
	showMouse();
}

void Screen_EoB::fadeTextColor(Palette *pal, int color, int rate) {
	assert(rate);
	uint8 *col = pal->getData();

	for (bool loop = true; loop;) {
		uint32 end = _system->getMillis() + _vm->tickLength();

		loop = false;
		for (int ii = 0; ii < 3; ii++) {
			uint8 c = col[color * 3 + ii];
			if (c > rate) {
				col[color * 3 + ii] -= rate;
				loop = true;
			} else if (c) {
				col[color * 3 + ii] = 0;
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
	int numBytes = (fadePal->getNumColors() - 1) * 3;

	for (int i = 0; i < numBytes; i++) {
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

void Screen_EoB::setFadeTable(const uint8 *table) {
	_dsShapeFadingTable = table;
	if (_bytesPerPixel == 2)
		memcpy(&_16bitPalette[0x100], table, 512);
}

void Screen_EoB::createFadeTable(const uint8 *palData, uint8 *dst, uint8 rootColor, uint8 weight) {
	if (!palData)
		return;

	const uint8 *src = palData + 3 * rootColor;
	uint8 r = *src++;
	uint8 g = *src++;
	uint8 b = *src;
	uint8 tr, tg, tb;
	src = palData + 3;

	*dst++ = 0;
	weight >>= 1;

	for (uint8 i = 1; i; i++) {
		uint16 tmp = (uint16)((*src - r) * weight) << 1;
		tr = *src++ - ((tmp >> 8) & 0xFF);
		tmp = (uint16)((*src - g) * weight) << 1;
		tg = *src++ - ((tmp >> 8) & 0xFF);
		tmp = (uint16)((*src - b) * weight) << 1;
		tb = *src++ - ((tmp >> 8) & 0xFF);

		const uint8 *d = palData + 3;
		uint16 v = 0xFFFF;
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
				col = ii;
			}
		}
		*dst++ = col;
	}
}

void Screen_EoB::createFadeTable16bit(const uint16 *palData, uint16 *dst, uint16 rootColor, uint8 weight) {
	rootColor = palData[rootColor];
	uint8 r8 = (rootColor & 0x1f);
	uint8 g8 = (rootColor & 0x3E0) >> 5;
	uint8 b8 = (rootColor & 0x7C00) >> 10;

	int root_r = r8 << 4;
	int root_g = g8 << 4;
	int root_b = b8 << 4;

	*dst++ = palData[0];

	for (uint8 i = 1; i; i++) {
		r8 = (palData[i] & 0x1f);
		g8 = (palData[i] & 0x3E0) >> 5;
		b8 = (palData[i] & 0x7C00) >> 10;

		int red = r8 << 4;
		int green = g8 << 4;
		int blue = b8 << 4;

		if (red > root_r) {
			red -= weight;
			if (root_r > red)
				red = root_r;
		} else {
			red += weight;
			if (root_r < red)
				red = root_r;
		}

		if (green > root_g) {
			green -= weight;
			if (root_g > green)
				green = root_g;
		} else {
			green += weight;
			if (root_g < green)
				green = root_g;
		}

		if (blue > root_b) {
			blue -= weight;
			if (root_b > blue)
				blue = root_b;
		} else {
			blue += weight;
			if (root_b < blue)
				blue = root_b;
		}

		r8 = red >> 4;
		g8 = green >> 4;
		b8 = blue >> 4;

		*dst++ = (b8 << 10) | (g8 << 5) | r8;
	}
}

const uint16 *Screen_EoB::getCGADitheringTable(int index) {
	return !(index & ~1) ? _cgaDitheringTables[index] : 0;
}

const uint8 *Screen_EoB::getEGADitheringTable() {
	return _egaDitheringTable;
}

bool Screen_EoB::loadFont(FontId fontId, const char *filename) {
	Font *&fnt = _fonts[fontId];
	int temp = 0;
	if (fnt) {
		delete fnt;
		fnt = nullptr;
	}

	if (_vm->gameFlags().platform == Common::kPlatformPC98 && _vm->game() == GI_EOB2) {
		if (fontId == FID_SJIS_SMALL_FNT) {
			const char *const *tbl = _vm->staticres()->loadStrings(kEoB2Ascii2SjisTables2, temp);
			assert(temp > 1);
			fnt = new PC98Font(12, true, 2, _vm->staticres()->loadRawData(kEoB2FontConvertTbl, temp), tbl[0], tbl[1]);
		} else {
			fnt = new PC98Font(12, false, 1);
		}
	} else if (fontId == FID_SJIS_SMALL_FNT) {
		if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
			fnt = new SJISFont12x12(_vm->staticres()->loadRawDataBe16(kEoB2FontLookupTbl, temp));
		} else if (_vm->gameFlags().platform == Common::kPlatformPC98) {
			fnt = new Font12x12PC98(12, _vm->staticres()->loadRawDataBe16(kEoB1Ascii2SjisTable1, temp),
				_vm->staticres()->loadRawDataBe16(kEoB1Ascii2SjisTable2, temp), _vm->staticres()->loadRawData(kEoB1FontLookupTable, temp));
		}
	} else if (_isAmiga) {
		fnt = new AmigaDOSFont(_vm->resource(), _vm->game() == GI_EOB2 && _vm->gameFlags().lang == Common::DE_DEU);
	} else if (_isSegaCD) {
		fnt = new SegaCDFont(_vm->gameFlags().lang, _vm->staticres()->loadRawDataBe16(kEoB1Ascii2SjisTable1, temp), _vm->staticres()->loadRawDataBe16(kEoB1Ascii2SjisTable2, temp),
			_vm->staticres()->loadRawData(kEoB1CharWidthTable1, temp), _vm->staticres()->loadRawData(kEoB1CharWidthTable2, temp), _vm->staticres()->loadRawData(kEoB1CharWidthTable3, temp));
	} else if (fontId == FID_CHINESE_FNT && _vm->game() == GI_EOB2 && _vm->gameFlags().lang == Common::ZH_TWN) {
		// We wrap all fonts in Big5 support but FID_CHINESE additionally attempts to match height
		OldDOSFont *ofnt = new OldDOSFont(_useHiResEGADithering ? Common::kRenderVGA : _renderMode, 12, false);
		ofnt->loadPCBIOSTall();
		fnt = new ChineseTwoByteFontEoB(_big5, ofnt);
		fnt->setColorMap(_textColorsMap);
		return true;
	} else {
		// We use normal VGA rendering in EOB II, since we do the complete EGA dithering in updateScreen().
		fnt = new OldDOSFont(_useHiResEGADithering ? Common::kRenderVGA : _renderMode, 12);
	}

	assert(fnt);

	Common::SeekableReadStream *file = _vm->resource()->createReadStream(filename);
	if (!file)
		error("Font file '%s' is missing", filename);

	bool ret = false;
	if (fnt) {
		ret = fnt->load(*file);
		if (_big5)
			fnt = new ChineseTwoByteFontEoB(_big5, fnt);
		fnt->setColorMap(_textColorsMap);
	}

	delete file;
	return ret;
}

void Screen_EoB::updateDirtyRects() {
	if (!_useHiResEGADithering) {
		Screen::updateDirtyRects();
		return;
	}

	if (_forceFullUpdate) {
		ditherRect(getCPagePtr(0), _egaDitheringTempPage, SCREEN_W * 2, SCREEN_W, SCREEN_H);
		_system->copyRectToScreen(_egaDitheringTempPage, SCREEN_W * 2, 0, 0, SCREEN_W * 2, SCREEN_H * 2);
	} else {
		const uint8 *page0 = getCPagePtr(0);
		Common::List<Common::Rect>::iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			ditherRect(page0 + it->top * SCREEN_W + it->left, _egaDitheringTempPage, SCREEN_W * 2, it->width(), it->height());
			_system->copyRectToScreen(_egaDitheringTempPage, SCREEN_W * 2, it->left * 2, it->top * 2, it->width() * 2, it->height() * 2);
		}
	}

	_forceFullUpdate = false;
	_dirtyRects.clear();
}

void Screen_EoB::ditherRect(const uint8 *src, uint8 *dst, int dstPitch, int srcW, int srcH, int colorKey) {
	while (srcH--) {
		uint8 *dst2 = dst + dstPitch;
		for (int i = 0; i < srcW; i++) {
			int in = *src++;
			if (in != colorKey) {
				in = _egaDitheringTable[in];
				*dst++ = *dst2++ = in >> 4;
				*dst++ = *dst2++ = in & 0x0F;
			} else {
				dst[0] = dst[1] = dst2[0] = dst2[1] = colorKey;
				dst += 2;
				dst2 += 2;
			}
		}
		src += (SCREEN_W - srcW);
		dst += ((dstPitch - srcW) * 2);
	}
}

void Screen_EoB::drawShapeSetPixel(uint8 *dst, uint8 col) {
	if (_bytesPerPixel == 2) {
		*(uint16*)dst = _16bitPalette[(_dsShapeFadingLevel << 8) + col];
		return;
	} else if (_useShapeShading) {
		if (_dsBackgroundFading) {
			if (_dsShapeFadingLevel) {
				col = *dst;
			} else {
				_dsBackgroundFadingXOffs &= 7;
				col = *(dst + _dsBackgroundFadingXOffs++);
			}
		}

		if (_dsShapeFadingLevel) {
			assert(_dsShapeFadingTable);
			uint8 cnt = _dsShapeFadingLevel;
			while (cnt--)
				col = _dsShapeFadingTable[col];
		}
	}

	*dst = col;
}

void Screen_EoB::scaleShapeProcessLine2Bit(uint8 *&shpDst, const uint8 *&shpSrc, uint32 transOffsetDst, uint32 transOffsetSrc) {
	for (int i = 0; i < _dsDiv; i++) {
		shpDst[0] = (_cgaScaleTable[shpSrc[0]] << 2) | (shpSrc[1] >> 6);
		shpDst[1] = ((shpSrc[1] & 0x0F) << 4) | ((shpSrc[2] >> 2) & 0x0F);
		shpDst[transOffsetDst] = (_cgaScaleTable[shpSrc[transOffsetSrc]] << 2) | (shpSrc[transOffsetSrc + 1] >> 6);
		shpDst[transOffsetDst + 1] = ((shpSrc[transOffsetSrc + 1] & 0x0F) << 4) | ((shpSrc[transOffsetSrc + 2] >> 2) & 0x0F);
		shpSrc += 3;
		shpDst += 2;
	}

	if (_dsRem == 1) {
		shpDst[0] = _cgaScaleTable[shpSrc[0]] << 2;
		shpDst[1] = 0;
		shpDst[transOffsetDst] = (_cgaScaleTable[shpSrc[transOffsetSrc]] << 2) | 3;
		shpDst[transOffsetDst + 1] = 0xFF;
		shpSrc++;
		shpDst += 2;

	} else if (_dsRem == 2) {
		shpDst[0] = (_cgaScaleTable[shpSrc[0]] << 2) | (shpSrc[1] >> 6);
		shpDst[1] = (shpSrc[1] & 0x3F) << 2;
		shpDst[transOffsetDst] = (_cgaScaleTable[shpSrc[transOffsetSrc]] << 2) | (shpSrc[transOffsetSrc + 1] >> 6);
		shpDst[transOffsetDst + 1] = ((shpSrc[transOffsetSrc + 1] & 0x3F) << 2) | 3;
		shpSrc += 2;
		shpDst += 2;
	}
}

void Screen_EoB::scaleShapeProcessLine4Bit(uint8 *&dst, const uint8 *&src) {
	for (int i = 0; i < _dsDiv; i++) {
		*dst++ = *src++;
		*dst++ = (READ_BE_UINT16(src) >> 4) & 0xFF;
		src += 2;
	}

	if (_dsRem == 1) {
		*dst++ = *src++;
		*dst++ = _dsScaleTrans;
	} else if (_dsRem == 2) {
		*dst++ = (src[0] & 0xF0) | (src[1] >> 4);
		src += 2;
		*dst++ = _dsScaleTrans;
		*dst++ = _dsScaleTrans;
		*dst++ = _dsScaleTrans;
	}
}

bool Screen_EoB::posWithinRect(int posX, int posY, int x1, int y1, int x2, int y2) {
	if (posX < x1 || posX > x2 || posY < y1 || posY > y2)
		return false;
	return true;
}

void Screen_EoB::setPagePixel16bit(int pageNum, int x, int y, uint16 color) {
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H);
	assert(_bytesPerPixel == 2);

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, 1, 1);

	((uint16*)_pagePtrs[pageNum])[y * SCREEN_W + x] = color;
}

void Screen_EoB::generateEGADitheringTable(const Palette &pal) {
	assert(_egaDitheringTable);
	const uint8 *src = pal.getData();
	uint8 *dst = _egaDitheringTable;

	for (int i = 256; i; i--) {
		int r = *src++;
		int g = *src++;
		int b = *src++;

		uint8 col = 0;
		uint16 min = 0x2E83;

		for (int ii = 256; ii; ii--) {
			const uint8 *palEntry = _egaMatchTable + (ii - 1) * 3;
			if (*palEntry == 0xFF)
				continue;

			int e_r = palEntry[0] - r;
			int e_g = palEntry[1] - g;
			int e_b = palEntry[2] - b;

			uint16 s = (e_r * e_r) + (e_g * e_g) + (e_b * e_b);

			if (s <= min) {
				min = s;
				col = ii - 1;
			}
		}
		*dst++ = col;
	}
}

void Screen_EoB::generateCGADitheringTables(const uint8 *mappingData) {
	for (int i = 0; i < 256; i++) {
		_cgaDitheringTables[0][i] = (mappingData[(i >> 4) + 16] << 8) | mappingData[i & 0x0F];
		_cgaDitheringTables[1][i] = (mappingData[i >> 4] << 8) | mappingData[(i & 0x0F) + 16];
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

#undef EXPLOSION_ANIM_DURATION
#undef VORTEX_ANIM_DURATION

uint16 *OldDOSFont::_cgaDitheringTable = 0;
int OldDOSFont::_numRef = 0;

OldDOSFont::OldDOSFont(Common::RenderMode mode, uint8 shadowColor, bool remapCharacters) : _renderMode(mode), _shadowColor(shadowColor), _remapCharacters(remapCharacters), _numGlyphsMax(128), _useOverlay(false), _scaleV(1), _colorMap8bit(0), _colorMap16bit(0) {
	_data = 0;
	_width = _height = _numGlyphs = 0;
	_bitmapOffsets = 0;
	_style = kStyleNone;

	_numRef++;
	if (!_cgaDitheringTable && _numRef == 1) {
		_cgaDitheringTable = new uint16[256]();
		static const uint bits[] = { 0, 3, 12, 15 };
		for (int i = 0; i < 256; i++)
			_cgaDitheringTable[i] = (bits[i & 3] << 8) | (bits[(i >> 2) & 3] << 12) | (bits[(i >> 4) & 3] << 0) | (bits[(i >> 6) & 3] << 4);
	}
}

OldDOSFont::~OldDOSFont() {
	unload();

	if (_numRef)
		--_numRef;

	if (_cgaDitheringTable && !_numRef) {
		delete[] _cgaDitheringTable;
		_cgaDitheringTable = 0;
	}
}

bool OldDOSFont::loadPCBIOSTall() {
	unload();

	_numGlyphs = 128;
	_width = 8;
	const int originalBytesPerGlyph = 8;
	const int originalHeight = 8;
	const int bytesPerGlyph = 15;
	_height = originalHeight * 2 - 1;
	_data = new uint8[_numGlyphs * bytesPerGlyph + _numGlyphs * sizeof(uint16)];
	assert(_data);

	_bitmapOffsets = (uint16 *)_data;

	for (int i = 0; i < _numGlyphs; ++i) {
		_bitmapOffsets[i] = _numGlyphs * sizeof(uint16) + i * bytesPerGlyph;
		byte *optr = _data + _bitmapOffsets[i];
		const byte *iptr = Graphics::DosFont::fontData_PCBIOS + i * originalBytesPerGlyph;
		*optr++ = *iptr++;
		for (int j = 1; j < originalHeight; j++) {
			*optr++ = *iptr;
			*optr++ = *iptr++;
		}
	}

	return true;
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

	_width = _data[_numGlyphsMax * 2 + 3];
	_height = _data[_numGlyphsMax * 2 + 2];
	_numGlyphs = (READ_LE_UINT16(_data + 2) / 2) - 2;

	_bitmapOffsets = (uint16*)(_data + 2);

	for (int i = 0; i < _numGlyphs; ++i)
		_bitmapOffsets[i] = READ_LE_UINT16(&_bitmapOffsets[i]);

	return true;
}

int OldDOSFont::getCharWidth(uint16 c) const {
	// Since these fonts have a fixed character width we always give a return value
	// even if there is no glyph for the specified character (which can't normally
	// happen anyway - you'd have to do something like importing a Japanese save file
	// into the English version).
	return _width;
}

void OldDOSFont::setColorMap(const uint8 *src) {
	_colorMap8bit = src;
}

void OldDOSFont::drawChar(uint16 c, byte *dst, int pitch, int bpp) const {
	uint16 color1 = _colorMap8bit[1];
	uint16 color2 = _colorMap8bit[0];

	if (_style == kStyleLeftShadow) {
		byte *dst2 = dst;
		for (int i = 0; i < _scaleV; ++i) {
			drawCharIntern(c, dst2 + pitch * _scaleV, pitch * _scaleV, 1, _shadowColor, 0);
			drawCharIntern(c, dst2 - 1, pitch * _scaleV, 1, _shadowColor, 0);
			drawCharIntern(c, dst2 - 1 + pitch * _scaleV, pitch * _scaleV, 1, _shadowColor, 0);
			dst2 += pitch;
		}
	}

	if (bpp == 2) {
		color1 = _colorMap16bit[1];
		color2 = _colorMap16bit[0];
	}

	for (int i = 0; i < _scaleV; ++i) {
		drawCharIntern(c, dst, pitch * _scaleV, bpp, color1, color2);
		dst += pitch;
	}
}

void OldDOSFont::drawCharIntern(uint16 c, byte *dst, int pitch, int bpp, int col1, int col2) const {
	static const uint16 renderMaskTable[] = {
		0x0000, 0x8000, 0xc000, 0xe000, 0xf000, 0xf800, 0xfc00, 0xfe00, 0xff00, 0xff80, 0xffc0, 0xffe0, 0xfff0, 0xfff8, 0xfffc, 0xfffe, 0xffff
	};

	if (_remapCharacters)
		c = convert(c);

	if (c >= _numGlyphs)
		return;

	pitch *= bpp;
	const uint8 *src = &_data[_bitmapOffsets[c]];

	int w = (_width - 1) >> 3;
	pitch -= _width * bpp;

	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderEGA) {
		col1 &= 0x0F;
		col2 &= 0x0F;
	}

	static const uint16 cgaColorMask[] = { 0, 0x5555, 0xAAAA, 0xFFFF };
	uint16 cgaMask1 = cgaColorMask[col1 & 3];
	uint16 cgaMask2 = cgaColorMask[col2 & 3];

	int cH = _height;
	while (cH--) {
		int cW = w;
		uint16 mask = renderMaskTable[_width];

		if (_renderMode == Common::kRenderCGA) {
			uint16 s = (*src++) << 8;
			if (_width > 8)
				s |= *src++;

			uint16 cmp1 = 0;
			uint16 cmp2 = 0;

			if (col1) {
				s &= mask;
				cmp1 = _cgaDitheringTable[s >> 8];
			}

			if (col2) {
				s = ~s & mask;
				cmp2 = _cgaDitheringTable[s >> 8];
			}

			uint16 cDst = 0;
			uint8 sh = 6;
			for (int i = 0; i < _width; i++) {
				cDst |= ((dst[i] & 3) << sh);
				sh = (sh - 2) & 0x0F;
			}

			uint16 out = (~(cmp1 | cmp2) & cDst) | (cmp1 & cgaMask1) | (cmp2 & cgaMask2);

			sh = 6;
			for (int i = 0; i < _width; i++) {
				*dst++ = (out >> sh) & 3;
				sh = (sh - 2) & 0x0F;
			}
		} else {
			for (bool runWidthLoop = true; runWidthLoop;) {
				uint16 s = (*src++) << 8;
				if (_width > 8)
					s |= *src++;

				for (uint16 i = 0x8000; i; i >>= 1) {
					if (!(mask & i)) {
						runWidthLoop = false;
						break;
					}

					if (s & i) {
						if (bpp == 2)
							*(uint16*)dst = col1;
						else if (col1)
							*dst = col1;
					} else {
						if (bpp == 2) {
							if (col2 != 0xFFFF)
								*(uint16*)dst = col2;
						} else if (col2) {
							*dst = col2;
						}
					}
					dst += bpp;
				}

				mask >>= 1;

				if (cW)
					cW--;
				else
					runWidthLoop = false;
			}
		}

		dst += pitch;
	}
}

uint16 OldDOSFont::convert(uint16 c) const {
	if (_width == 6) {
		switch (c) {
		case 0x81:
		case 0x9A:
			c = 0x5D;
			break;
		case 0x84:
		case 0x8E:
			c = 0x5B;
			break;
		case 0x94:
		case 0x99:
			c = 0x40;
		case 0xE1:
			// TODO: recheck this: no conversion for 'ß' ?
			break;
		default:
			break;
		}
	} else if (_width == 8) {
		switch (c) {
		case 0x81:
		case 0x9A:
		case 0x5D:
			c = 0x1D;
			break;
		case 0x84:
		case 0x5B:
			c = 0x1E;
			break;
		case 0x94:
		case 0x40:
			c = 0x1F;
			break;
		case 0x8E:
			c = 0x1B;
			break;
		case 0x99:
			c = 0x1C;
			break;
		case 0xE1:
			c = 0x19;
			break;
		default:
			break;
		}
	}
	return c;
}

void OldDOSFont::unload() {
	delete[] _data;
	_data = 0;
	_width = _height = _numGlyphs = 0;
	_bitmapOffsets = 0;
}

uint16 ChineseTwoByteFontEoB::translateBig5(uint16 in) const {
	// The original explicitly turns the braces '<' and '>' into '"'.
	if (in == '<' || in == '>')
		return '\"';
	else if (in < 0x80)
		return in;
	in = ((in & 0xff00) >> 8) | ((in & 0xff) << 8);
	if (_big5->hasGlyphForBig5Char(in))
		return in;
	return '?';
}

int ChineseTwoByteFontEoB::getCharWidth(uint16 c) const {
	uint16 t = translateBig5(c);
	return (t < 0x80) ? _singleByte->getCharWidth(t) : _big5->kChineseTraditionalWidth;
}

int ChineseTwoByteFontEoB::getCharHeight(uint16 c) const {
	uint16 t = translateBig5(c);
	return (t < 0x80) ? _singleByte->getCharHeight(t) : _big5->getFontHeight();
}

void ChineseTwoByteFontEoB::drawChar(uint16 c, byte *dst, int pitch, int bpp) const {
	uint16 t = translateBig5(c);
	if (t < 0x80)
		_singleByte->drawChar(t, dst, pitch, bpp);
	else
		_big5->drawBig5Char(dst, t,
				    _big5->kChineseTraditionalWidth, _big5->getFontHeight(), pitch, _colorMap[1], _colorMap[0], _border, bpp);
}

} // End of namespace Kyra

#endif // ENABLE_EOB
