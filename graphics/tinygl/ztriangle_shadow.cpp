
#include "graphics/tinygl/zbuffer.h"

namespace TinyGL {

#define ZCMP(z, zpix) ((z) >= (zpix))

void ZB_fillTriangleFlatShadowMask(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	ZBufferPoint *t, *pr1 = 0, *pr2 = 0, *l1 = 0, *l2 = 0;
	float fdx1, fdx2, fdy1, fdy2, fz;
	unsigned char *pm1;
	int part, update_left, update_right;

	int nb_lines, dx1, dy1, tmp, dx2, dy2;

	int error = 0, derror = 0;
	int x1 = 0, dxdy_min = 0, dxdy_max = 0;
	// warning: x2 is multiplied by 2^16
	int x2 = 0, dx2dy2 = 0;

	// we sort the vertex with increasing y
	if (p1->y < p0->y) {
		t = p0;
		p0 = p1;
		p1 = t;
	}
	if (p2->y < p0->y) {
		t = p2;
		p2 = p1;
		p1 = p0;
		p0 = t;
	} else if (p2->y < p1->y) {
		t = p1;
		p1 = p2;
		p2 = t;
	}

	// we compute dXdx and dXdy for all interpolated values

	fdx1 = (float)(p1->x - p0->x);
	fdy1 = (float)(p1->y - p0->y);

	fdx2 = (float)(p2->x - p0->x);
	fdy2 = (float)(p2->y - p0->y);

	fz = fdx1 * fdy2 - fdx2 * fdy1;
	if (fz == 0)
		return;
	fz = (float)(1.0 / fz);

	fdx1 *= fz;
	fdy1 *= fz;
	fdx2 *= fz;
	fdy2 *= fz;

	// screen coordinates

	pm1 = zb->shadow_mask_buf + zb->xsize * p0->y;

	for (part = 0; part < 2; part++) {
		if (part == 0) {
			if (fz > 0) {
				update_left = 1;
				update_right = 1;
				l1 = p0;
				l2 = p2;
				pr1 = p0;
				pr2 = p1;
			} else {
				update_left = 1;
				update_right = 1;
				l1 = p0;
				l2 = p1;
				pr1 = p0;
				pr2 = p2;
			}
			nb_lines = p1->y - p0->y;
		} else {
			// second part
			if (fz > 0) {
				update_left = 0;
				update_right = 1;
				pr1 = p1;
				pr2 = p2;
			} else {
				update_left = 1;
				update_right = 0;
				l1 = p1;
				l2 = p2;
			}
			nb_lines = p2->y - p1->y + 1;
		}

		// compute the values for the left edge
		if (update_left) {
			dy1 = l2->y - l1->y;
			dx1 = l2->x - l1->x;
			if (dy1 > 0)
				tmp = (dx1 << 16) / dy1;
			else
				tmp = 0;
			x1 = l1->x;
			error = 0;
			derror = tmp & 0x0000ffff;
			dxdy_min = tmp >> 16;
			dxdy_max = dxdy_min + 1;
		}

		// compute values for the right edge

		if (update_right) {
			dx2 = (pr2->x - pr1->x);
			dy2 = (pr2->y - pr1->y);
			if (dy2 > 0)
				dx2dy2 = ( dx2 << 16) / dy2;
			else
				dx2dy2 = 0;
			x2 = pr1->x << 16;
		}

		// we draw all the scan line of the part
		while (nb_lines > 0) {
			nb_lines--;
			// generic draw line
			{
				register unsigned char *pm;
				register int n;

				n = (x2 >> 16) - x1;
				pm = pm1 + x1;
				while (n >= 3) {
					for (int a = 0; a <= 3; a++) {
						pm[a] = 0xff;
					}
					pm += 4;
					n -= 4;
				}
				while (n >= 0) {
					pm[0] = 0xff;
					pm += 1;
					n -= 1;
				}
			}

			// left edge
			error += derror;
			if (error > 0) {
				error -= 0x10000;
				x1 += dxdy_max;
			} else {
				x1 += dxdy_min;
			}

			// right edge
			x2 += dx2dy2;

			// screen coordinates
			pm1 = pm1 + zb->xsize;
		}
	}
}

void ZB_fillTriangleFlatShadow(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	int color;
	ZBufferPoint *t, *pr1 = 0, *pr2 = 0, *l1 = 0, *l2 = 0;
	float fdx1, fdx2, fdy1, fdy2, fz, d1, d2;
	unsigned char *pm1;
	unsigned short *pz1;
	unsigned int *pz2;
	byte *pp1;
	int part, update_left, update_right;

	int nb_lines, dx1, dy1, tmp, dx2, dy2;

	int error = 0, derror = 0;
	int x1 = 0, dxdy_min = 0, dxdy_max = 0;
	// warning: x2 is multiplied by 2^16
	int x2 = 0, dx2dy2 = 0;

	int z1 = 0, dzdx, dzdy, dzdl_min = 0, dzdl_max = 0;

	// we sort the vertex with increasing y
	if (p1->y < p0->y) {
		t = p0;
		p0 = p1;
		p1 = t;
	}
	if (p2->y < p0->y) {
		t = p2;
		p2 = p1;
		p1 = p0;
		p0 = t;
	} else if (p2->y < p1->y) {
		t = p1;
		p1 = p2;
		p2 = t;
	}

	// we compute dXdx and dXdy for all interpolated values

	fdx1 = (float)(p1->x - p0->x);
	fdy1 = (float)(p1->y - p0->y);

	fdx2 = (float)(p2->x - p0->x);
	fdy2 = (float)(p2->y - p0->y);

	fz = fdx1 * fdy2 - fdx2 * fdy1;
	if (fz == 0)
		return;
	fz = (float)(1.0 / fz);

	fdx1 *= fz;
	fdy1 *= fz;
	fdx2 *= fz;
	fdy2 *= fz;

	d1 = (float)(p1->z - p0->z);
	d2 = (float)(p2->z - p0->z);
	dzdx = (int)(fdy2 * d1 - fdy1 * d2);
	dzdy = (int)(fdx1 * d2 - fdx2 * d1);

	// screen coordinates

	pp1 = zb->pbuf.getRawBuffer() + zb->linesize * p0->y;
	pm1 = zb->shadow_mask_buf + p0->y * zb->xsize;
	pz1 = zb->zbuf + p0->y * zb->xsize;
	pz2 = zb->zbuf2 + p0->y * zb->xsize;

	color = RGB_TO_PIXEL(zb->shadow_color_r, zb->shadow_color_g, zb->shadow_color_b);

	for (part = 0; part < 2; part++) {
		if (part == 0) {
			if (fz > 0) {
				update_left = 1;
				update_right = 1;
				l1 = p0;
				l2 = p2;
				pr1 = p0;
				pr2 = p1;
			} else {
				update_left = 1;
				update_right = 1;
				l1 = p0;
				l2 = p1;
				pr1 = p0;
				pr2 = p2;
			}
			nb_lines = p1->y - p0->y;
		} else {
			// second part
			if (fz > 0) {
				update_left = 0;
				update_right = 1;
				pr1 = p1;
				pr2 = p2;
			} else {
				update_left = 1;
				update_right = 0;
				l1 = p1;
				l2 = p2;
			}
			nb_lines = p2->y - p1->y + 1;
		}

		// compute the values for the left edge

		if (update_left) {
			dy1 = l2->y - l1->y;
			dx1 = l2->x - l1->x;
			if (dy1 > 0)
				tmp = (dx1 << 16) / dy1;
			else
				tmp = 0;
			x1 = l1->x;
			error = 0;
			derror = tmp & 0x0000ffff;
			dxdy_min = tmp >> 16;
			dxdy_max = dxdy_min + 1;

			z1 = l1->z;
			dzdl_min = (dzdy + dzdx * dxdy_min);
			dzdl_max = dzdl_min + dzdx;
		}

		// compute values for the right edge

		if (update_right) {
			dx2 = (pr2->x - pr1->x);
			dy2 = (pr2->y - pr1->y);
			if (dy2>0)
				dx2dy2 = ( dx2 << 16) / dy2;
			else
				dx2dy2 = 0;
			x2 = pr1->x << 16;
		}

		// we draw all the scan line of the part

		while (nb_lines > 0) {
			nb_lines--;
			// generic draw line
			{
				register unsigned char *pm;
				register int n;
				register unsigned short *pz;
				register unsigned int *pz_2;
				register unsigned int z, zz;

				n = (x2 >> 16) - x1;

				Graphics::PixelBuffer buf = zb->pbuf;
				buf = pp1 + x1 * PSZB;

				pm = pm1 + x1;
				pz = pz1 + x1;
				pz_2 = pz2 + x1;
				z = z1;
				while (n >= 3) {
					for (int a = 0; a < 4; a++) {
						zz = z >> ZB_POINT_Z_FRAC_BITS;
						if ((ZCMP(zz, pz[a])) && (ZCMP(z, pz_2[a])) && pm[0]) {
							buf.setPixelAt(a, color);
							pz_2[a] = z;
						}
						z += dzdx;
					}
					pz += 4;
					pz_2 += 4;
					pm += 4;
					buf.shiftBy(4);
					n -= 4;
				}
				while (n >= 0) {
					zz = z >> ZB_POINT_Z_FRAC_BITS;
					if ((ZCMP(zz, pz[0])) && (ZCMP(z, pz_2[0])) && pm[0]) {
						buf.setPixelAt(0, color);
						pz_2[0] = z;
					}
					pz += 1;
					pz_2 += 1;
					pm += 1;
					buf.shiftBy(1);
					n -= 1;
				}
			}

			// left edge
			error += derror;
			if (error > 0) {
				error -= 0x10000;
				x1 += dxdy_max;
				z1 += dzdl_max;
			} else {
				x1 += dxdy_min;
				z1 += dzdl_min;
			}

			// right edge
			x2 += dx2dy2;

			// screen coordinates
			pp1 += zb->linesize;
			pz1 += zb->xsize;
			pz2 += zb->xsize;
			pm1 += zb->xsize;
		}
	}
}

} // end of namespace TinyGL
