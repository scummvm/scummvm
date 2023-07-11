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

#ifndef SCUMM_GFX_H
#define SCUMM_GFX_H

#include "common/system.h"
#include "common/list.h"

#include "graphics/surface.h"

namespace Scumm {

class ScummEngine;

/*
 *  Bitmap compression algorithm labels
 */

#define BMCOMP_RAW256           1   // Raw pixels
#define BMCOMP_TOWNS_2          2
#define BMCOMP_TOWNS_3          3
#define BMCOMP_TOWNS_4          4
#define BMCOMP_TOWNS_7          7
#define BMCOMP_TRLE8BIT         8
#define BMCOMP_RLE8BIT          9

#define BMCOMP_PIX32            10  // Amiga 16/32 color compression

#define BMCOMP_ZIGZAG_V0        10  // Vertical
#define BMCOMP_ZIGZAG_V4        14
#define BMCOMP_ZIGZAG_V5        15
#define BMCOMP_ZIGZAG_V6        16
#define BMCOMP_ZIGZAG_V7        17
#define BMCOMP_ZIGZAG_V8        18

#define BMCOMP_ZIGZAG_H0        20  // Horizontal
#define BMCOMP_ZIGZAG_H4        24
#define BMCOMP_ZIGZAG_H5        25
#define BMCOMP_ZIGZAG_H6        26
#define BMCOMP_ZIGZAG_H7        27
#define BMCOMP_ZIGZAG_H8        28

#define BMCOMP_ZIGZAG_VT0       30  // Vertical with transparency
#define BMCOMP_ZIGZAG_VT4       34
#define BMCOMP_ZIGZAG_VT5       35
#define BMCOMP_ZIGZAG_VT6       36
#define BMCOMP_ZIGZAG_VT7       37
#define BMCOMP_ZIGZAG_VT8       38

#define BMCOMP_ZIGZAG_HT0       40  // Horizontal with transparency
#define BMCOMP_ZIGZAG_HT4       44
#define BMCOMP_ZIGZAG_HT5       45
#define BMCOMP_ZIGZAG_HT6       46
#define BMCOMP_ZIGZAG_HT7       47
#define BMCOMP_ZIGZAG_HT8       48

#define BMCOMP_MAJMIN_H0        60  // Major-Minor jump algorithm (-4 to +3)
#define BMCOMP_MAJMIN_H4        64
#define BMCOMP_MAJMIN_H5        65
#define BMCOMP_MAJMIN_H6        66
#define BMCOMP_MAJMIN_H7        67
#define BMCOMP_MAJMIN_H8        68

#define BMCOMP_MAJMIN_HT0       80  // Major-Minor jump algorithm (-4 to +3, with transparency)
#define BMCOMP_MAJMIN_HT4       84
#define BMCOMP_MAJMIN_HT5       85
#define BMCOMP_MAJMIN_HT6       86
#define BMCOMP_MAJMIN_HT7       87
#define BMCOMP_MAJMIN_HT8       88

#define BMCOMP_RMAJMIN_H0       100 // Run Major-Minor jump algorithm (-4 to +3)
#define BMCOMP_RMAJMIN_H4       104
#define BMCOMP_RMAJMIN_H5       105
#define BMCOMP_RMAJMIN_H6       106
#define BMCOMP_RMAJMIN_H7       107
#define BMCOMP_RMAJMIN_H8       108

#define BMCOMP_RMAJMIN_HT0      120 // Run Major-Minor jump algorithm (-4 to +3, with transparency)
#define BMCOMP_RMAJMIN_HT4      124
#define BMCOMP_RMAJMIN_HT5      125
#define BMCOMP_RMAJMIN_HT6      126
#define BMCOMP_RMAJMIN_HT7      127
#define BMCOMP_RMAJMIN_HT8      128

#define BMCOMP_NMAJMIN_H0       130 // New Major-Minor jump algorithm (-4 to +4)
#define BMCOMP_NMAJMIN_H4       134
#define BMCOMP_NMAJMIN_H5       135
#define BMCOMP_NMAJMIN_H6       136
#define BMCOMP_NMAJMIN_H7       137
#define BMCOMP_NMAJMIN_H8       138

#define BMCOMP_NMAJMIN_HT0      140 // New Major-Minor jump algorithm (-4 to +4, with transparency)
#define BMCOMP_NMAJMIN_HT4      144
#define BMCOMP_NMAJMIN_HT5      145
#define BMCOMP_NMAJMIN_HT6      146
#define BMCOMP_NMAJMIN_HT7      147
#define BMCOMP_NMAJMIN_HT8      148

#define BMCOMP_TPIX256          149 // Transparent raw pixels

#define BMCOMP_SOLID_COLOR_FILL 150

#define BMCOMP_CUSTOM_RU_TR     143


enum HerculesDimensions {
	kHercWidth = 720,
	kHercHeight = 350
};

/** Camera modes */
enum {
	kNormalCameraMode = 1,
	kFollowActorCameraMode = 2,
	kPanningCameraMode = 3
};

/** Camera state data */
struct CameraData {
	Common::Point _cur;
	Common::Point _dest;
	Common::Point _accel;
	Common::Point _last;
	int _leftTrigger, _rightTrigger;
	byte _follows, _mode;
	bool _movingToActor;

