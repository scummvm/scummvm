#include <cxxtest/TestSuite.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "common/fs.h"
#include "common/stream.h"

#include "graphics/surface.h"
#include "graphics/managed_surface.h"
#include "graphics/transparent_surface.h"

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/math.h"
#include "common/textconsole.h"
#include "graphics/blit.h"
#include "graphics/primitives.h"
#include "graphics/transparent_surface.h"
#include "graphics/transform_tools.h"

namespace OldTransparentSurface {

using namespace Graphics;

struct OldTransparentSurface : public Graphics::Surface {
	OldTransparentSurface();
	OldTransparentSurface(const Graphics::Surface &surf, bool copyData = false);

	static PixelFormat getSupportedPixelFormat() {
		return PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	}

	Common::Rect blit(Graphics::Surface &target, int posX = 0, int posY = 0,
	                  int flipping = FLIP_NONE,
	                  Common::Rect *pPartRect = nullptr,
	                  uint color = TS_ARGB(255, 255, 255, 255),
	                  int width = -1, int height = -1,
	                  TSpriteBlendMode blend = BLEND_NORMAL);
	Common::Rect blitClip(Graphics::Surface &target, Common::Rect clippingArea,
						int posX = 0, int posY = 0,
						int flipping = FLIP_NONE,
						Common::Rect *pPartRect = nullptr,
						uint color = TS_ARGB(255, 255, 255, 255),
						int width = -1, int height = -1,
						TSpriteBlendMode blend = BLEND_NORMAL);
	OldTransparentSurface *scale(int16 newWidth, int16 newHeight, bool filtering = false) const;

	OldTransparentSurface *rotoscale(const TransformStruct &transform, bool filtering = false) const;

	OldTransparentSurface *convertTo(const PixelFormat &dstFormat, const byte *palette = 0) const;

	float getRatio() {
		if (!w)
			return 0;

		return h / (float)w;
	}

	AlphaType getAlphaMode() const;
	void setAlphaMode(AlphaType);
private:
	AlphaType _alphaMode;
};

static const int kBModShift = 8;//img->format.bShift;
static const int kGModShift = 16;//img->format.gShift;
static const int kRModShift = 24;//img->format.rShift;
static const int kAModShift = 0;//img->format.aShift;

static const uint32 kBModMask = 0x0000ff00;
static const uint32 kGModMask = 0x00ff0000;
static const uint32 kRModMask = 0xff000000;
static const uint32 kAModMask = 0x000000ff;
static const uint32 kRGBModMask = (kRModMask | kGModMask | kBModMask);

#ifdef SCUMM_LITTLE_ENDIAN
static const int kAIndex = 0;
static const int kBIndex = 1;
static const int kGIndex = 2;
static const int kRIndex = 3;

#else
static const int kAIndex = 3;
static const int kBIndex = 2;
static const int kGIndex = 1;
static const int kRIndex = 0;
#endif

OldTransparentSurface::OldTransparentSurface() : Surface(), _alphaMode(ALPHA_FULL) {}

OldTransparentSurface::OldTransparentSurface(const Surface &surf, bool copyData) : Surface(), _alphaMode(ALPHA_FULL) {
	if (copyData) {
		copyFrom(surf);
	} else {
		w = surf.w;
		h = surf.h;
		pitch = surf.pitch;
		format = surf.format;
		// We need to cast the const qualifier away here because 'pixels'
		// always needs to be writable. 'surf' however is a constant Surface,
		// thus getPixels will always return const pixel data.
		pixels = const_cast<void *>(surf.getPixels());
	}
}

/**
 * Optimized version of doBlit to be used w/opaque blitting (no alpha).
 */
static void doBlitOpaqueFast(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep) {

	byte *in;
	byte *out;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {
			memcpy(out, in, 4);
			out[kAIndex] = 0xFF;
			out += 4;
			in += inStep;
		}
		outo += pitch;
		ino += inoStep;
	}
}

/**
 * Optimized version of doBlit to be used w/binary blitting (blit or no-blit, no blending).
 */
static void doBlitBinaryFast(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep) {

	byte *in;
	byte *out;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {
			uint32 pix = *(uint32 *)in;
			int a = in[kAIndex];

			if (a != 0) {   // Full opacity (Any value not exactly 0 is Opaque here)
				*(uint32 *)out = pix;
				out[kAIndex] = 0xFF;
			}
			out += 4;
			in += inStep;
		}
		outo += pitch;
		ino += inoStep;
	}
}

/**
 * Optimized version of doBlit to be used with alpha blended blitting
 * @param ino a pointer to the input surface
 * @param outo a pointer to the output surface
 * @param width width of the input surface
 * @param height height of the input surface
 * @param pitch pitch of the output surface - that is, width in bytes of every row, usually bpp * width of the TARGET surface (the area we are blitting to might be smaller, do the math)
 * @inStep size in bytes to skip to address each pixel, usually bpp of the source surface
 * @inoStep width in bytes of every row on the *input* surface / kind of like pitch
 * @color colormod in 0xAARRGGBB format - 0xFFFFFFFF for no colormod
 */
