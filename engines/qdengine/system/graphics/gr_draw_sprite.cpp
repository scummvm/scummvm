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

#include "common/textconsole.h"

#include "graphics/managed_surface.h"

#include "qdengine/qdengine.h"
#include "qdengine/system/graphics/gr_dispatcher.h"

namespace QDEngine {

void grDispatcher::putSpr_a(int x, int y, int sx, int sy, const byte *p, int mode, float scale) {
	debugC(4, kDebugGraphics, "grDispatcher::putSpr_a([%d, %d], [%d, %d], mode: %d, scale: %f)", x, y, sx, sy, mode, scale);

	int i, j, sx_dest, sy_dest;

	sx_dest = round(float(sx) * scale);
	sy_dest = round(float(sy) * scale);

	if (!sx_dest || !sy_dest) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest;
	int iy = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y0 = sy_dest,
		y1 = 0;
		iy = -1;
	}

	if (mode & GR_FLIP_HORIZONTAL) {
		x0 = sx_dest,
		x1 = 0;
		ix = -1;
	}

	sx *= 4;
	for (i = y0; i != y1; i += iy) {
		const byte *line_src = p + ((fy >> 16) * sx);

		fy += dy;
		fx = (1 << 15);

		for (j = x0; j != x1; j += ix) {
			const byte *src_data = line_src + (fx >> 16) * 4;
			uint32 a = src_data[3];

			if (a != 255 && clipCheck(x + j, y + i)) {
				if (a) {
					uint16 sc;
					getPixel(x + j, y + i, sc);
					setPixel(x + j, y + i, alpha_blend_565(make_rgb565u(src_data[2], src_data[1], src_data[0]), sc, a));
				} else
					setPixel(x + j, y + i, make_rgb565u(src_data[2], src_data[1], src_data[0]));
			}
			fx += dx;
		}
	}
}

void grDispatcher::putSpr(int x, int y, int sx, int sy, const byte *p, int mode, int spriteFormat, float scale) {
	debugC(4, kDebugGraphics, "grDispatcher::putSpr([%d, %d], [%d, %d], mode: %d, format: %d, scale: %f)", x, y, sx, sy, mode, spriteFormat, scale);

	int sx_dest = round(float(sx) * scale);
	int sy_dest = round(float(sy) * scale);

	if (!sx_dest || !sy_dest) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest;
	int iy = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y0 = sy_dest,
		y1 = 0;
		iy = -1;
	}

	if (mode & GR_FLIP_HORIZONTAL) {
		x0 = sx_dest,
		x1 = 0;
		ix = -1;
	}
	const uint16 *src = reinterpret_cast<const uint16 *>(p);

	for (int i = y0; i != y1; i += iy) {
		const uint16 *line_src = src + ((fy >> 16) * sx);

		fy += dy;
		fx = (1 << 15);

		for (int j = x0; j != x1; j += ix) {
			uint32 cl = line_src[fx >> 16];
			if (cl)
				setPixel(x + j, y + i, cl);
			fx += dx;
		}
	}
	return;
}

void grDispatcher::putSpr_a(int x, int y, int sx, int sy, const byte *p, int mode) {
	debugC(4, kDebugGraphics, "grDispatcher::putSpr_a([%d, %d], [%d, %d], mode: %d)", x, y, sx, sy, mode);

	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;
	int dx = -1;
	int dy = -1;

	if (mode & GR_FLIP_HORIZONTAL) {
		x += psx - 1;
		px = sx - px - psx;
	} else
		dx = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y += psy - 1;
		py = sy - py - psy;
	} else
		dy = 1;

	sx <<= 2;
	px <<= 2;

	const byte *data_ptr = p + py * sx;
	for (int i = 0; i < psy; i++) {
		uint16 *scr_buf = reinterpret_cast<uint16 *>(_screenBuf->getBasePtr(x, y));
		const byte *data_line = data_ptr + px;

		for (int j = 0; j < psx; j++) {
			uint32 a = data_line[3];
			if (a != 255) {
				if (a)
					*scr_buf = alpha_blend_565(make_rgb565u(data_line[2], data_line[1], data_line[0]), *scr_buf, a);
				else
					*scr_buf = make_rgb565u(data_line[2], data_line[1], data_line[0]);
			}

			scr_buf += dx;
			data_line += 4;
		}
		data_ptr += sx;
		y += dy;
	}
	return;
}

void grDispatcher::putSpr_rot(const Vect2i &pos, const Vect2i &size, const byte *data, bool has_alpha, int mode, float angle) {
	debugC(4, kDebugGraphics, "grDispatcher::putSpr_rot([%d, %d], [%d, %d], alpha: %d, mode: %d, angle: %f)", pos.x, pos.y, size.x, size.y, has_alpha, mode, angle);

	const int F_PREC = 16;

	int xc = pos.x + size.x / 2;
	int yc = pos.y + size.y / 2;

	float sn = sinf(angle);
	float cs = cosf(angle);

	int sx = round(fabs(cs) * float(size.x) + fabs(sn) * float(size.y)) + 2;
	int sy = round(fabs(sn) * float(size.x) + fabs(cs) * float(size.y)) + 2;

	int x0 = xc - sx / 2;
	int y0 = yc - sy / 2;

	int dx = 0;
	int dy = 0;

	if (!((int)(round(R2G(angle))) % 90)) {
		int angle_num = round(cycleAngle(angle) / (M_PI / 2.f));
		switch (angle_num) {
		case 1:
			dy = -2;
			break;
		case 2:
			dx = -2;
			dy = -2;
			break;
		case 3:
			dx = -2;
			break;
		}
	}

	if (!clip_rectangle(x0, y0, sx, sy))
		return;

	int sin_a = round(sn * float(1 << F_PREC));
	int cos_a = round(cs * float(1 << F_PREC));

	if (has_alpha) {
		for (int y = 0; y <= sy; y++) {
			uint16 *screen_ptr = (uint16 *)_screenBuf->getBasePtr(x0, y + y0);

			int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + ((size.x + 1 + dx) << (F_PREC - 1));
			int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + ((size.y + 1 + dy) << (F_PREC - 1));

			for (int x = 0; x <= sx; x++) {
				int xb = (xx >> F_PREC);
				int yb = (yy >> F_PREC);

				if (xb >= 0 && xb < size.x && yb >= 0 && yb < size.y) {
					if (mode & GR_FLIP_HORIZONTAL)
						xb = size.x - xb - 1;
					if (mode & GR_FLIP_VERTICAL)
						yb = size.y - yb - 1;

					const byte *data_ptr = data + size.x * 4 * yb + xb * 4;

					uint32 a = data_ptr[3];
					if (a != 255) {
						if (a)
							*screen_ptr = alpha_blend_565(make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]), *screen_ptr, a);
						else
							*screen_ptr = make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]);
					}
				}

				xx += cos_a;
				yy -= sin_a;

				screen_ptr++;
			}
		}
	} else {
		for (int y = 0; y <= sy; y++) {
			uint16 *screen_ptr = (uint16 *)_screenBuf->getBasePtr(x0, y + y0);

			int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + ((size.x + 1 + dx) << (F_PREC - 1));
			int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + ((size.y + 1 + dy) << (F_PREC - 1));

			for (int x = 0; x <= sx; x++) {
				int xb = (xx >> F_PREC);
				int yb = (yy >> F_PREC);

				if (xb >= 0 && xb < size.x && yb >= 0 && yb < size.y) {
					if (mode & GR_FLIP_HORIZONTAL)
						xb = size.x - xb - 1;
					if (mode & GR_FLIP_VERTICAL)
						yb = size.y - yb - 1;

					const byte *data_ptr = data + size.x * 3 * yb + xb * 3;
					if (data_ptr[0] || data_ptr[1] || data_ptr[2])
						*screen_ptr = make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]);
				}

				xx += cos_a;
				yy -= sin_a;

				screen_ptr++;
			}
		}
	}
}

