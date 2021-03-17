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

#ifndef GRAPHICS_SEGAGFX_H
#define GRAPHICS_SEGAGFX_H

#define SEGA_PERFORMANCE		true
#define SEGA_USE_MEMPOOL		true

#if SEGA_USE_MEMPOOL
#include "common/memorypool.h"
#endif

namespace Graphics {

struct Surface;

/**
 * SegaRenderer
 *
 * This class allows rendering the content of a virtual Sega VDP video ram (including all planes with their respective
 * coords and scroll states and including sprites) to a "normal" CLUT8 buffer. It can be used as a base class to have
 * better access to the vram (see e. g. EOB I SegaCD). The renderer handles the 4 palettes of the VDP (with 16 colors
 * each) by flagging the pixels with (paletteNo << 4), e. g. a pixel of palette 1, color 5 will be rendered as 0x15.
 * This has to be kept in mind when writing the palette code for your engine. Also, there is no parsing of control
 * register opcodes to set up plane dimensions, table locations etc., since I didn't see any need for that. This has
 * to be done through the public setup methods of the renderer.
 */
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
	SegaRenderer();
	~SegaRenderer();

	/**
	 * Set target render buffer resolution for internal purposes of this renderer only. It will not interact in any way with the ScummVM backend.
	 * @param w:		pixel width, 320 or 256 allowed
	 * @param h:		pixel height, 224 or 240 allowed
	 */
	void setResolution(int w, int h);

	/**
	 * Set address for a plane's nametable.
	 * @param plane:	plane id (from Plane enum)
	 * @param addr:		address in the vdp vram
	 */
	void setPlaneTableLocation(int plane, uint16 addr);

	/**
	 * Set pixel width and/or height for planes A and B.
	 * @param pixelWidth:	1024, 512 or 256 allowed
	 * @param pixelHeight:	1024, 512 or 256 allowed
	 *
	 * The hardware allows/demands separate modification of the vertical and horizontal properties.
	 * To allow this without making another function one of the pixelWidth/pixelHeight parameters
	 * can be set to -1 which will keep the existing value for that property.
	 */
	void setupPlaneAB(int pixelWidth, int pixelHeigth);

	/**
	 * Set pixel width and/or height and drawing mode for the window plane.
	 * @param pixelWidth:		1024, 512 or 256 allowed
	 * @param pixelHeight:		1024, 512 or 256 allowed
	 * @param horizontalMode:	kWinToLeft or kWinToRight
	 * @param verticalMode:		kWinToTop or kWinToBottom
	 *
	 * The hardware allows/demands separate modification of the vertical and horizontal properties.
	 * To allow this without making another function one of the pixelWidth/pixelHeight parameters
	 * can be set to -1 which will keep the existing value for that property.
	 */
	void setupWindowPlane(int blockX, int blockY, int horizontalMode, int verticalMode);

	/**
	 * Set address for the horizontal scroll table.
	 * @param addr:		address in the vdp vram
	 */
	void setHScrollTableLocation(int addr);

	/**
	 * Set address for the sprite table.
	 * @param addr:		address in the vdp vram
	 */
	void setSpriteTableLocation(int addr);

	/**
	 * Set plane pitch in (8 pixels wide) blocks.
	 * @param pitch:	pitch
	 */
	void setPitch(int pitch);

	/**
	 * Set horizontal scroll mode.
	 * @param mode:		see enum HScrollMode
	 */
	void setHScrollMode(int mode);

	/**
	 * Set vertical scroll mode.
	 * @param mode:		see enum VScrollMode
	 */
	void setVScrollMode(int mode);

	/**
	 * Load data from a buffer to a vram address.
	 * @param data:		source data ptr
	 * @param len:		byte size of data
	 * @param addr:		target address in the vdp vram
	 */
	void loadToVRAM(const void *data, uint16 len, uint16 addr);

	/**
	 * Fill vram area with bytes of the specified value.
	 * @param addr:		target address in the vdp vram
	 * @param val:		fill value
	 * @param len:		byte size of fill area
	 */
	void memsetVRAM(int addr, uint8 val, int len);

	/**
	 * Write uint16 value into vs ram. No endianness corrections take place here.
	 * @param addr:		target address in the vdp vs ram
	 * @param val:		value
	 */
	void writeUint16VSRAM(int addr, uint16 value);

	/**
	 * Write uint8 value into vram.
	 * @param addr:		target address in the vdp vram
	 * @param val:		value
	 */
	void writeUint8VRAM(int addr, uint8 value);

	/**
	 * Write uint16 value into vram. No endianness corrections take place here.
	 * @param addr:		target address in the vdp vram
	 * @param val:		value
	 */
	void writeUint16VRAM(int addr, uint16 value);

	/**
	 * Fills all planes with 0.
	 */
	void clearPlanes();

	/**
	 * Render vram in its current state to the target buffer/surface.
	 * @param surf/dest:		target surface or buffer
	 * @param renderBlockX, renderBlockY, renderBlockWidth, renderBlockHeight:
								block (8x8 pixels) coordinates of target render area. Values of -1 for maximum range / full screen rendering.
	 * @param spritesOnly:		skip rendering of the planes and only render the sprites.
	 */
	void render(Surface *surf, int renderBlockX = -1, int renderBlockY = -1, int renderBlockWidth = -1, int renderBlockHeight = -1, bool spritesOnly = false);
	void render(uint8 *dest, int renderBlockX = -1, int renderBlockY = -1, int renderBlockWidth = -1, int renderBlockHeight = -1, bool spritesOnly = false);

protected:
	uint16 _screenW, _screenH, _blocksW, _blocksH;
	uint8 *_vram;
	uint16 *_vsram;
	uint16 _pitch;

private:
	void fillBackground(uint8 *dst, int x, int y, int w, int h);
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
	uint16 *_hScrollTable;
	uint16 *_spriteTable;
	uint8 *_spriteMask;
	uint8 _hScrollMode;
	uint8 _vScrollMode;
	uint16 _numSpritesMax;
	uint8 _backgroundColor;

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
};

} // End of namespace Graphics

#endif // GRAPHICS_SEGAGFX_H
