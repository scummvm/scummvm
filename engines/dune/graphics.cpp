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

#include "dune/hsq.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/palette.h"

namespace Dune {

Graphics::Graphics(DuneEngine *engine) :
	_engine(engine),
	_global_y_offset(0)
{}

void Graphics::fadePaletteRange(const byte *target, byte *current, uint first, uint count, uint remaining) {
	if (!remaining || first >= 256 || count > 256 - first)
		return;
	for (uint i = 3 * first; i < 3 * (first + count); i++)
		current[i] += ((int)target[i] - (int)current[i]) / (int)remaining;
}

void Graphics::scrollEndingPage(const byte *previous, const byte *next, byte *display, uint rows) {
	if (rows > 152)
		return;
	const uint entered = rows * 320;
	memcpy(display, next + 48640 - entered, entered);
	memcpy(display + entered, previous, 48640 - entered);
	memcpy(display + 48640, previous + 48640, 15360);
}

void Graphics::fadePaletteFrame(const byte *previous, const byte *next, byte *current, uint frame) {
	if (frame >= 132)
		return;
	const bool fadingIn = frame >= 66;
	const uint phase = fadingIn ? frame - 66 : frame;
	const uint round = phase / 3;
	const uint chunk = phase % 3;
	for (uint i = 0; i < 765; i++) {
		const int steps = round + (i / 255 <= chunk ? 1 : 0);
		// 272E swaps colours 1..254 between the pending and old palettes;
		// the first colour is not swapped (normally black in both banks).
		const int origin = fadingIn ? (i < 3 ? previous[i] : next[i]) :
				(i < 3 ? next[i] : previous[i]);
		const int value = origin - 3 * (fadingIn ? 22 - steps : steps);
		current[i] = value > 0 ? value : 0;
	}
	memcpy(current + 765, previous + 765, 3);
}

static void orderedDissolve(const byte *target, byte *display, uint phase, uint height) {
	// The DNVGA 2FD7 table gives byte offsets inside a 4x4 block. Its first
	// sixteen passes clear the old VGA page; the second sixteen copy the new
	// page at the same positions, after the pending palette is installed.
	static const uint16 offsets[] = {
		321, 960, 643, 2, 320, 962, 0, 641,
		3, 961, 322, 963, 642, 1, 323, 640
	};
	if (phase >= 32)
		return;
	const uint offset = offsets[phase & 15];
	for (uint y = 0; y < height; y += 4) {
		for (uint x = 0; x < 320; x += 4) {
			const uint at = y * 320 + x + offset;
			display[at] = phase < 16 ? 0 : target[at];
		}
	}
}

void Graphics::dissolveMapPage(const byte *target, byte *display, uint phase) {
	// Effect 34 enters at 2DC0, replacing the default height with 200.
	orderedDissolve(target, display, phase, 200);
}

void Graphics::dissolveScenePage(const byte *target, byte *display, uint phase) {
	// Effect 10 enters at 2DC3 with 25E7's 152-row scene height.
	orderedDissolve(target, display, phase, 152);
}

void Graphics::dissolveRoomConversion(const byte *target, byte *display, uint phase) {
	// 2E66..2EE6 spirals around an 8x8 cell. The repeated 320 entry is
	// present in the driver: do not remove its extra paint/wait frame.
	static const uint16 offsets[] = {
		2240, 2241, 2242, 2243, 2244, 2245, 2246, 2247,
		1927, 1607, 1287, 967, 647, 327, 7, 6, 5, 4, 3, 2, 1, 0,
		320, 320, 640, 960, 1280, 1600, 1920,
		1921, 1922, 1923, 1924, 1925, 1926, 1606, 1286, 966, 646,
		326, 325, 324, 323, 322, 321, 641, 961, 1281, 1601,
		1602, 1603, 1604, 1605, 1285, 965, 645, 644, 643, 642,
		962, 1282, 1283, 1284, 964, 963
	};
	if (phase >= 2 * ARRAYSIZE(offsets))
		return;
	const bool copying = phase >= ARRAYSIZE(offsets);
	const uint offset = offsets[copying ? phase - ARRAYSIZE(offsets) : ARRAYSIZE(offsets) - 1 - phase];
	for (uint y = 0; y < 152; y += 8)
		for (uint x = 0; x < 320; x += 8) {
			const uint at = y * 320 + x + offset;
			display[at] = copying ? target[at] : 0;
		}
}

void Graphics::pixelateNotice(const byte *source, byte *dest, uint block, uint phase) {
	static const uint16 samples2[] = { 0, 321, 1, 320 };
	static const uint16 samples4[] = {
		321, 960, 643, 2, 320, 962, 0, 641,
		3, 961, 322, 963, 642, 1, 323, 640
	};
	static const uint16 samples8[] = {
		963, 1600, 1285, 644, 960, 1604, 640, 1283,
		645, 1603, 964, 1605, 1284, 643, 965, 1282
	};
	if (block != 2 && block != 4 && block != 8)
		return;
	const uint offset = block == 2 ? samples2[phase % 4] :
		(block == 4 ? samples4[phase % 16] : samples8[phase % 16]);
	for (uint y = 0; y < 152; y += block) {
		for (uint x = 0; x < 320; x += block) {
			const byte sample = source[y * 320 + x + offset];
			for (uint row = 0; row < block; row++)
				memset(dest + (y + row) * 320 + x, sample, block);
		}
	}
}

Common::Rect Graphics::globeMapZoomBounds(const Common::Point &anchor, const Common::Rect &limits, uint phase) {
	assert(phase < 8);
	const int step = phase + 1;
	return Common::Rect(MAX<int>(limits.left, anchor.x - 4 * step),
		MAX<int>(limits.top, anchor.y - 2 * step),
		MIN<int>(limits.right, anchor.x + 2 + 4 * step),
		MIN<int>(limits.bottom, anchor.y + 1 + 2 * step));
}

bool Graphics::pixelateGlobeMapBox(const byte *clean, byte *display, const Common::Rect &bounds, uint sample) {
	assert(clean != display && bounds.left >= 0 && bounds.top >= 0 &&
		bounds.right <= 320 && bounds.bottom <= 200 && bounds.width() >= 2);
	if (sample == 0) {
		// C551/C560 paints exclusive rectangle edges with ink 7.
		memset(display + bounds.top * 320 + bounds.left, 7, bounds.width());
		memset(display + (bounds.bottom - 1) * 320 + bounds.left, 7, bounds.width());
		for (int y = bounds.top; y < bounds.bottom; y++) {
			display[y * 320 + bounds.left] = 7;
			display[y * 320 + bounds.right - 1] = 7;
		}
	}
	// BD00 excludes the outline; 3581 truncates both dimensions to pairs.
	const int rows = (bounds.height() - 2) / 2;
	const int columns = (bounds.width() - 2) / 2;
	if (rows <= 0 || columns <= 0)
		return false;
	static const uint offsets[] = { 0, 321, 1, 320 };
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < columns; x++) {
			const uint at = (bounds.top + 1 + y * 2) * 320 + bounds.left + 1 + x * 2;
			const byte value = clean[at + offsets[sample & 3]];
			display[at] = display[at + 1] = display[at + 320] = display[at + 321] = value;
		}
	}
	return true;
}

