/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#ifndef GRAPHICS_TINYGL_ZBUFFER_H_
#define GRAPHICS_TINYGL_ZBUFFER_H_

#include "graphics/pixelbuffer.h"
#include "graphics/tinygl/texelbuffer.h"
#include "graphics/tinygl/gl.h"
#include "common/rect.h"

namespace TinyGL {

// Z buffer

#define ZB_Z_BITS 16

#define ZB_POINT_Z_FRAC_BITS 14

#define ZB_POINT_ST_FRAC_BITS 14
#define ZB_POINT_ST_FRAC_SHIFT     (ZB_POINT_ST_FRAC_BITS - 1)
#define ZB_POINT_ST_MAX            ( (c->_textureSize << ZB_POINT_ST_FRAC_BITS) - 1 )

#define ZB_POINT_RED_BITS         16
#define ZB_POINT_RED_FRAC_BITS    8
#define ZB_POINT_RED_FRAC_SHIFT   (ZB_POINT_RED_FRAC_BITS - 1)
#define ZB_POINT_RED_MAX          ( (1 << ZB_POINT_RED_BITS) - 1 )

#define ZB_POINT_GREEN_BITS       16
#define ZB_POINT_GREEN_FRAC_BITS  8
#define ZB_POINT_GREEN_FRAC_SHIFT (ZB_POINT_GREEN_FRAC_BITS - 1)
#define ZB_POINT_GREEN_MAX        ( (1 << ZB_POINT_GREEN_BITS) - 1 )

#define ZB_POINT_BLUE_BITS        16
#define ZB_POINT_BLUE_FRAC_BITS   8
#define ZB_POINT_BLUE_FRAC_SHIFT  (ZB_POINT_BLUE_FRAC_BITS - 1)
#define ZB_POINT_BLUE_MAX         ( (1 << ZB_POINT_BLUE_BITS) - 1 )

#define ZB_POINT_ALPHA_BITS       16
#define ZB_POINT_ALPHA_FRAC_BITS  8
#define ZB_POINT_ALPHA_FRAC_SHIFT (ZB_POINT_ALPHA_FRAC_BITS - 1)
#define ZB_POINT_ALPHA_MAX        ( (1 << ZB_POINT_ALPHA_BITS) - 1 )

#define RGB_TO_PIXEL(r, g, b) cmode.ARGBToColor(255, r, g, b) // Default to 255 alpha aka solid colour.

static const int DRAW_DEPTH_ONLY = 0;
static const int DRAW_FLAT = 1;
static const int DRAW_SMOOTH = 2;
static const int DRAW_SHADOW_MASK = 3;
static const int DRAW_SHADOW = 4;

struct Buffer {
	byte *pbuf;
	unsigned int *zbuf;
	bool used;
};

struct ZBufferPoint {
	int x, y, z;   // integer coordinates in the zbuffer
	int s, t;      // coordinates for the mapping
	int r, g, b, a;   // color indexes

	float sz, tz;  // temporary coordinates for mapping

	bool operator==(const ZBufferPoint &other) const {
		return	x == other.x &&
				y == other.y && 
				z == other.z &&
				s == other.s &&
				t == other.t && 
				r == other.r && 
				g == other.g && 
				b == other.b &&
				a == other.a;
	}
};

struct FrameBuffer {
	FrameBuffer(int xsize, int ysize, const Graphics::PixelBuffer &frame_buffer);
	FrameBuffer(int xsize, int ysize, const Graphics::PixelFormat &format);
	~FrameBuffer();

	Buffer *genOffscreenBuffer();
	void delOffscreenBuffer(Buffer *buffer);
	void clear(int clear_z, int z, int clear_color, int r, int g, int b);
	void clearRegion(int x, int y, int w, int h,int clear_z, int z, int clear_color, int r, int g, int b);

	byte *getPixelBuffer() {
		return pbuf.getRawBuffer(0);
	}

