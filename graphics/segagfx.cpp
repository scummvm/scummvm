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


#include "graphics/pixelformat.h"
#include "graphics/segagfx.h"
#include "graphics/surface.h"
#include "common/rect.h"

namespace Graphics {

SegaRenderer::SegaRenderer(const PixelFormat *systemPixelFormat) : _pixelFormat(systemPixelFormat ? *systemPixelFormat : PixelFormat::createFormatCLUT8()),	_pitch(64), _hScrollMode(0),
	_hScrollTable(0), _vScrollMode(0), _spriteTable(0), _numSpritesMax(0), _spriteMask(0), _backgroundColor(0), _planes{SegaPlane(), SegaPlane(), SegaPlane()}, _vram(nullptr), _vsram(nullptr),
	_tempBufferSprites(nullptr), _tempBufferSpritesPrio(nullptr), _tempBufferPlanes(nullptr), _tempBufferPlanesPrio(nullptr), _renderColorTable(nullptr), _rr(kUnspecified),
	_renderLineFragmentD(0), _renderLineFragmentM(0), _hINTEnable(false), _hINTCounter(0), _hINTCounterNext(0), _hINTHandler(nullptr), _destPitch(0), _displayEnabled(true), _realW(0), _realH(0)
#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
	, _arcIncX(0), _arcCntX(0), _arcIncY(0), _arcCntY(0), _xyAspectRatio(1.0), _scaleFactor(1.0)
#endif
{
	_vram = new uint8[0x10000]();
	assert(_vram);
	_vsram = new uint16[40]();
	assert(_vsram);

	if (_pixelFormat.bytesPerPixel > 1) {
		_renderColorTable = new uint32[0x100]();
		assert(_renderColorTable);
	}

	static const SegaRenderer::renderFuncD funcD[8] = {
		&SegaRenderer::renderLineFragmentD<false, false, false>,
		&SegaRenderer::renderLineFragmentD<false, false, true>,
		&SegaRenderer::renderLineFragmentD<false, true, false>,
		&SegaRenderer::renderLineFragmentD<false, true, true>,
		&SegaRenderer::renderLineFragmentD<true, false, false>,
		&SegaRenderer::renderLineFragmentD<true, false, true>,
		&SegaRenderer::renderLineFragmentD<true, true, false>,
		&SegaRenderer::renderLineFragmentD<true, true, true>
	};

	static const SegaRenderer::renderFuncM funcM[8] = {
		&SegaRenderer::renderLineFragmentM<false, false, false>,
		&SegaRenderer::renderLineFragmentM<false, false, true>,
		&SegaRenderer::renderLineFragmentM<false, true, false>,
		&SegaRenderer::renderLineFragmentM<false, true, true>,
		&SegaRenderer::renderLineFragmentM<true, false, false>,
		&SegaRenderer::renderLineFragmentM<true, false, true>,
		&SegaRenderer::renderLineFragmentM<true, true, false>,
		&SegaRenderer::renderLineFragmentM<true, true, true>
	};

	_renderLineFragmentD = funcD;
	_renderLineFragmentM = funcM;

	setResolution(320, 224);
#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
	setAspectRatioCorrection();
#endif
}

SegaRenderer::~SegaRenderer() {
	delete[] _vram;
	delete[] _vsram;
	delete[] _spriteMask;
	delete[] _tempBufferSprites;
	delete[] _tempBufferSpritesPrio;
	delete[] _tempBufferPlanes;
	delete[] _tempBufferPlanesPrio;
}

void SegaRenderer::setResolution(int w, int h) {
	assert(w == 320 || w == 256);
	assert(h == 224 || h == 240);

	_screenW = w;
	_screenH = h;
	_destPitch = w * _pixelFormat.bytesPerPixel;
	_blocksW = w >> 3;
	_blocksH = h >> 3;
	_numSpritesMax = w >> 2;

	delete[] _tempBufferSprites;
	delete[] _tempBufferSpritesPrio;
	delete[] _tempBufferPlanes;
	delete[] _tempBufferPlanesPrio;
	delete[] _spriteMask;

	if (_pixelFormat.bytesPerPixel > 1) {
		_tempBufferPlanes = new uint8[_screenW * _screenH]();
		assert(_tempBufferPlanes);
	}
	_tempBufferSprites = new uint8[_screenW * _screenH]();
	assert(_tempBufferSprites);
	_tempBufferSpritesPrio = new uint8[_screenW * _screenH]();
	assert(_tempBufferSpritesPrio);
	_tempBufferPlanesPrio = new uint8[_screenW]();
	assert(_tempBufferPlanesPrio);
	_spriteMask = new uint8[_screenW * _screenH]();
	assert(_spriteMask);
#if(!BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
	_realW = _screenW;
	_realH = _screenH;
}
#else
	setAspectRatioCorrection(_xyAspectRatio, _scaleFactor);
}

