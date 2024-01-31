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

#include "common/endian.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "graphics/colormasks.h"
#include "graphics/scaler.h"
#include "graphics/scaler/intern.h"
#include "graphics/palette.h"
#include "graphics/managed_surface.h"

template<typename ColorMask>
uint16 quadBlockInterpolate(const uint8 *src, uint32 srcPitch) {
	uint16 colorx1y1 = *(((const uint16 *)src));
	uint16 colorx2y1 = *(((const uint16 *)src) + 1);

	uint16 colorx1y2 = *(((const uint16 *)(src + srcPitch)));
	uint16 colorx2y2 = *(((const uint16 *)(src + srcPitch)) + 1);

	return interpolate16_1_1_1_1<ColorMask>(colorx1y1, colorx2y1, colorx1y2, colorx2y2);
}

template<typename ColorMask>
void createThumbnail_2(const uint8 *src, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	// Make sure the width and height is a multiple of 2.
	width &= ~1;
	height &= ~1;

	for (int y = 0; y < height; y += 2) {
		for (int x = 0; x < width; x += 2, dstPtr += 2) {
			*((uint16 *)dstPtr) = quadBlockInterpolate<ColorMask>(src + 2 * x, srcPitch);
		}
		dstPtr += (dstPitch - 2 * width / 2);
		src += 2 * srcPitch;
	}
}

template<typename ColorMask>
void createThumbnail_4(const uint8 *src, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	// Make sure the width and height is a multiple of 4
	width &= ~3;
	height &= ~3;

	for (int y = 0; y < height; y += 4) {
		for (int x = 0; x < width; x += 4, dstPtr += 2) {
			uint16 upleft = quadBlockInterpolate<ColorMask>(src + 2 * x, srcPitch);
			uint16 upright = quadBlockInterpolate<ColorMask>(src + 2 * (x + 2), srcPitch);
			uint16 downleft = quadBlockInterpolate<ColorMask>(src + srcPitch * 2 + 2 * x, srcPitch);
			uint16 downright = quadBlockInterpolate<ColorMask>(src + srcPitch * 2 + 2 * (x + 2), srcPitch);

			*((uint16 *)dstPtr) = interpolate16_1_1_1_1<ColorMask>(upleft, upright, downleft, downright);
		}
		dstPtr += (dstPitch - 2 * width / 4);
		src += 4 * srcPitch;
	}
}