	unsigned int *getZBuffer() {
		return _zbuf;
	}

	FORCEINLINE void readPixelRGB(int pixel, byte &r, byte &g, byte &b) {
		pbuf.getRGBAt(pixel, r, g, b);
	}

	FORCEINLINE bool compareDepth(unsigned int &zSrc, unsigned int &zDst) {
		if (!_depthTestEnabled)
			return true;

		switch (_depthFunc) {
		case TGL_NEVER:
			break;
		case TGL_LESS:
			if (zDst < zSrc)
				return true;
			break;
		case TGL_EQUAL:
			if (zDst == zSrc)
				return true;
			break;
		case TGL_LEQUAL:
			if (zDst <= zSrc)
				return true;
			break;
		case TGL_GREATER:
			if (zDst > zSrc)
				return true;
			break;
		case TGL_NOTEQUAL:
			if (zDst != zSrc)
				return true;
			break;
		case TGL_GEQUAL:
			if (zDst >= zSrc)
				return true;
			break;
		case TGL_ALWAYS:
			return true;
		}
		return false;
	}

	FORCEINLINE bool checkAlphaTest(byte aSrc) {
		if (!_alphaTestEnabled)
			return true;

		switch (_alphaTestFunc) {
		case TGL_NEVER:
			break;
		case TGL_LESS:
			if (aSrc < _alphaTestRefVal)
				return true;
			break;
		case TGL_EQUAL:
			if (aSrc == _alphaTestRefVal)
				return true;
			break;
		case TGL_LEQUAL:
			if (aSrc <= _alphaTestRefVal)
				return true;
			break;
		case TGL_GREATER:
			if (aSrc > _alphaTestRefVal)
				return true;
			break;
		case TGL_NOTEQUAL:
			if (aSrc != _alphaTestRefVal)
				return true;
			break;
		case TGL_GEQUAL:
			if (aSrc >= _alphaTestRefVal)
				return true;
			break;
		case TGL_ALWAYS:
			return true;
		}
		return false;
	}

	template <bool kEnableAlphaTest, bool kBlendingEnabled>
	FORCEINLINE void writePixel(int pixel, int value) {
		writePixel<kEnableAlphaTest, kBlendingEnabled, false>(pixel, value, 0);
	}

	template <bool kEnableAlphaTest, bool kBlendingEnabled, bool kDepthWrite>
	FORCEINLINE void writePixel(int pixel, int value, unsigned int z) {
		if (kBlendingEnabled == false) {
			this->pbuf.setPixelAt(pixel, value);
			if (kDepthWrite) {
				_zbuf[pixel] = z;
			}
		} else {
			byte rSrc, gSrc, bSrc, aSrc;
			this->pbuf.getFormat().colorToARGB(value, aSrc, rSrc, gSrc, bSrc);

			writePixel<kEnableAlphaTest, kBlendingEnabled, kDepthWrite>(pixel, aSrc, rSrc, gSrc, bSrc, z);
		}
	}

	FORCEINLINE void writePixel(int pixel, int value) {
		if (_alphaTestEnabled) {
			writePixel<true>(pixel, value);
		} else {
			writePixel<false>(pixel, value);
		}
	}

	template <bool kEnableAlphaTest>
	FORCEINLINE void writePixel(int pixel, int value) {
		if (_blendingEnabled) {
			writePixel<kEnableAlphaTest, true>(pixel, value);
		} else {
			writePixel<kEnableAlphaTest, false>(pixel, value);
		}
	}

	FORCEINLINE void writePixel(int pixel, byte rSrc, byte gSrc, byte bSrc) {
		writePixel(pixel, 255, rSrc, gSrc, bSrc);
	}

	FORCEINLINE bool scissorPixel(int x, int y) {
		return !_clipRectangle.contains(x, y);
	}

