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

#include "kyra/graphics/screen.h"
#include "kyra/kyra_v1.h"
#include "kyra/resource/resource.h"

#include "common/endian.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/config-manager.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "graphics/sjis.h"


#define KYRA_SCREEN_IDLE_REFRESH

namespace Kyra {

Screen::Screen(KyraEngine_v1 *vm, OSystem *system, const ScreenDim *dimTable, const int dimTableSize)
	: _system(system), _vm(vm), _sjisInvisibleColor(0), _dimTable(dimTable), _dimTableCount(dimTableSize),
	_cursorColorKey((vm->game() == GI_KYRA1 || vm->game() == GI_EOB1 || vm->game() == GI_EOB2) ? 0xFF : 0),
	_screenHeight(vm->gameFlags().platform == Common::kPlatformSegaCD ? SCREEN_H_SEGA_NTSC : SCREEN_H) {
	_debugEnabled = false;
	_forceFullUpdate = false;
	_maskMinY = _maskMaxY = -1;

	_mouseLockCount = 0;

	_drawShapeVar1 = 0;
	_drawShapeVar3 = 1;
	_drawShapeVar4 = 0;
	_drawShapeVar5 = 0;

	_dsShapeFadingTable = _dsColorTable = _dsTransparencyTable1 = _dsTransparencyTable2 = _dsBackgroundFadingTable = nullptr;
	_dsDstPage = nullptr;
	_dsShapeFadingLevel = _dsDrawLayer = _dsTmpWidth = _dsOffscreenLeft = _dsOffscreenRight = _dsScaleW = _dsScaleH = _dsOffscreenScaleVal1 = _dsOffscreenScaleVal2 = 0;

	memset(_shapePages, 0, sizeof(_shapePages));
	memset(_fonts, 0, sizeof(_fonts));

	_pagePtrsBuff = nullptr;
	memset(_pagePtrs, 0, sizeof(_pagePtrs));
	memset(_pageMapping, 0, sizeof(_pageMapping));
	memset(_sjisOverlayPtrs, 0, sizeof(_sjisOverlayPtrs));

	_renderMode = Common::kRenderDefault;
	_sjisMixedFontMode = false;

	_screenPalette = _internFadePalette = nullptr;
	_animBlockPtr = _textRenderBuffer = nullptr;
	_textRenderBufferSize = 0;

	_useHiColorScreen = _vm->gameFlags().useHiColorMode;
	_useShapeShading = true;
	_screenPageSize = SCREEN_PAGE_SIZE;
	_16bitPalette = nullptr;
	_16bitConversionPalette = nullptr;
	_16bitShadingLevel = 0;
	_bytesPerPixel = 1;
	_4bitPixelPacking = _useAmigaExtraColors = _isAmiga = _isSegaCD = _use16ColorMode = false;
	_useSJIS = _useOverlays = _useHiResEGADithering = false;

	_currentFont = FID_8_FNT;
	_fontStyles = 0;
	_paletteChanged = true;
	_textMarginRight = SCREEN_W;
	_overdrawMargin = false;
	_customDimTable = nullptr;
	_curDim = nullptr;
	_curDimIndex = 0;
	_animBlockSize = 0;

	_lineBreakChars = (_vm->gameFlags().platform == Common::kPlatformMacintosh) ? "\n\r" : "\r";
	_yTransOffs = 0;
	_dualPaletteModeSplitY = 0;

	_idleUpdateTimer = 0;
}

Screen::~Screen() {
	for (int i = 0; i < SCREEN_OVLS_NUM; ++i)
		delete[] _sjisOverlayPtrs[i];

	delete[] _pagePtrsBuff;

	for (int f = 0; f < ARRAYSIZE(_fonts); ++f)
		delete _fonts[f];

	delete _screenPalette;
	delete _internFadePalette;
	delete[] _animBlockPtr;
	delete[] _16bitPalette;
	delete[] _16bitConversionPalette;

	_sjisFontShared.reset();

	for (uint i = 0; i < _palettes.size(); ++i)
		delete _palettes[i];

	if (_customDimTable) {
		for (int i = 0; i < _dimTableCount; ++i)
			delete _customDimTable[i];
		delete[] _customDimTable;
	}
}

bool Screen::init() {
	_debugEnabled = false;
	_useOverlays = false;
	_useSJIS = false;
	_use16ColorMode = _vm->gameFlags().use16ColorMode;
	_4bitPixelPacking = (_use16ColorMode && _vm->game() == GI_LOL);
	_isAmiga = (_vm->gameFlags().platform == Common::kPlatformAmiga);
	_isSegaCD = (_vm->gameFlags().platform == Common::kPlatformSegaCD);
	// Amiga copper palette magic requires the use of more than 32 colors for some purposes.
	_useAmigaExtraColors = (_isAmiga && _vm->game() == GI_EOB2);

	// We only check the "render_mode" setting for both Eye of the Beholder
	// games here, since all the other games do not support the render_mode
	// setting or handle it differently, like Kyra 1 PC-98. This avoids
	// graphics glitches and crashes in other games, when the user sets his
	// global render_mode setting to EGA for example.
	// TODO/FIXME: It would be nice not to hardcode this. But there is no
	// trivial/non annoying way to do mode checks in an easy fashion right
	// now.
	// In a more general sense, we might want to think about a way to only
	// pass valid config values, as in values which the engine can work with,
	// to the engines. We already limit the selection via our GUIO flags in
	// the game specific settings, but this is not enough due to global
	// settings allowing everything.
	if (_vm->game() == GI_EOB1 || _vm->game() == GI_EOB2) {
		if (ConfMan.hasKey("render_mode"))
			_renderMode = Common::parseRenderMode(ConfMan.get("render_mode"));
	}

	// In VGA mode the odd and even page pointers point to the same buffers.
	for (int i = 0; i < SCREEN_PAGE_NUM; i++)
		_pageMapping[i] = i & ~1;
	// CGA and EGA modes use additional pages to do the CGA/EGA specific graphics conversions.
	if (_vm->game() == GI_EOB1 && (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderEGA)) {
		for (int i = 0; i < 8; i++)
			_pageMapping[i] = i;
	}

	memset(_shapePages, 0, sizeof(_shapePages));
	memset(_fonts, 0, sizeof(_fonts));

	_useOverlays = (_vm->gameFlags().useHiRes && _renderMode != Common::kRenderEGA);

	if (_useOverlays) {
		_useSJIS = (_vm->gameFlags().lang == Common::JA_JPN);
		_sjisInvisibleColor = (_vm->game() == GI_KYRA1) ? 0x80 : 0xF6;
		_sjisMixedFontMode = !_use16ColorMode;

		if (!_sjisOverlayPtrs[0]) {
			// We alway assume 2 bytes per pixel here when the backend is in hicolor mode, since this is the surface that is passed to the backend.
			// We do this regardsless of the paramater sent to enableHiColorMode() so as not to have to change the backend color mode.
			// Conversions from 8bit to 16bit have to take place when copying data to this surface here.
			int bpp = _useHiColorScreen ? 2 : 1;
			_sjisOverlayPtrs[0] = new uint8[SCREEN_OVL_SJIS_SIZE * bpp];
			assert(_sjisOverlayPtrs[0]);
			memset(_sjisOverlayPtrs[0], _sjisInvisibleColor, SCREEN_OVL_SJIS_SIZE * bpp);
		}

		for (int i = 1; i < SCREEN_OVLS_NUM; ++i) {
			if (!_sjisOverlayPtrs[i]) {
				_sjisOverlayPtrs[i] = new uint8[SCREEN_OVL_SJIS_SIZE];
				assert(_sjisOverlayPtrs[i]);
				memset(_sjisOverlayPtrs[i], _sjisInvisibleColor, SCREEN_OVL_SJIS_SIZE);
			}
		}

		if (_useSJIS) {
			_sjisFontShared = Common::SharedPtr<Graphics::FontSJIS>(Graphics::FontSJIS::createFont(_vm->gameFlags().platform));
			if (!_sjisFontShared.get())
				error("Could not load any SJIS font, neither the original nor ScummVM's 'SJIS.FNT'");

			if (_use16ColorMode)
				_fonts[FID_SJIS_TEXTMODE_FNT] = new SJISFont(_sjisFontShared, _sjisInvisibleColor, true, false, 0);
			else if (_vm->gameFlags().platform != Common::kPlatformPC98 || _vm->game() != GI_EOB2)
				_fonts[FID_SJIS_FNT] = new SJISFont(_sjisFontShared, _sjisInvisibleColor, false, _vm->game() != GI_LOL && _vm->game() != GI_EOB2, _vm->game() == GI_LOL ? 1 : 0);
		}
	}

	_curPage = 0;

	enableHiColorMode(false);

	const int paletteCount = _isAmiga ? 13 : 4;
	// We allow 256 color palettes in EGA mode, since original EOB II code does the same and requires it
	const int numColors = _use16ColorMode ? 16 : (_isAmiga ? 32 : (_renderMode == Common::kRenderCGA ? 4 : 256));
	const int numColorsInternal = _useAmigaExtraColors ? 64 : numColors;

	_dualPaletteModeSplitY = 0;

	_screenPalette = new Palette(numColorsInternal);
	assert(_screenPalette);

	_palettes.resize(paletteCount);
	_palettes[0] = new Palette(numColorsInternal);
	assert(_palettes[0]);

	for (int i = 1; i < paletteCount; ++i) {
		_palettes[i] = new Palette(numColors);
		assert(_palettes[i]);
	}

	// Setup CGA colors (if CGA mode is selected)
	if (_renderMode == Common::kRenderCGA) {
		Palette pal(5);
		pal.setCGAPalette(1, Palette::kIntensityHigh);
		// create additional black color 4 for use with the mouse cursor manager
		pal.fill(4, 1, 0);
		Screen::setScreenPalette(pal);
	}

	_internFadePalette = new Palette(numColorsInternal);
	assert(_internFadePalette);

	setScreenPalette(getPalette(0));

	// We setup the PC98 text mode palette at [16, 24], since that will be used
	// for KANJI characters in Lands of Lore.
	if (_use16ColorMode && _vm->gameFlags().platform == Common::kPlatformPC98) {
		uint8 palette[8 * 3];

		for (int i = 0; i < 8; ++i) {
			palette[i * 3 + 0] = ((i >> 1) & 1) * 0xFF;
			palette[i * 3 + 1] = ((i >> 2) & 1) * 0xFF;
			palette[i * 3 + 2] = ((i >> 0) & 1) * 0xFF;
		}

		_system->getPaletteManager()->setPalette(palette, 16, 8);
	}

	_customDimTable = new ScreenDim *[_dimTableCount];
	memset(_customDimTable, 0, sizeof(ScreenDim *) * _dimTableCount);

	_curDimIndex = -1;
	_curDim = nullptr;
	_charSpacing = 0;
	_lineSpacing = 0;
	_overdrawMargin = (_vm->game() == GI_EOB2 && _vm->gameFlags().lang == Common::ZH_TWN);
	for (int i = 0; i < ARRAYSIZE(_textColorsMap); ++i)
		_textColorsMap[i] = i;
	_textColorsMap16bit[0] = _textColorsMap16bit[1] = 0;
	_animBlockPtr = nullptr;
	_animBlockSize = 0;
	_mouseLockCount = 1;
	CursorMan.showMouse(false);

	_forceFullUpdate = false;

	return true;
}

bool Screen::enableScreenDebug(bool enable) {
	bool temp = _debugEnabled;

	if (_debugEnabled != enable) {
		_debugEnabled = enable;
		setResolution();
		_forceFullUpdate = true;
		updateScreen();
	}

	return temp;
}

void Screen::setResolution() {
	byte palette[3 * 256];
	if (!_useHiColorScreen)
		_system->getPaletteManager()->grabPalette(palette, 0, 256);

	int width = 320, height = 200;

	if (_vm->gameFlags().useHiRes) {
		height = 400;

		if (_debugEnabled)
			width = 960;
		else
			width = 640;
	} else {
		if (_debugEnabled)
			width = 640;
		else
			width = 320;
	}

	if (_vm->gameFlags().platform == Common::kPlatformSegaCD)
		height = 224;

	if (_useHiColorScreen) {
		Graphics::PixelFormat px(2, 5, 5, 5, 0, 10, 5, 0, 0);
		Common::List<Graphics::PixelFormat> tryModes = _system->getSupportedFormats();
		for (Common::List<Graphics::PixelFormat>::iterator g = tryModes.begin(); g != tryModes.end(); ++g) {
			if (g->bytesPerPixel != 2 || g->aBits()) {
				g = tryModes.reverse_erase(g);
			} else if (*g == px) {
				tryModes.clear();
				tryModes.push_back(px);
				break;
			}
		}
		initGraphics(width, height, tryModes);
		if (_system->getScreenFormat().bytesPerPixel != 2)
			error("Required graphics mode not supported by platform.");

	} else {
		initGraphics(width, height);
		_system->getPaletteManager()->setPalette(palette, 0, 256);
	}
}

void Screen::enableHiColorMode(bool enabled) {
	if (_useHiColorScreen && enabled) {
		if (!_16bitPalette)
			_16bitPalette = new uint16[1024];
		memset(_16bitPalette, 0, 1024 * sizeof(uint16));
		delete[] _16bitConversionPalette;
		_16bitConversionPalette = nullptr;
		_bytesPerPixel = 2;
	} else {
		if (_useHiColorScreen) {
			if (!_16bitConversionPalette)
				_16bitConversionPalette = new uint16[256];
			memset(_16bitConversionPalette, 0, 256 * sizeof(uint16));
		}

		delete[] _16bitPalette;
		_16bitPalette = nullptr;
		_bytesPerPixel = 1;
	}

	resetPagePtrsAndBuffers(_isSegaCD ? SCREEN_W * _screenHeight : SCREEN_PAGE_SIZE * _bytesPerPixel);
}

int Screen::updateScreen() {
	int res = 0;
	if (_forceFullUpdate) {
		res = SCREEN_W * SCREEN_H;
	} else if (!_dirtyRects.empty()) {
		for (Common::List<Common::Rect>::const_iterator i = _dirtyRects.begin(); i != _dirtyRects.end(); ++i)
			res += (i->width() * i->height());
		// Due to overlapping the value might be larger than the actual vga video memory size.
		res = MIN<int>(res, SCREEN_W * SCREEN_H);
	}

	bool needRealUpdate = _forceFullUpdate || !_dirtyRects.empty() || _paletteChanged;
	_paletteChanged = false;

	if (_useOverlays)
		updateDirtyRectsOvl();
	else if (_isAmiga && _dualPaletteModeSplitY)
		updateDirtyRectsAmiga();
	else
		updateDirtyRects();

	if (_debugEnabled) {
		needRealUpdate = true;

		if (!_useOverlays)
			_system->copyRectToScreen(getPagePtr(2), SCREEN_W, 320, 0, SCREEN_W, SCREEN_H);
		else
			_system->copyRectToScreen(getPagePtr(2), SCREEN_W, 640, 0, SCREEN_W, SCREEN_H);
	}

	if (needRealUpdate)
		updateBackendScreen(true);

	// I've determined this value for the estimated screen update time on legacy hardware experimentally
	// with a side-by-side comparison with DOSBox. This was as close as I got...
	return res / 4000;
}

#ifdef KYRA_SCREEN_IDLE_REFRESH
void Screen::updateBackendScreen(bool force) {
	if (force || _system->getMillis() >= _idleUpdateTimer) {
		_system->updateScreen();
		_idleUpdateTimer = _system->getMillis() + (force ? SCREEN_IDLEREFRESH_RESTART_MSEC : SCREEN_IDLEREFRESH_RATE_MSEC);
	}
}
#else
void Screen::updateBackendScreen(bool) {
	_system->updateScreen();
}
#endif

void Screen::updateDirtyRects() {
	if (_forceFullUpdate) {
		_system->copyRectToScreen(getCPagePtr(0), SCREEN_W, 0, _yTransOffs, SCREEN_W, _screenHeight - _yTransOffs);
	} else {
		const byte *page0 = getCPagePtr(0);
		Common::List<Common::Rect>::iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			_system->copyRectToScreen(page0 + it->top * SCREEN_W + it->left, SCREEN_W, it->left, it->top + _yTransOffs, it->width(), it->height());
		}
	}
	_forceFullUpdate = false;
	_dirtyRects.clear();
}

