/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-2022 Fabrice Bellard,
 * which is licensed under the MIT license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "common/endian.h"
#include "graphics/tinygl/texelbuffer.h"
#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/zgl.h"

namespace TinyGL {

static const int NB_INTERP = 8;

template <bool kDepthWrite, bool kSmoothMode, bool kFogMode, bool kEnableAlphaTest, bool kEnableScissor, bool kEnableBlending, bool kStencilEnabled, bool kDepthTestEnabled>
FORCEINLINE void FrameBuffer::putPixelNoTexture(int fbOffset, uint *pz, byte *ps, int _a,
                                                int x, int y, uint &z, uint &r, uint &g, uint &b, uint &a,
                                                int &dzdx, int &drdx, int &dgdx, int &dbdx, uint dadx,
                                                uint &fog, int fog_r, int fog_g, int fog_b, int &dfdx) {
	if (kEnableScissor && scissorPixel(x + _a, y)) {
		return;
	}
	if (kStencilEnabled) {
		bool stencilResult = stencilTest(ps[_a]);
		if (!stencilResult) {
			stencilOp(false, true, ps + _a);
			return;
		}
	}
	bool depthTestResult;
	if (kDepthTestEnabled) {
		depthTestResult = compareDepth(z, pz[_a]);
	} else {
		depthTestResult = true;
	}
	if (kStencilEnabled) {
		stencilOp(true, depthTestResult, ps + _a);
	}
	if (depthTestResult) {
		writePixel<kEnableAlphaTest, kEnableBlending, kDepthWrite, kFogMode>
		          (fbOffset + _a, a >> (ZB_POINT_ALPHA_BITS - 8), r >> (ZB_POINT_RED_BITS - 8), g >> (ZB_POINT_GREEN_BITS - 8), b >> (ZB_POINT_BLUE_BITS - 8),
		          z, fog, fog_r, fog_g, fog_b);
	}
	z += dzdx;
	if (kFogMode) {
		fog += dfdx;
	}
	if (kSmoothMode) {
		r += drdx;
		g += dgdx;
		b += dbdx;
		a += dadx;
	}
}

template <bool kDepthWrite, bool kLightsMode, bool kSmoothMode, bool kFogMode, bool kEnableAlphaTest, bool kEnableScissor, bool kEnableBlending, bool kStencilEnabled, bool kDepthTestEnabled>
FORCEINLINE void FrameBuffer::putPixelTexture(int fbOffset, const TexelBuffer *texture,
                                              uint wrap_s, uint wrap_t, uint *pz, byte *ps, int _a,
                                              int x, int y, uint &z, int &t, int &s,
                                              uint &r, uint &g, uint &b, uint &a,
                                              int &dzdx, int &dsdx, int &dtdx, int &drdx, int &dgdx, int &dbdx, uint dadx,
                                              uint &fog, int fog_r, int fog_g, int fog_b, int &dfdx) {
	if (kEnableScissor && scissorPixel(x + _a, y)) {
		return;
	}
	if (kStencilEnabled) {
		bool stencilResult = stencilTest(ps[_a]);
		if (!stencilResult) {
			stencilOp(false, true, ps + _a);
			return;
		}
	}
	bool depthTestResult;
	if (kDepthTestEnabled) {
		depthTestResult = compareDepth(z, pz[_a]);
	} else {
		depthTestResult = true;
	}
	if (kStencilEnabled) {
		stencilOp(true, depthTestResult, ps + _a);
	}
	if (depthTestResult) {
		uint8 c_a, c_r, c_g, c_b;
		texture->getARGBAt(wrap_s, wrap_t, s, t, c_a, c_r, c_g, c_b);
		if (kLightsMode) {
			uint l_a = (a >> (ZB_POINT_ALPHA_BITS - 8));
			uint l_r = (r >> (ZB_POINT_RED_BITS - 8));
			uint l_g = (g >> (ZB_POINT_GREEN_BITS - 8));
			uint l_b = (b >> (ZB_POINT_BLUE_BITS - 8));
			c_a = (c_a * l_a) >> (ZB_POINT_ALPHA_BITS - 8);
			c_r = (c_r * l_r) >> (ZB_POINT_RED_BITS - 8);
			c_g = (c_g * l_g) >> (ZB_POINT_GREEN_BITS - 8);
			c_b = (c_b * l_b) >> (ZB_POINT_BLUE_BITS - 8);
		}
		writePixel<kEnableAlphaTest, kEnableBlending, kDepthWrite, kFogMode>(fbOffset + _a, c_a, c_r, c_g, c_b, z, fog >> ZB_FOG_BITS, fog_r, fog_g, fog_b);
	}
	z += dzdx;
	s += dsdx;
	t += dtdx;
	if (kFogMode) {
		fog += dfdx;
	}
	if (kSmoothMode) {
		a += dadx;
		r += drdx;
		g += dgdx;
		b += dbdx;
	}
}

template <bool kDepthWrite, bool kEnableScissor, bool kStencilEnabled, bool kDepthTestEnabled>
FORCEINLINE void FrameBuffer::putPixelDepth(uint *pz, byte *ps, int _a, int x, int y, uint &z, int &dzdx) {
	if (kEnableScissor && scissorPixel(x + _a, y)) {
		return;
	}
	if (kStencilEnabled) {
		bool stencilResult = stencilTest(ps[_a]);
		if (!stencilResult) {
			stencilOp(false, true, ps + _a);
			return;
		}
	}
	bool depthTestResult;
	if (kDepthTestEnabled) {
		depthTestResult = compareDepth(z, pz[_a]);
	} else {
		depthTestResult = true;
	}
	if (kStencilEnabled) {
		stencilOp(true, depthTestResult, ps + _a);
	}
	if (kDepthWrite && depthTestResult) {
		pz[_a] = z;
	}
	z += dzdx;
}

template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode,
          bool kDepthWrite, bool kFogMode, bool kAlphaTestEnabled, bool kEnableScissor,
          bool kBlendingEnabled, bool kStencilEnabled, bool kDepthTestEnabled>
void FrameBuffer::fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const TexelBuffer *texture;
	float fdzdx = 0, fndzdx = 0, ndszdx = 0, ndtzdx = 0;

