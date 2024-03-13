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

#include "kyra/graphics/screen_lok.h"
#include "kyra/engine/kyra_lok.h"

#include "common/system.h"

#include "graphics/paletteman.h"

namespace Kyra {

Screen_LoK::Screen_LoK(KyraEngine_LoK *vm, OSystem *system)
	: Screen(vm, system, _screenDimTable, _screenDimTableCount) {
	_vm = vm;
	_unkPtr1 = _unkPtr2 = nullptr;
	_bitBlitNum = 0;
	memset(_saveLoadPage, 0, sizeof(_saveLoadPage));
	memset(_saveLoadPageOvl, 0, sizeof(_saveLoadPageOvl));
}

Screen_LoK::~Screen_LoK() {
	for (int i = 0; i < ARRAYSIZE(_saveLoadPage); ++i) {
		delete[] _saveLoadPage[i];
		_saveLoadPage[i] = nullptr;
	}

	for (int i = 0; i < ARRAYSIZE(_saveLoadPageOvl); ++i) {
		delete[] _saveLoadPageOvl[i];
		_saveLoadPageOvl[i] = nullptr;
	}

	delete[] _unkPtr1;
	delete[] _unkPtr2;
}

bool Screen_LoK::init() {
	if (!Screen::init())
		return false;

	for (uint i = 0; i < ARRAYSIZE(_bitBlitRects); i++) {
		_bitBlitRects[i].left = 0;
		_bitBlitRects[i].top = 0;
		_bitBlitRects[i].right = 0;
		_bitBlitRects[i].bottom = 0;
	}
	_bitBlitNum = 0;
	memset(_saveLoadPage, 0, sizeof(_saveLoadPage));
	memset(_saveLoadPageOvl, 0, sizeof(_saveLoadPageOvl));

	_unkPtr1 = new uint8[getRectSize(1, 144)]();
	assert(_unkPtr1);
	_unkPtr2 = new uint8[getRectSize(1, 144)]();
	assert(_unkPtr2);

	return true;
}

void Screen_LoK::loadBitmap(const char *filename, int tempPage, int dstPage, Palette *pal, bool skip) {
	const char *ext = filename + strlen(filename) - 3;

	Screen::loadBitmap(filename, tempPage, dstPage, pal, skip);

	if (_isAmiga) {
		if (!scumm_stricmp(ext, "MSC"))
			Screen::convertAmigaMsc(getPagePtr(dstPage));
		else
			Screen::convertAmigaGfx(getPagePtr(dstPage), 320, 200);
	}
}

void Screen_LoK::fadeSpecialPalette(int palIndex, int startIndex, int size, int fadeTime) {
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		return;

	assert(_vm->palTable1()[palIndex]);

	Palette tempPal(getPalette(0).getNumColors());
	tempPal.copy(getPalette(0));
	tempPal.copy(_vm->palTable1()[palIndex], 0, size, startIndex);

	fadePalette(tempPal, fadeTime * 18);

	getPalette(0).copy(tempPal, startIndex, size);
	setScreenPalette(getPalette(0));
	updateBackendScreen(true);
}

void Screen_LoK::addBitBlitRect(int x, int y, int w, int h) {
	if (_bitBlitNum >= kNumBitBlitRects)
		error("too many bit blit rects");

	_bitBlitRects[_bitBlitNum].left = x;
	_bitBlitRects[_bitBlitNum].top = y;
	_bitBlitRects[_bitBlitNum].right = x + w;
	_bitBlitRects[_bitBlitNum].bottom = y + h;
	++_bitBlitNum;
}

void Screen_LoK::bitBlitRects() {
	Common::Rect *cur = _bitBlitRects;
	while (_bitBlitNum) {
		_bitBlitNum--;
		copyRegion(cur->left, cur->top, cur->left, cur->top, cur->width(), cur->height(), 2, 0);
		++cur;
	}
}

void Screen_LoK::savePageToDisk(const char *file, int page) {
	if (!_saveLoadPage[page / 2]) {
		_saveLoadPage[page / 2] = new uint8[SCREEN_W * SCREEN_H];
		assert(_saveLoadPage[page / 2]);
	}
	memcpy(_saveLoadPage[page / 2], getPagePtr(page), SCREEN_W * SCREEN_H);

	if (_useOverlays) {
		if (!_saveLoadPageOvl[page / 2]) {
			_saveLoadPageOvl[page / 2] = new uint8[SCREEN_OVL_SJIS_SIZE];
			assert(_saveLoadPageOvl[page / 2]);
		}

		uint8 *srcPage = getOverlayPtr(page);
		if (!srcPage) {
			warning("trying to save unsupported overlay page %d", page);
			return;
		}

		memcpy(_saveLoadPageOvl[page / 2], srcPage, SCREEN_OVL_SJIS_SIZE);
	}
}

void Screen_LoK::loadPageFromDisk(const char *file, int page) {
	if (!_saveLoadPage[page / 2]) {
		warning("trying to restore page %d, but no backup found", page);
		return;
	}

	copyBlockToPage(page, 0, 0, SCREEN_W, SCREEN_H, _saveLoadPage[page / 2]);
	delete[] _saveLoadPage[page / 2];
	_saveLoadPage[page / 2] = nullptr;

	if (_saveLoadPageOvl[page / 2]) {
		uint8 *dstPage = getOverlayPtr(page);
		if (!dstPage) {
			warning("trying to restore unsupported overlay page %d", page);
			return;
		}

		memcpy(dstPage, _saveLoadPageOvl[page / 2], SCREEN_OVL_SJIS_SIZE);
		delete[] _saveLoadPageOvl[page / 2];
		_saveLoadPageOvl[page / 2] = nullptr;
	}
}

void Screen_LoK::queryPageFromDisk(const char *file, int page, uint8 *buffer) {
	if (!_saveLoadPage[page / 2]) {
		warning("trying to query page %d, but no backup found", page);
		return;
	}

	memcpy(buffer, _saveLoadPage[page / 2], SCREEN_W * SCREEN_H);
}

void Screen_LoK::deletePageFromDisk(int page) {
	delete[] _saveLoadPage[page / 2];
	_saveLoadPage[page / 2] = nullptr;

	if (_saveLoadPageOvl[page / 2]) {
		delete[] _saveLoadPageOvl[page / 2];
		_saveLoadPageOvl[page / 2] = nullptr;
	}
}

void Screen_LoK::copyBackgroundBlock(int x, int page, int flag) {
	if (x < 1)
		return;

	int height = 128;
	if (flag)
		height += 8;
	if (!(x & 1))
		++x;
	if (x == 19)
		x = 17;

	uint8 *ptr1 = _unkPtr1;
	uint8 *ptr2 = _unkPtr2;
	int oldVideoPage = _curPage;
	_curPage = page;

	int curX = x;
	copyRegionToBuffer(_curPage, 8, 8, 8, height, ptr2);
	for (int i = 0; i < 19; ++i) {
		int tempX = curX + 1;
		copyRegionToBuffer(_curPage, tempX << 3, 8, 8, height, ptr1);
		copyBlockToPage(_curPage, tempX << 3, 8, 8, height, ptr2);
		int newXPos = curX + x;
		if (newXPos > 37)
			newXPos = newXPos % 38;

		tempX = newXPos + 1;
		copyRegionToBuffer(_curPage, tempX << 3, 8, 8, height, ptr2);
		copyBlockToPage(_curPage, tempX << 3, 8, 8, height, ptr1);
		curX += x * 2;
		if (curX > 37) {
			curX = curX % 38;
		}
	}
	_curPage = oldVideoPage;
}

void Screen_LoK::copyBackgroundBlock2(int x) {
	copyBackgroundBlock(x, 4, 1);
}

void Screen_LoK::setTextColorMap(const uint8 *cmap) {
	setTextColor(cmap, 0, 11);
}

int Screen_LoK::getRectSize(int x, int y) {
	if (x < 1)
		x = 1;
	else if (x > 40)
		x = 40;

	if (y < 1)
		y = 1;
	else if (y > 200)
		y = 200;

	return ((x * y) << 3);
}

void Screen_LoK::setInterfacePalette(const Palette &pal, uint8 r, uint8 g, uint8 b) {
	if (!_isAmiga)
		return;

	uint8 screenPal[32 * 3];

	assert(32 <= pal.getNumColors());

	for (int i = 0; i < pal.getNumColors(); ++i) {
		if (i != 0x10) {
			screenPal[3 * i + 0] = (pal[i * 3 + 0] * 0xFF) / 0x3F;
			screenPal[3 * i + 1] = (pal[i * 3 + 1] * 0xFF) / 0x3F;
			screenPal[3 * i + 2] = (pal[i * 3 + 2] * 0xFF) / 0x3F;
		} else {
			screenPal[3 * i + 0] = (r * 0xFF) / 0x3F;
			screenPal[3 * i + 1] = (g * 0xFF) / 0x3F;
			screenPal[3 * i + 2] = (b * 0xFF) / 0x3F;
		}
	}

	_paletteChanged = true;
	_system->getPaletteManager()->setPalette(screenPal, 32, pal.getNumColors());
}

void Screen_LoK::postProcessCursor(uint8 *data, int width, int height, int pitch) {
	if (_vm->gameFlags().platform == Common::kPlatformAmiga && _dualPaletteModeSplitY) {
		pitch -= width;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				if (*data != _cursorColorKey)
					*data += 32;
				++data;
			}

			data += pitch;
		}
	}
}