	FORCEINLINE void writePixel(int pixel, byte aSrc, byte rSrc, byte gSrc, byte bSrc) {
		if (_alphaTestEnabled) {
			writePixel<true>(pixel, aSrc, rSrc, gSrc, bSrc);
		} else {
			writePixel<false>(pixel, aSrc, rSrc, gSrc, bSrc);
		}
	}

	template <bool kEnableAlphaTest>
	FORCEINLINE void writePixel(int pixel, byte aSrc, byte rSrc, byte gSrc, byte bSrc) {
		if (_blendingEnabled) {
			writePixel<kEnableAlphaTest, true>(pixel, aSrc, rSrc, gSrc, bSrc);
		} else {
			writePixel<kEnableAlphaTest, false>(pixel, aSrc, rSrc, gSrc, bSrc);
		}
	}

	template <bool kEnableAlphaTest, bool kBlendingEnabled>
	FORCEINLINE void writePixel(int pixel, byte aSrc, byte rSrc, byte gSrc, byte bSrc) {
		writePixel<kEnableAlphaTest, kBlendingEnabled, false>(pixel, aSrc, rSrc, gSrc, bSrc, 0);
	}

	template <bool kEnableAlphaTest, bool kBlendingEnabled, bool kDepthWrite>
	FORCEINLINE void writePixel(int pixel, byte aSrc, byte rSrc, byte gSrc, byte bSrc, unsigned int z) {
		if (kEnableAlphaTest) {
			if (!checkAlphaTest(aSrc))
				return;
		}
		if (kDepthWrite) {
			_zbuf[pixel] = z;
		}
		
		if (kBlendingEnabled == false) {
			this->pbuf.setPixelAt(pixel, aSrc, rSrc, gSrc, bSrc);
		} else {
			byte rDst, gDst, bDst, aDst;
			this->pbuf.getARGBAt(pixel, aDst, rDst, gDst, bDst);
			switch (_sourceBlendingFactor) {
			case TGL_ZERO:
				rSrc = gSrc = bSrc = 0;
				break;
			case TGL_ONE:
				break;
			case TGL_DST_COLOR:
				rSrc = (rDst * rSrc) >> 8;
				gSrc = (gDst * gSrc) >> 8;
				bSrc = (bDst * bSrc) >> 8;
				break;
			case TGL_ONE_MINUS_DST_COLOR:
				rSrc = (rSrc * (255 - rDst)) >> 8;
				gSrc = (gSrc * (255 - gDst)) >> 8;
				bSrc = (bSrc * (255 - bDst)) >> 8;
				break;
			case TGL_SRC_ALPHA:
				rSrc = (rSrc * aSrc) >> 8;
				gSrc = (gSrc * aSrc) >> 8;
				bSrc = (bSrc * aSrc) >> 8;
				break;
			case TGL_ONE_MINUS_SRC_ALPHA:
				rSrc = (rSrc * (255 - aSrc)) >> 8;
				gSrc = (gSrc * (255 - aSrc)) >> 8;
				bSrc = (bSrc * (255 - aSrc)) >> 8;
				break;
			case TGL_DST_ALPHA:
				rSrc = (rSrc * aDst) >> 8;
				gSrc = (gSrc * aDst) >> 8;
				bSrc = (bSrc * aDst) >> 8;
				break;
			case TGL_ONE_MINUS_DST_ALPHA:
				rSrc = (rSrc * (255 - aDst)) >> 8;
				gSrc = (gSrc * (255 - aDst)) >> 8;
				bSrc = (bSrc * (255 - aDst)) >> 8;
				break;
			default:
				break;
			}

			switch (_destinationBlendingFactor) {
			case TGL_ZERO:
				rDst = gDst = bDst = 0;
				break;
			case TGL_ONE:
				break;
			case TGL_DST_COLOR:
				rDst = (rDst * rSrc) >> 8;
				gDst = (gDst * gSrc) >> 8;
				bDst = (bDst * bSrc) >> 8;
				break;
			case TGL_ONE_MINUS_DST_COLOR:
				rDst = (rDst * (255 - rSrc)) >> 8;
				gDst = (gDst * (255 - gSrc)) >> 8;
				bDst = (bDst * (255 - bSrc)) >> 8;
				break;
			case TGL_SRC_ALPHA:
				rDst = (rDst * aSrc) >> 8;
				gDst = (gDst * aSrc) >> 8;
				bDst = (bDst * aSrc) >> 8;
				break;
			case TGL_ONE_MINUS_SRC_ALPHA:
				rDst = (rDst * (255 - aSrc)) >> 8;
				gDst = (gDst * (255 - aSrc)) >> 8;
				bDst = (bDst * (255 - aSrc)) >> 8;
				break;
			case TGL_DST_ALPHA:
				rDst = (rDst * aDst) >> 8;
				gDst = (gDst * aDst) >> 8;
				bDst = (bDst * aDst) >> 8;
				break;
			case TGL_ONE_MINUS_DST_ALPHA:
				rDst = (rDst * (255 - aDst)) >> 8;
				gDst = (gDst * (255 - aDst)) >> 8;
				bDst = (bDst * (255 - aDst)) >> 8;
				break;
			case TGL_SRC_ALPHA_SATURATE: {
				int factor = aSrc < 1 - aDst ? aSrc : 1 - aDst;
				rDst = (rDst * factor) >> 8;
				gDst = (gDst * factor) >> 8;
				bDst = (bDst * factor) >> 8;
				}
				break;
			default:
				break;
			}
			int finalR, finalG, finalB;
			finalR = rDst + rSrc;
			finalG = gDst + gSrc;
			finalB = bDst + bSrc;
			if (finalR > 255) { finalR = 255; }
			if (finalG > 255) { finalG = 255; }
			if (finalB > 255) { finalB = 255; }
			this->pbuf.setPixelAt(pixel, 255, finalR, finalG, finalB);
		}
	}

