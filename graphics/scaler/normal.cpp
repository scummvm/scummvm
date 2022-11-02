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

#include "graphics/scaler/normal.h"

#ifdef USE_SCALERS

/**
 * Trivial nearest-neighbor 2x scaler.
 */
template<typename Pixel>
void Normal2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	const int b = sizeof(Pixel);

	assert(IS_ALIGNED(dstPtr, 2));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += b * 2) {
			Pixel color = *(((const Pixel*)srcPtr) + i);

			*(Pixel *)(r) = color;
			*(Pixel *)(r + b) = color;
			*(Pixel *)(r + dstPitch) = color;
			*(Pixel *)(r + b + dstPitch) = color;
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
template<typename Pixel>
void Normal3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	const uint32 dstPitch2 = dstPitch * 2;
	const uint32 dstPitch3 = dstPitch * 3;
	const int b = sizeof(Pixel);

	assert(IS_ALIGNED(dstPtr, 2));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += b * 3) {
			Pixel color = *(((const Pixel *)srcPtr) + i);

			*(Pixel *)(r + b * 0) = color;
			*(Pixel *)(r + b * 1) = color;
			*(Pixel *)(r + b * 2) = color;
			*(Pixel *)(r + b * 0 + dstPitch) = color;
			*(Pixel *)(r + b * 1 + dstPitch) = color;
			*(Pixel *)(r + b * 2 + dstPitch) = color;
			*(Pixel *)(r + b * 0 + dstPitch2) = color;
			*(Pixel *)(r + b * 1 + dstPitch2) = color;
			*(Pixel *)(r + b * 2 + dstPitch2) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch3;
	}
}

/**
 * Trivial nearest-neighbor 4x scaler.
 */
template<typename Pixel>
void Normal4x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	const uint32 dstPitch2 = dstPitch * 2;
	const uint32 dstPitch3 = dstPitch * 3;
	const uint32 dstPitch4 = dstPitch * 4;
	const int b = sizeof(Pixel);

	assert(IS_ALIGNED(dstPtr, 2));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += b * 4) {
			Pixel color = *(((const Pixel *)srcPtr) + i);

			*(Pixel *)(r + b * 0) = color;
			*(Pixel *)(r + b * 1) = color;
			*(Pixel *)(r + b * 2) = color;
			*(Pixel *)(r + b * 3) = color;
			*(Pixel *)(r + b * 0 + dstPitch) = color;
			*(Pixel *)(r + b * 1 + dstPitch) = color;
			*(Pixel *)(r + b * 2 + dstPitch) = color;
			*(Pixel *)(r + b * 3 + dstPitch) = color;
			*(Pixel *)(r + b * 0 + dstPitch2) = color;
			*(Pixel *)(r + b * 1 + dstPitch2) = color;
			*(Pixel *)(r + b * 2 + dstPitch2) = color;
			*(Pixel *)(r + b * 3 + dstPitch2) = color;
			*(Pixel *)(r + b * 0 + dstPitch3) = color;
			*(Pixel *)(r + b * 1 + dstPitch3) = color;
			*(Pixel *)(r + b * 2 + dstPitch3) = color;
			*(Pixel *)(r + b * 3 + dstPitch3) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch4;
	}
}

/**
 * Trivial nearest-neighbor 5x scaler.
 */
template<typename Pixel>
void Normal5x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	const uint32 dstPitch2 = dstPitch * 2;
	const uint32 dstPitch3 = dstPitch * 3;
	const uint32 dstPitch4 = dstPitch * 4;
	const uint32 dstPitch5 = dstPitch * 5;
	const int b = sizeof(Pixel);

	assert(IS_ALIGNED(dstPtr, 2));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += b * 5) {
			Pixel color = *(((const Pixel *)srcPtr) + i);

			*(Pixel *)(r + b * 0) = color;
			*(Pixel *)(r + b * 1) = color;
			*(Pixel *)(r + b * 2) = color;
			*(Pixel *)(r + b * 3) = color;
			*(Pixel *)(r + b * 4) = color;
			*(Pixel *)(r + b * 0 + dstPitch) = color;
			*(Pixel *)(r + b * 1 + dstPitch) = color;
			*(Pixel *)(r + b * 2 + dstPitch) = color;
			*(Pixel *)(r + b * 3 + dstPitch) = color;
			*(Pixel *)(r + b * 4 + dstPitch) = color;
			*(Pixel *)(r + b * 0 + dstPitch2) = color;
			*(Pixel *)(r + b * 1 + dstPitch2) = color;
			*(Pixel *)(r + b * 2 + dstPitch2) = color;
			*(Pixel *)(r + b * 3 + dstPitch2) = color;
			*(Pixel *)(r + b * 4 + dstPitch2) = color;
			*(Pixel *)(r + b * 0 + dstPitch3) = color;
			*(Pixel *)(r + b * 1 + dstPitch3) = color;
			*(Pixel *)(r + b * 2 + dstPitch3) = color;
			*(Pixel *)(r + b * 3 + dstPitch3) = color;
			*(Pixel *)(r + b * 4 + dstPitch3) = color;
			*(Pixel *)(r + b * 0 + dstPitch4) = color;
			*(Pixel *)(r + b * 1 + dstPitch4) = color;
			*(Pixel *)(r + b * 2 + dstPitch4) = color;
			*(Pixel *)(r + b * 3 + dstPitch4) = color;
			*(Pixel *)(r + b * 4 + dstPitch4) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch5;
	}
}
#endif

void NormalScaler::scaleIntern(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
#ifdef USE_SCALERS
	if (_format.bytesPerPixel == 1) {
		switch (_factor) {
		case 2:
			Normal2x<uint8>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 3:
			Normal3x<uint8>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 4:
			Normal4x<uint8>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 5:
			Normal5x<uint8>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		}
	} else if (_format.bytesPerPixel == 2) {
		switch (_factor) {
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
		case 5:
			Normal5x<uint16>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		}
	} else {
		assert(_format.bytesPerPixel == 4);
		switch (_factor) {
		case 2:
			Normal2x<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 3:
			Normal3x<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 4:
			Normal4x<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 5:
			Normal5x<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		}
	}
#endif
}

uint NormalScaler::increaseFactor() {
#ifdef USE_SCALERS
	if (_factor < 5)
		setFactor(_factor + 1);
#endif
	return _factor;
}

uint NormalScaler::decreaseFactor() {
#ifdef USE_SCALERS
	if (_factor > 1)
		setFactor(_factor - 1);
#endif
	return _factor;
}


class NormalPlugin final : public ScalerPluginObject {
public:
	NormalPlugin();

	Scaler *createInstance(const Graphics::PixelFormat &format) const override;

	bool canDrawCursor() const override { return true; }
	uint extraPixels() const override { return 0; }
	const char *getName() const override;
	const char *getPrettyName() const override;
};

NormalPlugin::NormalPlugin() {
	_factors.push_back(1);
#ifdef USE_SCALERS
	_factors.push_back(2);
	_factors.push_back(3);
	_factors.push_back(4);
	_factors.push_back(5);
#endif
}

Scaler *NormalPlugin::createInstance(const Graphics::PixelFormat &format) const {
	return new NormalScaler(format);
}

const char *NormalPlugin::getName() const {
	return "normal";
}

const char *NormalPlugin::getPrettyName() const {
	return "Normal";
}

REGISTER_PLUGIN_STATIC(NORMAL, PLUGIN_TYPE_SCALER, NormalPlugin);