void grDispatcher::putSpr_rot(const Vect2i &pos, const Vect2i &size, const byte *data, bool has_alpha, int mode, float angle, const Vect2f &scale) {
	debugC(4, kDebugGraphics, "grDispatcher::putSpr_rot([%d, %d], [%d, %d], alpha: %d, mode: %d, angle: %f, scale: [%f, %f])", pos.x, pos.y, size.x, size.y, has_alpha, mode, angle, scale.x, scale.y);

	const int F_PREC = 16;

	int xc = pos.x + round(float(size.x) * scale.x / 2.f);
	int yc = pos.y + round(float(size.y) * scale.y / 2.f);

	float sn = sinf(angle);
	float cs = cosf(angle);

	int sx = round(fabs(cs) * float(size.x) * scale.x + fabs(sn) * float(size.y) * scale.y) + 2;
	int sy = round(fabs(sn) * float(size.x) * scale.x + fabs(cs) * float(size.y) * scale.y) + 2;

	int x0 = xc - sx / 2;
	int y0 = yc - sy / 2;

	if (!clip_rectangle(x0, y0, sx, sy))
		return;

	int sin_a = round(sinf(angle) * float(1 << F_PREC));
	int cos_a = round(cosf(angle) * float(1 << F_PREC));

	Vect2i iscale = Vect2i(scale.x * float(1 << F_PREC), scale.y * float(1 << F_PREC));
	Vect2i scaled_size = Vect2i(iscale.x * size.x, iscale.y * size.y);

	if (has_alpha) {
		for (int y = 0; y <= sy; y++) {
			uint16 *screen_ptr = (uint16 *)_screenBuf->getBasePtr(x0, y + y0);

			int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x / 2 + (1 << (F_PREC - 1));
			int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y / 2 + (1 << (F_PREC - 1));

			for (int x = 0; x <= sx; x++) {
				int xb = xx / iscale.x;
				int yb = yy / iscale.y;

				if (xb >= 0 && xb < size.x && yb >= 0 && yb < size.y) {
					if (mode & GR_FLIP_HORIZONTAL)
						xb = size.x - xb - 1;
					if (mode & GR_FLIP_VERTICAL)
						yb = size.y - yb - 1;

					const byte *data_ptr = data + size.x * 4 * yb + xb * 4;

					uint32 a = data_ptr[3];
					if (a != 255) {
						if (a)
							*screen_ptr = alpha_blend_565(make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]), *screen_ptr, a);
						else
							*screen_ptr = make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]);
					}
				}

				xx += cos_a;
				yy -= sin_a;

				screen_ptr++;
			}
		}
	} else {
		for (int y = 0; y <= sy; y++) {
			uint16 *screen_ptr = (uint16 *)_screenBuf->getBasePtr(x0, y + y0);

			int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x / 2 + (1 << (F_PREC - 1));
			int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y / 2 + (1 << (F_PREC - 1));

			for (int x = 0; x <= sx; x++) {
				int xb = xx / iscale.x;
				int yb = yy / iscale.y;

				if (xb >= 0 && xb < size.x && yb >= 0 && yb < size.y) {
					if (mode & GR_FLIP_HORIZONTAL)
						xb = size.x - xb - 1;
					if (mode & GR_FLIP_VERTICAL)
						yb = size.y - yb - 1;

					const byte *data_ptr = data + size.x * 3 * yb + xb * 3;
					*screen_ptr = make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]);
				}

				xx += cos_a;
				yy -= sin_a;

				screen_ptr++;
			}
		}
	}
}

void grDispatcher::putSprMask_rot(const Vect2i &pos, const Vect2i &size, const byte *data, bool has_alpha, uint32 mask_color, int mask_alpha, int mode, float angle) {
	const int F_PREC = 16;

	debugC(4, kDebugGraphics, "grDispatcher::putSprMask_rot([%d, %d], [%d, %d], alpha: %d, mask: %d, mask_alpha: %d, mode: %d, angle: %f)", pos.x, pos.y, size.x, size.y, has_alpha, mask_color, mask_alpha, mode, angle);

	int xc = pos.x + size.x / 2;
	int yc = pos.y + size.y / 2;

	float sn = sinf(angle);
	float cs = cosf(angle);

	int sx = round(fabs(cs) * float(size.x) + fabs(sn) * float(size.y)) + 2;
	int sy = round(fabs(sn) * float(size.x) + fabs(cs) * float(size.y)) + 2;

	int x0 = xc - sx / 2;
	int y0 = yc - sy / 2;

	if (!clip_rectangle(x0, y0, sx, sy))
		return;

	int sin_a = round(sn * float(1 << F_PREC));
	int cos_a = round(cs * float(1 << F_PREC));

	if (has_alpha) {
		byte mr, mg, mb;
		split_rgb565u(mask_color, mr, mg, mb);

		for (int y = 0; y <= sy; y++) {
			uint16 *screen_ptr = (uint16 *)_screenBuf->getBasePtr(x0, y + y0);

			int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + (size.x + 1) * (1 << (F_PREC - 1));
			int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + (size.y + 1) * (1 << (F_PREC - 1));

			for (int x = 0; x <= sx; x++) {
				int xb = (xx >> F_PREC);
				int yb = (yy >> F_PREC);

				if (xb >= 0 && xb < size.x && yb >= 0 && yb < size.y) {
					if (mode & GR_FLIP_HORIZONTAL)
						xb = size.x - xb - 1;
					if (mode & GR_FLIP_VERTICAL)
						yb = size.y - yb - 1;

					const byte *data_ptr = data + size.x * 4 * yb + xb * 4;

					uint32 a = data_ptr[3];
					if (a != 255) {
						a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

						uint32 r = (mr * (255 - a)) >> 8;
						uint32 g = (mg * (255 - a)) >> 8;
						uint32 b = (mb * (255 - a)) >> 8;

						uint32 cl = make_rgb565u(r, g, b);

						*screen_ptr = alpha_blend_565(cl, *screen_ptr, a);
					}
				}

				xx += cos_a;
				yy -= sin_a;

				screen_ptr++;
			}
		}

	} else {
		byte mr, mg, mb;
		split_rgb565u(mask_color, mr, mg, mb);

		mr = (mr * (255 - mask_alpha)) >> 8;
		mg = (mg * (255 - mask_alpha)) >> 8;
		mb = (mb * (255 - mask_alpha)) >> 8;

		uint32 mcl = make_rgb565u(mr, mg, mb);

		for (int y = 0; y <= sy; y++) {
			uint16 *screen_ptr = (uint16 *)_screenBuf->getBasePtr(x0, y + y0);

			int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + (size.x + 1) * (1 << (F_PREC - 1));
			int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + (size.y + 1) * (1 << (F_PREC - 1));

			for (int x = 0; x <= sx; x++) {
				int xb = (xx >> F_PREC);
				int yb = (yy >> F_PREC);

				if (xb >= 0 && xb < size.x && yb >= 0 && yb < size.y) {
					if (mode & GR_FLIP_HORIZONTAL)
						xb = size.x - xb - 1;
					if (mode & GR_FLIP_VERTICAL)
						yb = size.y - yb - 1;

					const byte *data_ptr = data + size.x * 3 * yb + xb * 3;
					if (data_ptr[0] || data_ptr[1] || data_ptr[2]) {
						*screen_ptr = alpha_blend_565(mcl, *screen_ptr, mask_alpha);
					}
				}

				xx += cos_a;
				yy -= sin_a;

				screen_ptr++;
			}
		}
	}
}