	ZBufferPoint *tp, *pr1 = 0, *pr2 = 0, *l1 = 0, *l2 = 0;
	float fdx1, fdx2, fdy1, fdy2, fz0, d1, d2;
	uint *pz1 = nullptr;
	byte *ps1 = nullptr;
	int part, update_left = 1, update_right = 1;

	int nb_lines, dx1, dy1, tmp, dx2, dy2, y;

	int error = 0, derror = 0;
	int x1 = 0, dxdy_min = 0, dxdy_max = 0;
	// warning: x2 is multiplied by 2^16
	int x2 = 0, dx2dy2 = 0;

	int z1 = 0, dzdx = 0, dzdy = 0, dzdl_min = 0, dzdl_max = 0;
	int f1 = 0, dfdx = 0, dfdy = 0, dfdl_min = 0, dfdl_max = 0;

	int r1 = 0, drdx = 0, drdy = 0, drdl_min = 0, drdl_max = 0;
	int g1 = 0, dgdx = 0, dgdy = 0, dgdl_min = 0, dgdl_max = 0;
	int b1 = 0, dbdx = 0, dbdy = 0, dbdl_min = 0, dbdl_max = 0;
	int a1 = 0, dadx = 0, dady = 0, dadl_min = 0, dadl_max = 0;

	float sz1 = 0.0, dszdx = 0, dszdy = 0, dszdl_min = 0.0, dszdl_max = 0.0;
	float tz1 = 0.0, dtzdx = 0, dtzdy = 0, dtzdl_min = 0.0, dtzdl_max = 0.0;

	byte fog_r = 0, fog_g = 0, fog_b = 0;

	// we sort the vertex with increasing y
	if (p1->y < p0->y) {
		tp = p0;
		p0 = p1;
		p1 = tp;
	}
	if (p2->y < p0->y) {
		tp = p2;
		p2 = p1;
		p1 = p0;
		p0 = tp;
	} else if (p2->y < p1->y) {
		tp = p1;
		p1 = p2;
		p2 = tp;
	}

	// we compute dXdx and dXdy for all interpolated values

	fdx1 = (float)(p1->x - p0->x);
	fdy1 = (float)(p1->y - p0->y);

	fdx2 = (float)(p2->x - p0->x);
	fdy2 = (float)(p2->y - p0->y);

	fz0 = fdx1 * fdy2 - fdx2 * fdy1;
	if (fz0 == 0)
		return;
	fz0 = (float)(1.0 / fz0);

	fdx1 *= fz0;
	fdy1 *= fz0;
	fdx2 *= fz0;
	fdy2 *= fz0;

