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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-2022 Fabrice Bellard,
 * which is licensed under the MIT license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#ifndef GRAPHICS_TINYGL_ZBUFFER_H_
#define GRAPHICS_TINYGL_ZBUFFER_H_

#include "graphics/surface.h"
#include "graphics/tinygl/pixelbuffer.h"
#include "graphics/tinygl/texelbuffer.h"
#include "graphics/tinygl/gl.h"

#include "common/rect.h"

namespace TinyGL {

// Z buffer

#define ZB_Z_BITS 16

#define ZB_FOG_BITS               16
#define ZB_FOG_MAX                ( (1 << ZB_FOG_BITS) - 1 )

#define ZB_POINT_Z_FRAC_BITS 14

#define ZB_POINT_ST_FRAC_BITS 14
#define ZB_POINT_ST_FRAC_SHIFT     (ZB_POINT_ST_FRAC_BITS - 1)
#define ZB_POINT_ST_MAX            ( (_textureSize << ZB_POINT_ST_FRAC_BITS) - 1 )

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

#define RGB_TO_PIXEL(r, g, b)     _pbufFormat.ARGBToColor(255, r, g, b) // Default to 255 alpha aka solid colour.

static const int DRAW_DEPTH_ONLY = 0;
static const int DRAW_FLAT = 1;
static const int DRAW_SMOOTH = 2;

struct Buffer {
	byte *pbuf;
	uint *zbuf;
	bool used;
};

struct ZBufferPoint {
	int x, y, z;      // integer coordinates in the zbuffer
	int s, t;         // coordinates for the mapping
	int r, g, b, a;   // color indexes
	float sz, tz;     // temporary coordinates for mapping
	int f;            // fog factor

