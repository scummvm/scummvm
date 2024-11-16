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
#include "qdengine/system/graphics/rle_compress.h"


namespace QDEngine {

#ifdef _GR_ENABLE_ZBUFFER
void grDispatcher::putSpr_rle_z(int x, int y, int z, int sx, int sy, const class RLEBuffer *p, int mode, bool alpha_flag) {
	debugC(4, kDebugGraphics, "grDispatcher::putSpr_rle_z([%d, %d], [%d, %d], mode: %d, alpha: %d", x, y, sx, sy, mode, alpha_flag);

	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;

	if (bytes_per_pixel() == 4) {
		int dx = -4;
		int zdx = -1;
		int dy = -1;

		int x3 = x * 4;

		if (mode & GR_FLIP_HORIZONTAL) {
			x3 += (psx - 1) * 4;
			x += psx - 1;
			px = sx - px - psx;
		} else {
			dx = 4;
			zdx = 1;
		}

		psx += px;

		if (mode & GR_FLIP_VERTICAL) {
			y += psy - 1;
			py = sy - py - psy;
		} else
			dy = 1;

		for (int i = 0; i < psy; i++) {
			byte *scr_buf = reinterpret_cast<byte *>(screenBuf + yTable[y] + x3);
			zbuf_t *z_buf = zbuffer_ + SizeX * y + x;

			const int8 *rle_header = p->header_ptr(py + i);
			const uint32 *rle_data = p->data_ptr(py + i);

			int j = 0;
			int8 count = 0;
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
								*reinterpret_cast<uint32 *>(scr_buf) = *rle_data;
								*z_buf = z;
							}
							z_buf += zdx;
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
									*reinterpret_cast<uint32 *>(scr_buf) = *rle_data++;
									*z_buf = z;
								}
								z_buf += zdx;
								scr_buf += dx;
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
							uint32 a = reinterpret_cast<const byte *>(rle_data)[3];
							if (a != 255) {
								scr_buf[0] = reinterpret_cast<const byte *>(rle_data)[0] + ((a * scr_buf[0]) >> 8);
								scr_buf[1] = reinterpret_cast<const byte *>(rle_data)[1] + ((a * scr_buf[1]) >> 8);
								scr_buf[2] = reinterpret_cast<const byte *>(rle_data)[2] + ((a * scr_buf[2]) >> 8);
								*z_buf = z;
							}
							z_buf += zdx;
							scr_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					} else {
						if (count < 0) {
							count = -count;
							while (count && j < psx) {
								uint32 a = reinterpret_cast<const byte *>(rle_data)[3];
								if (a != 255) {
									scr_buf[0] = reinterpret_cast<const byte *>(rle_data)[0] + ((a * scr_buf[0]) >> 8);
									scr_buf[1] = reinterpret_cast<const byte *>(rle_data)[1] + ((a * scr_buf[1]) >> 8);
									scr_buf[2] = reinterpret_cast<const byte *>(rle_data)[2] + ((a * scr_buf[2]) >> 8);
									*z_buf = z;
								}
								rle_data++;
								z_buf += zdx;
								scr_buf += dx;
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
		} else  {
			dx = 3;
			zdx = 1;
		}

		psx += px;

		if (mode & GR_FLIP_VERTICAL) {
			y += psy - 1;
			py = sy - py - psy;
		} else
			dy = 1;

		for (int i = 0; i < psy; i++) {
			byte *scr_buf = reinterpret_cast<byte *>(screenBuf + yTable[y] + x3);
			zbuf_t *z_buf = zbuffer_ + SizeX * y + x;

			const int8 *rle_header = p->header_ptr(py + i);
			const uint32 *rle_data = p->data_ptr(py + i);

			int j = 0;
			int8 count = 0;
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
								scr_buf[0] = reinterpret_cast<const byte *>(rle_data)[0];
								scr_buf[1] = reinterpret_cast<const byte *>(rle_data)[1];
								scr_buf[2] = reinterpret_cast<const byte *>(rle_data)[2];
								*z_buf = z;
							}
							z_buf += zdx;
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
									scr_buf[0] = reinterpret_cast<const byte *>(rle_data)[0];
									scr_buf[1] = reinterpret_cast<const byte *>(rle_data)[1];
									scr_buf[2] = reinterpret_cast<const byte *>(rle_data)[2];
									*z_buf = z;
								}
								scr_buf += dx;
								z_buf += zdx;
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
							uint32 a = reinterpret_cast<const byte *>(rle_data)[3];
							if (a != 255) {
								scr_buf[0] = reinterpret_cast<const byte *>(rle_data)[0] + ((a * scr_buf[0]) >> 8);
								scr_buf[1] = reinterpret_cast<const byte *>(rle_data)[1] + ((a * scr_buf[1]) >> 8);
								scr_buf[2] = reinterpret_cast<const byte *>(rle_data)[2] + ((a * scr_buf[2]) >> 8);
								*z_buf = z;
							}
							scr_buf += dx;
							z_buf += zdx;
							count--;
							j++;
						}
						rle_data++;
					} else {
						if (count < 0) {
							count = -count;
							while (count && j < psx) {
								uint32 a = reinterpret_cast<const byte *>(rle_data)[3];
								if (a != 255) {
									scr_buf[0] = reinterpret_cast<const byte *>(rle_data)[0] + ((a * scr_buf[0]) >> 8);
									scr_buf[1] = reinterpret_cast<const byte *>(rle_data)[1] + ((a * scr_buf[1]) >> 8);
									scr_buf[2] = reinterpret_cast<const byte *>(rle_data)[2] + ((a * scr_buf[2]) >> 8);
									*z_buf = z;
								}
								scr_buf += dx;
								z_buf += zdx;
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
		return;
	}
	if (bytes_per_pixel() == 2) {
		int dx = -1;
		int dy = -1;

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

		for (int i = 0; i < psy; i++) {
			uint16 *scr_buf = reinterpret_cast<uint16 *>(screenBuf + yTable[y] + x * 2);
			zbuf_t *z_buf = zbuffer_ + y * SizeX + x;

			const int8 *rle_header = p->header_ptr(py + i);
			const uint32 *rle_data = p->data_ptr(py + i);

			int j = 0;
			int8 count = 0;
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
								*scr_buf = reinterpret_cast<const uint16 *>(rle_data)[0];
								*z_buf = z;
							}
							scr_buf += dx;
							z_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					} else {
						if (count < 0) {
							count = -count;
							while (count && j < psx) {
								if (*rle_data) {
									*scr_buf = reinterpret_cast<const uint16 *>(rle_data)[0];
									*z_buf = z;
								}
								scr_buf += dx;
								z_buf += dx;
								rle_data++;
								count--;
								j++;
							}
						}
					}
					count = *rle_header++;
				}
			} else {
				const uint32 mask_r = (pixel_format_ == GR_RGB565) ? mask_565_r : mask_555_r;
				const uint32 mask_g = (pixel_format_ == GR_RGB565) ? mask_565_g : mask_555_g;
				const uint32 mask_b = (pixel_format_ == GR_RGB565) ? mask_565_b : mask_555_b;

				while (j < psx) {
					if (count > 0) {
						while (count && j < psx) {
							uint32 a = reinterpret_cast<const uint16 *>(rle_data)[1];
							if (a != 255) {
								uint32 sc = *scr_buf;
								*scr_buf = reinterpret_cast<const uint16 *>(rle_data)[0] +
								           (((((sc & mask_r) * a) >> 8) & mask_r) |
								            ((((sc & mask_g) * a) >> 8) & mask_g) |
								            ((((sc & mask_b) * a) >> 8) & mask_b));
								*z_buf = z;
							}
							scr_buf += dx;
							z_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					} else {
						if (count < 0) {
							count = -count;
							while (count && j < psx) {
								uint32 a = reinterpret_cast<const uint16 *>(rle_data)[1];
								if (a != 255) {
									uint32 sc = *scr_buf;
									*scr_buf = reinterpret_cast<const uint16 *>(rle_data)[0] +
									           (((((sc & mask_r) * a) >> 8) & mask_r) |
									            ((((sc & mask_g) * a) >> 8) & mask_g) |
									            ((((sc & mask_b) * a) >> 8) & mask_b));
									*z_buf = z;
								}
								scr_buf += dx;
								z_buf += dx;
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
		return;
	}
}

void grDispatcher::putSpr_rle_z(int x, int y, int z, int sx, int sy, const class RLEBuffer *p, int mode, float scale, bool alpha_flag) {
	debugC(4, kDebugGraphics, "grDispatcher::putSpr_rle_z([%d, %d], [%d, %d], mode: %d, scale: %f, alpha: %d", x, y, sx, sy, mode, scale, alpha_flag);

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
		if (!alpha_flag) {
			const uint16 *line_src = reinterpret_cast<const uint16 *>(RLEBuffer::get_buffer(0));

			for (int i = y0; i != y1; i += iy) {
				p->decode_line(fy >> 16);

				fy += dy;
				fx = (1 << 15);

				for (int j = x0; j != x1; j += ix) {
					if (ClipCheck(x + j, y + i)) {
						uint32 cl = line_src[(fx >> 16) << 1];
						if (cl) {
							setPixelFast(x + j, y + i, cl);
							put_z(x + j, y + i, z);
						}
					}
					fx += dx;
				}
			}
		} else {
			const uint16 *line_src = reinterpret_cast<const uint16 *>(RLEBuffer::get_buffer(0));

			const uint32 mask_r = (pixel_format_ == GR_RGB565) ? mask_565_r : mask_555_r;
			const uint32 mask_g = (pixel_format_ == GR_RGB565) ? mask_565_g : mask_555_g;
			const uint32 mask_b = (pixel_format_ == GR_RGB565) ? mask_565_b : mask_555_b;

			for (int i = y0; i != y1; i += iy) {
				p->decode_line(fy >> 16);

				fy += dy;
				fx = (1 << 15);

				for (int j = x0; j != x1; j += ix) {
					if (ClipCheck(x + j, y + i)) {
						uint32 a = line_src[((fx >> 16) << 1) + 1];
						if (a != 255) {
							uint32 cl = line_src[(fx >> 16) << 1];

							uint32 scl;
							getPixel(x + j, y + i, scl);

							scl = cl +
							      (((((scl & mask_r) * a) >> 8) & mask_r) |
							       ((((scl & mask_g) * a) >> 8) & mask_g) |
							       ((((scl & mask_b) * a) >> 8) & mask_b));

							setPixelFast(x + j, y + i, scl);
							put_z(x + j, y + i, z);
						}
					}
					fx += dx;
				}
			}
		}
		return;
	}

	if (bytes_per_pixel() == 3 || bytes_per_pixel() == 4) {
		int sx3 = sx * 3;
		const byte *line_src = RLEBuffer::get_buffer(0);

		if (!alpha_flag) {
			for (int i = y0; i != y1; i += iy) {
				p->decode_line(fy >> 16);

				fy += dy;
				fx = (1 << 15);

				for (int j = x0; j != x1; j += ix) {
					if (ClipCheck(x + j, y + i)) {
						int idx = (fx >> 16) << 2;

						uint32 r = line_src[idx + 2];
						uint32 g = line_src[idx + 1];
						uint32 b = line_src[idx + 0];

						if (r || g || b) {
							setPixelFast(x + j, y + i, r, g, b);
							put_z(x + j, y + i, z);
						}
					}
					fx += dx;
				}
			}
		} else {
			for (int i = y0; i != y1; i += iy) {
				p->decode_line(fy >> 16);

				fy += dy;
				fx = (1 << 15);

				for (int j = x0; j != x1; j += ix) {
					if (ClipCheck(x + j, y + i)) {
						int idx = (fx >> 16) << 2;

						uint32 a = line_src[idx + 3];
						if (a != 255) {
							uint32 sr, sg, sb;
							getPixel(x + j, y + i, sr, sg, sb);

							uint32 r = line_src[idx + 2] + ((a * sr) >> 8);
							uint32 g = line_src[idx + 1] + ((a * sg) >> 8);
							uint32 b = line_src[idx + 0] + ((a * sb) >> 8);

							setPixelFast(x + j, y + i, r, g, b);
							put_z(x + j, y + i, z);
						}
					}

					fx += dx;
				}
			}
		}

		return;
	}
}
#endif
} // namespace QDEngine