	void reset() {
		_cur.x = _cur.y = 0;
		_dest.x = _dest.y = 0;
		_accel.x = _accel.y = 0;
		_last.x = _last.y = 0;
		_leftTrigger = 0;
		_rightTrigger = 0;
		_follows = 0;
		_mode = 0;
		_movingToActor = 0;
	}
};

/** Virtual screen identifiers */
enum VirtScreenNumber {
	kMainVirtScreen = 0,	// The 'stage'
	kTextVirtScreen = 1,	// In V0-V3 games: the area where text is printed
	kVerbVirtScreen = 2,	// The verb area
	kUnkVirtScreen = 3		// ?? Not sure what this one is good for...
};

/**
 * In all Scumm games, one to four virtual screen (or 'windows') together make
 * up the content of the actual screen. Thinking of virtual screens as fixed
 * size, fixed location windows might help understanding them. Typical, in all
 * scumm games there is either one single virtual screen covering the entire
 * real screen (mostly in all newer games, e.g. Sam & Max, and all V7+ games).
 * The classic setup consists of three virtual screens: one at the top of the
 * screen, where all conversation texts are printed; then the main one (which
 * I like calling 'the stage', since all the actors are doing their stuff
 * there), and finally the lower part of the real screen is taken up by the
 * verb area.
 * Finally, in V5 games and some V6 games, it's almost the same as in the
 * original games, except that there is no separate conversation area.
 *
 * If you now wonder what the last screen is/was good for: I am not 100% sure,
 * but it appears that it was used by the original engine to display stuff
 * like the pause message, or questions ("Do you really want to restart?").
 * It seems that it is not used at all by ScummVM, so we probably could just
 * get rid of it and save a couple kilobytes of RAM.
 *
 * Each of these virtual screens has a fixed number or id (see also
 * \ref VirtScreenNumber).
 */
struct VirtScreen : Graphics::Surface {
	/**
	 * The unique id of this screen (corresponds to its position in the
	 * ScummEngine:_virtscr array).
	 */
	VirtScreenNumber number;

	/**
	 * Vertical position of the virtual screen. Tells how much the virtual
	 * screen is shifted along the y axis relative to the real screen.
	 */
	uint16 topline;

	/**
	 * Horizontal scroll offset, tells how far the screen is scrolled to the
	 * right. Only used for the main screen. After all, verbs and the
	 * conversation text box don't have to scroll.
	 */
	uint16 xstart;

	/**
	 * Flag indicating whether this screen has a back buffer or not. This is
	 * yet another feature which is only used by the main screen.
	 * Strictly spoken one could remove this variable and replace checks
	 * on it with checks on backBuf. But since some code needs to temporarily
	 * disable the backBuf (so it can abuse drawBitmap; see drawVerbBitmap()
	 * and useIm01Cursor()), we keep it (at least for now).
	 */
	bool hasTwoBuffers;

	/**
	 * Pointer to the screen's back buffer, if it has one (see also
	 * the hasTwoBuffers member).
	 * The backBuf is used by drawBitmap to store the background graphics of
	 * the active room. This eases redrawing: whenever a portion of the screen
	 * has to be redrawn, first a copy from the backBuf content to screenPtr is
	 * performed. Then, any objects/actors in that area are redrawn atop that.
	 */
	byte *backBuf;

	/**
	 * Array containing for each visible strip of this virtual screen the
	 * coordinate at which the dirty region of that strip starts.
	 * 't' stands for 'top' - the top coordinate of the dirty region.
	 * This together with bdirty is used to do efficient redrawing of
	 * the screen.
	 */
	uint16 tdirty[80 + 1];

	/**
	 * Array containing for each visible strip of this virtual screen the
	 * coordinate at which the dirty region of that strip end.
	 * 'b' stands for 'bottom' - the bottom coordinate of the dirty region.
	 * This together with tdirty is used to do efficient redrawing of
	 * the screen.
	 */
	uint16 bdirty[80 + 1];

	void clear() {
		// FIXME: Call Graphics::Surface clear / constructor?
		number = kMainVirtScreen;
		topline = 0;
		xstart = 0;
		hasTwoBuffers = false;
		backBuf = nullptr;
		for (uint i = 0; i < ARRAYSIZE(tdirty); i++) tdirty[i] = 0;
		for (uint i = 0; i < ARRAYSIZE(bdirty); i++) bdirty[i] = 0;
	}

	/**
	 * Convenience method to set the whole tdirty and bdirty arrays to one
	 * specific value each. This is mostly used to mark every as dirty in
	 * a single step, like so:
	 *   vs->setDirtyRange(0, vs->height);
	 * or to mark everything as clean, like so:
	 *   vs->setDirtyRange(0, 0);
	 */
	void setDirtyRange(int top, int bottom) {
		for (int i = 0; i < 80 + 1; i++) {
			tdirty[i] = top;
			bdirty[i] = bottom;
		}
	}

	byte *getPixels(int x, int y) const {
		return (byte *)pixels + y * pitch + (xstart + x) * format.bytesPerPixel;
	}

	byte *getBackPixels(int x, int y) const {
		return (byte *)backBuf + y * pitch + (xstart + x) * format.bytesPerPixel;
	}
};

/** Palette cycles */
struct ColorCycle {
	uint16 delay;
	uint16 counter;
	uint16 flags;
	byte start;
	byte end;
};

struct StripTable;

#define CHARSET_MASK_TRANSPARENCY	 0xFD
#define CHARSET_MASK_TRANSPARENCY_32 0xFDFDFDFD

class Gdi {
protected:
	ScummEngine *_vm;

	byte _paletteMod;
	byte *_roomPalette;
	byte _transparentColor;
	byte _decomp_shr, _decomp_mask;
	uint32 _vertStripNextInc;

	bool _zbufferDisabled;

	/** Flag which is true when an object is being rendered, false otherwise. */
	bool _objectMode;

public:
	/** Flag which is true when loading objects or titles for distaff, in PCEngine version of Loom. */
	bool _distaff;

	int _numZBuffer;
	int _imgBufOffs[8];
	int32 _numStrips;

protected:
	/* Bitmap decompressors */
	bool decompressBitmap(byte *dst, int dstPitch, const byte *src, int numLinesToProcess);

	void drawStripEGA(byte *dst, int dstPitch, const byte *src, int height) const;

