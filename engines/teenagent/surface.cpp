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

#include "teenagent/surface.h"
#include "teenagent/pack.h"
#include "common/stream.h"
#include "common/debug.h"

namespace TeenAgent {

Surface::Surface() : x(0), y(0) {
}

Surface::~Surface() {
	free();
}

void Surface::load(Common::SeekableReadStream *stream, Type type) {
	//debug(0, "load()");
	free();

	x = y = 0;

	uint16 w_ = stream->readUint16LE();
	uint16 h_ = stream->readUint16LE();

	if (type != kTypeLan) {
		uint16 pos = stream->readUint16LE();
		x = pos % 320;
		y = pos / 320;
	}

	//debug(0, "declared info: %ux%u (%04xx%04x) -> %u,%u", w_, h_, w_, h_, x, y);
	if (stream->eos() || w_ == 0)
		return;

	if (w_ * h_ > stream->size()) {
		debug(0, "invalid surface %ux%u -> %u,%u", w_, h_, x, y);
		return;
	}

	//debug(0, "creating surface %ux%u -> %u,%u", w_, h_, x, y);
	create(w_, h_, Graphics::PixelFormat::createFormatCLUT8());

	stream->read(pixels, w_ * h_);
}

Common::Rect Surface::render(Graphics::Surface *surface, int dx, int dy, bool mirror, Common::Rect src_rect, uint zoom) const {
	if (src_rect.isEmpty()) {
		src_rect = Common::Rect(0, 0, w, h);
	}
	Common::Rect dst_rect(x + dx, y + dy, x + dx + zoom * src_rect.width() / 256, y + dy + zoom * src_rect.height() / 256);
	if (dst_rect.left < 0) {
		src_rect.left = -dst_rect.left;
		dst_rect.left = 0;
	}
	if (dst_rect.right > surface->w) {
		src_rect.right -= dst_rect.right - surface->w;
		dst_rect.right = surface->w;
	}
	if (dst_rect.top < 0) {
		src_rect.top -= dst_rect.top;
		dst_rect.top = 0;
	}
	if (dst_rect.bottom > surface->h) {
		src_rect.bottom -= dst_rect.bottom - surface->h;
		dst_rect.bottom = surface->h;
	}
	if (src_rect.isEmpty() || dst_rect.isEmpty())
		return Common::Rect();

	if (zoom == 256) {
		const byte *src = (const byte *)getBasePtr(0, src_rect.top);
		byte *dst_base = (byte *)surface->getBasePtr(dst_rect.left, dst_rect.top);

		for (int i = src_rect.top; i < src_rect.bottom; ++i) {
			byte *dst = dst_base;
			for (int j = src_rect.left; j < src_rect.right; ++j) {
				byte p = src[(mirror? w - j - 1: j)];
				if (p != 0xff)
					*dst++ = p;
				else
					++dst;
			}
			dst_base += surface->pitch;
			src += pitch;
		}
	} else {
		byte *dst = (byte *)surface->getBasePtr(dst_rect.left, dst_rect.top);
		for(int i = 0; i < dst_rect.height(); ++i) {
			for (int j = 0; j < dst_rect.width(); ++j) {
				int px = j * 256 / zoom;
				const byte *src = (const byte *)getBasePtr(src_rect.left + (mirror? w - px - 1: px), src_rect.top + i * 256 / zoom);
				byte p = *src;
				if (p != 0xff)
					dst[j] = p;
			}
			dst += surface->pitch;
		}
	}
	return dst_rect;
}

} // End of namespace TeenAgent
