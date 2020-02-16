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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#ifdef ENABLE_EOB

#include "common/system.h"
#include "kyra/resource/resource.h"
#include "kyra/graphics/screen_eob.h"
#include "kyra/graphics/screen_eob_segacd.h"

namespace Kyra {

void Screen_EoB::sega_initGraphics() {
	_segaRenderer = new SegaRenderer(this);
	_segaRenderer->setResolution(320, 224);
	_segaRenderer->setPlaneTableLocation(SegaRenderer::kPlaneA, 0xC000);
	_segaRenderer->setPlaneTableLocation(SegaRenderer::kPlaneB, 0xE000);
	_segaRenderer->setPlaneTableLocation(SegaRenderer::kWindowPlane, 0xF000);
	_segaRenderer->setupPlaneAB(1024, 256);
	_segaRenderer->setupWindowPlane(0, 0, SegaRenderer::kWinToLeft, SegaRenderer::kWinToTop);
	_segaRenderer->setHScrollTableLocation(0xD800);
	_segaRenderer->setSpriteTableLocation(0xDC00);
	_segaAnimator = new SegaAnimator(_segaRenderer);
}

void Screen_EoB::sega_selectPalette(int srcPalID, int dstPalID, bool set) {
	if (srcPalID < -1 || srcPalID > 59 || dstPalID < 0 || dstPalID > 3)
		return;

	const uint16 *src = &_segaCurPalette[dstPalID << 4];
	uint8 rgbColors[48];
	uint8 *dst = rgbColors;

	if (srcPalID >= 31 && srcPalID <= 38) {
		src = &_segaCustomPalettes[(srcPalID - 31) << 4];
	} else if (srcPalID >= 0) {
		int temp = 0;
		const uint16 *palettes = _vm->staticres()->loadRawDataBe16(kEoB1PalettesSega, temp);
		if (!palettes)
			return;
		src = &palettes[srcPalID << 4];
	}

	// R: bits 1, 2, 3   G: bits 5, 6, 7   B: bits 9, 10, 11
	for (int i = 0; i < 16; ++i) {
		uint16 in = *src++;
		_segaCurPalette[dstPalID << 4 | i] = in;
#if 0
		static const uint8 col[8] = { 0, 52, 87, 116, 144, 172, 206, 255 };
		*dst++ = col[CLIP<int>(((in & 0x00F) >> 1) + brightness, 0, 7)];
		*dst++ = col[CLIP<int>(((in & 0x0F0) >> 5) + brightness, 0, 7)];
		*dst++ = col[CLIP<int>(((in & 0xF00) >> 9) + brightness, 0, 7)];
#else
		*dst++ = CLIP<int>(((in & 0x00F) >> 1) + _palFaders[dstPalID]._brCur, 0, 7) * 255 / 7;
		*dst++ = CLIP<int>(((in & 0x0F0) >> 5) + _palFaders[dstPalID]._brCur, 0, 7) * 255 / 7;
		*dst++ = CLIP<int>(((in & 0xF00) >> 9) + _palFaders[dstPalID]._brCur, 0, 7) * 255 / 7;
#endif
	}

	getPalette(0).copy(rgbColors, 0, 16, dstPalID << 4);
/*
	// Make half intensity palette
	dst = rgbColors;
	for (int i = 0; i < 16; ++i) {
		*dst++ >= 1;
		*dst++ >= 1;
		*dst++ >= 1;
	}

	getPalette(0).copy(rgbColors, 0, 16, (dstPalID << 4) | 0x40);
	memcpy(rgbColors, getPalette(0).getData() + (dstPalID << 4), 48);

	// Make double intensity palette
	dst = rgbColors;
	for (int i = 0; i < 16; ++i) {
		*dst++ <= 1;
		*dst++ <= 1;
		*dst++ <= 1;
	}

	getPalette(0).copy(rgbColors, 0, 16, (dstPalID << 4) | 0x80);
*/

	if (set) 
		setScreenPalette(getPalette(0));
}

void Screen_EoB::sega_loadCustomPaletteData(Common::ReadStream *in) {
	uint16 *dst = _segaCustomPalettes;
	for (int i = 0; i < 8; ++i) {
		*dst++ = 0;
		in->readUint16BE();
		if (in->eos())
			break;
		for (int ii = 1; ii < 16; ++ii)
			*dst++ = in->readUint16BE();
	}
}

void Screen_EoB::sega_updatePaletteFaders(int palID) {
	int first = 0;
	int last = 3;

	if (palID >= 0)
		first = last = palID;

	bool update = false;
	for (int i = first; i <= last; ++i) {
		PaletteFader &f = _palFaders[i];
		f._needRefresh = false;
		if (f._fadeDelay == 0 && f._brCur != f._brDest) {
			f._brCur = f._brDest;
			f._needRefresh = true;
		}
		if (f._brCur == f._brDest)
			continue;
		if (--f._fadeTimer)
			continue;

		f._brCur += f._fadeIncr;
		f._fadeTimer = f._fadeDelay;
		f._needRefresh = true;
	}

	for (int i = first; i <= last; ++i) {
		if (_palFaders[i]._needRefresh) {
			sega_selectPalette(-1, i, true);
			update = true;
			_palFaders[i]._needRefresh = false;
		}
	}

	if (update)
		updateScreen();
}

void Screen_EoB::sega_fadePalette(int delay, int16 brEnd, int dstPalID, bool waitForCompletion, bool noUpdate) {
	int first = 0;
	int last = 3;
	uint32 tickMillis = 0;

	if (dstPalID >= 0)
		first = last = dstPalID;

	if (!noUpdate) {
		for (int i = first; i <= last; ++i) {
			PaletteFader &f = _palFaders[i];
			f._needRefresh = false;
			if (f._brCur < brEnd)
				f._fadeIncr = 1;
			else if (f._brCur > brEnd)
				f._fadeIncr = -1;
			else
				continue;

			f._brDest = brEnd;
			f._fadeDelay = f._fadeTimer = delay;
		}
	}

	if (!waitForCompletion)
		return;

	for (bool runLoop = true; runLoop; ) {
		uint32 now = _vm->_system->getMillis();
		sega_updatePaletteFaders(dstPalID);

		runLoop = false;
		for (int i = first; i <= last; ++i) {
			if (_palFaders[i]._brCur != _palFaders[i]._brDest)
				runLoop = true;
		}

		tickMillis += 16667;
		uint32 ms = tickMillis / 1000;
		_vm->delayUntil(now + ms);
		tickMillis -= (ms * 1000);

		if (_vm->shouldQuit()) {
			for (int i = first; i <= last; ++i)
				_palFaders[i]._fadeDelay = 0;
		}
	}
}

void Screen_EoB::sega_paletteOps(int16 op, int16 par1, int16 par2) {
	assert(op >= 0 && op <= 6);
	switch (op) {
	case 6:
		// Force palette update and wait for completion
		break;
	case 5:
		// Force palette update, don't wait
		break;
	case 4:
		// TODO
		assert(0);
		break;
	default:
		sega_fadePalette(par2, par1, op, false);
	}
}

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

SegaRenderer::SegaRenderer(Screen_EoB *screen) : _screen(screen), _drChain(0), _prioChainStart(0), _prioChainEnd(0), _pitch(64), _hScrollMode(0), _hScrollTable(0), _vScrollMode(0), _spriteTable(0), _numSpritesMax(0), _spriteMask(0)
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
	int temp;

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