void SegaRenderer::setAspectRatioCorrection(double xyAspectRatio, double scaleFactor) {
	assert (xyAspectRatio > 0.0 || xyAspectRatio <= 2.0 || scaleFactor > 0.0 || scaleFactor <= 8.0);

	_xyAspectRatio = xyAspectRatio;
	_scaleFactor = scaleFactor;
	_arcCntX = _arcIncY = 0;
	_arcIncX = _arcIncY = trunc(scaleFactor * 0x10000);

	double screenRatio = (double)_screenW / _screenH;

	if (screenRatio > xyAspectRatio)
		_arcIncY = trunc((scaleFactor * 0x10000) / xyAspectRatio);
	else if (screenRatio < xyAspectRatio)
		_arcIncX = trunc(xyAspectRatio * scaleFactor * 0x10000);

	_realW = (_screenW * _arcIncX) >> 16;
	_realH = (_screenH * _arcIncY) >> 16;

	_destPitch = _realW * _pixelFormat.bytesPerPixel;
}
#endif

void SegaRenderer::getRealResolution(int &w, int &h) {
	w = _realW;
	h = _realH;
}

void SegaRenderer::setPlaneTableLocation(int plane, uint16 addr) {
	assert(plane >= kPlaneA && plane <= kWindowPlane);
	_planes[plane].nameTable = (uint16*)(&_vram[addr]);
}

void SegaRenderer::setupPlaneAB(int pixelWidth, int pixelHeigth) {
	for (int i = 0; i < 2; ++i) {
		if (pixelWidth != -1)
			_planes[i].w = pixelWidth >> 3;
		if (pixelHeigth != -1)
			_planes[i].h = pixelHeigth >> 3;
		_planes[i].mod = _planes[i].h;
		_planes[i].nameTableSize = _planes[i].w * _planes[i].h;
	}
	_pitch = pixelWidth >> 3;
}

void SegaRenderer::setupWindowPlane(int baseX, int baseY, int horizontalMode, int verticalMode) {
	if (baseX != -1) {
		baseX <<= 1; // The column unit size is 2 tiles (16 pixels)
		_planes[kWindowPlane].blockX = horizontalMode ? baseX : 0;
	}
	if (baseY != -1) // The line unit size is 1 tile (8 pixels)
		_planes[kWindowPlane].blockY = verticalMode ? baseY : 0;
	_planes[kWindowPlane].w = horizontalMode ? _blocksW - baseX : baseX;
	_planes[kWindowPlane].h = verticalMode ? _blocksH - baseY : baseY;
	_planes[kWindowPlane].mod = _blocksH;
	_planes[kWindowPlane].nameTableSize = (_planes[kWindowPlane].w || _planes[kWindowPlane].h) ? _blocksH * _pitch : 0;
}

void SegaRenderer::setHScrollTableLocation(int addr) {
	assert(addr <= 0xFFFF);
	_hScrollTable = (uint16*)(&_vram[addr]);
}

void SegaRenderer::setSpriteTableLocation(int addr) {
	assert(addr <= 0xFFFF);
	_spriteTable = (uint16*)(&_vram[addr]);
}

void SegaRenderer::setHScrollMode(int mode) {
	_hScrollMode = mode;
}

void SegaRenderer::setVScrollMode(int mode) {
	_vScrollMode = mode;
}

void SegaRenderer::enableDisplay(bool enable) {
	_displayEnabled = enable;
}

void SegaRenderer::loadToVRAM(const void *data, uint16 len, uint16 addr) {
	assert(data);
	assert(addr + len <= 0x10000);
	memcpy(_vram + addr, data, len);
}

void SegaRenderer::memsetVRAM(int addr, uint8 val, int len) {
	assert(addr + len <= 0x10000);
	memset(_vram + addr, val, len);
}

void SegaRenderer::writeUint16VSRAM(int addr, uint16 value) {
	assert(addr < 80);
	assert(!(addr & 1));
	_vsram[addr >> 1] = value;
}

void SegaRenderer::writeUint8VRAM(int addr, uint8 value) {
	assert(addr < 0x10000);
	_vram[addr] = value;
}

void SegaRenderer::writeUint16VRAM(int addr, uint16 value) {
	assert(addr < 0x10000);
	*((uint16*)(_vram + addr)) = value;
}

void SegaRenderer::clearPlanes() {
	for (int i = 0; i < 3; ++i) {
		if (_planes[i].nameTableSize)
			memset(_planes[i].nameTable, 0, _planes[i].nameTableSize * sizeof(uint16));
	}
}

#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
#define MERGE_LINES \
	(this->*ml)(d, rd1, rd2, _tempBufferPlanesPrio, rd3, w); \
	rd1 += _screenW; \
	rd2 += _screenW; \
	rd3 += _screenW; \
	d += _destPitch; \
	_arcCntY += (_arcIncY - 0x10000); \
	while (_arcCntY >= 0x10000) { \
		_arcCntY -= 0x10000; \
		memcpy(d, d - _destPitch, w2); \
		d += _destPitch; \
	} \
	hINTUpdate(); \
	ml = getLineHandler()
#else
#define MERGE_LINES \
	(this->*ml)(d, rd1, rd2, _tempBufferPlanesPrio, rd3, w); \
	rd1 += _screenW; \
	rd2 += _screenW; \
	rd3 += _screenW; \
	d += _destPitch; \
	hINTUpdate(); \
	ml = getLineHandler()
#endif

