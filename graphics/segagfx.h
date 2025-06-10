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

#include "common/func.h"
#include "graphics/pixelformat.h"

// It could probably be disabled if the backend does this in satisfactory way
#define BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT				true

namespace Graphics {

struct PixelFormat;
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
	SegaRenderer(const PixelFormat *systemPixelFormat = nullptr);
	~SegaRenderer();

	/**
	 * Set target render buffer resolution for internal purposes of this renderer only. It will not interact in any way with the ScummVM backend.
	 * @param w:		pixel width, 320 or 256 allowed
	 * @param h:		pixel height, 224 or 240 allowed
	 */
	void setResolution(int w, int h);

#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
	/**
	 * Enable or disable aspect ratio correction. This can be achieved rather cost-effectively with a scale factor of 1.0 (no scaling).
	 * Unfortunately, some of the relevant resolutions here aren't really made for this. While it may be visually bearable to correct a 320x224
	 * image to 320x240, this will not be the case for Snatcher's 256x224 resolution. The text font glyphs will be distorted in a very ugly
	 * and obvious way.That's why we have the scaling. Set it to 2.0 and the corrected image will be okay. Keep in mind though, how this will
	 * interact with and/or impact a scaler or shader setting. So it might make sense not to overdo it with the scale factor (apart from
	 * the obvious, that the aspect ratio correction will not come anywhere near as cheap any more).
	 * 
	 * @param xyAspectRatio:		desired x/y ratio. E. g., for 4 : 3, type '4.0 / 3.0'. To turn it off, use 1.0.
	 * @param scaleFactor:			scale factor for the aspect ratio correction, e. g. 1.0 for no scaling, 2.0 for double size
	 */
	void setAspectRatioCorrection(double xyAspectRatio = 1.0, double scaleFactor = 1.0);
#endif

	/**
	 * Get the screen pixel width and height required to init the backend. These will be the same values as passed to setResolution() if the aspect
	 * ratio correction is set to off, but it will differ when that option is enabled.
	 * @param w:		pixel width,
	 * @param h:		pixel height
	 */
	void getRealResolution(int &w, int &h);

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
	 * @param baseX:				base x coordinate (in 16 px units)
	 * @param baseY:				base y coordinate (in  8 px units)
	 * @param horizontalMode:		kWinToLeft or kWinToRight (window is either from the left of the screen to base x or from base x to the right of the screen)
	 * @param verticalMode:			kWinToTop or kWinToBottom (window is either from the top of the screen to base y or from base y to the bottom of the screen)
	 *
	 * The hardware allows/demands separate modification of the vertical and horizontal properties.
	 * To allow this without making another function one of the pixelWidth/pixelHeight parameters
	 * can be set to -1 which will keep the existing value for that property.
	 */
	void setupWindowPlane(int baseX, int baseY, int horizontalMode, int verticalMode);

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
	 * Enable display or fill with background color.
	 * @param enable:	true = display, false = fill with background color
	 */
	void enableDisplay(bool enable);

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
	 * Write uint16 value into vs ram. Values must be BE.
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
	 * Write uint16 value into vram. Values must be BE.
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
	//void render(Surface *surf, int renderBlockX = -1, int renderBlockY = -1, int renderBlockWidth = -1, int renderBlockHeight = -1, bool spritesOnly = false);
	//void render(uint8 *dest, int renderBlockX = -1, int renderBlockY = -1, int renderBlockWidth = -1, int renderBlockHeight = -1, bool spritesOnly = false);

	/**
	 * Render vram in its current state to the target buffer/surface.
	 * @param surf/dst:		Target surface or buffer.
	 * @param x, y, w, h:		Pixel coordinates of source and target render area. Values of -1 for maximum range / full screen rendering.
	 */
	void render(void *dst, int x = -1, int y = -1, int w = -1, int h = -1);

	/**
	 * Render sprites only
	 * @param dst:								Target buffer for non-prio sprites.
	 * @param dstPrio:							Target buffer for prio sprites.
	 * @param clipX, clipY, clipW, clipH:		Pixel coordinates of source and target render area. Values of -1 for maximum range / full screen rendering.
	 */
	void renderSprites(uint8 *dst, uint8 *dstPrio, int clipX = -1, int clipY = -1, int clipW = -1, int clipH = -1);

	/**
	 * Refresh a part of the target buffer/surface with the latest render() result.
	 * The only purpose of this function (at least for me) is to convert the pixels of non-8bit dest buffers/surfaces to a changed palette.
	 * @param dst:				Target surface or buffer.
	 * @param x, y, w, h:		Pixel coordinates of target render area. Values of -1 for maximum range / full screen refresh.
	 */
	template<typename T> void refresh(T *dest, int x = -1, int y = -1, int w = -1, int h = -1);

