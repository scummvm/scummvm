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
 *
 */

#ifndef GRAPHICS_TRANSPARENTSURFACE_H
#define GRAPHICS_TRANSPARENTSURFACE_H

#include "graphics/surface.h"
#include "graphics/transform_struct.h"

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifdef SCUMM_LITTLE_ENDIAN
#define TS_RGB(R,G,B)       (uint32)((0xff << 24) | ((R) << 16) | ((G) << 8) | (B))
#define TS_ARGB(A,R,G,B)    (uint32)(((R) << 24) | ((G) << 16) | ((B) << 8) | (A))
#else
#define TS_RGB(R,G,B)       (uint32)(((R) << 24) | ((G) << 16) | (B << 8) | 0xff)
#define TS_ARGB(A,R,G,B)    (uint32)(((R) << 24) | ((G) << 16) | ((B) << 8) | (A))
#endif

namespace Graphics {

/**
 * @defgroup graphics_transparent_surface Transparent surface
 * @ingroup graphics
 *
 * @brief TransparentSurface class.
 *
 * @{
 */

// Enums
/**
 @brief The possible flipping parameters for the blit method.
 */
enum FLIP_FLAGS {
	/// The image will not be flipped.
	FLIP_NONE = 0,
	/// The image will be flipped at the horizontal axis.
	FLIP_H = 1,
	/// The image will be flipped at the vertical axis.
	FLIP_V = 2,
	/// The image will be flipped at the horizontal and vertical axis.
	FLIP_HV = FLIP_H | FLIP_V,
	/// The image will be flipped at the horizontal and vertical axis.
	FLIP_VH = FLIP_H | FLIP_V
};

enum AlphaType {
	ALPHA_OPAQUE = 0,
	ALPHA_BINARY = 1,
	ALPHA_FULL = 2
};

enum TFilteringMode {
	FILTER_NEAREST = 0,
	FILTER_BILINEAR = 1
};

/**
 * A transparent graphics surface, which implements alpha blitting.
 */
struct TransparentSurface : public Graphics::Surface {
	TransparentSurface();
	TransparentSurface(const Graphics::Surface &surf, bool copyData = false);

	/**
	 * Returns the pixel format all operations of TransparentSurface support.
	 *
	 * Unlike Surface TransparentSurface only works with a fixed pixel format.
	 * This format can be queried using this static function.
	 *
	 * @return Supported pixel format.
	 */
	static PixelFormat getSupportedPixelFormat() {
		return PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	}

	/**
	 @brief renders the surface to another surface
	 @param target a pointer to the target surface. In most cases this is the framebuffer.
	 @param posX the position on the X-axis in the target image in pixels where the image is supposed to be rendered.<br>
	 The default value is 0.
	 @param posY the position on the Y-axis in the target image in pixels where the image is supposed to be rendered.<br>
	 The default value is 0.
	 @param flipping how the the image should be flipped.<br>
	 The default value is Graphics::FLIP_NONE (no flipping)
	 @param pPartRect Pointer on Common::Rect which specifies the section to be rendered. If the whole image has to be rendered the Pointer is NULL.<br>
	 This referes to the unflipped and unscaled image.<br>
	 The default value is NULL.
	 @param color an ARGB color value, which determines the parameters for the color modulation und alpha blending.<br>
	 The alpha component of the color determines the alpha blending parameter (0 = no covering, 255 = full covering).<br>
	 The color components determines the color for color modulation.<br>
	 The default value is TS_ARGB(255, 255, 255, 255) (full covering, no color modulation).
	 The macros TS_RGB and TS_ARGB can be used for the creation of the color value.
	 @param width the output width of the screen section.
	 The images will be scaled if the output width of the screen section differs from the image section.<br>
	 The value -1 determines that the image should not be scaled.<br>
	 The default value is -1.
	 @param height the output height of the screen section.
	 The images will be scaled if the output width of the screen section differs from the image section.<br>
	 The value -1 determines that the image should not be scaled.<br>
	 The default value is -1.
	 @return returns false if the rendering failed.
	 */
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

	void applyColorKey(uint8 r, uint8 g, uint8 b, bool overwriteAlpha = false);
	void setAlpha(uint8 alpha, bool skipTransparent = false);

	/**
	 * @brief Scale function; this returns a transformed version of this surface after rotation and
	 * scaling. Please do not use this if angle != 0, use rotoscale.
	 *
	 * @param newWidth the resulting width.
	 * @param newHeight the resulting height.
	 * @param filtering Whether or not to use bilinear filtering.
	 * @see TransformStruct
	 */
	TransparentSurface *scale(uint16 newWidth, uint16 newHeight, bool filtering = false) const;

	/**
	 * @brief Rotoscale function; this returns a transformed version of this surface after rotation and
	 * scaling. Please do not use this if angle == 0, use plain old scaling function.
	 *
	 * @param transform a TransformStruct wrapping the required info. @see TransformStruct
	 *
	 */
	template <TFilteringMode filteringMode>
	TransparentSurface *rotoscaleT(const TransformStruct &transform) const;

	TransparentSurface *rotoscale(const TransformStruct &transform) const;

	TransparentSurface *convertTo(const PixelFormat &dstFormat, const byte *palette = 0) const;

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

/**
 * A deleter for Surface objects which can be used with SharedPtr.
 *
 * This deleter assures Surface::free is called on deletion.
 */
/*struct SharedPtrTransparentSurfaceDeleter {
    void operator()(TransparentSurface *ptr) {
        ptr->free();
        delete ptr;
    }
};*/
/** @} */
} // End of namespace Graphics


#endif