void Graphics::revealGlobeMapFrame(const byte *target, byte *display, uint phase) {
	assert(target != display && phase < 74);
	int at = 74 * 320 + 156;
	// 2AB0 copies four rows, then adjusts DI to the next tile. Each chunk
	// has one timer wait, not a wait for every tile. Its last chunk is not
	// a complete scene: C108/C4CD copies the full target afterwards.
	for (uint chunk = 0; chunk <= phase; chunk++) {
		int count, step;
		if (chunk == 0) {
			count = 1; step = 8;
		} else if (chunk == 1) {
			count = 2; step = 1280;
		} else {
			const uint part = (chunk - 2) & 3;
			count = 2 + ((chunk - 2) / 4) * 2 + (part >= 2);
			static const int starts[] = { -1288, -1272, 1288, 1272 };
			static const int steps[] = { -8, -1280, 8, 1280 };
			at += starts[part];
			step = steps[part];
		}
		if (chunk == phase) {
			for (int tile = 0; tile < count; tile++) {
				const int begin = at + tile * step;
				assert(begin >= 0 && begin + 968 <= 64000);
				for (int row = 0; row < 4; row++)
					memcpy(display + begin + row * 320, target + begin + row * 320, 8);
			}
		}
		at += count * step;
	}
}

void Graphics::rollPanelMenuFrame(const byte *oldPage, const byte *target, byte *display, uint phase) {
	if (phase >= 17)
		return;
	const int step = 17 - phase;
	const byte *source = step >= 9 ? oldPage : target;
	if (step == 9) {
		// 3280 fills forty rows, leaving the final row for the next frame.
		for (int row = 159; row < 199; row++) {
			for (int x = 0; x < 136; x++)
				display[row * 320 + 92 + x] = row < 175 || row >= 183 ? 0xfe :
					((x + row - 175) & 1) ? 0xf2 : 0x08;
		}
		return;
	}
	static const uint16 ratios[] = {
		0, 0x0111, 0x0107, 0x0104, 0x0102, 0x0203, 0x0504, 0x0201, 0x0501,
		0, 0x0501, 0x0201, 0x0504, 0x0203, 0x0102, 0x0104, 0x0107, 0x0111
	};
	int lower = 179 * 320 + 92, upper = lower - 320;
	memcpy(display + lower, source + lower, 136);
	lower += 320;
	int fromLower = lower, fromUpper = upper;
	const int repeat = ratios[step] & 255, skip = ratios[step] >> 8;
	int copies = repeat, remaining = 20;
	while (--remaining >= 0) {
		memcpy(display + lower, source + fromLower, 136);
		lower += 136; fromLower += 136;
		memcpy(display + upper, source + fromUpper, 136);
		upper += 136; fromUpper += 136;
		if (--copies == 0) {
			remaining -= skip;
			if (remaining <= 0) {
				lower -= 456;
				upper += 184;
				break;
			}
			fromLower -= skip * 320;
			fromUpper += skip * 320;
			copies = repeat;
		}
		lower -= 456; fromLower -= 456;
		upper += 184; fromUpper += 184;
	}
	// 3330's two alternating stripe words switch at the midpoint.
	uint16 first = step < 9 ? 0xf208 : 0xfefe;
	uint16 second = step < 9 ? 0xfefe : 0xf208;
	int rows = step < 9 ? step : 18 - step;
	auto fill = [&](uint16 word, int at) {
		for (int x = 0; x < 136; x += 2) {
			display[at + x] = word & 255;
			display[at + x + 1] = word >> 8;
		}
	};
	while (rows--) {
		fill(second, upper); fill(first, lower);
		first = (first << 8) | (first >> 8);
		second = (second << 8) | (second >> 8);
		lower -= 320; upper += 320;
	}
	while (lower >= 159 * 320) {
		fill(0xfefe, upper); fill(0xfefe, lower);
		lower -= 320; upper += 320;
	}
	if (step == 1) {
		for (int row = 159; row < 200; row++)
			memcpy(display + row * 320 + 92, target + row * 320 + 92, 136);
	}
}