void SegaRenderer::render(void *dst, int x, int y, int w, int h) {
	if (x == -1)
		x = 0;
	if (y == -1)
		y = 0;
	if (w == -1)
		w = _screenW;
	if (h == -1)
		h = _screenH;

	_rr = kUnspecified;
	int y2 = y + h;
#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
	int w2 = ((w * _arcIncX) >> 16) * _pixelFormat.bytesPerPixel;
#endif

	assert(dst);
	assert(x >= 0 && x + w <= _screenW);
	assert(w > 0);
	assert(y >= 0 && y2 <= _screenH);
	assert(h > 0);

	memset(_tempBufferSprites, _backgroundColor, _screenW * _screenH);
	memset(_tempBufferSpritesPrio, _backgroundColor, _screenW * _screenH);
	renderSprites(_tempBufferSprites, _tempBufferSpritesPrio, x, y, w, h);

	MergeLinesProc ml = getLineHandler();

	uint8 *rd1 = _pixelFormat.bytesPerPixel == 1 ? reinterpret_cast<uint8*>(dst) : _tempBufferPlanes;
	uint8 *rd2 = _tempBufferSprites;
	uint8 *rd3 = _tempBufferSpritesPrio;

	int16 wY1 = _planes[kWindowPlane].blockY << 3;
	int16 wY2 = wY1 + (_planes[kWindowPlane].h << 3);
	int16 wX1 = _planes[kWindowPlane].blockX << 3;
	int16 wX2 = wX1 + (_planes[kWindowPlane].w << 3);

	rd1 += (y * _screenW + x);
	rd2 += (y * _screenW + x);
	rd3 += (y * _screenW + x);

#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
	uint8 *d = reinterpret_cast<uint8*>(dst) + (((y * _arcIncY) >> 16) * _destPitch + ((x * _arcIncX) >> 16) * _pixelFormat.bytesPerPixel);
	_arcCntX = (x * _arcIncX) & 0xFFFF;
	_arcCntY = (y * _arcIncY) & 0xFFFF;
#else
	uint8 *d = reinterpret_cast<uint8*>(dst) + y * _destPitch + x * _pixelFormat.bytesPerPixel;
#endif

	if (x == 0 && y == 0 && w == _screenW && h == _screenH) {
		memset(rd1, _backgroundColor, _screenW * _screenH);
	} else {
		uint8 *rd4 = rd1;
		for (int y1 = y; y1 < y2; ++y1) {
			memset(rd4, _backgroundColor, w);
			rd4 += _screenW;
		}
	}

	_hINTCounter = 0;
	for (int i = 0; i <= y; ++i) {
		hINTUpdate();
		ml = getLineHandler();
	}

	if (_planes[kPlaneA].nameTable != _planes[kPlaneB].nameTable) {
		// Include plane A (only draw if the nametable is not identical to that of plane B)
		if (_planes[kWindowPlane].nameTableSize != 0) {
			// With window plane, if available
			for (int y1 = y; y1 < y2; ++y1) {
				memset(_tempBufferPlanesPrio, 0, _screenW);
				if (_displayEnabled) {
					renderPlaneLine<false>(rd1, _tempBufferPlanesPrio, kPlaneB, y1, x, w);
					if (y1 >= wY1 && y1 < wY2) {
						renderPlaneLine<true>(rd1, _tempBufferPlanesPrio, kWindowPlane, y1, x, w);
					} else {
						int tmpX = (x > wX1) ? x : wX1;
						int tmpW = (x + w < wX2) ? w : wX2 - tmpX;
						if (tmpW > 0)
							renderPlaneLine<true>(rd1, _tempBufferPlanesPrio, kWindowPlane, y1, tmpX, tmpW);
						if (x < wX1)
							renderPlaneLine<false>(rd1, _tempBufferPlanesPrio, kPlaneA, y1, x, wX1 - x);
						else if (x + w > wX2)
							renderPlaneLine<false>(rd1, _tempBufferPlanesPrio, kPlaneA, y1, wX2, x + w - wX2);
					}
				}

				MERGE_LINES;
			}
		} else {
			// Without window plane
			for (int y1 = y; y1 < y2; ++y1) {
				memset(_tempBufferPlanesPrio, 0, _screenW);
				if (_displayEnabled) {
					renderPlaneLine<false>(rd1, _tempBufferPlanesPrio, kPlaneB, y1, x, w);
					renderPlaneLine<false>(rd1, _tempBufferPlanesPrio, kPlaneA, y1, x, w);
				}

				MERGE_LINES;
			}
		}
	} else if (_planes[kWindowPlane].nameTableSize != 0) {
		// Include window plane if available
		for (int y1 = y; y1 < y2; ++y1) {
			memset(_tempBufferPlanesPrio, 0, _screenW);
			if (_displayEnabled) {
				renderPlaneLine<false>(rd1, _tempBufferPlanesPrio, kPlaneB, y1, x, w);
				if (y1 >= wY1 && y1 < wY2) {
					renderPlaneLine<true>(rd1, _tempBufferPlanesPrio, kWindowPlane, y1, x, w);
				} else {
					int tmpX = (x > wX1) ? x : wX1;
					int tmpW = (x + w < wX2) ? w : wX2 - tmpX;
					if (tmpW > 0)
						renderPlaneLine<true>(rd1, _tempBufferPlanesPrio, kWindowPlane, y1, tmpX, tmpW);
				}
			}

			MERGE_LINES;
		}
	} else {
		// Only plane B
		for (int y1 = y; y1 < y2; ++y1) {
			memset(_tempBufferPlanesPrio, 0, _screenW);
			if (_displayEnabled)
				renderPlaneLine<false>(rd1, _tempBufferPlanesPrio, kPlaneB, y1, x, w);

			MERGE_LINES;
		}
	}
}
#undef MERGE_LINES

