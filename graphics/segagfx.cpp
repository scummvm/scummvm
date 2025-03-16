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


#include "graphics/segagfx.h"
#include "graphics/surface.h"
#include "common/rect.h"

namespace Graphics {

#if SEGA_PERFORMANCE
#define mRenderLineFragment(hFlip, oddStart, oddEnd, useMask, dst, mask, src, start, end, pal) \
{ \
	int rlfOffs = 0; \
	if (hFlip) \
		rlfOffs |= 4; \
	if (oddStart) \
		rlfOffs |= 2; \
	if (oddEnd) \
		rlfOffs |= 1; \
	if (useMask) \
		(this->*_renderLineFragmentM[rlfOffs])(dst, mask, src, start, end, pal); \
	else \
		(this->*_renderLineFragmentD[rlfOffs])(dst, src, start, end, pal); \
}
#else
#define mRenderLineFragment(hFlip, oddStart, oddEnd, useMask, dst, mask, src, start, end, pal) \
{ \
	if (hFlip) \
		renderLineFragment<true>(dst, mask, src, start, end, pal); \
	else \
		renderLineFragment<false>(dst, mask, src, start, end, pal); \
}
#endif

SegaRenderer::SegaRenderer() : _prioChainStart(0), _prioChainEnd(0), _pitch(64), _hScrollMode(0), _hScrollTable(0), _vScrollMode(0), _spriteTable(0), _numSpritesMax(0), _spriteMask(0), _backgroundColor(0)
#if SEGA_PERFORMANCE
, _renderLineFragmentD(0), _renderLineFragmentM(0)
#endif
{
	_vram = new uint8[0x10000];
	assert(_vram);
	memset(_vram, 0, 0x10000 * sizeof(uint8));
	_vsram = new uint16[40];
	assert(_vsram);
	memset(_vsram, 0, 40 * sizeof(uint16));

#if SEGA_PERFORMANCE
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
#endif

	setResolution(320, 224);
}

SegaRenderer::~SegaRenderer() {
	delete[] _vram;
	delete[] _vsram;
	delete[] _spriteMask;
}

void SegaRenderer::setResolution(int w, int h) {
	assert(w == 320 || w == 256);
	assert(h == 224 || h == 240);

	_screenW = w;
	_screenH = h;
	_blocksW = w >> 3;
	_blocksH = h >> 3;
	_numSpritesMax = w >> 2;

	delete[] _spriteMask;
	_spriteMask = new uint8[w * h];
	assert(_spriteMask);
	memset(_spriteMask, 0, w * h * sizeof(uint8));
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
}

void SegaRenderer::setupWindowPlane(int blockX, int blockY, int horizontalMode, int verticalMode) {
	if (blockX != -1)
		_planes[kWindowPlane].blockX = horizontalMode ? blockX : 0;
	if (blockY != -1)
		_planes[kWindowPlane].blockY = verticalMode ? blockY : 0;
	_planes[kWindowPlane].w = horizontalMode ? _blocksW - blockX : blockX;
	_planes[kWindowPlane].h = verticalMode ? _blocksH - blockY : blockY;
	_planes[kWindowPlane].mod = _planes[kWindowPlane].blockY + _planes[kWindowPlane].h;
	_planes[kWindowPlane].nameTableSize = _planes[kWindowPlane].w * _planes[kWindowPlane].h;
}

void SegaRenderer::setHScrollTableLocation(int addr) {
	assert(addr <= 0xFFFF);
	_hScrollTable = (uint16*)(&_vram[addr]);
}

void SegaRenderer::setSpriteTableLocation(int addr) {
	assert(addr <= 0xFFFF);
	_spriteTable = (uint16*)(&_vram[addr]);
}

void SegaRenderer::setPitch(int pitch) {
	_pitch = pitch;
}

void SegaRenderer::setHScrollMode(int mode) {
	_hScrollMode = mode;
}

void SegaRenderer::setVScrollMode(int mode) {
	_vScrollMode = mode;
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

void SegaRenderer::render(Surface *surf, int renderBlockX, int renderBlockY, int renderBlockWidth, int renderBlockHeight, bool spritesOnly) {
	if ((_screenW != surf->w) || (_screenH != surf->h))
		error("SegaRenderer::render(): Render target surface needs to be have the same width and height as passed to SegaRenderer::setResolution().");

	if (surf->format.bpp() > 1)
		error("SegaRenderer::render(): Unsupported color mode in render target surface.");

	render((uint8*)surf->getPixels(), renderBlockX, renderBlockY, renderBlockWidth, renderBlockHeight, spritesOnly);
}

void SegaRenderer::render(uint8 *dest, int renderBlockX, int renderBlockY, int renderBlockWidth, int renderBlockHeight, bool spritesOnly) {
	if (renderBlockX == -1)
		renderBlockX = 0;
	if (renderBlockY == -1)
		renderBlockY = 0;
	if (renderBlockWidth == -1)
		renderBlockWidth = _blocksW;
	if (renderBlockHeight == -1)
		renderBlockHeight = _blocksH;

	assert(dest);
	uint8 *renderBuffer = dest;

	fillBackground(dest, renderBlockX, renderBlockY, renderBlockWidth, renderBlockHeight);

	// Plane B
	if (!spritesOnly)
		renderPlanePart(kPlaneB, renderBuffer, renderBlockX, renderBlockY, renderBlockX + renderBlockWidth, renderBlockY + renderBlockHeight);

	// Plane A (only draw if the nametable is not identical to that of plane B)
	if (_planes[kPlaneA].nameTable != _planes[kPlaneB].nameTable && !spritesOnly) {
		// If the window plane is active the rendering of plane A becomes more tedious because the window plane
		// kind of replaces plane A in the space that is covered by it.
		if (_planes[kWindowPlane].nameTableSize) {
			SegaPlane *p = &_planes[kWindowPlane];
			renderPlanePart(kPlaneA, renderBuffer, MAX<int>(0, renderBlockX), MAX<int>(0, renderBlockY), MIN<int>(p->blockX, renderBlockX + renderBlockWidth), MIN<int>(_blocksH, renderBlockY + renderBlockHeight));
			renderPlanePart(kPlaneA, renderBuffer, MAX<int>(0, renderBlockX), MAX<int>(0, renderBlockY), MIN<int>(_blocksW, renderBlockX + renderBlockWidth), MIN<int>(p->blockY, renderBlockY + renderBlockHeight));
			renderPlanePart(kPlaneA, renderBuffer, MAX<int>(p->blockX + p->w, renderBlockX), MAX<int>(0, renderBlockY), MIN<int>(_blocksW, renderBlockX + renderBlockWidth), MIN<int>(_blocksH, renderBlockY + renderBlockHeight));
			renderPlanePart(kPlaneA, renderBuffer, MAX<int>(0, renderBlockX), MAX<int>(p->blockY + p->h, renderBlockY), MIN<int>(_blocksW, renderBlockX + renderBlockWidth), MIN<int>(_blocksH, renderBlockY + renderBlockHeight));
		} else {
			renderPlanePart(kPlaneA, renderBuffer, renderBlockX, renderBlockY, renderBlockX + renderBlockWidth, renderBlockY + renderBlockHeight);
		}
	}

	// Window Plane
	if (_planes[kWindowPlane].nameTableSize && !spritesOnly) {
		SegaPlane *p = &_planes[kWindowPlane];
		renderPlanePart(kWindowPlane, renderBuffer, MIN<int>(p->blockX, renderBlockX + renderBlockWidth), MIN<int>(p->blockY, renderBlockY + renderBlockHeight), MAX<int>(p->blockX + p->w, renderBlockX), MAX<int>(p->blockY + p->h, renderBlockY));
	}

	// Sprites
	memset(_spriteMask, 0xFF, (uint32)_screenW * (uint32)_screenH * sizeof(uint8));
	const uint16 *pos = _spriteTable;
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

		// Sprite masking. Can't happen in EOB at least, since the animator automatically adds 128 to x and y coords for all sprites.
		// Same in Snatcher. If this triggers anywhere else it will need to be added...
		assert(!(x == 0 && y >= 128));

		x -= 128;
		y -= 128;

		/*if ((x >> 3) < renderBlockX) {
			bW = MIN<int>(0, (int)bW - (renderBlockX - (x >> 3)));
			x = (renderBlockX << 3);

		}

		if ((y >> 3) < renderBlockY) {
			bH = MIN<int>(0, (int)bH - (renderBlockY - (y >> 3)));
			y = (renderBlockY << 3);
		}

		bW = MIN<int>(bW, renderBlockWidth);
		bH = MIN<int>(bH, renderBlockHeight);*/

		uint8 *dst = renderBuffer + y * _screenW + x;
		uint8 *msk = _spriteMask + y * _screenW + x;
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


		for (int blX = 0; blX < bW; ++blX) {
			uint8 *dst2 = dst;
			uint8 *msk2 = msk;
			for (int blY = 0; blY < bH; ++blY) {
				renderSpriteTile(dst, msk, x + (blX << 3), y + (blY << 3), tile, pal, vflip, hflip, prio);
				tile += vstep;
				dst += (_screenW << 3);
				msk += (_screenW << 3);
			}
			tile += hstep;
			dst = dst2 + 8;
			msk = msk2 + 8;
		}

		pos = next ? &_spriteTable[next << 2] : 0;
	}

	// Priority Tiles
	// Instead of going through all rendering passes for all planes again (only now drawing the
	// prio tiles instead of the non-priority tiles) I have collected the data for the priority
	// tiles on the way and put that data into a chain. Should be faster...
	for (const PrioTileRenderObj *e = _prioChainStart; e; e = e->_next)
		mRenderLineFragment(e->_hflip, e->_start & 1, e->_end & 1, e->_mask, e->_dst, e->_mask, e->_src, e->_start, e->_end, e->_pal)

	clearPrioChain();
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

void SegaRenderer::renderPlanePart(int plane, uint8 *dstBuffer, int x1, int y1, int x2, int y2) {
	SegaPlane *p = &_planes[plane];
	uint8 *dst = dstBuffer + (y1 << 3) * _screenW + (x1 << 3);

	for (int y = y1; y < y2; ++y) {
		int hScrollTableIndex = (plane == kWindowPlane) ? -1 : (_hScrollMode == kHScrollFullScreen) ? plane : (y1 << 4) + plane;
		uint8 *dst2 = dst;
		for (int x = x1; x < x2; ++x) {
			int vScrollTableIndex = (plane == kWindowPlane) ? -1 : (_vScrollMode == kVScrollFullScreen) ? plane : (x & ~1) + plane;
			uint16 vscrNt = 0;
			uint16 vscrPxStart = 0;
			uint16 vscrPxEnd = 8;

			if (vScrollTableIndex != -1) {
				vscrNt = FROM_BE_16(_vsram[vScrollTableIndex]) & 0x3FF;
				vscrPxStart = vscrNt & 7;
				vscrNt >>= 3;
			}

			int ty = (vscrNt + y) % p->mod;

			renderPlaneTile(dst, x, &p->nameTable[ty * _pitch], vscrPxStart, vscrPxEnd, hScrollTableIndex, _pitch);

			if (vscrPxStart) {
				ty = (ty + 1) % p->mod;
				uint16 dstOffs = (vscrPxEnd - vscrPxStart) * _screenW;
				vscrPxEnd = vscrPxStart;
				vscrPxStart = 0;
				renderPlaneTile(dst + dstOffs, x, &p->nameTable[ty * _pitch], vscrPxStart, vscrPxEnd, hScrollTableIndex, _pitch);
			}
			dst += 8;
		}
		dst = dst2 + (_screenW << 3);
	}
}

void SegaRenderer::renderPlaneTile(uint8 *dst, int ntblX, const uint16 *ntblLine, int vScrollLSBStart, int vScrollLSBEnd, int hScrollTableIndex, uint16 pitch) {
	for (int bY = vScrollLSBStart; bY < vScrollLSBEnd; ++bY) {
		uint8 *dst2 = dst;
		uint16 hscrNt = 0;
		uint16 hscrPx = 0;

		if (hScrollTableIndex != -1) {
			hscrNt = (-(FROM_BE_16(_hScrollTable[hScrollTableIndex]))) & 0x3FF;
			hscrPx = hscrNt & 7;
			hscrNt >>= 3;
		}

		const uint16 *pNt = &ntblLine[(ntblX + hscrNt) % pitch];
		if (pNt < (const uint16*)(&_vram[0x10000])) {
			uint16 nt = FROM_BE_16(*pNt);
			uint16 pal = ((nt >> 13) & 3) << 4;
			bool hflip = (nt & 0x800);
			int y = bY % 8;
			if (nt & 0x1000) // vflip
				y = 7 - y;

			// We skip the priority tiles here and draw them later
			if (nt & 0x8000)
				initPrioRenderTask(dst, 0, &_vram[((nt & 0x7FF) << 5) + (y << 2) + (hscrPx >> 1)], hscrPx, 8, pal, hflip);
			else
				mRenderLineFragment(hflip, hscrPx & 1, 0, 0, dst, 0, &_vram[((nt & 0x7FF) << 5) + (y << 2) + (hscrPx >> 1)], hscrPx, 8, pal);
		}

		if (hscrPx) {
			dst += (8 - hscrPx);
			pNt = &ntblLine[(ntblX + hscrNt + 1) % pitch];
			if (pNt < (const uint16*)(&_vram[0x10000])) {
				uint16 nt = FROM_BE_16(*pNt);
				uint16 pal = ((nt >> 13) & 3) << 4;
				bool hflip = (nt & 0x800);
				int y = bY % 8;
				if (nt & 0x1000) // vflip
					y = 7 - y;

				// We skip the priority tiles here and draw them later
				if (nt & 0x8000)
					initPrioRenderTask(dst, 0, &_vram[((nt & 0x7FF) << 5) + (y << 2)], 0, hscrPx, pal, hflip);
				else
					mRenderLineFragment(hflip, 0, hscrPx & 1, 0, dst, 0, &_vram[((nt & 0x7FF) << 5) + (y << 2)], 0, hscrPx, pal)
			}
		}

		if (hScrollTableIndex != -1 && _hScrollMode == kHScroll1PixelRows)
			hScrollTableIndex += 2;
		dst = dst2 + _screenW;
	}
}

void SegaRenderer::renderSpriteTile(uint8 *dst, uint8 *mask, int x, int y, uint16 tile, uint8 pal, bool vflip, bool hflip, bool prio) {
	if (y <= -8 || y >= _screenH || x <= -8 || x >= _screenW)
		return;

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

	for (int bY = ystart; bY < yend; ++bY) {
		uint8 *dst2 = dst;
		uint8 *msk2 = mask;

		if (prio)
			initPrioRenderTask(dst, mask, src, xstart, xend, pal, hflip);
		else
			mRenderLineFragment(hflip, xstart & 1, xend & 1, 1, dst, mask, src, xstart, xend, pal);

		src += incr;
		dst = dst2 + _screenW;
		mask = msk2 + _screenW;
	}
}

#if SEGA_PERFORMANCE
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
		dst++;
		mask++;
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
		dst++;
	}
}
#else
template<bool hflip> void SegaRenderer::renderLineFragment(uint8 *dst, uint8 *mask, const uint8 *src, int start, int end, uint8 pal) {
	if (hflip) {
		src += ((end - 1 - start) >> 1);
		if (end & 1) {
			start++;
			end++;
		}
	}

	if (mask) {
		for (int bX = start; bX < end; ++bX) {
			uint8 col = hflip ? ((bX & 1) ? *src-- >> 4 : *src & 0x0F) : ((bX & 1) ? *src++ & 0x0F : *src >> 4);
			if (col & *mask) {
				*dst = pal | col;
				*mask = 0;
			}
			dst++;
			mask++;
		}
	} else {
		for (int bX = start; bX < end; ++bX) {
			uint8 col = hflip ? ((bX & 1) ? *src-- >> 4 : *src & 0x0F) : ((bX & 1) ? *src++ & 0x0F : *src >> 4);
			if (col)
				*dst = pal | col;
			dst++;
		}
	}
}
#endif

#undef mRenderLineFragment

void SegaRenderer::initPrioRenderTask(uint8 *dst, uint8 *mask, const uint8 *src, int start, int end, uint8 pal, bool hflip) {
#if SEGA_USE_MEMPOOL
	_prioChainEnd =	new (_prioRenderMemPool) PrioTileRenderObj(_prioChainEnd, dst, mask, src, start, end, pal, hflip);
#else
	_prioChainEnd = new PrioTileRenderObj(_prioChainEnd, dst, mask, src, start, end, pal, hflip);
#endif
	if (!_prioChainStart)
		_prioChainStart = _prioChainEnd;
}

void SegaRenderer::clearPrioChain() {
	while (_prioChainEnd) {
		_prioChainEnd->_next = 0;
		PrioTileRenderObj *e = _prioChainEnd->_pred;
#if SEGA_USE_MEMPOOL
		_prioRenderMemPool.deleteChunk(_prioChainEnd);
#else
		delete _prioChainEnd;
#endif
		_prioChainEnd = e;
	}
	_prioChainStart = 0;
}

} // End of namespace Graphics