template<typename ColorMask>
static void scaleThumbnail(Graphics::Surface &in, Graphics::Surface &out) {
	while (in.w / out.w >= 4 || in.h / out.h >= 4) {
		createThumbnail_4<ColorMask>((const uint8 *)in.getPixels(), in.pitch, (uint8 *)in.getPixels(), in.pitch, in.w, in.h);
		in.w /= 4;
		in.h /= 4;
	}

	while (in.w / out.w >= 2 || in.h / out.h >= 2) {
		createThumbnail_2<ColorMask>((const uint8 *)in.getPixels(), in.pitch, (uint8 *)in.getPixels(), in.pitch, in.w, in.h);
		in.w /= 2;
		in.h /= 2;
	}

	if ((in.w == out.w && in.h < out.h) || (in.w < out.w && in.h == out.h)) {
		// In this case we simply center the input surface in the output
		uint8 *dst = (uint8 *)out.getBasePtr((out.w - in.w) / 2, (out.h - in.h) / 2);
		const uint8 *src = (const uint8 *)in.getPixels();

		for (int y = 0; y < in.h; ++y) {
			memcpy(dst, src, in.w * in.format.bytesPerPixel);
			src += in.pitch;
			dst += out.pitch;
		}
	} else {
		// Assure the aspect of the scaled image still matches the original.
		int targetWidth = out.w, targetHeight = out.h;

		const float inputAspect = (float)in.w / in.h;
		const float outputAspect = (float)out.w / out.h;

		if (inputAspect > outputAspect) {
			targetHeight = int(targetWidth / inputAspect);
		} else if (inputAspect < outputAspect) {
			targetWidth = int(targetHeight * inputAspect);
		}

		// Make sure we are still in the bounds of the output
		assert(targetWidth <= out.w);
		assert(targetHeight <= out.h);

		// Center the image on the output surface
		byte *dst = (byte *)out.getBasePtr((out.w - targetWidth) / 2, (out.h - targetHeight) / 2);
		const uint dstLineIncrease = out.pitch - targetWidth * out.format.bytesPerPixel;

		const float scaleFactorX = (float)targetWidth / in.w;
		const float scaleFactorY = (float)targetHeight / in.h;

		for (int y = 0; y < targetHeight; ++y) {
			const float yFrac = (y / scaleFactorY);
			const int y1 = (int)yFrac;
			const int y2 = (y1 + 1 < in.h) ? (y1 + 1) : (in.h - 1);

			for (int x = 0; x < targetWidth; ++x) {
				const float xFrac = (x / scaleFactorX);
				const int x1 = (int)xFrac;
				const int x2 = (x1 + 1 < in.w) ? (x1 + 1) : (in.w - 1);

				// Look up colors at the points
				uint8 p1R, p1G, p1B;
				in.format.colorToRGBT<ColorMask>(READ_UINT16(in.getBasePtr(x1, y1)), p1R, p1G, p1B);
				uint8 p2R, p2G, p2B;
				in.format.colorToRGBT<ColorMask>(READ_UINT16(in.getBasePtr(x2, y1)), p2R, p2G, p2B);
				uint8 p3R, p3G, p3B;
				in.format.colorToRGBT<ColorMask>(READ_UINT16(in.getBasePtr(x1, y2)), p3R, p3G, p3B);
				uint8 p4R, p4G, p4B;
				in.format.colorToRGBT<ColorMask>(READ_UINT16(in.getBasePtr(x2, y2)), p4R, p4G, p4B);

				const float xDiff = xFrac - x1;
				const float yDiff = yFrac - y1;

				uint8 pR = (uint8)((1 - yDiff) * ((1 - xDiff) * p1R + xDiff * p2R) + yDiff * ((1 - xDiff) * p3R + xDiff * p4R));
				uint8 pG = (uint8)((1 - yDiff) * ((1 - xDiff) * p1G + xDiff * p2G) + yDiff * ((1 - xDiff) * p3G + xDiff * p4G));
				uint8 pB = (uint8)((1 - yDiff) * ((1 - xDiff) * p1B + xDiff * p2B) + yDiff * ((1 - xDiff) * p3B + xDiff * p4B));

				WRITE_UINT16(dst, out.format.RGBToColorT<ColorMask>(pR, pG, pB));
				dst += 2;
			}

			// Move to the next line
			dst = (byte *)dst + dstLineIncrease;
		}
	}
}


/**
 * Copies the current screen contents to a new surface, using RGB565 format.
 * WARNING: surf->free() must be called by the user to avoid leaking.
 *
 * @param surf      the surface to store the data in it
 */
static bool grabScreen565(Graphics::Surface *surf) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return false;

	assert(screen->format.bytesPerPixel == 1 || screen->format.bytesPerPixel == 2
	       || screen->format.bytesPerPixel == 4);
	assert(screen->getPixels() != 0);

	Graphics::PixelFormat screenFormat = g_system->getScreenFormat();

	surf->create(screen->w, screen->h, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	Graphics::Palette *palette = nullptr;
	if (screenFormat.bytesPerPixel == 1) {
		palette = new Graphics::Palette(256);
		assert(palette);
		g_system->getPaletteManager()->grabPalette(palette->data, 0, palette->size);
	}

	for (int y = 0; y < screen->h; ++y) {
		for (int x = 0; x < screen->w; ++x) {
			byte r = 0, g = 0, b = 0;

			if (screenFormat.bytesPerPixel == 1) {
				uint8 pixel = *(uint8 *)screen->getBasePtr(x, y);
				r = palette->data[pixel * 3 + 0];
				g = palette->data[pixel * 3 + 1];
				b = palette->data[pixel * 3 + 2];
			} else if (screenFormat.bytesPerPixel == 2) {
				uint16 col = READ_UINT16(screen->getBasePtr(x, y));
				screenFormat.colorToRGB(col, r, g, b);
			} else if (screenFormat.bytesPerPixel == 4) {
				uint32 col = READ_UINT32(screen->getBasePtr(x, y));
				screenFormat.colorToRGB(col, r, g, b);
			}

			*((uint16 *)surf->getBasePtr(x, y)) = surf->format.RGBToColor(r, g, b);
		}
	}

	delete[] palette;

	g_system->unlockScreen();
	return true;
}