void grDispatcher::putSprMask_rot(const Vect2i &pos, const Vect2i &size, const byte *data, bool has_alpha, uint32 mask_color, int mask_alpha, int mode, float angle, const Vect2f &scale) {
	const int F_PREC = 16;

	debugC(4, kDebugGraphics, "grDispatcher::putSprMask_rot([%d, %d], [%d, %d], alpha: %d, mask: %d, mask_alpha: %d, mode: %d, angle: %f, scale: [%f, %f])", pos.x, pos.y, size.x, size.y, has_alpha, mask_color, mask_alpha, mode, angle, scale.x, scale.y);

	int xc = pos.x + round(float(size.x) * scale.x / 2.f);
	int yc = pos.y + round(float(size.y) * scale.y / 2.f);

	float sn = sinf(angle);
	float cs = cosf(angle);

	int sx = round(fabs(cs) * float(size.x) * scale.x + fabs(sn) * float(size.y) * scale.y) + 2;
	int sy = round(fabs(sn) * float(size.x) * scale.x + fabs(cs) * float(size.y) * scale.y) + 2;

	int x0 = xc - sx / 2;
	int y0 = yc - sy / 2;

	if (!clip_rectangle(x0, y0, sx, sy))
		return;

	int sin_a = round(sinf(angle) * float(1 << F_PREC));
	int cos_a = round(cosf(angle) * float(1 << F_PREC));

	Vect2i iscale = Vect2i(scale.x * float(1 << F_PREC), scale.y * float(1 << F_PREC));
	Vect2i scaled_size = Vect2i(iscale.x * size.x, iscale.y * size.y);

	if (has_alpha) {
		byte mr, mg, mb;
		split_rgb565u(mask_color, mr, mg, mb);

		for (int y = 0; y <= sy; y++) {
			uint16 *screen_ptr = (uint16 *)_screenBuf->getBasePtr(x0, y + y0);

			int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x / 2 + (1 << (F_PREC - 1));
			int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y / 2 + (1 << (F_PREC - 1));

			for (int x = 0; x <= sx; x++) {
				int xb = xx / iscale.x;
				int yb = yy / iscale.y;

				if (xb >= 0 && xb < size.x && yb >= 0 && yb < size.y) {
					if (mode & GR_FLIP_HORIZONTAL)
						xb = size.x - xb - 1;
					if (mode & GR_FLIP_VERTICAL)
						yb = size.y - yb - 1;

					const byte *data_ptr = data + size.x * 4 * yb + xb * 4;

					uint32 a = data_ptr[3];
					if (a != 255) {
						a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

						uint32 r = (mr * (255 - a)) >> 8;
						uint32 g = (mg * (255 - a)) >> 8;
						uint32 b = (mb * (255 - a)) >> 8;

						uint32 cl = make_rgb565u(r, g, b);

						*screen_ptr = alpha_blend_565(cl, *screen_ptr, a);
					}
				}

				xx += cos_a;
				yy -= sin_a;

				screen_ptr++;
			}
		}
	} else {
		byte mr, mg, mb;
		split_rgb565u(mask_color, mr, mg, mb);

		mr = (mr * (255 - mask_alpha)) >> 8;
		mg = (mg * (255 - mask_alpha)) >> 8;
		mb = (mb * (255 - mask_alpha)) >> 8;

		for (int y = 0; y <= sy; y++) {
			uint16 *screen_ptr = (uint16 *)_screenBuf->getBasePtr(x0, y + y0);

			int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x / 2 + (1 << (F_PREC - 1));
			int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y / 2 + (1 << (F_PREC - 1));

			for (int x = 0; x <= sx; x++) {
				int xb = xx / iscale.x;
				int yb = yy / iscale.y;

				if (xb >= 0 && xb < size.x && yb >= 0 && yb < size.y) {
					if (mode & GR_FLIP_HORIZONTAL)
						xb = size.x - xb - 1;
					if (mode & GR_FLIP_VERTICAL)
						yb = size.y - yb - 1;

					const byte *data_ptr = data + size.x * 3 * yb + xb * 3;
					*screen_ptr = make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]);
				}

				xx += cos_a;
				yy -= sin_a;

				screen_ptr++;
			}
		}
	}
}