	bool operator==(const ZBufferPoint &other) const {
		return
			x == other.x &&
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
	FrameBuffer(int width, int height, const Graphics::PixelFormat &format, bool enableStencilBuffer);
	~FrameBuffer();

	Graphics::PixelFormat getPixelFormat() {
		return _pbufFormat;
	}

	byte *getPixelBuffer() {
		return _pbuf.getRawBuffer();
	}

	int getPixelBufferWidth() {
		return _pbufWidth;
	}

	int getPixelBufferHeight() {
		return _pbufHeight;
	}

	const uint *getZBuffer() {
		return _zbuf;
	}

	Graphics::Surface *copyToBuffer(const Graphics::PixelFormat &dstFormat) {
		Graphics::Surface tmp;
		tmp.init(_pbufWidth, _pbufHeight, _pbufPitch, _pbuf.getRawBuffer(), _pbufFormat);
		return tmp.convertTo(dstFormat);
	}

	void getSurfaceRef(Graphics::Surface &surface) {
		surface.init(_pbufWidth, _pbufHeight, _pbufPitch, _pbuf.getRawBuffer(), _pbufFormat);
	}

private:

	FORCEINLINE bool compareDepth(uint &zSrc, uint &zDst) {
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

	FORCEINLINE bool stencilTest(byte sSrc) {
		switch (_stencilTestFunc) {
		case TGL_NEVER:
			break;
		case TGL_LESS:
			if ((_stencilRefVal & _stencilMask) < (sSrc & _stencilMask))
				return true;
			break;
		case TGL_LEQUAL:
			if ((_stencilRefVal & _stencilMask) <= (sSrc & _stencilMask))
				return true;
			break;
		case TGL_GREATER:
			if ((_stencilRefVal & _stencilMask) > (sSrc & _stencilMask))
				return true;
			break;
		case TGL_GEQUAL:
			if ((_stencilRefVal & _stencilMask) >= (sSrc & _stencilMask))
				return true;
			break;
		case TGL_EQUAL:
			if ((_stencilRefVal & _stencilMask) == (sSrc & _stencilMask))
				return true;
			break;
		case TGL_NOTEQUAL:
			if ((_stencilRefVal & _stencilMask) != (sSrc & _stencilMask))
				return true;
			break;
		case TGL_ALWAYS:
			return true;
		}
		return false;
	}

	FORCEINLINE void stencilOp(bool stencilTestResult, bool depthTestResult, byte *sDst) {
		int op = !stencilTestResult ? _stencilSfail : !depthTestResult ? _stencilDpfail : _stencilDppass;
		byte value = *sDst;
		switch (op) {
		case TGL_KEEP:
			return;
		case TGL_ZERO:
			value = 0;
			break;
		case TGL_REPLACE:
			value = _stencilRefVal;
			break;
		case TGL_INCR:
			if (value < 255)
				value++;
			break;
		case TGL_INCR_WRAP:
			value++;
			break;
		case TGL_DECR:
			if (value > 0)
				value--;
			break;
		case TGL_DECR_WRAP:
			value--;
			break;
		case TGL_INVERT:
			value = ~value;
		}
		*sDst = value & _stencilWriteMask;
	}

	template <bool kEnableAlphaTest, bool kBlendingEnabled>
	FORCEINLINE void writePixel(int pixel, int value) {
		writePixel<kEnableAlphaTest, kBlendingEnabled, false>(pixel, value, 0);
	}

	template <bool kEnableAlphaTest, bool kBlendingEnabled, bool kDepthWrite>
	FORCEINLINE void writePixel(int pixel, int value, uint z) {
		if (kBlendingEnabled == false) {
			_pbuf.setPixelAt(pixel, value);
			if (kDepthWrite) {
				_zbuf[pixel] = z;
			}
		} else {
			byte rSrc, gSrc, bSrc, aSrc;
			_pbuf.getFormat().colorToARGB(value, aSrc, rSrc, gSrc, bSrc);

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

	template <bool kDepthWrite, bool kSmoothMode, bool kFogMode, bool kEnableAlphaTest, bool kEnableScissor, bool kEnableBlending, bool kStencilEnabled, bool kDepthTestEnabled>
	void putPixelNoTexture(int fbOffset, uint *pz, byte *ps, int _a,
	                       int x, int y, uint &z, uint &r, uint &g, uint &b, uint &a,
	                       int &dzdx, int &drdx, int &dgdx, int &dbdx, uint dadx,
	                       uint &fog, int fog_r, int fog_g, int fog_b, int &dfdx);

	template <bool kDepthWrite, bool kLightsMode, bool kSmoothMode, bool kFogMode, bool kEnableAlphaTest, bool kEnableScissor, bool kEnableBlending, bool kStencilEnabled, bool kDepthTestEnabled>
	void putPixelTexture(int fbOffset, const TexelBuffer *texture,
	                     uint wrap_s, uint wrap_t, uint *pz, byte *ps, int _a,
	                     int x, int y, uint &z, int &t, int &s,
	                     uint &r, uint &g, uint &b, uint &a,
	                     int &dzdx, int &dsdx, int &dtdx, int &drdx, int &dgdx, int &dbdx, uint dadx,
	                     uint &fog, int fog_r, int fog_g, int fog_b, int &dfdx);

	template <bool kDepthWrite, bool kEnableScissor, bool kStencilEnabled, bool kDepthTestEnabled>
	void putPixelDepth(uint *pz, byte *ps, int _a, int x, int y, uint &z, int &dzdx);


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

public:

	FORCEINLINE void writePixel(int pixel, byte aSrc, byte rSrc, byte gSrc, byte bSrc) {
		if (_alphaTestEnabled) {
			writePixel<true>(pixel, aSrc, rSrc, gSrc, bSrc);
		} else {
			writePixel<false>(pixel, aSrc, rSrc, gSrc, bSrc);
		}
	}

private:

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
	FORCEINLINE void writePixel(int pixel, byte aSrc, byte rSrc, byte gSrc, byte bSrc, uint z) {
		writePixel<kEnableAlphaTest, kBlendingEnabled, false, false>(pixel, aSrc, rSrc, gSrc, bSrc, z, 0.0f, 0, 0, 0);
	}

	template <bool kEnableAlphaTest, bool kBlendingEnabled, bool kDepthWrite, bool kFogMode>
	FORCEINLINE void writePixel(int pixel, byte aSrc, byte rSrc, byte gSrc, byte bSrc, float z, uint fog, byte fog_r, byte fog_g, byte fog_b) {
		if (kEnableAlphaTest) {
			if (!checkAlphaTest(aSrc))
				return;
		}

		if (kDepthWrite) {
			_zbuf[pixel] = z;
		}

		if (kFogMode) {
			int oneMinusFog = (1 << ZB_FOG_BITS) - fog;
			int finalR = (rSrc * fog + fog_r * oneMinusFog) >> ZB_FOG_BITS;
			int finalG = (gSrc * fog + fog_g * oneMinusFog) >> ZB_FOG_BITS;
			int finalB = (bSrc * fog + fog_b * oneMinusFog) >> ZB_FOG_BITS;
			if (finalR > 255) {
				rSrc = 255;
			} else {
				rSrc = finalR;
			}
			if (finalG > 255) {
				gSrc = 255;
			} else {
				gSrc = finalG;
			}
			if (finalB > 255) {
				bSrc = 255;
			} else {
				bSrc = finalB;
			}
		}

		if (!kBlendingEnabled) {
			_pbuf.setPixelAt(pixel, aSrc, rSrc, gSrc, bSrc);
		} else {
			byte rDst, gDst, bDst, aDst;
			_pbuf.getARGBAt(pixel, aDst, rDst, gDst, bDst);
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
			int finalR = rDst + rSrc;
			int finalG = gDst + gSrc;
			int finalB = bDst + bSrc;
			if (finalR > 255) {
				finalR = 255;
			}
			if (finalG > 255) {
				finalG = 255;
			}
			if (finalB > 255) {
				finalB = 255;
			}
			_pbuf.setPixelAt(pixel, 255, finalR, finalG, finalB);
		}
	}

public:

	void clear(int clear_z, int z, int clear_color, int r, int g, int b,
	           bool clearStencil, int stencilValue);
	void clearRegion(int x, int y, int w, int h, bool clearZ, int z,
	                 bool clearColor, int r, int g, int b, bool clearStencil, int stencilValue);

	void setScissorRectangle(const Common::Rect &rect) {
		_clipRectangle = rect;
		_enableScissor = true;
	}

	void resetScissorRectangle() {
		_enableScissor = false;
	}

	void enableBlending(bool enable) {
		_blendingEnabled = enable;
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

	void enableDepthTest(bool enable) {
		_depthTestEnabled = enable;
	}

	void setDepthFunc(int func) {
		_depthFunc = func;
	}

	void enableDepthWrite(bool enable) {
		_depthWrite = enable;
	}

	void enableStencilTest(bool enable) {
		_stencilTestEnabled = enable;
	}

	void setStencilWriteMask(uint stencilWriteMask) {
		_stencilWriteMask = stencilWriteMask;
	}

	void setStencilTestFunc(int stencilFunc, int stencilValue, uint stencilMask) {
		_stencilTestFunc = stencilFunc;
		_stencilRefVal = stencilValue;
		_stencilMask = stencilMask;
	}

	void setStencilOp(int stencilSfail, int stencilDpfail, int stencilDppass) {
		_stencilSfail = stencilSfail;
		_stencilDpfail = stencilDpfail;
		_stencilDppass = stencilDppass;
	}

	void setOffsetStates(int offsetStates) {
		_offsetStates = offsetStates;
	}

	void setOffsetFactor(float offsetFactor) {
		_offsetFactor = offsetFactor;
	}

	void setOffsetUnits(float offsetUnits) {
		_offsetUnits = offsetUnits;
	}

	void setTexture(const TexelBuffer *texture, uint wraps, uint wrapt) {
		_currentTexture = texture;
		_wrapS = wraps;
		_wrapT = wrapt;
	}

	void setTextureSizeAndMask(int textureSize, int textureSizeMask) {
		_textureSize = textureSize;
		_textureSizeMask = textureSizeMask;
	}

	void setFogEnabled(bool enable) {
		_fogEnabled = enable;
	}

	void setFogColor(float colorR, float colorG, float colorB) {
		_fogColorR = colorR;
		_fogColorG = colorG;
		_fogColorB = colorB;
	}

private:

	/**
	* Blit the buffer to the screen buffer, checking the depth of the pixels.
	* Eack pixel is copied if and only if its depth value is bigger than the
	* depth value of the screen pixel, so if it is 'above'.
	*/
	Buffer *genOffscreenBuffer();
	void delOffscreenBuffer(Buffer *buffer);
	void blitOffscreenBuffer(Buffer *buffer);
	void selectOffscreenBuffer(Buffer *buffer);
	void clearOffscreenBuffer(Buffer *buffer);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode,
	          bool kDepthWrite, bool kFogMode, bool kAlphaTestEnabled, bool kEnableScissor,
	          bool kBlendingEnabled, bool kStencilEnabled, bool kDepthTestEnabled>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode,
	          bool kDepthWrite, bool kFogMode, bool kAlphaTestEnabled, bool kEnableScissor,
	          bool kBlendingEnabled, bool kStencilEnabled>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode,
	          bool kDepthWrite, bool kFogMode, bool enableAlphaTest, bool kEnableScissor, bool kBlendingEnabled>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode,
	          bool kDepthWrite, bool kFogMode, bool enableAlphaTest, bool kEnableScissor>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode,
	          bool kDepthWrite, bool kFogMode, bool enableAlphaTest>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode,
	          bool kDepthWrite, bool kFogMode>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode,
	          bool kDepthWrite>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

public:

	void fillTriangleTextureMappingPerspectiveSmooth(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleTextureMappingPerspectiveFlat(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleDepthOnly(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleFlat(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleSmooth(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	void plot(ZBufferPoint *p);
	void fillLine(ZBufferPoint *p1, ZBufferPoint *p2);
	void fillLineZ(ZBufferPoint *p1, ZBufferPoint *p2);

private:

	void fillLineFlatZ(ZBufferPoint *p1, ZBufferPoint *p2);
	void fillLineInterpZ(ZBufferPoint *p1, ZBufferPoint *p2);
	void fillLineFlat(ZBufferPoint *p1, ZBufferPoint *p2);
	void fillLineInterp(ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool kDepthWrite>
	FORCEINLINE void putPixel(uint pixelOffset, int color, int x, int y, uint z);

	template <bool kDepthWrite, bool kEnableScissor>
	FORCEINLINE void putPixel(uint pixelOffset, int color, int x, int y, uint z);

	template <bool kEnableScissor>
	FORCEINLINE void putPixel(uint pixelOffset, int color, int x, int y);

	template <bool kInterpRGB, bool kInterpZ, bool kDepthWrite>
	void drawLine(const ZBufferPoint *p1, const ZBufferPoint *p2);

	template <bool kInterpRGB, bool kInterpZ, bool kDepthWrite, bool kEnableScissor>
	void drawLine(const ZBufferPoint *p1, const ZBufferPoint *p2);

	Buffer _offscreenBuffer;
	Graphics::PixelBuffer _pbuf;
	int _pbufWidth;
	int _pbufHeight;
	int _pbufPitch;
	Graphics::PixelFormat _pbufFormat;
	int _pbufBpp;

	uint *_zbuf;
	byte *_sbuf;

	bool _enableStencil;
	int _textureSize;
	int _textureSizeMask;

	Common::Rect _clipRectangle;
	bool _enableScissor;

	const TexelBuffer *_currentTexture;
	uint _wrapS, _wrapT;
	bool _blendingEnabled;
	int _sourceBlendingFactor;
	int _destinationBlendingFactor;
	bool _alphaTestEnabled;
	int _alphaTestFunc;
	int _alphaTestRefVal;
	bool _depthTestEnabled;
	bool _depthWrite;
	bool _stencilTestEnabled;
	int _stencilTestFunc;
	int _stencilRefVal;
	uint _stencilMask;
	uint _stencilWriteMask;
	int _stencilSfail;
	int _stencilDpfail;
	int _stencilDppass;
	int _depthFunc;
	int _offsetStates;
	float _offsetFactor;
	float _offsetUnits;
	bool _fogEnabled;
	float _fogColorR;
	float _fogColorG;
	float _fogColorB;
};

// memory.c
void gl_free(void *p);
void *gl_malloc(int size);
void *gl_zalloc(int size);

} // end of namespace TinyGL

#endif