	if (kInterpRGB && kFogMode) {
		fog_r = _fogColorR * 255;
		fog_g = _fogColorG * 255;
		fog_b = _fogColorB * 255;
		d1 = (float)(p1->f - p0->f);
		d2 = (float)(p2->f - p0->f);
		dfdx = (int)(fdy2 * d1 - fdy1 * d2);
		dfdy = (int)(fdx1 * d2 - fdx2 * d1);
	}

	if (kInterpZ) {
		d1 = (float)(p1->z - p0->z);
		d2 = (float)(p2->z - p0->z);
		dzdx = (int)(fdy2 * d1 - fdy1 * d2);
		dzdy = (int)(fdx1 * d2 - fdx2 * d1);
	}

	if (kInterpRGB && kSmoothMode) {
		d1 = (float)(p1->r - p0->r);
		d2 = (float)(p2->r - p0->r);
		drdx = (int)(fdy2 * d1 - fdy1 * d2);
		drdy = (int)(fdx1 * d2 - fdx2 * d1);

		d1 = (float)(p1->g - p0->g);
		d2 = (float)(p2->g - p0->g);
		dgdx = (int)(fdy2 * d1 - fdy1 * d2);
		dgdy = (int)(fdx1 * d2 - fdx2 * d1);

		d1 = (float)(p1->b - p0->b);
		d2 = (float)(p2->b - p0->b);
		dbdx = (int)(fdy2 * d1 - fdy1 * d2);
		dbdy = (int)(fdx1 * d2 - fdx2 * d1);

		d1 = (float)(p1->a - p0->a);
		d2 = (float)(p2->a - p0->a);
		dadx = (int)(fdy2 * d1 - fdy1 * d2);
		dady = (int)(fdx1 * d2 - fdx2 * d1);
	}

	if (kInterpST || kInterpSTZ) {
		if (kInterpSTZ) {
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
		} else {
			p0->sz = (float)p0->s;
			p0->tz = (float)p0->t;
			p1->sz = (float)p1->s;
			p1->tz = (float)p1->t;
			p2->sz = (float)p2->s;
			p2->tz = (float)p2->t;
		}

		d1 = p1->sz - p0->sz;
		d2 = p2->sz - p0->sz;
		dszdx = (fdy2 * d1 - fdy1 * d2);
		dszdy = (fdx1 * d2 - fdx2 * d1);

		d1 = p1->tz - p0->tz;
		d2 = p2->tz - p0->tz;
		dtzdx = (fdy2 * d1 - fdy1 * d2);
		dtzdy = (fdx1 * d2 - fdx2 * d1);
	}

	int polyOffset = 0;
	if (kInterpZ && kInterpRGB && (_offsetStates & TGL_OFFSET_FILL)) {
		int m = MAX(ABS(dzdx), ABS(dzdy));
		polyOffset = -m * _offsetFactor + -_offsetUnits * (1 << 6);
	}

	// screen coordinates

	int pp1 = _pbufWidth * p0->y;
	pz1 = _zbuf + p0->y * _pbufWidth;
	if (kStencilEnabled) {
		ps1 = _sbuf + p0->y * _pbufWidth;
	}

	if (kInterpRGB && !kSmoothMode) {
		r1 = p2->r;
		g1 = p2->g;
		b1 = p2->b;
		a1 = p2->a;
	}

	if (kInterpRGB && (kInterpST || kInterpSTZ)) {
		texture = _currentTexture;
		fdzdx = (float)dzdx;
		fndzdx = NB_INTERP * fdzdx;
		ndszdx = NB_INTERP * dszdx;
		ndtzdx = NB_INTERP * dtzdx;
	}

