
// Z buffer: 16,32 bits Z / 16 bits color

#include "common/scummsys.h"


#include "common/endian.h"

#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/zgl.h"

namespace TinyGL {

uint8 PSZB;

static void ZB_copyBuffer(ZBuffer *zb, void *buf, int linesize) {
	unsigned char *p1;
	byte *q;
	int n;

	q = zb->pbuf.getRawBuffer();
	p1 = (unsigned char *)buf;
	n = zb->xsize * zb->pixelbytes;
	for (int y = 0; y < zb->ysize; y++) {
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
	int n, v;
	unsigned int *p;
	unsigned short *q;

	p = (unsigned int *)adr;
	v = val | (val << 16);

	n = count >> 3;
	for (int i = 0; i < n; i++) {
		p[0] = v;
		p[1] = v;
		p[2] = v;
		p[3] = v;
		p += 4;
	}

	q = (unsigned short *) p;
	n = count & 7;
	for (int i = 0; i < n; i++)
		*q++ = val;
}

void memset_l(void *adr, int val, int count) {
	int n, v;
	unsigned int *p;

	p = (unsigned int *)adr;
	v = val;
	n = count >> 2;
	for (int i = 0; i < n; i++) {
		p[0] = v;
		p[1] = v;
		p[2] = v;
		p[3] = v;
		p += 4;
	}

	n = count & 3;
	for (int i = 0; i < n; i++)
		*p++ = val;
}

ZBuffer::ZBuffer(int xsize, int ysize, const Graphics::PixelBuffer &frame_buffer) {
	int size;

	this->xsize = xsize;
	this->ysize = ysize;
	this->cmode = frame_buffer.getFormat();
	PSZB = this->pixelbytes = this->cmode.bytesPerPixel;
	this->pixelbits = this->cmode.bytesPerPixel * 8;
	this->linesize = (xsize * this->pixelbytes + 3) & ~3;

	size = this->xsize * this->ysize * sizeof(unsigned int);

	this->zbuf = (unsigned int *)gl_malloc(size);

	if (!frame_buffer) {
		byte *pbuf = (byte *)gl_malloc(this->ysize * this->linesize);
		this->pbuf.set(this->cmode, pbuf);
		this->frame_buffer_allocated = 1;
	} else {
		this->frame_buffer_allocated = 0;
		this->pbuf = frame_buffer;
	}

	this->current_texture = NULL;
	this->shadow_mask_buf = NULL;

	this->buffer.pbuf = this->pbuf.getRawBuffer();
	this->buffer.zbuf = this->zbuf;
}

ZBuffer::~ZBuffer() {
	if (frame_buffer_allocated)
		pbuf.free();
	gl_free(zbuf);
}

Buffer* ZBuffer::genOffscreenBuffer() {
	Buffer *buf = (Buffer *)gl_malloc(sizeof(Buffer));
	buf->pbuf = (byte *)gl_malloc(this->ysize * this->linesize);
	int size = this->xsize * this->ysize * sizeof(unsigned int);
	buf->zbuf = (unsigned int *)gl_malloc(size);

	return buf;
}

void ZBuffer::delOffscreenBuffer(Buffer *buf) {
	gl_free(buf->pbuf);
	gl_free(buf->zbuf);
	gl_free(buf);
}

void ZBuffer::clear( int clear_z, int z, int clear_color, int r, int g, int b ) {
	uint32 color;
	byte *pp;

	if (clear_z) {
		memset_l(this->zbuf, z, this->xsize * this->ysize);
	}
	if (clear_color) {
		pp = this->pbuf.getRawBuffer();
		for (int y = 0; y < this->ysize; y++) {
			color = this->cmode.RGBToColor(r, g, b);
			memset_s(pp, color, this->xsize);
			pp = pp + this->linesize;
		}
	}
}

void ZBuffer::blitOffscreenBuffer( Buffer* buf ) {
	// TODO: could be faster, probably.
	if (buf->used) {
		for (int i = 0; i < this->xsize * this->ysize; ++i) {
			unsigned int d1 = buf->zbuf[i];
			unsigned int d2 = this->zbuf[i];
			if (d1 > d2) {
				const int offset = i * PSZB;
				memcpy(this->pbuf.getRawBuffer() + offset, buf->pbuf + offset, PSZB);
				memcpy(this->zbuf + i, buf->zbuf + i, sizeof(int));
			}
		}
	}
}

void ZBuffer::selectOffscreenBuffer( Buffer* buf ) {
	if (buf) {
		this->pbuf = buf->pbuf;
		this->zbuf = buf->zbuf;
		buf->used = true;
	} else {
		this->pbuf = this->buffer.pbuf;
		this->zbuf = this->buffer.zbuf;
	}
}

void ZBuffer::clearOffscreenBuffer( Buffer* buf ) {
	memset(buf->pbuf, 0, this->ysize * this->linesize);
	memset(buf->zbuf, 0, this->ysize * this->xsize * sizeof(unsigned int));
	buf->used = false;
}

} // end of namespace TinyGL
