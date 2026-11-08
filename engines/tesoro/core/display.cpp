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
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Tesoro {

Display::Display() {
	_frame.resize(kWidth * kHeight);
}

void Display::queueImage(int32 x, int32 y, uint16 depth, const AnimImage &image, uint32 extra) {
	_queue.addSprite(x, y, depth, image, extra);
}

void Display::queueImageScaled(int32 x, int32 y, uint16 depth, const AnimImage &image, int32 scaleX, int32 scaleY, uint32 extra) {
	if (_scaled.size() >= kMaxScaled) {
		warning("Display: too many scaled images in one frame");
		return;
	}

	Scaled item = { &image, x, y, depth, scaleX, scaleY, extra };
	_scaled.push_back(item);
}

void Display::fadeOut(int steps, int delayMs, Graphics::Screen *screen) {
	if (steps <= 0) {
		warning("Display: a fade needs at least one step");
		return;
	}

	const Common::Array<uint16> saved = _frame;
	const int step = 0x100 / steps;
	if (step == 0) {
		warning("Display: too many fade steps");
		return;
	}

	for (int factor = 0x100 - step; factor >= 0; factor -= step) {
		uint16 red[64], green[64], blue[64];
		for (uint i = 0; i < 64; ++i) {
			const uint16 scaled = (uint16)((i * factor) >> 8);
			red[i] = (uint16)(scaled << 11);
			green[i] = (uint16)(scaled << 5);
			blue[i] = scaled;
		}

		const uint32 start = g_system->getMillis();
		for (uint i = 0; i < _frame.size(); ++i) {
			const uint16 p = saved[i];
			_frame[i] = red[(p >> 11) & 0x1f] | blue[p & 0x1f] | green[(p >> 5) & 0x3f];
		}
		screen->copyRectToSurface(&_frame[0], kWidth * 2, 0, 0, kWidth, kHeight);
		screen->addDirtyRect(Common::Rect(kWidth, kHeight));
		screen->update();
		while (g_system->getMillis() - start < (uint32)delayMs)
			g_system->delayMillis(2);
	}
}

void Display::setShadeTable(const byte *table) {
	_shades.resize(0x8000);
	memcpy(&_shades[0], table, 0x8000);
}

void Display::startFrame() {
	_queue.reset();
	_texts.clear();
	_scaled.clear();
	_layerDepth = 0;
}

void Display::compose() {
	// The scaled images are put on in order from the deepest to the nearest.
	for (uint i = 0; i + 1 < _scaled.size(); ++i) {
		uint deepest = i;
		for (uint j = i + 1; j < _scaled.size(); ++j) {
			if (_scaled[deepest].depth < _scaled[j].depth)
				deepest = j;
		}
		SWAP(_scaled[i], _scaled[deepest]);
	}

	// Those behind the layer depth go on first, under the rows still to be composed.
	uint drawn = 0;
	while (drawn < _scaled.size() && _layerDepth < _scaled[drawn].depth) {
		const Scaled &s = _scaled[drawn++];
		_queue.rasterScaledImage(&_frame[0], s.x, s.y, s.depth, *s.image, s.scaleX, s.scaleY, s.extra);
	}

	uint16 *out = &_frame[0];
	uint16 *const end = out + _frame.size();

	for (uint y = 0; y < kHeight; ++y) {
		// walk row by row painting each span into the output frame buffer
		const RenderQueue::Row &row = _queue.row(y);
		for (int n = row.head(); n; n = row.span(n).next) {
			const RenderQueue::Span &span = row.span(n);

			if (span.x0 == -1) {
				// A shadow span: x1 is its number of pixels and the colour is the page of the table.
				const uint count = (uint16)span.x1;
				if (out + count > end || _shades.empty()) {
					warning("Display: spans run past the frame");
					return;
				}
				const byte *pixels = (const byte *)span.pixels;
				for (uint i = 0; i < count; ++i)
					out[i] = RenderQueue::shade(&_shades[0], span.colour, READ_LE_UINT16(pixels + 2 * i));
				out += count;
				continue;
			}

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

	// The rest go on top of the finished rows.
	for (; drawn < _scaled.size(); ++drawn) {
		const Scaled &s = _scaled[drawn];
		_queue.rasterScaledImage(&_frame[0], s.x, s.y, s.depth, *s.image, s.scaleX, s.scaleY, s.extra);
	}

	// The texts go over everything.
	_texts.draw(&_frame[0]);
}

} // End of namespace Tesoro