	if (fz0 > 0) {
		l1 = p0;
		l2 = p2;
		pr1 = p0;
		pr2 = p1;
	} else {
		l1 = p0;
		l2 = p1;
		pr1 = p0;
		pr2 = p2;
	}
	nb_lines = p1->y - p0->y;
	y = p0->y;
	for (part = 0; part < 2; part++) {
		if (part == 1) {
			// second part
			if (fz0 > 0) {
				update_left = 0;
				pr1 = p1;
				pr2 = p2;
			} else {
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

			if (kInterpRGB && kFogMode) {
				f1 = l1->f;
				dfdl_min = (dfdy + dfdx * dxdy_min);
				dfdl_max = dfdl_min + dfdx;
			}

			if (kInterpZ) {
				z1 = l1->z + polyOffset;
				dzdl_min = (dzdy + dzdx * dxdy_min);
				dzdl_max = dzdl_min + dzdx;
			}

			if (kInterpRGB && kSmoothMode) {
				r1 = l1->r;
				drdl_min = (drdy + drdx * dxdy_min);
				drdl_max = drdl_min + drdx;

				g1 = l1->g;
				dgdl_min = (dgdy + dgdx * dxdy_min);
				dgdl_max = dgdl_min + dgdx;

				b1 = l1->b;
				dbdl_min = (dbdy + dbdx * dxdy_min);
				dbdl_max = dbdl_min + dbdx;

				a1 = l1->a;
				dadl_min = (dady + dadx * dxdy_min);
				dadl_max = dadl_min + dadx;
			}

			if (kInterpST || kInterpSTZ) {
				sz1 = l1->sz;
				dszdl_min = (dszdy + dszdx * dxdy_min);
				dszdl_max = dszdl_min + dszdx;

				tz1 = l1->tz;
				dtzdl_min = (dtzdy + dtzdx * dxdy_min);
				dtzdl_max = dtzdl_min + dtzdx;
			}
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
			int x = x1;
			if (!kInterpRGB) {
				int n;
				uint *pz;
				byte *ps = nullptr;
				uint z;
				n = (x2 >> 16) - x1;
				if (kInterpZ) {
					pz = pz1 + x1;
					z = z1;
				}
				if (kStencilEnabled) {
					ps = ps1 + x1;
				}
				while (n >= 3) {
					putPixelDepth<kDepthWrite, kEnableScissor, kStencilEnabled, kDepthTestEnabled>(pz, ps, 0, x, y, z, dzdx);
					putPixelDepth<kDepthWrite, kEnableScissor, kStencilEnabled, kDepthTestEnabled>(pz, ps, 1, x, y, z, dzdx);
					putPixelDepth<kDepthWrite, kEnableScissor, kStencilEnabled, kDepthTestEnabled>(pz, ps, 2, x, y, z, dzdx);
					putPixelDepth<kDepthWrite, kEnableScissor, kStencilEnabled, kDepthTestEnabled>(pz, ps, 3, x, y, z, dzdx);
					if (kInterpZ) {
						pz += 4;
					}
					if (kStencilEnabled) {
						ps += 4;
					}
					n -= 4;
					x += 4;
				}
				while (n >= 0) {
					putPixelDepth<kDepthWrite, kEnableScissor, kStencilEnabled, kDepthTestEnabled>(pz, ps, 0, x, y, z, dzdx);
					if (kInterpZ) {
						pz += 1;
					}
					if (kStencilEnabled) {
						ps += 1;
					}
					n -= 1;
					x += 1;
				}
			} else if (!(kInterpST || kInterpSTZ)) {
				uint *pz;
				byte *ps = nullptr;
				int pp;
				uint z, r, g, b, a, fog;
				int n = (x2 >> 16) - x1;
				pp = pp1 + x1;
				r = r1;
				g = g1;
				b = b1;
				a = a1;
				if (kFogMode) {
					fog = f1;
				}
				if (kInterpZ) {
					pz = pz1 + x1;
					z = z1;
				}
				if (kStencilEnabled) {
					ps = ps1 + x1;
				}
				while (n >= 3) {
					putPixelNoTexture<kDepthWrite, kSmoothMode, kFogMode, kAlphaTestEnabled, kEnableScissor, kBlendingEnabled, kStencilEnabled, kDepthTestEnabled>
					                 (pp, pz, ps, 0, x, y, z, r, g, b, a, dzdx, drdx, dgdx, dbdx, dadx, fog, fog_r, fog_g, fog_b, dfdx);
					putPixelNoTexture<kDepthWrite, kSmoothMode, kFogMode, kAlphaTestEnabled, kEnableScissor, kBlendingEnabled, kStencilEnabled, kDepthTestEnabled>
					                 (pp, pz, ps, 1, x, y, z, r, g, b, a, dzdx, drdx, dgdx, dbdx, dadx, fog, fog_r, fog_g, fog_b, dfdx);
					putPixelNoTexture<kDepthWrite, kSmoothMode, kFogMode, kAlphaTestEnabled, kEnableScissor, kBlendingEnabled, kStencilEnabled, kDepthTestEnabled>
					                 (pp, pz, ps, 2, x, y, z, r, g, b, a, dzdx, drdx, dgdx, dbdx, dadx, fog, fog_r, fog_g, fog_b, dfdx);
					putPixelNoTexture<kDepthWrite, kSmoothMode, kFogMode, kAlphaTestEnabled, kEnableScissor, kBlendingEnabled, kStencilEnabled, kDepthTestEnabled>
					                 (pp, pz, ps, 3, x, y, z, r, g, b, a, dzdx, drdx, dgdx, dbdx, dadx, fog, fog_r, fog_g, fog_b, dfdx);
					pp += 4;
					if (kInterpZ) {
						pz += 4;
					}
					if (kStencilEnabled) {
						ps += 4;
					}
					n -= 4;
					x += 4;
				}
				while (n >= 0) {
					putPixelNoTexture<kDepthWrite, kSmoothMode, kFogMode, kAlphaTestEnabled, kEnableScissor, kBlendingEnabled, kStencilEnabled, kDepthTestEnabled>
					                 (pp, pz, ps, 0, x, y, z, r, g, b, a, dzdx, drdx, dgdx, dbdx, dadx, fog, fog_r, fog_g, fog_b, dfdx);
					pp += 1;
					if (kInterpZ) {
						pz += 1;
					}
					if (kStencilEnabled) {
						ps += 1;
					}
					n -= 1;
					x += 1;
				}
			} else if (kInterpST || kInterpSTZ) {
				uint *pz;
				byte *ps = nullptr;
				int s, t;
				uint z, r, g, b, a, fog;
				int n, pp;
				float sz, tz, fz, zinv;
				int dsdx, dtdx;

				n = (x2 >> 16) - x1;
				fz = (float)z1;
				zinv = (float)(1.0 / fz);

				pp = pp1 + x1;
				if (kFogMode) {
					fog = f1;
				}
				if (kInterpZ) {
					pz = pz1 + x1;
					z = z1;
				}
				if (kStencilEnabled) {
					ps = ps1 + x1;
				}
				sz = sz1;
				tz = tz1;
				r = r1;
				g = g1;
				b = b1;
				a = a1;
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
					for (int _a = 0; _a < NB_INTERP; _a++) {
						putPixelTexture<kDepthWrite, kInterpRGB, kSmoothMode, kFogMode, kAlphaTestEnabled, kEnableScissor, kBlendingEnabled, kStencilEnabled, kDepthTestEnabled>
						               (pp, texture, _wrapS, _wrapT, pz, ps, _a, x, y, z, t, s, r, g, b, a, dzdx, dsdx, dtdx, drdx, dgdx, dbdx, dadx, fog, fog_r, fog_g, fog_b, dfdx);
					}
					pp += NB_INTERP;
					if (kInterpZ) {
						pz += NB_INTERP;
					}
					if (kStencilEnabled) {
						ps += NB_INTERP;
					}
					sz += ndszdx;
					tz += ndtzdx;
					n -= NB_INTERP;
					x += NB_INTERP;
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
					putPixelTexture<kDepthWrite, kInterpRGB, kSmoothMode, kFogMode, kAlphaTestEnabled, kEnableScissor, kBlendingEnabled, kStencilEnabled, kDepthTestEnabled>
					               (pp, texture, _wrapS, _wrapT, pz, ps, 0, x, y, z, t, s, r, g, b, a, dzdx, dsdx, dtdx, drdx, dgdx, dbdx, dadx, fog, fog_r, fog_g, fog_b, dfdx);
					pp += 1;
					if (kInterpZ) {
						pz += 1;
					}
					if (kStencilEnabled) {
						ps += 1;
					}
					n -= 1;
					x += 1;
				}
			}

			// left edge
			error += derror;
			if (error > 0) {
				error -= 0x10000;
				x1 += dxdy_max;
				if (kInterpRGB && kFogMode) {
					f1 += dfdl_max;
				}
				if (kInterpZ) {
					z1 += dzdl_max;
				}
				if (kInterpRGB && kSmoothMode) {
					r1 += drdl_max;
					g1 += dgdl_max;
					b1 += dbdl_max;
					a1 += dadl_max;
				}
				if (kInterpST || kInterpSTZ) {
					sz1 += dszdl_max;
					tz1 += dtzdl_max;
				}
			} else {
				x1 += dxdy_min;
				if (kInterpRGB && kFogMode) {
					f1 += dfdl_min;
				}
				if (kInterpZ) {
					z1 += dzdl_min;
				}
				if (kInterpRGB && kSmoothMode) {
					r1 += drdl_min;
					g1 += dgdl_min;
					b1 += dbdl_min;
					a1 += dadl_min;
				}
				if (kInterpST || kInterpSTZ) {
					sz1 += dszdl_min;
					tz1 += dtzdl_min;
				}
			}

			// right edge
			x2 += dx2dy2;

			// screen coordinates
			if (kInterpRGB) {
				pp1 += _pbufWidth;
			}
			if (kInterpZ) {
				pz1 += _pbufWidth;
			}
			if (kStencilEnabled) {
				ps1 += _pbufWidth;
			}

			nb_lines--;
			y++;
		}
	}
}

