
#include "common/endian.h"
#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/zgl.h"

namespace TinyGL {

static const int NB_INTERP = 8;

#define SAR_RND_TO_ZERO(v,n) (v / (1 << n))

template <bool depthWrite>
FORCEINLINE static void putPixelFlat(FrameBuffer *buffer, int buf, unsigned int *pz, int _a,
                                     unsigned int &z, int color, int &dzdx) {
	if (!buffer->scissorPixel(buf + _a) && buffer->compareDepth(z, pz[_a])) {
		buffer->writePixel(buf + _a, color);
		if (depthWrite) {
			pz[_a] = z;
		}
	}
	z += dzdx;
}

template <bool depthWrite>
FORCEINLINE static void putPixelSmooth(FrameBuffer *buffer, int buf, unsigned int *pz, int _a,
                                       unsigned int &z, int &tmp, unsigned int &rgb, int &dzdx, unsigned int &drgbdx) {
	if (!buffer->scissorPixel(buf + _a) && buffer->compareDepth(z, pz[_a])) {
		tmp = rgb & 0xF81F07E0;
		buffer->writePixel(buf + _a, tmp | (tmp >> 16));
		if (depthWrite) {
			pz[_a] = z;
		}
	}
	z += dzdx;
	rgb = (rgb + drgbdx) & (~0x00200800);
}

template <bool depthWrite>
FORCEINLINE static void putPixelDepth(FrameBuffer *buffer, int buf, unsigned int *pz, int _a, unsigned int &z, int &dzdx) {
	if (!buffer->scissorPixel(buf + _a) && buffer->compareDepth(z, pz[_a])) {
		if (depthWrite) {
			pz[_a] = z;
		}
	}
	z += dzdx;
}

template <bool depthWrite, bool lightsMode, bool smoothMode>
FORCEINLINE static void putPixelTextureMappingPerspective(FrameBuffer *buffer, int buf,
                        Graphics::PixelFormat &textureFormat, Graphics::PixelBuffer &texture, unsigned int *pz, int _a,
                        unsigned int &z, unsigned int &t, unsigned int &s, int &tmp, unsigned int &rgba, unsigned int &a,
                        int &dzdx, int &dsdx, int &dtdx, unsigned int &drgbdx, unsigned int dadx) {
	if (!buffer->scissorPixel(buf + _a) && buffer->compareDepth(z, pz[_a])) {
		unsigned sss = (s & buffer->_textureSizeMask) >> ZB_POINT_ST_FRAC_BITS;
		unsigned ttt = (t & buffer->_textureSizeMask) >> ZB_POINT_ST_FRAC_BITS;
		int pixel = ttt * buffer->_textureSize + sss;
		uint8 c_a, c_r, c_g, c_b;
		uint32 *textureBuffer = (uint32 *)texture.getRawBuffer(pixel);
		uint32 col = *textureBuffer;
		c_a = (col >> textureFormat.aShift) & 0xFF;
		c_r = (col >> textureFormat.rShift) & 0xFF;
		c_g = (col >> textureFormat.gShift) & 0xFF;
		c_b = (col >> textureFormat.bShift) & 0xFF;
		unsigned int l_a = (a / 256);
		c_a = (c_a * l_a) / 256;
		if (lightsMode) {
			tmp = rgba & 0xF81F07E0;
			unsigned int light = tmp | (tmp >> 16);
			unsigned int l_r = (light & 0xF800) >> 8;
			unsigned int l_g = (light & 0x07E0) >> 3;
			unsigned int l_b = (light & 0x001F) << 3;
			c_r = (c_r * l_r) / 256;
			c_g = (c_g * l_g) / 256;
			c_b = (c_b * l_b) / 256;
		}
		buffer->writePixel(buf + _a, c_a, c_r, c_g, c_b);
		if (depthWrite) {
			pz[_a] = z;
		}
	}
	z += dzdx;
	s += dsdx;
	t += dtdx;
	if (smoothMode) {
		a += dadx;
		rgba = (rgba + drgbdx) & (~0x00200800);
	}
}

template <bool interpRGB, bool interpZ, bool interpST, bool interpSTZ, int drawLogic, bool depthWrite>
void FrameBuffer::fillTriangle(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	Graphics::PixelBuffer texture;
	Graphics::PixelFormat textureFormat;
	float fdzdx = 0, fndzdx = 0, ndszdx = 0, ndtzdx = 0;
	int _drgbdx = 0;

	ZBufferPoint *tp, *pr1 = 0, *pr2 = 0, *l1 = 0, *l2 = 0;
	float fdx1, fdx2, fdy1, fdy2, fz0, d1, d2;
	unsigned int *pz1 = NULL;
	unsigned char *pm1 = NULL;
	int part, update_left, update_right;
	int color = 0;

	int nb_lines, dx1, dy1, tmp, dx2, dy2;

	int error = 0, derror = 0;
	int x1 = 0, dxdy_min = 0, dxdy_max = 0;
	// warning: x2 is multiplied by 2^16
	int x2 = 0, dx2dy2 = 0;

	int z1 = 0, dzdx = 0, dzdy = 0, dzdl_min = 0, dzdl_max = 0;

	int r1 = 0, drdx = 0, drdy = 0, drdl_min = 0, drdl_max = 0;
	int g1 = 0, dgdx = 0, dgdy = 0, dgdl_min = 0, dgdl_max = 0;
	int b1 = 0, dbdx = 0, dbdy = 0, dbdl_min = 0, dbdl_max = 0;
	int a1 = 0, dadx = 0, dady = 0, dadl_min = 0, dadl_max = 0;

	int s1 = 0, dsdx = 0, dsdy = 0, dsdl_min = 0, dsdl_max = 0;
	int t1 = 0, dtdx = 0, dtdy = 0, dtdl_min = 0, dtdl_max = 0;

	float sz1 = 0.0, dszdx = 0, dszdy = 0, dszdl_min = 0.0, dszdl_max = 0.0;
	float tz1 = 0.0, dtzdx = 0, dtzdy = 0, dtzdl_min = 0.0, dtzdl_max = 0.0;

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

	if (interpZ) {
		d1 = (float)(p1->z - p0->z);
		d2 = (float)(p2->z - p0->z);
		dzdx = (int)(fdy2 * d1 - fdy1 * d2);
		dzdy = (int)(fdx1 * d2 - fdx2 * d1);
	}

	if (interpRGB) {
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

	if (interpST) {
		d1 = (float)(p1->s - p0->s);
		d2 = (float)(p2->s - p0->s);
		dsdx = (int)(fdy2 * d1 - fdy1 * d2);
		dsdy = (int)(fdx1 * d2 - fdx2 * d1);

		d1 = (float)(p1->t - p0->t);
		d2 = (float)(p2->t - p0->t);
		dtdx = (int)(fdy2 * d1 - fdy1 * d2);
		dtdy = (int)(fdx1 * d2 - fdx2 * d1);
	}

	if (interpSTZ) {
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

	int pp1 = xsize * p0->y;
	pz1 = zbuf + p0->y * xsize;

	switch (drawLogic) {
	case DRAW_SHADOW_MASK:
		pm1 = shadow_mask_buf + p0->y * xsize;
		break;
	case DRAW_SHADOW:
		pm1 = shadow_mask_buf + p0->y * xsize;
		color = RGB_TO_PIXEL(shadow_color_r, shadow_color_g, shadow_color_b);
		break;
	case DRAW_DEPTH_ONLY:
		break;
	case DRAW_FLAT:
		color = RGB_TO_PIXEL(p2->r, p2->g, p2->b);
		break;
	case DRAW_SMOOTH:
		_drgbdx = (SAR_RND_TO_ZERO(drdx, 6) << 22) & 0xFFC00000;
		_drgbdx |= SAR_RND_TO_ZERO(dgdx, 5) & 0x000007FF;
		_drgbdx |= (SAR_RND_TO_ZERO(dbdx, 7) << 12) & 0x001FF000;
		break;
	default:
		break;
	}

	if ((interpST || interpSTZ) && (drawLogic == DRAW_FLAT || drawLogic == DRAW_SMOOTH)) {
		texture = current_texture;
		textureFormat = texture.getFormat();
		assert(textureFormat.bytesPerPixel == 4);
		fdzdx = (float)dzdx;
		fndzdx = NB_INTERP * fdzdx;
		ndszdx = NB_INTERP * dszdx;
		ndtzdx = NB_INTERP * dtzdx;
		_drgbdx = ((drdx / (1 << 6)) << 22) & 0xFFC00000;
		_drgbdx |= (dgdx / (1 << 5)) & 0x000007FF;
		_drgbdx |= ((dbdx / (1 << 7)) << 12) & 0x001FF000;
	}

	for (part = 0; part < 2; part++) {
		if (part == 0) {
			if (fz0 > 0) {
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
			if (fz0 > 0) {
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

			if (interpZ) {
				z1 = l1->z;
				dzdl_min = (dzdy + dzdx * dxdy_min);
				dzdl_max = dzdl_min + dzdx;
			}

			if (interpRGB) {
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

			if (interpST) {
				s1 = l1->s;
				dsdl_min = (dsdy + dsdx * dxdy_min);
				dsdl_max = dsdl_min + dsdx;

				t1 = l1->t;
				dtdl_min = (dtdy + dtdx * dxdy_min);
				dtdl_max = dtdl_min + dtdx;
			}

			if (interpSTZ) {
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
			nb_lines--;
			{
				if (drawLogic == DRAW_DEPTH_ONLY ||
						(drawLogic == DRAW_FLAT && !(interpST || interpSTZ))) {
					int pp;
					int n;
					unsigned int *pz;
					unsigned int z;
					int buf = pp1 + x1;
					n = (x2 >> 16) - x1;
					pp = pp1 + x1;
					if (interpZ) {
						pz = pz1 + x1;
						z = z1;
					}
					while (n >= 3) {
						if (drawLogic == DRAW_DEPTH_ONLY) {
							putPixelDepth<depthWrite>(this, buf, pz, 0, z, dzdx);
							putPixelDepth<depthWrite>(this, buf, pz, 1, z, dzdx);
							putPixelDepth<depthWrite>(this, buf, pz, 2, z, dzdx);
							putPixelDepth<depthWrite>(this, buf, pz, 3, z, dzdx);
							buf += 4;
						}
						if (drawLogic == DRAW_FLAT) {
							putPixelFlat<depthWrite>(this, pp, pz, 0, z, color, dzdx);
							putPixelFlat<depthWrite>(this, pp, pz, 1, z, color, dzdx);
							putPixelFlat<depthWrite>(this, pp, pz, 2, z, color, dzdx);
							putPixelFlat<depthWrite>(this, pp, pz, 3, z, color, dzdx);
						}
						if (interpZ) {
							pz += 4;
						}
						pp += 4;
						n -= 4;
					}
					while (n >= 0) {
						if (drawLogic == DRAW_DEPTH_ONLY) {
							putPixelDepth<depthWrite>(this, buf, pz, 0, z, dzdx);
							buf ++;
						}
						if (drawLogic == DRAW_FLAT) {
							putPixelFlat<depthWrite>(this, pp, pz, 0, z, color, dzdx);
						}
						if (interpZ) {
							pz += 1;
						}
						pp += 1;
						n -= 1;
					}
				} else if (drawLogic == DRAW_SHADOW_MASK) {
					unsigned char *pm;
					int n;

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
				} else if (drawLogic == DRAW_SHADOW) {
					unsigned char *pm;
					int n;
					unsigned int *pz;
					unsigned int z;

					n = (x2 >> 16) - x1;

					int buf = pp1 + x1;

					pm = pm1 + x1;
					pz = pz1 + x1;
					z = z1;
					while (n >= 3) {
						for (int a = 0; a < 4; a++) {
							if (compareDepth(z, pz[a]) && pm[0]) {
								writePixel(buf + a, color);
								if (depthWrite) {
									pz[a] = z;
								}
							}
							z += dzdx;
						}
						pz += 4;
						pm += 4;
						buf += 4;
						n -= 4;
					}
					while (n >= 0) {
						if (compareDepth(z, pz[0]) && pm[0]) {
							writePixel(buf, color);
							if (depthWrite) {
								pz[0] = z;
							}
						}
						pz += 1;
						pm += 1;
						buf += 1;
						n -= 1;
					}
				} else if (drawLogic == DRAW_SMOOTH && !(interpST || interpSTZ)) {
					unsigned int *pz;
					int buf = pp1 + x1;
					unsigned int z, rgb, drgbdx;
					int n;
					n = (x2 >> 16) - x1;
					pz = pz1 + x1;
					z = z1;
					rgb = (r1 << 16) & 0xFFC00000;
					rgb |= (g1 >> 5) & 0x000007FF;
					rgb |= (b1 << 5) & 0x001FF000;
					drgbdx = _drgbdx;
					while (n >= 3) {
						putPixelSmooth<depthWrite>(this, buf, pz, 0, z, tmp, rgb, dzdx, drgbdx);
						putPixelSmooth<depthWrite>(this, buf, pz, 1, z, tmp, rgb, dzdx, drgbdx);
						putPixelSmooth<depthWrite>(this, buf, pz, 2, z, tmp, rgb, dzdx, drgbdx);
						putPixelSmooth<depthWrite>(this, buf, pz, 3, z, tmp, rgb, dzdx, drgbdx);
						pz += 4;
						buf += 4;
						n -= 4;
					}
					while (n >= 0) {
						putPixelSmooth<depthWrite>(this, buf, pz, 0, z, tmp, rgb, dzdx, drgbdx);
						buf += 1;
						pz += 1;
						n -= 1;
					}
				} else if (interpST || interpSTZ) {
					unsigned int *pz;
					unsigned int s, t, z, rgb, a, drgbdx;
					int n;
					float sz, tz, fz, zinv;
					n = (x2 >> 16) - x1;
					fz = (float)z1;
					zinv = (float)(1.0 / fz);

					int buf = pp1 + x1;

					pz = pz1 + x1;
					z = z1;
					sz = sz1;
					tz = tz1;
					rgb = (r1 << 16) & 0xFFC00000;
					rgb |= (g1 >> 5) & 0x000007FF;
					rgb |= (b1 << 5) & 0x001FF000;
					a = a1;
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
							putPixelTextureMappingPerspective<depthWrite, interpRGB, drawLogic == DRAW_SMOOTH>(this, buf, textureFormat, texture,
							                           pz, _a, z, t, s, tmp, rgb, a, dzdx, dsdx, dtdx, drgbdx, dadx);
						}
						pz += NB_INTERP;
						buf += NB_INTERP;
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
						putPixelTextureMappingPerspective<depthWrite, interpRGB, drawLogic == DRAW_SMOOTH>(this, buf, textureFormat, texture,
						                           pz, 0, z, t, s, tmp, rgb, a, dzdx, dsdx, dtdx, drgbdx, dadx);
						pz += 1;
						buf += 1;
						n -= 1;
					}
				}
			}

			// left edge
			error += derror;
			if (error > 0) {
				error -= 0x10000;
				x1 += dxdy_max;
				if (interpZ) {
					z1 += dzdl_max;
				}

				if (interpRGB) {
					r1 += drdl_max;
					g1 += dgdl_max;
					b1 += dbdl_max;
					a1 += dadl_max;
				}

				if (interpST) {
					s1 += dsdl_max;
					t1 += dtdl_max;
				}

				if (interpSTZ) {
					sz1 += dszdl_max;
					tz1 += dtzdl_max;
				}
			} else {
				x1 += dxdy_min;
				if (interpZ) {
					z1 += dzdl_min;
				}
				if (interpRGB) {
					r1 += drdl_min;
					g1 += dgdl_min;
					b1 += dbdl_min;
					a1 += dadl_min;
				}
				if (interpST) {
					s1 += dsdl_min;
					t1 += dtdl_min;
				}
				if (interpSTZ) {
					sz1 += dszdl_min;
					tz1 += dtzdl_min;
				}
			}

			// right edge
			x2 += dx2dy2;

			// screen coordinates
			pp1 += xsize;
			pz1 += xsize;

			if (drawLogic == DRAW_SHADOW || drawLogic == DRAW_SHADOW_MASK)
				pm1 = pm1 + xsize;
		}
	}
}

void FrameBuffer::fillTriangleDepthOnly(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = false;
	const bool interpST = false;
	const bool interpSTZ = false;
	if (_depthWrite)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_DEPTH_ONLY, true>(p0, p1, p2);
	else 
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_DEPTH_ONLY, false>(p0, p1, p2);
}

void FrameBuffer::fillTriangleFlat(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = false;
	const bool interpST = false;
	const bool interpSTZ = false;
	if (_depthWrite)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_FLAT, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_FLAT, false>(p0, p1, p2);
}

// Smooth filled triangle.
void FrameBuffer::fillTriangleSmooth(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = true;
	const bool interpST = false;
	const bool interpSTZ = false;
	if (_depthWrite)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_SMOOTH, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_SMOOTH, false>(p0, p1, p2);
}

void FrameBuffer::fillTriangleTextureMappingPerspectiveSmooth(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = true;
	const bool interpST = false;
	const bool interpSTZ = true;
	if (_depthWrite)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_SMOOTH, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_SMOOTH, false>(p0, p1, p2);
}

void FrameBuffer::fillTriangleTextureMappingPerspectiveFlat(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = true;
	const bool interpST = false;
	const bool interpSTZ = true;
	if (_depthWrite)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_FLAT, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_FLAT, false>(p0, p1, p2);
}

void FrameBuffer::fillTriangleFlatShadowMask(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = false;
	const bool interpST = false;
	const bool interpSTZ = false;
	if (_depthWrite)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_SHADOW_MASK, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_SHADOW_MASK, false>(p0, p1, p2);
}

void FrameBuffer::fillTriangleFlatShadow(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2) {
	const bool interpZ = true;
	const bool interpRGB = false;
	const bool interpST = false;
	const bool interpSTZ = false;
	if (_depthWrite)
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_SHADOW, true>(p0, p1, p2);
	else
		fillTriangle<interpRGB, interpZ, interpST, interpSTZ, DRAW_SHADOW, false>(p0, p1, p2);
}

} // end of namespace TinyGL