void grDispatcher::putSpr(int x, int y, int sx, int sy, const byte *p, int mode, int spriteFormat) {
	debugC(4, kDebugGraphics, "grDispatcher::putSpr([%d, %d], [%d, %d], mode: %d, format: %d)", x, y, sx, sy, mode, spriteFormat);

	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;
	int dx = -1;
	int dy = -1;

	if (mode & GR_FLIP_HORIZONTAL) {
		x += psx - 1;
		px = sx - px - psx;
	} else
		dx = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y += psy - 1;
		py = sy - py - psy;
	} else
		dy = 1;

	if (spriteFormat == GR_RGB888) {
		sx *= 3;
		px *= 3;

		const byte *data_ptr = p + py * sx;

		for (int i = 0; i < psy; i++) {
			uint16 *scr_buf = reinterpret_cast<uint16 *>(_screenBuf->getBasePtr(x, y));
			const byte *data_line = data_ptr + px;

			for (int j = 0; j < psx; j++) {
				if (data_line[0] || data_line[1] || data_line[2])
					*scr_buf = make_rgb565u(data_line[2], data_line[1], data_line[0]);
				scr_buf += dx;
				data_line += 3;
			}

			data_ptr += sx;
			y += dy;
		}
	} else if (spriteFormat == GR_RGB565) {
		sx *= 2;
		px *= 2;

		const byte *data_ptr = p + py * sx;

		for (int i = 0; i < psy; i++) {
			uint16 *scr_buf = reinterpret_cast<uint16 *>(_screenBuf->getBasePtr(x, y));
			const byte *data_line = data_ptr + px;

			for (int j = 0; j < psx; j++) {
				if (*data_line)
					*scr_buf = *(const uint16 *)data_line;
				scr_buf += dx;
				data_line += 2;
			}

			data_ptr += sx;
			y += dy;
		}
	}

}

void grDispatcher::drawSprContour_a(int x, int y, int sx, int sy, const byte *p, int contour_color, int mode) {
	debugC(4, kDebugGraphics, "grDispatcher::drawSprContour_a([%d, %d], [%d, %d], contour: %d, mode: %d)", x, y, sx, sy, contour_color, mode);

	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;
	int dpx, dpy;
	if (mode & GR_FLIP_HORIZONTAL) {
		px = sx - px - 1;
		dpx = -1;
	} else
		dpx = 1;

	if (mode & GR_FLIP_VERTICAL) {
		py = sy - py - 1;
		dpy = -sx;
	} else
		dpy = sx;

	sx <<= 1;
	dpy <<= 1;

	const uint16 *pic_buf = reinterpret_cast<const uint16 *>(p) + py * sx;
	for (int i = 0; i < psy; i++) {
		int jj = px;
		int empty_pixel = 1;
		for (int j = 0; j < psx; j++) {
			if (pic_buf[jj * 2 + 1] < 200) {
				if (empty_pixel)
					setPixelFast(x + j, y + i, contour_color);
				empty_pixel = 0;
			} else {
				if (!empty_pixel)
					setPixelFast(x + j - 1, y + i, contour_color);
				empty_pixel = 1;
			}
			jj += dpx;
		}
		if (!empty_pixel)
			setPixelFast(x + psx - 1, y + i, contour_color);
		pic_buf += dpy;
	}
	int jj = px;
	for (int j = 0; j < psx; j++) {
		int empty_pixel = 1;
		pic_buf = reinterpret_cast<const uint16 *>(p) + py * sx;
		for (int i = 0; i < psy; i++) {
			if (pic_buf[jj * 2 + 1] < 200) {
				if (empty_pixel)
					setPixelFast(x + j, y + i, contour_color);
				empty_pixel = 0;
			} else {
				if (!empty_pixel)
					setPixelFast(x + j, y + i - 1, contour_color);
				empty_pixel = 1;
			}
			pic_buf += dpy;
		}
		if (!empty_pixel)
			setPixelFast(x + j, y + psy - 1, contour_color);
		jj += dpx;
	}
	return;
}

