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
#include "qdengine/qd_fwd.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/rle_compress.h"


namespace QDEngine {

void grDispatcher::putSpr_rle(int x, int y, int sx, int sy, const class RLEBuffer *p, int mode, bool alpha_flag) {
	debugC(2, kDebugGraphics, "grDispatcher::putSpr_rle(%d, %d, %d, %d)", x, y, sx, sy);

	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;
	int dx = -1;
	int dy = -1;

	if (mode & GR_FLIP_HORIZONTAL) {
		x += (psx - 1);
		px = sx - px - psx;
	} else
		dx = 1;

	psx += px;

	if (mode & GR_FLIP_VERTICAL) {
		y += psy - 1;
		py = sy - py - psy;
	} else
		dy = 1;

	for (int i = 0; i < psy; i++) {
		uint16 *scr_buf = reinterpret_cast<uint16 *>(_screenBuf->getBasePtr(x, y));

		const char *rle_header = p->header_ptr(py + i);
		const uint32 *rle_data = p->data_ptr(py + i);

		int j = 0;
		char count = 0;
		while (j < px) {
			count = *rle_header++;
			if (count > 0) {
				if (count + j <= px) {
					j += count;
					rle_data++;
					count = 0;
				} else {
					count -= px - j;
					j = px;
				}
			} else {
				if (j - count <= px) {
					j -= count;
					rle_data -= count;
					count = 0;
				} else {
					count += px - j;
					rle_data += px - j;
					j = px;
				}
			}
		}

		if (!alpha_flag) {
			while (j < psx) {
				if (count > 0) {
					while (count && j < psx) {
						if (*rle_data) {
							const byte *rle_buf = (const byte *)rle_data;
							uint32 cl = make_rgb565u(rle_buf[2], rle_buf[1], rle_buf[0]);
							*scr_buf = cl;
						}
						scr_buf += dx;
						count--;
						j++;
					}
					rle_data++;
				} else {
					if (count < 0) {
						count = -count;
						while (count && j < psx) {
							if (*rle_data) {
								const byte *rle_buf = (const byte *)rle_data;
								uint32 cl = make_rgb565u(rle_buf[2], rle_buf[1], rle_buf[0]);
								*scr_buf = cl;
							}
							scr_buf += dx;
							rle_data++;
							count--;
							j++;
						}
					}
				}
				count = *rle_header++;
			}
		} else {
			while (j < psx) {
				if (count > 0) {
					while (count && j < psx) {
						const byte *rle_buf = (const byte *)rle_data;
						uint32 a = rle_buf[3];
						*scr_buf = alpha_blend_565(make_rgb565u(rle_buf[2], rle_buf[1], rle_buf[0]), *scr_buf, a);
						scr_buf += dx;
						count--;
						j++;
					}
					rle_data++;
				} else {
					if (count < 0) {
						count = -count;
						while (count && j < psx) {
							const byte *rle_buf = (const byte *)rle_data;
							uint32 a = rle_buf[3];
							*scr_buf = alpha_blend_565(make_rgb565u(rle_buf[2], rle_buf[1], rle_buf[0]), *scr_buf, a);
							scr_buf += dx;
							rle_data++;
							count--;
							j++;
						}
					}
				}
				count = *rle_header++;
			}
		}
		y += dy;
	}
}

void grDispatcher::putSpr_rle(int x, int y, int sx, int sy, const class RLEBuffer *p, int mode, float scale, bool alpha_flag) {
	debugC(2, kDebugGraphics, "grDispatcher::putSpr_rle(%d, %d, %d, %d, scale=%f)", x, y, sx, sy, scale);

	int sx_dest = round(float(sx) * scale);
	int sy_dest = round(float(sy) * scale);

	if (sx_dest <= 0 || sy_dest <= 0) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest - 1;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest - 1;
	int iy = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y0 = sy_dest - 1,
		y1 = 0;
		iy = -1;
	}

