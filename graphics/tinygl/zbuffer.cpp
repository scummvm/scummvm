
// Z buffer: 16,32 bits Z / 16 bits color

#include "common/scummsys.h"


#include "common/endian.h"

#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/zgl.h"

namespace TinyGL {

uint8 PSZB;

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

FrameBuffer::FrameBuffer(int width, int height, const Graphics::PixelBuffer &frame_buffer) : _depthWrite(true) {
	int size;

	this->xsize = width;
	this->ysize = height;
	this->cmode = frame_buffer.getFormat();
	PSZB = this->pixelbytes = this->cmode.bytesPerPixel;
	this->pixelbits = this->cmode.bytesPerPixel * 8;
	this->linesize = (xsize * this->pixelbytes + 3) & ~3;

	this->setScissorRectangle(0, xsize, 0, ysize);

	size = this->xsize * this->ysize * sizeof(unsigned int);

	this->_zbuf = (unsigned int *)gl_malloc(size);

	if (!frame_buffer) {
		byte *pixelBuffer = (byte *)gl_malloc(this->ysize * this->linesize);
		this->pbuf.set(this->cmode, pixelBuffer);
		this->frame_buffer_allocated = 1;
	} else {
		this->frame_buffer_allocated = 0;
		this->pbuf = frame_buffer;
	}

	this->current_texture = NULL;
	this->shadow_mask_buf = NULL;

	this->buffer.pbuf = this->pbuf.getRawBuffer();
	this->buffer.zbuf = this->_zbuf;
	_blendingEnabled = false;
	_alphaTestEnabled = false;
	_depthFunc = TGL_LESS;
}

FrameBuffer::~FrameBuffer() {
	if (frame_buffer_allocated)
		pbuf.free();
	gl_free(_zbuf);
}

Buffer *FrameBuffer::genOffscreenBuffer() {
	Buffer *buf = (Buffer *)gl_malloc(sizeof(Buffer));
	buf->pbuf = (byte *)gl_malloc(this->ysize * this->linesize);
	int size = this->xsize * this->ysize * sizeof(unsigned int);
	buf->zbuf = (unsigned int *)gl_malloc(size);

	return buf;
}

void FrameBuffer::delOffscreenBuffer(Buffer *buf) {
	gl_free(buf->pbuf);
	gl_free(buf->zbuf);
	gl_free(buf);
}

void FrameBuffer::clear(int clearZ, int z, int clearColor, int r, int g, int b) {
	uint32 color;
	byte *pp;

	if (clearZ) {
		memset_l(this->_zbuf, z, this->xsize * this->ysize);
	}
	if (clearColor) {
		pp = this->pbuf.getRawBuffer();
		color = this->cmode.RGBToColor(r, g, b);
		for (int y = 0; y < this->ysize; y++) {
			memset_s(pp, color, this->xsize);
			pp = pp + this->linesize;
		}
	}
}

void FrameBuffer::clearRegion(int x, int y, int w, int h, int clearZ, int z, int clearColor, int r, int g, int b) {
	uint32 color;
	byte *pp;

	if (clearZ) {
		for (int row = y; row < y + h; row++) {
			memset_l(this->_zbuf + x + (row * this->xsize), z, w);
		}
	}
	if (clearColor) {
		pp = this->pbuf.getRawBuffer() + y * this->linesize;
		color = this->cmode.RGBToColor(r, g, b);
		for (int row = y; row < y + h; row++) {
			memset_s(pp + x * this->pixelbytes, color, w);
			pp = pp + this->linesize;
		}
	}
}


void FrameBuffer::blitOffscreenBuffer(Buffer *buf) {
	// TODO: could be faster, probably.
	if (buf->used) {
		for (int i = 0; i < this->xsize * this->ysize; ++i) {
			unsigned int d1 = buf->zbuf[i];
			unsigned int d2 = this->_zbuf[i];
			if (d1 > d2) {
				const int offset = i * PSZB;
				memcpy(this->pbuf.getRawBuffer() + offset, buf->pbuf + offset, PSZB);
				memcpy(this->_zbuf + i, buf->zbuf + i, sizeof(int));
			}
		}
	}
}

void FrameBuffer::selectOffscreenBuffer(Buffer *buf) {
	if (buf) {
		this->pbuf = buf->pbuf;
		this->_zbuf = buf->zbuf;
		buf->used = true;
	} else {
		this->pbuf = this->buffer.pbuf;
		this->_zbuf = this->buffer.zbuf;
	}
}

void FrameBuffer::clearOffscreenBuffer(Buffer *buf) {
	memset(buf->pbuf, 0, this->ysize * this->linesize);
	memset(buf->zbuf, 0, this->ysize * this->xsize * sizeof(unsigned int));
	buf->used = false;
}

void FrameBuffer::setTexture(const Graphics::PixelBuffer &texture) {
	current_texture = texture;
}

} // end of namespace TinyGL
