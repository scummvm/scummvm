
// Z buffer: 16,32 bits Z / 16 bits color

#include "common/scummsys.h"


#include "common/endian.h"

#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/zgl.h"

namespace TinyGL {

uint8 PSZB;

ZBuffer *ZB_open(int xsize, int ysize, const Graphics::PixelBuffer &frame_buffer) {
	ZBuffer *zb;
	int size;

	zb = (ZBuffer *)gl_malloc(sizeof(ZBuffer));
	if (!zb)
		return NULL;

	zb->xsize = xsize;
	zb->ysize = ysize;
	zb->cmode = frame_buffer.getFormat();
	PSZB = zb->pixelbytes = zb->cmode.bytesPerPixel;
	zb->pixelbits = zb->cmode.bytesPerPixel * 8;
	zb->linesize = (xsize * zb->pixelbytes + 3) & ~3;

	size = zb->xsize * zb->ysize * sizeof(unsigned int);

	zb->zbuf = (unsigned int *)gl_malloc(size);
	if (!zb->zbuf)
		goto error;

	if (!frame_buffer) {
		byte *pbuf = (byte *)gl_malloc(zb->ysize * zb->linesize);
		if (!pbuf) {
			gl_free(zb->zbuf);
			goto error;
		}
		zb->pbuf.set(zb->cmode, pbuf);
		zb->frame_buffer_allocated = 1;
	} else {
		zb->frame_buffer_allocated = 0;
		zb->pbuf = frame_buffer;
	}

	zb->current_texture = NULL;
	zb->shadow_mask_buf = NULL;

	zb->buffer.pbuf = zb->pbuf.getRawBuffer();
	zb->buffer.zbuf = zb->zbuf;

	return zb;
error:
	gl_free(zb);
	return NULL;
}

void ZB_close(ZBuffer *zb) {
    if (zb->frame_buffer_allocated)
		zb->pbuf.free();

    gl_free(zb->zbuf);
	gl_free(zb);
}

void ZB_resize(ZBuffer *zb, void *frame_buffer, int xsize, int ysize) {
	int size;

	// xsize must be a multiple of 4
	xsize = xsize & ~3;

	zb->xsize = xsize;
	zb->ysize = ysize;
	zb->linesize = (xsize * zb->pixelbytes + 3) & ~3;

	size = zb->xsize * zb->ysize * sizeof(unsigned int);

	gl_free(zb->zbuf);
	zb->zbuf = (unsigned int *)gl_malloc(size);

	if (zb->frame_buffer_allocated)
		zb->pbuf.free();

	if (!frame_buffer) {
		byte *pbuf = (byte *)gl_malloc(zb->ysize * zb->linesize);
		zb->pbuf.set(zb->cmode, pbuf);
		zb->frame_buffer_allocated = 1;
	} else {
		zb->pbuf = (byte *)frame_buffer;
		zb->frame_buffer_allocated = 0;
	}
}

static void ZB_copyBuffer(ZBuffer *zb, void *buf, int linesize) {
	unsigned char *p1;
	byte *q;
	int y, n;

	q = zb->pbuf.getRawBuffer();
	p1 = (unsigned char *)buf;
	n = zb->xsize * zb->pixelbytes;
	for (y = 0; y < zb->ysize; y++) {
		memcpy(p1, q, n);
		p1 += linesize;
		q = q + zb->linesize;
	}
}

void ZB_copyFrameBuffer(ZBuffer *zb, void *buf, int linesize) {
	ZB_copyBuffer(zb, buf, linesize);
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

void ZB_clear(ZBuffer *zb, int clear_z, int z, int clear_color, int r, int g, int b) {
	uint32 color;
	int y;
	byte *pp;

	if (clear_z) {
		memset_l(zb->zbuf, z, zb->xsize * zb->ysize);
	}
	if (clear_color) {
		pp = zb->pbuf.getRawBuffer();
		for (y = 0; y < zb->ysize; y++) {
			color = zb->cmode.RGBToColor(r, g, b);
			memset_s(pp, color, zb->xsize);
			pp = pp + zb->linesize;
		}
	}
}

Buffer *ZB_genOffscreenBuffer(ZBuffer *zb) {
	Buffer *buf = (Buffer *)gl_malloc(sizeof(Buffer));
	buf->pbuf = (byte *)gl_malloc(zb->ysize * zb->linesize);
	int size = zb->xsize * zb->ysize * sizeof(unsigned int);
	buf->zbuf = (unsigned int *)gl_malloc(size);

	return buf;
}

void ZB_delOffscreenBuffer(ZBuffer *zb, Buffer *buf) {
	gl_free(buf->pbuf);
	gl_free(buf->zbuf);
	gl_free(buf);
}

void ZB_blitOffscreenBuffer(ZBuffer *zb, Buffer *buf) {
	// TODO: could be faster, probably.
	if (buf->used) {
		for (int i = 0; i < zb->xsize * zb->ysize; ++i) {
			unsigned int d1 = buf->zbuf[i];
			unsigned int d2 = zb->zbuf[i];
			if (d1 > d2) {
				const int offset = i * PSZB;
				memcpy(zb->pbuf.getRawBuffer() + offset, buf->pbuf + offset, PSZB);
				memcpy(zb->zbuf + i, buf->zbuf + i, sizeof(int));
			}
		}
	}
}

void ZB_selectOffscreenBuffer(ZBuffer *zb, Buffer *buf) {
	if (buf) {
		zb->pbuf = buf->pbuf;
		zb->zbuf = buf->zbuf;
		buf->used = true;
	} else {
		zb->pbuf = zb->buffer.pbuf;
		zb->zbuf = zb->buffer.zbuf;
	}
}

void ZB_clearOffscreenBuffer(ZBuffer *zb, Buffer *buf) {
	memset(buf->pbuf, 0, zb->ysize * zb->linesize);
	memset(buf->zbuf, 0, zb->ysize * zb->xsize * sizeof(unsigned int));
	buf->used = false;
}

} // end of namespace TinyGL
