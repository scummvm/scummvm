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

#include "graphics/scaler/dotmatrix.h"
#include "graphics/scaler.h"

DotMatrixPlugin::DotMatrixPlugin() {
	_factor = 1;
	_factors.push_back(1);
	_factors.push_back(2);
}

void DotMatrixPlugin::initialize(Graphics::PixelFormat format) {
	lookup[0] = lookup[10] = format.RGBToColor(0, 63, 0);
	lookup[1] = lookup[11] = format.RGBToColor(0, 0, 63);
	lookup[2] = lookup[8] = format.RGBToColor(63, 0, 0);
	lookup[4] = lookup[6] =
		lookup[12] = lookup[14] = format.RGBToColor(63, 63, 63);
	lookup[5] = lookup[7] =
		lookup[9] = lookup[13] =
		lookup[15] = lookup[16] = format.RGBToColor(0, 0, 0);
	_format = format;
}

void DotMatrixPlugin::scale(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
	if (!_doScale) {
		scale1x(srcPtr, srcPitch, dstPtr, dstPitch, width, height, _format.bytesPerPixel);
		return;
	}
	switch (_factor) {
	case 1:
		scale1x(srcPtr, srcPitch, dstPtr, dstPitch, width, height, _format.bytesPerPixel);
		break;
	case 2:
		scaleIntern(srcPtr, srcPitch, dstPtr, dstPitch, width, height, x, y);
		break;
	}
}

uint DotMatrixPlugin::increaseFactor() {
	if (_factor < 2)
		++_factor;
	return _factor;
}

uint DotMatrixPlugin::decreaseFactor() {
	if (_factor > 1)
		--_factor;
	return _factor;
}

const char *DotMatrixPlugin::getName() const {
	return "DotMatrix";
}

static inline uint16 DOT_16(const uint16 *dotmatrix, uint16 c, int j, int i) {
	return c - ((c >> 2) & dotmatrix[((j & 3) << 2) + (i & 3)]);
}

void DotMatrixPlugin::scaleIntern(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
					int width, int height, int x, int y) {

	const uint16 *dotmatrix = lookup;

	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;

	int ja = (y * 2) & 3;
	int ia = (x * 2) & 3;

	for (int j = 0, jj = 0; j < height; ++j, jj += 2) {
		for (int i = 0, ii = 0; i < width; ++i, ii += 2) {
			uint16 c = *(p + i);
			*(q + ii) = DOT_16(dotmatrix, c, jj + ja, ii + ia);
			*(q + ii + 1) = DOT_16(dotmatrix, c, jj + ja, ii + ia + 1);
			*(q + ii + nextlineDst) = DOT_16(dotmatrix, c, jj + ja + 1, ii + ia);
			*(q + ii + nextlineDst + 1) = DOT_16(dotmatrix, c, jj + ja + 1, ii + ia+ 1);
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

REGISTER_PLUGIN_STATIC(DOTMATRIX, PLUGIN_TYPE_SCALER, DotMatrixPlugin);
