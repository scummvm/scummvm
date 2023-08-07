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

namespace Graphics {

/**
 * @defgroup graphics_transparent_surface Transparent surface
 * @ingroup graphics
 *
 * @brief TransparentSurface class.
 *
 * @{
 */

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
	 @param flipping how the image should be flipped.<br>
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

	/**
	 * @brief Scale function; this returns a transformed version of this surface after rotation and
	 * scaling. Please do not use this if angle != 0, use rotoscale.
	 *
	 * @param newWidth the resulting width.
	 * @param newHeight the resulting height.
	 * @param filtering Whether or not to use bilinear filtering.
	 * @see TransformStruct
	 */
	TransparentSurface *scale(int16 newWidth, int16 newHeight, bool filtering = false) const;

	/**
	 * @brief Rotoscale function; this returns a transformed version of this surface after rotation and
	 * scaling. Please do not use this if angle == 0, use plain old scaling function.
	 *
	 * @param transform a TransformStruct wrapping the required info. @see TransformStruct
	 * @param filtering Whether or not to use bilinear filtering.
	 *
	 */
	TransparentSurface *rotoscale(const TransformStruct &transform, bool filtering = false) const;

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