void Screen::updateDirtyRectsAmiga() {
	if (_forceFullUpdate) {
		uint32 *pos = (uint32*)(_pagePtrs[0] + _dualPaletteModeSplitY * SCREEN_W);
		uint16 h = (SCREEN_H - _dualPaletteModeSplitY) * (SCREEN_W >> 2);
		while (h--)
			*pos++ |= 0x20202020;
		_system->copyRectToScreen(getCPagePtr(0), SCREEN_W, 0, 0, SCREEN_W, SCREEN_H);
	} else {
		Common::List<Common::Rect>::iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			if (it->bottom >= _dualPaletteModeSplitY) {
				int16 startY = MAX<int16>(_dualPaletteModeSplitY, it->top);
				int16 h = it->bottom - startY + 1;
				int16 w = it->width();
				uint8 *pos = _pagePtrs[0] + startY * SCREEN_W + it->left;
				while (h--) {
					for (int x = 0; x < w; ++x)
						*pos++ |= 0x20;
					pos += (SCREEN_W - w);
				}
			}
			_system->copyRectToScreen(_pagePtrs[0] + it->top * SCREEN_W + it->left, SCREEN_W, it->left, it->top, it->width(), it->height());
		}
	}

	_forceFullUpdate = false;
	_dirtyRects.clear();
}

#define mScale2x(dst, dstPitch, src, srcPitch, w, h) \
	if (!_useHiColorScreen) \
		scale2x<uint8, uint16>(dst, dstPitch, src, srcPitch, w, h); \
	else if (_bytesPerPixel == 2) \
		scale2x<uint16, uint32>(dst, dstPitch, src, srcPitch, w, h); \
	 else \
		scale2x<uint8, uint32>(dst, dstPitch, src, srcPitch, w, h)

void Screen::updateDirtyRectsOvl() {
	if (_forceFullUpdate) {
		const byte *src = getCPagePtr(0);
		byte *dst = _sjisOverlayPtrs[0];
		mScale2x(dst, 640, src, SCREEN_W, SCREEN_W, SCREEN_H);
		mergeOverlay(0, 0, 640, 400);
		_system->copyRectToScreen(dst, _useHiColorScreen ? 1280 : 640, 0, 0, 640, 400);
	} else {
		const byte *page0 = getCPagePtr(0);
		byte *ovl0 = _sjisOverlayPtrs[0];
		int dstBpp = _useHiColorScreen ? 2 : 1;

		Common::List<Common::Rect>::iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			byte *dst = ovl0 + it->top * 1280 * dstBpp + (it->left << dstBpp);
			const byte *src = page0 + it->top * SCREEN_W * _bytesPerPixel + it->left * _bytesPerPixel;
			mScale2x(dst, 640, src, SCREEN_W, it->width(), it->height());
			mergeOverlay(it->left<<1, it->top<<1, it->width()<<1, it->height()<<1);
			_system->copyRectToScreen(dst, _useHiColorScreen ? 1280 : 640, it->left << 1, it->top << 1, it->width() << 1, it->height() << 1);
		}
	}

	_forceFullUpdate = false;
	_dirtyRects.clear();
}

#undef mScale2x

template<typename srcType, typename scaleToType>
void Screen::scale2x(uint8 *dst, int dstPitch, const uint8 *src, int srcPitch, int w, int h) {
	int dstAdd = dstPitch - w;
	int srcAdd = srcPitch - w;
	scaleToType *dstL1 = (scaleToType*)dst;
	scaleToType *dstL2 = (scaleToType*)(dst + dstPitch * (sizeof(scaleToType) >> 1));
	const srcType *src1 = (const srcType*)src;

	while (h--) {
		for (int x = 0; x < w; x++) {
			scaleToType col = (sizeof(srcType) == 1 && sizeof(scaleToType) == 4) ? _16bitConversionPalette[*src1++] : *src1++;
			*dstL1++ = *dstL2++ = col | (col << (sizeof(scaleToType) << 2));
		}
		dstL1 += dstAdd; dstL2 += dstAdd;
		src1 += srcAdd;
	}
}

template void Screen::scale2x<uint8, uint16>(uint8 *dst, int dstPitch, const uint8 *src, int srcPitch, int w, int h);
template void Screen::scale2x<uint16, uint32>(uint8 *dst, int dstPitch, const uint8 *src, int srcPitch, int w, int h);
template void Screen::scale2x<uint8, uint32>(uint8 *dst, int dstPitch, const uint8 *src, int srcPitch, int w, int h);

template<typename pixelType>
void Screen::mergeOverlayImpl(int x, int y, int w, int h) {
	const uint8 *src = _sjisOverlayPtrs[1] + y * 640 + x;
	uint16 *p16 = _16bitPalette ? _16bitPalette : _16bitConversionPalette;
	pixelType *dst = (pixelType*)(_sjisOverlayPtrs[0] + y * 640 * sizeof(pixelType) + x * sizeof(pixelType));
	int add = 640 - w;

	while (h--) {
		for (x = 0; x < w; ++x) {
			uint8 col = *src++;
			if (col != _sjisInvisibleColor)
				*dst = (sizeof(pixelType) == 2) ? p16[col] : col;
			dst++;
		}
		dst += add;
		src += add;
	}
}

template void Screen::mergeOverlayImpl<uint8>(int x, int y, int w, int h);
template void Screen::mergeOverlayImpl<uint16>(int x, int y, int w, int h);

const ScreenDim *Screen::getScreenDim(int dim) const {
	assert(dim < _dimTableCount);
	return _customDimTable[dim] ? _customDimTable[dim] : &_dimTable[dim];
}

void Screen::modifyScreenDim(int dim, int x, int y, int w, int h) {
	if (!_customDimTable[dim])
		_customDimTable[dim] = new ScreenDim;

	memcpy(_customDimTable[dim], &_dimTable[dim], sizeof(ScreenDim));
	_customDimTable[dim]->sx = x;
	_customDimTable[dim]->sy = y;
	_customDimTable[dim]->w = w;
	_customDimTable[dim]->h = h;
	if (dim == _curDimIndex || _vm->game() == GI_LOL)
		setScreenDim(dim);
}

void Screen::setScreenDim(int dim) {
	_curDim = getScreenDim(dim);
	_curDimIndex = dim;
}

void Screen::resetPagePtrsAndBuffers(int pageSize) {
	_screenPageSize = pageSize;

	delete[] _pagePtrsBuff;
	memset(_pagePtrs, 0, sizeof(_pagePtrs));

	Common::Array<uint8> realPages;
	for (int i = 0; i < SCREEN_PAGE_NUM; i++) {
		if (Common::find(realPages.begin(), realPages.end(), _pageMapping[i]) == realPages.end())
			realPages.push_back(_pageMapping[i]);
	}

	int numPages = realPages.size();
	uint32 bufferSize = numPages * _screenPageSize;
 
	uint8 *pos = new uint8[bufferSize]();
	_pagePtrsBuff = pos;

	memset(_pagePtrs, 0, sizeof(_pagePtrs));
	for (int i = 0; i < SCREEN_PAGE_NUM; i++) {
		if (_pagePtrs[_pageMapping[i]]) {
			_pagePtrs[i] = _pagePtrs[_pageMapping[i]];
		} else if (pos < &_pagePtrsBuff[bufferSize]) {
			_pagePtrs[i] = pos;
			pos += _screenPageSize;
		} else {
			error("Screen::resetPagePtrsAndBuffers(): Failed to allocate screen page buffers");
		}
	}
}

uint8 *Screen::getPagePtr(int pageNum) {
	assert(pageNum < SCREEN_PAGE_NUM);
	return _pagePtrs[pageNum];
}

const uint8 *Screen::getCPagePtr(int pageNum) const {
	assert(pageNum < SCREEN_PAGE_NUM);
	return _pagePtrs[pageNum];
}

uint8 *Screen::getPageRect(int pageNum, int x, int y, int w, int h) {
	assert(pageNum < SCREEN_PAGE_NUM);
	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, w, h);
	return _pagePtrs[pageNum] + y * SCREEN_W + x;
}

void Screen::clearPage(int pageNum) {
	assert(pageNum < SCREEN_PAGE_NUM);
	if (pageNum == 0 || pageNum == 1)
		_forceFullUpdate = true;
	memset(getPagePtr(pageNum), 0, _screenPageSize);
	clearOverlayPage(pageNum);
}

int Screen::setCurPage(int pageNum) {
	assert(pageNum < SCREEN_PAGE_NUM);
	int previousPage = _curPage;
	_curPage = pageNum;
	return previousPage;
}

void Screen::clearCurPage() {
	if (_curPage == 0 || _curPage == 1)
		_forceFullUpdate = true;
	memset(getPagePtr(_curPage), 0, _screenPageSize);
	clearOverlayPage(_curPage);
}

void Screen::copyWsaRect(int x, int y, int w, int h, int dimState, int plotFunc, const uint8 *src,
						int unk1, const uint8 *unkPtr1, const uint8 *unkPtr2) {
	uint8 *dstPtr = getPagePtr(_curPage);
	uint8 *origDst = dstPtr;

	const ScreenDim *dim = getScreenDim(dimState);
	int dimX1 = dim->sx << 3;
	int dimX2 = dim->w << 3;
	dimX2 += dimX1;

	int dimY1 = dim->sy;
	int dimY2 = dim->h;
	dimY2 += dimY1;

	int temp = y - dimY1;
	if (temp < 0) {
		if ((temp += h) <= 0)
			return;
		else {
			SWAP(temp, h);
			y += temp - h;
			src += (temp - h) * w;
		}
	}

	temp = dimY2 - y;
	if (temp <= 0)
		return;

	if (temp < h)
		h = temp;

	int srcOffset = 0;
	temp = x - dimX1;
	if (temp < 0) {
		temp = -temp;
		srcOffset = temp;
		x += temp;
		w -= temp;
	}

	int srcAdd = 0;

	temp = dimX2 - x;
	if (temp <= 0)
		return;

	if (temp < w) {
		SWAP(w, temp);
		temp -= w;
		srcAdd = temp;
	}

	dstPtr += y * SCREEN_W + x;
	uint8 *dst = dstPtr;

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, w, h);

	if (!_use16ColorMode)
		clearOverlayRect(_curPage, x, y, w, h);

	temp = h;
	int curY = y;
	while (h--) {
		src += srcOffset;
		++curY;
		int cW = w;

		switch (plotFunc) {
		case 0:
			memcpy(dst, src, cW);
			src += cW;
			break;

		case 1:
			while (cW--) {
				uint8 d = *src++;
				uint8 t = unkPtr1[d];
				if (t != 0xFF)
					d = unkPtr2[*dst + (t << 8)];
				*dst++ = d;
			}
			break;

		case 4:
			while (cW--) {
				uint8 d = *src++;
				if (d)
					*dst = d;
				++dst;
			}
			break;

		case 5:
			while (cW--) {
				uint8 d = *src++;
				if (d) {
					uint8 t = unkPtr1[d];
					if (t != 0xFF)
						d = unkPtr2[*dst + (t << 8)];
					*dst = d;
				}
				++dst;
			}
			break;

		case 8:
		case 9:
			while (cW--) {
				uint8 d = *src++;
				uint8 t = _shapePages[0][dst - origDst] & 7;
				if (unk1 < t && (curY > _maskMinY && curY < _maskMaxY))
					d = _shapePages[1][dst - origDst];
				*dst++ = d;
			}
			break;

		case 12:
		case 13:
			while (cW--) {
				uint8 d = *src++;
				if (d) {
					uint8 t = _shapePages[0][dst - origDst] & 7;
					if (unk1 < t && (curY > _maskMinY && curY < _maskMaxY))
						d = _shapePages[1][dst - origDst];
					*dst++ = d;
				} else {
					d = _shapePages[1][dst - origDst];
					*dst++ = d;
				}
			}
			break;

		default:
			break;
		}

		dst = (dstPtr += SCREEN_W);
		src += srcAdd;
	}
}

int Screen::getPagePixel(int pageNum, int x, int y) {
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < _screenHeight);
	if (_bytesPerPixel == 1)
		return _pagePtrs[pageNum][y * SCREEN_W + x];
	else
		return ((uint16*)_pagePtrs[pageNum])[y * SCREEN_W + x];
}

void Screen::setPagePixel(int pageNum, int x, int y, uint8 color) {
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < _screenHeight);

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, 1, 1);

	if (_4bitPixelPacking) {
		color &= 0x0F;
		color |= (color << 4);
	} else if (_renderMode == Common::kRenderCGA) {
		color &= 0x03;
	} else if (_use16ColorMode || (_renderMode == Common::kRenderEGA && !_useHiResEGADithering)) {
		color &= 0x0F;
	}

	if (_bytesPerPixel == 2) {
		((uint16*)_pagePtrs[pageNum])[y * SCREEN_W + x] = _16bitPalette[color];
	} else {
		_pagePtrs[pageNum][y * SCREEN_W + x] = color;
	}
}

void Screen::fadeFromBlack(int delay, const UpdateFunctor *upFunc) {
	fadePalette(getPalette(0), delay, upFunc);
}

void Screen::fadeToBlack(int delay, const UpdateFunctor *upFunc) {
	if (_renderMode == Common::kRenderEGA)
		return;

	Palette pal(getPalette(0).getNumColors());
	fadePalette(pal, delay, upFunc);
}

void Screen::fadePalette(const Palette &pal, int delay, const UpdateFunctor *upFunc) {
	if (_renderMode == Common::kRenderEGA || _bytesPerPixel == 2)
		setScreenPalette(pal);

	updateScreen();

	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderEGA || _bytesPerPixel == 2)
		return;

	int diff = 0, delayInc = 0;
	getFadeParams(pal, delay, delayInc, diff);

	int delayAcc = 0;
	while (!_vm->shouldQuit()) {
		delayAcc += delayInc;

		int refreshed = fadePalStep(pal, diff);

		if (upFunc && upFunc->isValid())
			(*upFunc)();
		else if (_useHiColorScreen)
			updateScreen();
		else
			updateBackendScreen(true);

		if (!refreshed)
			break;

		_vm->delay((delayAcc >> 8) * 1000 / 60);
		delayAcc &= 0xFF;
	}

	// In case we should quit we setup the final palette here. This avoids
	// ugly palette glitches when quitting while fading. This can for example
	// be noticed when quitting while viewing the family album in Kyra3.
	if (_vm->shouldQuit()) {
		setScreenPalette(pal);
	}
}

void Screen::getFadeParams(const Palette &pal, int delay, int &delayInc, int &diff) {
	uint8 maxDiff = 0;

	for (int i = 0; i < pal.getNumColors() * 3; ++i) {
		diff = ABS(pal[i] - (*_screenPalette)[i]);
		maxDiff = MAX<uint8>(maxDiff, diff);
	}

	delayInc = (delay << 8) & 0x7FFF;
	if (maxDiff != 0)
		delayInc /= maxDiff;

	delay = delayInc;
	for (diff = 1; diff <= maxDiff; ++diff) {
		if (delayInc >= 512)
			break;
		delayInc += delay;
	}
}

int Screen::fadePalStep(const Palette &pal, int diff) {
	_internFadePalette->copy(*_screenPalette);

	bool needRefresh = false;

	for (int i = 0; i < pal.getNumColors() * 3; ++i) {
		int c1 = pal[i];
		int c2 = (*_internFadePalette)[i];
		if (c1 != c2) {
			needRefresh = true;
			if (c1 > c2) {
				c2 += diff;
				if (c1 < c2)
					c2 = c1;
			}

			if (c1 < c2) {
				c2 -= diff;
				if (c1 > c2)
					c2 = c1;
			}

			(*_internFadePalette)[i] = (uint8)c2;
		}
	}

	if (needRefresh)
		setScreenPalette(*_internFadePalette);

	return needRefresh ? 1 : 0;
}

void Screen::setPaletteIndex(uint8 index, uint8 red, uint8 green, uint8 blue) {
	Palette &pal = getPalette(0);

	const int offset = index * 3;

	if (pal[offset + 0] == red && pal[offset + 1] == green && pal[offset + 2] == blue)
		return;

	pal[offset + 0] = red;
	pal[offset + 1] = green;
	pal[offset + 2] = blue;

	setScreenPalette(pal);
}

void Screen::getRealPalette(int num, uint8 *dst) {
	const int colors = _use16ColorMode ? 16 : (_isAmiga ? 32 : 256);
	const uint8 *palData = getPalette(num).getData();

	if (!palData) {
		memset(dst, 0, colors * 3);
		return;
	}

	for (int i = 0; i < colors; ++i) {
		dst[0] = (palData[0] * 0xFF) / 0x3F;
		dst[1] = (palData[1] * 0xFF) / 0x3F;
		dst[2] = (palData[2] * 0xFF) / 0x3F;
		dst += 3;
		palData += 3;
	}
}

