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

#include "common/array.h"
#include "ultima/ultima2/data/data.h"
#include "ultima/ultima2/gfx/composite.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

namespace {

// For each possible window of 4 consecutive pixels (the first in the top
// two bits), the colors of the middle two. A middle pixel shows as a solid
// CGA color only if it matches a neighbor, otherwise as the "transition"
// color of the two middle pixels together. Built the same way as the
// driver's own table
byte g_leftColor[256];
byte g_rightColor[256];
bool g_tableReady = false;

void buildTable() {
	for (int w = 0; w < 256; ++w) {
		int p0 = (w >> 6) & 3, p1 = (w >> 4) & 3, p2 = (w >> 2) & 3, p3 = w & 3;
		int mid = (p1 << 2) | p2;

		g_leftColor[w] = (p1 == p0 || p1 == p2) ? (p1 << 2) | p1 : mid;
		g_rightColor[w] = (p3 == p2 || p1 == p2) ? (p2 << 2) | p2 : mid;
	}

	g_tableReady = true;
}

} // End of anonymous namespace

void compositeDecode(Graphics::ManagedSurface &surf, const Common::Rect &area) {
	if (!g_tableReady)
		buildTable();

	Common::Rect r = area;
	r.clip(Common::Rect(surf.w, surf.h));
	if (r.isEmpty())
		return;

	// The output overwrites the pixels it reads, so work from a copy
	int w = r.width(), h = r.height();
	Common::Array<byte> src(w * h);
	for (int y = 0; y < h; ++y)
		memcpy(&src[y * w], surf.getBasePtr(r.left, r.top + y), w);

	// Anything that isn't a CGA color doesn't take part in the decode
	auto cga = [&](int x, int y) -> int {
		byte v = src[y * w + x];
		return v <= 3 ? v : 0;
	};

	for (int y = 0; y < h; ++y) {
		byte *dst = (byte *)surf.getBasePtr(r.left, r.top + y);

		for (int x = 0; x < w; x += 2) {
			// The driver reads the byte after the end of a line from the start
			// of the line after next in the same CGA bank, rather than
			// treating it as blank
			int p0 = (x > 0) ? cga(x - 1, y) : 0;
			int p3 = (x + 2 < w) ? cga(x + 2, y) : ((y + 2 < h) ? cga(0, y + 2) : 0);
			int window = (p0 << 6) | (cga(x, y) << 4) | (cga(x + 1, y) << 2) | p3;

			if (src[y * w + x] <= 3)
				dst[x] = Data::COMPOSITE_PALETTE_BASE + g_leftColor[window];
			if (x + 1 < w && src[y * w + x + 1] <= 3)
				dst[x + 1] = Data::COMPOSITE_PALETTE_BASE + g_rightColor[window];
		}
	}

	surf.markAllDirty();
}

void compositeDecodeFrom(const byte *src, Graphics::ManagedSurface &dst, const Common::Rect &area) {
	if (!g_tableReady)
		buildTable();

	Common::Rect r = area;
	r.clip(Common::Rect(dst.w, dst.h));
	if (r.isEmpty())
		return;

	const int w = dst.w;
	auto cga = [&](int x, int y) -> int {
		return src[y * w + x] & 3;
	};

	for (int y = r.top; y < r.bottom; ++y) {
		byte *out = (byte *)dst.getBasePtr(0, y);

		// Pairs are aligned to even columns of the whole picture
		for (int x = r.left & ~1; x < r.right; x += 2) {
			int p0 = (x > 0) ? cga(x - 1, y) : 0;
			int p3 = (x + 2 < w) ? cga(x + 2, y) : ((y + 2 < dst.h) ? cga(0, y + 2) : 0);
			int window = (p0 << 6) | (cga(x, y) << 4) | (cga(x + 1, y) << 2) | p3;

			out[x] = Data::COMPOSITE_PALETTE_BASE + g_leftColor[window];
			out[x + 1] = Data::COMPOSITE_PALETTE_BASE + g_rightColor[window];
		}
	}

	dst.addDirtyRect(Common::Rect(r.left & ~1, r.top, MIN<int>(r.right + 1, w), r.bottom));
}

void compositeFinish(Graphics::ManagedSurface &surf, const Common::Rect &area) {
	if (g_engine->getRenderMode() == Data::RENDER_CGA_COMPOSITE)
		compositeDecode(surf, area);
}

void compositeFinish(Graphics::ManagedSurface &surf) {
	compositeFinish(surf, Common::Rect(surf.w, surf.h));
}

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima
