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
 */

#include "common/algorithm.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "graphics/primitives.h"
#include "graphics/surface.h"

namespace Graphics {

template<typename T>
static void plotPoint(int x, int y, int color, void *data) {
	Surface *s = (Surface *)data;
	if (x >= 0 && x < s->w && y >= 0 && y < s->h) {
		T *ptr = (T *)s->getBasePtr(x, y);
		*ptr = (T)color;
	}
}

void Surface::drawLine(int x0, int y0, int x1, int y1, uint32 color) {
	if (format.bytesPerPixel == 1)
		Graphics::drawLine(x0, y0, x1, y1, color, plotPoint<byte>, this);
	else if (format.bytesPerPixel == 2)
		Graphics::drawLine(x0, y0, x1, y1, color, plotPoint<uint16>, this);
	else if (format.bytesPerPixel == 4)
		Graphics::drawLine(x0, y0, x1, y1, color, plotPoint<uint32>, this);
	else
		error("Surface::drawLine: bytesPerPixel must be 1, 2, or 4");
}

void Surface::create(uint16 width, uint16 height, const PixelFormat &f) {
	free();

	w = width;
	h = height;
	format = f;
	pitch = w * format.bytesPerPixel;

	pixels = calloc(width * height, format.bytesPerPixel);
	assert(pixels);
}

void Surface::free() {
	::free(pixels);
	pixels = 0;
	w = h = pitch = 0;
	format = PixelFormat();
}

void Surface::copyFrom(const Surface &surf) {
	create(surf.w, surf.h, surf.format);
	memcpy(pixels, surf.pixels, h * pitch);
}

void Surface::hLine(int x, int y, int x2, uint32 color) {
	// Clipping
	if (y < 0 || y >= h)
		return;

	if (x2 < x)
		SWAP(x2, x);

	if (x < 0)
		x = 0;
	if (x2 >= w)
		x2 = w - 1;

	if (x2 < x)
		return;

	if (format.bytesPerPixel == 1) {
		byte *ptr = (byte *)getBasePtr(x, y);
		memset(ptr, (byte)color, x2 - x + 1);
	} else if (format.bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)getBasePtr(x, y);
		Common::set_to(ptr, ptr + (x2 - x + 1), (uint16)color);
	} else if (format.bytesPerPixel == 4) {
		uint32 *ptr = (uint32 *)getBasePtr(x, y);
		Common::set_to(ptr, ptr + (x2 - x + 1), color);
	} else {
		error("Surface::hLine: bytesPerPixel must be 1, 2, or 4");
	}
}

void Surface::vLine(int x, int y, int y2, uint32 color) {
	// Clipping
	if (x < 0 || x >= w)
		return;

	if (y2 < y)
		SWAP(y2, y);

	if (y < 0)
		y = 0;
	if (y2 >= h)
		y2 = h - 1;

	if (format.bytesPerPixel == 1) {
		byte *ptr = (byte *)getBasePtr(x, y);
		while (y++ <= y2) {
			*ptr = (byte)color;
			ptr += pitch;
		}
	} else if (format.bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)getBasePtr(x, y);
		while (y++ <= y2) {
			*ptr = (uint16)color;
			ptr += pitch / 2;
		}

	} else if (format.bytesPerPixel == 4) {
		uint32 *ptr = (uint32 *)getBasePtr(x, y);
		while (y++ <= y2) {
			*ptr = color;
			ptr += pitch / 4;
		}
	} else {
		error("Surface::vLine: bytesPerPixel must be 1, 2, or 4");
	}
}

