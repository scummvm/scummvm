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

#include "qdengine/qd_fwd.h"
#include "qdengine/system/graphics/gr_dispatcher.h"


namespace QDEngine {

#ifdef _GR_ENABLE_ZBUFFER
void grDispatcher::putSpr_a_z(int x, int y, int z, int sx, int sy, const byte *p, int mode, float scale) {
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

	if (bytes_per_pixel() == 2) {
		const uint16 *src = reinterpret_cast<const uint16 *>(p);
		sx <<= 1;
		if (pixel_format_ == GR_ARGB1555) {
			for (i = y0; i != y1; i += iy) {
				const uint16 *line_src = src + ((fy >> 16) * sx);

				fy += dy;
				fx = (1 << 15);

				for (j = x0; j != x1; j += ix) {
					uint32 a = line_src[((fx >> 16) << 1) + 1];
					if (a != 255 && ClipCheck(x + j, y + i)) {
						uint32 sc;
						getPixel(x + j, y + i, sc);
						setPixel(x + j, y + i, alpha_blend_555(line_src[(fx >> 16) << 1], sc, a));
						put_z(x + j, y + i, z);
					}
					fx += dx;
				}
			}
		} else {
			for (i = y0; i != y1; i += iy) {
				const uint16 *line_src = src + ((fy >> 16) * sx);

				fy += dy;
				fx = (1 << 15);

				for (j = x0; j != x1; j += ix) {
					uint32 a = line_src[((fx >> 16) << 1) + 1];
					if (a != 255 && ClipCheck(x + j, y + i)) {
						uint32 sc;
						getPixel(x + j, y + i, sc);
						setPixel(x + j, y + i, alpha_blend_565(line_src[(fx >> 16) << 1], sc, a));
						put_z(x + j, y + i, z);
					}
					fx += dx;
				}
			}
		}
		return;
	}
	if (bytes_per_pixel() == 3 || bytes_per_pixel() == 4) {
		int sx3 = sx * 4;

		for (i = y0; i != y1; i += iy) {
			const byte *line_src = p + ((fy >> 16) * sx3);

			fy += dy;
			fx = (1 << 15);

			for (j = x0; j != x1; j += ix) {
				int idx = (fx >> 16) << 2;
				uint32 a = line_src[idx + 3];
				if (a != 255 && ClipCheck(x + j, y + i)) {
					uint32 sr, sg, sb;
					getPixel(x + j, y + i, sr, sg, sb);

					uint32 r = line_src[idx + 2] + ((a * sr) >> 8);
					uint32 g = line_src[idx + 1] + ((a * sg) >> 8);
					uint32 b = line_src[idx + 0] + ((a * sb) >> 8);

					setPixel(x + j, y + i, r, g, b);
					put_z(x + j, y + i, z);
				}

				fx += dx;
			}
		}
		return;
	}
}

void grDispatcher::putSpr_z(int x, int y, int z, int sx, int sy, const byte *p, int mode, float scale) {
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

	if (bytes_per_pixel() == 2) {
		const uint16 *src = reinterpret_cast<const uint16 *>(p);

		for (int i = y0; i != y1; i += iy) {
			const uint16 *line_src = src + ((fy >> 16) * sx);

			fy += dy;
			fx = (1 << 15);

			for (int j = x0; j != x1; j += ix) {
				uint32 cl = line_src[fx >> 16];
				if (cl) {
					setPixel(x + j, y + i, cl);
					put_z(x + j, y + i, z);
				}
				fx += dx;
			}
		}
		return;
	}

	if (bytes_per_pixel() == 3) {
		int sx3 = sx * 3;
		for (int i = y0; i != y1; i += iy) {
			const byte *line_src = p + ((fy >> 16) * sx3);

			fy += dy;
			fx = (1 << 15);

			for (int j = x0; j != x1; j += ix) {
				int idx = (fx >> 16) * 3;

				uint32 r = line_src[idx + 2];
				uint32 g = line_src[idx + 1];
				uint32 b = line_src[idx + 0];

				if (r || g || b) {
					setPixel(x + j, y + i, r, g, b);
					put_z(x + j, y + i, z);
				}

				fx += dx;
			}
		}
		return;
	}

	if (bytes_per_pixel() == 4) {
		const uint32 *src = reinterpret_cast<const uint32 *>(p);

		for (int i = y0; i != y1; i += iy) {
			const uint32 *line_src = src + ((fy >> 16) * sx);

			fy += dy;
			fx = (1 << 15);

			for (int j = x0; j != x1; j += ix) {
				uint32 cl = line_src[fx >> 16];
				if (cl) {
					setPixel(x + j, y + i, cl);
					put_z(x + j, y + i, z);
				}
				fx += dx;
			}
		}
		return;
	}
}

void grDispatcher::putSpr_a_z(int x, int y, int z, int sx, int sy, const byte *p, int mode) {
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;

	if (bytes_per_pixel() == 4) {
		int dx = -4;
		int zdx = -1;
		int dy = -1;

		int x4 = x * 4;

		if (mode & GR_FLIP_HORIZONTAL) {
			x4 += (psx - 1) * 4;
			x += psx - 1;
			px = sx - px - psx;
		} else {
			dx = 4;
			zdx = 1;
		}

		if (mode & GR_FLIP_VERTICAL) {
			y += psy - 1;
			py = sy - py - psy;
		} else
			dy = 1;

		int px3 = px * 4;
		int sx3 = sx * 4;

		const byte *data_ptr = p + py * sx3;

		for (int i = 0; i < psy; i++) {
			byte *scr_buf = reinterpret_cast<byte *>(screenBuf + yTable[y] + x4);
			zbuf_t *zbuf = zbuffer_ + y * SizeX + x;
			const byte *data_line = data_ptr + px3;

			for (int j = 0; j < psx; j++) {
				uint32 a = data_line[3];
				if (a != 255) {
					if (a) {
						scr_buf[0] = data_line[0] + ((a * scr_buf[0]) >> 8);
						scr_buf[1] = data_line[1] + ((a * scr_buf[1]) >> 8);
						scr_buf[2] = data_line[2] + ((a * scr_buf[2]) >> 8);
					} else {
						scr_buf[0] = data_line[0];
						scr_buf[1] = data_line[1];
						scr_buf[2] = data_line[2];
					}
					*zbuf = z;
				}
				scr_buf += dx;
				zbuf += zdx;
				data_line += 4;
			}
			data_ptr += sx3;
			y += dy;
		}
		return;
	}
	if (bytes_per_pixel() == 3) {
		int dx = -3;
		int zdx = -1;
		int dy = -1;

		int x3 = x * 3;

		if (mode & GR_FLIP_HORIZONTAL) {
			x3 += (psx - 1) * 3;
			x += psx - 1;
			px = sx - px - psx;
		} else {
			dx = 3;
			zdx = 1;
		}

		if (mode & GR_FLIP_VERTICAL) {
			y += psy - 1;
			py = sy - py - psy;
		} else
			dy = 1;

		int px3 = px * 4;
		int sx3 = sx * 4;

		const byte *data_ptr = p + py * sx3;

		for (int i = 0; i < psy; i++) {
			byte *scr_buf = reinterpret_cast<byte *>(screenBuf + yTable[y] + x3);
			zbuf_t *zbuf = zbuffer_ + y * SizeX + x;
			const byte *data_line = data_ptr + px3;

			for (int j = 0; j < psx; j++) {
				uint32 a = data_line[3];
				if (a != 255) {
					if (a) {
						scr_buf[0] = data_line[0] + ((a * scr_buf[0]) >> 8);
						scr_buf[1] = data_line[1] + ((a * scr_buf[1]) >> 8);
						scr_buf[2] = data_line[2] + ((a * scr_buf[2]) >> 8);
					} else {
						scr_buf[0] = data_line[0];
						scr_buf[1] = data_line[1];
						scr_buf[2] = data_line[2];
					}
					*zbuf = z;
				}
				scr_buf += dx;
				zbuf += zdx;
				data_line += 4;
			}
			data_ptr += sx3;
			y += dy;
		}
		return;
	}
	if (bytes_per_pixel() == 2) {
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

		x <<= 1;
		sx <<= 1;
		px <<= 1;

		const uint16 *data_ptr = reinterpret_cast<const uint16 *>(p) + py * sx;

		if (pixel_format_ == GR_RGB565) {
			for (int i = 0; i < psy; i++) {
				uint16 *scr_buf = reinterpret_cast<uint16 *>(screenBuf + yTable[y] + x);
				zbuf_t *zbuf = zbuffer_ + y * SizeX + (x >> 1);
				const uint16 *data_line = data_ptr + px;

				for (int j = 0; j < psx; j++) {
					uint32 a = data_line[1];
					*scr_buf = alpha_blend_565(*data_line, *scr_buf, a);
					if (a != 255) *zbuf = z;
					scr_buf += dx;
					zbuf += dx;
					data_line += 2;
				}
				data_ptr += sx;
				y += dy;
			}
		} else {
			for (int i = 0; i < psy; i++) {
				uint16 *scr_buf = reinterpret_cast<uint16 *>(screenBuf + yTable[y] + x);
				zbuf_t *zbuf = zbuffer_ + y * SizeX + (x >> 1);
				const uint16 *data_line = data_ptr + px;

				for (int j = 0; j < psx; j++) {
					uint32 a = data_line[1];
					*scr_buf = alpha_blend_555(*data_line, *scr_buf, a);
					if (a != 255) *zbuf = z;
					scr_buf += dx;
					zbuf += dx;
					data_line += 2;
				}
				data_ptr += sx;
				y += dy;
			}
		}
		return;
	}
}

void grDispatcher::putSpr_z(int x, int y, int z, int sx, int sy, const byte *p, int mode) {
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;

	if (bytes_per_pixel() == 4) {
		int dx = -4;
		int zdx = -1;
		int dy = -1;

		int x4 = x * 4;

		if (mode & GR_FLIP_HORIZONTAL) {
			x4 += psx * 4 - 4;
			x += psx - 1;
			px = sx - px - psx;
		} else {
			dx = 4;
			zdx = 1;
		}

		if (mode & GR_FLIP_VERTICAL) {
			y += psy - 1;
			py = sy - py - psy;
		} else
			dy = 1;

		int px3 = px * 3;
		int sx3 = sx * 3;

		const byte *data_ptr = p + py * sx3;

		for (int i = 0; i < psy; i++) {
			byte *scr_buf = reinterpret_cast<byte *>(screenBuf + yTable[y] + x4);
			zbuf_t *zbuf = zbuffer_ + y * SizeX + x;
			const byte *data_line = data_ptr + px3;

			for (int j = 0; j < psx; j++) {
				if (data_line[0] || data_line[1] || data_line[2]) {
					scr_buf[0] = data_line[0];
					scr_buf[1] = data_line[1];
					scr_buf[2] = data_line[2];
					*zbuf = z;
				}
				scr_buf += dx;
				zbuf += zdx;
				data_line += 3;
			}

			data_ptr += sx3;
			y += dy;
		}
		return;
	}
	if (bytes_per_pixel() == 3) {
		int dx = -3;
		int zdx = -1;
		int dy = -1;

		int x3 = x * 3;

		if (mode & GR_FLIP_HORIZONTAL) {
			x3 += psx * 3 - 3;
			x += psx - 1;
			px = sx - px - psx;
		} else {
			dx = 3;
			zdx = 1;
		}

		if (mode & GR_FLIP_VERTICAL) {
			y += psy - 1;
			py = sy - py - psy;
		} else
			dy = 1;

		int px3 = px * 3;
		int sx3 = sx * 3;

		const byte *data_ptr = p + py * sx3;

		for (int i = 0; i < psy; i++) {
			byte *scr_buf = reinterpret_cast<byte *>(screenBuf + yTable[y] + x3);
			zbuf_t *zbuf = zbuffer_ + y * SizeX + x;
			const byte *data_line = data_ptr + px3;

			for (int j = 0; j < psx; j++) {
				if (data_line[0] || data_line[1] || data_line[2]) {
					scr_buf[0] = data_line[0];
					scr_buf[1] = data_line[1];
					scr_buf[2] = data_line[2];
					*zbuf = z;
				}
				scr_buf += dx;
				zbuf += zdx;
				data_line += 3;
			}

			data_ptr += sx3;
			y += dy;
		}
		return;
	}
	if (bytes_per_pixel() == 2) {
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

		x <<= 1;

		const uint16 *data_ptr = reinterpret_cast<const uint16 *>(p) + py * sx;

		for (int i = 0; i < psy; i++) {
			uint16 *scr_buf = reinterpret_cast<uint16 *>(screenBuf + yTable[y] + x);
			zbuf_t *zbuf = zbuffer_ + y * SizeX + x;
			const uint16 *data_line = data_ptr + px;

			for (int j = 0; j < psx; j++) {
				if (*data_line) {
					*scr_buf = *data_line;
					*zbuf = z;
				}
				zbuf += dx;
				scr_buf += dx;
				data_line++;
			}

			data_ptr += sx;
			y += dy;
		}
		return;
	}
}
#endif

} // namespace QDEngine
