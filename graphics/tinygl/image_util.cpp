
#include "graphics/tinygl/zgl.h"

namespace TinyGL {

// image conversion

void gl_convertRGB_to_5R6G5B8A(unsigned short *pixmap, unsigned char *rgba, int xsize, int ysize) {
	int i, n;
	unsigned char *p, *p2;

	p = rgba;
	n = xsize * ysize;
	p2 = (unsigned char *)pixmap;
	for (i = 0; i < n; i++) {
		unsigned short pixel = ((p[0] & 0xF8) << 8) | ((p[1] & 0xFC) << 3) | ((p[2] & 0xF8) >> 3);
#ifdef SCUMM_BIG_ENDIAN
		p2[3 * i + 0] = pixel >> 8;
		p2[3 * i + 1] = pixel & 0xff;
#else
		p2[3 * i + 0] = pixel & 0xff;
		p2[3 * i + 1] = pixel >> 8;
#endif
		p2[3 * i + 2] = p[3];
		p += 4;
	}
}

// linear interpolation with xf, yf normalized to 2^16

#define INTERP_NORM_BITS  16
#define INTERP_NORM       (1 << INTERP_NORM_BITS)

static inline int interpolate(int v00, int v01, int v10, int xf, int yf) {
	return v00 + (((v01 - v00) * xf + (v10 - v00) * yf) >> INTERP_NORM_BITS);
}

// TODO: more accurate resampling 

void gl_resizeImage(unsigned char *dest, int xsize_dest, int ysize_dest,
                    unsigned char *src, int xsize_src, int ysize_src) {
	unsigned char *pix, *pix_src;
	int point1_offset = 0, point2_offset = 0, point3_offset = 0;
	float x1, y1, x1inc, y1inc;
	int xi, yi, j, xf, yf, x, y;

	pix = dest;
	pix_src = src;
  
	x1inc = (float)(xsize_src - 1) / (float)(xsize_dest - 1);
	y1inc = (float)(ysize_src - 1) / (float)(ysize_dest - 1);

	y1 = 0;
	for (y = 0; y < ysize_dest; y++) {
		x1 = 0;
		for (x = 0; x < xsize_dest; x++) {
			xi = (int)x1;
			yi = (int)y1;
			xf = (int)((x1 - floor(x1)) * INTERP_NORM);
			yf = (int)((y1 - floor(y1)) * INTERP_NORM);

			if ((xf + yf) <= INTERP_NORM) {
				for (j = 0; j < 3; j++) {
					point1_offset = (yi * xsize_src + xi) * 4 + j;
					if ((xi + 1) < xsize_src)
						point2_offset = (yi * xsize_src + xi + 1) * 4 + j;
					else
						point2_offset = point1_offset;
					if ((yi + 1) < ysize_src)
						point3_offset = ((yi + 1) * xsize_src + xi) * 4 + j;
					else
						point3_offset = point1_offset;
					pix[j] = interpolate(pix_src[point1_offset], pix_src[point2_offset], pix_src[point3_offset], xf, yf);
				}
				pix[3] = pix_src[(yi * xsize_src + xi) * 4 + 3];
			} else {
				xf = INTERP_NORM - xf;
				yf = INTERP_NORM - yf;
				for (j = 0; j < 3; j++) {
					pix[j] = interpolate(pix_src[point1_offset], pix_src[point2_offset], pix_src[point3_offset], xf, yf);
					if ((xi + 1) < xsize_src) {
						if ((yi + 1) < ysize_src)
							point1_offset = ((yi + 1) * xsize_src + xi + 1) * 4 + j;
						else
							point1_offset = (yi * xsize_src + xi + 1) * 4 + j;
					} else {
						if ((yi + 1) < ysize_src)
							point1_offset = ((yi + 1) * xsize_src + xi) * 4 + j;
						else
							point1_offset = (yi * xsize_src + xi) * 4 + j;
					}
					if ((yi + 1) < ysize_src)
						point2_offset = ((yi + 1) * xsize_src + xi) * 4 + j;
					else
						point2_offset = (yi * xsize_src + xi) * 4 + j;
					if ((xi + 1) < xsize_src)
						point3_offset = (yi * xsize_src + xi + 1) * 4 + j;
					else
						point3_offset = (yi * xsize_src + xi) * 4 + j;
					pix[j] = interpolate(pix_src[point1_offset], pix_src[point2_offset], pix_src[point3_offset], xf, yf);
				}
				pix[3] = pix_src[(yi * xsize_src + xi) * 4 + 3];
			}
			pix += 4;
			x1 += x1inc;
		}
		y1 += y1inc;
	}
}

#define FRAC_BITS 16

// resizing with no interlating nor nearest pixel
void gl_resizeImageNoInterpolate(unsigned char *dest, int xsize_dest, int ysize_dest,
                                 unsigned char *src, int xsize_src, int ysize_src) {
	unsigned char *pix, *pix_src, *pix1;
	int x1, y1, x1inc, y1inc;
	int xi, yi, x, y;

	pix = dest;
	pix_src = src;

	x1inc = (int)((float)((xsize_src) << FRAC_BITS) / (float)(xsize_dest));
	y1inc = (int)((float)((ysize_src) << FRAC_BITS) / (float)(ysize_dest));

	y1 = 0;
	for (y = 0; y < ysize_dest; y++) {
		x1 = 0;
		for (x = 0; x < xsize_dest; x++) {
			xi = x1 >> FRAC_BITS;
			yi = y1 >> FRAC_BITS;
			pix1 = pix_src + (yi * xsize_src + xi) * 4;

			pix[0] = pix1[0];
			pix[1] = pix1[1];
			pix[2] = pix1[2];
			pix[3] = pix1[3];

			pix += 4;
			x1 += x1inc;
		}
		y1 += y1inc;
	}
}

} // end of namespace TinyGL