void grDispatcher::drawSprContour(int x, int y, int sx, int sy, const byte *p, int contour_color, int mode) {
	debugC(4, kDebugGraphics, "grDispatcher::drawSprContour([%d, %d], [%d, %d], contour: %d, mode: %d)", x, y, sx, sy, contour_color, mode);

	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;

	int dpx, dpy;
	if (mode & GR_FLIP_HORIZONTAL) {
		px = sx - px - 1;
		dpx = -1;
	} else
		dpx = 1;

	if (mode & GR_FLIP_VERTICAL) {
		py = sy - py - 1;
		dpy = -sx;
	} else
		dpy = sx;

	const uint16 *pic_buf = reinterpret_cast<const uint16 *>(p) + py * sx;
	for (int i = 0; i < psy; i++) {
		int jj = px;
		int empty_pixel = 1;
		for (int j = 0; j < psx; j++) {
			uint32 cl = pic_buf[jj];
			if (cl) {
				if (empty_pixel)
					setPixelFast(x + j, y + i, contour_color);
				empty_pixel = 0;
			} else {
				if (!empty_pixel)
					setPixelFast(x + j - 1, y + i, contour_color);
				empty_pixel = 1;
			}
			jj += dpx;
		}
		if (!empty_pixel)
			setPixelFast(x + psx - 1, y + i, contour_color);
		pic_buf += dpy;
	}
	int jj = px;
	for (int j = 0; j < psx; j++) {
		int empty_pixel = 1;
		pic_buf = reinterpret_cast<const uint16 *>(p) + py * sx;
		for (int i = 0; i < psy; i++) {
			uint32 cl = pic_buf[jj];
			if (cl) {
				if (empty_pixel)
					setPixelFast(x + j, y + i, contour_color);
				empty_pixel = 0;
			} else {
				if (!empty_pixel)
					setPixelFast(x + j, y + i - 1, contour_color);
				empty_pixel = 1;
			}
			pic_buf += dpy;
		}
		if (!empty_pixel)
			setPixelFast(x + j, y + psy - 1, contour_color);
		jj += dpx;
	}
	return;
}

void grDispatcher::drawSprContour(int x, int y, int sx, int sy, const byte *p, int contour_color, int mode, float scale) {
	debugC(4, kDebugGraphics, "grDispatcher::drawSprContour([%d, %d], [%d, %d], contour: %d, mode: %d, scale: %f)", x, y, sx, sy, contour_color, mode, scale);

	int sx_dest = round(float(sx) * scale);
	int sy_dest = round(float(sy) * scale);

	if (!sx_dest || !sy_dest) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest;
	int iy = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y0 = sy_dest,
		y1 = 0;
		iy = -1;
	}

	if (mode & GR_FLIP_HORIZONTAL) {
		x0 = sx_dest,
		x1 = 0;
		ix = -1;
	}
	const uint16 *src = reinterpret_cast<const uint16 *>(p);
	for (int i = y0; i != y1; i += iy) {
		const uint16 *line_src = src + ((fy >> 16) * sx);

		fy += dy;
		fx = (1 << 15);

		int empty_pixel = 1;

		for (int j = x0; j != x1; j += ix) {
			if (line_src[fx >> 16]) {
				if (empty_pixel)
					setPixel(x + j, y + i, contour_color);
				empty_pixel = 0;
			} else {
				if (!empty_pixel)
					setPixel(x + j - 1, y + i, contour_color);
				empty_pixel = 1;
			}
			fx += dx;
		}
		if (!empty_pixel)
			setPixel(x + x1 - 1, y + i, contour_color);
	}
	fx = (1 << 15);
	for (int j = x0; j != x1; j += ix) {
		fy = (1 << 15);
		int empty_pixel = 1;

		for (int i = y0; i != y1; i += iy) {
			const uint16 *line_src = src + ((fy >> 16) * sx);

			if (line_src[fx >> 16]) {
				if (empty_pixel)
					setPixel(x + j, y + i, contour_color);
				empty_pixel = 0;
			} else {
				if (!empty_pixel)
					setPixel(x + j, y + i - 1, contour_color);
				empty_pixel = 1;
			}
			fy += dy;
		}
		if (!empty_pixel)
			setPixel(x + j, y + y1 - 1, contour_color);

		fx += dx;
	}
	return;
}