void SegaRenderer::renderSprites(uint8 *dst, uint8 *dstPrio, int clipX, int clipY, int clipW, int clipH) {
	if (clipX == -1)
		clipX = 0;
	if (clipY == -1)
		clipY = 0;
	if (clipW == -1)
		clipW = _screenW;
	if (clipH == -1)
		clipH = _screenH;

	memset(_spriteMask, 0xFF, _screenW * _screenH);
	const uint16 *pos = _spriteTable;

	int clipX2 = clipX + clipW;
	int clipY2 = clipY + clipH;

	for (int i = 0; i < _numSpritesMax && pos; ++i) {
		int y = FROM_BE_16(*pos++) & 0x3FF;
		uint16 in = FROM_BE_16(*pos++);
		uint8 bH = ((in >> 8) & 3) + 1;
		uint8 bW = ((in >> 10) & 3) + 1;
		uint8 next = in & 0x7F;
		in = FROM_BE_16(*pos++);
		uint16 pal = ((in >> 13) & 3) << 4;
		bool prio = (in & 0x8000);
		bool hflip = (in & 0x800);
		bool vflip = (in & 0x1000);
		uint16 tile = in++ & 0x7FF;
		int x = FROM_BE_16(*pos) & 0x3FF;
		uint16 pres = dstPrio != nullptr && prio ? kHasPrioSprites : kHasSprites;
		bool renderedSomething = false;
		bool spriteMasking = (x == 0 && (y & 0x1FF) >= 128);

		x = (x & 0x1FF) - 128;
		y = (y & 0x1FF) - 128;

		uint8 *d = (dstPrio != nullptr && prio ? dstPrio : dst) + y * _screenW + x;
		uint8 *msk = _spriteMask + y * _screenW + x;

		if (spriteMasking) {
			int mH = MIN<int>(bH << 3, _screenH - y);
			if (mH > 0)
				memset(msk + 128, 0,  mH * _screenW);
			pos = next ? &_spriteTable[next << 2] : 0;
			continue;
		}

		int8 hstep = 0;
		int8 vstep = 1;

		if (hflip)
			tile += ((bW - 1) * bH);

		if (vflip) {
			tile += (bH - 1);
			vstep = -1;
			hstep = hflip ? 0 : 2 * bH;
		} else {
			hstep = hflip ? -2 * bH : 0;
		}

		int spitch = _screenW << 3;

		for (int blX = 0; blX < bW; ++blX) {
			if (x >= clipX2)
				break;

			uint8 *dst2 = d;
			uint8 *msk2 = msk;

			if (x > clipX - 8) {
				SprTileProc tr = (x < 0 || x > _screenW - 8) ? &SegaRenderer::renderSpriteTileXClipped : &SegaRenderer::renderSpriteTileDef;
				int y2 = y;

				for (int blY = 0; blY < bH; ++blY) {
					if (y > clipY - 8 && y < clipY2) {
						(this->*tr)(d, msk, x, y, tile, pal, vflip, hflip);
						renderedSomething = true;
					}

					tile += vstep;
					d += spitch;
					msk += spitch;
					y += 8;
				}
				y = y2;
			} else {
				tile += (vstep * bH);
			}

			tile += hstep;
			x += 8;
			d = dst2 + 8;
			msk = msk2 + 8;
		}

		if (renderedSomething)
			_rr |= pres;

		pos = next ? &_spriteTable[next << 2] : 0;
	}
}

template<typename T> void SegaRenderer::refresh(T *dst, int x, int y, int w, int h) {
	if (sizeof(T) == 1)
		return;

	uint8 *rd1 = _tempBufferPlanes + (y * _screenW + x);
	dst += (y * _screenW + x);

	if (x == 0 && y == 0 && w == _screenW && h == _screenH) {
		for (uint32 s = w * h; s; --s)
			*dst++ = _renderColorTable[*rd1++];
	} else {
		uint16 pitch = _screenW - w;
		while (h--) {
			for (int x1 = x; x1 < x + w; ++x1)
				*dst++ = _renderColorTable[*rd1++];
			rd1 += pitch;
			dst += pitch;
		}
	}
}

void SegaRenderer::setRenderColorTable(const uint8 *colors, uint16 first, uint16 num) {
	if (_renderColorTable == nullptr)
		return;

	uint32 *dst = _renderColorTable + first;
	while (num--) {
		*dst++ = _pixelFormat.RGBToColor(colors[0], colors[1], colors[2]);
		colors += 3;
	}
}

void SegaRenderer::hINT_enable(bool enable) {
	_hINTEnable = enable;
}

void SegaRenderer::hINT_setCounter(uint8 counter) {
	_hINTCounterNext = counter;
}

void SegaRenderer::hINT_setHandler(const HINTHandler *hdl) {
	_hINTHandler = hdl;
}

void SegaRenderer::fillBackground(uint8 *dst, int blockX, int blockY, int blockW, int blockH) {
	uint32 *pos = (uint32*)(dst + (blockY << 3) * _screenW + (blockX << 3));
	blockW <<= 1;
	blockH <<= 3;
	int pitch = _screenW >> 2;

	uint32 fill = (_backgroundColor << 24) | (_backgroundColor << 16) | (_backgroundColor << 8) | _backgroundColor;
	while (blockH--) {
		uint32 *pos2 = pos;
		for (int x = 0; x < blockW; ++x)
			*pos2++ = fill;
		pos += pitch;
	}
}