	if (mode & GR_FLIP_HORIZONTAL) {
		x0 = sx_dest - 1,
		x1 = 0;
		ix = -1;
	}
	if (!alpha_flag) {
		const byte *line_src = RLEBuffer::get_buffer(0);
		for (int i = y0; i != y1; i += iy) {
			p->decode_line(fy >> 16);

			fy += dy;
			fx = (1 << 15);

			for (int j = x0; j != x1; j += ix) {
				if (clipCheck(x + j, y + i)) {
					const byte *src_data = line_src + (fx >> 16) * 3;
					if (src_data[0] || src_data[1] || src_data[2])
						setPixelFast(x + j, y + i, make_rgb565u(src_data[2], src_data[1], src_data[0]));
				}
				fx += dx;
			}
		}
	} else {
		const byte *line_src = RLEBuffer::get_buffer(0);
		for (int i = y0; i != y1; i += iy) {
			p->decode_line(fy >> 16);

			fy += dy;
			fx = (1 << 15);

			for (int j = x0; j != x1; j += ix) {
				if (clipCheck(x + j, y + i)) {
					const byte *src_data = line_src + ((fx >> 16) << 2);

					uint32 a = src_data[3];
					if (a != 255) {
						uint32 cl = make_rgb565u(src_data[2], src_data[1], src_data[0]);

						if (a) {
							uint16 scl;
							getPixel(x + j, y + i, scl);

							setPixelFast(x + j, y + i, alpha_blend_565(cl, scl, a));
						} else
							setPixelFast(x + j, y + i, cl);
					}
				}
				fx += dx;
			}
		}
	}
}

void grDispatcher::putSprMask_rle(int x, int y, int sx, int sy, const RLEBuffer *p, uint32 mask_color, int mask_alpha, int mode, bool alpha_flag) {
	debugC(2, kDebugGraphics, "grDispatcher::putSprMask_rle(%d, %d, %d, %d)", x, y, sx, sy);

	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;

	int dx = -1;
	int dy = -1;

	x *= 2;

	if (mode & GR_FLIP_HORIZONTAL) {
		x += (psx - 1) * 2;
		px = sx - px - psx;
	} else
		dx = 1;

	psx += px;

	if (mode & GR_FLIP_VERTICAL) {
		y += psy - 1;
		py = sy - py - psy;
	} else
		dy = 1;

	warning("STUB: grDispatcher::putSprMask_rle");
	for (int i = 0; i < psy; i++) {
		uint16 *scr_buf = reinterpret_cast<uint16 *>(_screenBuf->getBasePtr(x, y));

		const char *rle_header = p->header_ptr(py + i);
		const uint32 *rle_data = p->data_ptr(py + i);

		int j = 0;
		char count = 0;
		while (j < px) {
			count = *rle_header++;
			if (count > 0) {
				if (count + j <= px) {
					j += count;
					rle_data++;
					count = 0;
				} else {
					count -= px - j;
					j = px;
				}
			} else {
				if (j - count <= px) {
					j -= count;
					rle_data -= count;
					count = 0;
				} else {
					count += px - j;
					rle_data += px - j;
					j = px;
				}
			}
		}
		byte mr, mg, mb;
		split_rgb565u(mask_color, mr, mg, mb);

		mr = (mr * (255 - mask_alpha)) >> 8;
		mg = (mg * (255 - mask_alpha)) >> 8;
		mb = (mb * (255 - mask_alpha)) >> 8;

		uint32 cl = make_rgb565u(mr, mg, mb);

		if (!alpha_flag) {
			while (j < psx) {
				if (count > 0) {
					while (count && j < psx) {
						if (*rle_data) {
							*scr_buf = cl;
						}
						scr_buf += dx;
						count--;
						j++;
					}
					rle_data++;
				} else {
					if (count < 0) {
						count = -count;
						while (count && j < psx) {
							if (*rle_data) {
								*scr_buf = cl;
							}
							scr_buf += dx;
							rle_data++;
							count--;
							j++;
						}
					}
				}
				count = *rle_header++;
			}
		} else {
			while (j < psx) {
				if (count > 0) {
					while (count && j < psx) {
						const byte *rle_buf = (const byte *)rle_data;
						uint32 a = rle_buf[3];

						if (a != 255) {
							a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

							uint32 r = (mr * (255 - a)) >> 8;
							uint32 g = (mg * (255 - a)) >> 8;
							uint32 b = (mb * (255 - a)) >> 8;

							cl = make_rgb565u(r, g, b);
							*scr_buf = alpha_blend_565(cl, *scr_buf, a);
						}

						scr_buf += dx;
						count--;
						j++;
					}
					rle_data++;
				} else {
					if (count < 0) {
						count = -count;
						while (count && j < psx) {
							const byte *rle_buf = (const byte *)rle_data;
							uint32 a = rle_buf[3];

							if (a != 255) {
								a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

								uint32 r = (mr * (255 - a)) >> 8;
								uint32 g = (mg * (255 - a)) >> 8;
								uint32 b = (mb * (255 - a)) >> 8;

								cl = make_rgb565u(r, g, b);
								*scr_buf = alpha_blend_565(cl, *scr_buf, a);
							}

							scr_buf += dx;
							rle_data++;
							count--;
							j++;
						}
					}
				}
				count = *rle_header++;
			}
		}
		y += dy;
	}
}