template<bool rgbmod, bool alphamod>
static void doBlitAlphaBlendImpl(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {

	byte *in;
	byte *out;

	byte ca = alphamod ? ((color >> kAModShift) & 0xFF) : 255;
	byte cr = rgbmod   ? ((color >> kRModShift) & 0xFF) : 255;
	byte cg = rgbmod   ? ((color >> kGModShift) & 0xFF) : 255;
	byte cb = rgbmod   ? ((color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {

			uint32 ina = in[kAIndex] * ca >> 8;

			if (ina != 0) {
				uint outb = (out[kBIndex] * (255 - ina) >> 8);
				uint outg = (out[kGIndex] * (255 - ina) >> 8);
				uint outr = (out[kRIndex] * (255 - ina) >> 8);

				out[kAIndex] = 255;
				out[kBIndex] = outb + (in[kBIndex] * ina * cb >> 16);
				out[kGIndex] = outg + (in[kGIndex] * ina * cg >> 16);
				out[kRIndex] = outr + (in[kRIndex] * ina * cr >> 16);
			}

			in += inStep;
			out += 4;
		}
		outo += pitch;
		ino += inoStep;
	}
}

static void doBlitAlphaBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {

	bool rgbmod   = ((color & kRGBModMask) != kRGBModMask);
	bool alphamod = ((color & kAModMask)   != kAModMask);

	if (rgbmod) {
		if (alphamod) {
			doBlitAlphaBlendImpl<true, true>(ino, outo, width, height, pitch, inStep, inoStep, color);
		} else {
			doBlitAlphaBlendImpl<true, false>(ino, outo, width, height, pitch, inStep, inoStep, color);
		}
	} else {
		if (alphamod) {
			doBlitAlphaBlendImpl<false, true>(ino, outo, width, height, pitch, inStep, inoStep, color);
		} else {
			doBlitAlphaBlendImpl<false, false>(ino, outo, width, height, pitch, inStep, inoStep, color);
		}
	}
}

/**
 * Optimized version of doBlit to be used with additive blended blitting
 */
template<bool rgbmod, bool alphamod>
static void doBlitAdditiveBlendImpl(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {

	byte *in;
	byte *out;

	byte ca = alphamod ? ((color >> kAModShift) & 0xFF) : 255;
	byte cr = rgbmod   ? ((color >> kRModShift) & 0xFF) : 255;
	byte cg = rgbmod   ? ((color >> kGModShift) & 0xFF) : 255;
	byte cb = rgbmod   ? ((color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {

			uint32 ina = in[kAIndex] * ca >> 8;

			if (ina != 0) {
				if (cb != 255) {
					out[kBIndex] = MIN<uint>(out[kBIndex] + ((in[kBIndex] * cb * ina) >> 16), 255u);
				} else {
					out[kBIndex] = MIN<uint>(out[kBIndex] + (in[kBIndex] * ina >> 8), 255u);
				}

				if (cg != 255) {
					out[kGIndex] = MIN<uint>(out[kGIndex] + ((in[kGIndex] * cg * ina) >> 16), 255u);
				} else {
					out[kGIndex] = MIN<uint>(out[kGIndex] + (in[kGIndex] * ina >> 8), 255u);
				}

				if (cr != 255) {
					out[kRIndex] = MIN<uint>(out[kRIndex] + ((in[kRIndex] * cr * ina) >> 16), 255u);
				} else {
					out[kRIndex] = MIN<uint>(out[kRIndex] + (in[kRIndex] * ina >> 8), 255u);
				}
			}

			in += inStep;
			out += 4;
		}

		outo += pitch;
		ino += inoStep;
	}
}

static void doBlitAdditiveBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {

	bool rgbmod   = ((color & kRGBModMask) != kRGBModMask);
	bool alphamod = ((color & kAModMask)   != kAModMask);

	if (rgbmod) {
		if (alphamod) {
			doBlitAdditiveBlendImpl<true, true>(ino, outo, width, height, pitch, inStep, inoStep, color);
		} else {
			doBlitAdditiveBlendImpl<true, false>(ino, outo, width, height, pitch, inStep, inoStep, color);
		}
	} else {
		if (alphamod) {
			doBlitAdditiveBlendImpl<false, true>(ino, outo, width, height, pitch, inStep, inoStep, color);
		} else {
			doBlitAdditiveBlendImpl<false, false>(ino, outo, width, height, pitch, inStep, inoStep, color);
		}
	}
}

/**
 * Optimized version of doBlit to be used with subtractive blended blitting
 */
template<bool rgbmod>
static void doBlitSubtractiveBlendImpl(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {

	byte *in;
	byte *out;

	byte cr = rgbmod   ? ((color >> kRModShift) & 0xFF) : 255;
	byte cg = rgbmod   ? ((color >> kGModShift) & 0xFF) : 255;
	byte cb = rgbmod   ? ((color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {

			out[kAIndex] = 255;
			if (cb != 255) {
				out[kBIndex] = MAX(out[kBIndex] - ((in[kBIndex] * cb  * (out[kBIndex]) * in[kAIndex]) >> 24), 0);
			} else {
				out[kBIndex] = MAX(out[kBIndex] - (in[kBIndex] * (out[kBIndex]) * in[kAIndex] >> 16), 0);
			}

			if (cg != 255) {
				out[kGIndex] = MAX(out[kGIndex] - ((in[kGIndex] * cg  * (out[kGIndex]) * in[kAIndex]) >> 24), 0);
			} else {
				out[kGIndex] = MAX(out[kGIndex] - (in[kGIndex] * (out[kGIndex]) * in[kAIndex] >> 16), 0);
			}

			if (cr != 255) {
				out[kRIndex] = MAX(out[kRIndex] - ((in[kRIndex] * cr * (out[kRIndex]) * in[kAIndex]) >> 24), 0);
			} else {
				out[kRIndex] = MAX(out[kRIndex] - (in[kRIndex] * (out[kRIndex]) * in[kAIndex] >> 16), 0);
			}

			in += inStep;
			out += 4;
		}
		outo += pitch;
		ino += inoStep;
	}
}

static void doBlitSubtractiveBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {

	bool rgbmod   = ((color & kRGBModMask) != kRGBModMask);

	if (rgbmod) {
		doBlitSubtractiveBlendImpl<true>(ino, outo, width, height, pitch, inStep, inoStep, color);
	} else {
		doBlitSubtractiveBlendImpl<false>(ino, outo, width, height, pitch, inStep, inoStep, color);
	}
}

/**
 * Optimized version of doBlit to be used with multiply blended blitting
 */
template<bool rgbmod, bool alphamod>
static void doBlitMultiplyBlendImpl(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {

	byte *in;
	byte *out;

	byte ca = alphamod ? ((color >> kAModShift) & 0xFF) : 255;
	byte cr = rgbmod   ? ((color >> kRModShift) & 0xFF) : 255;
	byte cg = rgbmod   ? ((color >> kGModShift) & 0xFF) : 255;
	byte cb = rgbmod   ? ((color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {

			uint32 ina = in[kAIndex] * ca >> 8;

			if (ina != 0) {
				if (cb != 255) {
					out[kBIndex] = MIN<uint>(out[kBIndex] * ((in[kBIndex] * cb * ina) >> 16) >> 8, 255u);
				} else {
					out[kBIndex] = MIN<uint>(out[kBIndex] * (in[kBIndex] * ina >> 8) >> 8, 255u);
				}

				if (cg != 255) {
					out[kGIndex] = MIN<uint>(out[kGIndex] * ((in[kGIndex] * cg * ina) >> 16) >> 8, 255u);
				} else {
					out[kGIndex] = MIN<uint>(out[kGIndex] * (in[kGIndex] * ina >> 8) >> 8, 255u);
				}

				if (cr != 255) {
					out[kRIndex] = MIN<uint>(out[kRIndex] * ((in[kRIndex] * cr * ina) >> 16) >> 8, 255u);
				} else {
					out[kRIndex] = MIN<uint>(out[kRIndex] * (in[kRIndex] * ina >> 8) >> 8, 255u);
				}
			}

			in += inStep;
			out += 4;
		}
		outo += pitch;
		ino += inoStep;
	}

}

static void doBlitMultiplyBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {

	bool rgbmod   = ((color & kRGBModMask) != kRGBModMask);
	bool alphamod = ((color & kAModMask)   != kAModMask);

	if (rgbmod) {
		if (alphamod) {
			doBlitMultiplyBlendImpl<true, true>(ino, outo, width, height, pitch, inStep, inoStep, color);
		} else {
			doBlitMultiplyBlendImpl<true, false>(ino, outo, width, height, pitch, inStep, inoStep, color);
		}
	} else {
		if (alphamod) {
			doBlitMultiplyBlendImpl<false, true>(ino, outo, width, height, pitch, inStep, inoStep, color);
		} else {
			doBlitMultiplyBlendImpl<false, false>(ino, outo, width, height, pitch, inStep, inoStep, color);
		}
	}
}

Common::Rect OldTransparentSurface::blit(Graphics::Surface &target, int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height, TSpriteBlendMode blendMode) {

	Common::Rect retSize;
	retSize.top = 0;
	retSize.left = 0;
	retSize.setWidth(0);
	retSize.setHeight(0);
	// Check if we need to draw anything at all
	int ca = (color >> kAModShift) & 0xff;

	if (ca == 0) {
		return retSize;
	}

	// Create an encapsulating surface for the data
	OldTransparentSurface srcImage(*this, false);
	// TODO: Is the data really in the screen format?
	if (format.bytesPerPixel != 4) {
		warning("OldTransparentSurface can only blit 32bpp images, but got %d", format.bytesPerPixel * 8);
		return retSize;
	}

	if (pPartRect) {

		int xOffset = pPartRect->left;
		int yOffset = pPartRect->top;

		if (flipping & FLIP_V) {
			yOffset = srcImage.h - pPartRect->bottom;
		}

		if (flipping & FLIP_H) {
			xOffset = srcImage.w - pPartRect->right;
		}

		srcImage.pixels = getBasePtr(xOffset, yOffset);
		srcImage.w = pPartRect->width();
		srcImage.h = pPartRect->height();

		debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %08x, %d, %d)", posX, posY, flipping,
			  pPartRect->left,  pPartRect->top, pPartRect->width(), pPartRect->height(), color, width, height);
	} else {

		debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %08x, %d, %d)", posX, posY, flipping, 0, 0,
			  srcImage.w, srcImage.h, color, width, height);
	}

	if (width == -1) {
		width = srcImage.w;
	}
	if (height == -1) {
		height = srcImage.h;
	}

#ifdef SCALING_TESTING
	// Hardcode scaling to 66% to test scaling
	width = width * 2 / 3;
	height = height * 2 / 3;
#endif

	Graphics::Surface *img = nullptr;
	Graphics::Surface *imgScaled = nullptr;
	byte *savedPixels = nullptr;
	if ((width != srcImage.w) || (height != srcImage.h)) {
		// Scale the image
		img = imgScaled = srcImage.scale(width, height);
		savedPixels = (byte *)img->getPixels();
	} else {
		img = &srcImage;
	}

	// Handle off-screen clipping
	if (posY < 0) {
		img->h = MAX(0, (int)img->h - -posY);
		if (!(flipping & FLIP_V))
			img->setPixels((byte *)img->getBasePtr(0, -posY));
		posY = 0;
	}

	if (posX < 0) {
		img->w = MAX(0, (int)img->w - -posX);
		if (!(flipping & FLIP_H))
			img->setPixels((byte *)img->getBasePtr(-posX, 0));
		posX = 0;
	}

	if (img->w > target.w - posX) {
		if (flipping & FLIP_H)
			img->setPixels((byte *)img->getBasePtr(img->w - target.w + posX, 0));
		img->w = CLIP((int)img->w, 0, (int)MAX((int)target.w - posX, 0));
	}

	if (img->h > target.h - posY) {
		if (flipping & FLIP_V)
			img->setPixels((byte *)img->getBasePtr(0, img->h - target.h + posY));
		img->h = CLIP((int)img->h, 0, (int)MAX((int)target.h - posY, 0));
	}

	// Flip surface
	if ((img->w > 0) && (img->h > 0)) {
		int xp = 0, yp = 0;

		int inStep = 4;
		int inoStep = img->pitch;
		if (flipping & FLIP_H) {
			inStep = -inStep;
			xp = img->w - 1;
		}

		if (flipping & FLIP_V) {
			inoStep = -inoStep;
			yp = img->h - 1;
		}

		byte *ino = (byte *)img->getBasePtr(xp, yp);
		byte *outo = (byte *)target.getBasePtr(posX, posY);

		if (color == 0xFFFFFFFF && blendMode == BLEND_NORMAL && _alphaMode == ALPHA_OPAQUE) {
			doBlitOpaqueFast(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
		} else if (color == 0xFFFFFFFF && blendMode == BLEND_NORMAL && _alphaMode == ALPHA_BINARY) {
			doBlitBinaryFast(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				doBlitAdditiveBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				doBlitSubtractiveBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else if (blendMode == BLEND_MULTIPLY) {
				doBlitMultiplyBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else {
				assert(blendMode == BLEND_NORMAL);
				doBlitAlphaBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			}
		}

	}

	retSize.setWidth(img->w);
	retSize.setHeight(img->h);

	if (imgScaled) {
		imgScaled->setPixels(savedPixels);
		imgScaled->free();
		delete imgScaled;
	}

	return retSize;
}

Common::Rect OldTransparentSurface::blitClip(Graphics::Surface &target, Common::Rect clippingArea, int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height, TSpriteBlendMode blendMode) {
	Common::Rect retSize;
	retSize.top = 0;
	retSize.left = 0;
	retSize.setWidth(0);
	retSize.setHeight(0);
	// Check if we need to draw anything at all
	int ca = (color >> kAModShift) & 0xff;

	if (ca == 0) {
		return retSize;
	}

	// Create an encapsulating surface for the data
	OldTransparentSurface srcImage(*this, false);
	// TODO: Is the data really in the screen format?
	if (format.bytesPerPixel != 4) {
		warning("OldTransparentSurface can only blit 32bpp images, but got %d", format.bytesPerPixel * 8);
		return retSize;
	}

	if (pPartRect) {

		int xOffset = pPartRect->left;
		int yOffset = pPartRect->top;

		if (flipping & FLIP_V) {
			yOffset = srcImage.h - pPartRect->bottom;
		}

		if (flipping & FLIP_H) {
			xOffset = srcImage.w - pPartRect->right;
		}

		srcImage.pixels = getBasePtr(xOffset, yOffset);
		srcImage.w = pPartRect->width();
		srcImage.h = pPartRect->height();

		debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %08x, %d, %d)", posX, posY, flipping,
			pPartRect->left, pPartRect->top, pPartRect->width(), pPartRect->height(), color, width, height);
	} else {

		debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %08x, %d, %d)", posX, posY, flipping, 0, 0,
			srcImage.w, srcImage.h, color, width, height);
	}

	if (width == -1) {
		width = srcImage.w;
	}
	if (height == -1) {
		height = srcImage.h;
	}

#ifdef SCALING_TESTING
	// Hardcode scaling to 66% to test scaling
	width = width * 2 / 3;
	height = height * 2 / 3;
#endif

	Graphics::Surface *img = nullptr;
	Graphics::Surface *imgScaled = nullptr;
	byte *savedPixels = nullptr;
	if ((width != srcImage.w) || (height != srcImage.h)) {
		// Scale the image
		img = imgScaled = srcImage.scale(width, height);
		savedPixels = (byte *)img->getPixels();
	} else {
		img = &srcImage;
	}

	// Handle off-screen clipping
	if (posY < clippingArea.top) {
		img->h = MAX(0, (int)img->h - (clippingArea.top - posY));
		if (!(flipping & FLIP_V))
			img->setPixels((byte *)img->getBasePtr(0, clippingArea.top - posY));
		posY = clippingArea.top;
	}

	if (posX < clippingArea.left) {
		img->w = MAX(0, (int)img->w - (clippingArea.left - posX));
		if (!(flipping & FLIP_H))
			img->setPixels((byte *)img->getBasePtr(clippingArea.left - posX, 0));
		posX = clippingArea.left;
	}

	if (img->w > clippingArea.right - posX) {
		if (flipping & FLIP_H)
			img->setPixels((byte *)img->getBasePtr(img->w - clippingArea.right + posX, 0));
		img->w = CLIP((int)img->w, 0, (int)MAX((int)clippingArea.right - posX, 0));
	}

	if (img->h > clippingArea.bottom - posY) {
		if (flipping & FLIP_V)
			img->setPixels((byte *)img->getBasePtr(0, img->h - clippingArea.bottom + posY));
		img->h = CLIP((int)img->h, 0, (int)MAX((int)clippingArea.bottom - posY, 0));
	}

	// Flip surface
	if ((img->w > 0) && (img->h > 0)) {
		int xp = 0, yp = 0;

		int inStep = 4;
		int inoStep = img->pitch;
		if (flipping & FLIP_H) {
			inStep = -inStep;
			xp = img->w - 1;
		}

		if (flipping & FLIP_V) {
			inoStep = -inoStep;
			yp = img->h - 1;
		}

		byte *ino = (byte *)img->getBasePtr(xp, yp);
		byte *outo = (byte *)target.getBasePtr(posX, posY);

		if (color == 0xFFFFFFFF && blendMode == BLEND_NORMAL && _alphaMode == ALPHA_OPAQUE) {
			doBlitOpaqueFast(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
		} else if (color == 0xFFFFFFFF && blendMode == BLEND_NORMAL && _alphaMode == ALPHA_BINARY) {
			doBlitBinaryFast(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				doBlitAdditiveBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				doBlitSubtractiveBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else if (blendMode == BLEND_MULTIPLY) {
				doBlitMultiplyBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else {
				assert(blendMode == BLEND_NORMAL);
				doBlitAlphaBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			}
		}

	}

	retSize.setWidth(img->w);
	retSize.setHeight(img->h);

	if (imgScaled) {
		imgScaled->setPixels(savedPixels);
		imgScaled->free();
		delete imgScaled;
	}

	return retSize;
}

AlphaType OldTransparentSurface::getAlphaMode() const {
	return _alphaMode;
}

void OldTransparentSurface::setAlphaMode(AlphaType mode) {
	_alphaMode = mode;
}

OldTransparentSurface *OldTransparentSurface::scale(int16 newWidth, int16 newHeight, bool filtering) const {

	OldTransparentSurface *target = new OldTransparentSurface();

	target->create(newWidth, newHeight, format);

	if (filtering) {
		scaleBlitBilinear((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format);
	} else {
		scaleBlit((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format);
	}

	return target;
}

OldTransparentSurface *OldTransparentSurface::rotoscale(const TransformStruct &transform, bool filtering) const {

	Common::Point newHotspot;
	Common::Rect rect = TransformTools::newRect(Common::Rect((int16)w, (int16)h), transform, &newHotspot);

	OldTransparentSurface *target = new OldTransparentSurface();

	target->create((uint16)rect.right - rect.left, (uint16)rect.bottom - rect.top, this->format);

	if (filtering) {
		rotoscaleBlitBilinear((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format, transform, newHotspot);
	} else {
		rotoscaleBlit((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format, transform, newHotspot);
	}

	return target;
}

OldTransparentSurface *OldTransparentSurface::convertTo(const PixelFormat &dstFormat, const byte *palette) const {
	assert(pixels);

	OldTransparentSurface *surface = new OldTransparentSurface();

	// If the target format is the same, just copy
	if (format == dstFormat) {
		surface->copyFrom(*this);
		return surface;
	}

	if (format.bytesPerPixel == 0 || format.bytesPerPixel > 4)
		error("Surface::convertTo(): Can only convert from 1Bpp, 2Bpp, 3Bpp, and 4Bpp");

	if (dstFormat.bytesPerPixel != 2 && dstFormat.bytesPerPixel != 4)
		error("Surface::convertTo(): Can only convert to 2Bpp and 4Bpp");

	surface->create(w, h, dstFormat);

	if (format.bytesPerPixel == 1) {
		// Converting from paletted to high color
		assert(palette);

		for (int y = 0; y < h; y++) {
			const byte *srcRow = (const byte *)getBasePtr(0, y);
			byte *dstRow = (byte *)surface->getBasePtr(0, y);

			for (int x = 0; x < w; x++) {
				byte index = *srcRow++;
				byte r = palette[index * 3];
				byte g = palette[index * 3 + 1];
				byte b = palette[index * 3 + 2];

				uint32 color = dstFormat.RGBToColor(r, g, b);

				if (dstFormat.bytesPerPixel == 2)
					*((uint16 *)dstRow) = color;
				else
					*((uint32 *)dstRow) = color;

				dstRow += dstFormat.bytesPerPixel;
			}
		}
	} else {
		// Converting from high color to high color
		for (int y = 0; y < h; y++) {
			const byte *srcRow = (const byte *)getBasePtr(0, y);
			byte *dstRow = (byte *)surface->getBasePtr(0, y);

			for (int x = 0; x < w; x++) {
				uint32 srcColor;
				if (format.bytesPerPixel == 2)
					srcColor = READ_UINT16(srcRow);
				else if (format.bytesPerPixel == 3)
					srcColor = READ_UINT24(srcRow);
				else
					srcColor = READ_UINT32(srcRow);

				srcRow += format.bytesPerPixel;

				// Convert that color to the new format
				byte r, g, b, a;
				format.colorToARGB(srcColor, a, r, g, b);
				uint32 color = dstFormat.ARGBToColor(a, r, g, b);

				if (dstFormat.bytesPerPixel == 2)
					*((uint16 *)dstRow) = color;
				else
					*((uint32 *)dstRow) = color;

				dstRow += dstFormat.bytesPerPixel;
			}
		}
	}

	return surface;
}

} // namespace OldTransparentSurface

static int save_bitmap(const char *path, const Graphics::Surface *surf) {
    Common::FSNode fileNode(path);
    Common::SeekableWriteStream *out = fileNode.createWriteStream();
#ifdef SCUMM_LITTLE_ENDIAN
	const Graphics::PixelFormat requiredFormat_3byte(3, 8, 8, 8, 0, 16, 8, 0, 0);
#else
	const Graphics::PixelFormat requiredFormat_3byte(3, 8, 8, 8, 0, 0, 8, 16, 0);
#endif
	Graphics::ManagedSurface surface(surf->w, surf->h, requiredFormat_3byte);

	// Copy from the source surface without alpha transparency
	Graphics::ManagedSurface temp = surf;
	temp.format.aLoss = 8;
	surface.rawBlitFrom(temp, Common::Rect(0, 0, surf->w, surf->h),
		Common::Point(0, 0));

	// Write out the bitmap
	int dstPitch = surface.w * 3;
	int extraDataLength = (dstPitch % 4) ? 4 - (dstPitch % 4) : 0;
	int padding = 0;

	out->writeByte('B');
	out->writeByte('M');
	out->writeUint32LE(surface.h * dstPitch + 54);
	out->writeUint32LE(0);
	out->writeUint32LE(54);
	out->writeUint32LE(40);
	out->writeUint32LE(surface.w);
	out->writeUint32LE(surface.h);
	out->writeUint16LE(1);
	out->writeUint16LE(24);
	out->writeUint32LE(0);
	out->writeUint32LE(0);
	out->writeUint32LE(0);
	out->writeUint32LE(0);
	out->writeUint32LE(0);
	out->writeUint32LE(0);

	for (uint y = surface.h; y-- > 0;) {
		out->write((const void *)surface.getBasePtr(0, y), dstPitch);
		out->write(&padding, extraDataLength);
	}

	return true;
}

static bool areSurfacesEqual(const Graphics::Surface *a, const Graphics::Surface *b) {
    if (a->w != b->w || a->h != b->h) return false;

    for (int y = 0; y < a->h; y++) {
        for (int x = 0; x < a->w; x++) {
            if (a->getPixel(x, y) != b->getPixel(x, y)) return false;
        }
    }

    return true;
}

class BlendBlitUnfilteredTestSuite : public CxxTest::TestSuite {
public:
    void test_blend_blit_unfiltered() {
        Common::Rect dsts[] = {
            Common::Rect(4, 4, 4+16, 4+16), // Case 0 (source clipping)
            Common::Rect(24, 20, 24+16, 20+16), // Case 1 (outside of destination)
            Common::Rect(0, 0, 32, 32), // Case 2 (stretching bigger)
            Common::Rect(3, 3, 8, 8), // Case 3 (stretching smaller)
            Common::Rect(8, 4, 8+32, 4+32), // Case 4 (stretching outside of destination)
            Common::Rect(-4, -4, -4+16, -4+16), // Case 5 (outside of destination 2)
        }, srcs[] = {
            Common::Rect(0, 0, 16, 16), // Case 0 (source clipping)
            Common::Rect(0, 0, 16, 16), // Case 1 (outside of destination)
            Common::Rect(0, 0, 16, 16), // Case 2 (stretching)
            Common::Rect(0, 0, 16, 16), // Case 3 (stretching smaller)
            Common::Rect(0, 0, 16, 16), // Case 4 (stretching outside of destination)
            Common::Rect(0, 0, 16, 16), // Case 5 (outside of destination 2)
        };

	    Graphics::Surface baseSurface, destSurface;
	    baseSurface.create(16, 16, OldTransparentSurface::OldTransparentSurface::getSupportedPixelFormat());
	    destSurface.create(32, 32, OldTransparentSurface::OldTransparentSurface::getSupportedPixelFormat());
	    for (int y = 0; y < baseSurface.h; y++) {
	    	for (int x = 0; x < baseSurface.w; x++) {
                int i = x / 4 + y / 4;
	    		baseSurface.setPixel(x, y, baseSurface.format.ARGBToColor((i & 16) * 255, (i & 1) * 255, (i & 2) * 255, (i & 4) * 255));
	    	}
	    }

	    OldTransparentSurface::OldTransparentSurface oldSurf(baseSurface, true);
	    OldTransparentSurface::OldTransparentSurface oldSurfDest(destSurface, true);
	    Graphics::TransparentSurface newSurf(baseSurface, true);
	    Graphics::TransparentSurface newSurfDest(destSurface, true);
	    Graphics::ManagedSurface managedSurf(&baseSurface, DisposeAfterUse::NO);
	    Graphics::ManagedSurface managedSurfDest(&destSurface, DisposeAfterUse::NO);
        const char *blendModes[] = {
            "BLEND_NORMAL",
            "BLEND_ADDITIVE",
            "BLEND_SUBTRACTIVE",
            "BLEND_MULTIPLY",
        }, *alphaTypes[] = {
            "ALPHA_OPAQUE",
            "ALPHA_BINARY",
            "ALPHA_FULL",
        }, *flipNames[] = {
            "FLIP_NONE",
            "FLIP_H",
            "FLIP_V",
            "FLIP_HV",
        }, *rectNames[] = {
            "0 -> (source clipping)",
            "1 -> (outside of destination)",
            "2 -> (stretching bigger)",
            "3 -> (stretching smaller)",
            "4 -> (stretching outside of destination)",
            "5 -> (outside of destination)",
        };

        for (int blendMode = 0; blendMode < Graphics::NUM_BLEND_MODES; blendMode++) {
        for (int alphaType = 0; alphaType <= Graphics::ALPHA_FULL; alphaType++) {
        for (int ba = 255; ba >= 0; ba = (ba == 255 ? 128 : (ba == 128 ? 0 : -1))) {
        for (int br = 255; br >= 0; br = (br == 255 ? 128 : (br == 128 ? 0 : -1))) {
        for (int bg = 255; bg >= 0; bg = (bg == 255 ? 128 : (bg == 128 ? 0 : -1))) {
        for (int bb = 255; bb >= 0; bb = (bb == 255 ? 128 : (bb == 128 ? 0 : -1))) {
        for (int a = 255; a >= 0; a = (a == 255 ? 128 : (a == 128 ? 0 : -1))) {
        for (int r = 255; r >= 0; r = (r == 255 ? 128 : (r == 128 ? 0 : -1))) {
        for (int g = 255; g >= 0; g = (g == 255 ? 128 : (g == 128 ? 0 : -1))) {
        for (int b = 255; b >= 0; b = (b == 255 ? 128 : (b == 128 ? 0 : -1))) {
        for (int flipping = 0; flipping <= 3; flipping++) {
        for (int rect = 0; rect < sizeof(srcs)/sizeof(srcs[0]); rect++) {
            oldSurfDest.fillRect(Common::Rect(0, 0, oldSurfDest.w, oldSurfDest.h), oldSurfDest.format.ARGBToColor(ba, br, bg, bb));
            oldSurf.setAlphaMode((Graphics::AlphaType)alphaType);
            oldSurf.blit(oldSurfDest, dsts[rect].left, dsts[rect].top, flipping, &srcs[rect], TS_ARGB(a, r, g, b), dsts[rect].width(), dsts[rect].height(), (Graphics::TSpriteBlendMode)blendMode);
            newSurfDest.fillRect(Common::Rect(0, 0, newSurfDest.w, newSurfDest.h), newSurfDest.format.ARGBToColor(ba, br, bg, bb));
            newSurf.setAlphaMode((Graphics::AlphaType)alphaType);
            newSurf.blit(newSurfDest, dsts[rect].left, dsts[rect].top, flipping, &srcs[rect], TS_ARGB(a, r, g, b), dsts[rect].width(), dsts[rect].height(), (Graphics::TSpriteBlendMode)blendMode);
            managedSurfDest.fillRect(Common::Rect(0, 0, managedSurfDest.w, managedSurfDest.h), managedSurfDest.format.ARGBToColor(ba, br, bg, bb));
            managedSurfDest.blendBlitFrom(managedSurf, srcs[rect], dsts[rect], flipping, BLENDBLIT_ARGB(a, r, g, b), (Graphics::TSpriteBlendMode)blendMode, (Graphics::AlphaType)alphaType);



            if (!areSurfacesEqual(&oldSurfDest, &newSurfDest)) {
                warning("blendMode: %s, alphaType: %s, a: %d, r: %d, g: %d, b: %d, flipping: %s, test rect id: %s",
                    blendModes[blendMode], alphaTypes[alphaType], a, r, g, b, flipNames[flipping], rectNames[rect]);
                save_bitmap("sourceSurf.bmp", &newSurf);
                save_bitmap("oldSurfDest.bmp", &oldSurfDest);
                save_bitmap("newSurfDest.bmp", &newSurfDest);
                save_bitmap("managedSurfDest.bmp", managedSurfDest.surfacePtr());
                TS_FAIL("oldSurfDest and newSurfDest are not equal!");
                return;
            }
            if (!areSurfacesEqual(&oldSurfDest, managedSurfDest.surfacePtr())) {
                warning("blendMode: %s, alphaType: %s, a: %d, r: %d, g: %d, b: %d, flipping: %s, test rect id: %s",
                    blendModes[blendMode], alphaTypes[alphaType], a, r, g, b, flipNames[flipping], rectNames[rect]);
                save_bitmap("sourceSurf.bmp", &newSurf);
                save_bitmap("oldSurfDest.bmp", &oldSurfDest);
                save_bitmap("newSurfDest.bmp", &newSurfDest);
                save_bitmap("managedSurfDest.bmp", managedSurfDest.surfacePtr());
                TS_FAIL("oldSurfDest and managedSurfDest are not equal!");
                return;
            }
            if (!areSurfacesEqual(&newSurfDest, managedSurfDest.surfacePtr())) {
                warning("blendMode: %s, alphaType: %s, a: %d, r: %d, g: %d, b: %d, flipping: %s, test rect id: %s",
                    blendModes[blendMode], alphaTypes[alphaType], a, r, g, b, flipNames[flipping], rectNames[rect]);
                save_bitmap("sourceSurf.bmp", &newSurf);
                save_bitmap("oldSurfDest.bmp", &oldSurfDest);
                save_bitmap("newSurfDest.bmp", &newSurfDest);
                save_bitmap("managedSurfDest.bmp", managedSurfDest.surfacePtr());
                TS_FAIL("newSurfDest and managedSurfDest are not equal!");
                return;
            }
        } // rect
        } // flipping
        } // b
        } // g
        } // r
        } // a
        } // bb
        } // bg
        } // br
        } // ba
        } // alpha
        } // blend

	    baseSurface.free();
    }
};