template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode, bool kDepthWrite, bool kFogMode, bool kEnableAlphaTest, bool kEnableScissor, bool kEnableBlending, bool kStencilEnabled>
void FrameBuffer::fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_depthTestEnabled) {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, kFogMode, kEnableAlphaTest, kEnableScissor, kEnableBlending, kStencilEnabled, true>(p0, p1, p2);
	} else {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, kFogMode, kEnableAlphaTest, kEnableScissor, kEnableBlending, kStencilEnabled, false>(p0, p1, p2);
	}
}

template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode, bool kDepthWrite, bool kFogMode, bool kEnableAlphaTest, bool kEnableScissor, bool kEnableBlending>
void FrameBuffer::fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_sbuf && _stencilTestEnabled) {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, kFogMode, kEnableAlphaTest, kEnableScissor, kEnableBlending, true>(p0, p1, p2);
	} else {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, kFogMode, kEnableAlphaTest, kEnableScissor, kEnableBlending, false>(p0, p1, p2);
	}
}

template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode, bool kDepthWrite, bool kFogMode, bool kEnableAlphaTest, bool kEnableScissor>
void FrameBuffer::fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_blendingEnabled) {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, kFogMode, kEnableAlphaTest, kEnableScissor, true>(p0, p1, p2);
	} else {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, kFogMode, kEnableAlphaTest, kEnableScissor, false>(p0, p1, p2);
	}
}