void grDispatcher::drawSprContour_a(int x, int y, int sx, int sy, const byte *p, int contour_color, int mode, float scale) {
	debugC(4, kDebugGraphics, "grDispatcher::drawSprContour_a([%d, %d], [%d, %d], contour: %d, mode: %d, scale: %f)", x, y, sx, sy, contour_color, mode, scale);

	int sx_dest = round(float(sx) * scale);
	int sy_dest = round(float(sy) * scale);

	if (!sx_dest || !sy_dest) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest;
	int iy = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y0 = sy_dest,
		y1 = 0;
		iy = -1;
	}

	if (mode & GR_FLIP_HORIZONTAL) {
		x0 = sx_dest,
		x1 = 0;
		ix = -1;
	}
	sx <<= 1;
	const uint16 *src = reinterpret_cast<const uint16 *>(p);

	for (int i = y0; i != y1; i += iy) {
		const uint16 *line_src = src + ((fy >> 16) * sx);

		fy += dy;
		fx = (1 << 15);

		int empty_pixel = 1;

		for (int j = x0; j != x1; j += ix) {
			if (line_src[((fx >> 16) << 1) + 1] < 200) {
				if (empty_pixel)
					setPixel(x + j, y + i, contour_color);
				empty_pixel = 0;
			} else {
				if (!empty_pixel)
					setPixel(x + j - 1, y + i, contour_color);
				empty_pixel = 1;
			}
			fx += dx;
		}
		if (!empty_pixel)
			setPixel(x + x1 - 1, y + i, contour_color);
	}

	fx = (1 << 15);
	for (int j = x0; j != x1; j += ix) {
		fy = (1 << 15);
		int empty_pixel = 1;

		for (int i = y0; i != y1; i += iy) {
			const uint16 *line_src = src + ((fy >> 16) * sx);

			if (line_src[((fx >> 16) << 1) + 1] < 200) {
				if (empty_pixel)
					setPixel(x + j, y + i, contour_color);
				empty_pixel = 0;
			} else {
				if (!empty_pixel)
					setPixel(x + j, y + i - 1, contour_color);
				empty_pixel = 1;
			}
			fy += dy;
		}
		if (!empty_pixel)
			setPixel(x + j, y + y1 - 1, contour_color);

		fx += dx;
	}

	return;
}

void grDispatcher::putChar(int x, int y, uint32 color, int font_sx, int font_sy, const byte *font_alpha, const grScreenRegion &chr_region) {
	int px = chr_region.x();
	int py = chr_region.y();

	int psx = chr_region.size_x();
	int psy = chr_region.size_y();

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;

	const byte *alpha_buf = font_alpha + px + py * font_sx;

	color = make_rgb(color);

	for (int i = 0; i < psy; i++, y++) {
		uint16 *scr_buf = reinterpret_cast<uint16 *>(_screenBuf->getBasePtr(x, y));
		for (int j = 0; j < psx; j++) {
			uint32 a = alpha_buf[j];
			uint32 a1 = 255 - a;
			if (a) {
				if (a != 255) {
					uint32 scr_col = *scr_buf;
					*scr_buf++ = (((((color & mask_565_r) * a) >> 8) & mask_565_r) |
									((((color & mask_565_g) * a) >> 8) & mask_565_g) |
									((((color & mask_565_b) * a) >> 8) & mask_565_b)) +
									(((((scr_col & mask_565_r) * a1) >> 8) & mask_565_r) |
									((((scr_col & mask_565_g) * a1) >> 8) & mask_565_g) |
									((((scr_col & mask_565_b) * a1) >> 8) & mask_565_b));
				} else
					*scr_buf++ = color;
			} else scr_buf++;
		}
		alpha_buf += font_sx;
	}
	return;
}

void grDispatcher::putSprMask(int x, int y, int sx, int sy, const byte *p, uint32 mask_color, int mask_alpha, int mode) {
	debugC(4, kDebugGraphics, "grDispatcher::putSprMask([%d, %d], [%d, %d], mask: %d, alpha: %d, mode: %d)", x, y, sx, sy, mask_color, mask_alpha, mode);

	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;
	int dx = -1;
	int dy = -1;

	if (mode & GR_FLIP_HORIZONTAL) {
		x += psx - 1;
		px = sx - px - psx;
	} else
		dx = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y += psy - 1;
		py = sy - py - psy;
	} else
		dy = 1;

	px *= 3;
	sx *= 3;

	const byte *data_ptr = p + py * sx + px;

	byte mr, mg, mb;
	split_rgb565u(mask_color, mr, mg, mb);

	mr = (mr * (255 - mask_alpha)) >> 8;
	mg = (mg * (255 - mask_alpha)) >> 8;
	mb = (mb * (255 - mask_alpha)) >> 8;

	uint32 mcl = make_rgb565u(mr, mg, mb);

	warning("STUB: grDispatcher::putSprMask");
	for (int i = 0; i < psy; i++) {
		uint16 *scr_buf = (uint16 *)(_screenBuf->getBasePtr(x, y));
		const byte *data_line = data_ptr;

		for (int j = 0; j < psx; j++) {
			if (data_line[0] || data_line[1] || data_line[2])
				*scr_buf = alpha_blend_565(mcl, *scr_buf, mask_alpha);
			scr_buf += dx;
			data_line += 3;
		}

		data_ptr += sx;
		y += dy;
	}
}