void Screen::setScreenPalette(const Palette &pal) {
	uint8 screenPal[256 * 3];
	_screenPalette->copy(pal);

	for (int i = 0; i < pal.getNumColors(); ++i) {
		screenPal[3 * i + 0] = (pal[i * 3 + 0] * 0xFF) / 0x3F;
		screenPal[3 * i + 1] = (pal[i * 3 + 1] * 0xFF) / 0x3F;
		screenPal[3 * i + 2] = (pal[i * 3 + 2] * 0xFF) / 0x3F;
	}

	_paletteChanged = true;

	if (_useHiColorScreen) {
		if (_16bitPalette)
			memcpy(_16bitPalette, pal.getData(), 512);

		// Generate 16bit palette for the 8bit/16 bit conversion in scale2x()
		if (_16bitConversionPalette) {
			Graphics::PixelFormat pixelFormat = _system->getScreenFormat();
			for (int i = 0; i < 256; ++i)
				_16bitConversionPalette[i] = pixelFormat.RGBToColor(screenPal[i * 3], screenPal[i * 3 + 1], screenPal[i * 3 + 2]);
			// The whole Surface has to be converted again after each palette change
			_forceFullUpdate = true;
		}
		return;
	}

	_system->getPaletteManager()->setPalette(screenPal, 0, pal.getNumColors());
}

void Screen::transposeScreenOutputY(int yAdd) {
	updateScreen();
	_yTransOffs = yAdd;
}

void Screen::enableDualPaletteMode(int splitY) {
	_dualPaletteModeSplitY = splitY;

	_forceFullUpdate = true;
	_dirtyRects.clear();

	// TODO: We might need to reset the mouse cursor

	updateScreen();
}

void Screen::disableDualPaletteMode() {
	_dualPaletteModeSplitY = 0;
	_forceFullUpdate = true;
}

void Screen::copyToPage0(int y, int h, uint8 page, uint8 *seqBuf) {
	assert(y + h <= _screenHeight);
	const uint8 *src = getPagePtr(page) + y * SCREEN_W;
	uint8 *dstPage = getPagePtr(0) + y * SCREEN_W;
	for (int i = 0; i < h; ++i) {
		for (int x = 0; x < SCREEN_W; ++x) {
			if (seqBuf[x] != src[x]) {
				seqBuf[x] = src[x];
				dstPage[x] = src[x];
			}
		}
		src += SCREEN_W;
		seqBuf += SCREEN_W;
		dstPage += SCREEN_W;
	}
	addDirtyRect(0, y, SCREEN_W, h);
	// This would remove the text in the end sequence of
	// the (Kyrandia 1) FM-TOWNS version.
	// Since this method is just used for the Seqplayer
	// this shouldn't be a problem anywhere else, so it's
	// safe to disable the call here.
	//clearOverlayRect(0, 0, y, SCREEN_W, h);
}

void Screen::copyRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage, int flags) {
	if (x2 < 0) {
		if (x2  <= -w)
			return;
		w += x2;
		x1 -= x2;
		x2 = 0;
	} else if (x2 + w >= SCREEN_W) {
		if (x2 > SCREEN_W)
			return;
		w = SCREEN_W - x2;
	}

	if (y2 < 0) {
		if (y2 <= -h)
			return;
		h += y2;
		y1 -= y2;
		y2 = 0;
	} else if (y2 + h >= _screenHeight) {
		if (y2 > _screenHeight)
			return;
		h = _screenHeight - y2;
	}

	const uint8 *src = getPagePtr(srcPage) + y1 * SCREEN_W * _bytesPerPixel + x1 * _bytesPerPixel;
	uint8 *dst = getPagePtr(dstPage) + y2 * SCREEN_W * _bytesPerPixel + x2 * _bytesPerPixel;

	if (src == dst)
		return;

	if (dstPage == 0 || dstPage == 1)
		addDirtyRect(x2, y2, w, h);

	copyOverlayRegion(x1, y1, x2, y2, w, h, srcPage, dstPage);

	if (flags & CR_NO_P_CHECK) {
		while (h--) {
			memmove(dst, src, w * _bytesPerPixel);
			src += SCREEN_W * _bytesPerPixel;
			dst += SCREEN_W * _bytesPerPixel;
		}
	} else {
		while (h--) {
			for (int i = 0; i < w; ++i) {
				if (_bytesPerPixel == 2) {
					uint px = *(const uint16*)&src[i << 1];
					if (px)
						*(uint16*)&dst[i << 1] = px;
				} else {
					if (src[i])
						dst[i] = src[i];
				}
			}
			src += SCREEN_W * _bytesPerPixel;
			dst += SCREEN_W * _bytesPerPixel;
		}
	}
}

void Screen::copyRegionToBuffer(int pageNum, int x, int y, int w, int h, uint8 *dest) {
	if (y < 0) {
		dest += (-y) * w * _bytesPerPixel;
		h += y;
		y = 0;
	} else if (y + h > _screenHeight) {
		h = _screenHeight - y;
	}

	int pitch = w;
	if (x < 0) {
		dest += -x * _bytesPerPixel;
		w += x;
		x = 0;
	} else if (x + w > SCREEN_W) {
		w = SCREEN_W - x;
	}

	if (w < 0 || h < 0)
		return;

	uint8 *pagePtr = getPagePtr(pageNum);

	for (int i = y; i < y + h; ++i)
		memcpy(dest + (i - y) * pitch * _bytesPerPixel, pagePtr + i * SCREEN_W * _bytesPerPixel + x * _bytesPerPixel, w * _bytesPerPixel);
}

void Screen::copyPage(uint8 srcPage, uint8 dstPage) {
	uint8 *src = getPagePtr(srcPage);
	uint8 *dst = getPagePtr(dstPage);
	if (src != dst)
		memcpy(dst, src, SCREEN_W * _screenHeight * _bytesPerPixel);
	copyOverlayRegion(0, 0, 0, 0, SCREEN_W, _screenHeight, srcPage, dstPage);

	if (dstPage == 0 || dstPage == 1)
		_forceFullUpdate = true;
}

void Screen::copyBlockToPage(int pageNum, int x, int y, int w, int h, const uint8 *src) {
	if (y < 0) {
		src += (-y) * w * _bytesPerPixel;
		h += y;
		y = 0;
	} else if (y + h > _screenHeight) {
		h = _screenHeight - y;
	}

	int pitch = w;
	if (x < 0) {
		src += -x * _bytesPerPixel;
		w += x;
		x = 0;
	} else if (x + w > SCREEN_W) {
		w = SCREEN_W - x;
	}

	if (w < 0 || h < 0)
		return;

	uint8 *dst = getPagePtr(pageNum) + y * SCREEN_W * _bytesPerPixel + x * _bytesPerPixel;

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, w, h);

	clearOverlayRect(pageNum, x, y, w, h);

	while (h--) {
		memcpy(dst, src, w * _bytesPerPixel);
		dst += SCREEN_W * _bytesPerPixel;
		src += pitch * _bytesPerPixel;
	}
}

void Screen::shuffleScreen(int sx, int sy, int w, int h, int srcPage, int dstPage, int ticks, bool transparent) {
	assert(sx >= 0 && w <= SCREEN_W);
	int x;
	uint16 x_offs[SCREEN_W];
	for (x = 0; x < SCREEN_W; ++x)
		x_offs[x] = x;

	for (x = 0; x < w; ++x) {
		int i = _vm->_rnd.getRandomNumber(w - 1);
		SWAP(x_offs[x], x_offs[i]);
	}

	assert(sy >= 0 && h <= SCREEN_H);
	int y;
	uint8 y_offs[SCREEN_H];
	for (y = 0; y < SCREEN_H; ++y)
		y_offs[y] = y;

	for (y = 0; y < h; ++y) {
		int i = _vm->_rnd.getRandomNumber(h - 1);
		SWAP(y_offs[y], y_offs[i]);
	}

	int32 start, now;
	int wait;
	for (y = 0; y < h && !_vm->shouldQuit(); ++y) {
		start = (int32)_system->getMillis();
		int y_cur = y;
		for (x = 0; x < w; ++x) {
			int i = sx + x_offs[x];
			int j = sy + y_offs[y_cur];
			++y_cur;
			if (y_cur >= h)
				y_cur = 0;

			uint8 color = getPagePixel(srcPage, i, j);
			if (!transparent || color != 0)
				setPagePixel(dstPage, i, j, color);
		}
		// forcing full update for now
		_forceFullUpdate = true;
		updateScreen();
		now = (int32)_system->getMillis();
		wait = ticks * _vm->tickLength() - (now - start);
		if (wait > 0)
			_vm->delay(wait);
	}

	copyOverlayRegion(sx, sy, sx, sy, w, h, srcPage, dstPage);

	if (_vm->shouldQuit()) {
		copyRegion(sx, sy, sx, sy, w, h, srcPage, dstPage);
		updateBackendScreen(true);
	}
}

void Screen::fillRect(int x1, int y1, int x2, int y2, uint8 color, int pageNum, bool xored) {
	assert(x2 < SCREEN_W && y2 < _screenHeight);
	uint16 color16 = 0;
	if (pageNum == -1)
		pageNum = _curPage;

	uint8 *dst = getPagePtr(pageNum) + y1 * SCREEN_W * _bytesPerPixel + x1 * _bytesPerPixel;

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x1, y1, x2-x1+1, y2-y1+1);

	clearOverlayRect(pageNum, x1, y1, x2-x1+1, y2-y1+1);

	if (_4bitPixelPacking) {
		color &= 0x0F;
		color |= (color << 4);
	} else if (_renderMode == Common::kRenderCGA) {
		color &= 0x03;
	} else if (_use16ColorMode || (_renderMode == Common::kRenderEGA && !_useHiResEGADithering)) {
		color &= 0x0F;
	} else if (_bytesPerPixel == 2)
		color16 = shade16bitColor(_16bitPalette[color]);

	if (xored) {
		// no 16 bit support for this (unneeded)
		for (; y1 <= y2; ++y1) {
			for (int x = x1; x <= x2; ++x)
				dst[x] ^= color;
			dst += SCREEN_W;
		}
	} else {
		for (; y1 <= y2; ++y1) {
			if (_bytesPerPixel == 2) {
				uint16 *ptr = (uint16*)dst;
				for (int i = 0; i < x2 - x1 + 1; i++)
					*ptr++ = color16;
			} else {
				memset(dst, color, x2 - x1 + 1);
			}
			dst += SCREEN_W * _bytesPerPixel;
		}
	}
}

void Screen::drawBox(int x1, int y1, int x2, int y2, int color) {
	drawClippedLine(x1, y1, x2, y1, color);
	drawClippedLine(x1, y1, x1, y2, color);
	drawClippedLine(x2, y1, x2, y2, color);
	drawClippedLine(x1, y2, x2, y2, color);
}

void Screen::drawShadedBox(int x1, int y1, int x2, int y2, int color1, int color2) {
	assert(x1 >= 0 && y1 >= 0);
	fillRect(x1, y1, x2, y1 + 1, color1);
	fillRect(x2 - 1, y1, x2, y2, color1);

	drawClippedLine(x1, y1, x1, y2, color2);
	drawClippedLine(x1 + 1, y1 + 1, x1 + 1, y2 - 1, color2);
	drawClippedLine(x1, y2 - 1, x2 - 1, y2 - 1, color2);
	drawClippedLine(x1, y2, x2, y2, color2);
}

void Screen::drawClippedLine(int x1, int y1, int x2, int y2, int color) {
	if (x1 < 0)
		x1 = 0;
	else if (x1 > 319)
		x1 = 319;

	if (x2 < 0)
		x2 = 0;
	else if (x2 > 319)
		x2 = 319;

	if (y1 < 0)
		y1 = 0;
	else if (y1 > 199)
		y1 = 199;

	if (y2 < 0)
		y2 = 0;
	else if (y2 > 199)
		y2 = 199;

	if (x1 == x2)
		if (y1 > y2)
			drawLine(true, x1, y2, y1 - y2 + 1, color);
		else
			drawLine(true, x1, y1, y2 - y1 + 1, color);
	else
		if (x1 > x2)
			drawLine(false, x2, y1, x1 - x2 + 1, color);
		else
			drawLine(false, x1, y1, x2 - x1 + 1, color);
}

void Screen::drawLine(bool vertical, int x, int y, int length, int color) {
	uint8 *ptr = getPagePtr(_curPage) + y * SCREEN_W * _bytesPerPixel + x * _bytesPerPixel;

	if (_4bitPixelPacking) {
		color &= 0x0F;
		color |= (color << 4);
	} else if (_renderMode == Common::kRenderCGA) {
		color &= 0x03;
	} else if (_use16ColorMode || (_renderMode == Common::kRenderEGA && !_useHiResEGADithering)) {
		color &= 0x0F;
	} else if (_bytesPerPixel == 2)
		color = shade16bitColor(_16bitPalette[color]);

	if (vertical) {
		assert((y + length) <= SCREEN_H);
		int currLine = 0;
		while (currLine < length) {
			if (_bytesPerPixel == 2)
				*(uint16*)ptr = color;
			else
				*ptr = color;
			ptr += SCREEN_W * _bytesPerPixel;
			currLine++;
		}
	} else {
		assert((x + length) <= SCREEN_W);
		if (_bytesPerPixel == 2) {
			for (int i = 0; i < length; i++) {
				*(uint16*)ptr = color;
				ptr += 2;
			}
		} else {
			memset(ptr, color, length);
		}
	}

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, (vertical) ? 1 : length, (vertical) ? length : 1);

	clearOverlayRect(_curPage, x, y, (vertical) ? 1 : length, (vertical) ? length : 1);
}

void Screen::setAnimBlockPtr(int size) {
	delete[] _animBlockPtr;
	_animBlockPtr = new uint8[size]();
	assert(_animBlockPtr);
	_animBlockSize = size;
}

void Screen::setTextColor(const uint8 *cmap8, int a, int b) {
	memcpy(&_textColorsMap[a], cmap8, (b - a + 1));
	// We need to update the color tables of all fonts, we
	// setup so far here.
	for (int i = 0; i < FID_NUM; ++i) {
		if (_fonts[i])
			_fonts[i]->setColorMap(_textColorsMap);
	}
}

void Screen::setTextColor16bit(const uint16 *cmap16) {
	assert(cmap16);
	_textColorsMap16bit[0] = cmap16[0];
	_textColorsMap16bit[1] = cmap16[1];
	// We need to update the color tables of all fonts, we
	// setup so far here.
	for (int i = 0; i < FID_NUM; ++i) {
		if (_fonts[i])
			_fonts[i]->set16bitColorMap(_textColorsMap16bit);
	}
}

int Screen::setFontStyles(FontId fontId, int styles) {
	assert(_fonts[fontId]);
	SWAP(_fontStyles, styles);
	_fonts[fontId]->setStyles(_fontStyles);
	return styles;
}

bool Screen::loadFont(FontId fontId, const char *filename) {
	if (fontId == FID_SJIS_FNT) {
		warning("Trying to replace system SJIS font");
		return true;
	}

	Font *&fnt = _fonts[fontId];
	int temp = 0;

	if (!fnt) {
		if (_vm->game() == GI_KYRA1 && _isAmiga) {
			fnt = new AMIGAFont();
		} else if (fontId == FID_CHINESE_FNT) {
			Common::Array<Font*> *fa = new Common::Array<Font*>;
			if (_vm->game() == GI_KYRA1) {
				const uint16 *lookupTable = _vm->staticres()->loadRawDataBe16(k1TwoByteFontLookupTable, temp);
				fa->push_back(new ChineseOneByteFontLoK(SCREEN_W));
				fa->push_back(new ChineseTwoByteFontLoK(SCREEN_W, lookupTable, temp));
				fnt = new MultiSubsetFont(fa);
			} else {
				Font *fn1 = 0;
				Font *fn2 = 0;
				if (_vm->game() == GI_KYRA2) {
					fn1 = new ChineseOneByteFontHOF(SCREEN_W);
					fn2 = new ChineseTwoByteFontHOF(SCREEN_W);
				}
#ifdef ENABLE_LOL
				else if (_vm->game() == GI_LOL) {
					// Same as next one but with different spacing
					fn1 = new ChineseOneByteFontLoL(SCREEN_W);
					fn2 = new ChineseTwoByteFontLoL(SCREEN_W);
				}
#endif
				else {
					fn1 = new ChineseOneByteFontMR(SCREEN_W);
					fn2 = new ChineseTwoByteFontMR(SCREEN_W);
				}
				fa->push_back(fn1);
				fa->push_back(fn2);
				fnt = new MultiSubsetFont(fa);
				_vm->staticres()->setLanguage(_vm->gameFlags().extraLang, k2FontData);
				const uint8 *oneByteData = _vm->staticres()->loadRawData(k2FontData, temp);
				Common::MemoryReadStream str(oneByteData, temp);
				fnt->load(str);
			}
		} else if (fontId == FID_KOREAN_FNT) {
			const uint16 *lookupTable = _vm->staticres()->loadRawDataBe16(k1TwoByteFontLookupTable, temp);
			fnt = new JohabFontLoK(_fonts[FID_8_FNT], lookupTable, temp);
		} else {
			fnt = new DOSFont();
		}
		assert(fnt);
	}

	Common::SeekableReadStream *file = _vm->resource()->createReadStream(filename);
	if (!file)
		error("Font file '%s' is missing", filename);

	bool ret = fnt->load(*file);

	fnt->setColorMap(_textColorsMap);
	delete file;
	return ret;
}

