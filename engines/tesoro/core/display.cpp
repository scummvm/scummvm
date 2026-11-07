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

#include "tesoro/core/display.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Tesoro {

Display::Display() {
	_frame.resize(kWidth * kHeight);
}

void Display::queueImage(int32 x, int32 y, uint16 depth, const AnimImage &image, uint32 extra) {
	_queue.addSprite(x, y, depth, image, extra);
}

void Display::startFrame() {
	_queue.reset();
}

void Display::compose() {
	uint16 *out = &_frame[0];
	uint16 *const end = out + _frame.size();

	for (uint y = 0; y < kHeight; ++y) {
		// walk row by row painting each span into the output frame buffer
		const RenderQueue::Row &row = _queue.row(y);
		for (int n = row.head(); n; n = row.span(n).next) {
			const RenderQueue::Span &span = row.span(n);
			const uint count = (uint16)(span.x1 - span.x0 + 1);
			if (out + count > end) {
				warning("Display: spans run past the frame");
				return;
			}

			if (span.pixels == RenderQueue::kSolid) {
				for (uint i = 0; i < count; ++i)
					out[i] = span.colour;
			} else if (span.pixels != RenderQueue::kNoPixels) {
				const byte *pixels = (const byte *)span.pixels;
				for (uint i = 0; i < count; ++i)
					out[i] = READ_LE_UINT16(pixels + 2 * i);
			}
			out += count;
		}
	}
}

} // End of namespace Tesoro
