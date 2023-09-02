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

//
// Render Surface Shape Display include file
//

//
// Macros to define before including this:
//
// #define NO_CLIPPING to disable shape clipping
//
// #define XFORM_SHAPES to enable XFORMing
//
// #define XFORM_CONDITIONAL to an argument of the function so XFORM can be
// enabled/disabled with a bool
//
// #define BLEND_SHAPES(src,dst) to an a specified blend function.
//
// #define BLEND_CONDITIONAL to an argument of the function so BLEND
// painting can be enabled/disabled with a bool
//

//
// Macros defined by this file:
//
// USE_XFORM_FUNC - Checks to see if we want to use XForm Blending for this pixel
//
// CUSTOM_BLEND - Final Blend for invisiblity
//

//
// XForm = TRUE
//
#ifdef XFORM_SHAPES

#ifdef XFORM_CONDITIONAL
#define USE_XFORM_FUNC ((XFORM_CONDITIONAL) && xform_map[color])
#else
#define USE_XFORM_FUNC (xform_map[color])
#endif

//
// XForm = FALSE
//
#else
#define USE_XFORM_FUNC 0
#endif

//
// Invisilibity = TRUE
//
#ifdef BLEND_SHAPES

#ifdef BLEND_CONDITIONAL
#define CUSTOM_BLEND(src) static_cast<uintX>((BLEND_CONDITIONAL)?BLEND_SHAPES(src,*dstpix):src)
#else
#define CUSTOM_BLEND(src) static_cast<uintX>(BLEND_SHAPES(src,*dstpix))
#endif

//
// Invisilibity = FALSE
//
#else

#define CUSTOM_BLEND(src) static_cast<uintX>(src)

#endif

//
// The Function
//

	const Graphics::Surface &src =  frame->getSurface();
	const uint8 keycolor = frame->_keycolor;

	Common::Rect srcRect(0, 0, src.w, src.h);
	Common::Rect dstRect(x, y, x, y + src.h);

	const int srcStep = sizeof(uint8);
	int dstStep = sizeof(uintX);

	if (mirrored) {
		dstRect.translate(frame->_xoff, -frame->_yoff);
		dstRect.left = dstRect.right - src.w;
		dstStep = -dstStep;
	} else {
		dstRect.translate(-frame->_xoff, -frame->_yoff);
		dstRect.right = dstRect.left + src.w;
	}

#ifndef NO_CLIPPING

	if (dstRect.left < clipWindow.left) {
		if (mirrored) {
			srcRect.right += dstRect.left - clipWindow.left;
		} else {
			srcRect.left -= dstRect.left - clipWindow.left;
		}
		dstRect.left = clipWindow.left;
	}
	if (dstRect.top < clipWindow.top) {
		srcRect.top -= dstRect.top - clipWindow.top;
		dstRect.top = clipWindow.top;
	}
	if (dstRect.right > clipWindow.right) {
		if (mirrored) {
			srcRect.left += dstRect.right - clipWindow.right;
		} else {
			srcRect.right -= dstRect.right - clipWindow.right;
		}
		dstRect.right = clipWindow.right;
	}
	if (dstRect.bottom > clipWindow.bottom) {
		srcRect.bottom -= dstRect.bottom - clipWindow.bottom;
		dstRect.bottom = clipWindow.bottom;
	}

#endif

	const int w = srcRect.width();
	const int h = srcRect.height();
	const int srcDelta = src.pitch - (w * srcStep);
	const int dstDelta = pitch - (w * dstStep);

	const uint8 *srcPixels = reinterpret_cast<const uint8 *>(src.getBasePtr(srcRect.left, srcRect.top));
	uint8 *dstPixels = reinterpret_cast<uint8 *>(pixels + (mirrored ? dstRect.right - 1 : dstRect.left) * sizeof(uintX) + pitch * dstRect.top);

	for (int i = 0; i < h; i++)  {
		for (int j = 0; j < w; j++) {
			const uint8 color = *srcPixels;
			if (color != keycolor) {
				uintX *dstpix = reinterpret_cast<uintX *>(dstPixels);
				#ifdef XFORM_SHAPES
				if (USE_XFORM_FUNC) {
					*dstpix = CUSTOM_BLEND(BlendPreModulated(xform_map[color], *dstpix, format));
				}
				else
				#endif
				{
					*dstpix = CUSTOM_BLEND(map[color]);
				}
			}
			srcPixels += srcStep;
			dstPixels += dstStep;
		}

		srcPixels += srcDelta;
		dstPixels += dstDelta;
	}

#undef CUSTOM_BLEND
#undef USE_XFORM_FUNC
