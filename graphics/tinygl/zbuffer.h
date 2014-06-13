#ifndef GRAPHICS_TINYGL_ZBUFFER_H_
#define GRAPHICS_TINYGL_ZBUFFER_H_

#include "graphics/pixelbuffer.h"

namespace TinyGL {

// Z buffer

#define ZB_Z_BITS 16

#define ZB_POINT_Z_FRAC_BITS 14

#define ZB_POINT_S_MIN ( (1 << 13) )
#define ZB_POINT_S_MAX ( (1 << 22) - (1 << 13) )
#define ZB_POINT_T_MIN ( (1 << 21) )
#define ZB_POINT_T_MAX ( (1 << 30) - (1 << 21) )

#define ZB_POINT_RED_MIN ( (1 << 10) )
#define ZB_POINT_RED_MAX ( (1 << 16) - (1 << 10) )
#define ZB_POINT_GREEN_MIN ( (1 << 9) )
#define ZB_POINT_GREEN_MAX ( (1 << 16) - (1 << 9) )
#define ZB_POINT_BLUE_MIN ( (1 << 10) )
#define ZB_POINT_BLUE_MAX ( (1 << 16) - (1 << 10) )

// display modes
#define ZB_MODE_5R6G5B  1  // true color 16 bits

#define RGB_TO_PIXEL(r,g,b) cmode.RGBToColor(r, g, b)
typedef byte PIXEL;

#define PSZSH 4

extern uint8 PSZB;

struct Buffer {
	byte *pbuf;
	unsigned int *zbuf;
	bool used;
};

struct ZBufferPoint {
	int x, y, z;   // integer coordinates in the zbuffer
	int s, t;      // coordinates for the mapping
	int r, g, b;   // color indexes

	float sz, tz;  // temporary coordinates for mapping
};

struct FrameBuffer {
	FrameBuffer(int xsize, int ysize, const Graphics::PixelBuffer &frame_buffer);
	~FrameBuffer();

	Buffer *genOffscreenBuffer();
	void delOffscreenBuffer(Buffer *buffer);
	void clear(int clear_z, int z, int clear_color, int r, int g, int b);

	/**
	* Blit the buffer to the screen buffer, checking the depth of the pixels.
	* Eack pixel is copied if and only if its depth value is bigger than the
	* depth value of the screen pixel, so if it is 'above'.
	*/
	void blitOffscreenBuffer(Buffer *buffer);
	void selectOffscreenBuffer(Buffer *buffer);
	void clearOffscreenBuffer(Buffer *buffer);
	void setTexture(const Graphics::PixelBuffer &texture);

	void fillTriangleMappingPerspective(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleDepthOnly(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleFlat(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleFlatShadowMask(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleFlatShadow(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleSmooth(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
	void fillTriangleMapping(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);

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
	Graphics::PixelBuffer pbuf;
	int frame_buffer_allocated;

	unsigned char *dctable;
	int *ctable;
	Graphics::PixelBuffer current_texture;
};

// memory.c
void gl_free(void *p);
void *gl_malloc(int size);
void *gl_zalloc(int size);

} // end of namespace TinyGL

#endif