	void drawStripComplex(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const;
	void drawStripBasicH(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const;
	void drawStripBasicV(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const;

	void drawStripRaw(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const;
	void unkDecode8(byte *dst, int dstPitch, const byte *src, int height) const;
	void unkDecode9(byte *dst, int dstPitch, const byte *src, int height) const;
	void unkDecode10(byte *dst, int dstPitch, const byte *src, int height) const;
	void unkDecode11(byte *dst, int dstPitch, const byte *src, int height) const;
	void drawStrip3DO(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const;

	void drawStripHE(byte *dst, int dstPitch, const byte *src, int width, int height, const bool transpCheck) const;
	virtual void writeRoomColor(byte *dst, byte color) const;

	/* Mask decompressors */
	void decompressMaskImgOr(byte *dst, const byte *src, int height) const;
	void decompressMaskImg(byte *dst, const byte *src, int height) const;

	/* Misc */
	int getZPlanes(const byte *smap_ptr, const byte *zplane_list[9], bool bmapImage) const;

	virtual bool drawStrip(byte *dstPtr, VirtScreen *vs,
					int x, int y, const int width, const int height,
					int stripnr, const byte *smap_ptr);

	virtual void decodeMask(int x, int y, const int width, const int height,
	                int stripnr, int numzbuf, const byte *zplane_list[9],
	                bool transpStrip, byte flag);

	virtual void prepareDrawBitmap(const byte *ptr, VirtScreen *vs,
					const int x, const int y, const int width, const int height,
	                int stripnr, int numstrip);

public:
	Gdi(ScummEngine *vm);
	virtual ~Gdi();

	virtual void setRenderModeColorMap(const byte *map) {}
	virtual byte remapColorToRenderMode(byte col) const { return col; }

	virtual void init();
	virtual void roomChanged(byte *roomptr);
	virtual void loadTiles(byte *roomptr);
	void setTransparentColor(byte transparentColor) { _transparentColor = transparentColor; }

	void drawBitmap(const byte *ptr, VirtScreen *vs, int x, int y, const int width, const int height,
	                int stripnr, int numstrip, byte flag);

#ifdef ENABLE_HE
	void drawBMAPBg(const byte *ptr, VirtScreen *vs);
	void drawBMAPObject(const byte *ptr, VirtScreen *vs, int obj, int x, int y, int w, int h);
#endif

	byte *getMaskBuffer(int x, int y, int z);
	void disableZBuffer() { _zbufferDisabled = true; }
	void enableZBuffer() { _zbufferDisabled = false; }

	void resetBackground(int top, int bottom, int strip);

	enum DrawBitmapFlags {
		dbAllowMaskOr   = 1 << 0,
		dbDrawMaskOnAll = 1 << 1,
		dbObjectMode    = 2 << 2
	};
};

class GdiHE : public Gdi {
protected:
	const byte *_tmskPtr;

protected:
	void decompressTMSK(byte *dst, const byte *tmsk, const byte *src, int height) const;

	void decodeMask(int x, int y, const int width, const int height,
	                int stripnr, int numzbuf, const byte *zplane_list[9],
	                bool transpStrip, byte flag) override;

	void prepareDrawBitmap(const byte *ptr, VirtScreen *vs,
					const int x, const int y, const int width, const int height,
	                int stripnr, int numstrip) override;
public:
	GdiHE(ScummEngine *vm);
};

class GdiNES : public Gdi {
protected:
	struct {
		byte nametable[16][64], nametableObj[16][64];
		byte attributes[64], attributesObj[64];
		byte masktable[16][8], masktableObj[16][8];
		int  objX;
		bool hasmask;
	} _NES;

protected:
	void decodeNESGfx(const byte *room);
	void decodeNESObject(const byte *ptr, int xpos, int ypos, int width, int height);

	void drawStripNES(byte *dst, byte *mask, int dstPitch, int stripnr, int top, int height);
	void drawStripNESMask(byte *dst, int stripnr, int top, int height) const;

	bool drawStrip(byte *dstPtr, VirtScreen *vs,
					int x, int y, const int width, const int height,
					int stripnr, const byte *smap_ptr) override;

	void decodeMask(int x, int y, const int width, const int height,
	                int stripnr, int numzbuf, const byte *zplane_list[9],
	                bool transpStrip, byte flag) override;

	void prepareDrawBitmap(const byte *ptr, VirtScreen *vs,
					const int x, const int y, const int width, const int height,
	                int stripnr, int numstrip) override;

public:
	GdiNES(ScummEngine *vm);

	void roomChanged(byte *roomptr) override;
};

#ifdef USE_RGB_COLOR
class GdiPCEngine : public Gdi {
protected:
	struct {
		uint16 nametable[4096], nametableObj[512];
		byte colortable[4096], colortableObj[512];
		uint16 masktable[4096], masktableObj[512];
		int maskIDSize;
		int numTiles;
		int numMasks;
		byte *roomTiles, *staffTiles;
		byte *masks;
	} _PCE;

protected:
	void decodePCEngineGfx(const byte *room);
	void decodeStrip(const byte *ptr, uint16 *tiles, byte *colors, uint16 *masks, int numRows, bool isObject);
	void setTileData(byte *tile, int index, byte byte0, byte byte1);
	void decodePCEngineTileData(const byte *ptr);
	void decodePCEngineMaskData(const byte *ptr);
	void decodePCEngineObject(const byte *ptr, int xpos, int ypos, int width, int height);

	void drawStripPCEngine(byte *dst, byte *mask, int dstPitch, int stripnr, int top, int height);
	void drawStripPCEngineMask(byte *dst, int stripnr, int top, int height) const;

	bool drawStrip(byte *dstPtr, VirtScreen *vs,
					int x, int y, const int width, const int height,
					int stripnr, const byte *smap_ptr) override;

	void decodeMask(int x, int y, const int width, const int height,
	                int stripnr, int numzbuf, const byte *zplane_list[9],
	                bool transpStrip, byte flag) override;

	void prepareDrawBitmap(const byte *ptr, VirtScreen *vs,
					const int x, const int y, const int width, const int height,
	                int stripnr, int numstrip) override;

public:
	GdiPCEngine(ScummEngine *vm);
	~GdiPCEngine() override;

	void loadTiles(byte *roomptr) override;
	void roomChanged(byte *roomptr) override;
};
#endif

class GdiV1 : public Gdi {
protected:
	/** Render settings which are specific to the v0/v1 graphic decoders. */
	struct {
		byte colors[4];
		byte charMap[2048], objectMap[2048], picMap[4096], colorMap[4096];
		byte maskMap[4096], maskChar[4096];
	} _V1;

	const byte *_colorMap = 0;

protected:
	void decodeV1Gfx(const byte *src, byte *dst, int size) const;

	void drawStripV1Object(byte *dst, int dstPitch, int stripnr, int width, int height);
	void drawStripV1Background(byte *dst, int dstPitch, int stripnr, int height);
	void drawStripV1Mask(byte *dst, int stripnr, int width, int height) const;

	bool drawStrip(byte *dstPtr, VirtScreen *vs,
					int x, int y, const int width, const int height,
					int stripnr, const byte *smap_ptr) override;

	void decodeMask(int x, int y, const int width, const int height,
	                int stripnr, int numzbuf, const byte *zplane_list[9],
	                bool transpStrip, byte flag) override;

	void prepareDrawBitmap(const byte *ptr, VirtScreen *vs,
					const int x, const int y, const int width, const int height,
	                int stripnr, int numstrip) override;

public:
	GdiV1(ScummEngine *vm);

	void setRenderModeColorMap(const byte *map) override;
	byte remapColorToRenderMode(byte col) const override;

	void roomChanged(byte *roomptr) override;
};

class GdiV2 : public Gdi {
protected:
	/** For V2 games, we cache offsets into the room graphics, to speed up things. */
	StripTable *_roomStrips;

protected:
	StripTable *generateStripTable(const byte *src, int width, int height, StripTable *table) const;

	bool drawStrip(byte *dstPtr, VirtScreen *vs,
					int x, int y, const int width, const int height,
					int stripnr, const byte *smap_ptr) override;

	void decodeMask(int x, int y, const int width, const int height,
	                int stripnr, int numzbuf, const byte *zplane_list[9],
	                bool transpStrip, byte flag) override;

	void prepareDrawBitmap(const byte *ptr, VirtScreen *vs,
					const int x, const int y, const int width, const int height,
	                int stripnr, int numstrip) override;

public:
	GdiV2(ScummEngine *vm);
	~GdiV2() override;

	void roomChanged(byte *roomptr) override;
};

#ifdef USE_RGB_COLOR
class GdiHE16bit : public GdiHE {
protected:
	void writeRoomColor(byte *dst, byte color) const override;
public:
	GdiHE16bit(ScummEngine *vm);
};
#endif

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
// Helper class for FM-Towns output (required for specific hardware effects like switching graphics layers on and off).
class TownsScreen {
public:
	enum {
		kDirtyRectsMax = 20,
		kFullRedraw = (kDirtyRectsMax + 1)
	};
public:
	TownsScreen(OSystem *system);
	~TownsScreen();

	void setupLayer(int layer, int width, int height, int scaleW, int scaleH, int numCol, void *srcPal = 0);
	void clearLayer(int layer);
	void fillLayerRect(int layer, int x, int y, int w, int h, int col);
	void addDirtyRect(int x, int y, int w, int h);
	void toggleLayers(int flags);
	void scrollLayer(int layer, int offset, int top, int bottom, bool fast);
	void update();
	bool isScrolling(int layer, int direction, int threshold = 0) const {
		return (layer & ~1) ? false :
			(direction == 0 ? (_layers[layer].scrollRemainder != threshold) :
				(direction == 1 ? (_layers[layer].scrollRemainder > threshold) : (_layers[layer].scrollRemainder < threshold)));
	}

	uint8 *getLayerPixels(int layer, int x, int y) const;
	int getLayerPitch(int layer) const { assert (layer >= 0 && layer < 2); return _layers[layer].pitch; }
	int getLayerWidth(int layer) const { assert (layer >= 0 && layer < 2); return _layers[layer].width; }
	int getLayerHeight(int layer) const { assert (layer >= 0 && layer < 2); return _layers[layer].height; }
	int getLayerBpp(int layer) const { assert (layer >= 0 && layer < 2); return _layers[layer].bpp; }
	int getLayerScaleW(int layer) const { assert (layer >= 0 && layer < 2); return _layers[layer].scaleW; }
	int getLayerScaleH(int layer) const { assert (layer >= 0 && layer < 2); return _layers[layer].scaleH; }

private:
	struct TownsScreenLayer {
		uint8 *pixels = nullptr;
		uint8 *palette = nullptr;
		int pitch = 0;
		int width = 0;
		int height = 0;
		int bpp = 0;
		int numCol = 0;
		uint16 hScroll = 0;
		uint8 scaleW = 0;
		uint8 scaleH = 0;
		int scrollRemainder = 0;
		bool onBottom = false;
		bool enabled = false;
		bool ready = false;
		uint16 *bltTmpPal= nullptr;
	} _layers[2];

	template<typename dstPixelType, typename srcPixelType, int scaleW, int scaleH, bool col4bit> void transferRect(uint8 *dst, TownsScreenLayer *l, int x, int y, int w, int h);
	template<typename dstPixelType> void updateScreenBuffer();

#ifdef USE_RGB_COLOR
	void update16BitPalette();
	uint16 calc16BitColor(const uint8 *palEntry);
#endif

	int _height;
	int _width;
	int _pitch;
	bool _semiSmoothScroll;
	Graphics::PixelFormat _pixelFormat;

	int _numDirtyRects;
	Common::List<Common::Rect> _dirtyRects;
	OSystem *_system;
};
#endif // DISABLE_TOWNS_DUAL_LAYER_MODE

class MajMinCodec {
public:

	struct {
		bool repeatMode;
		int repeatCount;
		byte color;
		byte shift;
		uint16 bits;
		byte numBits;
		const byte *dataPtr;
		byte buffer[336];
	} _majMinData;

	void setupBitReader(byte shift, const byte *src);
	void skipData(int32 numSkip);
	void decodeLine(byte *buf, int32 numBytes, int32 dir);
	inline byte readBits(byte n);
};

} // End of namespace Scumm

#endif