template<bool isWindow> void SegaRenderer::renderPlaneLine(uint8 *planeBuffPos, uint8 *planePrioBuffPos, int srcPlane, int y, int clipX, int clipW) {
	SegaPlane *p = &_planes[isWindow ? kWindowPlane : srcPlane];

	int16 vbX = clipX >> 3;
	uint16 hscrNt = vbX;
	uint16 hscrPxFirst = clipX & 7;
	uint16 hscrPxLast = MIN<uint16>(clipW, 8);
	uint16 oddFlag = 0;
	uint16 hStep = hscrPxLast;
	uint16 vscrNt = y >> 3;
	uint16 vscrPxStart = y & 7;
	uint8 *d1 = planeBuffPos;
	uint8 *d2 = planePrioBuffPos;

	if (!isWindow) {
		hscrNt = clipX + (-FROM_BE_16(_hScrollTable[_hScrollMode == kHScrollFullScreen ? srcPlane : ((_hScrollMode == kHScroll1PixelRows ? y : (y & ~7)) << 1) + srcPlane]) & 0x3FF);
		hscrPxFirst = hscrNt & 7;
		hscrNt >>= 3;
		oddFlag = (hscrPxFirst & 1) << 1;
		hStep -= hscrPxFirst;
		if (_vScrollMode == kVScrollFullScreen) {
			vscrNt = y + (FROM_BE_16(_vsram[srcPlane]) & 0x3FF);
			vscrPxStart = vscrNt & 7;
			vscrNt >>= 3;
		}
	}

	while (clipW) {
		if (!isWindow && _vScrollMode == kVScroll16PixelStrips) {
			vscrNt = y + (FROM_BE_16(_vsram[((vbX % _blocksW) & ~1) + srcPlane]) & 0x3FF);
			vscrPxStart = vscrNt & 7;
			vscrNt >>= 3;
		}

		const uint16 *pNt = &p->nameTable[(vscrNt % p->mod) * _pitch + (hscrNt % _pitch)];

		if (pNt < (const uint16*)(&_vram[0x10000])) {
			uint16 nt = FROM_BE_16(*pNt);
			uint16 pal = ((nt >> 13) & 3) << 4;
			bool hflip = (nt & 0x800);
			int yl = (nt & 0x1000) ? 7 - vscrPxStart : vscrPxStart;

			renderFuncD proc = _renderLineFragmentD[(hflip ? 4 : 0) | oddFlag];
			(this->*proc)((nt &0x8000) ? d2 : d1, &_vram[((nt & 0x7FF) << 5) + (yl << 2) + (hscrPxFirst >> 1)], hscrPxFirst, hscrPxLast, pal);
		}

		++vbX;
		++hscrNt;
		d1 += hStep;
		d2 += hStep;
		clipW -= hStep;
		hscrPxFirst = 0;

		if (clipW >= 8) {
			hStep = 8;
			oddFlag = 0;
			continue;
		}

		hscrPxLast = hStep = clipW;
		oddFlag = hscrPxLast & 1;
	}
}

#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
#define ML_SETPIXEL \
if (adjustAspectRatio) { \
	arcCnt += _arcIncX; \
	if (sizeof(T) > 1) { \
		while (arcCnt >= 0x10000) { \
			arcCnt -= 0x10000; \
			*d++ = _renderColorTable[*bottomLayer]; \
		} \
	} else { \
		arcCnt -= 0x10000; \
		while (arcCnt >= 0x10000) { \
			arcCnt -= 0x10000; \
			bottomLayer[1] = bottomLayer[0]; \
			++bottomLayer; \
		} \
	} \
} else { \
	if (sizeof(T) > 1) \
		*d++ = _renderColorTable[*bottomLayer]; \
}
#else
#define ML_SETPIXEL \
	if (sizeof(T) > 1) \
		*d++ = _renderColorTable[*bottomLayer];
#endif