#pragma mark -

Screen_LoK_16::Screen_LoK_16(KyraEngine_LoK *vm, OSystem *system) : Screen_LoK(vm, system) {
	memset(_paletteDither, 0, sizeof(_paletteDither));
}

void Screen_LoK_16::setScreenPalette(const Palette &pal) {
	_screenPalette->copy(pal);

	for (int i = 0; i < 256; ++i)
		paletteMap(i, pal[i * 3 + 0] << 2, pal[i * 3 + 1] << 2, pal[i * 3 + 2] << 2);

	set16ColorPalette(_palette16);
	_forceFullUpdate = true;
}

void Screen_LoK_16::fadePalette(const Palette &pal, int delay, const UpdateFunctor *upFunc) {
	uint8 notBlackFlag = 0;
	for (int i = 0; i < 768; ++i) {
		if ((*_screenPalette)[i])
			notBlackFlag |= 1;
		if (pal[i])
			notBlackFlag |= 2;
	}

	if (notBlackFlag == 1 || notBlackFlag == 2) {
		bool upFade = false;

		for (int i = 0; i < 768; ++i) {
			if ((*_screenPalette)[i] < pal[i]) {
				upFade = true;
				break;
			}
		}

		if (upFade) {
			for (int i = 0; i < 256; ++i)
				paletteMap(i, pal[i * 3 + 0] << 2, pal[i * 3 + 1] << 2, pal[i * 3 + 2] << 2);
			_forceFullUpdate = true;
		}

		uint8 color16Palette[16 * 3];

		if (upFade)
			memset(color16Palette, 0, sizeof(color16Palette));
		else
			memcpy(color16Palette, _palette16, sizeof(color16Palette));

		set16ColorPalette(color16Palette);
		updateScreen();

		for (int i = 0; i < 16; ++i) {
			set16ColorPalette(color16Palette);

			for (int k = 0; k < 48; ++k) {
				if (upFade) {
					if (color16Palette[k] < _palette16[k])
						++color16Palette[k];
				} else {
					if (color16Palette[k] > 0)
						--color16Palette[k];
				}
			}

			if (upFunc && upFunc->isValid())
				(*upFunc)();
			else
				updateBackendScreen(true);

			_vm->delay((delay >> 5) * _vm->tickLength());
		}
	}

	setScreenPalette(pal);
}

