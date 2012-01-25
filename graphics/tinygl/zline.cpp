
#include "graphics/tinygl/zbuffer.h"

namespace TinyGL {

#define ZCMP(z,zpix) ((z) >= (zpix))

void ZB_plot(ZBuffer * zb, ZBufferPoint * p) {
	unsigned short *pz;
	unsigned int *pz_2;
	PIXEL *pp;
	unsigned int zz;

	pz = zb->zbuf + (p->y * zb->xsize + p->x);
	pz_2 = zb->zbuf2 + (p->y * zb->xsize + p->x);
	pp = (PIXEL *)((char *) zb->pbuf.getRawBuffer() + zb->linesize * p->y + p->x * PSZB);
	zz = p->z >> ZB_POINT_Z_FRAC_BITS;
	if ((ZCMP(zz, *pz)) && (ZCMP((unsigned int)p->z, *pz_2))) {
		*pp = RGB_TO_PIXEL(p->r, p->g, p->b);
		*pz_2 = p->z;
    }
}

#define INTERP_Z
static void ZB_line_flat_z(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2, int color) {
#include "graphics/tinygl/zline.h"
}

// line with color interpolation
#define INTERP_Z
#define INTERP_RGB
static void ZB_line_interp_z(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2) {
#include "graphics/tinygl/zline.h"
}

// no Z interpolation
static void ZB_line_flat(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2, int color) {
#include "graphics/tinygl/zline.h"
}

#define INTERP_RGB
static void ZB_line_interp(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2) {
#include "graphics/tinygl/zline.h"
}

void ZB_line_z(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2) {
	int color1, color2;

	color1 = RGB_TO_PIXEL(p1->r, p1->g, p1->b);
	color2 = RGB_TO_PIXEL(p2->r, p2->g, p2->b);

    // choose if the line should have its color interpolated or not
	if (color1 == color2) {
		ZB_line_flat_z(zb, p1, p2, color1);
	} else {
		ZB_line_interp_z(zb, p1, p2);
	}
}

void ZB_line(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2) {
	int color1, color2;

	color1 = RGB_TO_PIXEL(p1->r, p1->g, p1->b);
	color2 = RGB_TO_PIXEL(p2->r, p2->g, p2->b);

	// choose if the line should have its color interpolated or not
	if (color1 == color2) {
		ZB_line_flat(zb, p1, p2, color1);
	} else {
		ZB_line_interp(zb, p1, p2);
	}
}

} // end of namespace TinyGL