Screen::FontId Screen::setFont(FontId fontId) {
	FontId prev = _currentFont;
	_currentFont = fontId;

	assert(_fonts[_currentFont]);
	return prev;
}

int Screen::getFontHeight() const {
	return _fonts[_currentFont]->getHeight();
}

int Screen::getFontWidth() const {
	return _fonts[_currentFont]->getWidth();
}

int Screen::getCharWidth(uint16 c) const {
	int width = _fonts[_currentFont]->getCharWidth(c);
	if (_isSegaCD || _fonts[_currentFont]->getType() == Font::kASCII || (_fonts[_currentFont]->getType() == Font::kJohab && c < 0x80))
		width += _charSpacing;
	return width;
}

int Screen::getCharHeight(uint16 c) const {
	return _fonts[_currentFont]->getCharHeight(c);
}

int Screen::getTextWidth(const char *str, bool nextWordOnly) {
	int curLineLen = 0;
	int maxLineLen = 0;

	FontId curFont = _currentFont;
	Font::Type curType = _fonts[curFont]->getType();

	while (1) {
		if (_sjisMixedFontMode && curType == Font::kASCII)
			setFont((*str & 0x80) ? ((_vm->game() == GI_EOB2 && curFont == FID_6_FNT) ? FID_SJIS_SMALL_FNT : FID_SJIS_FNT) : curFont);

		uint16 c = fetchChar(str);

		if (c == 0 || (nextWordOnly && (c == 2 || c == 6 || c == 13 || c == 32 || c == 0x4081))) {
			break;
		} else if (c < 128 && _lineBreakChars.contains((char)c)) {
			if (curLineLen > maxLineLen)
				maxLineLen = curLineLen;
			else
				curLineLen = 0;
		} else {
			curLineLen += getCharWidth(c);
		}
	}

	return MAX(curLineLen, maxLineLen);
}

int Screen::getNumberOfCharacters(const char *str) {
	int res = 0;
	while (fetchChar(str))
		++res;
	return res;
}

void Screen::printText(const char *str, int x, int y, uint8 color1, uint8 color2, int pitch) {
	uint16 cmap16[2];
	if (_16bitPalette) {
		cmap16[0] = color2 ? shade16bitColor(_16bitPalette[color2]) : 0xFFFF;
		cmap16[1] = _16bitPalette[color1];
		setTextColor16bit(cmap16);
	}

	uint8 cmap8[2];
	cmap8[0] = color2;
	cmap8[1] = color1;
	setTextColor(cmap8, 0, 1);

	FontId curFont = _currentFont;
	Font::Type curType = _fonts[curFont]->getType();

	if (x < 0)
		x = 0;
	else if (x >= SCREEN_W)
		return;

	int x_start = x;
	if (y < 0)
		y = 0;
	else if (y >= _screenHeight)
		return;

	int charHeight = 0;
	bool enableWordWrap = _isSegaCD && _vm->gameFlags().lang != Common::JA_JPN;

	while (1) {
		if (_sjisMixedFontMode && curType == Font::kASCII)
			setFont((*str & 0x80) ? ((_vm->game() == GI_EOB2 && curFont == FID_6_FNT) ? FID_SJIS_SMALL_FNT : FID_SJIS_FNT) : curFont);

		uint16 c = fetchChar(str);
		charHeight = MAX<int>(charHeight, getCharHeight(c));

		if (c == 0) {
			break;
		} else if (c < 128 && _lineBreakChars.contains((char)c)) {
			x = x_start;
			y += (charHeight + _lineSpacing);
		} else {
			bool needDrawing = true;
			int charWidth = getCharWidth(c);
			int needSpace = enableWordWrap ? getTextWidth(str, true) + charWidth : charWidth;
			if (x + needSpace > _textMarginRight) {
				if (_overdrawMargin && (x + needSpace <= Screen::SCREEN_W)) {
					// The Chinese version of EOB II has a weird way of handling the right margin.
					// It will squeeze in the final character even if it goes over the margin
					// (see e. g. the chargen screen "Your party is complete. Select the PLAY button...").
					drawChar(c, x, y, pitch);
					needDrawing = false;
				}
				x = x_start;
				y += (charHeight + _lineSpacing);
				if (enableWordWrap) {
					// skip space at beginning of the line
					c = fetchChar(str);
					if (c == 0)
						return;
					charWidth = getCharWidth(c);
				}
				if (y >= _screenHeight)
					break;
			}
			if (needDrawing) {
				drawChar(c, x, y, pitch);
				x += charWidth;
			}
		}
	}
}

uint16 Screen::fetchChar(const char *&s) const {
	const int fontType = _fonts[_currentFont]->getType();
	if (fontType == Font::kASCII)
		return (uint8)*s++;

	uint16 ch = (uint8)*s++;

	if (((fontType == Font::kSJIS || fontType == Font::kJIS_X0201) && (ch <= 0x7F || (ch >= 0xA1 && ch <= 0xDF))) ||
		((fontType == Font::kBIG5 || fontType == Font::kJohab) && ch < 0x80))
			return ch;

	ch |= (uint8)(*s++) << 8;
	return ch;
}

void Screen::drawChar(uint16 c, int x, int y, int pitch) {
	Font *fnt = _fonts[_currentFont];
	assert(fnt);

	const bool useOverlay = fnt->usesOverlay();
	const int charWidth = fnt->getCharWidth(c);
	const int charHeight = fnt->getCharHeight(c);

	if (x < 0 || y < 0)
		return;
	if (x + charWidth > SCREEN_W || y + charHeight > _screenHeight)
		return;

	if (_isSegaCD) {
		fnt->drawChar(c, _textRenderBuffer + (((y >> 3) * pitch + (x >> 3)) << 5) + ((y & 7) << 2) + ((x & 7) >> 1), pitch, x & 7, y & 7);
	} else if (useOverlay) {
		uint8 *destPage = getOverlayPtr(_curPage);
		if (!destPage) {
			warning("trying to draw SJIS char on unsupported page %d", _curPage);
			return;
		}

		int bpp = (_currentFont == Screen::FID_SJIS_LARGE_FNT) ? 2 : 1;
		destPage += (y * 2) * 640 * bpp + (x * 2 * bpp);

		fnt->drawChar(c, destPage, 640, bpp);
	} else {
		fnt->drawChar(c, getPagePtr(_curPage) + y * SCREEN_W * _bytesPerPixel + x * _bytesPerPixel, SCREEN_W, _bytesPerPixel);
	}

	if (!_isSegaCD && (_curPage == 0 || _curPage == 1))
		addDirtyRect(x, y, charWidth, charHeight);
}

void Screen::drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...) {
	if (!shapeData)
		return;

	if (_vm->gameFlags().useAltShapeHeader)
		shapeData += 2;

	if (*shapeData & 1)
		flags |= kDRAWSHP_COMPACT;

	va_list args;
	va_start(args, flags);

	static const int drawShapeVar2[] = {
		1, 3, 2, 5, 4, 3, 2, 1
	};

	_dsShapeFadingTable = nullptr;
	_dsShapeFadingLevel = 0;
	_dsColorTable = nullptr;
	_dsTransparencyTable1 = nullptr;
	_dsTransparencyTable2 = nullptr;
	_dsBackgroundFadingTable = nullptr;
	_dsDrawLayer = 0;

	if (flags & kDRAWSHP_COLOR) {
		_dsColorTable = va_arg(args, uint8 *);
	}

	if (flags & kDRAWSHP_FADE) {
		_dsShapeFadingTable = va_arg(args, uint8 *);
		_dsShapeFadingLevel = va_arg(args, int);
		if (!_dsShapeFadingLevel)
			flags &= ~kDRAWSHP_FADE;
	}

	if (flags & kDRAWSHP_TRANSPARENT) {
		_dsTransparencyTable1 = va_arg(args, uint8 *);
		_dsTransparencyTable2 = va_arg(args, uint8 *);
	}

	if (flags & kDRAWSHP_PREDATOR) {
		_drawShapeVar1 = (_drawShapeVar1 + 1) & 0x7;
		_drawShapeVar3 = drawShapeVar2[_drawShapeVar1];
		_drawShapeVar4 = 0;
		_drawShapeVar5 = 256;
	}

	if (flags & kDRAWSHP_MORPH)
		_drawShapeVar5 = va_arg(args, int);

	if (flags & kDRAWSHP_PRIORITY)
		_dsDrawLayer = va_arg(args, int);

	if (flags & kDRAWSHP_SCALE) {
		_dsScaleW = va_arg(args, int);
		_dsScaleH = va_arg(args, int);
	} else {
		_dsScaleW = 0x100;
		_dsScaleH = 0x100;
	}

	if ((flags & kDRAWSHP_BCKGRNDFADE) && _vm->game() != GI_KYRA1)
		_dsBackgroundFadingTable = va_arg(args, uint8 *);

	va_end(args);

	static const DsMarginSkipFunc dsMarginFunc[] = {
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeMarginScaleUpwind,
		&Screen::drawShapeMarginScaleDownwind,
		&Screen::drawShapeMarginScaleUpwind,
		&Screen::drawShapeMarginScaleDownwind
	};

	static const DsMarginSkipFunc dsSkipFunc[] = {
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeSkipScaleUpwind,
		&Screen::drawShapeSkipScaleDownwind,
		&Screen::drawShapeSkipScaleUpwind,
		&Screen::drawShapeSkipScaleDownwind
	};

	static const DsLineFunc dsLineFunc[] = {
		&Screen::drawShapeProcessLineNoScaleUpwind,
		&Screen::drawShapeProcessLineNoScaleDownwind,
		&Screen::drawShapeProcessLineNoScaleUpwind,
		&Screen::drawShapeProcessLineNoScaleDownwind,
		&Screen::drawShapeProcessLineScaleUpwind,
		&Screen::drawShapeProcessLineScaleDownwind,
		&Screen::drawShapeProcessLineScaleUpwind,
		&Screen::drawShapeProcessLineScaleDownwind
	};

	static const DsPlotFunc dsPlotFunc[] = {
		&Screen::drawShapePlotType0,		// used by Kyra 1 + 2
		&Screen::drawShapePlotType1,		// used by Kyra 3
		nullptr,
		&Screen::drawShapePlotType3_7,		// used by Kyra 3 (shadow)
		&Screen::drawShapePlotType4,		// used by Kyra 1, 2 + 3
		&Screen::drawShapePlotType5,		// used by Kyra 1
		&Screen::drawShapePlotType6,		// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType3_7,		// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType8,		// used by Kyra 2
		&Screen::drawShapePlotType9,		// used by Kyra 1 + 3
		nullptr,
		&Screen::drawShapePlotType11_15,	// used by Kyra 1 (invisibility) + Kyra 3 (shadow)
		&Screen::drawShapePlotType12,		// used by Kyra 2
		&Screen::drawShapePlotType13,		// used by Kyra 1
		&Screen::drawShapePlotType14,		// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType11_15,	// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType16,		// used by LoL PC-98/16 Colors (teleporters),
		nullptr, nullptr, nullptr,
		&Screen::drawShapePlotType20,		// used by LoL (heal spell effect)
		&Screen::drawShapePlotType21,		// used by LoL (white tower spirits)
		nullptr, nullptr, nullptr, nullptr,	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr,
		&Screen::drawShapePlotType33,		// used by LoL (blood spots on the floor)
		nullptr, nullptr, nullptr,
		&Screen::drawShapePlotType37,		// used by LoL (monsters)
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		&Screen::drawShapePlotType48,		// used by LoL (slime spots on the floor)
		nullptr, nullptr, nullptr,
		&Screen::drawShapePlotType52,		// used by LoL (projectiles)
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr
	};

	int scaleCounterV = 0;

	const int drawFunc = flags & 0x0F;
	DsMarginSkipFunc dsProcessMargin = dsMarginFunc[drawFunc];
	DsMarginSkipFunc dsScaleSkip = dsSkipFunc[drawFunc];
	DsLineFunc dsProcessLine = dsLineFunc[drawFunc];

	const int ppc = (flags >> 8) & 0x3F;
	DsPlotFunc dsPlot = dsPlotFunc[ppc];
	DsPlotFunc dsPlot2 = dsPlotFunc[ppc], dsPlot3 = dsPlotFunc[ppc];
	if (_vm->gameFlags().gameID == GI_KYRA3 && (flags & kDRAWSHP_PRIORITY))
		dsPlot3 = dsPlotFunc[ppc & ~8];

	if (!dsPlot || !dsPlot2 || !dsPlot3) {
		if (!dsPlot2)
			warning("Missing drawShape plotting method type %d", ppc);
		if (dsPlot3 != dsPlot2 && !dsPlot3)
			warning("Missing drawShape plotting method type %d", ppc & ~8);
		return;
	}

	int curY = y;
	const uint8 *src = shapeData;
	uint8 *dst = _dsDstPage = getPagePtr(pageNum);

	const ScreenDim *dsDim = getScreenDim(sd);
	dst += (dsDim->sx << 3);

	if (!(flags & kDRAWSHP_WINREL))
		x -= (dsDim->sx << 3);

	int x2 = (dsDim->w << 3);
	int y1 = dsDim->sy;
	if (flags & kDRAWSHP_WINREL)
		y += y1;

	int y2 = y1 + dsDim->h;

	uint16 shapeFlags = READ_LE_UINT16(src); src += 2;

	int shapeHeight = *src++;
	uint16 shapeWidth = READ_LE_UINT16(src); src += 2;

	int shpWidthScaled1 = shapeWidth;
	int shpWidthScaled2 = shapeWidth;

	if (flags & kDRAWSHP_SCALE) {
		shapeHeight = (shapeHeight * _dsScaleH) >> 8;
		shpWidthScaled1 = shpWidthScaled2 = (shapeWidth * _dsScaleW) >> 8;

		if (!shapeHeight || !shpWidthScaled1)
			return;
	}

	if (flags & kDRAWSHP_CENTER) {
		x -= (shpWidthScaled1 >> 1);
		y -= (shapeHeight >> 1);
	}

	src += 3;

	uint16 frameSize = READ_LE_UINT16(src); src += 2;

	int colorTableColors = ((_vm->game() != GI_KYRA1) && (shapeFlags & 4)) ? *src++ : 16;

	if (!(flags & kDRAWSHP_COLOR) && (shapeFlags & 1))
		_dsColorTable = src;

	if (flags & kDRAWSHP_COMPACT)
		src += colorTableColors;

	if (!(shapeFlags & 2)) {
		decodeFrame4(src, _animBlockPtr, frameSize);
		src = _animBlockPtr;
	}

	int t = (flags & kDRAWSHP_YFLIP) ? y2 - y - shapeHeight : y - y1;

	if (t < 0) {
		shapeHeight += t;
		if (shapeHeight <= 0) {
			return;
		}

		t *= -1;
		const uint8 *srcBackUp = nullptr;

		do {
			_dsOffscreenScaleVal1 = 0;
			srcBackUp = src;
			_dsTmpWidth = shapeWidth;

			int cnt = shapeWidth;
			(this->*dsScaleSkip)(dst, src, cnt);

			scaleCounterV += _dsScaleH;

			if (scaleCounterV & 0xFF00) {
				uint8 r = scaleCounterV >> 8;
				scaleCounterV &= 0xFF;
				t -= r;
			}
		} while (!(scaleCounterV & 0xFF00) && (t > 0));

		if (t < 0) {
			src = srcBackUp;
			scaleCounterV += (-t << 8);
		}

		if (!(flags & kDRAWSHP_YFLIP))
			y = y1;
	}

	t = (flags & kDRAWSHP_YFLIP) ? y + shapeHeight - y1 : y2 - y;
	if (t <= 0)
		return;

	if (t < shapeHeight) {
		shapeHeight = t;
		if (flags & kDRAWSHP_YFLIP)
			y = y1;
	}

	_dsOffscreenLeft = 0;
	if (x < 0) {
		shpWidthScaled1 += x;
		_dsOffscreenLeft = -x;
		if (_dsOffscreenLeft >= shpWidthScaled2)
			return;
		x = 0;
	}

	_dsOffscreenRight = 0;
	t = x2 - x;

	if (t <= 0)
		return;

	if (t < shpWidthScaled1) {
		shpWidthScaled1 = t;
		_dsOffscreenRight = shpWidthScaled2 - _dsOffscreenLeft - shpWidthScaled1;
	}

	int dsPitch = 320;
	int ty = y;

	if (flags & kDRAWSHP_YFLIP) {
		dsPitch *= -1;
		ty = ty - 1 + shapeHeight;
	}

	if (flags & kDRAWSHP_XFLIP) {
		SWAP(_dsOffscreenLeft, _dsOffscreenRight);
		dst += (shpWidthScaled1 - 1);
	}

	dst += (320 * ty + x);

	if (flags & kDRAWSHP_SCALE) {
		_dsOffscreenRight = 0;
		_dsOffscreenScaleVal2 = _dsOffscreenLeft;
		_dsOffscreenLeft <<= 8;
		_dsOffscreenScaleVal1 = (_dsOffscreenLeft % _dsScaleW) * -1;
		_dsOffscreenLeft /= _dsScaleW;
	}

	if (shapeHeight <= 0 || shpWidthScaled1 <= 0)
		return;

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, shpWidthScaled1, shapeHeight);
	clearOverlayRect(pageNum, x, y, shpWidthScaled1, shapeHeight);

	uint8 *d = dst;

	bool normalPlot = true;
	while (true) {
		while (!(scaleCounterV & 0xFF00)) {
			scaleCounterV += _dsScaleH;
			if (!(scaleCounterV & 0xFF00)) {
				_dsTmpWidth = shapeWidth;
				int cnt = shapeWidth;
				(this->*dsScaleSkip)(d, src, cnt);
			}
		}

		const uint8 *b_src = src;

		do {
			src = b_src;
			_dsTmpWidth = shapeWidth;
			int cnt = _dsOffscreenLeft;
			int scaleState = (this->*dsProcessMargin)(d, src, cnt);

			if (_dsTmpWidth) {
				cnt += shpWidthScaled1;
				if (cnt > 0) {
					if (flags & kDRAWSHP_PRIORITY)
						normalPlot = (curY > _maskMinY && curY < _maskMaxY);
					dsPlot = normalPlot ? dsPlot2 : dsPlot3;
					(this->*dsProcessLine)(d, src, dsPlot, cnt, scaleState);
				}
				cnt += _dsOffscreenRight;
				if (cnt)
					(this->*dsScaleSkip)(d, src, cnt);
			}
			dst += dsPitch;
			d = dst;
			++curY;

			if (!--shapeHeight)
				return;

			scaleCounterV -= 0x100;
		} while (scaleCounterV & 0xFF00);
	}
}

int Screen::drawShapeMarginNoScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	while (cnt-- > 0) {
		if (*src++)
			continue;
		cnt = cnt + 1 - (*src++);
	}

	cnt++;
	dst -= cnt;
	return 0;
}

int Screen::drawShapeMarginNoScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	while (cnt-- > 0) {
		if (*src++)
			continue;
		cnt = cnt + 1 - (*src++);
	}

	cnt++;
	dst += cnt;
	return 0;
}

int Screen::drawShapeMarginScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	_dsTmpWidth -= cnt;

	while (cnt > 0) {
		--cnt;
		if (*src++)
			continue;

		cnt = cnt + 1 - (*src++);
	}

	if (!cnt)
		return _dsOffscreenScaleVal1;

	_dsTmpWidth += cnt;

	int i = (_dsOffscreenLeft - cnt) * _dsScaleW;
	int res = i & 0xFF;
	i >>= 8;
	i -= _dsOffscreenScaleVal2;
	dst += i;
	cnt = -i;

	return res;
}

int Screen::drawShapeMarginScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	_dsTmpWidth -= cnt;

	while (cnt > 0) {
		--cnt;
		if (*src++)
			continue;

		cnt = cnt + 1 - (*src++);
	}

	if (!cnt)
		return _dsOffscreenScaleVal1;

	_dsTmpWidth += cnt;

	int i = (_dsOffscreenLeft - cnt) * _dsScaleW;
	int res = i & 0xFF;
	i >>= 8;
	i -= _dsOffscreenScaleVal2;
	dst -= i;
	cnt = -i;

	return res;
}

int Screen::drawShapeSkipScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	cnt = _dsTmpWidth;

	if (cnt <= 0)
		return 0;

	do {
		--cnt;
		if (*src++)
			continue;
		cnt = cnt + 1 - (*src++);
	} while (cnt > 0);

	return 0;
}

int Screen::drawShapeSkipScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	cnt = _dsTmpWidth;
	bool found = false;

	if (cnt == 0)
		return 0;

	do {
		--cnt;
		if (*src++)
			continue;
		found = true;
		cnt = cnt + 1 - (*src++);
	} while (cnt > 0);

	return found ? 0 : _dsOffscreenScaleVal1;
}

void Screen::drawShapeProcessLineNoScaleUpwind(uint8 *&dst, const uint8 *&src, const DsPlotFunc plot, int &cnt, int16) {
	do {
		uint8 c = *src++;
		if (c) {
			uint8 *d = dst++;
			(this->*plot)(d, c);
			cnt--;
		} else {
			c = *src++;
			dst += c;
			cnt -= c;
		}
	} while (cnt > 0);
}

void Screen::drawShapeProcessLineNoScaleDownwind(uint8 *&dst, const uint8 *&src, const DsPlotFunc plot, int &cnt, int16) {
	do {
		uint8 c = *src++;
		if (c) {
			uint8 *d = dst--;
			(this->*plot)(d, c);
			cnt--;
		} else {
			c = *src++;
			dst -= c;
			cnt -= c;
		}
	} while (cnt > 0);
}

void Screen::drawShapeProcessLineScaleUpwind(uint8 *&dst, const uint8 *&src, const DsPlotFunc plot, int &cnt, int16 scaleState) {
	int c = 0;

	do {
		if ((scaleState & 0x8000) || !(scaleState & 0xFF00)) {
			c = *src++;
			_dsTmpWidth--;
			if (c) {
				scaleState += _dsScaleW;
			} else {
				_dsTmpWidth++;
				c = *src++;
				_dsTmpWidth -= c;
				int r = c * _dsScaleW + scaleState;
				dst += (r >> 8);
				cnt -= (r >> 8);
				scaleState = r & 0xFF;
			}
		} else if (scaleState) {
			(this->*plot)(dst++, c);
			scaleState -= 0x100;
			cnt--;
		}
	} while (cnt > 0);

	cnt = -1;
}

void Screen::drawShapeProcessLineScaleDownwind(uint8 *&dst, const uint8 *&src, const DsPlotFunc plot, int &cnt, int16 scaleState) {
	int c = 0;

	do {
		if ((scaleState & 0x8000) || !(scaleState & 0xFF00)) {
			c = *src++;
			_dsTmpWidth--;
			if (c) {
				scaleState += _dsScaleW;
			} else {
				_dsTmpWidth++;
				c = *src++;
				_dsTmpWidth -= c;
				int r = c * _dsScaleW + scaleState;
				dst -= (r >> 8);
				cnt -= (r >> 8);
				scaleState = r & 0xFF;
			}
		} else {
			(this->*plot)(dst--, c);
			scaleState -= 0x100;
			cnt--;
		}
	} while (cnt > 0);

	cnt = -1;
}

void Screen::drawShapePlotType0(uint8 *dst, uint8 cmd) {
	*dst = cmd;
}

void Screen::drawShapePlotType1(uint8 *dst, uint8 cmd) {
	for (int i = 0; i < _dsShapeFadingLevel; ++i)
		cmd = _dsShapeFadingTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType3_7(uint8 *dst, uint8 cmd) {
	cmd = *dst;
	for (int i = 0; i < _dsShapeFadingLevel; ++i)
		cmd = _dsShapeFadingTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType4(uint8 *dst, uint8 cmd) {
	*dst = _dsColorTable[cmd];
}

void Screen::drawShapePlotType5(uint8 *dst, uint8 cmd) {
	cmd = _dsColorTable[cmd];
	for (int i = 0; i < _dsShapeFadingLevel; ++i)
		cmd = _dsShapeFadingTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType6(uint8 *dst, uint8 cmd) {
	int t = _drawShapeVar4 + _drawShapeVar5;
	if (t & 0xFF00) {
		cmd = dst[_drawShapeVar3];
		t &= 0xFF;
	} else {
		cmd = _dsColorTable[cmd];
	}

	_drawShapeVar4 = t;
	*dst = cmd;
}

void Screen::drawShapePlotType8(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7F) & 0x87;
	if (_dsDrawLayer < t)
		cmd = _shapePages[1][relOffs];

	*dst = cmd;
}

void Screen::drawShapePlotType9(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7F) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		for (int i = 0; i < _dsShapeFadingLevel; ++i)
			cmd = _dsShapeFadingTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType11_15(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7F) & 0x87;

	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		cmd = *dst;
		for (int i = 0; i < _dsShapeFadingLevel; ++i)
			cmd = _dsShapeFadingTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType12(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7F) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		cmd = _dsColorTable[cmd];
	}

	*dst = cmd;
}

void Screen::drawShapePlotType13(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7F) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		cmd = _dsColorTable[cmd];
		for (int i = 0; i < _dsShapeFadingLevel; ++i)
			cmd = _dsShapeFadingTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType14(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7F) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		t = _drawShapeVar4 + _drawShapeVar5;
		if (t & 0xFF00) {
			cmd = dst[_drawShapeVar3];
			t &= 0xFF;
		} else {
			cmd = _dsColorTable[cmd];
		}
	}

	_drawShapeVar4 = t;
	*dst = cmd;
}

void Screen::drawShapePlotType16(uint8 *dst, uint8 cmd) {
	uint8 tOffs = _dsTransparencyTable1[cmd];
	if (!(tOffs & 0x80))
		cmd = _dsTransparencyTable2[tOffs << 8 | *dst];
	*dst = cmd;
}

void Screen::drawShapePlotType20(uint8 *dst, uint8 cmd) {
	cmd = _dsColorTable[cmd];
	uint8 tOffs = _dsTransparencyTable1[cmd];
	if (!(tOffs & 0x80))
		cmd = _dsTransparencyTable2[tOffs << 8 | *dst];

	*dst = cmd;
}

void Screen::drawShapePlotType21(uint8 *dst, uint8 cmd) {
	cmd = _dsColorTable[cmd];
	uint8 tOffs = _dsTransparencyTable1[cmd];
	if (!(tOffs & 0x80))
		cmd = _dsTransparencyTable2[tOffs << 8 | *dst];

	for (int i = 0; i < _dsShapeFadingLevel; ++i)
		cmd = _dsShapeFadingTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType33(uint8 *dst, uint8 cmd) {
	if (cmd == 255) {
		*dst = _dsBackgroundFadingTable[*dst];
	} else {
		for (int i = 0; i < _dsShapeFadingLevel; ++i)
			cmd = _dsShapeFadingTable[cmd];
		if (cmd)
			*dst = cmd;
	}
}

void Screen::drawShapePlotType37(uint8 *dst, uint8 cmd) {
	cmd = _dsColorTable[cmd];

	if (cmd == 255) {
		cmd = _dsBackgroundFadingTable[*dst];
	} else {
		for (int i = 0; i < _dsShapeFadingLevel; ++i)
			cmd = _dsShapeFadingTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType48(uint8 *dst, uint8 cmd) {
	uint8 offs = _dsTransparencyTable1[cmd];
	if (!(offs & 0x80))
		cmd = _dsTransparencyTable2[(offs << 8) | *dst];
	*dst = cmd;
}

void Screen::drawShapePlotType52(uint8 *dst, uint8 cmd) {
	cmd = _dsColorTable[cmd];
	uint8 offs = _dsTransparencyTable1[cmd];

	if (!(offs & 0x80))
		cmd = _dsTransparencyTable2[(offs << 8) | *dst];

	*dst = cmd;
}

void Screen::decodeFrame1(const uint8 *src, uint8 *dst, uint32 size) {
	const uint8 *dstEnd = dst + size;

	struct Pattern {
		const uint8 *pos;
		uint16 len;
	};

	Pattern *patterns = new Pattern[3840];
	uint16 numPatterns = 0;
	uint8 nib = 0;

	uint16 code = decodeEGAGetCode(src, nib);
	uint8 last = code & 0xFF;

	uint8 *dstPrev = dst;
	uint16 count = 1;
	uint16 countPrev = 1;

	*dst++ = last;

	while (dst < dstEnd) {
		code = decodeEGAGetCode(src, nib);
		uint8 cmd = code >> 8;

		if (cmd--) {
			code = (cmd << 8) | (code & 0xFF);
			uint8 *tmpDst = dst;

			if (code < numPatterns) {
				const uint8 *tmpSrc = patterns[code].pos;
				countPrev = patterns[code].len;
				last = *tmpSrc;
				for (int i = 0; i < countPrev; i++)
					*dst++ = *tmpSrc++;

			} else {
				const uint8 *tmpSrc = dstPrev;
				count = countPrev;
				for (int i = 0; i < countPrev; i++)
					*dst++ = *tmpSrc++;
				*dst++ = last;
				countPrev++;
			}

			if (numPatterns < 3840) {
				patterns[numPatterns].pos = dstPrev;
				patterns[numPatterns++].len = ++count;
			}

			dstPrev = tmpDst;
			count = countPrev;

		} else {
			*dst++ = last = (code & 0xFF);

			if (numPatterns < 3840) {
				patterns[numPatterns].pos = dstPrev;
				patterns[numPatterns++].len = ++count;
			}

			dstPrev = dst - 1;
			count = 1;
			countPrev = 1;
		}
	}
	delete[] patterns;
}

uint16 Screen::decodeEGAGetCode(const uint8 *&pos, uint8 &nib) {
	uint16 res = READ_BE_UINT16(pos++);
	if ((++nib) & 1) {
		res >>= 4;
	} else {
		pos++;
		res &= 0xFFF;
	}
	return res;
}

void Screen::decodeFrame3(const uint8 *src, uint8 *dst, uint32 size, bool isAmiga) {
	const uint8 *dstEnd = dst + size;
	while (dst < dstEnd) {
		int8 code = *src++;
		if (code == 0) {
			uint16 sz = isAmiga ? READ_LE_UINT16(src) : READ_BE_UINT16(src);
			src += 2;
			memset(dst, *src++, sz);
			dst += sz;
		} else if (code < 0) {
			memset(dst, *src++, -code);
			dst -= code;
		} else {
			memcpy(dst, src, code);
			dst += code;
			src += code;
		}
	}
}

uint Screen::decodeFrame4(const uint8 *src, uint8 *dst, uint32 dstSize) {
	uint8 *dstOrig = dst;
	uint8 *dstEnd = dst + dstSize;
	while (1) {
		int count = dstEnd - dst;
		if (count == 0)
			break;

		uint8 code = *src++;
		if (!(code & 0x80)) { // 8th bit isn't set
			int len = MIN(count, (code >> 4) + 3); //upper half of code is the length
			int offs = ((code & 0xF) << 8) | *src++; //lower half of code as byte 2 of offset.
			const uint8 *dstOffs = dst - offs;
			while (len--)
				*dst++ = *dstOffs++;
		} else if (code & 0x40) { // 7th bit is set
			int len = (code & 0x3F) + 3;
			if (code == 0xFE) {
				len = READ_LE_UINT16(src); src += 2;
				if (len > count)
					len = count;

				memset(dst, *src++, len); dst += len;
			} else {
				if (code == 0xFF) {
					len = READ_LE_UINT16(src);
					src += 2;
				}

				int offs = READ_LE_UINT16(src); src += 2;
				if (len > count)
					len = count;

				const uint8 *dstOffs = dstOrig + offs;
				while (len--)
					*dst++ = *dstOffs++;
			}
		} else if (code != 0x80) { // not just the 8th bit set.
			//Copy some bytes from source to dest.
			int len = MIN(count, code & 0x3F);
			while (len--)
				*dst++ = *src++;
		} else {
			break;
		}
	}
	return dst - dstOrig;
}

void Screen::decodeFrameDelta(uint8 *dst, const uint8 *src, bool noXor) {
	if (noXor)
		wrapped_decodeFrameDelta<true>(dst, src);
	else
		wrapped_decodeFrameDelta<false>(dst, src);
}

template<bool noXor>
void Screen::wrapped_decodeFrameDelta(uint8 *dst, const uint8 *src) {
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				if (noXor)
					*dst++ = code;
				else
					*dst++ ^= code;
			}
		} else if (code & 0x80) {
			code -= 0x80;
			if (code != 0) {
				dst += code;
			} else {
				uint16 subcode = READ_LE_UINT16(src); src += 2;
				if (subcode == 0) {
					break;
				} else if (subcode & 0x8000) {
					subcode -= 0x8000;
					if (subcode & 0x4000) {
						uint16 len = subcode - 0x4000;
						code = *src++;
						while (len--) {
							if (noXor)
								*dst++ = code;
							else
								*dst++ ^= code;
						}
					} else {
						while (subcode--) {
							if (noXor)
								*dst++ = *src++;
							else
								*dst++ ^= *src++;
						}
					}
				} else {
					dst += subcode;
				}
			}
		} else {
			while (code--) {
				if (noXor)
					*dst++ = *src++;
				else
					*dst++ ^= *src++;
			}
		}
	}
}

void Screen::decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch, bool noXor) {
	if (noXor)
		wrapped_decodeFrameDeltaPage<true>(dst, src, pitch);
	else
		wrapped_decodeFrameDeltaPage<false>(dst, src, pitch);
}

void Screen::convertAmigaGfx(uint8 *data, int w, int h, int depth, bool wsa, int bytesPerPlane) {
	const int planeWidth = (bytesPerPlane == -1) ? (w + 7) / 8 : bytesPerPlane;
	const int planeSize = planeWidth * h;
	const uint imageSize = planeSize * depth;

	// Our static buffer which holds the plane data. We need this
	// because the "data" pointer is both source and destination pointer.
	// The buffer has enough space to fit the AMIGA MSC files, which are
	// the biggest graphics files found in the AMIGA version.
	static uint8 temp[40320];
	assert(imageSize <= sizeof(temp));

	// WSA files store their graphics data in a little different format, than
	// the usual AMIGA graphics format used in BitMaps. Thus we need to do
	// some special handling for them here. Means we convert them into
	// the usual format.
	//
	// TODO: We might think of moving this conversion into the WSAMovieAmiga
	// class.
	if (wsa) {
		const byte *src = data;
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < planeWidth; ++x)
				for (int i = 0; i < depth; ++i)
					temp[y * planeWidth + x + planeSize * i] = *src++;
		}
	} else {
		memcpy(temp, data, imageSize);
	}

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			const int bytePos = x / 8 + y * planeWidth;
			const int bitPos = 7 - (x & 7); // x & 7 == x % 8

			byte col = 0;

			for (int i = 0; i < depth; ++i)
				col |= ((temp[bytePos + planeSize * i] >> bitPos) & 1) << i;

			*data++ = col;
		}
	}
}

void Screen::convertAmigaMsc(uint8 *data) {
	// MSC files are always 320x144, thus we can safely assume
	// this to be correct. Also they contain 7 planes instead
	// of the normal 5 planes, which is used in 32 color mode.
	// The need for 7 planes can be explained, because the MSC
	// files have 6 bits for the layer number (bits 1 to 6)
	// and one bit for the "blocked" flag (bit 0), and every
	// plane contains one bit per pixel.
	convertAmigaGfx(data, 320, 144, 7);

	// We need to do some post conversion, since
	// the AMIGA MSC format is different from the DOS
	// one we use internally for our code.That is even
	// after converting it from the AMIGA plane based
	// approach to one byte per pixel approach.
	for (int i = 0; i < 320 * 144; ++i) {
		// The lowest bit indicates, whether the position
		// is walkable or not. If the bit is set, the
		// position is walkable, elsewise it is blocked.
		if (data[i] & 1)
			data[i] &= 0xFE;
		else
			data[i] |= 0x80;

		// The graphics layer for the pixel is saved
		// in the following format:
		// The highest bit set indicates the number of
		// the graphics layer. We count the first
		// bit as 0 here, thus we need to add one,
		// to get the correct number.
		//
		// Funnily since the first bit (bit 0) is
		// resevered for testing whether the position
		// is walkable or not, there is no possibility
		// for layer 1 to be present.
		int layer = 0;
		for (int k = 0; k < 7; ++k)
			if (data[i] & (1 << k))
				layer = k + 1;

		data[i] &= 0x80;
		data[i] |= layer;
	}
}

template<bool noXor>
void Screen::wrapped_decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch) {
	int count = 0;
	uint8 *dstNext = dst;
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				if (noXor)
					*dst++ = code;
				else
					*dst++ ^= code;

				if (++count == pitch) {
					count = 0;
					dstNext += SCREEN_W;
					dst = dstNext;
				}
			}
		} else if (code & 0x80) {
			code -= 0x80;
			if (code != 0) {
				dst += code;

				count += code;
				while (count >= pitch) {
					count -= pitch;
					dstNext += SCREEN_W;
					dst = dstNext + count;
				}
			} else {
				uint16 subcode = READ_LE_UINT16(src); src += 2;
				if (subcode == 0) {
					break;
				} else if (subcode & 0x8000) {
					subcode -= 0x8000;
					if (subcode & 0x4000) {
						uint16 len = subcode - 0x4000;
						code = *src++;
						while (len--) {
							if (noXor)
								*dst++ = code;
							else
								*dst++ ^= code;

							if (++count == pitch) {
								count = 0;
								dstNext += SCREEN_W;
								dst = dstNext;
							}
						}
					} else {
						while (subcode--) {
							if (noXor)
								*dst++ = *src++;
							else
								*dst++ ^= *src++;

							if (++count == pitch) {
								count = 0;
								dstNext += SCREEN_W;
								dst = dstNext;
							}
						}
					}
				} else {
					dst += subcode;

					count += subcode;
					while (count >= pitch) {
						count -= pitch;
						dstNext += SCREEN_W;
						dst = dstNext + count;
					}

				}
			}
		} else {
			while (code--) {
				if (noXor)
					*dst++ = *src++;
				else
					*dst++ ^= *src++;

				if (++count == pitch) {
					count = 0;
					dstNext += SCREEN_W;
					dst = dstNext;
				}
			}
		}
	}
}

uint8 *Screen::encodeShape(int x, int y, int w, int h, int flags) {
	uint8 *srcPtr = &_pagePtrs[_curPage][y * SCREEN_W + x];
	int16 shapeSize = 0;
	uint8 *tmp = srcPtr;
	int xpos = w;

	for (int i = h; i > 0; --i) {
		uint8 *start = tmp;
		shapeSize += w;
		xpos = w;
		while (xpos) {
			uint8 value = *tmp++;
			--xpos;

			if (!value) {
				shapeSize += 2;
				int16 curX = xpos;
				bool skip = false;

				while (xpos) {
					value = *tmp++;
					--xpos;

					if (value) {
						skip = true;
						break;
					}
				}

				if (!skip)
					++curX;

				curX -= xpos;
				shapeSize -= curX;

				while (curX > 0xFF) {
					curX -= 0xFF;
					shapeSize += 2;
				}
			}
		}

		tmp = start + SCREEN_W;
	}

	int16 shapeSize2 = shapeSize;
	if (_vm->gameFlags().useAltShapeHeader)
		shapeSize += 12;
	else
		shapeSize += 10;

	if (flags & 1)
		shapeSize += 16;

	uint8 table[274];
	int tableIndex = 0;

	uint8 *newShape = nullptr;
	newShape = new uint8[shapeSize+16];
	assert(newShape);

	byte *dst = newShape;

	if (_vm->gameFlags().useAltShapeHeader)
		dst += 2;

	WRITE_LE_UINT16(dst, (flags & 3)); dst += 2;
	*dst = h; dst += 1;
	WRITE_LE_UINT16(dst, w); dst += 2;
	*dst = h; dst += 1;
	WRITE_LE_UINT16(dst, shapeSize); dst += 2;
	WRITE_LE_UINT16(dst, shapeSize2); dst += 2;

	byte *src = srcPtr;
	if (flags & 1) {
		dst += 16;
		memset(table, 0, sizeof(table));
		tableIndex = 1;
	}

	for (int ypos = h; ypos > 0; --ypos) {
		uint8 *srcBackUp = src;
		xpos = w;
		while (xpos) {
			uint8 value = *src++;
			if (value) {
				if (flags & 1) {
					if (!table[value]) {
						if (tableIndex == 16) {
							value = 1;
						} else {
							table[0x100+tableIndex] = value;
							table[value] = tableIndex;
							++tableIndex;
							value = table[value];
						}
					} else {
						value = table[value];
					}
				}
				--xpos;
				*dst++ = value;
			} else {
				int16 temp = 1;
				--xpos;

				while (xpos) {
					if (*src)
						break;
					++src;
					++temp;
					--xpos;
				}

				while (temp > 0xFF) {
					*dst++ = 0;
					*dst++ = 0xFF;
					temp -= 0xFF;
				}

				if (temp & 0xFF) {
					*dst++ = 0;
					*dst++ = temp & 0xFF;
				}
			}
		}
		src = srcBackUp + SCREEN_W;
	}

	if (!(flags & 2)) {
		if (shapeSize > _animBlockSize) {
			dst = newShape;
			if (_vm->gameFlags().useAltShapeHeader)
				dst += 2;

			flags = READ_LE_UINT16(dst);
			flags |= 2;
			WRITE_LE_UINT16(dst, flags);
		} else {
			src = newShape;
			if (_vm->gameFlags().useAltShapeHeader)
				src += 2;
			if (flags & 1)
				src += 16;

			src += 10;
			uint8 *shapePtrBackUp = src;
			dst = _animBlockPtr;
			memcpy(dst, src, shapeSize2);

			int16 size = encodeShapeAndCalculateSize(_animBlockPtr, shapePtrBackUp, shapeSize2);
			if (size > shapeSize2) {
				shapeSize -= shapeSize2 - size;
				uint8 *newShape2 = new uint8[shapeSize];
				assert(newShape2);
				memcpy(newShape2, newShape, shapeSize);
				delete[] newShape;
				newShape = newShape2;
			} else {
				dst = shapePtrBackUp;
				src = _animBlockPtr;
				memcpy(dst, src, shapeSize2);
				dst = newShape;
				if (_vm->gameFlags().useAltShapeHeader)
					dst += 2;
				flags = READ_LE_UINT16(dst);
				flags |= 2;
				WRITE_LE_UINT16(dst, flags);
			}
		}
	}

	dst = newShape;
	if (_vm->gameFlags().useAltShapeHeader)
		dst += 2;
	WRITE_LE_UINT16((dst + 6), shapeSize);

	if (flags & 1) {
		dst = newShape + 10;
		if (_vm->gameFlags().useAltShapeHeader)
			dst += 2;
		src = &table[0x100];
		memcpy(dst, src, sizeof(uint8)*16);
	}

	return newShape;
}

int16 Screen::encodeShapeAndCalculateSize(uint8 *from, uint8 *to, int size_to) {
	byte *fromPtrEnd = from + size_to;
	bool skipPixel = true;
	byte *tempPtr = nullptr;
	byte *toPtr = to;
	byte *fromPtr = from;
	byte *toPtr2 = to;

	*to++ = 0x81;
	*to++ = *from++;

	while (from < fromPtrEnd) {
		byte *curToPtr = to;
		to = fromPtr;
		int size = 1;

		while (true) {
			byte curPixel = *from;
			if (curPixel == *(from+0x40)) {
				byte *toBackUp = to;
				to = from;

				for (int i = 0; i < (fromPtrEnd - from); ++i) {
					if (*to++ != curPixel)
						break;
				}
				--to;
				uint16 diffSize = (to - from);
				if (diffSize >= 0x41) {
					skipPixel = false;
					from = to;
					to = curToPtr;
					*to++ = 0xFE;
					WRITE_LE_UINT16(to, diffSize); to += 2;
					*to++ = curPixel;
					curToPtr = to;
					to = toBackUp;
					continue;
				} else {
					to = toBackUp;
				}
			}

			bool breakLoop = false;
			while (true) {
				if ((from - to) == 0) {
					breakLoop = true;
					break;
				}
				for (int i = 0; i < (from - to); ++i) {
					if (*to++ == curPixel)
						break;
				}
				if (*(to-1) == curPixel) {
					if (*(from+size-1) != *(to+size-2))
						continue;

					byte *fromBackUp = from;
					byte *toBackUp = to;
					--to;
					const int checkSize = fromPtrEnd - from;
					for (int i = 0; i < checkSize; ++i) {
						if (*from++ != *to++)
							break;
					}
					if (*(from - 1) == *(to - 1))
						++to;
					from = fromBackUp;
					int temp = to - toBackUp;
					to = toBackUp;
					if (temp >= size) {
						size = temp;
						tempPtr = toBackUp - 1;
					}
					break;
				} else {
					breakLoop = true;
					break;
				}
			}

			if (breakLoop)
				break;
		}

		to = curToPtr;
		if (size > 2) {
			uint16 word = 0;
			if (size <= 0x0A) {
				uint16 diffSize = from - tempPtr;
				if (diffSize <= 0x0FFF) {
					byte highByte = ((diffSize & 0xFF00) >> 8) + (((size & 0xFF) - 3) << 4);
					word = ((diffSize & 0xFF) << 8) | highByte;
					WRITE_LE_UINT16(to, word); to += 2;
					from += size;
					skipPixel = false;
					continue;
				}
			}

			if (size > 0x40) {
				*to++ = 0xFF;
				WRITE_LE_UINT16(to, size); to += 2;
			} else {
				*to++ = ((size & 0xFF) - 3) | 0xC0;
			}

			word = tempPtr - fromPtr;
			WRITE_LE_UINT16(to, word); to += 2;
			from += size;
			skipPixel = false;
		} else {
			if (!skipPixel) {
				toPtr2 = to;
				*to++ = 0x80;
			}

			if (*toPtr2 == 0xBF) {
				toPtr2 = to;
				*to++ = 0x80;
			}

			++(*toPtr2);
			*to++ = *from++;
			skipPixel = true;
		}
	}
	*to++ = 0x80;

	return (to - toPtr);
}

uint16 Screen::shade16bitColor(uint16 col) {
	uint8 r = (col & 0x1f);
	uint8 g = (col & 0x3E0) >> 5;
	uint8 b = (col & 0x7C00) >> 10;

	r = (r > _16bitShadingLevel) ? r - _16bitShadingLevel : 0;
	g = (g > _16bitShadingLevel) ? g - _16bitShadingLevel : 0;
	b = (b > _16bitShadingLevel) ? b - _16bitShadingLevel : 0;

	return (b << 10) | (g << 5) | r;
}

void Screen::hideMouse() {
	++_mouseLockCount;
	CursorMan.showMouse(false);
}

void Screen::showMouse() {
	if (_mouseLockCount == 1) {
		CursorMan.showMouse(true);

		// We need to call OSystem::updateScreen here, else the mouse cursor
		// will only be visible on mouse movement.
		updateBackendScreen(true);
	}

	if (_mouseLockCount > 0)
		_mouseLockCount--;
}


bool Screen::isMouseVisible() const {
	return _mouseLockCount == 0;
}

void Screen::setShapePages(int page1, int page2, int minY, int maxY) {
	_shapePages[0] = _pagePtrs[page1];
	_shapePages[1] = _pagePtrs[page2];
	_maskMinY = minY;
	_maskMaxY = maxY;
}

void Screen::setMouseCursor(int x, int y, const byte *shape) {
	if (!shape)
		return;

	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;

	int mouseHeight = *(shape + 2);
	int mouseWidth = (READ_LE_UINT16(shape + 3)) + 2;

	if (_vm->gameFlags().useAltShapeHeader)
		shape -= 2;

	if (_vm->gameFlags().useHiRes) {
		x <<= 1;
		y <<= 1;
		mouseWidth <<= 1;
		mouseHeight <<= 1;
	}

	uint8 *cursor = new uint8[mouseHeight * mouseWidth];
	fillRect(0, 0, mouseWidth, mouseHeight, _cursorColorKey, 8);
	drawShape(8, shape, 0, 0, 0, 0);

	int xOffset = 0;

	if (_vm->gameFlags().useHiRes) {
		xOffset = mouseWidth;
		scale2x<uint8, uint16>(getPagePtr(8) + mouseWidth, SCREEN_W, getPagePtr(8), SCREEN_W, mouseWidth, mouseHeight);
		postProcessCursor(getPagePtr(8) + mouseWidth, mouseWidth, mouseHeight, SCREEN_W);
	} else {
		postProcessCursor(getPagePtr(8), mouseWidth, mouseHeight, SCREEN_W);
	}

	CursorMan.showMouse(false);
	copyRegionToBuffer(8, xOffset, 0, mouseWidth, mouseHeight, cursor);
	CursorMan.replaceCursor(cursor, mouseWidth, mouseHeight, x, y, _cursorColorKey);
	if (isMouseVisible())
		CursorMan.showMouse(true);
	delete[] cursor;

	// makes sure that the cursor is drawn
	// we do not use Screen::updateScreen here
	// so we can be sure that changes to page 0
	// are NOT updated on the real screen here
	updateBackendScreen(true);
}

Palette &Screen::getPalette(int num) {
	assert(num >= 0 && (uint)num < _palettes.size());
	return *_palettes[num];
}

void Screen::copyPalette(const int dst, const int src) {
	getPalette(dst).copy(getPalette(src));
}

byte Screen::getShapeFlag1(int x, int y) {
	uint8 color = _shapePages[0][y * SCREEN_W + x];
	color &= 0x80;
	color ^= 0x80;

	if (color & 0x80)
		return 1;
	return 0;
}

byte Screen::getShapeFlag2(int x, int y) {
	uint8 color = _shapePages[0][y * SCREEN_W + x];
	color &= 0x7F;
	color &= 0x87;
	return color;
}

int Screen::getDrawLayer(int x, int y) {
	int xpos = x - 8;
	int ypos = y - 1;
	int layer = 1;

	for (int curX = xpos; curX < xpos + 16; ++curX) {
		int tempLayer = getShapeFlag2(curX, ypos);

		if (layer < tempLayer)
			layer = tempLayer;

		if (layer >= 7)
			return 7;
	}
	return layer;
}

int Screen::getDrawLayer2(int x, int y, int height) {
	int xpos = x - 8;
	int ypos = y - 1;
	int layer = 1;

	for (int useX = xpos; useX < xpos + 16; ++useX) {
		for (int useY = ypos - height; useY < ypos; ++useY) {
			int tempLayer = getShapeFlag2(useX, useY);

			if (tempLayer > layer)
				layer = tempLayer;

			if (tempLayer >= 7)
				return 7;
		}
	}
	return layer;
}


int Screen::setNewShapeHeight(uint8 *shape, int height) {
	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;

	int oldHeight = shape[2];
	shape[2] = height;
	return oldHeight;
}

int Screen::resetShapeHeight(uint8 *shape) {
	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;

	int oldHeight = shape[2];
	shape[2] = shape[5];
	return oldHeight;
}

void Screen::blockInRegion(int x, int y, int width, int height) {
	assert(_shapePages[0]);
	byte *toPtr = _shapePages[0] + (y * 320 + x);
	for (int i = 0; i < height; ++i) {
		byte *backUpTo = toPtr;
		for (int i2 = 0; i2 < width; ++i2)
			*toPtr++ &= 0x7F;
		toPtr = (backUpTo + 320);
	}
}

void Screen::blockOutRegion(int x, int y, int width, int height) {
	assert(_shapePages[0]);
	byte *toPtr = _shapePages[0] + (y * 320 + x);
	for (int i = 0; i < height; ++i) {
		byte *backUpTo = toPtr;
		for (int i2 = 0; i2 < width; ++i2)
			*toPtr++ |= 0x80;
		toPtr = (backUpTo + 320);
	}
}

void Screen::rectClip(int &x, int &y, int w, int h) {
	if (x < 0)
		x = 0;
	else if (x + w >= 320)
		x = 320 - w;

	if (y < 0)
		y = 0;
	else if (y + h >= 200)
		y = 200 - h;
}

void Screen::shakeScreen(int times) {
	static const int8 _shakeParaPC[] = { 32, 0, -4, 32, 0, 0 };
	static const int8 _shakeParaFMTOWNS[] = { 32, 0, -4, 48, 0, 4, 32, -4, 0, 32, 4, 0, 32, 0, 0 };

	const int8 *shakeData = _shakeParaPC;
	int steps = ARRAYSIZE(_shakeParaPC) / 3;

	// The FM-TOWNS version has a slightly better shake animation
	// TODO: check PC-98 version
	if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		shakeData = _shakeParaFMTOWNS;
		steps = ARRAYSIZE(_shakeParaFMTOWNS) / 3;
	}

	Common::Event event;

	while (times--) {
		const int8 *data = shakeData;
		for (int i = 0; i < steps; ++i) {
			// The original PC version did not need an artificial delay, but we do or the shake will be
			// too fast to be actually seen.
			uint32 end = _system->getMillis() + data[0];
			_system->setShakePos(data[1], data[2]);

			for (uint32 now = _system->getMillis(); now < end; ) {
				// Update the event manager to keep smooth mouse pointer movement.
				while (_vm->getEventManager()->pollEvent(event)) {
					if (event.type == Common::EVENT_KEYDOWN) {
						// This is really the only thing that should be handled.
						if (event.kbd.keycode == Common::KEYCODE_q && event.kbd.hasFlags(Common::KBD_CTRL))
							_vm->quitGame();
					}
				}
				updateBackendScreen(true);
				now = _system->getMillis();
				_system->delayMillis(MIN<uint>(end - now, 10));
			}
			data += 3;
		}
	}
}

void Screen::loadBitmap(const char *filename, int tempPage, int dstPage, Palette *pal, bool skip) {
	uint32 fileSize;
	uint8 *srcData = _vm->resource()->fileData(filename, &fileSize);

	if (!srcData) {
		warning("couldn't load bitmap: '%s'", filename);
		return;
	}

	if (skip)
		srcData += 4;

	const char *ext = filename + strlen(filename) - 3;
	uint8 compType = srcData[2];
	uint32 imgSize = (_vm->game() == GI_KYRA2 && !scumm_stricmp(ext, "CMP")) ? READ_LE_UINT16(srcData) : READ_LE_UINT32(srcData + 4);
	uint16 palSize = READ_LE_UINT16(srcData + 8);

	if (pal && palSize)
		loadPalette(srcData + 10, *pal, palSize);

	uint8 *srcPtr = srcData + 10 + palSize;
	uint8 *dstData = getPagePtr(dstPage);
	memset(dstData, 0, _screenPageSize);
	if (dstPage == 0 || tempPage == 0)
		_forceFullUpdate = true;

	switch (compType) {
	case 0:
		memcpy(dstData, srcPtr, imgSize);
		break;
	case 1:
		Screen::decodeFrame1(srcPtr, dstData, imgSize);
		break;
	case 3:
		Screen::decodeFrame3(srcPtr, dstData, imgSize, _isAmiga);
		break;
	case 4:
		Screen::decodeFrame4(srcPtr, dstData, imgSize);
		break;
	default:
		error("Unhandled bitmap compression %d", compType);
	}

	if (skip)
		srcData -= 4;

	delete[] srcData;
}

bool Screen::loadPalette(const char *filename, Palette &pal) {
	if (_renderMode == Common::kRenderCGA)
		return true;

	Common::SeekableReadStream *stream = _vm->resource()->createReadStream(filename);

	if (!stream)
		return false;

	debugC(3, kDebugLevelScreen, "Screen::loadPalette('%s', %p)", filename, (const void *)&pal);

	const int maxCols = _16bitPalette ? 256 : pal.getNumColors();
	int numCols = 0;

	if (_isAmiga) {
		numCols = stream->size() / Palette::kAmigaBytesPerColor;
		pal.loadAmigaPalette(*stream, 0, MIN(maxCols, numCols));
	} else if (_vm->gameFlags().platform == Common::kPlatformPC98 && _use16ColorMode) {
		numCols = stream->size() / Palette::kPC98BytesPerColor;
		pal.loadPC98Palette(*stream, 0, MIN(maxCols, numCols));
	} else if (_renderMode == Common::kRenderEGA) {
		numCols = stream->size();
		// There aren't any 16 color EGA palette files. So this shouldn't ever get triggered.
		assert (numCols != 16);
		numCols /= Palette::kVGABytesPerColor;
		pal.loadVGAPalette(*stream, 0, numCols);
	} else {
		if (_bytesPerPixel == 2) {
			numCols = stream->size() / 2;
			pal.loadHiColorPalette(*stream, 0, numCols);
		} else if (!_16bitPalette) {
			numCols = stream->size() / Palette::kVGABytesPerColor;
			pal.loadVGAPalette(*stream, 0, MIN(maxCols, numCols));
		} else {
			error("Screen::loadPalette(): Failed to load file '%s' with invalid size %d in HiColor mode", filename, (int)stream->size());
		}
	}

	if (numCols > maxCols)
		warning("Palette file '%s' includes %d colors, but the target palette only support %d colors", filename, numCols, maxCols);

	delete stream;
	return true;
}

bool Screen::loadPaletteTable(const char *filename, int firstPalette) {
	Common::SeekableReadStream *stream = _vm->resource()->createReadStream(filename);

	if (!stream)
		return false;

	debugC(3, kDebugLevelScreen, "Screen::loadPaletteTable('%s', %d)", filename, firstPalette);

	if (_isAmiga) {
		const int numColors = getPalette(firstPalette).getNumColors();
		const int palSize = getPalette(firstPalette).getNumColors() * Palette::kAmigaBytesPerColor;
		const int numPals = stream->size() / palSize;

		for (int i = 0; i < numPals; ++i)
			getPalette(i + firstPalette).loadAmigaPalette(*stream, 0, numColors);
	} else {
		const int numColors = getPalette(firstPalette).getNumColors();
		const int palSize = getPalette(firstPalette).getNumColors() * Palette::kVGABytesPerColor;
		const int numPals = stream->size() / palSize;

		for (int i = 0; i < numPals; ++i)
			getPalette(i + firstPalette).loadVGAPalette(*stream, 0, numColors);
	}

	delete stream;
	return true;
}

void Screen::loadPalette(const byte *data, Palette &pal, int bytes) {
	Common::MemoryReadStream stream(data, bytes, DisposeAfterUse::NO);

	if (_isAmiga) {
		// EOB II Amiga sometimes has multiple palettes here one after
		// the other (64 bytes each). We only load the first one here.
		pal.loadAmigaPalette(stream, 0, MIN<int>(32, stream.size() / Palette::kAmigaBytesPerColor));
	} else if (_vm->gameFlags().platform == Common::kPlatformPC98 && _use16ColorMode) {
		pal.loadPC98Palette(stream, 0, stream.size() / Palette::kPC98BytesPerColor);
	} else if (_renderMode == Common::kRenderEGA) {
		// EOB II checks the number of palette bytes to distinguish between real EGA palettes
		// and normal palettes (which are used to generate a color map).
		if (stream.size() == 16)
			pal.loadEGAPalette(stream, 0, stream.size());
		else
			pal.loadVGAPalette(stream, 0, stream.size() / Palette::kVGABytesPerColor);
	} else
		pal.loadVGAPalette(stream, 0, stream.size() / Palette::kVGABytesPerColor);
}

// dirty rect handling

void Screen::addDirtyRect(int x, int y, int w, int h) {
	if (_dirtyRects.size() >= kMaxDirtyRects || _forceFullUpdate) {
		_forceFullUpdate = true;
		return;
	}

	Common::Rect r(x, y, x + w, y + h);

	// Clip rectangle
	r.clip(SCREEN_W, _screenHeight - _yTransOffs);

	// If it is empty after clipping, we are done
	if (r.isEmpty())
		return;

	// Check if the new rectangle is contained within another in the list
	Common::List<Common::Rect>::iterator it;
	for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ) {
		// If we find a rectangle which fully contains the new one,
		// we can abort the search.
		if (it->contains(r))
			return;

		// Conversely, if we find rectangles which are contained in
		// the new one, we can remove them
		if (r.contains(*it))
			it = _dirtyRects.erase(it);
		else
			++it;
	}

	// If we got here, we can safely add r to the list of dirty rects.
	_dirtyRects.push_back(r);
}

// overlay functions

byte *Screen::getOverlayPtr(int page) {
	if (page == 0 || page == 1)
		return _sjisOverlayPtrs[1];
	else if (page == 2 || page == 3)
		return _sjisOverlayPtrs[2];

	if (_vm->game() == GI_KYRA2) {
		if (page == 12 || page == 13)
			return _sjisOverlayPtrs[3];
	} else if (_vm->game() == GI_LOL) {
		if (page == 4 || page == 5)
			return _sjisOverlayPtrs[3];
		if (page == 6 || page == 7)
			return _sjisOverlayPtrs[4];
		if (page == 12 || page == 13)
			return _sjisOverlayPtrs[5];
	}

	return nullptr;
}

void Screen::clearOverlayPage(int page) {
	byte *dst = getOverlayPtr(page);
	if (!dst)
		return;
	memset(dst, _sjisInvisibleColor, SCREEN_OVL_SJIS_SIZE);
}

void Screen::clearOverlayRect(int page, int x, int y, int w, int h) {
	byte *dst = getOverlayPtr(page);

	if (!dst || w < 0 || h < 0)
		return;

	x <<= 1; y <<= 1;
	w <<= 1; h <<= 1;

	dst += y * 640 + x;

	if (w == 640 && h == 400) {
		memset(dst, _sjisInvisibleColor, SCREEN_OVL_SJIS_SIZE);
	} else {
		while (h--) {
			memset(dst, _sjisInvisibleColor, w);
			dst += 640;
		}
	}
}

void Screen::copyOverlayRegion(int x, int y, int x2, int y2, int w, int h, int srcPage, int dstPage) {
	byte *dst = getOverlayPtr(dstPage);
	const byte *src = getOverlayPtr(srcPage);

	if (!dst || !src)
		return;

	x <<= 1; x2 <<= 1;
	y <<= 1; y2 <<= 1;
	w <<= 1; h <<= 1;

	if (w == 640 && h == 400) {
		memcpy(dst, src, SCREEN_OVL_SJIS_SIZE);
	} else {
		dst += y2 * 640 + x2;
		src += y * 640 + x;

		while (h--) {
			for (x = 0; x < w; ++x)
				memmove(dst, src, w);
			dst += 640;
			src += 640;
		}
	}
}

void Screen::crossFadeRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage) {
	if (srcPage > 13 || dstPage > 13)
		error("Screen::crossFadeRegion(): attempting to use temp page as source or dest page.");

	hideMouse();

	uint16 *wB = (uint16 *)_pagePtrs[14];
	uint8 *hB = _pagePtrs[14] + 640 * _bytesPerPixel;

	for (int i = 0; i < w; i++)
		wB[i] = i;

	for (int i = 0; i < h; i++)
		hB[i] = i;

	for (int i = 0; i < w; i++)
		SWAP(wB[_vm->_rnd.getRandomNumberRng(0, w - 1)], wB[i]);

	for (int i = 0; i < h; i++)
		SWAP(hB[_vm->_rnd.getRandomNumberRng(0, h - 1)], hB[i]);

	uint8 *s = _pagePtrs[srcPage];
	uint8 *d = _pagePtrs[dstPage];

	for (int i = 0; i < h; i++) {
		int iH = i;
		uint32 end = _system->getMillis() + 3;
		for (int ii = 0; ii < w; ii++) {
			int sX = (x1 + wB[ii]);
			int sY = (y1 + hB[iH]);
			int dX = (x2 + wB[ii]);
			int dY = (y2 + hB[iH]);

			if (++iH >= h)
				iH = 0;

			if (_bytesPerPixel == 2)
				((uint16*)d)[dY * 320 + dX] = ((uint16*)s)[sY * 320 + sX];
			else
				d[dY * 320 + dX] = s[sY * 320 + sX];
			addDirtyRect(dX, dY, 1, 1);
		}

		// This tries to speed things up, to get similar speeds as in DOSBox etc.
		// We can't write single pixels directly into the video memory like the original did.
		// We also (unlike the original) want to aim at similar speeds for all platforms.
		if (!(i % 10))
			updateScreen();

		uint32 cur = _system->getMillis();
		if (end > cur)
			_system->delayMillis(end - cur);
	}

	updateScreen();
	showMouse();
}

#pragma mark -

DOSFont::DOSFont() {
	_data = _widthTable = _heightTable = nullptr;
	_colorMap = nullptr;
	_width = _height = _numGlyphs = 0;
	_bitmapOffsets = nullptr;
}

bool DOSFont::load(Common::SeekableReadStream &file) {
	unload();

	_data = new uint8[file.size()];
	assert(_data);

	file.read(_data, file.size());
	if (file.err())
		return false;

	const uint16 fontSig = READ_LE_UINT16(_data + 2);

	if (fontSig != 0x0500) {
		warning("DOSFont: invalid font: %.04X)", fontSig);
		return false;
	}

	const uint16 descOffset = READ_LE_UINT16(_data + 4);

	_width = _data[descOffset + 5];
	_height = _data[descOffset + 4];
	_numGlyphs = _data[descOffset + 3] + 1;

	_bitmapOffsets = (uint16 *)(_data + READ_LE_UINT16(_data + 6));
	_widthTable = _data + READ_LE_UINT16(_data + 8);
	_heightTable = _data + READ_LE_UINT16(_data + 12);

	for (int i = 0; i < _numGlyphs; ++i)
		_bitmapOffsets[i] = READ_LE_UINT16(&_bitmapOffsets[i]);

	return true;
}

int DOSFont::getCharWidth(uint16 c) const {
	if (c >= _numGlyphs)
		return 0;
	return _widthTable[c];
}

void DOSFont::drawChar(uint16 c, byte *dst, int pitch, int) const {
	if (c >= _numGlyphs)
		return;

	if (!_bitmapOffsets[c])
		return;

	const uint8 *src = _data + _bitmapOffsets[c];
	const uint8 charWidth = _widthTable[c];

	if (!charWidth)
		return;

	pitch -= charWidth;

	uint8 charH1 = _heightTable[c * 2 + 0];
	uint8 charH2 = _heightTable[c * 2 + 1];
	uint8 charH0 = _height - (charH1 + charH2);

	while (charH1--) {
		uint8 col = _colorMap[0];
		for (int i = 0; i < charWidth; ++i) {
			if (col != 0)
				*dst = col;
			++dst;
		}
		dst += pitch;
	}

	while (charH2--) {
		uint8 b = 0;
		for (int i = 0; i < charWidth; ++i) {
			uint8 col;
			if (i & 1) {
				col = _colorMap[b >> 4];
			} else {
				b = *src++;
				col = _colorMap[b & 0xF];
			}
			if (col != 0) {
				*dst = col;
			}
			++dst;
		}
		dst += pitch;
	}

	while (charH0--) {
		uint8 col = _colorMap[0];
		for (int i = 0; i < charWidth; ++i) {
			if (col != 0)
				*dst = col;
			++dst;
		}
		dst += pitch;
	}
}

void DOSFont::unload() {
	delete[] _data;
	_data = _widthTable = _heightTable = nullptr;
	_colorMap = nullptr;
	_width = _height = _numGlyphs = 0;
	_bitmapOffsets = nullptr;
}


AMIGAFont::AMIGAFont() {
	_width = _height = 0;
	memset(_chars, 0, sizeof(_chars));
}

bool AMIGAFont::load(Common::SeekableReadStream &file) {
	const uint16 dataSize = file.readUint16BE();
	if (dataSize + 2 != file.size())
		return false;

	_width = file.readByte();
	_height = file.readByte();

	// Read the character definition offset table
	uint16 offsets[ARRAYSIZE(_chars)];
	for (int i = 0; i < ARRAYSIZE(_chars); ++i)
		offsets[i] = file.readUint16BE() + 4;

	if (file.err())
		return false;

	for (int i = 0; i < ARRAYSIZE(_chars); ++i) {
		file.seek(offsets[i], SEEK_SET);

		_chars[i].yOffset = file.readByte();
		_chars[i].xOffset = file.readByte();
		_chars[i].width = file.readByte();
		file.readByte(); // unused

		// If the y offset is 255, then the character
		// does not have any bitmap representation
		if (_chars[i].yOffset != 255) {
			Character::Graphics &g = _chars[i].graphics;

			g.width = file.readUint16BE();
			g.height = file.readUint16BE();

			int depth = file.readByte();
			int specialWidth = file.readByte();
			int flags = file.readByte();
			int bytesPerPlane = file.readByte();

			assert(depth != 0 && specialWidth == 0 && flags == 0 && bytesPerPlane != 0);

			// Allocate a temporary buffer to store the plane data
			const int planesSize = bytesPerPlane * g.height * depth;
			uint8 *tempData = new uint8[MAX(g.width * g.height, planesSize)];
			assert(tempData);

			file.read(tempData, planesSize);

			// Convert the plane based graphics to our graphic format
			Screen::convertAmigaGfx(tempData, g.width, g.height, depth, false, bytesPerPlane);

			// Create a buffer perfectly fitting the character
			g.bitmap = new uint8[g.width * g.height];
			assert(g.bitmap);

			memcpy(g.bitmap, tempData, g.width * g.height);
			delete[] tempData;
		}

		if (file.err())
			return false;
	}

	return !file.err();
}

int AMIGAFont::getCharWidth(uint16 c) const {
	if (c >= 255)
		return 0;
	return _chars[c].width;
}

void AMIGAFont::drawChar(uint16 c, byte *dst, int pitch, int) const {
	if (c >= 255)
		return;

	if (_chars[c].yOffset == 255)
		return;

	dst += _chars[c].yOffset * pitch;
	dst += _chars[c].xOffset;

	pitch -= _chars[c].graphics.width;

	const uint8 *src = _chars[c].graphics.bitmap;
	assert(src);

	for (int y = 0; y < _chars[c].graphics.height; ++y) {
		for (int x = 0; x < _chars[c].graphics.width; ++x) {
			if (*src)
				*dst = *src;
			++src;
			++dst;
		}

		dst += pitch;
	}
}

void AMIGAFont::unload() {
	_width = _height = 0;
	for (int i = 0; i < ARRAYSIZE(_chars); ++i)
		delete[] _chars[i].graphics.bitmap;
	memset(_chars, 0, sizeof(_chars));
}

SJISFont::SJISFont(Common::SharedPtr<Graphics::FontSJIS> &font, const uint8 invisColor, bool is16Color, bool drawOutline, int extraSpacing)
	: _colorMap(nullptr), _font(font), _invisColor(invisColor), _isTextMode(is16Color), _style(kStyleNone), _drawOutline(drawOutline), _sjisWidthOffset(extraSpacing) {
	assert(_font);
	_font->setDrawingMode(_drawOutline ? Graphics::FontSJIS::kOutlineMode : Graphics::FontSJIS::kDefaultMode);
}

int SJISFont::getHeight() const {
	return _font->getFontHeight() >> 1;
}

int SJISFont::getWidth() const {
	return (_font->getMaxFontWidth() >> 1) + _sjisWidthOffset;
}

int SJISFont::getCharWidth(uint16 c) const {
	if (c <= 0x7F || (c >= 0xA1 && c <= 0xDF))
		return _font->getCharWidth('a') >> 1;
	else
		return getWidth();
}

void SJISFont::setColorMap(const uint8 *src) {
	_colorMap = src;
}

void SJISFont::drawChar(uint16 c, byte *dst, int pitch, int) const {
	uint8 color1, color2;

	if (_isTextMode) {
		// PC98 16 color games specify a color value which is for the
		// PC98 text mode palette, thus we need to remap it.
		color1 = ((_colorMap[1] >> 5) & 0x7) + 16;
		color2 = ((_colorMap[0] >> 5) & 0x7) + 16;
	} else {
		color1 = _colorMap[1];
		color2 = _colorMap[0];
	}

	if (!_isTextMode && _colorMap[0] == _invisColor)
		_font->setDrawingMode(Graphics::FontSJIS::kDefaultMode);
	else
		_font->setDrawingMode(_drawOutline ? Graphics::FontSJIS::kOutlineMode : Graphics::FontSJIS::kDefaultMode);

	_font->toggleFatPrint(_style == kStyleFat);
	_font->drawChar(dst, c, 640, 1, color1, color2, 640, 400);
}

ChineseFont::ChineseFont(int pitch, int renderWidth, int renderHeight, int spacingWidth, int spacingHeight, int extraSpacingWidth, int extraSpacingHeight) : Font(),
	_renderWidth(renderWidth), _renderHeight(renderHeight), _spacingWidth(spacingWidth), _spacingHeight(spacingHeight), _pitch(pitch), _border(false), _colorMap(nullptr),
	_borderExtraSpacingWidth(extraSpacingWidth), _borderExtraSpacingHeight(extraSpacingHeight), _glyphData(0), _glyphDataSize(0), _pixelColorShading(true) {
}

ChineseFont::~ChineseFont() {
	delete[] _glyphData;
}

bool ChineseFont::load(Common::SeekableReadStream &data) {
	if (_glyphData)
		return false;

	if (!data.size())
		return false;

	_glyphDataSize = data.size();
	uint8 *dst = new uint8[_glyphDataSize];
	if (!dst)
		return false;

	data.read(dst, _glyphDataSize);
	_glyphData = dst;

	return true;
}

void ChineseFont::setColorMap(const uint8 *src) {
	_colorMap = src;
	processColorMap();
}

void ChineseFont::drawChar(uint16 c, byte *dst, int pitch, int) const {
	static const int8 drawSeqNormal[4] = { 0, 0, 0, -1 };
	static const int8 drawSeqOutline[19] = { 1, 0, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 0, -1 };

	if (!hasGlyphForCharacter(c))
		return;

	uint32 offs = getFontOffset(c);
	assert(offs < _glyphDataSize);
	const uint8 *glyphData = _glyphData + offs;

	for (const int8 *i = _border ? drawSeqOutline : drawSeqNormal; *i != -1; i += 3) {
		const uint8 *data = glyphData;
		uint8 *dst3 = dst;
		dst = &dst3[i[0] + i[1] * _pitch];
		for (int h = 0; h < _renderHeight; ++h) {
			uint8 in = 0;
			int bt = -1;
			uint8 *dst2 = dst;
			for (int x = 0; x < _renderWidth; ++x) {
				if (bt == -1) {
					in = *data++;
					bt = 7;
				}
				if (in & (1 << (bt--))) {
					if (_pixelColorShading)
						*(uint16*)dst = _textColor[i[2]];
					else
						*dst = _textColor[i[2]] & 0xff;
				}
				dst++;
			}
			dst = dst2 + _pitch;
		}
		dst = dst3;
	}
}

MultiSubsetFont::~MultiSubsetFont() {
	for (Common::Array<Font*>::const_iterator i = _subsets->begin(); i != _subsets->end(); ++i)
		delete (*i);
	delete _subsets;
}

bool MultiSubsetFont::load(Common::SeekableReadStream &data) {
	for (Common::Array<Font*>::const_iterator i = _subsets->begin(); i != _subsets->end(); ++i)
		if ((*i)->load(data))
			return true;
	return false;
}

void MultiSubsetFont::setStyles(int styles) {
	for (Common::Array<Font*>::const_iterator i = _subsets->begin(); i != _subsets->end(); ++i)
		(*i)->setStyles(styles);
}

int MultiSubsetFont::getHeight() const {
	int res = 0;
	for (Common::Array<Font*>::const_iterator i = _subsets->begin(); i != _subsets->end(); ++i)
		res = MAX<int>(res, (*i)->getHeight());
	return res;
}

int MultiSubsetFont::getWidth() const {
	int res = 0;
	for (Common::Array<Font*>::const_iterator i = _subsets->begin(); i != _subsets->end(); ++i)
		res = MAX<int>(res, (*i)->getWidth());
	return res;
}

int MultiSubsetFont::getCharWidth(uint16 c) const {
	int res = 0;
	for (Common::Array<Font*>::const_iterator i = _subsets->begin(); i != _subsets->end(); ++i) {
		if ((res = (*i)->getCharWidth(c)) != -1)
			break;
	}
	return res > 0 ? res : 0;
}

int MultiSubsetFont::getCharHeight(uint16 c) const {
	int res = 0;
	for (Common::Array<Font*>::const_iterator i = _subsets->begin(); i != _subsets->end(); ++i) {
		if ((res = (*i)->getCharHeight(c)) != -1)
			break;
	}
	return res > 0 ? res : 0;
}

void MultiSubsetFont::setColorMap(const uint8 *src) {
	for (Common::Array<Font*>::const_iterator i = _subsets->begin(); i != _subsets->end(); ++i)
		(*i)->setColorMap(src);
}

void MultiSubsetFont::drawChar(uint16 c, byte *dst, int pitch, int) const {
	for (Common::Array<Font*>::const_iterator i = _subsets->begin(); i != _subsets->end(); ++i)
		(*i)->drawChar(c, dst, pitch, 0);
}

#pragma mark -

Palette::Palette(const int numColors) : _palData(nullptr), _numColors(numColors) {
	_palData = new uint8[numColors * 3]();
	assert(_palData);
}

Palette::~Palette() {
	delete[] _palData;
	_palData = nullptr;
}

void Palette::loadVGAPalette(Common::ReadStream &stream, int startIndex, int colors) {
	assert(startIndex + colors <= _numColors);

	uint8 *pos = _palData + startIndex * 3;
	for (int i = 0 ; i < colors * 3; i++)
		*pos++ = stream.readByte() & 0x3F;
}

void Palette::loadHiColorPalette(Common::ReadStream &stream, int startIndex, int colors) {
	uint16 *pos = (uint16*)(_palData + startIndex * 2);

	Graphics::PixelFormat currentFormat = g_system->getScreenFormat();
	Graphics::PixelFormat originalFormat(2, 5, 5, 5, 0, 5, 10, 0, 0);

	for (int i = 0; i < colors; i++) {
		uint8 r, g, b;
		originalFormat.colorToRGB(stream.readUint16LE(), r, g, b);
		*pos++ = currentFormat.RGBToColor(r, g, b);
	}
}

void Palette::loadEGAPalette(Common::ReadStream &stream, int startIndex, int colors) {
	assert(startIndex + colors <= 16);

	uint8 *dst = _palData + startIndex * 3;
	for (int i = 0; i < colors; i++) {
		uint8 index = stream.readByte();
		assert(index < _egaNumColors);
		memcpy(dst, &_egaColors[index * 3], 3);
		dst += 3;
	}
}

void Palette::setCGAPalette(int palIndex, CGAIntensity intensity) {
	assert(_numColors >= _cgaNumColors);
	assert(!(palIndex & ~1));
	memcpy(_palData, _cgaColors[palIndex * 2 + intensity], _cgaNumColors * 3);
}

void Palette::loadAmigaPalette(Common::ReadStream &stream, int startIndex, int colors) {
	assert(startIndex + colors <= _numColors);

	for (int i = 0; i < colors; ++i) {
		uint16 col = stream.readUint16BE();
		_palData[(i + startIndex) * 3 + 2] = ((col & 0xF) * 0x3F) / 0xF; col >>= 4;
		_palData[(i + startIndex) * 3 + 1] = ((col & 0xF) * 0x3F) / 0xF; col >>= 4;
		_palData[(i + startIndex) * 3 + 0] = ((col & 0xF) * 0x3F) / 0xF; col >>= 4;
	}
}

void Palette::loadPC98Palette(Common::ReadStream &stream, int startIndex, int colors) {
	assert(startIndex + colors <= _numColors);

	for (int i = 0; i < colors; ++i) {
		const byte g = stream.readByte(), r = stream.readByte(), b = stream.readByte();

		_palData[(i + startIndex) * 3 + 0] = ((r & 0xF) * 0x3F) / 0xF;
		_palData[(i + startIndex) * 3 + 1] = ((g & 0xF) * 0x3F) / 0xF;
		_palData[(i + startIndex) * 3 + 2] = ((b & 0xF) * 0x3F) / 0xF;
	}
}

void Palette::clear() {
	memset(_palData, 0, _numColors * 3);
}

void Palette::fill(int firstCol, int numCols, uint8 value) {
	assert(firstCol >= 0 && firstCol + numCols <= _numColors);

	memset(_palData + firstCol * 3, CLIP<int>(value, 0, 63), numCols * 3);
}

void Palette::copy(const Palette &source, int firstCol, int numCols, int dstStart) {
	if (numCols == -1)
		numCols = MIN(source.getNumColors(), _numColors) - firstCol;
	if (dstStart == -1)
		dstStart = firstCol;

	assert(numCols >= 0 && numCols <= _numColors);
	assert(firstCol >= 0 && firstCol <= source.getNumColors());
	assert(dstStart >= 0 && dstStart + numCols <= _numColors);

	memmove(_palData + dstStart * 3, source._palData + firstCol * 3, numCols * 3);
}

void Palette::copy(const uint8 *source, int firstCol, int numCols, int dstStart) {
	if (dstStart == -1)
		dstStart = firstCol;

	assert(numCols >= 0 && numCols <= _numColors);
	assert(firstCol >= 0);
	assert(dstStart >= 0 && dstStart + numCols <= _numColors);

	memmove(_palData + dstStart * 3, source + firstCol * 3, numCols * 3);
}

uint8 *Palette::fetchRealPalette() const {
	uint8 *buffer = new uint8[_numColors * 3];
	assert(buffer);

	uint8 *dst = buffer;
	const uint8 *palData = _palData;

	for (int i = 0; i < _numColors; ++i) {
		dst[0] = (palData[0] << 2) | (palData[0] & 3);
		dst[1] = (palData[1] << 2) | (palData[1] & 3);
		dst[2] = (palData[2] << 2) | (palData[2] & 3);

		dst += 3;
		palData += 3;
	}

	return buffer;
}

const uint8 Palette::_egaColors[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0x00, 0xAA, 0xAA,
	0xAA, 0x00, 0x00, 0xAA,	0x00, 0xAA, 0xAA, 0x55, 0x00, 0xAA, 0xAA, 0xAA,
	0x55, 0x55, 0x55, 0x55, 0x55, 0xFF, 0x55, 0xFF,	0x55, 0x55, 0xFF, 0xFF,
	0xFF, 0x55, 0x55, 0xFF, 0x55, 0xFF, 0xFF, 0xFF, 0x55, 0xFF, 0xFF, 0xFF
};

const int Palette::_egaNumColors = ARRAYSIZE(_egaColors) / 3;

const uint8 Palette::_cgaColors[4][12] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x2A, 0x00, 0x00, 0x2A, 0x15, 0x00 },
	{ 0x00, 0x00, 0x00, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x15, 0x3F, 0x3F, 0x15 },
	{ 0x00, 0x00, 0x00, 0x00, 0x2A, 0x2A, 0x2A, 0x00, 0x2A, 0x2A, 0x2A, 0x2A },
	{ 0x00, 0x00, 0x00, 0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x3F, 0x3F, 0x3F, 0x3F }
};

const int Palette::_cgaNumColors = ARRAYSIZE(_cgaColors[0]) / 3;

} // End of namespace Kyra
