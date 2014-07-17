#ifndef GRAPHICS_TINYGL_ZBUFFER_H_
#define GRAPHICS_TINYGL_ZBUFFER_H_

#include "graphics/pixelbuffer.h"
#include "graphics/tinygl/gl.h"

namespace TinyGL {

// Z buffer

#define ZB_Z_BITS 16

#define ZB_POINT_Z_FRAC_BITS 14

#define ZB_POINT_ST_FRAC_BITS 14
#define ZB_POINT_ST_FRAC_SHIFT     (ZB_POINT_ST_FRAC_BITS - 1)
#define ZB_POINT_ST_MIN            ( (1 << ZB_POINT_ST_FRAC_SHIFT) )
#define ZB_POINT_ST_MAX            ( (c->_textureSize << ZB_POINT_ST_FRAC_BITS) - (1 << ZB_POINT_ST_FRAC_SHIFT) )

#define ZB_POINT_RED_MIN ( (1 << 10) )
#define ZB_POINT_RED_MAX ( (1 << 16) - (1 << 10) )
#define ZB_POINT_GREEN_MIN ( (1 << 9) )
#define ZB_POINT_GREEN_MAX ( (1 << 16) - (1 << 9) )
#define ZB_POINT_BLUE_MIN ( (1 << 10) )
#define ZB_POINT_BLUE_MAX ( (1 << 16) - (1 << 10) )
#define ZB_POINT_ALPHA_MIN ( (1 << 10) )
#define ZB_POINT_ALPHA_MAX ( (1 << 16) - (1 << 10) )

#define RGB_TO_PIXEL(r, g, b) cmode.ARGBToColor(255, r >> 8, g >> 8, b >> 8) // Default to 255 alpha aka solid colour.

static const int DRAW_DEPTH_ONLY = 0;
static const int DRAW_FLAT = 1;
static const int DRAW_SMOOTH = 2;
static const int DRAW_SHADOW_MASK = 3;
static const int DRAW_SHADOW = 4;

extern uint8 PSZB;

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
};

struct FrameBuffer {
	FrameBuffer(int xsize, int ysize, const Graphics::PixelBuffer &frame_buffer);
	~FrameBuffer();

	Buffer *genOffscreenBuffer();
	void delOffscreenBuffer(Buffer *buffer);
	void clear(int clear_z, int z, int clear_color, int r, int g, int b);

	byte *getPixelBuffer() {
		return pbuf.getRawBuffer(0);
	}

	FORCEINLINE void readPixelRGB(int pixel, byte &r, byte &g, byte &b) {
		pbuf.getRGBAt(pixel, r, g, b);
	}

	FORCEINLINE bool compareDepth(unsigned int &zSrc, unsigned int &zDst) {
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

	FORCEINLINE void writePixel(int pixel, int value) {
		byte rSrc, gSrc, bSrc, aSrc;
		this->pbuf.getFormat().colorToARGB(value, aSrc, rSrc, gSrc, bSrc);
		if (!checkAlphaTest(aSrc))
			return;

		if (_blendingEnabled == false) {
			this->pbuf.setPixelAt(pixel, value);
		} else {
			writePixel(pixel, aSrc, rSrc, gSrc, bSrc);
		}
	}

	FORCEINLINE void writePixel(int pixel, byte rSrc, byte gSrc, byte bSrc) {
		writePixel(pixel, 255, rSrc, gSrc, bSrc);
	}

	FORCEINLINE void writePixel(int pixel, byte aSrc, byte rSrc, byte gSrc, byte bSrc) {
		if (!checkAlphaTest(aSrc))
			return;

		if (_blendingEnabled == false) {
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

	void enableBlending(bool enable);
	void setBlendingFactors(int sfactor, int dfactor);
	void enableAlphaTest(bool enable);
	void setAlphaTestFunc(int func, float ref);
	void setDepthFunc(int func);
	void enableDepthWrite(bool enable) {
		this->_depthWrite = enable;
	}

	bool isAlphaBlendingEnabled() {
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
	void setTexture(const Graphics::PixelBuffer &texture);

	template <bool interpRGB, bool interpZ, bool interpST, bool interpSTZ, int drawLogic, bool depthWrite>
	void fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

	template <bool interpRGB, bool interpZ, bool depthWrite>
	void fillLineGeneric(ZBufferPoint *p1, ZBufferPoint *p2, int color);

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
	void fillLineFlatZ(ZBufferPoint *p1, ZBufferPoint *p2, int color);
	void fillLineInterpZ(ZBufferPoint *p1, ZBufferPoint *p2);
	void fillLineFlat(ZBufferPoint *p1, ZBufferPoint *p2, int color);
	void fillLineInterp(ZBufferPoint *p1, ZBufferPoint *p2);

	int xsize, ysize;
	int linesize; // line size, in bytes
	Graphics::PixelFormat cmode;
	int pixelbits;
	int pixelbytes;

	Buffer buffer;

	unsigned int *zbuf;
	unsigned char *shadow_mask_buf;
	int shadow_color_r;
	int shadow_color_g;
	int shadow_color_b;
	int frame_buffer_allocated;

	unsigned char *dctable;
	int *ctable;
	Graphics::PixelBuffer current_texture;
	int _textureSize;
	int _textureSizeMask;

private:

	bool _depthWrite;
	Graphics::PixelBuffer pbuf;
	bool _blendingEnabled;
	int _sourceBlendingFactor;
	int _destinationBlendingFactor;
	bool _alphaTestEnabled;
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