	/**
	 * Set palette for render operations on non-8bit dest buffers/surfaces
	 * @param colors:			8bit palette for the final color conversion of the rendered surface/buffer.
	 * @param numColors:		Number of colors in the palette.
	 */
	void setRenderColorTable(const uint8 *colors, uint16 first, uint16 num);

public:
	class HINTClient {
	public:
		virtual ~HINTClient() {}
		virtual void hINTCallback(SegaRenderer *sr) = 0;
	};
	typedef Common::Functor1Mem<SegaRenderer*, void, HINTClient> HINTHandler;

	/**
	 * Horizontal Interrupt
	 * @hINT_enable:			enable/disable
	 * @hINT_setCounter			number of scan lines between hINT handler invocation
	 * @hINT_setHandler			set hINT handler callback
	 */
	void hINT_enable(bool enable);
	void hINT_setCounter(uint8 counter);
	void hINT_setHandler(const HINTHandler *hdl);

protected:
	uint16 _screenW, _screenH, _blocksW, _blocksH;
	uint16 _realW, _realH;
	uint16 _destPitch;
	uint8 *_vram;
	uint16 *_vsram;
	uint16 _pitch;

private:
	void fillBackground(uint8 *dst, int x, int y, int w, int h);
	template<bool isWindow> void renderPlaneLine(uint8 *planeBuffPos, uint8 *planePrioBuffPos, int srcPlane, int y, int clipX, int clipW);
	template<typename T, bool withSprites, bool withPrioSprites, bool adjustAspectRatio> void mergeLines(void *dst, uint8 *bottomLayer, const uint8 *spriteLayer, const uint8 *prioLayer, const uint8 *prioSpriteLayer, int clipW);

	typedef void (SegaRenderer::*MergeLinesProc)(void*, uint8*, const uint8*, const uint8*, const uint8*, int);
	MergeLinesProc getLineHandler() const;
	void hINTUpdate();

	void renderSpriteTileXClipped(uint8 *dst, uint8 *mask, int x, int y, uint16 tile, uint8 pal, bool vflip, bool hflip);
	void renderSpriteTileDef(uint8 *dst, uint8 *mask, int x, int y, uint16 tile, uint8 pal, bool vflip, bool hflip);
	typedef void (SegaRenderer::*SprTileProc)(uint8*, uint8*, int, int, uint16, uint8, bool, bool);

	template<bool hflip, bool oddStart, bool oddEnd> void renderLineFragmentM(uint8 *dst, uint8 *mask, const uint8 *src, int start, int end, uint8 pal);
	template<bool hflip, bool oddStart, bool oddEnd> void renderLineFragmentD(uint8 *dst, const uint8 *src, int start, int end, uint8 pal);
	typedef void(SegaRenderer::*renderFuncM)(uint8*, uint8*, const uint8*, int, int, uint8);
	typedef void(SegaRenderer::*renderFuncD)(uint8*, const uint8*, int, int, uint8);
	const renderFuncM *_renderLineFragmentM;
	const renderFuncD *_renderLineFragmentD;

	struct SegaPlane {
		SegaPlane() : blockX(0), blockY(0), w(0), h(0), mod(0), pitch(0), nameTable(0), nameTableSize(0) {}
		int blockX, blockY;
		uint16 w, h, mod, pitch;
		uint16 *nameTable;
		uint16 nameTableSize;
	};

	SegaPlane _planes[3];
	uint16 *_hScrollTable;
	uint16 *_spriteTable;
	uint8 *_spriteMask;
	uint8 *_spriteMaskPrio;
	uint8 _hScrollMode;
	uint8 _vScrollMode;
	uint16 _numSpritesMax;
	uint8 _backgroundColor;

	uint8 *_tempBufferSprites;
	uint8 *_tempBufferSpritesPrio;
	uint8 *_tempBufferPlanes;
	uint8 *_tempBufferPlanesPrio;
	uint32 *_renderColorTable;

	const PixelFormat _pixelFormat;
	bool _displayEnabled;

	enum RenderResult : uint16 {
		kUnspecified		=		0,
		kHasSprites			=		1	<<	0,
		kHasPrioSprites		=		1	<<	1
	};

	uint16 _rr;

	bool _hINTEnable;
	uint8 _hINTCounterNext;
	uint8 _hINTCounter;
	const HINTHandler *_hINTHandler;

#if(BUILD_SEGAGFX_ASPECT_RATIO_CORRECTION_SUPPORT)
	// Aspect ratio correction
	uint32 _arcIncX, _arcIncY, _arcCntX, _arcCntY;
	double _xyAspectRatio, _scaleFactor;
#endif
};

} // End of namespace Graphics

#endif // GRAPHICS_SEGAGFX_H
