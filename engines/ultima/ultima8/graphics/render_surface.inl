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
// #define FLIP_SHAPES to flip rendering
//
// #define FLIP_CONDITIONAL to an argument of the function so FLIPPING can be
// enabled/disabled with a bool
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
// NOT_CLIPPED_Y - Does Y Clipping check per line
//
// NOT_CLIPPED_X - Does X Clipping check per Pixel
//
// LINE_END_ASSIGN - Calcuates the line_end pointer required for X clipping
//
// XNEG - Negates X values if doing shape flipping
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
#define USE_XFORM_FUNC ((XFORM_CONDITIONAL) && xform_map[*srcpix])
#else
#define USE_XFORM_FUNC (xform_map[*srcpix])
#endif

//
// XForm = FALSE
//
#else
#define USE_XFORM_FUNC 0
#endif


//
// Flipping = TRUE
//
#ifdef FLIP_SHAPES

#ifdef FLIP_CONDITIONAL
const int32 neg = (FLIP_CONDITIONAL)?-1:0;
#define XNEG(x) (((x)+neg)^neg)
#else
#define XNEG(x) (-(x))
#endif

// Flipping = FALSE
#else
#define XNEG(x)(+(x))
#endif


//
// No Clipping = TRUE
//
#ifdef NO_CLIPPING

#define LINE_END_ASSIGN //
#define NOT_CLIPPED_X (1)
#define NOT_CLIPPED_Y (1)
#define OFFSET_PIXELS (pixels)

//
// No Clipping = FALSE
//
#else

	const int		scrn_width = clipWindow.width();
	const int		scrn_height = clipWindow.height();

#define LINE_END_ASSIGN const uintX *dst_line_end = dst_line_start + scrn_width
#define NOT_CLIPPED_X (dstpix >= dst_line_start && dstpix < dst_line_end)
#define NOT_CLIPPED_Y (line >= 0 && line < scrn_height)
#define OFFSET_PIXELS (off_pixels)

	uint8			*off_pixels  = pixels + clipWindow.left * sizeof(uintX) + clipWindow.top * pitch;
	x -= clipWindow.left;
	y -= clipWindow.top;

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
// Destination Alpha Masking
//
#ifdef DESTALPHA_MASK

#define NOT_DESTINATION_MASKED	(*pixptr & RenderSurface::_format.aMask)

#else

#define NOT_DESTINATION_MASKED	(1)

#endif

//
// The Function
//

	const uint8	keycolor = frame->_keycolor;

	const Graphics::Surface &src =  frame->getSurface();
	const uint8 *srcpixels = reinterpret_cast<const uint8 *>(src.getPixels());

	const int width_ = src.w;
	const int height_ = src.h;
	x -= XNEG(frame->_xoff);
	y -= frame->_yoff;

	assert(pixels && srcpixels);

	for (int i = 0; i < height_; i++)  {
		const int line = y + i;

		if (NOT_CLIPPED_Y) {
			const uint8	*srcline = srcpixels + i * width_;
			uintX *dst_line_start = reinterpret_cast<uintX *>(OFFSET_PIXELS + pitch * line);
			LINE_END_ASSIGN;

			for (int xpos = 0; xpos < width_; xpos++) {
				if (srcline[xpos] == keycolor)
					continue;

				uintX *dstpix = dst_line_start + x + XNEG(xpos);

				if (NOT_CLIPPED_X && NOT_DESTINATION_MASKED) {
					const uint8 *srcpix = srcline + xpos;
					#ifdef XFORM_SHAPES
					if (USE_XFORM_FUNC) {
						*dstpix = CUSTOM_BLEND(BlendPreModulated(xform_map[*srcpix], *dstpix, format));
					}
					else
					#endif
					{
						*dstpix = CUSTOM_BLEND(map[*srcpix]);
					}
				}
			}
		}
	}

#undef NOT_DESTINATION_MASKED
#undef OFFSET_PIXELS
#undef CUSTOM_BLEND
#undef LINE_END_ASSIGN
#undef NOT_CLIPPED_X
#undef NOT_CLIPPED_Y
#undef XNEG
#undef USE_XFORM_FUNC
