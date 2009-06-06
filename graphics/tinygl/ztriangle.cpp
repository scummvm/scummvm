
#include "graphics/tinygl/zbuffer.h"

namespace TinyGL {

#define ZCMP(z, zpix) ((z) >= (zpix))

void ZB_fillTriangleFlat(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	int color;

#define INTERP_Z

#define DRAW_INIT()	{							\
	color = RGB_TO_PIXEL(p2->r, p2->g, p2->b);	\
}

#define PUT_PIXEL(_a) {						\
	zz = z >> ZB_POINT_Z_FRAC_BITS;			\
	if ((ZCMP(zz, pz[_a])) && (ZCMP(z, pz_2[_a]))) {	\
      pp[_a] = color;						\
      pz_2[_a] = z;							\
	}										\
	z += dzdx;								\
}

#include "graphics/tinygl/ztriangle.h"
}

// Smooth filled triangle.
// The code below is very tricky :)

void ZB_fillTriangleSmooth(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	int _drgbdx;

#define INTERP_Z
#define INTERP_RGB

#define SAR_RND_TO_ZERO(v,n) (v / (1 << n))

#define DRAW_INIT() {											\
	_drgbdx = (SAR_RND_TO_ZERO(drdx, 6) << 22) & 0xFFC00000;	\
	_drgbdx |= SAR_RND_TO_ZERO(dgdx, 5) & 0x000007FF;			\
	_drgbdx |= (SAR_RND_TO_ZERO(dbdx, 7) << 12) & 0x001FF000; 	\
}

#define PUT_PIXEL(_a) {						\
	zz = z >> ZB_POINT_Z_FRAC_BITS;			\
	if ((ZCMP(zz, pz[_a])) && (ZCMP(z, pz_2[_a]))) {	\
		tmp = rgb & 0xF81F07E0;				\
		pp[_a] = tmp | (tmp >> 16);			\
		pz_2[_a] = z;						\
	}										\
	z += dzdx;								\
	rgb = (rgb + drgbdx) & (~0x00200800);	\
}

#define DRAW_LINE()	{								\
	register unsigned short *pz;					\
	register unsigned int *pz_2;					\
	register PIXEL *pp;								\
	register unsigned int tmp, z, zz, rgb, drgbdx;	\
	register int n;									\
	n = (x2 >> 16) - x1;							\
	pp = pp1 + x1;									\
	pz = pz1 + x1;									\
	pz_2 = pz2 + x1;								\
	z = z1;											\
	rgb =(r1 << 16) & 0xFFC00000;					\
	rgb |= (g1 >> 5) & 0x000007FF;					\
	rgb |= (b1 << 5) & 0x001FF000;					\
	drgbdx = _drgbdx;								\
	while (n >= 3) {								\
		PUT_PIXEL(0);								\
		PUT_PIXEL(1);								\
		PUT_PIXEL(2);								\
		PUT_PIXEL(3);								\
		pz += 4;									\
		pz_2 += 4;									\
		pp += 4;									\
		n -= 4;										\
	}												\
	while (n >= 0) {								\
		PUT_PIXEL(0);								\
		pz += 1;									\
		pz_2 += 1;									\
		pp += 1;									\
		n -= 1;										\
	}												\
}

#include "graphics/tinygl/ztriangle.h"
}

void ZB_setTexture(ZBuffer *zb, PIXEL *texture) {
	zb->current_texture=texture;
}

void ZB_fillTriangleMapping(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	PIXEL *texture;

#define INTERP_Z
#define INTERP_ST

#define DRAW_INIT()	{				\
	texture = zb->current_texture;	\
}

#define PUT_PIXEL(_a) {						\
	zz = z >> ZB_POINT_Z_FRAC_BITS;			\
	if ((ZCMP(zz, pz[_a])) && (ZCMP(z, pz_2[_a]))) {	\
		pp[_a] = texture[((t & 0x3FC00000) | s) >> 14];	\
		pz_2[_a] = z;						\
	}										\
	z += dzdx;								\
	s += dsdx;								\
	t += dtdx;								\
}

#include "graphics/tinygl/ztriangle.h"
}

