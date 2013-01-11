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

 #define RGB_TO_PIXEL(r,g,b) zb->cmode.RGBToColor(r, g, b)
typedef byte PIXEL;

#define PSZSH 4

extern uint8 PSZB;

struct Buffer {
	byte *pbuf;
	unsigned int *zbuf;
	bool used;
};

typedef struct {
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
} ZBuffer;

typedef struct {
	int x,y,z;     // integer coordinates in the zbuffer
	int s,t;       // coordinates for the mapping
	int r,g,b;     // color indexes

	float sz,tz;   // temporary coordinates for mapping
} ZBufferPoint;

// zbuffer.c

Buffer *ZB_genOffscreenBuffer(ZBuffer *zb);
void ZB_delOffscreenBuffer(ZBuffer *zb, Buffer *buffer);
/**
 * Blit the buffer to the screen buffer, checking the depth of the pixels.
 * Eack pixel is copied if and only if its depth value is bigger than the
 * depth value of the screen pixel, so if it is 'above'.
 */
void ZB_blitOffscreenBuffer(ZBuffer *zb, Buffer *buffer);
void ZB_selectOffscreenBuffer(ZBuffer *zb, Buffer *buffer);
void ZB_clearOffscreenBuffer(ZBuffer *zb, Buffer *buffer);

ZBuffer *ZB_open(int xsize, int ysize, const Graphics::PixelBuffer &buffer);
void ZB_close(ZBuffer *zb);
void ZB_resize(ZBuffer *zb, void *frame_buffer, int xsize, int ysize);
void ZB_clear(ZBuffer *zb, int clear_z, int z, int clear_color, int r, int g, int b);
// linesize is in BYTES
void ZB_copyFrameBuffer(ZBuffer *zb, void *buf, int linesize);

// zline.c

void ZB_plot(ZBuffer *zb,ZBufferPoint *p);
void ZB_line(ZBuffer *zb,ZBufferPoint *p1,ZBufferPoint *p2);
void ZB_line_z(ZBuffer * zb, ZBufferPoint * p1, ZBufferPoint * p2);

// ztriangle.c */

void ZB_setTexture(ZBuffer *zb, const Graphics::PixelBuffer &texture);
void ZB_fillTriangleFlat(ZBuffer *zb, ZBufferPoint *p1,
						 ZBufferPoint *p2, ZBufferPoint *p3);
void ZB_fillTriangleFlatShadowMask(ZBuffer *zb, ZBufferPoint *p1,
						 ZBufferPoint *p2, ZBufferPoint *p3);
void ZB_fillTriangleFlatShadow(ZBuffer *zb, ZBufferPoint *p1,
						 ZBufferPoint *p2, ZBufferPoint *p3);
void ZB_fillTriangleSmooth(ZBuffer *zb, ZBufferPoint *p1,
						   ZBufferPoint *p2, ZBufferPoint *p3);
void ZB_fillTriangleMapping(ZBuffer *zb, ZBufferPoint *p1,
							ZBufferPoint *p2, ZBufferPoint *p3);
void ZB_fillTriangleMappingPerspective(ZBuffer *zb, ZBufferPoint *p0,
									   ZBufferPoint *p1, ZBufferPoint *p2);
typedef void (*ZB_fillTriangleFunc)(ZBuffer *, ZBufferPoint *,
									ZBufferPoint *, ZBufferPoint *);

// memory.c
void gl_free(void *p);
void *gl_malloc(int size);
void *gl_zalloc(int size);

} // end of namespace TinyGL

#endif