template<typename T, bool withSprites, bool withPrioSprites, bool adjustAspectRatio> void SegaRenderer::mergeLines(void *dst, uint8 *bottomLayer, const uint8 *spriteLayer, const uint8 *prioLayer, const uint8 *prioSpriteLayer, int clipW) {
	T *d = reinterpret_cast<T*>(dst);

	if (sizeof(T) == 1) {
		if (!bottomLayer)
			bottomLayer = reinterpret_cast<uint8*>(dst);
		assert(bottomLayer);
	}

#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
	uint32 arcCnt = _arcCntX;
#endif

	if (withPrioSprites) {
		if (withSprites) {
			while (clipW--) {
				if (*prioSpriteLayer)
					*bottomLayer = *prioSpriteLayer;
				else if (*prioLayer)
					*bottomLayer = *prioLayer;
				else if (*spriteLayer)
					*bottomLayer = *spriteLayer;

				ML_SETPIXEL
				++bottomLayer;
				++spriteLayer;
				++prioLayer;
				++prioSpriteLayer;
			}
		} else {
			while (clipW--) {
				if (*prioSpriteLayer)
					*bottomLayer = *prioSpriteLayer;
				else if (*prioLayer)
					*bottomLayer = *prioLayer;

				ML_SETPIXEL
				++bottomLayer;
				++prioLayer;
				++prioSpriteLayer;
			}
		}
	} else if (withSprites) {
		while (clipW--) {
			if (*prioLayer)
				*bottomLayer = *prioLayer;
			else if (*spriteLayer)
				*bottomLayer = *spriteLayer;

			ML_SETPIXEL
			++bottomLayer;
			++spriteLayer;
			++prioLayer;
		}
	} else {
		while (clipW--) {
			if (*prioLayer)
				*bottomLayer = *prioLayer;

			ML_SETPIXEL
			++bottomLayer;
			++prioLayer;
		}
	}
}
#undef ML_SETPIXEL

SegaRenderer::MergeLinesProc SegaRenderer::getLineHandler() const {
	static const MergeLinesProc mpc[] = {
		nullptr,
		&SegaRenderer::mergeLines<uint8, false, false, false>,
		&SegaRenderer::mergeLines<uint16, false, false, false>,
		nullptr,
		&SegaRenderer::mergeLines<uint32, false, false, false>,
		&SegaRenderer::mergeLines<uint8, true, false, false>,
		&SegaRenderer::mergeLines<uint16, true, false, false>,
		nullptr,
		&SegaRenderer::mergeLines<uint32, true, false, false>,
		&SegaRenderer::mergeLines<uint8, false, true, false>,
		&SegaRenderer::mergeLines<uint16, false, true, false>,
		nullptr,
		&SegaRenderer::mergeLines<uint32, false, true, false>,
		&SegaRenderer::mergeLines<uint8, true, true, false>,
		&SegaRenderer::mergeLines<uint16, true, true, false>,
		nullptr,
		&SegaRenderer::mergeLines<uint32, true, true, false>,
#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
		nullptr,
		&SegaRenderer::mergeLines<uint8, false, false, true>,
		&SegaRenderer::mergeLines<uint16, false, false, true>,
		nullptr,
		&SegaRenderer::mergeLines<uint32, false, false, true>,
		&SegaRenderer::mergeLines<uint8, true, false, true>,
		&SegaRenderer::mergeLines<uint16, true, false, true>,
		nullptr,
		&SegaRenderer::mergeLines<uint32, true, false, true>,
		&SegaRenderer::mergeLines<uint8, false, true, true>,
		&SegaRenderer::mergeLines<uint16, false, true, true>,
		nullptr,
		&SegaRenderer::mergeLines<uint32, false, true, true>,
		&SegaRenderer::mergeLines<uint8, true, true, true>,
		&SegaRenderer::mergeLines<uint16, true, true, true>,
		nullptr,
		&SegaRenderer::mergeLines<uint32, true, true, true>
#endif
	};

	uint index = _pixelFormat.bytesPerPixel;
	if (_displayEnabled) {
		if (_rr & kHasSprites)
			index += 4;
		if (_rr & kHasPrioSprites)
			index += 8;
#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
		if (_arcIncX != 0x10000)
			index += 17;
#endif
	}

	assert(index < ARRAYSIZE(mpc));
	MergeLinesProc res = mpc[index];
	assert(res);

	return res;
}

void SegaRenderer::hINTUpdate() {
	if (!_hINTCounter--) {
		if (_hINTEnable && _hINTHandler && _hINTHandler->isValid())
			(*_hINTHandler)(this);
		_hINTCounter = _hINTCounterNext;
	}
}

