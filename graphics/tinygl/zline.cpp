
#include "graphics/tinygl/zbuffer.h"

namespace TinyGL {

template <bool interpRGB, bool interpZ, bool depthWrite>
FORCEINLINE static void putPixel(FrameBuffer *buffer, int pixelOffset,
                                 const Graphics::PixelFormat &cmode, unsigned int *pz, unsigned int &z, int &color, unsigned int &r,
                                 unsigned int &g, unsigned int &b) {
	if (buffer->scissorPixel(pixelOffset)) {
		return;
	}
	if (interpZ) {
		if (buffer->compareDepth(z, *pz)) {
			if (interpRGB) {
				buffer->writePixel(pixelOffset, RGB_TO_PIXEL(r, g, b));
			} else {
				buffer->writePixel(pixelOffset, color);
			}
			if (depthWrite) {
				*pz = z;
			}
		}
	} else {
		if (interpRGB) {
			buffer->writePixel(pixelOffset, RGB_TO_PIXEL(r, g, b));
		} else {
			buffer->writePixel(pixelOffset, color);
		}
	}
}

template <bool interpRGB, bool interpZ, bool depthWrite>
FORCEINLINE static void drawLine(FrameBuffer *buffer, ZBufferPoint *p1, ZBufferPoint *p2,
                                 int &pixelOffset, const Graphics::PixelFormat &cmode, unsigned int *pz, unsigned int &z, int &color,
                                 unsigned int &r, unsigned int &g, unsigned int &b, int dx, int dy, int inc_1, int inc_2) {
	int n = dx;
	int rinc, ginc, binc;
	int zinc;
	if (interpZ) {
		zinc = (p2->z - p1->z) / n;
	}
	if (interpRGB) {
		rinc = ((p2->r - p1->r) << 8) / n;
		ginc = ((p2->g - p1->g) << 8) / n;
		binc = ((p2->b - p1->b) << 8) / n;
	}
	int a = 2 * dy - dx;
	dy = 2 * dy;
	dx = 2 * dx - dy;
	int pp_inc_1 = (inc_1);
	int pp_inc_2 = (inc_2);
	do {
		putPixel<interpRGB, interpZ, depthWrite>(buffer, pixelOffset, cmode, pz, z, color, r, g, b);
		if (interpZ) {
			z += zinc;
		}
		if (interpRGB) {
			r += rinc;
			g += ginc;
			b += binc;
		}
		if (a > 0) {
			pixelOffset += pp_inc_1;
			if (interpZ) {
				pz += inc_1;
			}
			a -= dx;
		} else {
			pixelOffset += pp_inc_2;
			if (interpZ) {
				pz += inc_2;
			}
			a += dy;
		}
	} while (--n >= 0);
}

template <bool interpRGB, bool interpZ, bool depthWrite>
void FrameBuffer::fillLineGeneric(ZBufferPoint *p1, ZBufferPoint *p2, int color) {
	int dx, dy, sx;
	unsigned int r, g, b;
	unsigned int *pz = NULL;
	unsigned int z;
	int pixelOffset;

	if (p1->y > p2->y || (p1->y == p2->y && p1->x > p2->x)) {
		ZBufferPoint *tmp;
		tmp = p1;
		p1 = p2;
		p2 = tmp;
	}
	sx = xsize;
	pixelOffset = xsize * p1->y + p1->x;
	if (interpZ) {
		pz = _zbuf + (p1->y * sx + p1->x);
		z = p1->z;
	}
	dx = p2->x - p1->x;
	dy = p2->y - p1->y;
	if (interpRGB) {
		r = p2->r << 8;
		g = p2->g << 8;
		b = p2->b << 8;
	}

	if (dx == 0 && dy == 0) {
		putPixel<interpRGB, interpZ, depthWrite>(this, pixelOffset, cmode, pz, z, color, r, g, b);
	} else if (dx > 0) {
		if (dx >= dy) {
			drawLine<interpRGB, interpZ, depthWrite>(this, p1, p2, pixelOffset, cmode, pz, z, color, r, g, b, dx, dy, sx + 1, 1);
		} else {
			drawLine<interpRGB, interpZ, depthWrite>(this, p1, p2, pixelOffset, cmode, pz, z, color, r, g, b, dx, dy, sx + 1, sx);
		}
	} else {
		dx = -dx;
		if (dx >= dy) {
			drawLine<interpRGB, interpZ, depthWrite>(this, p1, p2, pixelOffset, cmode, pz, z, color, r, g, b, dx, dy, sx - 1, -1);
		} else {
			drawLine<interpRGB, interpZ, depthWrite>(this, p1, p2, pixelOffset, cmode, pz, z, color, r, g, b, dx, dy, sx - 1, sx);
		}
	}
}

void FrameBuffer::plot(ZBufferPoint *p) {
	unsigned int *pz;
	unsigned int r, g, b;

	pz = _zbuf + (p->y * xsize + p->x);
	int col = RGB_TO_PIXEL(p->r, p->g, p->b);
	unsigned int z = p->z;
	if (_depthWrite)
		putPixel<false, true, true>(this, linesize * p->y + p->x * PSZB, cmode, pz, z, col, r, g, b);
	else 
		putPixel<false, true, false>(this, linesize * p->y + p->x * PSZB, cmode, pz, z, col, r, g, b);
}

void FrameBuffer::fillLineFlatZ(ZBufferPoint *p1, ZBufferPoint *p2, int color) {
	if (_depthWrite)
		fillLineGeneric<false, true, true>(p1, p2, color);
	else
		fillLineGeneric<false, true, false>(p1, p2, color);
}

// line with color interpolation
void FrameBuffer::fillLineInterpZ(ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_depthWrite)
		fillLineGeneric<true, true, true>(p1, p2, 0);
	else
		fillLineGeneric<true, true, false>(p1, p2, 0);
}

// no Z interpolation
void FrameBuffer::fillLineFlat(ZBufferPoint *p1, ZBufferPoint *p2, int color) {
	if (_depthWrite)
		fillLineGeneric<false, false, true>(p1, p2, color);
	else
		fillLineGeneric<false, false, false>(p1, p2, color);
}

void FrameBuffer::fillLineInterp(ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_depthWrite)
		fillLineGeneric<false, true, true>(p1, p2, 0);
	else
		fillLineGeneric<false, true, false>(p1, p2, 0);
}

void FrameBuffer::fillLineZ(ZBufferPoint *p1, ZBufferPoint *p2) {
	int color1, color2;

	color1 = RGB_TO_PIXEL(p1->r, p1->g, p1->b);
	color2 = RGB_TO_PIXEL(p2->r, p2->g, p2->b);

	// choose if the line should have its color interpolated or not
	if (color1 == color2) {
		fillLineFlatZ(p1, p2, color1);
	} else {
		fillLineInterpZ(p1, p2);
	}
}

void FrameBuffer::fillLine(ZBufferPoint *p1, ZBufferPoint *p2) {
	int color1, color2;

	color1 = RGB_TO_PIXEL(p1->r, p1->g, p1->b);
	color2 = RGB_TO_PIXEL(p2->r, p2->g, p2->b);

	// choose if the line should have its color interpolated or not
	if (color1 == color2) {
		fillLineFlat(p1, p2, color1);
	} else {
		fillLineInterp(p1, p2);
	}
}

} // end of namespace TinyGL
