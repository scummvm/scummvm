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
 */

#include "graphics/scaler/dotmatrix.h"
#include "graphics/scaler.h"

DotMatrixScaler::DotMatrixScaler(const Graphics::PixelFormat &format) : Scaler(format) {
	_factor = 2;

	if (format.bytesPerPixel == 2) {
		uint16 *lookup16 = (uint16 *)lookup;
		lookup16[0] = lookup16[10] = format.RGBToColor(0, 63, 0);
		lookup16[1] = lookup16[11] = format.RGBToColor(0, 0, 63);
		lookup16[2] = lookup16[8] = format.RGBToColor(63, 0, 0);
		lookup16[4] = lookup16[6] =
			lookup16[12] = lookup16[14] = format.RGBToColor(63, 63, 63);
		lookup16[3] = lookup16[5] = lookup16[7] =
			lookup16[9] = lookup16[13] =
			lookup16[15] = lookup16[16] = format.RGBToColor(0, 0, 0);
	} else {
		uint32 *lookup32 = (uint32 *)lookup;
		lookup32[0] = lookup32[10] = format.ARGBToColor(0, 0, 63, 0);
		lookup32[1] = lookup32[11] = format.ARGBToColor(0, 0, 0, 63);
		lookup32[2] = lookup32[8] = format.ARGBToColor(0, 63, 0, 0);
		lookup32[4] = lookup32[6] =
			lookup32[12] = lookup32[14] = format.ARGBToColor(0, 63, 63, 63);
		lookup32[3] = lookup32[5] = lookup32[7] =
			lookup32[9] = lookup32[13] =
			lookup32[15] = lookup32[16] = format.ARGBToColor(0, 0, 0, 0);
	}
}

void DotMatrixScaler::scaleIntern(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
	if (_format.bytesPerPixel == 2) {
		scaleIntern<uint16>(srcPtr, srcPitch, dstPtr, dstPitch, width, height, x, y);
	} else {
		scaleIntern<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height, x, y);
	}
}

uint DotMatrixScaler::increaseFactor() {
	return _factor;
}

uint DotMatrixScaler::decreaseFactor() {
	return _factor;
}

template<typename Pixel>
static inline Pixel DOT(const Pixel *dotmatrix, Pixel c, int j, int i) {
	return c - ((c >> 2) & dotmatrix[((j & 3) << 2) + (i & 3)]);
}

template<typename Pixel>
void DotMatrixScaler::scaleIntern(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
					int width, int height, int x, int y) {

	const Pixel *dotmatrix = (Pixel *)lookup;

	const uint32 nextlineSrc = srcPitch / sizeof(Pixel);
	const Pixel *p = (const Pixel *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(Pixel);
	Pixel *q = (Pixel *)dstPtr;

	int ja = (y * 2) & 3;
	int ia = (x * 2) & 3;

	for (int j = 0, jj = 0; j < height; ++j, jj += 2) {
		for (int i = 0, ii = 0; i < width; ++i, ii += 2) {
			Pixel c = *(p + i);
			*(q + ii) = DOT<Pixel>(dotmatrix, c, jj + ja, ii + ia);
			*(q + ii + 1) = DOT<Pixel>(dotmatrix, c, jj + ja, ii + ia + 1);
			*(q + ii + nextlineDst) = DOT<Pixel>(dotmatrix, c, jj + ja + 1, ii + ia);
			*(q + ii + nextlineDst + 1) = DOT<Pixel>(dotmatrix, c, jj + ja + 1, ii + ia+ 1);
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}


class DotMatrixPlugin final : public ScalerPluginObject {
public:
	DotMatrixPlugin();

	Scaler *createInstance(const Graphics::PixelFormat &format) const override;

	bool canDrawCursor() const override { return false; }
	uint extraPixels() const override { return 0; }
	const char *getName() const override;
	const char *getPrettyName() const override;
};

DotMatrixPlugin::DotMatrixPlugin() {
	_factors.push_back(2);
}

Scaler *DotMatrixPlugin::createInstance(const Graphics::PixelFormat &format) const {
	return new DotMatrixScaler(format);
}

const char *DotMatrixPlugin::getName() const {
	return "dotmatrix";
}

const char *DotMatrixPlugin::getPrettyName() const {
	return "DotMatrix";
}

REGISTER_PLUGIN_STATIC(DOTMATRIX, PLUGIN_TYPE_SCALER, DotMatrixPlugin);