void grDispatcher::putSprMask_rle(int x, int y, int sx, int sy, const RLEBuffer *p, uint32 mask_color, int mask_alpha, int mode, float scale, bool alpha_flag) {
	debugC(2, kDebugGraphics, "grDispatcher::putSprMask_rle(%d, %d, %d, %d, scale=%f)", x, y, sx, sy, scale);

	int sx_dest = round(float(sx) * scale);
	int sy_dest = round(float(sy) * scale);

	if (sx_dest <= 0 || sy_dest <= 0) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest - 1;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest - 1;
	int iy = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y0 = sy_dest - 1,
		y1 = 0;
		iy = -1;
	}

	if (mode & GR_FLIP_HORIZONTAL) {
		x0 = sx_dest - 1,
		x1 = 0;
		ix = -1;
	}
	if (!alpha_flag) {
		byte mr, mg, mb;
		split_rgb565u(mask_color, mr, mg, mb);

		mr = (mr * (255 - mask_alpha)) >> 8;
		mg = (mg * (255 - mask_alpha)) >> 8;
		mb = (mb * (255 - mask_alpha)) >> 8;

		uint32 mcl = (_pixel_format == GR_RGB565) ? make_rgb565u(mr, mg, mb) : make_rgb555u(mr, mg, mb);

		const byte *line_src = RLEBuffer::get_buffer(0);

		for (int i = y0; i != y1; i += iy) {
			p->decode_line(fy >> 16);

			fy += dy;
			fx = (1 << 15);

			for (int j = x0; j != x1; j += ix) {
				if (clipCheck(x + j, y + i)) {
					const byte *src_buf = line_src + ((fx >> 16) << 2);
					if (src_buf[0] || src_buf[1] || src_buf[2]) {
						uint16 scl;
						getPixel(x + j, y + i, scl);
						setPixelFast(x + j, y + i, alpha_blend_565(mcl, scl, mask_alpha));
					}
				}
				fx += dx;
			}
		}
	} else {
		const byte *line_src = RLEBuffer::get_buffer(0);
		byte mr, mg, mb;
		split_rgb565u(mask_color, mr, mg, mb);

		for (int i = y0; i != y1; i += iy) {
			p->decode_line(fy >> 16);

			fy += dy;
			fx = (1 << 15);

			for (int j = x0; j != x1; j += ix) {
				if (clipCheck(x + j, y + i)) {
					const byte *src_buf = line_src + ((fx >> 16) << 2);
					uint32 a = src_buf[3];
					if (a != 255) {
						uint16 scl;
						getPixel(x + j, y + i, scl);

						a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

						uint32 r = (mr * (255 - a)) >> 8;
						uint32 g = (mg * (255 - a)) >> 8;
						uint32 b = (mb * (255 - a)) >> 8;

						uint32 cl = make_rgb565u(r, g, b);

						setPixelFast(x + j, y + i, alpha_blend_565(cl, scl, a));
					}
				}
				fx += dx;
			}
		}
		}
}