	void copyToBuffer(Graphics::PixelBuffer &buf) {
		buf.copyBuffer(0, xsize * ysize, pbuf);
	}

	void copyFromBuffer(Graphics::PixelBuffer buf) {
		pbuf.copyBuffer(0, xsize * ysize, buf);
	}
	
	void enableBlending(bool enable) {
		_blendingEnabled = enable;
	}

	void enableDepthTest(bool enable) {
		_depthTestEnabled = enable;
	}

	void setBlendingFactors(int sFactor, int dFactor) {
		_sourceBlendingFactor = sFactor;
		_destinationBlendingFactor = dFactor;
	}

	void enableAlphaTest(bool enable) {
		_alphaTestEnabled = enable;
	}

	void setAlphaTestFunc(int func, int ref) {
		_alphaTestFunc = func;
		_alphaTestRefVal = ref;
	}

	void setDepthFunc(int func) {
		_depthFunc = func;
	}

	void enableDepthWrite(bool enable) {
		this->_depthWrite = enable;
	}

	bool isAlphaBlendingEnabled() const {
		return _sourceBlendingFactor == TGL_SRC_ALPHA && _destinationBlendingFactor == TGL_ONE_MINUS_SRC_ALPHA;
	}

	/**
	* Blit the buffer to the screen buffer, checking the depth of the pixels.
	* Eack pixel is copied if and only if its depth value is bigger than the
	* depth value of the screen pixel, so if it is 'above'.
	*/
	void blitOffscreenBuffer(Buffer *buffer);
	void selectOffscreenBuffer(Buffer *buffer);
	void clearOffscreenBuffer(Buffer *buffer);
	void setTexture(const Graphics::TexelBuffer *texture, unsigned int wraps, unsigned int wrapt);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, int kDrawLogic, bool kDepthWrite, bool enableAlphaTest, bool kEnableScissor, bool enableBlending>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, int kDrawMode, bool kDepthWrite, bool enableAlphaTest, bool kEnableScissor>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, int kDrawMode, bool kDepthWrite, bool enableAlphaTest>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, int kDrawMode, bool kDepthWrite>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, int kDrawMode>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	void fillTriangleTextureMappingPerspectiveSmooth(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleTextureMappingPerspectiveFlat(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleDepthOnly(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleFlat(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleSmooth(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleFlatShadowMask(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleFlatShadow(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	void plot(ZBufferPoint *p);
	void fillLine(ZBufferPoint *p1, ZBufferPoint *p2);
	void fillLineZ(ZBufferPoint *p1, ZBufferPoint *p2);
	void fillLineFlatZ(ZBufferPoint *p1, ZBufferPoint *p2);
	void fillLineInterpZ(ZBufferPoint *p1, ZBufferPoint *p2);
	void fillLineFlat(ZBufferPoint *p1, ZBufferPoint *p2);
	void fillLineInterp(ZBufferPoint *p1, ZBufferPoint *p2);

	void setScissorRectangle(const Common::Rect &rect) {
		_clipRectangle = rect;
		_enableScissor = true;
	}
	void resetScissorRectangle() {
		_enableScissor = false;
	}

	Common::Rect _clipRectangle;
	bool _enableScissor;
	int xsize, ysize;
	int linesize; // line size, in bytes
	Graphics::PixelFormat cmode;
	int pixelbytes;

	Buffer buffer;

	unsigned char *shadow_mask_buf;
	int shadow_color_r;
	int shadow_color_g;
	int shadow_color_b;
	int frame_buffer_allocated;

	unsigned char *dctable;
	int *ctable;
	const Graphics::TexelBuffer *current_texture;
	int _textureSize;
	int _textureSizeMask;
	unsigned int wrapS, wrapT;

	FORCEINLINE bool isBlendingEnabled() const { return _blendingEnabled; }
	FORCEINLINE void getBlendingFactors(int &sourceFactor, int &destinationFactor) const { sourceFactor = _sourceBlendingFactor; destinationFactor = _destinationBlendingFactor; }
	FORCEINLINE bool isAlphaTestEnabled() const { return _alphaTestEnabled; }
	FORCEINLINE bool isDepthWriteEnabled() const { return _depthWrite; }
	FORCEINLINE int getDepthFunc() const { return _depthFunc; }
	FORCEINLINE int getDepthWrite() const { return _depthWrite; }
	FORCEINLINE int getAlphaTestFunc() const { return _alphaTestFunc; }
	FORCEINLINE int getAlphaTestRefVal() const { return _alphaTestRefVal; }
	FORCEINLINE int getDepthTestEnabled() const { return _depthTestEnabled; }

private:

	template <bool kDepthWrite>
	FORCEINLINE void putPixel(unsigned int pixelOffset, int color, int x, int y, unsigned int z);

	template <bool kDepthWrite, bool kEnableScissor>
	FORCEINLINE void putPixel(unsigned int pixelOffset, int color, int x, int y, unsigned int z);

	template <bool kEnableScissor>
	FORCEINLINE void putPixel(unsigned int pixelOffset, int color, int x, int y);

	template <bool kInterpRGB, bool kInterpZ, bool kDepthWrite>
	void drawLine(const ZBufferPoint *p1, const ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kDepthWrite, bool kEnableScissor>
	void drawLine(const ZBufferPoint *p1, const ZBufferPoint *p2);

	unsigned int *_zbuf;
	bool _depthWrite;
	Graphics::PixelBuffer pbuf;
	bool _blendingEnabled;
	int _sourceBlendingFactor;
	int _destinationBlendingFactor;
	bool _alphaTestEnabled;
	bool _depthTestEnabled;
	int _alphaTestFunc;
	int _alphaTestRefVal;
	int _depthFunc;
};

// memory.c
void gl_free(void *p);
void *gl_malloc(int size);
void *gl_zalloc(int size);

} // end of namespace TinyGL

#endif