bool Graphics::revealScene(const byte *target, byte *display, uint16 &sequence, uint pairs) {
	assert(sequence > 0 && sequence < 0x8000);
	for (uint sample = 0; sample < pairs; sample++) {
		display[sequence] = target[sequence];
		// MOVSB advances SI before the driver's ADD SI,7FFE.
		const uint second = sequence + 0x7fff;
		if (second < 320 * 152)
			display[second] = target[second];
		const bool odd = sequence & 1;
		sequence >>= 1;
		if (odd)
			sequence ^= 0x4400;
		if (sequence == 1) {
			display[0] = target[0];
			return true;
		}
	}
	return false;
}

/** DNVGA 3733: inclusive horizontal edges and its asymmetric bottom row. */
static void xorWindowOutline(byte *display, int movingLeft, int movingTop, int width, int height) {
	const int left = CLIP(movingLeft, 4, 316), top = CLIP(movingTop, 4, 148);
	const int right = CLIP(movingLeft + width, 4, 316), bottom = CLIP(movingTop + height, 4, 148);
	// 3733 paints both horizontal edges inclusively. Its vertical loop is
	// bottom-top-2 rows, leaving the bottom edge at bottom-1. For a height
	// <=2 both horizontal edges overlap and cancel; a one-column outline
	// likewise cancels its two vertical sides.
	const int rows = MAX(bottom - top - 2, 0);
	for (int at = left; at <= right; at++) {
		display[top * 320 + at] ^= 0x0f;
		display[(top + (rows ? rows + 1 : 0)) * 320 + at] ^= 0x0f;
	}
	for (int row = 1; row <= rows; row++) {
		display[(top + row) * 320 + left] ^= 0x0f;
		display[(top + row) * 320 + right] ^= 0x0f;
	}
}

