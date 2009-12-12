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
 * $URL$
 * $Id$
 */

#include "teenagent/surface.h"
#include "teenagent/pack.h"
#include "common/stream.h"
#include "common/debug.h"

namespace TeenAgent {

Surface::Surface() : x(0), y(0) {
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
	create(w_, h_, 1);

	stream->read(pixels, w_ * h_);
}

Common::Rect Surface::render(Graphics::Surface *surface, int dx, int dy, bool mirror, Common::Rect src_rect, uint zoom) const {
	if (src_rect.isEmpty()) {
		src_rect = Common::Rect(0, 0, w, h);
	} else if (src_rect.right > w)
		src_rect.right = w;
	else if (src_rect.bottom > h) 
		src_rect.bottom = h;

	if (zoom == 256) {
		assert(x + dx + src_rect.width() <= surface->w);
		assert(y + dy + src_rect.height() <= surface->h);

		byte *src = (byte *)getBasePtr(src_rect.left, src_rect.top);
		byte *dst = (byte *)surface->getBasePtr(dx + x, dy + y);

		for (int i = src_rect.top; i < src_rect.bottom; ++i) {
			for (int j = 0; j < src_rect.width(); ++j) {
				byte p = src[j];
				if (p != 0xff)
					dst[(mirror? src_rect.width() - j - 1: j)] = p;
			}
			dst += surface->pitch;
			src += pitch;
		}
	} else {
		Common::Rect dst_rect(src_rect);
		dst_rect.right = (dst_rect.width() * zoom / 256) + dst_rect.left;
		dst_rect.top = dst_rect.bottom - (dst_rect.height() * zoom / 256);

		assert(x + dx + dst_rect.width() <= surface->w);
		assert(y + dy + dst_rect.height() <= surface->h);

		byte *dst = (byte *)surface->getBasePtr(dx + x, dy + y + dst_rect.top - src_rect.top);
		for(int i = 0; i < dst_rect.height(); ++i) {
			for (int j = 0; j < dst_rect.width(); ++j) {
				int px = j * 256 / zoom;
				byte *src = (byte *)getBasePtr(src_rect.left + (mirror? w - px - 1: px), src_rect.top + i * 256 / zoom);
				byte p = *src;
				if (p != 0xff)
					dst[j + dst_rect.left] = p;
			}
			dst += surface->pitch;
		}
	}
	return Common::Rect(x + dx, y + dy, x + w + dx, y + h + dy);
}

} // End of namespace TeenAgent