void Screen_LoK_16::getFadeParams(const Palette &pal, int delay, int &delayInc, int &diff) {
	error("Screen_LoK_16::getFadeParams called");
}

int Screen_LoK_16::fadePalStep(const Palette &pal, int diff) {
	error("Screen_LoK_16::fadePalStep called");
	return 0;   // for compilers that don't support NORETURN
}

void Screen_LoK_16::paletteMap(uint8 idx, int r, int g, int b) {
	const int red = r;
	const int green = g;
	const int blue = b;

	uint16 rgbDiff = 1000;
	int rDiff = 0, gDiff = 0, bDiff = 0;

	int index1 = -1;

	for (int i = 0; i < 16; ++i) {
		const int realR = _palette16[i * 3 + 0] << 4;
		const int realG = _palette16[i * 3 + 1] << 4;
		const int realB = _palette16[i * 3 + 2] << 4;

		uint16 diff = ABS(r - realR) + ABS(g - realG) + ABS(b - realB);

		if (diff < rgbDiff) {
			rgbDiff = diff;
			index1 = i;

			rDiff = r - realR;
			gDiff = g - realG;
			bDiff = b - realB;
		}
	}

	r = rDiff / 4 + red;
	g = gDiff / 4 + green;
	b = bDiff / 4 + blue;

	rgbDiff = 1000;
	int index2 = -1;

	for (int i = 0; i < 16; ++i) {
		const int realR = _palette16[i * 3 + 0] << 4;
		const int realG = _palette16[i * 3 + 1] << 4;
		const int realB = _palette16[i * 3 + 2] << 4;

		uint16 diff = ABS(r - realR) + ABS(g - realG) + ABS(b - realB);

		if (diff < rgbDiff) {
			rgbDiff = diff;
			index2 = i;
		}
	}

	_paletteDither[idx].bestMatch = index1;
	_paletteDither[idx].invertMatch = index2;
}