template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode, bool kDepthWrite, bool kFogMode, bool kEnableAlphaTest>
void FrameBuffer::fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_enableScissor) {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, kFogMode, kEnableAlphaTest, true>(p0, p1, p2);
	} else {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, kFogMode, kEnableAlphaTest, false>(p0, p1, p2);
	}
}

template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode, bool kDepthWrite, bool kFogMode>
void FrameBuffer::fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_alphaTestEnabled) {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, kFogMode, true>(p0, p1, p2);
	} else {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, kFogMode, false>(p0, p1, p2);
	}
}

template <bool kInterpRGB, bool kInterpZ, bool kInterpST, bool kInterpSTZ, bool kSmoothMode, bool kDepthWrite>
void FrameBuffer::fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_fogEnabled) {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, true>(p0, p1, p2);
	}  else {
		fillTriangle<kInterpRGB, kInterpZ, kInterpST, kInterpSTZ, kSmoothMode, kDepthWrite, false>(p0, p1, p2);
	}
}

void FrameBuffer::fillTriangleDepthOnly(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = false;
	const bool interpST = false;
	const bool interpSTZ = false;
	const bool smoothMode = false;
	if (_depthWrite && _depthTestEnabled)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, smoothMode, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, smoothMode, false>(p0, p1, p2);
}

void FrameBuffer::fillTriangleFlat(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = true;
	const bool interpST = false;
	const bool interpSTZ = false;
	const bool smoothMode = false;
	if (_depthWrite && _depthTestEnabled)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, smoothMode, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, smoothMode, false>(p0, p1, p2);
}

// Smooth filled triangle.
void FrameBuffer::fillTriangleSmooth(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = true;
	const bool interpST = false;
	const bool interpSTZ = false;
	const bool smoothMode = true;
	if (_depthWrite && _depthTestEnabled)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, smoothMode, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, smoothMode, false>(p0, p1, p2);
}

void FrameBuffer::fillTriangleTextureMappingPerspectiveSmooth(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = true;
	const bool interpST = true;
	const bool interpSTZ = true;
	const bool smoothMode = true;
	if (_depthWrite && _depthTestEnabled)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, smoothMode, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, smoothMode, false>(p0, p1, p2);
}

void FrameBuffer::fillTriangleTextureMappingPerspectiveFlat(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = true;
	const bool interpST = false;
	const bool interpSTZ = true;
	const bool smoothMode = false;
	if (_depthWrite && _depthTestEnabled)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, smoothMode, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, smoothMode, false>(p0, p1, p2);
}

} // end of namespace TinyGL
