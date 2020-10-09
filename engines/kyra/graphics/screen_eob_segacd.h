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

#ifndef KYRA_SCREEN_EOB_SEGACD_H
#define KYRA_SCREEN_EOB_SEGACD_H

#ifdef ENABLE_EOB

#define SEGA_PERFORMANCE		true
#define SEGA_USE_MEMPOOL		true

#include "kyra/graphics/screen_eob.h"

#if SEGA_USE_MEMPOOL
#include "common/memorypool.h"
#endif

namespace Kyra {

class SegaRenderer {
public:
	enum Plane {
		kPlaneA = 0,
		kPlaneB = 1,
		kWindowPlane = 2
	};

	enum WindowMode {
		kWinToLeft = 0,
		kWinToTop = 0,
		kWinToRight = 1,
		kWinToBottom = 1
	};

	enum HScrollMode {
		kHScrollFullScreen = 0,
		kHScroll8PixelRows,
		kHScroll1PixelRows
	};

	enum VScrollMode {
		kVScrollFullScreen = 0,
		kVScroll16PixelStrips
	};

public:
	SegaRenderer(Screen_EoB *screen);
	~SegaRenderer();

	void setResolution(int w, int h);
	void setPlaneTableLocation(int plane, uint16 addr);
	// The hardware allows/demands separate modification of the vertical and horizontal properties.
	// To allow this without making another function the w/h parameters can be set to -1 which will
	// keep the existing value for that property.
	void setupPlaneAB(int pixelWidth, int pixelHeigth);
	// The hardware allows/demands separate modification of the vertical and horizontal properties.
	// To allow this without making another function the blockX/Y parameters can be set to -1 which
	// will keep the existing value for that property.
	void setupWindowPlane(int blockX, int blockY, int horizontalMode, int verticalMode);
	void setHScrollTableLocation(int addr);
	void setSpriteTableLocation(int addr);
	void setPitch(int pitch);
	void setHScrollMode(int mode);
	void setVScrollMode(int mode);

	void loadToVRAM(const void *data, uint16 dataSize, uint16 addr);
	void loadStreamToVRAM(Common::SeekableReadStream *in, uint16 addr, bool compressedData = false);
	void memsetVRAM(int addr, uint8 val, int len);
	void fillRectWithTiles(int vramArea, int x, int y, int w, int h, uint16 nameTblEntry, bool incr = false, bool topToBottom = false, const uint16 *patternTable = 0);
	void writeUint16VSRAM(int addr, uint16 value);
	void writeUint8VRAM(int addr, uint8 value);
	void writeUint16VRAM(int addr, uint16 value);
	void clearPlanes();

	void render(int destPageNum, int renderLeft = -1, int renderTop = -1, int renderWidth = -1, int renderHeight = -1, bool spritesOnly = false);
private:
	void renderPlanePart(int plane, uint8 *dstBuffer, int x1, int y1, int x2, int y2);
	void renderPlaneTile(uint8 *dst, int destX, const uint16 *nameTable, int vScrollLSBStart, int vScrollLSBEnd, int hScrollTableIndex, uint16 pitch);
	void renderSpriteTile(uint8 *dst, uint8 *mask, int x, int y, uint16 tile, uint8 pal, bool vflip, bool hflip, bool prio);
#if SEGA_PERFORMANCE
	template<bool hflip, bool oddStart, bool oddEnd> void renderLineFragmentM(uint8 *dst, uint8 *mask, const uint8 *src, int start, int end, uint8 pal);
	template<bool hflip, bool oddStart, bool oddEnd> void renderLineFragmentD(uint8 *dst, const uint8 *src, int start, int end, uint8 pal);
	typedef void(SegaRenderer::*renderFuncM)(uint8*, uint8*, const uint8*, int, int, uint8);
	typedef void(SegaRenderer::*renderFuncD)(uint8*, const uint8*, int, int, uint8);
	const renderFuncM *_renderLineFragmentM;
	const renderFuncD *_renderLineFragmentD;
#else
	template<bool hflip> void renderLineFragment(uint8 *dst, uint8 *mask, const uint8 *src, int start, int end, uint8 pal);
#endif

	void initPrioRenderTask(uint8 *dst, uint8 *mask, const uint8 *src, int start, int end, uint8 pal, bool hflip);
	void clearPrioChain();

	struct SegaPlane {
		SegaPlane() : blockX(0), blockY(0), w(0), h(0), mod(0), nameTable(0), nameTableSize(0) {}
		int blockX, blockY;
		uint16 w, h, mod;
		uint16 *nameTable;
		uint16 nameTableSize;
	};

	SegaPlane _planes[3];
	uint8 *_vram;
	uint16 *_vsram;
	uint16 *_hScrollTable;
	uint16 *_spriteTable;
	uint8 *_spriteMask;
	uint8 _hScrollMode;
	uint8 _vScrollMode;
	uint16 _pitch;
	uint16 _numSpritesMax;

	struct PrioTileRenderObj {
		PrioTileRenderObj(PrioTileRenderObj *chainEnd, uint8 *dst, uint8 *mask, const uint8 *src, int start, int end, uint8 pal, bool hflip) :
			_pred(chainEnd), _next(0), _dst(dst), _mask(mask), _src(src), _start(start), _end(end), _pal(pal), _hflip(hflip) {
			if (_pred)
				_pred->_next = this;
		}
		uint8 *_dst;
		uint8 *_mask;
		const uint8 *_src;
		int _start;
		int _end;
		uint8 _pal;
		bool _hflip;
		PrioTileRenderObj *_pred;
		PrioTileRenderObj *_next;
	};

#if SEGA_USE_MEMPOOL
	Common::ObjectPool<PrioTileRenderObj> _prioRenderMemPool;
#endif
	PrioTileRenderObj *_prioChainStart, *_prioChainEnd;
	uint16 _screenW, _screenH, _blocksW, _blocksH;
	Screen_EoB *_screen;
};

class SegaAnimator {
public:
	SegaAnimator(SegaRenderer *renderer);
	~SegaAnimator();

	void initSprite(int id, int16 x, int16 y, uint16 nameTbl, uint16 hw);
	void clearSprites();
	void moveMorphSprite(int id, uint16 nameTbl, int16 addX, int16 addY);
	void moveSprites(int id, uint16 num, int16 addX, int16 addY);
	void moveSprites2(int id, uint16 num, int16 addX, int16 addY);

	void update();

private:
	struct Sprite {
		int16 x;
		int16 y;
		uint16 nameTbl;
		uint16 hw;
	};

	uint16 *_tempBuffer;
	Sprite *_sprites;
	SegaRenderer *_renderer;
	bool _needUpdate;
};

class ScrollManager {
public:
	ScrollManager(SegaRenderer *renderer);
	~ScrollManager();

	void setVScrollTimers(uint16 destA, int incrA, int delayA, uint16 destB, int incrB, int delayB);
	void setHScrollTimers(uint16 destA, int incrA, int delayA, uint16 destB, int incrB, int delayB);
	void updateScrollTimers();
	void fastForward();

private:
	struct ScrollTimer {
		ScrollTimer() : _offsCur(0), _offsDest(0), _incr(0), _delay(0), _timer(0) {}
		int16 _offsCur;
		int16 _offsDest;
		int16 _incr;
		int16 _delay;
		int16 _timer;
	};

	ScrollTimer *_vScrollTimers;
	ScrollTimer *_hScrollTimers;
	SegaRenderer *_renderer;
};

} // End of namespace Kyra

#endif // ENABLE_EOB

#endif
