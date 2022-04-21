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

#include "mtropolis/render.h"
#include "mtropolis/runtime.h"

#include "graphics/surface.h"
#include "graphics/managed_surface.h"

namespace MTropolis {

template<class TNumber, int TResolution>
struct OrderedDitherGenerator {
	static void generateOrderedDither(TNumber (&pattern)[TResolution][TResolution]);
};

template<class TNumber>
struct OrderedDitherGenerator<TNumber, 1> {
	static void generateOrderedDither(TNumber (&pattern)[1][1]);
};

template<class TNumber, int TResolution>
void OrderedDitherGenerator<TNumber, TResolution>::generateOrderedDither(TNumber (&pattern)[TResolution][TResolution]) {
	const int kHalfResolution = TResolution / 2;
	byte halfRes[kHalfResolution][kHalfResolution];

	OrderedDitherGenerator<kHalfResolution>::generateOrderedDither(halfRes);

	const int kHalfResNumSteps = kHalfResolution * kHalfResolution;
	for (int y = 0; y < kHalfResolution; y++) {
		for (int x = 0; x < kHalfResolution; x++) {
			pattern[y * 2][x * 2] = halfRes[y][x];
			pattern[y * 2 + 1][x * 2 + 1] = halfRes[y][x] + kHalfResNumSteps * 1;
			pattern[y * 2][x * 2 + 1] = halfRes[y][x] + kHalfResNumSteps * 2;
			pattern[y * 2 + 1][x * 2] = halfRes[y][x] + kHalfResNumSteps * 3;
		}
	}
}

template<class TNumber>
void OrderedDitherGenerator<TNumber, 1>::generateOrderedDither(TNumber (&pattern)[1][1]) {
	pattern[0][0] = 0;
}

inline int quantize8To5(int value, byte orderedDither16x16) {
	return (value * 249 + (orderedDither16x16 << 3)) >> 11;
}

Window::Window(int32 x, int32 y, int16 width, int16 height, const Graphics::PixelFormat &format) : _x(x), _y(y) {
	_surface.reset(new Graphics::ManagedSurface(width, height, format));
}

Window::~Window() {
}

int32 Window::getX() const {
	return _x;
}

int32 Window::getY() const {
	return _y;
}

void Window::setPosition(int32 x, int32 y) {
	_x = x;
	_y = y;
}


const Common::SharedPtr<Graphics::ManagedSurface> &Window::getSurface() const {
	return _surface;
}

const Graphics::PixelFormat& Window::getPixelFormat() const {
	return _surface->format;
}

namespace Render {

uint32 resolveRGB(uint8 r, uint8 g, uint8 b, const Graphics::PixelFormat &fmt) {
	uint32 rPlaced = (static_cast<uint32>(r) >> (8 - fmt.rBits())) << fmt.rShift;
	uint32 gPlaced = (static_cast<uint32>(g) >> (8 - fmt.gBits())) << fmt.gShift;
	uint32 bPlaced = (static_cast<uint32>(b) >> (8 - fmt.bBits())) << fmt.bShift;
	uint32 aPlaced = (static_cast<uint32>(255) >> (8 - fmt.aBits())) << fmt.aShift;

	return rPlaced | gPlaced | bPlaced | aPlaced;
}

} // End of namespace Render

} // End of namespace MTropolis