void grDispatcher::putSpr_rle_rot(const Vect2i &pos, const Vect2i &size, const RLEBuffer *data, bool has_alpha, int mode, float angle) {
	byte *buf = (byte *)temp_buffer(size.x * size.y * 4);

	byte *buf_ptr = buf;
	for (int i = 0; i < size.y; i++) {
		data->decode_line(i, buf_ptr);
		buf_ptr += size.x * 4;
	}

	if (!has_alpha) {
		uint32 *p = (uint32 *)buf;
		buf_ptr = buf + 3;
		for (int i = 0; i < size.y; i++) {
			for (int j = 0; j < size.x; j++) {
				if (!*p++)
					*buf_ptr = 255;

				buf_ptr += 4;
			}
		}
	}

	putSpr_rot(pos, size, buf, true, mode, angle);
}

void grDispatcher::putSpr_rle_rot(const Vect2i &pos, const Vect2i &size, const RLEBuffer *data, bool has_alpha, int mode, float angle, const Vect2f &scale) {
	byte *buf = (byte *)temp_buffer(size.x * size.y * 4);

	byte *buf_ptr = buf;
	for (int i = 0; i < size.y; i++) {
		data->decode_line(i, buf_ptr);
		buf_ptr += size.x * 4;
	}

	if (!has_alpha) {
		uint32 *p = (uint32 *)buf;
		buf_ptr = buf + 3;
		for (int i = 0; i < size.y; i++) {
			for (int j = 0; j < size.x; j++) {
				if (!*p++)
					*buf_ptr = 255;

				buf_ptr += 4;
			}
		}
	}

	putSpr_rot(pos, size, buf, true, mode, angle, scale);
}

void grDispatcher::putSprMask_rle_rot(const Vect2i &pos, const Vect2i &size, const RLEBuffer *data, bool has_alpha, uint32 mask_color, int mask_alpha, int mode, float angle) {
	byte *buf = (byte *)temp_buffer(size.x * size.y * 4);

	byte *buf_ptr = buf;
	for (int i = 0; i < size.y; i++) {
		data->decode_line(i, buf_ptr);
		buf_ptr += size.x * 4;
	}

	if (!has_alpha) {
		uint32 *p = (uint32 *)buf;
		buf_ptr = buf + 3;
		for (int i = 0; i < size.y; i++) {
			for (int j = 0; j < size.x; j++) {
				if (!*p++)
					*buf_ptr = 255;

				buf_ptr += 4;
			}
		}
	}

	putSprMask_rot(pos, size, buf, true, mask_color, mask_alpha, mode, angle);
}

void grDispatcher::putSprMask_rle_rot(const Vect2i &pos, const Vect2i &size, const RLEBuffer *data, bool has_alpha, uint32 mask_color, int mask_alpha, int mode, float angle, const Vect2f &scale) {
	byte *buf = (byte *)temp_buffer(size.x * size.y * 4);

	byte *buf_ptr = buf;
	for (int i = 0; i < size.y; i++) {
		data->decode_line(i, buf_ptr);
		buf_ptr += size.x * 4;
	}

	if (!has_alpha) {
		uint32 *p = (uint32 *)buf;
		buf_ptr = buf + 3;
		for (int i = 0; i < size.y; i++) {
			for (int j = 0; j < size.x; j++) {
				if (!*p++)
					*buf_ptr = 255;

				buf_ptr += 4;
			}
		}
	}

	putSprMask_rot(pos, size, buf, true, mask_color, mask_alpha, mode, angle, scale);
}