void grDispatcher::putSprMask(int x, int y, int sx, int sy, const byte *p, uint32 mask_color, int mask_alpha, int mode, float scale) {
	debugC(4, kDebugGraphics, "grDispatcher::putSprMask([%d, %d], [%d, %d], mask: %d, alpha: %d, mode: %d, scale: %f)", x, y, sx, sy, mask_color, mask_alpha, mode, scale);

	int sx_dest = round(float(sx) * scale);
	int sy_dest = round(float(sy) * scale);

	if (!sx_dest || !sy_dest) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest;
	int iy = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y0 = sy_dest,
		y1 = 0;
		iy = -1;
	}

	if (mode & GR_FLIP_HORIZONTAL) {
		x0 = sx_dest,
		x1 = 0;
		ix = -1;
	}

	byte mr, mg, mb;
	split_rgb565u(mask_color, mr, mg, mb);

	mr = (mr * (255 - mask_alpha)) >> 8;
	mg = (mg * (255 - mask_alpha)) >> 8;
	mb = (mb * (255 - mask_alpha)) >> 8;

	uint32 mcl = make_rgb565u(mr, mg, mb);

	sx *= 3;

	for (int i = y0; i != y1; i += iy) {
		const byte *line_src = p + ((fy >> 16) * sx);

		fy += dy;
		fx = (1 << 15);

		for (int j = x0; j != x1; j += ix) {
			const byte *src_data = line_src + (fx >> 16) * 3;
			if (src_data[0] || src_data[1] || src_data[2]) {
				uint16 scl;
				getPixel(x + j, y + i, scl);
				setPixel(x + j, y + i, alpha_blend_565(mcl, scl, mask_alpha));
			}
			fx += dx;
		}
	}
}

void grDispatcher::putSprMask_a(int x, int y, int sx, int sy, const byte *p, uint32 mask_color, int mask_alpha, int mode) {
	debugC(4, kDebugGraphics, "grDispatcher::putSprMask_a([%d, %d], [%d, %d], mask: %d, alpha: %d, mode: %d)", x, y, sx, sy, mask_color, mask_alpha, mode);

	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;
	int dx = -1;
	int dy = -1;

	if (mode & GR_FLIP_HORIZONTAL) {
		x += psx - 1;
		px = sx - px - psx;
	} else
		dx = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y += psy - 1;
		py = sy - py - psy;
	} else
		dy = 1;

	sx <<= 2;
	px <<= 2;

	const byte *data_ptr = p + py * sx + px;

	byte mr, mg, mb;
	split_rgb565u(mask_color, mr, mg, mb);

	for (int i = 0; i < psy; i++) {
		uint16 *scr_buf = (uint16 *)(_screenBuf->getBasePtr(x, y));
		const byte *data_line = data_ptr;

		for (int j = 0; j < psx; j++) {
			uint32 a = data_line[3];

			if (a != 255) {
				a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

				uint32 r = (mr * (255 - a)) >> 8;
				uint32 g = (mg * (255 - a)) >> 8;
				uint32 b = (mb * (255 - a)) >> 8;

				uint32 cl = make_rgb565u(r, g, b);

				*scr_buf = alpha_blend_565(cl, *scr_buf, a);
			}
			scr_buf += dx;
			data_line += 4;
		}
		data_ptr += sx;
		y += dy;
	}
}

void grDispatcher::putSprMask_a(int x, int y, int sx, int sy, const byte *p, uint32 mask_color, int mask_alpha, int mode, float scale) {
	debugC(4, kDebugGraphics, "grDispatcher::putSprMask_a([%d, %d], [%d, %d], mask: %d, alpha: %d, mode: %d, scale: %f)", x, y, sx, sy, mask_color, mask_alpha, mode, scale);

	int i, j, sx_dest, sy_dest;

	sx_dest = round(float(sx) * scale);
	sy_dest = round(float(sy) * scale);

	if (!sx_dest || !sy_dest) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest;
	int iy = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y0 = sy_dest,
		y1 = 0;
		iy = -1;
	}

	if (mode & GR_FLIP_HORIZONTAL) {
		x0 = sx_dest,
		x1 = 0;
		ix = -1;
	}

	sx <<= 2;
	byte mr, mg, mb;
	split_rgb565u(mask_color, mr, mg, mb);

	for (i = y0; i != y1; i += iy) {
		const byte *line_src = p + ((fy >> 16) * sx);

		fy += dy;
		fx = (1 << 15);

		for (j = x0; j != x1; j += ix) {
			const byte *src_data = line_src + (fx >> 16) * 4;
			uint32 a = src_data[3];

			if (a != 255 && clipCheck(x + j, y + i)) {
				uint16 sc;
				getPixel(x + j, y + i, sc);

				a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

				uint32 r = (mr * (255 - a)) >> 8;
				uint32 g = (mg * (255 - a)) >> 8;
				uint32 b = (mb * (255 - a)) >> 8;

				uint32 cl = make_rgb565u(r, g, b);

				setPixel(x + j, y + i, alpha_blend_565(cl, sc, a));
			}
			fx += dx;
		}
	}
}

} // namespace QDEngine
