/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "dune/graphics.h"

#include "dune/dune.h"
#include "dune/hsq.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/util.h"
#include "engines/util.h"
#include "graphics/palette.h"

namespace Dune {

Graphics::Graphics(DuneEngine *engine) :
	_engine(engine),
	_global_y_offset(0)
{}

void Graphics::setMode() {
	initGraphics(320, 200);
}

void Graphics::getInfo(byte **screen, int *size) {
	*screen = new byte[320*200];
	*size = 64000;
}

void Graphics::drawCursor(int x, int y, const byte *cursorData) {
	int hotspot_y = READ_LE_INT16(cursorData+0);
	int hotspot_x = READ_LE_INT16(cursorData+2);

	y = MAX(y - hotspot_y, 0);
	x = MAX(x - hotspot_x, 0);
	// DNVGA includes _global_y_offset but doesn't check for overflow?

	int w = MIN(16, 200 - x);
	int h = MIN(16, 320 - y);

	byte *dst = _framebuffer + 320 * y + x;

	for (int cy = 0; cy != h; ++cy) {
		uint16_t mask_b = READ_LE_UINT16(cursorData+4+2*cy);
		uint16_t mask_w = READ_LE_UINT16(cursorData+4+2*cy+32);

		for (int cx = 0; cx != w; ++cx) {
			byte v = *dst;
			if (!(mask_b & 0x8000)) {
				v = 0x00;
			}
			if (mask_w & 0x8000) {
				v = 0x0f;
			}
			mask_b <<= 1;
			mask_w <<= 1;
			*dst++ = v;
		}
		dst += 320 - w;
	}
}

void Graphics::setGlobalYOffset(int y_offset) {
	assert(y_offset >= 0);
	assert(y_offset < 200);

	_global_y_offset = y_offset;
}

byte *Graphics::getPixelPtr(int x, int y) {
	y += _global_y_offset;
	if (y > 199) y = 199;
	return _framebuffer + 320 * y + x;
}

void Graphics::blitGraphics(byte *buf, int buf_len, int x_offset, int y_offset, int w, int h, byte flags, byte mode) {
	Common::MemoryReadStream r(buf, buf_len);
	blitGraphics(&r, x_offset, y_offset, w, h, flags, mode);
}

#define ADVANCE(p) do { p += (!flip_x ? 1 : -1); } while (0)

void Graphics::draw_4bpp(bool flip_x, bool flip_y, Common::ReadStream *r, int x_offset, int y_offset, int w, int h, byte mode) {
	for (int y = 0; y < h; ++y) {
		byte *dst = getPixelPtr(x_offset, y + y_offset);
		int line_remain = 4 * ((w + 3) / 4);

		if (flip_x) {
			dst += w - 1;
		}

		do {
			byte value = r->readByte();
			byte p1 = (value & 0x0f);
			byte p2 = (value >> 4);
			if (p1) {
				*dst = p1 + mode;
			}
			ADVANCE(dst);
			if (p2) {
				*dst = p2 + mode;
			}
			ADVANCE(dst);
			line_remain -= 2;
		} while (line_remain > 0);
	}
}

void Graphics::draw_4bpp_rle(bool flip_x, bool flip_y, Common::ReadStream *r, int x_offset, int y_offset, int w, int h, byte mode) {
	for (int y = 0; y != h; ++y) {
		byte *dst = getPixelPtr(x_offset, y + y_offset);
		int line_remain = 4 * ((w + 3) / 4);

		if (flip_x) {
			dst += w - 1;
		}

		do {
			byte cmd = r->readByte();
			if (cmd & 0x80) {
				int count = 257 - cmd;
				byte value = r->readByte();

				byte p1 = (value & 0x0f);
				byte p2 = (value >> 4);
				for (int i = 0; i != count; ++i) {
					if (p1) {
						*dst = p1 + mode;
					}
					ADVANCE(dst);
					if (p2) {
						*dst = p2 + mode;
					}
					ADVANCE(dst);
				}
				line_remain -= 2 * count;
			} else {
				int count = cmd + 1;
				for (int i = 0; i != count; ++i) {
					byte value = r->readByte();

					byte p1 = (value & 0x0f);
					byte p2 = (value >> 4);
					if (p1) {
						*dst = p1 + mode;
					}
					ADVANCE(dst);
					if (p2) {
						*dst = p2 + mode;
					}
					ADVANCE(dst);
				}
				line_remain -= 2 * count;
			}
		} while (line_remain > 0);
	}
}

void Graphics::draw_8bpp(bool flip_x, bool flip_y, Common::ReadStream *r, int x_offset, int y_offset, int w, int h, byte mode) {
	for (int y = 0; y < h; ++y) {
		byte *dst = getPixelPtr(x_offset, y + y_offset);
		int line_remain = 4 * ((w + 3) / 4);

		if (flip_x) {
			dst += w - 1;
		}

		// mode 255 means that 0 is transparent.
		if (mode == 255) {
			for (int x = 0; x != line_remain; ++x) {
				byte value = r->readByte();
				if (value) {
					*dst = value;
				}
				ADVANCE(dst);
			}
		} else {
			r->read(dst, w);
		}
	}
}

void Graphics::draw_8bpp_rle(bool flip_x, bool flip_y, Common::ReadStream *r, int x_offset, int y_offset, int w, int h, byte mode) {
	for (int y = 0; y != h; ++y) {
		int line_remain = 4 * ((w + 3) / 4);

		byte *dst = getPixelPtr(x_offset, y + y_offset);

		if (flip_x) {
			dst += w - 1;
		}

		do {
			byte cmd = r->readByte();
			if (cmd & 0x80) {
				int count = 257 - cmd;
				byte value = r->readByte();
				if (mode == 255 && value == 0) {
					dst += flip_x ? -count : count;
				} else {
					for (int i = 0; i != count; ++i) {
						*dst = value;
						ADVANCE(dst);
					}
				}
				line_remain -= count;
			} else {
				int count = cmd + 1;
				for (int i = 0; i != count; ++i) {
					byte value = r->readByte();
					if (mode == 255 && value == 0) {
					} else {
						*dst = value;
					}
					ADVANCE(dst);
				}
				line_remain -= count;
			}
		} while (line_remain > 0);
	}
}

void Graphics::blitGraphics(Common::ReadStream *r, int x_offset, int y_offset, int w, int h, byte flags, byte mode) {
	if (x_offset+w > 320) {
		return;
	}
	if (y_offset+h > 200) {
		return;
	}
	assert(x_offset >= 0);
	assert(x_offset < 320);
	assert(w <= 320);
	assert(x_offset+w <= 320);
	assert(y_offset >= 0);
	assert(y_offset < 200);
	assert(h <= 200);
	assert(y_offset+h <= 200);

	bool flip_x = flags & 0x20;
	bool flip_y = flags & 0x40;

	if (mode < 254) {
		if ((flags & 0x80) == 0) {
			draw_4bpp(flip_x, flip_y, r, x_offset, y_offset, w, h, mode);
		} else {
			draw_4bpp_rle(flip_x, flip_y, r, x_offset, y_offset, w, h, mode);
		}
	} else {
		if ((flags & 0x80) == 0) {
			draw_8bpp(flip_x, flip_y, r, x_offset, y_offset, w, h, mode);
		} else {
			draw_8bpp_rle(flip_x, flip_y, r, x_offset, y_offset, w, h, mode);
		}
	}
}

void Graphics::copyScaled(
	byte *src, uint16 src_pitch,
	int x_offset, int y_offset,
	int dst_w, int dst_h, byte flags,
	byte pal_offset, uint16 src_delta_fp)
{
	bool flip_x = !!(flags & 0x40);
	bool flip_y = !!(flags & 0x20);

	if (flip_x) {
		x_offset += dst_w - 1;
	}
	if (flip_y) {
		y_offset += dst_h - 1;
	}

	// Source coordinates in 8.8 fixed point
	uint16 src_x_fp = 0;
	uint16 src_y_fp = 0;

	for (int dst_y = 0; dst_y != dst_h; dst_y++, src_y_fp += src_delta_fp) {
		byte *dst = getPixelPtr(x_offset, dst_y + y_offset);
		uint16 src_y = (src_y_fp >> 8);
		src_x_fp = 0;
		for (int dst_x = 0; dst_x != dst_w; dst_x++, src_x_fp += src_delta_fp) {
			uint16 src_x = (src_x_fp >> 8);

			byte value = src[src_y * src_pitch + src_x / 2];
			if (src_x & 1) {
				value >>= 4;
			}
			value &= 0x0f;
			if (value != 0) {
				*dst = value + pal_offset;
			}
			ADVANCE(dst);
		}
	}
}

#undef ADVANCE

void Graphics::drawNoise(uint16 x, uint16 y, uint16 count, uint16 start_color, uint16 color_increment_fp, uint16 xor_state, uint16 xor_increment) {
	assert(count > 0);

	byte *dst = getPixelPtr(x, y);
	do {
		bool shouldXor = xor_state & 1;
		xor_state = xor_state >> 1;
		if (shouldXor) {
			xor_state = xor_state ^ xor_increment;
		}
		*dst++ = (start_color >> 8) + (int8(xor_state & 3) - 1);
		start_color = start_color + color_increment_fp;
		count = count - 1;
	} while (count != 0);
}

} // namespace Dune