void Screen_LoK_16::convertTo16Colors(uint8 *page, int w, int h, int pitch, int keyColor) {
	const int rowAdd = pitch * 2 - w;

	uint8 *row1 = page;
	uint8 *row2 = page + pitch;

	for (int i = 0; i < h; i += 2) {
		for (int k = 0; k < w; k += 2) {
			if (keyColor == -1 || keyColor != *row1) {
				const PaletteDither &dither = _paletteDither[*row1];

				*row1++ = dither.bestMatch;
				*row1++ = dither.invertMatch;
				*row2++ = dither.invertMatch;
				*row2++ = dither.bestMatch;
			} else {
				row1 += 2;
				row2 += 2;
			}
		}

		row1 += rowAdd;
		row2 += rowAdd;
	}
}

void Screen_LoK_16::mergeOverlay(int x, int y, int w, int h) {
	byte *dst = _sjisOverlayPtrs[0] + y * 640 + x;

	// We do a game screen rect to 16 color dithering here. It is
	// important that we do not dither the overlay, since else the
	// japanese fonts will look wrong.
	convertTo16Colors(dst, w, h, 640);

	const byte *src = _sjisOverlayPtrs[1] + y * 640 + x;

	int add = 640 - w;

	while (h--) {
		for (x = 0; x < w; ++x, ++dst) {
			byte col = *src++;
			if (col != _sjisInvisibleColor)
				*dst = _paletteDither[col].bestMatch;
		}
		dst += add;
		src += add;
	}
}

void Screen_LoK_16::set16ColorPalette(const uint8 *pal) {
	uint8 palette[16 * 3];
	for (int i = 0; i < 16; ++i) {
		palette[i * 3 + 0] = (pal[i * 3 + 0] * 0xFF) / 0x0F;
		palette[i * 3 + 1] = (pal[i * 3 + 1] * 0xFF) / 0x0F;
		palette[i * 3 + 2] = (pal[i * 3 + 2] * 0xFF) / 0x0F;
	}

	_system->getPaletteManager()->setPalette(palette, 0, 16);
}

ChineseOneByteFontLoK::ChineseOneByteFontLoK(int pitch) : ChineseFont(pitch, 8, 14, 9, 17, 0, 0) {
	_border = _pixelColorShading = false;
}

void ChineseOneByteFontLoK::processColorMap() {
	_textColor[0] = _colorMap[1];
	_textColor[1] = _colorMap[0];
}

ChineseTwoByteFontLoK::ChineseTwoByteFontLoK(int pitch, const uint16 *lookupTable, uint32 lookupTableSize) : ChineseFont(pitch, 15, 14, 18, 17, 0, 3),
_lookupTable(lookupTable), _lookupTableSize(lookupTableSize) {
	assert(lookupTable);
}

bool ChineseTwoByteFontLoK::hasGlyphForCharacter(uint16 c) const {
	for (uint32 i = 0; i < _lookupTableSize; ++i) {
		if (_lookupTable[i] == c)
			return true;
	}
	return false;
}

uint32 ChineseTwoByteFontLoK::getFontOffset(uint16 c) const {
	for (uint32 i = 0; i < _lookupTableSize; ++i) {
		if (_lookupTable[i] == c)
			return i * 28;
	}
	return 0;
}

void ChineseTwoByteFontLoK::processColorMap() {
	_border = (_colorMap[0] == 12);
	uint8 cs = _colorMap[1];

	if (_colorMap[1] == 9)
		cs = 83;
	else if (_colorMap[1] == 5)
		cs = 207;
	else if (_colorMap[1] == 2)
		cs = 74;
	else if (_colorMap[1] == 15)
		cs = 161;
	else if (_colorMap[1] > 15 && _colorMap[1] < 248)
		cs += 1;

	_textColor[0] = _colorMap[1] | (cs << 8);
	_textColor[0] = TO_LE_16(_textColor[0]);
	_textColor[1] = _colorMap[0] | (_colorMap[0] << 8);
}

JohabFontLoK::JohabFontLoK(Font *&font8fat, const uint16 *lookupTable, uint32 lookupTableSize) : _font8fat(font8fat), _height(15), _width(15), _fileData(0), _colorMap(0), _glyphTemp(0) {
	assert(lookupTable);
	assert(lookupTableSize == 224);
	for (int i = 0; i < 7; ++i)
		_2byteTables[i] = &lookupTable[i << 5];
	memset(_glyphData, 0, sizeof(_glyphData));
	_glyphTemp = new uint8[30];
}

