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

#include "graphics/scaler/normal.h"
#include "graphics/scaler.h"

NormalPlugin::NormalPlugin() {
	_factor = 1;
	_factors.push_back(1);
	_factors.push_back(2);
	_factors.push_back(3);
	_factors.push_back(4);
}

void NormalPlugin::initialize(Graphics::PixelFormat format) {
}

/**
 * Trivial nearest-neighbor 4x scaler.
 */
void Normal4x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	const uint32 dstPitch2 = dstPitch * 2;
	const uint32 dstPitch3 = dstPitch * 3;
	const uint32 dstPitch4 = dstPitch * 4;

	assert(IS_ALIGNED(dstPtr, 2));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += 8) {
			uint16 color = *(((const uint16 *)srcPtr) + i);

			*(uint16 *)(r + 0) = color;
			*(uint16 *)(r + 2) = color;
			*(uint16 *)(r + 4) = color;
			*(uint16 *)(r + 6) = color;
			*(uint16 *)(r + 0 + dstPitch) = color;
			*(uint16 *)(r + 2 + dstPitch) = color;
			*(uint16 *)(r + 4 + dstPitch) = color;
			*(uint16 *)(r + 6 + dstPitch) = color;
			*(uint16 *)(r + 0 + dstPitch2) = color;
			*(uint16 *)(r + 2 + dstPitch2) = color;
			*(uint16 *)(r + 4 + dstPitch2) = color;
			*(uint16 *)(r + 6 + dstPitch2) = color;
			*(uint16 *)(r + 0 + dstPitch3) = color;
			*(uint16 *)(r + 2 + dstPitch3) = color;
			*(uint16 *)(r + 4 + dstPitch3) = color;
			*(uint16 *)(r + 6 + dstPitch3) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch4;
	}
}

void NormalPlugin::scale(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
	switch (_factor) {
	case 1:
		Normal1x(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		break;
	case 2:
		Normal2x(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		break;
	case 3:
		Normal3x(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		break;
	case 4:
		Normal4x(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		break;
	}
}

uint NormalPlugin::increaseFactor() {
	if (_factor < 4)
		++_factor;
	return _factor;
}

uint NormalPlugin::decreaseFactor() {
	if (_factor > 1)
		--_factor;
	return _factor;
}

const char *NormalPlugin::getName() const {
	return "normal";
}

const char *NormalPlugin::getPrettyName() const {
	return "Normal";
}

REGISTER_PLUGIN_STATIC(NORMAL, PLUGIN_TYPE_SCALER, NormalPlugin);