void SegaRenderer::renderSpriteTileXClipped(uint8 *dst, uint8 *mask, int x, int y, uint16 tile, uint8 pal, bool vflip, bool hflip) {
	const uint8 *src = &_vram[tile << 5];

	if (y < 0) {
		dst -= (y * _screenW);
		mask -= (y * _screenW);
	} if (x < 0) {
		dst -= x;
		mask -= x;
	}

	int xstart = CLIP<int>(-x, 0, 7);
	int xend = CLIP<int>(_screenW - x, 0, 8);
	src += (xstart >> 1);

	int ystart = CLIP<int>(-y, 0, 7);
	int yend = CLIP<int>(_screenH - y, 0, 8);
	src += ((vflip ? (7 - ystart) : ystart) << 2);
	int incr = vflip ? -4 : 4;

	renderFuncM proc = _renderLineFragmentM[(hflip ? 4 : 0) | ((xstart & 1) ? 2 : 0) | ((xend & 1) ? 1 : 0)];

	for (int bY = ystart; bY < yend; ++bY) {
		uint8 *dst2 = dst;
		uint8 *msk2 = mask;

		(this->*proc)(dst, mask, src, xstart, xend, pal);

		src += incr;
		dst = dst2 + _screenW;
		mask = msk2 + _screenW;
	}
}

void SegaRenderer::renderSpriteTileDef(uint8 *dst, uint8 *mask, int x, int y, uint16 tile, uint8 pal, bool vflip, bool hflip) {
	const uint8 *src = &_vram[tile << 5];

	if (y < 0) {
		dst -= (y * _screenW);
		mask -= (y * _screenW);
	}

	int ystart = CLIP<int>(-y, 0, 7);
	int yend = CLIP<int>(_screenH - y, 0, 8);
	src += ((vflip ? (7 - ystart) : ystart) << 2);
	int incr = vflip ? -4 : 4;

	renderFuncM proc = _renderLineFragmentM[hflip ? 4 : 0];

	for (int bY = ystart; bY < yend; ++bY) {
		uint8 *dst2 = dst;
		uint8 *msk2 = mask;

		(this->*proc)(dst, mask, src, 0, 8, pal);

		src += incr;
		dst = dst2 + _screenW;
		mask = msk2 + _screenW;
	}
}

template<bool hflip, bool oddStart, bool oddEnd> void SegaRenderer::renderLineFragmentM(uint8 *dst, uint8 *mask, const uint8 *src, int start, int end, uint8 pal) {
	if (hflip)
		src += ((end - 1 - start) >> 1);

	for (int i = (end - start) >> 1; i; --i) {
		uint8 col = hflip ? (oddEnd ? *src-- >> 4 : *src & 0x0F) : (oddStart ? *src++ & 0x0F : *src >> 4);
		uint8 col2 = hflip ? (oddEnd ? *src & 0x0F : *src-- >> 4) : (oddStart ? *src >> 4 : *src++ & 0x0F);
		if (col & *mask) {
			*dst = pal | col;
			*mask = 0;
		}
		dst++;
		mask++;
		if (col2 & *mask) {
			*dst = pal | col2;
			*mask = 0;
		}
		dst++;
		mask++;
	}
	if (oddStart != oddEnd) {
		uint8 col = hflip ? (oddEnd ? *src-- >> 4 : *src & 0x0F) : (oddStart ? *src++ & 0x0F : *src >> 4);
		if (col & *mask) {
			*dst = pal | col;
			*mask = 0;
		}
	}
}

template<bool hflip, bool oddStart, bool oddEnd> void SegaRenderer::renderLineFragmentD(uint8 *dst, const uint8 *src, int start, int end, uint8 pal) {
	if (hflip)
		src += ((end - 1 - start) >> 1);

	for (int i = (end - start) >> 1; i; --i) {
		uint8 col = hflip ? (oddEnd ? *src-- >> 4 : *src & 0x0F) : (oddStart ? *src++ & 0x0F : *src >> 4);
		uint8 col2 = hflip ? (oddEnd ? *src & 0x0F : *src-- >> 4) : (oddStart ? *src >> 4 : *src++ & 0x0F);
		if (col)
			*dst = pal | col;
		dst++;
		if (col2)
			*dst = pal | col2;
		dst++;
	}
	if (oddStart != oddEnd) {
		uint8 col = hflip ? (oddEnd ? *src-- >> 4 : *src & 0x0F) : (oddStart ? *src++ & 0x0F : *src >> 4);
		if (col)
			*dst = pal | col;
	}
}

} // End of namespace Graphics