void Surface::fillRect(Common::Rect r, uint32 color) {
	r.clip(w, h);

	if (!r.isValidRect())
		return;

	int width = r.width();
	int lineLen = width;
	int height = r.height();
	bool useMemset = true;

	if (format.bytesPerPixel == 2) {
		lineLen *= 2;
		if ((uint16)color != ((color & 0xff) | (color & 0xff) << 8))
			useMemset = false;
	} else if (format.bytesPerPixel == 4) {
		useMemset = false;
	} else if (format.bytesPerPixel != 1) {
		error("Surface::fillRect: bytesPerPixel must be 1, 2, or 4");
	}

	if (useMemset) {
		byte *ptr = (byte *)getBasePtr(r.left, r.top);
		while (height--) {
			memset(ptr, (byte)color, lineLen);
			ptr += pitch;
		}
	} else {
		if (format.bytesPerPixel == 2) {
			uint16 *ptr = (uint16 *)getBasePtr(r.left, r.top);
			while (height--) {
				Common::set_to(ptr, ptr + width, (uint16)color);
				ptr += pitch / 2;
			}
		} else {
			uint32 *ptr = (uint32 *)getBasePtr(r.left, r.top);
			while (height--) {
				Common::set_to(ptr, ptr + width, color);
				ptr += pitch / 4;
			}
		}
	}
}

void Surface::frameRect(const Common::Rect &r, uint32 color) {
	hLine(r.left, r.top, r.right - 1, color);
	hLine(r.left, r.bottom - 1, r.right - 1, color);
	vLine(r.left, r.top, r.bottom - 1, color);
	vLine(r.right - 1, r.top, r.bottom - 1, color);
}

void Surface::move(int dx, int dy, int height) {
	// Short circuit check - do we have to do anything anyway?
	if ((dx == 0 && dy == 0) || height <= 0)
		return;

	if (format.bytesPerPixel != 1 && format.bytesPerPixel != 2 && format.bytesPerPixel != 4)
		error("Surface::move: bytesPerPixel must be 1, 2, or 4");

	byte *src, *dst;
	int x, y;

	// vertical movement
	if (dy > 0) {
		// move down - copy from bottom to top
		dst = (byte *)pixels + (height - 1) * pitch;
		src = dst - dy * pitch;
		for (y = dy; y < height; y++) {
			memcpy(dst, src, pitch);
			src -= pitch;
			dst -= pitch;
		}
	} else if (dy < 0) {
		// move up - copy from top to bottom
		dst = (byte *)pixels;
		src = dst - dy * pitch;
		for (y = -dy; y < height; y++) {
			memcpy(dst, src, pitch);
			src += pitch;
			dst += pitch;
		}
	}

	// horizontal movement
	if (dx > 0) {
		// move right - copy from right to left
		dst = (byte *)pixels + (pitch - format.bytesPerPixel);
		src = dst - (dx * format.bytesPerPixel);
		for (y = 0; y < height; y++) {
			for (x = dx; x < w; x++) {
				if (format.bytesPerPixel == 1) {
					*dst-- = *src--;
				} else if (format.bytesPerPixel == 2) {
					*(uint16 *)dst = *(const uint16 *)src;
					src -= 2;
					dst -= 2;
				} else if (format.bytesPerPixel == 4) {
					*(uint32 *)dst = *(const uint32 *)src;
					src -= 4;
					dst -= 4;
				}
			}
			src += pitch + (pitch - dx * format.bytesPerPixel);
			dst += pitch + (pitch - dx * format.bytesPerPixel);
		}
	} else if (dx < 0)  {
		// move left - copy from left to right
		dst = (byte *)pixels;
		src = dst - (dx * format.bytesPerPixel);
		for (y = 0; y < height; y++) {
			for (x = -dx; x < w; x++) {
				if (format.bytesPerPixel == 1) {
					*dst++ = *src++;
				} else if (format.bytesPerPixel == 2) {
					*(uint16 *)dst = *(const uint16 *)src;
					src += 2;
					dst += 2;
				} else if (format.bytesPerPixel == 4) {
					*(uint32 *)dst = *(const uint32 *)src;
					src += 4;
					dst += 4;
				}
			}
			src += pitch - (pitch + dx * format.bytesPerPixel);
			dst += pitch - (pitch + dx * format.bytesPerPixel);
		}
	}
}

} // End of namespace Graphics
