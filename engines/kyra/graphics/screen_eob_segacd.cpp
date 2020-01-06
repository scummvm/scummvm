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
#include "graphics/palette.h"
#include "kyra/resource/resource.h"

namespace Kyra {

struct PaletteFader {
	PaletteFader() : _brCur(0), _brDest(0), _fadeIncr(0), _fadeDelay(0), _fadeTimer(0) {}
	int16 _brCur;
	int16 _brDest;
	int16 _fadeIncr;
	int16 _fadeDelay;
	int16 _fadeTimer;
};

void Screen_EoB::sega_selectPalette(int srcPalID, int dstPalID, int brightness, bool set) {
	if (srcPalID < -1 || srcPalID > 59 || dstPalID < 0 || dstPalID > 3)
		return;

	const uint16 *src = &_segaPalette[dstPalID << 4];
	uint8 rgbColors[48];
	uint8 *dst = rgbColors;

	if (srcPalID >= 0) {
		int temp = 0;
		const uint16 *palettes = _vm->staticres()->loadRawDataBe16(kEoB1PalettesSega, temp);
		if (!palettes)
			return;
		src = &palettes[srcPalID << 4];
	}

	// R: bits 1, 2, 3   G: bits 5, 6, 7   B: bits 9, 10, 11
	for (int i = 0; i < 16; ++i) {
		uint16 in = *src++;
		_segaPalette[dstPalID << 4 | i] = in;
#if 0
		static const uint8 col[8] = { 0, 52, 87, 116, 144, 172, 206, 255 };
		*dst++ = col[CLIP<int>(((in & 0x00F) >> 1) + brightness, 0, 7)];
		*dst++ = col[CLIP<int>(((in & 0x0F0) >> 5) + brightness, 0, 7)];
		*dst++ = col[CLIP<int>(((in & 0xF00) >> 9) + brightness, 0, 7)];
#else
		*dst++ = CLIP<int>(((in & 0x00F) >> 1) + brightness, 0, 7) * 255 / 7;
		*dst++ = CLIP<int>(((in & 0x0F0) >> 5) + brightness, 0, 7) * 255 / 7;
		*dst++ = CLIP<int>(((in & 0xF00) >> 9) + brightness, 0, 7) * 255 / 7;
#endif
	}

	getPalette(0).copy(rgbColors, 0, 16, dstPalID << 4);

	if (set) {
		_vm->_system->getPaletteManager()->setPalette(rgbColors, dstPalID << 4, 16);
		_paletteChanged = true;
	}
}

void Screen_EoB::sega_fadePalette(int delay, int brEnd, int dstPalID) {
	int first = 0;
	int last = 3;
	if (dstPalID >= 0)
		first = last = dstPalID;

	PaletteFader faders[4];

	for (int i = first; i <= last; ++i) {
		PaletteFader &f = faders[i];
		f._brCur = _brState[i];
		if (f._brCur < brEnd)
			f._fadeIncr = 1;
		else if (f._brCur > brEnd)
			f._fadeIncr = -1;
		else
			continue;

		f._brDest = brEnd;
		f._fadeDelay = f._fadeTimer = delay;
	}

	for (bool runLoop = true; runLoop; ) {
		uint32 end = _vm->_system->getMillis() + 17;
		bool update = false;
		for (int i = first; i <= last; ++i) {
			_brState[i] = 127;
			PaletteFader &f = faders[i];
			if (f._fadeDelay == 0 && f._brCur != f._brDest) {
				f._brCur = f._brDest;
				update = true;
			}
			if (f._brCur == f._brDest)
				continue;
			if (--f._fadeTimer)
				continue;

			f._brCur += f._fadeIncr;
			f._fadeTimer = f._fadeDelay;
			_brState[i] = faders[i]._brCur;
		}
		
		for (int i = first; i <= last; ++i) {
			if (_brState[i] != 127) {
				sega_selectPalette(-1, i, _brState[i], true);
				update = true;
				_brState[i] = 127;
			}
		}

		if (update)
			updateScreen();

		runLoop = false;
		for (int i = first; i <= last; ++i) {
			if (faders[i]._brCur != faders[i]._brDest)
				runLoop = true;
			_brState[i] = faders[i]._brCur;
		}

		_vm->delayUntil(end);

		if (_vm->shouldQuit()) {
			for (int i = first; i <= last; ++i)
				faders[i]._fadeDelay = 0;
		}
	}
}

void Screen_EoB::sega_paletteOps(int op, int par1, int par2) {
	assert(op >= 0 && op <= 6);
	switch (op) {
	case 6:
		// Force palette update and wait for completion
		break;
	case 5:
		// Force palette update, don't wait
		break;
	case 4:
		assert(0);
		break;
	default:
		sega_fadePalette(par2, par1, op);
	}
}

SegaRenderer::SegaRenderer(Screen_EoB *screen) : _screen(screen), _drChain(0), _pitch(64), _hScrollMode(0), _hScrollTable(0) {
	_vram = new uint8[0x10000];
	assert(_vram);
	memset(_vram, 0, 0x10000 * sizeof(uint8));
	setResolution(320, 224);
}

SegaRenderer::~SegaRenderer() {
	clearDirtyRects();
	delete[] _vram;
}

void SegaRenderer::setResolution(int w, int h) {
	assert(w == 320 || w == 256);
	assert(h == 224 || h == 240);

	_screenW = w;
	_screenH = h;
	_blocksW = w >> 3;
	_blocksH = h >> 3;
}

void SegaRenderer::setPlaneTableLocation(int plane, uint16 addr) {
	assert(plane >= kPlaneA && plane <= kWindowPlane);
	assert(addr <= 0xFFFF);
	_planes[plane].nameTable = (uint16*)(&_vram[addr]);
}

void SegaRenderer::setupPlaneAB(int plane, uint16 w, uint16 h) {
	assert(plane >= kPlaneA && plane <= kPlaneB);
	_planes[plane].w = w;
	_planes[plane].h = h;
	_planes[plane].nameTableSize = (w * h) >> 6;
}

void SegaRenderer::setupWindowPlane(int blockX, int blockY, int horizontalMode, int verticalMode) {
	_planes[kWindowPlane].blockX = horizontalMode ? blockX : 0;
	_planes[kWindowPlane].blockY = verticalMode ? blockY : 0;
	_planes[kWindowPlane].w = horizontalMode ? _blocksW - blockX : blockX;
	_planes[kWindowPlane].h = verticalMode ? _blocksH - blockY : blockY;
}

void SegaRenderer::setHScrollTableLocation(int addr) {
	assert(addr <= 0xFFFF);
	_hScrollTable = (uint16*)(&_vram[addr]);
}

void SegaRenderer::setPitch(int pitch) {
	_pitch = pitch;
}

void SegaRenderer::setHScrollMode(int mode) {
	_hScrollMode = mode;
}

void SegaRenderer::loadToVRAM(const uint16 *data, int dataSize, int addr) {
	assert(data);
	assert(addr <= 0xFFFF);
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

void SegaRenderer::fillRectWithTiles(int addr, int x, int y, int w, int h, uint16 nameTblEntry, bool incr) {
	addDirtyRect(x << 3, y << 3, w << 3, h << 3);
	uint16 *dst = (uint16*)(&_vram[addr]);
	dst += (y * _pitch + x);
	if (incr) {
		while (h--) {
			for (int i = w; i; --i)
				*dst++ = nameTblEntry++;
			dst += (_pitch - w);
		}
	} else {
		while (h--) {
			for (int i = w; i; --i)
				*dst++ = nameTblEntry;
			dst += (_pitch - w);
		}
	}
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

	// Planes A and B
	const uint8 order[2] = { kPlaneB, kPlaneA };
	for (int p = 0; p < ARRAYSIZE(order); ++p) {
		uint16 *ntbl = _planes[p].nameTable;
		uint8 *dst = renderBuffer;

		for (int y = 0; y < _blocksH; ++y) {
			int hscrTblIdx = (_hScrollMode == kHScrollFullScreen) ? p : (y << 4) + p;
			uint8 *dst2 = dst;
			for (int x = 0; x < _blocksW; ++x) {
				renderTile(dst, x, ntbl, hscrTblIdx);
				dst += 8;
			}
			ntbl += _pitch;
			dst = dst2 + (_screenW << 3);
		}

		if (_planes[kPlaneA].nameTable == _planes[kPlaneB].nameTable)
			break;
	}

	// Window Plane
	// TODO: implement support if necessary. The following lines are just a modified copy/paste
	//       from above and nowhere guaranteed to work properly.
	/*{
		SegaPlane *p = &_planes[kWindowPlane];
		uint16 *ntbl = p->nameTable;
		uint8 *dst = _renderBuffer;

		for (int y = p->blockY; y < p->blockY + p->h; ++y) {
			uint8 *dst2 = dst;
			for (int x = p->blockX; x < p->blockX + p->w; ++x) {
				renderTile(dst, x, ntbl, -1);
				dst += 8;
			}
			ntbl += _blocksW;
			dst = dst2 + (_screenW << 3);
		}
	}*/

	// Sprites
	// TODO: implement support if necessary.

	// Priority Tiles
	// All tiles that have been skipped so far due to a priority flag will be drawn now.
	/*
	  TODO: implement support if necessary.
	*/

	_screen->copyBlockToPage(destPageNum, 0, 0, _screenW, Screen::SCREEN_H, _renderBuffer);
}

void SegaRenderer::renderTile(uint8 *dst, int destX, uint16 *nameTable, int hScrollTableIndex) {
	for (int bY = 0; bY < 8; ++bY) {
		uint8 *dst2 = dst;
		uint16 hscrNt = 0;
		uint16 hscrPx = 0;

		if (hScrollTableIndex != -1) {
			hscrNt = _hScrollTable[hScrollTableIndex] & 0x3FF;
			hscrPx = hscrNt & 7;
			hscrNt >>= 3;
		}

		uint16 nt = nameTable[(destX + hscrNt) % _pitch];

		// Check all the following things
		// TODO: implement support if necessary.
		// Priority
		assert(!(nt & 0x8000));
		// Vertical flip
		assert(!(nt & 0x1000));
		// Horizontal flip
		assert(!(nt & 0x800));

		renderLineFragment(dst, &_vram[((nt & 0x7FF) << 5) + (bY << 2) + (hscrPx >> 1)], hscrPx, 8, ((nt >> 13) & 3) << 4);

		nt = nameTable[(destX + hscrNt + 1) % _pitch];
		// Priority
		assert(!(nt & 0x8000));
		// Vertical flip
		assert(!(nt & 0x1000));
		// Horizontal flip
		assert(!(nt & 0x800));

		renderLineFragment(dst, &_vram[((nt & 0x7FF) << 5) + (bY << 2)], 0, hscrPx, ((nt >> 13) & 3) << 4);
	
		if (hScrollTableIndex != -1 && _hScrollMode == kHScroll1PixelRows)
			hScrollTableIndex += 2;
		dst = dst2 + _screenW;
	}
}

void SegaRenderer::renderLineFragment(uint8 *&dst, const uint8 *src, int start, int end, uint8 pal) {
	for (int bX = start; bX < end; ++bX) {
		uint8 col = (bX & 1) ? *src++ & 0x0F : *src >> 4;
		if (col)
			*dst = pal | col;
		dst++;
	}
}

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
	for (DRChainEntry *e = _drChain; e; e = e->next) {
		int w = e->rect.width();
		int h = e->rect.height();
		if (w == _screenW && h == _screenH) {
			_screen->_forceFullUpdate = true;
			return;
		} 
		_screen->addDirtyRect(e->rect.left, e->rect.top, w, h);
	}
}

void SegaRenderer::clearDirtyRects() {
	while (_drChain) {
		DRChainEntry *e = _drChain->next;
		delete _drChain;
		_drChain = e;
	}
}

} // End of namespace Kyra

#endif // ENABLE_EOB
