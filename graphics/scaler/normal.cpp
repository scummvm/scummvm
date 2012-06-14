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

NormalPlugin::NormalPlugin() {
	_factor = 1;
	_factors.push_back(1);
#ifdef USE_SCALERS
	_factors.push_back(2);
	_factors.push_back(3);
	_factors.push_back(4);
#endif
}

void NormalPlugin::initialize(Graphics::PixelFormat format) {
	_format = format;
}

/**
 * Trivial 'scaler' - in fact it doesn't do any scaling but just copies the
 * source to the destination.
 */
template<typename pixel>
void Normal1x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	// Spot the case when it can all be done in 1 hit
	int BytesPerPixel = sizeof(pixel);
	if ((srcPitch == BytesPerPixel * (uint)width) && (dstPitch == BytesPerPixel * (uint)width)) {
		memcpy(dstPtr, srcPtr, BytesPerPixel * width * height);
		return;
	}
	while (height--) {
		memcpy(dstPtr, srcPtr, BytesPerPixel * width);
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

#ifdef USE_SCALERS

/**
 * Trivial nearest-neighbor 2x scaler.
 */
template<typename pixel>
void Normal2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	int b = sizeof(pixel);

	assert(IS_ALIGNED(dstPtr, 2));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += b * 2) {
			pixel color = *(((const pixel*)srcPtr) + i);

			*(pixel *)(r) = color;
			*(pixel *)(r + b) = color;
			*(pixel *)(r + dstPitch) = color;
			*(pixel *)(r + b + dstPitch) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch << 1;
	}
}


#ifdef USE_ARM_SCALER_ASM
extern "C" void Normal2xARM(const uint8  *srcPtr,
                                  uint32  srcPitch,
                                  uint8  *dstPtr,
                                  uint32  dstPitch,
                                  int     width,
                                  int     height);

#else
/**
 * Template Specialization that writes 2 pixels at a time.
 */
template<>
void Normal2x<uint16>(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;

	assert(IS_ALIGNED(dstPtr, 4));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += 4) {
			uint32 color = *(((const uint16*)srcPtr) + i);

			color |= color << 16;

			*(uint32 *)(r) = color;
			*(uint32 *)(r + dstPitch) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch << 1;
	}
}
#endif

/**
 * Trivial nearest-neighbor 3x scaler.
 */
template<typename pixel>
void Normal3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	const uint32 dstPitch2 = dstPitch * 2;
	const uint32 dstPitch3 = dstPitch * 3;
	int b = sizeof(pixel);

	assert(IS_ALIGNED(dstPtr, 2));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += b * 3) {
			pixel color = *(((const pixel *)srcPtr) + i);

			*(pixel *)(r + b * 0) = color;
			*(pixel *)(r + b * 1) = color;
			*(pixel *)(r + b * 2) = color;
			*(pixel *)(r + b * 0 + dstPitch) = color;
			*(pixel *)(r + b * 1 + dstPitch) = color;
			*(pixel *)(r + b * 2 + dstPitch) = color;
			*(pixel *)(r + b * 0 + dstPitch2) = color;
			*(pixel *)(r + b * 1 + dstPitch2) = color;
			*(pixel *)(r + b * 2 + dstPitch2) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch3;
	}
}

/**
 * Trivial nearest-neighbor 4x scaler.
 */
template<typename pixel>
void Normal4x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	const uint32 dstPitch2 = dstPitch * 2;
	const uint32 dstPitch3 = dstPitch * 3;
	const uint32 dstPitch4 = dstPitch * 4;
	int b = sizeof(pixel);

	assert(IS_ALIGNED(dstPtr, 2));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += b * 4) {
			pixel color = *(((const pixel *)srcPtr) + i);

			*(pixel *)(r + b * 0) = color;
			*(pixel *)(r + b * 1) = color;
			*(pixel *)(r + b * 2) = color;
			*(pixel *)(r + b * 3) = color;
			*(pixel *)(r + b * 0 + dstPitch) = color;
			*(pixel *)(r + b * 1 + dstPitch) = color;
			*(pixel *)(r + b * 2 + dstPitch) = color;
			*(pixel *)(r + b * 3 + dstPitch) = color;
			*(pixel *)(r + b * 0 + dstPitch2) = color;
			*(pixel *)(r + b * 1 + dstPitch2) = color;
			*(pixel *)(r + b * 2 + dstPitch2) = color;
			*(pixel *)(r + b * 3 + dstPitch2) = color;
			*(pixel *)(r + b * 0 + dstPitch3) = color;
			*(pixel *)(r + b * 1 + dstPitch3) = color;
			*(pixel *)(r + b * 2 + dstPitch3) = color;
			*(pixel *)(r + b * 3 + dstPitch3) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch4;
	}
}
#endif

void NormalPlugin::scale(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
#ifdef USE_SCALERS
	if (_format.bytesPerPixel == 2) {
		switch (_factor) {
		case 1:
			Normal1x<uint16>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 2:
#ifdef USE_ARM_SCALER_ASM
			Normal2xARM(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
#else
			Normal2x<uint16>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
#endif
			break;
		case 3:
			Normal3x<uint16>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 4:
			Normal4x<uint16>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		}
	} else {
		assert(_format.bytesPerPixel == 4);
		switch (_factor) {
		case 1:
			Normal1x<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 2:
			Normal2x<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 3:
			Normal3x<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 4:
			Normal4x<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		}
	}
#else
	Normal1x<uint16>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
#endif
}

uint NormalPlugin::increaseFactor() {
#ifdef USE_SCALERS
	if (_factor < 4)
		++_factor;
#endif
	return _factor;
}

uint NormalPlugin::decreaseFactor() {
#ifdef USE_SCALERS
	if (_factor > 1)
		--_factor;
#endif
	return _factor;
}

const char *NormalPlugin::getName() const {
	return "normal";
}

const char *NormalPlugin::getPrettyName() const {
	return "Normal";
}

REGISTER_PLUGIN_STATIC(NORMAL, PLUGIN_TYPE_SCALER, NormalPlugin);