void Graphics::xorMapWindowFrame(byte *display, const Common::Point &anchor,
		const Common::Rect &bounds, uint phase, bool closing) {
	assert(phase >= 1 && phase <= 15);
	// 5F25 supplies anchor-10; the driver adds eight. Signed shifts in
	// 38FB use magnitude then sign, i.e. division truncating toward zero.
	const int x = anchor.x - 2, y = anchor.y - 2;
	// 39BB starts from the FULL target, negating the cached steps. It is
	// not the opening sequence in reverse: division leaves remainders.
	const int step = closing ? -(int)phase : (int)phase;
	xorWindowOutline(display,
		(closing ? bounds.left : x) + (bounds.left - x) / 16 * step,
		(closing ? bounds.top : y) + (bounds.top - y) / 16 * step,
		(closing ? bounds.width() : 0) + bounds.width() / 16 * step,
		(closing ? bounds.height() : 0) + bounds.height() / 16 * step);
}

void Graphics::xorTroopWindowFrame(byte *display, const Common::Point &anchor,
		const Common::Rect &bounds, uint phase, bool closing) {
	assert(phase < 32 && bounds.width() >= 20 && bounds.height() >= 20);
	assert(bounds.left >= 0 && bounds.top >= 0 && bounds.right <= 320 && bounds.bottom <= 200);
	const int x = CLIP<int>(anchor.x, 0, 300), y = MAX<int>(anchor.y, 0);
	const int halfWidth = (bounds.width() - 20) / 2;
	const int halfHeight = (bounds.height() - 20) / 2;
	const int centreX = bounds.left + halfWidth, centreY = bounds.top + halfHeight;
	const int moveX = (centreX - x) / 8, moveY = (centreY - y) / 8;
	const int step = phase & 7;
	// Both halves have two identical accumulating passes. The second pass
	// removes the first, rather than restoring each outline before advancing.
	if ((!closing && phase < 16) || (closing && phase >= 16)) {
		xorWindowOutline(display,
			closing ? centreX - moveX * (step + 1) : x + moveX * step,
			closing ? centreY - moveY * (step + 1) : y + moveY * step, 20, 20);
		return;
	}
	// 37B1 paints ten pixels along each horizontal corner, and nine along
	// each vertical corner. 3841 starts from the LAST growth step (7), not
	// the full filled report. Keep division remainders exactly as the driver.
	const int growth = closing ? 7 - step : step;
	const int growX = halfWidth / 8 * growth, growY = halfHeight / 8 * growth;
	const int left = centreX - growX, top = centreY - growY;
	const int right = left + 20 + growX * 2 - 1, bottom = top + 20 + growY * 2 - 1;
	for (int i = 0; i < 10; i++) {
		display[top * 320 + left + i] ^= 0x0f;
		display[top * 320 + right - i] ^= 0x0f;
		display[bottom * 320 + left + i] ^= 0x0f;
		display[bottom * 320 + right - i] ^= 0x0f;
	}
	for (int i = 1; i < 10; i++) {
		display[(top + i) * 320 + left] ^= 0x0f;
		display[(top + i) * 320 + right] ^= 0x0f;
		display[(bottom - i) * 320 + left] ^= 0x0f;
		display[(bottom - i) * 320 + right] ^= 0x0f;
	}
}