	for (int i = 0; i < 6; ++i)
		_patternTables[i] = _screen->_vm->staticres()->loadRawDataBe16(kEoB1PatternTable0 + i, temp);

	setResolution(320, 224);
}

SegaRenderer::~SegaRenderer() {
	clearDirtyRects();
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
	assert(addr <= 0xFFFF);
	_planes[plane].nameTable = (uint16*)(&_vram[addr]);
}

void SegaRenderer::setupPlaneAB(int w, int h) {
	for (int i = 0; i < 2; ++i) {
		if (w != -1)
			_planes[i].w = w;
		if (h != -1)
			_planes[i].h = h;
		_planes[i].nameTableSize = (w * h) >> 6;
	}
}

void SegaRenderer::setupWindowPlane(int blockX, int blockY, int horizontalMode, int verticalMode) {
	if (blockX != -1)
		_planes[kWindowPlane].blockX = horizontalMode ? blockX : 0;
	if (blockY != -1)
		_planes[kWindowPlane].blockY = verticalMode ? blockY : 0;
	_planes[kWindowPlane].w = horizontalMode ? _blocksW - blockX : blockX;
	_planes[kWindowPlane].h = verticalMode ? _blocksH - blockY : blockY;
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

void SegaRenderer::loadToVRAM(const void *data, int dataSize, int addr) {
	assert(data);
	assert(addr + dataSize <= 0xFFFF);
	memcpy(_vram + addr, data, dataSize);
	checkUpdateDirtyRects(addr, dataSize);
}

void SegaRenderer::loadToVRAM(Common::SeekableReadStreamEndian *in, int addr, bool compressedData) {
	assert(in);
	assert(addr <= 0xFFFF);

	uint8 *dst = _vram + addr;

	if (compressedData) {
		uint16 decodeSize = 0;
		uint8 *data = new uint8[in->size()];
		uint32 readSize = in->read(data, in->size());
		decodeSize = READ_LE_UINT16(data + 2);
		assert(decodeSize < readSize);
		assert(decodeSize < 0x10000 - addr);
		_screen->decodeBIN(data + 4, dst, decodeSize);
		delete[] data;
	} else {
		assert(in->size() < 0x10000 - addr);
		in->read(dst, in->size());
	}
	addDirtyRect(0, 0, _screenW, _screenH);
}

void SegaRenderer::memsetVRAM(int addr, uint8 val, int len) {
	assert(addr + len <= 0xFFFF);
	memset(_vram + addr, val, len);
	void checkUpdateDirtyRects(int addr, int len);
}

void SegaRenderer::fillRectWithTiles(int vramArea, int x, int y, int w, int h, uint16 nameTblEntry, bool incr, bool topToBottom, int presetPatternID) {
	uint16 addr = vramArea ? (vramArea == 1 ? 0xE000 : 0xF000) : 0xC000;
	addDirtyRect(x << 3, y << 3, w << 3, h << 3);
	if (y & 0x8000) {
		y &= ~0x8000;
		addr = 0xE000;
	}

	uint16 *dst = (uint16*)(&_vram[addr]);
	dst += (y * _pitch + x);
	int ptch = _pitch - w;

	assert(addr + 2 * (y * _pitch + x + h * _pitch + w) <= 0xFFFF);

	if (presetPatternID > -1) {
		assert(presetPatternID < 6);
		const uint16 *tbl = _patternTables[presetPatternID];
		while (h--) {
			const uint16 *pos = tbl;
			for (int i = w; i; --i)
				*dst++ = nameTblEntry + *pos++;
			dst += ptch;
			tbl += w;
		}
	} else if (incr) {
		if (topToBottom) {
			while (w--) {
				uint16 *dst2 = dst;
				for (int i = h; i; --i) {
					*dst = nameTblEntry++;
					dst += _pitch;
				}
				dst = ++dst2;
			}
		} else {
			while (h--) {
				for (int i = w; i; --i)
					*dst++ = nameTblEntry++;
				dst += ptch;
			}
		}
	} else {
		if (topToBottom) {
			while (w--) {
				uint16 *dst2 = dst;
				for (int i = h; i; --i) {
					*dst = nameTblEntry;
					dst += _pitch;
				}
				dst = ++dst2;
			}
		} else {
			while (h--) {
				for (int i = w; i; --i)
					*dst++ = nameTblEntry;
				dst += ptch;
			}
		}
	}
}

void SegaRenderer::writeVSRAMValue(int addr, uint16 value) {
	assert(addr < 80);
	_vsram[addr >> 1] = value;
}

void SegaRenderer::render(int destPageNum) {
	uint8 *renderBuffer = _screen->getPagePtr(destPageNum);
	memset(renderBuffer, 0, _screenW * _screenH);

	if (destPageNum == 0)
		sendDirtyRectsToScreen();

	// It is assumed that no dirty rects are needed if the rendering takes place to any other page than page 0.
	// So if you'd render to e. g. page 2 and afterwards to page 0 your dirty rects would be lost. This is
	// intentional for now, since I assume that this will be in harmony with the actual usage of the renderer.
	clearDirtyRects();

	// Plane B
	renderPlanePart(kPlaneB, renderBuffer, 0, 0, _blocksW, _blocksH);

	// Plane A (only draw if the nametable is not identical to that of plane B)
	if (_planes[kPlaneA].nameTable != _planes[kPlaneB].nameTable) {
		// If the window plane is active the rendering of plane A becomes more tedious because the window plane
		// kind of replaces plane A in the space that is covered by it.
		if (_planes[kWindowPlane].h && _planes[kWindowPlane].w) {
			SegaPlane *p = &_planes[kWindowPlane];
			renderPlanePart(kPlaneA, renderBuffer, 0, 0, p->blockX, _blocksH);
			renderPlanePart(kPlaneA, renderBuffer, 0, 0, _blocksW, p->blockY);
			renderPlanePart(kPlaneA, renderBuffer, p->blockX + p->w, 0, _blocksW, _blocksH);
			renderPlanePart(kPlaneA, renderBuffer, 0, p->blockY + p->h, _blocksH, _blocksH);
		} else {
			renderPlanePart(kPlaneA, renderBuffer, 0, 0, _blocksW, _blocksH);
		}
	}

	// Window Plane
	if (_planes[kWindowPlane].h && _planes[kWindowPlane].w) {
		SegaPlane *p = &_planes[kWindowPlane];
		renderPlanePart(kWindowPlane, renderBuffer, p->blockX, p->blockY, p->blockX + p->w, p->blockY + p->h);
	}

	// Sprites
	memset(_spriteMask, 0xFF, _screenW * _screenH * sizeof(uint8));
	const uint16 *pos = _spriteTable;
	for (int i = 0; i < _numSpritesMax && pos; ++i) {
		int y = *pos++ & 0x3FF;
		uint8 bH = ((*pos >> 8) & 3) + 1;
		uint8 bW = ((*pos >> 10) & 3) + 1;
		uint8 next = *pos++ & 0x7F;
		uint16 pal = ((*pos >> 13) & 3) << 4;
		bool prio = (*pos & 0x8000);
		bool hflip = (*pos & 0x800);
		bool vflip = (*pos & 0x1000);
		uint16 tile = *pos++ & 0x7FF;
		int x = *pos & 0x3FF;

		// Sprite masking. Can't happen really, since the animator automatically adds 128 to x and y coords for all sprites.
		assert(!(x == 0 && y >= 128));

		assert(!hflip);
		assert(!vflip);

		x -= 128;
		y -= 128;

		uint8 *dst = renderBuffer + y * _screenW + x;
		uint8 *msk = _spriteMask + y * _screenW + x;

		for (int blX = 0; blX < bW; ++blX) {
			uint8 *dst2 = dst;
			uint8 *msk2 = msk;
			for (int blY = 0; blY < bH; ++blY) {
				renderSpriteTile(dst, msk, x + (blX << 3), y + (blY << 3), tile++, pal, vflip, hflip, prio);
				dst += (_screenW << 3);
				msk += (_screenW << 3);
			}
			dst = dst2 + 8;
			msk = msk2 + 8;
		}

		pos = next ? &_spriteTable[next << 2] : 0;
	}

	// Priority Tiles
	// Instead of going through all rendering passes for all planes again (only now drawing the
	// prio tiles instead of the non-priority tiles) I have collected the data for the priority
	// tiles on the way and put that data in a chain. Should be faster...
	// The priority sprite tiles aren't rendered against the sprite mask table here which is wrong.
	// Priority sprites are a rare thing, though. EOB probably doesn't have any...
	for (PrioTileRenderObj *e = _prioChainStart; e; e = e->_next)
		mRenderLineFragment(e->_hflip, e->_start & 1, e->_end & 1, 0, e->_dst, 0, e->_src, e->_start, e->_end, e->_pal)
		
	clearPrioChain();
}

void SegaRenderer::renderPlanePart(int plane, uint8 *dstBuffer, int x1, int y1, int x2, int y2) {
	SegaPlane *p = &_planes[plane];
	uint16 *ntbl = p->nameTable + y1 * _pitch + x1;
	uint8 *dst = dstBuffer + (y1 << 3) * _screenW + (x1 << 3);

	for (int y = y1; y < y2; ++y) {
		int hScrollTableIndex = (plane == kWindowPlane) ? -1 : (_hScrollMode == kHScrollFullScreen) ? plane : (y1 << 4) + plane;
		uint8 *dst2 = dst;
		for (int x = x1; x < x2; ++x) {
			int vScrollTableIndex = (plane == kWindowPlane) ? -1 : (_vScrollMode == kVScrollFullScreen) ? plane : (x >> 1) + plane;
			renderPlaneTile(dst, x, ntbl, vScrollTableIndex, hScrollTableIndex, _pitch, p->nameTableSize);
			dst += 8;
		}
		ntbl += _pitch;
		dst = dst2 + (_screenW << 3);
	}
}

void SegaRenderer::renderPlaneTile(uint8 *dst, int destX, uint16 *nameTable, int vScrollTableIndex, int hScrollTableIndex, uint16 pitch, uint16 nameTableSize) {
	uint16 vscrNt = 0;
	uint16 vscrPx = 0;

	if (vScrollTableIndex != -1) {
		vscrNt = _vsram[vScrollTableIndex] & 0x3FF;
		vscrPx = vscrNt & 7;
		vscrNt >>= 3;
	}

	for (int bY = vscrPx; bY < vscrPx + 8; ++bY) {
		uint8 *dst2 = dst;
		uint16 hscrNt = 0;
		uint16 hscrPx = 0;

		if (bY == 8) {
			nameTable += pitch;
			if (hScrollTableIndex != -1 && _hScrollMode == kHScroll8PixelRows)
				hScrollTableIndex += 16;
		}

		if (hScrollTableIndex != -1) {
			hscrNt = _hScrollTable[hScrollTableIndex] & 0x3FF;
			hscrPx = hscrNt & 7;
			hscrNt >>= 3;
		}

		uint16 nt = nameTable[((vscrNt * pitch) % nameTableSize) + ((destX + hscrNt) % pitch)];
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

		if (hscrPx) {
			dst += (8 - hscrPx);
			nt = nameTable[((vscrNt * pitch) % nameTableSize) + ((destX + hscrNt + 1) % pitch)];
			pal = ((nt >> 13) & 3) << 4;
			hflip = (nt & 0x800);
			y = bY % 8;
			if (nt & 0x1000) // vflip
				y = 7 - y;

			// We skip the priority tiles here and draw them later
			if (nt & 0x8000)
				initPrioRenderTask(dst, 0, &_vram[((nt & 0x7FF) << 5) + (y << 2)], 0, hscrPx, pal, hflip);
			else
				mRenderLineFragment(hflip, 0, hscrPx & 1, 0, dst, 0, &_vram[((nt & 0x7FF) << 5) + (y << 2)], 0, hscrPx, pal)
		}

		if (hScrollTableIndex != -1 && _hScrollMode == kHScroll1PixelRows)
			hScrollTableIndex += 2;
		dst = dst2 + _screenW;
	}
}

#undef vflip

void SegaRenderer::renderSpriteTile(uint8 *dst, uint8 *mask, int x, int y, uint16 tile, uint8 pal, bool vflip, bool hflip, bool prio) {
	if (y <= -8 || y >= _screenH || x <= -8 || x >= _screenW)
		return;

	const uint8 *src = &_vram[tile << 5];
	if (vflip)
		src += 31;

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
	src += (ystart << 2);

	for (int bY = ystart; bY < yend; ++bY) {
		uint8 *dst2 = dst;
		uint8 *msk2 = mask;

		if (prio)
			initPrioRenderTask(dst, mask, src, xstart, xend, pal, hflip);
		else
			mRenderLineFragment(hflip, xstart & 1, xend & 1, 1, dst, mask, src, xstart, xend, pal);

		src += 4;
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
			/*if (col == 0x3E)
				*dst |= 0x40;
			else if (col == 0x3E)
				*dst |= 0x80;
			else*/
				*dst = pal | col;
			*mask = 0;
		}
		dst++;
		mask++;
		if (col2 & *mask) {
			/*if (col2 == 0x3E)
				*dst |= 0x40;
			else if (col2 == 0x3E)
				*dst |= 0x80;
			else*/
				*dst = pal | col2;
			*mask = 0;
		}
		dst++;
		mask++;
	}
	if (oddStart != oddEnd) {
		uint8 col = hflip ? (oddEnd ? *src-- >> 4 : *src & 0x0F) : (oddStart ? *src++ & 0x0F : *src >> 4);
		if (col & *mask) {
			/*if (col == 0x3E)
				*dst |= 0x40;
			else if (col == 0x3E)
				*dst |= 0x80;
			else*/
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
				/*if (col == 0x3E)
					*dst |= 0x40;
				else if (col == 0x3E)
					*dst |= 0x80;
				else*/
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

void SegaRenderer::checkUpdateDirtyRects(int addr, int len) {
	//void *tbl[] = { _vram, _hScrollTable, , _planes[kPlaneA].nameTable, _planes[kPlaneB].nameTable, _planes[kWindowPlane].nameTable };
	addDirtyRect(0, 0, _screenW, _screenH);
	/*uint8 *addE = &_vram[addr];
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	if (addE >= (uint8*)_planes[kPlaneA].nameTable && addE < (uint8*)_planes[kPlaneA].nameTable + _planes[kPlaneA].nameTableSize) {
		x = ((addE - (uint8*)_planes[kPlaneA].nameTable) >> 1) % _pitch;
		y = ((addE - (uint8*)_planes[kPlaneA].nameTable) >> 1) / _pitch;
		w = len > _pitch;
	}*/

	//if (addr >= _hScrollTable && addr < _hScrollTable + 0x400);
}

void SegaRenderer::addDirtyRect(int x, int y, int w, int h) {
	_drChain = new DRChainEntry(_drChain, x, y, w, h);
}

void SegaRenderer::sendDirtyRectsToScreen() {
	for (DRChainEntry *e = _drChain; e; e = e->_next) {
		int w = e->_rect.width();
		int h = e->_rect.height();
		if (w == _screenW && h == _screenH) {
			_screen->_forceFullUpdate = true;
			return;
		} 
		_screen->addDirtyRect(e->_rect.left, e->_rect.top, w, h);
	}
}

void SegaRenderer::clearDirtyRects() {
	while (_drChain) {
		DRChainEntry *e = _drChain->_next;
		delete _drChain;
		_drChain = e;
	}
}

void SegaRenderer::initPrioRenderTask(uint8 *dst, uint8 *mask, const uint8 *src, int start, int end, uint8 pal, bool hflip) {
	_prioChainEnd = new PrioTileRenderObj(_prioChainEnd, dst, mask, src, start, end, pal, hflip);
	if (!_prioChainStart)
		_prioChainStart = _prioChainEnd;
}

void SegaRenderer::clearPrioChain() {
	while (_prioChainEnd) {
		_prioChainEnd->_next = 0;
		PrioTileRenderObj *e = _prioChainEnd->_pred;
		delete _prioChainEnd;
		_prioChainEnd = e;
	}
	_prioChainStart = 0;
}

SegaAnimator::SegaAnimator(SegaRenderer *renderer) : _renderer(renderer), _needUpdate(false) {
	_sprites = new Sprite[80];
	assert(_sprites);
	memset(_sprites, 0, sizeof(Sprite) * 80);
	_tempBuffer = new uint16[320];
	assert(_tempBuffer);
	memset(_tempBuffer, 0, sizeof(uint16) * 320);
	int linkCnt = 1;
	for (int i = 1; i < 317; i += 4)
		_tempBuffer[i] = linkCnt++;
	clearSprites();
	_renderer->memsetVRAM(0xDC00, 0, 0x400);
}

SegaAnimator::~SegaAnimator() {
	delete[] _sprites;
}

void SegaAnimator::initSprite(int id, int16 x, int16 y, uint16 nameTbl, uint16 hw) {
	assert(id < 80);
	Sprite &s = _sprites[id];
	s.x = x;
	s.y = y;
	s.nameTbl = nameTbl;
	s.hw = hw;
	_needUpdate = true;
}

void SegaAnimator::clearSprites() {
	for (Sprite *s = _sprites; s != &_sprites[80]; ++s)
		s->x = 0x4000;
	_needUpdate = true;
}

void SegaAnimator::moveMorphSprite(int id, uint16 nameTbl, int16 addX, int16 addY) {
	assert(id < 80);
	Sprite &s = _sprites[id];
	s.x += addX;
	s.y += addY;
	s.nameTbl = nameTbl;
	_needUpdate = true;
}

void SegaAnimator::moveSprites(int id, uint16 num, int16 addX, int16 addY) {
	assert(id < 80);
	Sprite *s = &_sprites[id];
	while (num--) {
		s->x += addX;
		s->y += addY;
		s++;
	}
	_needUpdate = true;
}

void SegaAnimator::moveSprites2(int id, uint16 num, int16 addX, int16 addY) {
	assert(id < 80);
	Sprite *s = &_sprites[id];
	uint16 sbx = s->x;
	uint16 sby = s->y;
	while (num--) {
		s->x = s->x - sbx + addX;
		s->y = s->y - sby + addY;
		s++;
	}
	_needUpdate = true;
}

void SegaAnimator::update() {
	if (!_needUpdate)
		return;
	
	uint16 *dst = _tempBuffer;
	for (Sprite *s = _sprites; s != &_sprites[80]; ++s) {
		if (s->x == 0x4000)
			continue;
		*dst++ = (uint16)(s->y + 128);
		*dst++ = (*dst & 0xFF) | (s->hw << 8);
		*dst++ = s->nameTbl;
		*dst++ = (uint16)(s->x + 128);
	}

	for (dst; dst < &_tempBuffer[320]; dst += 4)
		*dst = 0;

	_renderer->loadToVRAM(_tempBuffer, 640, 0xDC00);
	_needUpdate = false;
}

SegaCDFont::SegaCDFont() : Font(), _data(0), _colorMap(0), _width(0), _height(0) {

}

SegaCDFont::~SegaCDFont() {
	delete[] _data;
}

bool SegaCDFont::load(Common::SeekableReadStream &file) {
	uint32 size = file.size();
	if (!size)
		return false;

	delete[] _data;

	_data = new uint8[size];
	file.read(_data, size);
	return true;
}

void SegaCDFont::drawChar(uint16 c, byte *dst, int pitch, int) const {

}

#undef mRenderLineFragment

} // End of namespace Kyra

#endif // ENABLE_EOB