inline bool rle_alpha_b(uint32 pixel) {
	return (reinterpret_cast<byte *>(&pixel)[3] < 200);
}
inline bool rle_alpha_b16(uint16 pixel) {
	return pixel < 200;
}

void grDispatcher::drawSprContour(int x, int y, int sx, int sy, const class RLEBuffer *p, int contour_color, int mode, bool alpha_flag) {
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;
 	int dx = -1;
	int dy = -1;

	x *= 2;

	if (mode & GR_FLIP_HORIZONTAL) {
		x += (psx - 1) * 2;
		px = sx - px - psx;
	} else
		dx = 1;

	if (mode & GR_FLIP_VERTICAL) {
		y += psy - 1;
		py = sy - py - psy;
	} else
		dy = 1;

	const uint16 *data0 = reinterpret_cast<const uint16 *>(RLEBuffer::get_buffer(0));
	const uint16 *data1 = reinterpret_cast<const uint16 *>(RLEBuffer::get_buffer(1));

	px <<= 1;
	psx <<= 1;

	warning("STUB: grDispatcher::drawSprContour");
	for (int i = 0; i < psy; i++) {
		uint16 *scr_buf = reinterpret_cast<uint16 *>(_screenBuf->getBasePtr(x, y));
		uint16 *scr_buf_prev = (i) ? reinterpret_cast<uint16 *>(_screenBuf + _yTable[y - dy] + x) : scr_buf;
		p->decode_line(py + i, i & 1);

		const uint16 *data_ptr = (i & 1) ? data1 + px : data0 + px;
		const uint16 *data_ptr_prev = (i & 1) ? data0 + px : data1 + px;

		if (!alpha_flag) {
			uint32 pixel = 0;
			for (int j = 0; j < psx; j += 2) {
				pixel = data_ptr[j];

				if (!pixel && j && data_ptr[j - 2]) {
					*(scr_buf - dx) = contour_color;
				}
				if ((pixel && (!j || !data_ptr[j - 2]))) {
					*scr_buf = contour_color;
				} else {
					if (pixel && (!i || !data_ptr_prev[j])) {
						*scr_buf = contour_color;
					}
				}
				if (!pixel && i && data_ptr_prev[j]) {
					*scr_buf_prev = contour_color;
				}

				scr_buf += dx;
				scr_buf_prev += dx;
			}
			if (pixel) *(scr_buf - dx) = contour_color;
		} else {
			bool pixel = false;
			for (int j = 0; j < psx; j += 2) {
				pixel = rle_alpha_b16(data_ptr[j + 1]);

				if (!pixel && j && rle_alpha_b16(data_ptr[j - 1])) {
					*(scr_buf - dx) = contour_color;
				}
				if ((pixel && (!j || !rle_alpha_b16(data_ptr[j - 1])))) {
					*scr_buf = contour_color;
				} else {
					if (pixel && (!i || !rle_alpha_b16(data_ptr_prev[j + 1]))) {
						*scr_buf = contour_color;
					}
				}
				if (!pixel && i && rle_alpha_b16(data_ptr_prev[j + 1])) {
					*scr_buf_prev = contour_color;
				}

				scr_buf += dx;
				scr_buf_prev += dx;
			}
			if (pixel) *(scr_buf - dx) = contour_color;
		}

		y += dy;
	}
	uint16 *scr_buf_prev = reinterpret_cast<uint16 *>(_screenBuf + _yTable[y - dy] + x);
	const uint16 *data_ptr_prev = (psy & 1) ? data0 + px : data1 + px;
	if (!alpha_flag) {
		for (int j = 0; j < psx; j += 2) {
			if (data_ptr_prev[j])
				*scr_buf_prev = contour_color;
			scr_buf_prev += dx;
		}
	} else {
		for (int j = 0; j < psx; j += 2) {
			if (rle_alpha_b16(data_ptr_prev[j + 1]))
				*scr_buf_prev = contour_color;
			scr_buf_prev += dx;
		}
	}

}

void grDispatcher::drawSprContour(int x, int y, int sx, int sy, const class RLEBuffer *p, int contour_color, int mode, float scale, bool alpha_flag) {
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

	if (!alpha_flag) {
		const uint16 *line0 = reinterpret_cast<const uint16 *>(RLEBuffer::get_buffer(0));
		const uint16 *line1 = reinterpret_cast<const uint16 *>(RLEBuffer::get_buffer(1));

		for (int i = y0; i != y1; i += iy) {
			p->decode_line(fy >> 16, i & 1);
			const uint16 *line_src = (i & 1) ? line1 : line0;
			const uint16 *line_src_prev = (i & 1) ? line0 : line1;

			fy += dy;
			fx = (1 << 15);

			uint32 cl = 0;
			for (int j = x0; j != x1; j += ix) {
				if (clipCheck(x + j, y + i)) {
					cl = line_src[(fx >> 16) << 1];
					if (!cl && j != x0 && line_src[((fx - dx) >> 16) << 1])
						setPixel(x + j - ix, y + i, contour_color);

					if (cl && (j == x0 || !line_src[((fx - dx) >> 16) << 1])) {
						setPixelFast(x + j, y + i, contour_color);
					} else {
						if (cl && (i == y0 || !line_src_prev[(fx >> 16) << 1]))
							setPixelFast(x + j, y + i, contour_color);
					}

					if (!cl && i != y0 && line_src_prev[(fx >> 16) << 1])
						setPixel(x + j, y + i - iy, contour_color);
				}
				fx += dx;
			}
			if (cl) setPixel(x + x1 - ix, y + i, contour_color);
		}
		fx = (1 << 15);
		for (int j = x0; j != x1; j += ix) {
			const uint16 *line_src_prev = (y1 & 1) ? line0 : line1;
			if (line_src_prev[(fx >> 16) << 1])
				setPixel(x + j, y + y1 - iy, contour_color);
			fx += dx;
		}
	} else {
		const uint16 *line0 = reinterpret_cast<const uint16 *>(RLEBuffer::get_buffer(0));
		const uint16 *line1 = reinterpret_cast<const uint16 *>(RLEBuffer::get_buffer(1));

		for (int i = y0; i != y1; i += iy) {
			p->decode_line(fy >> 16, i & 1);
			const uint16 *line_src = (i & 1) ? line1 : line0;
			const uint16 *line_src_prev = (i & 1) ? line0 : line1;

			fy += dy;
			fx = (1 << 15);

			bool cl = false;
			for (int j = x0; j != x1; j += ix) {
				if (clipCheck(x + j, y + i)) {
					cl = rle_alpha_b16(line_src[((fx >> 16) << 1) + 1]);
					if (!cl && j != x0 && rle_alpha_b16(line_src[(((fx - dx) >> 16) << 1) + 1]))
						setPixel(x + j - ix, y + i, contour_color);

					if (cl && (j == x0 || !rle_alpha_b16(line_src[(((fx - dx) >> 16) << 1) + 1]))) {
						setPixelFast(x + j, y + i, contour_color);
					} else {
						if (cl && (i == y0 || !rle_alpha_b16(line_src_prev[((fx >> 16) << 1) + 1])))
							setPixelFast(x + j, y + i, contour_color);
					}

					if (!cl && i != y0 && rle_alpha_b16(line_src_prev[((fx >> 16) << 1) + 1]))
						setPixel(x + j, y + i - iy, contour_color);
				}
				fx += dx;
			}
			if (cl) setPixel(x + x1 - ix, y + i, contour_color);
		}
		fx = (1 << 15);
		for (int j = x0; j != x1; j += ix) {
			const uint16 *line_src_prev = (y1 & 1) ? line0 : line1;
			if (rle_alpha_b16(line_src_prev[((fx >> 16) << 1) + 1]))
				setPixel(x + j, y + y1 - iy, contour_color);
			fx += dx;
		}
	}
}

} // namespace QDEngine
