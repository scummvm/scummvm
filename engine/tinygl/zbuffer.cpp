
// Z buffer: 16,32 bits Z / 16 bits color

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "tinygl/zbuffer.h"

ZBuffer *ZB_open(int xsize, int ysize, int mode, void *frame_buffer) {
	ZBuffer *zb;
	int size;

	zb = (ZBuffer *)gl_malloc(sizeof(ZBuffer));
	if (zb == NULL)
	return NULL;

	zb->xsize = xsize;
	zb->ysize = ysize;
	zb->mode = mode;
	zb->linesize = (xsize * PSZB + 3) & ~3;

	switch (mode) {
	case ZB_MODE_5R6G5B:
		break;
	default:
		goto error;
	}

	size = zb->xsize * zb->ysize * sizeof(unsigned short);

	zb->zbuf = (unsigned short *)gl_malloc(size);
	if (zb->zbuf == NULL)
		goto error;

	size = zb->xsize * zb->ysize * sizeof(unsigned long);

	zb->zbuf2 = (unsigned long *)gl_malloc(size);
	if (zb->zbuf2 == NULL) {
		gl_free(zb->zbuf);
		goto error;
	}
	if (frame_buffer == NULL) {
		zb->pbuf = (PIXEL *)gl_malloc(zb->ysize * zb->linesize);
		if (zb->pbuf == NULL) {
			gl_free(zb->zbuf);
			gl_free(zb->zbuf2);
			goto error;
		}
		zb->frame_buffer_allocated = 1;
	} else {
		zb->frame_buffer_allocated = 0;
		zb->pbuf = (PIXEL *)frame_buffer;
	}

	zb->current_texture = NULL;

	return zb;
error:
	gl_free(zb);
	return NULL;
}

void ZB_close(ZBuffer *zb) {
    if (zb->frame_buffer_allocated)
		gl_free(zb->pbuf);

    gl_free(zb->zbuf);
    gl_free(zb->zbuf2);
    gl_free(zb);
}

void ZB_resize(ZBuffer *zb, void *frame_buffer, int xsize, int ysize) {
	int size;

	// xsize must be a multiple of 4
	xsize = xsize & ~3;

	zb->xsize = xsize;
	zb->ysize = ysize;
	zb->linesize = (xsize * PSZB + 3) & ~3;

	size = zb->xsize * zb->ysize * sizeof(unsigned short);

	gl_free(zb->zbuf);
	zb->zbuf = (unsigned short *)gl_malloc(size);

	size = zb->xsize * zb->ysize * sizeof(unsigned long);

	gl_free(zb->zbuf2);
	zb->zbuf2 = (unsigned long *)gl_malloc(size);

	if (zb->frame_buffer_allocated)
		gl_free(zb->pbuf);

	if (frame_buffer == NULL) {
		zb->pbuf = (PIXEL *)gl_malloc(zb->ysize * zb->linesize);
		zb->frame_buffer_allocated = 1;
	} else {
		zb->pbuf = (PIXEL *)frame_buffer;
		zb->frame_buffer_allocated = 0;
	}
}

static void ZB_copyBuffer(ZBuffer *zb, void *buf, int linesize) {
	unsigned char *p1;
	PIXEL *q;
	int y, n;

	q = zb->pbuf;
	p1 = (unsigned char *)buf;
	n = zb->xsize * PSZB;
	for (y = 0; y < zb->ysize; y++) {
		memcpy(p1, q, n);
		p1 += linesize;
		q = (PIXEL *)((char *) q + zb->linesize);
	}
}

void ZB_copyFrameBuffer(ZBuffer *zb, void *buf, int linesize) {
	switch (zb->mode) {
	case ZB_MODE_5R6G5B:
		ZB_copyBuffer(zb, buf, linesize);
		break;
	default:
		assert(0);
	}
}

// adr must be aligned on an 'int'
void memset_s(void *adr, int val, int count) {
	int i, n, v;
	unsigned int *p;
	unsigned short *q;

	p = (unsigned int *)adr;
	v = val | (val << 16);

	n = count >> 3;
	for (i = 0; i < n; i++) {
		p[0] = v;
		p[1] = v;
		p[2] = v;
		p[3] = v;
		p += 4;
	}

	q = (unsigned short *) p;
	n = count & 7;
	for (i = 0; i < n; i++)
		*q++ = val;
}

void memset_l(void *adr, int val, int count) {
	int i, n, v;
	unsigned int *p;

	p = (unsigned int *)adr;
	v = val;
	n = count >> 2;
	for (i = 0; i < n; i++) {
		p[0] = v;
		p[1] = v;
		p[2] = v;
		p[3] = v;
		p += 4;
	}

    n = count & 3;
    for (i = 0; i < n; i++)
	*p++ = val;
}

void ZB_clear(ZBuffer *zb, int clear_z, int z, 
			  int clear_color, int r, int g, int b) {
	int color;
	int y;
	PIXEL *pp;

	if (clear_z) {
		memset_s(zb->zbuf, z, zb->xsize * zb->ysize);
	}
	if (clear_z) {
		memset_l(zb->zbuf, z, zb->xsize * zb->ysize);
	}
	if (clear_color) {
		pp = zb->pbuf;
		for (y = 0; y < zb->ysize; y++) {
			color = RGB_TO_PIXEL(r, g, b);
			memset_s(pp, color, zb->xsize);
			pp = (PIXEL *)((char *)pp + zb->linesize);
		}
	}
}