void ZB_fillTriangleMappingPerspective(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	PIXEL *texture;
	float fdzdx, fndzdx, ndszdx, ndtzdx;
	int _drgbdx;

#define NB_INTERP 8

	ZBufferPoint *t, *pr1 = 0, *pr2 = 0, *l1 = 0, *l2 = 0;
	float fdx1, fdx2, fdy1, fdy2, fz, d1, d2;
	unsigned short *pz1;
	unsigned int *pz2;
	PIXEL *pp1;
	int part, update_left, update_right;

	int nb_lines, dx1, dy1, tmp, dx2, dy2;

	int error = 0, derror = 0;
	int x1 = 0, dxdy_min = 0, dxdy_max = 0;
	// warning: x2 is multiplied by 2^16
	int x2 = 0, dx2dy2 = 0;

	int z1 = 0, dzdx, dzdy, dzdl_min = 0, dzdl_max = 0;

	int r1 = 0, drdx, drdy, drdl_min = 0, drdl_max = 0;
	int g1 = 0, dgdx, dgdy, dgdl_min = 0, dgdl_max = 0;
	int b1 = 0, dbdx, dbdy, dbdl_min = 0, dbdl_max = 0;

	float sz1 = 0.0, dszdx, dszdy, dszdl_min = 0.0, dszdl_max = 0.0;
	float tz1 = 0.0, dtzdx, dtzdy, dtzdl_min = 0.0, dtzdl_max = 0.0;

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

	d1 = (float)(p1->r - p0->r);
	d2 = (float)(p2->r - p0->r);
	drdx = (int)(fdy2 * d1 - fdy1 * d2);
	drdy = (int)(fdx1 * d2 - fdx2 * d1);

	d1 = (float)(p1->g - p0->g);
	d2 = (float)(p2->g - p0->g);
	dgdx = (int) (fdy2 * d1 - fdy1 * d2);
	dgdy = (int) (fdx1 * d2 - fdx2 * d1);

	d1 = (float)(p1->b - p0->b);
	d2 = (float)(p2->b - p0->b);
	dbdx = (int)(fdy2 * d1 - fdy1 * d2);
	dbdy = (int)(fdx1 * d2 - fdx2 * d1);
  
	{
		float zz;
		zz = (float)p0->z;
		p0->sz = (float)p0->s * zz;
		p0->tz = (float)p0->t * zz;
		zz = (float)p1->z;
		p1->sz = (float)p1->s * zz;
		p1->tz = (float)p1->t * zz;
		zz = (float)p2->z;
		p2->sz = (float)p2->s * zz;
		p2->tz = (float)p2->t * zz;

		d1 = p1->sz - p0->sz;
		d2 = p2->sz - p0->sz;
		dszdx = (fdy2 * d1 - fdy1 * d2);
		dszdy = (fdx1 * d2 - fdx2 * d1);

		d1 = p1->tz - p0->tz;
		d2 = p2->tz - p0->tz;
		dtzdx = (fdy2 * d1 - fdy1 * d2);
		dtzdy = (fdx1 * d2 - fdx2 * d1);
	}

	// screen coordinates

	pp1 = (PIXEL *)((char *)zb->pbuf + zb->linesize * p0->y);
	pz1 = zb->zbuf + p0->y * zb->xsize;
	pz2 = zb->zbuf2 + p0->y * zb->xsize;

	texture = zb->current_texture;
	fdzdx = (float)dzdx;
	fndzdx = NB_INTERP * fdzdx;
	ndszdx = NB_INTERP * dszdx;
	ndtzdx = NB_INTERP * dtzdx;
	_drgbdx = ((drdx / (1 << 6)) << 22) & 0xFFC00000;
	_drgbdx |= (dgdx / (1 << 5)) & 0x000007FF;
	_drgbdx |= ((dbdx / (1 << 7)) << 12) & 0x001FF000;

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

			r1 = l1->r;
			drdl_min = (drdy + drdx * dxdy_min);
			drdl_max = drdl_min + drdx;

			g1 = l1->g;
			dgdl_min = (dgdy + dgdx * dxdy_min);
			dgdl_max = dgdl_min + dgdx;

			b1 = l1->b;
			dbdl_min = (dbdy + dbdx * dxdy_min);
			dbdl_max = dbdl_min + dbdx;

			sz1 = l1->sz;
			dszdl_min = (dszdy + dszdx * dxdy_min);
			dszdl_max = dszdl_min + dszdx;

			tz1 = l1->tz;
			dtzdl_min = (dtzdy + dtzdx * dxdy_min);
			dtzdl_max = dtzdl_min + dtzdx;
		}

		// compute values for the right edge

		if (update_right) {
			dx2 = (pr2->x - pr1->x);
			dy2 = (pr2->y - pr1->y);
			if (dy2 > 0)
				dx2dy2 = (dx2 << 16) / dy2;
			else
				dx2dy2 = 0;
			x2 = pr1->x << 16;
		}

		// we draw all the scan line of the part

		while (nb_lines > 0) {
			nb_lines--;
			{
				register unsigned short *pz;
				register unsigned int *pz_2;
				register PIXEL *pp;
				register unsigned int s, t, z, zz, rgb, drgbdx;
				register int n, dsdx, dtdx;
				float sz, tz, fz, zinv;
				n = (x2 >> 16) - x1;
				fz = (float)z1;
				zinv = (float)(1.0 / fz);
				pp = (PIXEL *)((char *)pp1 + x1 * PSZB);
				pz = pz1 + x1;
				pz_2 = pz2 + x1;
				z = z1;
				sz = sz1;
				tz = tz1;
				rgb = (r1 << 16) & 0xFFC00000;
				rgb |= (g1 >> 5) & 0x000007FF;
				rgb |= (b1 << 5) & 0x001FF000;
				drgbdx = _drgbdx;
				while (n >= (NB_INTERP - 1)) {
					{
						float ss, tt;
						ss = sz * zinv;
						tt = tz * zinv;
						s = (int)ss;
						t = (int)tt;
						dsdx = (int)((dszdx - ss * fdzdx) * zinv);
						dtdx = (int)((dtzdx - tt * fdzdx) * zinv);
						fz += fndzdx;
						zinv = (float)(1.0 / fz);
					}
					for (int _a = 0; _a < 8; _a++) {
						zz = z >> ZB_POINT_Z_FRAC_BITS;
						if ((ZCMP(zz, pz[_a])) && (ZCMP(z, pz_2[_a]))) {
							unsigned ttt = (t & 0x003FC000) >> (9 - PSZSH);
							unsigned sss = (s & 0x003FC000) >> (17 - PSZSH);
							char *ptr = (char *)(texture) + (((ttt | sss) >> 1) * 3);
							PIXEL pixel = *(PIXEL *)ptr;
							char alpha = *(ptr + 2);
							if (alpha == '\xff') {
								tmp = rgb & 0xF81F07E0;
								unsigned int light = tmp | (tmp >> 16);
								unsigned int c_r = (pixel & 0xF800) >> 8;
								unsigned int c_g = (pixel & 0x07E0) >> 3;
								unsigned int c_b = (pixel & 0x001F) << 3;
								unsigned int l_r = (light & 0xF800) >> 8;
								unsigned int l_g = (light & 0x07E0) >> 3;
								unsigned int l_b = (light & 0x001F) << 3;
								c_r = (c_r * l_r) / 256;
								c_g = (c_g * l_g) / 256;
								c_b = (c_b * l_b) / 256;
								pixel = ((c_r & 0xF8) << 8) | ((c_g & 0xFC) << 3) | (c_b >> 3);
								pp[_a] = pixel;
								pz_2[_a] = z;
							}
						}
						z += dzdx;
						s += dsdx;
						t += dtdx;
						rgb = (rgb + drgbdx) & (~0x00200800);
					}

					pz += NB_INTERP;
					pz_2 += NB_INTERP;
					pp = (PIXEL *)((char *)pp + NB_INTERP * PSZB);
					n -= NB_INTERP;
					sz += ndszdx;
					tz += ndtzdx;
				}

				{
					float ss, tt;
					ss = sz * zinv;
					tt = tz * zinv;
					s = (int)ss;
					t = (int)tt;
					dsdx = (int)((dszdx - ss * fdzdx) * zinv);
					dtdx = (int)((dtzdx - tt * fdzdx) * zinv);
				}

				while (n >= 0) {
					{
						zz = z >> ZB_POINT_Z_FRAC_BITS;
						if ((ZCMP(zz, pz[0])) && (ZCMP(z, pz_2[0]))) {
							unsigned ttt = (t & 0x003FC000) >> (9 - PSZSH);
							unsigned sss = (s & 0x003FC000) >> (17 - PSZSH);
							char *ptr = (char *)(texture) + (((ttt | sss) >> 1) * 3);
							PIXEL pixel = *(PIXEL *)ptr;
							char alpha = *(ptr + 2);
							if (alpha == '\xff') {
								tmp = rgb & 0xF81F07E0;
								unsigned int light = tmp | (tmp >> 16);
								unsigned int c_r = (pixel & 0xF800) >> 8;
								unsigned int c_g = (pixel & 0x07E0) >> 3;
								unsigned int c_b = (pixel & 0x001F) << 3;
								unsigned int l_r = (light & 0xF800) >> 8;
								unsigned int l_g = (light & 0x07E0) >> 3;
								unsigned int l_b = (light & 0x001F) << 3;
								c_r = (c_r * l_r) / 256;
								c_g = (c_g * l_g) / 256;
								c_b = (c_b * l_b) / 256;
								pixel = ((c_r & 0xF8) << 8) | ((c_g & 0xFC) << 3) | (c_b >> 3);
								pp[0] = pixel;
								pz_2[0] = z;
							}
						}
						z += dzdx;
						s += dsdx;
						t += dtdx;
						rgb = (rgb + drgbdx) & (~0x00200800);
					}
					pz += 1;
					pz_2 += 1;
					pp = (PIXEL *)((char *)pp + PSZB);
					n -= 1;
				}
			}
  
			// left edge
			error += derror;
			if (error > 0) {
				error -= 0x10000;
				x1 += dxdy_max;

				z1 += dzdl_max;

				r1 += drdl_max;
				g1 += dgdl_max;
				b1 += dbdl_max;

				sz1 += dszdl_max;
				tz1 += dtzdl_max;
			} else {
				x1 += dxdy_min;

				z1 += dzdl_min;

				r1 += drdl_min;
				g1 += dgdl_min;
				b1 += dbdl_min;

				sz1 += dszdl_min;
				tz1 += dtzdl_min;
			}

			// right edge
			x2 += dx2dy2;

			// screen coordinates
			pp1 = (PIXEL *)((char *)pp1 + zb->linesize);
			pz1 += zb->xsize;
			pz2 += zb->xsize;
		}
	}
}

} // end of namespace TinyGL