JohabFontLoK::~JohabFontLoK() {
	delete[] _fileData;
	delete[] _glyphTemp;
}

bool JohabFontLoK::load(Common::SeekableReadStream &data) {
	if (_fileData)
		return false;

	if (!data.size())
		return false;

	uint32 fileSize = data.size();

	if (fileSize != (kNumJongseong + kNumJungseong + kNumChoseong) * 30) {
		warning("HangulFontLoK::load(): Invalid font file size '%d' (expected: '%d').", fileSize, (kNumJongseong + kNumJungseong + kNumChoseong) * 30);
		return false;
	}

	uint8 *dst = new uint8[fileSize];
	if (!dst)
		return false;

	data.read(dst, fileSize);
	_fileData = dst;

	_glyphData[0] = _fileData;
	_glyphData[1] = _glyphData[0] + (kNumJongseong * 30);
	_glyphData[2] = _glyphData[1] + (kNumJungseong * 30);

	return true;
}

int JohabFontLoK::getCharWidth(uint16 c) const {
	assert(_font8fat);
	return (c >= 0x80) ? _width + 1 : _font8fat->getCharWidth(c);
}

int JohabFontLoK::getCharHeight(uint16 c) const {
	return _colorMap[3] ? _height + 2 : _height;
}

void JohabFontLoK::setColorMap(const uint8 *src) {
	_colorMap = src;
	assert(_font8fat);
	_font8fat->setColorMap(src);
}

void JohabFontLoK::drawChar(uint16 c, byte *dst, int pitch, int) const {
	if (c < 0x80) {
		assert(_font8fat);
		_font8fat->drawChar(c, dst + (c == '\"' ? 0 : 5) * pitch, pitch, 0);
		return;
	}

	const uint8 *glyph = createGlyph(c);
	dst += (pitch + 1);

	if (_colorMap[3]) {
		renderGlyph(dst - 1, glyph, _colorMap[3], pitch);
		renderGlyph(dst + 1, glyph, _colorMap[3], pitch);
		renderGlyph(dst - pitch, glyph, _colorMap[3], pitch);
		renderGlyph(dst + pitch, glyph, _colorMap[3], pitch);
	}

	renderGlyph(dst, glyph, _colorMap[1], pitch);
}

const uint8 *JohabFontLoK::createGlyph(uint16 chr) const {
	memset(_glyphTemp, 0, 30);

	uint16 t[3];
	memset(t, 0, sizeof(t));

	chr = (chr << 8) | (chr >> 8);
	uint8 i1 = chr & 0x1f;
	uint8 i2 = (chr >> 5) & 0x1f;
	uint8 i3 = (chr >> 10) & 0x1f;

	// determine jungseong glyph part
	uint16 r1 = _2byteTables[1][i2];
	if ((int16)r1 > 0)
		r1 += (_2byteTables[3][i3] + _2byteTables[6][i1] - 3);

	// determine jongseong glyph part
	uint16 r2 = _2byteTables[0][i3];
	if ((int16)r2 > 0)
		r2 += (_2byteTables[4][i2] + _2byteTables[6][i1]);

	// determine choseong glyph part
	uint16 r3 = _2byteTables[2][i1];
	if ((int16)r3 > 0)
		r3 += (_2byteTables[5][i2] - 3);

	t[0] = r2 >> 5;
	t[1] = (r1 >> 5) - 2;
	t[2] = (r3 >> 5) - 2;

	const uint8 lim[3] = { kNumJongseong, kNumJungseong, kNumChoseong };

	for (int l = 0; l < 3; ++l) {
		if (t[l] <= lim[l]) {
			const uint8 *src = &_glyphData[l][t[l] * 30];
			for (int i = 0; i < 30; ++i)
				_glyphTemp[i] |= *src++;
		}
	}

	return _glyphTemp;
}

void JohabFontLoK::renderGlyph(byte *dst, const uint8 *glyph, uint8 col, int pitch) const {
	const uint8 *src = glyph;
	pitch -= 15;

	for (int y = 0; y < _height; ++y) {
		uint8 m = 0;
		uint8 in = 0;
		for (int x = 0; x < _width; ++x) {
			if (m == 0) {
				in = *src++;
				m = 0x80;
			}
			if (in & m)
				*dst = col;
			dst++;
			m >>= 1;
		}
		dst += pitch;
	}
}

} // End of namespace Kyra