static bool createThumbnail(Graphics::Surface &out, Graphics::Surface &in) {
	int height;
	if ((in.w == 320 && in.h == 200) || (in.w == 640 && in.h == 400)) {
		height = kThumbnailHeight1;
	} else {
		height = kThumbnailHeight2;
	}

	out.create(kThumbnailWidth, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	assert(out.format == Graphics::createPixelFormat<565>());
	scaleThumbnail<Graphics::ColorMasks<565> >(in, out);
	in.free();
	return true;
}

bool createThumbnailFromScreen(Graphics::Surface *surf) {
	assert(surf);

	Graphics::Surface screen;

	if (!grabScreen565(&screen))
		return false;

	return createThumbnail(*surf, screen);
}

bool createThumbnail(Graphics::Surface *surf, const uint8 *pixels, int w, int h, const uint8 *palette) {
	assert(surf);

	Graphics::Surface screen;
	screen.create(w, h, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	for (int y = 0; y < screen.h; ++y) {
		for (int x = 0; x < screen.w; ++x) {
			byte r, g, b;
			r = palette[pixels[y * w + x] * 3];
			g = palette[pixels[y * w + x] * 3 + 1];
			b = palette[pixels[y * w + x] * 3 + 2];

			*((uint16 *)screen.getBasePtr(x, y)) = screen.format.RGBToColor(r, g, b);
		}
	}

	return createThumbnail(*surf, screen);
}

bool createThumbnail(Graphics::Surface *surf, Graphics::ManagedSurface *in) {
	assert(surf);

	Graphics::Surface screen;

	if (in->hasPalette()) {
		Graphics::Palette palette(256);
		in->grabPalette(palette, 0, 256);
		return createThumbnail(surf, (const uint8 *)in->getPixels(), in->w, in->h, palette.data);
	} else {
		screen.convertFrom(in->rawSurface(), Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
		return createThumbnail(*surf, screen);
	}
}

// this is somewhat awkward, but createScreenShot should logically be in graphics,
// but moving other functions in this file into that namespace breaks several engines
namespace Graphics {
bool createScreenShot(Graphics::Surface &surf) {
	Graphics::PixelFormat screenFormat = g_system->getScreenFormat();
	//convert surface to 2 bytes pixel format to avoid problems with palette saving and loading
	if ((screenFormat.bytesPerPixel == 1) || (screenFormat.bytesPerPixel == 2)) {
		return grabScreen565(&surf);
	} else {
		Graphics::Surface *screen = g_system->lockScreen();
		if (!screen) {
			return false;
		}
		surf.create(screen->w, screen->h, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
		for (int y = 0; y < screen->h; ++y) {
			for (int x = 0; x < screen->w; ++x) {
				byte r = 0, g = 0, b = 0, a = 0;
				uint32 col = READ_UINT32(screen->getBasePtr(x, y));
				screenFormat.colorToARGB(col, a, r, g, b);
				*((uint32 *)surf.getBasePtr(x, y)) = surf.format.ARGBToColor(a, r, g, b);
			}
		}
		g_system->unlockScreen();
		return true;
	}
}
} // End of namespace Graphics
