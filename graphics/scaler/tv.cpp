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

#include "graphics/scaler/tv.h"
#include "graphics/scaler.h"
#include "graphics/colormasks.h"

TVPlugin::TVPlugin() {
	_factor = 2;
	_factors.push_back(2);
}

void TVPlugin::scaleIntern(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
	if (_format.bytesPerPixel == 2) {
		if (_format.gLoss == 2)
			scaleIntern<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		else
			scaleIntern<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	} else {
		if (_format.aLoss == 0)
			scaleIntern<Graphics::ColorMasks<8888> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		else
			scaleIntern<Graphics::ColorMasks<888> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	}

}

uint TVPlugin::increaseFactor() {
	return _factor;
}

uint TVPlugin::decreaseFactor() {
	return _factor;
}

const char *TVPlugin::getName() const {
	return "tv";
}

const char *TVPlugin::getPrettyName() const {
	return "TV";
}

template<typename ColorMask>
void TVPlugin::scaleIntern(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
					int width, int height) {
	typedef typename ColorMask::PixelType Pixel;

	const uint32 nextlineSrc = srcPitch / sizeof(Pixel);
	const Pixel *p = (const Pixel *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(Pixel);
	Pixel *q = (Pixel *)dstPtr;

	while (height--) {
		for (int i = 0, j = 0; i < width; ++i, j += 2) {
			Pixel p1 = *(p + i);
			Pixel pi;

			pi = (((p1 & ColorMask::kRedBlueMask) * 7) >> 3) & ColorMask::kRedBlueMask;
			pi |= (((p1 & ColorMask::kGreenMask) * 7) >> 3) & ColorMask::kGreenMask;
			pi |= p1 & ColorMask::kAlphaMask;

			uint8 r, g, b;
			_format.colorToRGB(p1,r,g,b);
			pi = _format.RGBToColor((r*7)/8, (g*7) / 8, (b*7) / 8);

			*(q + j) = p1;
			*(q + j + 1) = p1;
			*(q + j + nextlineDst) = pi;
			*(q + j + nextlineDst + 1) = pi;
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

REGISTER_PLUGIN_STATIC(TV, PLUGIN_TYPE_SCALER, TVPlugin);