void Graphics::distortNotice(const byte *source, byte *dest, uint16 &row, uint16 &previous) {
	// Driver 3487..34FA, followed by the 80 sentinel. Unlike a full-page
	// redraw, this retains the overhangs until the wave's next restoring row.
	static const int8 offsets[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3,
		3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, -1, -1,
		-1, -2, -2, -2, -2, -3, -3, -3, -3, -3, -4, -4, -4, -4, -4, -4,
		-5, -5, -5, -5, -5, -5, -5, -5, -5, -4, -4, -4, -4, -4, -4, -3,
		-3, -3, -3, -3, -3, -2, -2, -2, -2, -2, -2, -2, -1, -1, -1, -1,
		-1, -1, -1, -1
	};
	assert(source != dest && row <= 148);
	assert(previous == 5 || (previous >= 8 && previous <= 47048 && (previous - 8) % 320 == 0));
	uint y = row++;
	// The original increments before checking the bottom. Thus row zero
	// occurs twice on wrapping, once with the stored next row still zero.
	if (y >= 148)
		y = row = 0;
	memcpy(dest + previous - 5, source + previous - 5, 314);
	previous = y * 320 + 8;
	for (uint i = 0; i < ARRAYSIZE(offsets); i++) {
		const uint at = y * 320 + 8;
		memcpy(dest + at + offsets[i], source + at, 304);
		if (++y == 148)
			y = 0;
	}
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

	// The pointer is sixteen square, and what hangs over the right or the
	// bottom of the page is left off: the two bounds had the screen's width
	// and height the wrong way about.
	int w = MIN(16, 320 - x);
	int h = MIN(16, 200 - y);

	byte *dst = _framebuffer + 320 * y + x;

	for (int cy = 0; cy != h; ++cy) {
		uint16_t mask_b = READ_LE_UINT16(cursorData+4+2*cy);
		uint16_t mask_w = READ_LE_UINT16(cursorData+4+2*cy+32);

		// The row the pointer is on, so that put() knows where it ends.
		getPixelPtr(0, y + cy - _global_y_offset);

		for (int cx = 0; cx != w; ++cx) {
			byte v = holds(dst) ? *dst : 0;
			if (!(mask_b & 0x8000)) {
				v = 0x00;
			}
			if (mask_w & 0x8000) {
				v = 0x0f;
			}
			mask_b <<= 1;
			mask_w <<= 1;
			put(dst, v);
			dst++;
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
	// A row off the page is left off it; put() drops what is written there.
	y += _global_y_offset;

	_rowFrom = _framebuffer + 320 * y;
	_rowTo = _rowFrom + 320;

	return _framebuffer + 320 * y + x;
}

void Graphics::blitGraphics(byte *buf, int buf_len, int x_offset, int y_offset, int w, int h, byte flags, byte mode) {
	Common::MemoryReadStream r(buf, buf_len);
	blitGraphics(&r, x_offset, y_offset, w, h, flags, mode);
}

void Graphics::draw_4bpp(bool flip_x, bool flip_y, Common::ReadStream *r, int x_offset, int y_offset, int w, int h, byte mode) {
	for (int y = 0; y < h; ++y) {
		const int row = y_offset + (flip_y ? h - 1 - y : y);
		// Packed rows are rounded up to four pixels, but that padding is not
		// part of the sprite. It must be consumed without painting either edge.
		const int rowPixels = 4 * ((w + 3) / 4);
		for (int x = 0; x < rowPixels; x += 2) {
			const byte value = r->readByte();
			for (int nibble = 0; nibble < 2; nibble++) {
				const int column = x + nibble;
				const byte pixel = (value >> (4 * nibble)) & 0x0f;
				if (column < w && pixel)
					putSpritePixel(x_offset + (flip_x ? w - 1 - column : column), row, pixel + mode);
			}
		}
	}
}

void Graphics::draw_4bpp_rle(bool flip_x, bool flip_y, Common::ReadStream *r, int x_offset, int y_offset, int w, int h, byte mode) {
	for (int y = 0; y != h; ++y) {
		const int row = y_offset + (flip_y ? h - 1 - y : y);
		const int rowPixels = 4 * ((w + 3) / 4);
		int x = 0;
		while (x < rowPixels) {
			const byte cmd = r->readByte();
			const bool repeat = (cmd & 0x80) != 0;
			const int count = repeat ? 257 - cmd : cmd + 1;
			const byte repeated = repeat ? r->readByte() : 0;
			for (int i = 0; i < count; i++) {
				const byte value = repeat ? repeated : r->readByte();
				for (int nibble = 0; nibble < 2; nibble++, x++) {
					const byte pixel = (value >> (4 * nibble)) & 0x0f;
					if (x < w && pixel)
						putSpritePixel(x_offset + (flip_x ? w - 1 - x : x), row, pixel + mode);
				}
			}
		}
	}
}

void Graphics::draw_8bpp(bool flip_x, bool flip_y, Common::ReadStream *r, int x_offset, int y_offset, int w, int h, byte mode) {
	for (int y = 0; y < h; ++y) {
		const int row = y_offset + (flip_y ? h - 1 - y : y);
		for (int x = 0; x != w; ++x) {
			const byte value = r->readByte();
			if (value || mode != 255)
				putSpritePixel(x_offset + (flip_x ? w - 1 - x : x), row, value);
		}
	}
}

void Graphics::draw_8bpp_rle(bool flip_x, bool flip_y, Common::ReadStream *r, int x_offset, int y_offset, int w, int h, byte mode) {
	for (int y = 0; y != h; ++y) {
		const int row = y_offset + (flip_y ? h - 1 - y : y);
		int x = 0;
		while (x < w) {
			const byte cmd = r->readByte();
			const bool repeat = (cmd & 0x80) != 0;
			const int count = repeat ? 257 - cmd : cmd + 1;
			const byte repeated = repeat ? r->readByte() : 0;
			for (int i = 0; i < count; i++, x++) {
				const byte value = repeat ? repeated : r->readByte();
				if (x < w && (value || mode != 255))
					putSpritePixel(x_offset + (flip_x ? w - 1 - x : x), row, value);
			}
		}
	}
}

void Graphics::blitGraphics(Common::ReadStream *r, int x_offset, int y_offset, int w, int h, byte flags, byte mode) {
	if (w <= 0 || h <= 0)
		return;

	// Only wholly invisible sprites can be skipped. Each decoder consumes
	// clipped rows normally and checks coordinates before writing a pixel.
	const int top = y_offset + _global_y_offset;
	if (x_offset >= 320 || top >= 200 || x_offset + w <= 0 || top + h <= 0)
		return;

	// 0xE5B reads the sprite's own flags: 0x40 turns it about the upright and
	// 0x20 about the level, which is the way round the two used to be, and the
	// level one was never drawn at all.
	bool flip_x = flags & 0x40;
	bool flip_y = flags & 0x20;

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

void Graphics::copyMapSprite(const byte *src, uint pitch, int x, int y, int width, int height,
		byte palette, const Common::Rect &clip, bool rle) {
	const int first = MAX(0, clip.left - x);
	const int last = MIN(width, clip.right - x);
	const int top = MAX(0, clip.top - y - _global_y_offset);
	const int bottom = MIN(height, clip.bottom - y - _global_y_offset);
	const bool bothSides = !rle && x < clip.left && x + width > clip.right;
	for (int row = top; row < bottom; row++) {
		for (int column = first; column < last; column++) {
			const byte packed = src[row * pitch + column / 2];
			const byte pixel = (packed >> (4 * (column & 1))) & 0x0f;
			// 179D..17A6 uses JZ where the right-only path uses JNZ. Thus
			// a transparent third nibble skips the fourth too, except when
			// the left-clipping prologue starts directly at that fourth one.
			if (bothSides && column > first && (column & 3) == 3 && !(packed & 0x0f))
				continue;
			if (pixel)
				putSpritePixel(x + column, y + row, pixel + palette);
		}
	}
}

void Graphics::copyScaled(
	byte *src, uint16 src_pitch,
	int x_offset, int y_offset,
	int dst_w, int dst_h, byte flags,
	byte pal_offset, uint16 src_delta_fp)
{
	if (dst_w <= 0 || dst_h <= 0)
		return;
	bool flip_x = !!(flags & 0x40);
	bool flip_y = !!(flags & 0x20);

	// Source coordinates in 8.8 fixed point
	uint32 src_x_fp = 0;
	uint32 src_y_fp = 0;

	for (int dst_y = 0; dst_y != dst_h; dst_y++, src_y_fp += src_delta_fp) {
		const int row = y_offset + (flip_y ? dst_h - 1 - dst_y : dst_y);
		const uint32 src_y = (src_y_fp >> 8);
		src_x_fp = 0;
		for (int dst_x = 0; dst_x != dst_w; dst_x++, src_x_fp += src_delta_fp) {
			const uint32 src_x = (src_x_fp >> 8);

			byte value = src[src_y * src_pitch + src_x / 2];
			if (src_x & 1) {
				value >>= 4;
			}
			value &= 0x0f;
			if (value != 0) {
				putSpritePixel(x_offset + (flip_x ? dst_w - 1 - dst_x : dst_x), row, value + pal_offset);
			}
		}
	}
}

void Graphics::drawLine(int x0, int y0, int x1, int y1, byte colour) {
	enum { kPictureWidth = 320, kPictureHeight = 152 };

	const int dx = ABS(x1 - x0);
	const int dy = -ABS(y1 - y0);
	const int sx = x0 < x1 ? 1 : -1;
	const int sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;

	for (;;) {
		if (x0 >= 0 && x0 < kPictureWidth && y0 >= 0 && y0 < kPictureHeight)
			put(getPixelPtr(x0, y0), colour);

		if (x0 == x1 && y0 == y1)
			break;

		const int e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
	}
}

void Graphics::fillRect(int x1, int y1, int x2, int y2, byte colour) {
	x1 = MAX(x1, 0);
	y1 = MAX(y1, 0);
	x2 = MIN(x2, 320);
	y2 = MIN(y2, 200);

	for (int y = y1; y < y2; y++) {
		byte *dst = getPixelPtr(x1, y);

		for (int x = x1; x < x2; x++)
			put(dst++, colour);
	}
}

uint16 Graphics::drawNoise(uint16 x, uint16 y, uint16 count, uint16 start_color, uint16 color_increment_fp,
						   uint16 xor_state, uint16 xor_increment, bool leftwards) {
	// A run has to stay on the screen: a scene read with the wrong bank can
	// name one that does not.
	if (count == 0 || x >= 320 || y >= 200)
		return xor_state;

	// A run drawn the other way about starts at its right hand end and walks
	// back, which is what STD at 0x3D00 is for; either way it stops at the
	// edge of the screen.
	count = MIN<uint16>(count, leftwards ? x + 1 : 320 - x);

	byte *dst = getPixelPtr(x, y);
	do {
		bool shouldXor = xor_state & 1;
		xor_state = xor_state >> 1;
		if (shouldXor) {
			xor_state = xor_state ^ xor_increment;
		}
		put(dst, (start_color >> 8) + (int8(xor_state & 3) - 1));
		dst += leftwards ? -1 : 1;
		start_color = start_color + color_increment_fp;
		count = count - 1;
	} while (count != 0);

	return xor_state;
}

} // namespace Dune
