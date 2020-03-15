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
#define USE_XFORM_FUNC ((XFORM_CONDITIONAL) && xform_pal[*linedata])
#else
#define USE_XFORM_FUNC (xform_pal[*linedata])
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

#define LINE_END_ASSIGN()
#define NOT_CLIPPED_X (1)
#define NOT_CLIPPED_Y (1)
#define OFFSET_PIXELS (_pixels)

//
// No Clipping = FALSE
//	
#else

#define LINE_END_ASSIGN() do { line_end = line_start+scrn_width; } while (0)
#define NOT_CLIPPED_Y (line >= 0 && line < scrn_height)
#define NOT_CLIPPED_X (pixptr >= line_start && pixptr < line_end)

	int					scrn_width = _clipWindow.w;
	int					scrn_height = _clipWindow.h;
	uintX				*line_end;

#define OFFSET_PIXELS (off_pixels)

	uint8				*off_pixels  = static_cast<uint8*>(_pixels) + static_cast<sintptr>(_clipWindow.x)*sizeof(uintX) + static_cast<sintptr>(_clipWindow.y)*_pitch;
	x -= _clipWindow.x;
	y -= _clipWindow.y;

#endif


//
// Invisilibity = TRUE
//
#ifdef BLEND_SHAPES

#ifdef BLEND_CONDITIONAL
#define CUSTOM_BLEND(src) static_cast<uintX>((BLEND_CONDITIONAL)?BLEND_SHAPES(src,*pixptr):src)
#else
#define CUSTOM_BLEND(src) static_cast<uintX>(BLEND_SHAPES(src,*pixptr))
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

#define NOT_DESTINATION_MASKED	(*pixptr & RenderSurface::_format.a_mask)

#else

#define NOT_DESTINATION_MASKED	(1)

#endif

//
// The Function
//

// All the variables we want

	const uint8			*linedata;
	int32				xpos;
	sintptr				line; // sintptr for pointer arithmetic
	int32				dlen;

	uintX				*pixptr;
	uintX				*endrun;
	uintX				*line_start;
	uint32				pix;

	// Sanity check
	if (framenum >= s->frameCount())
		return;
	if (s->getPalette() == 0)
		return;

	ShapeFrame		*frame			= s->getFrame(framenum);
	const uint8		*rle_data		= frame->_rle_data;
	const uint32	*line_offsets	= frame->_line_offsets;
	const uint32	*pal			= untformed_pal?
										&(s->getPalette()->_native_untransformed[0]):
										&(s->getPalette()->_native[0]);

	
#ifdef XFORM_SHAPES
	const uint32	*xform_pal		= untformed_pal?
										&(s->getPalette()->_xform_untransformed[0]):
										&(s->getPalette()->_xform[0]);
#endif

	int32 width_ = frame->_width;
	int32 height_ = frame->_height;
	x -= XNEG(frame->_xoff);
	y -= frame->_yoff;

	// Do it this way if compressed
	if (frame->_compressed) for (int i=0; i<height_; i++)  {
		xpos = 0;
		line = y+i;

		if (NOT_CLIPPED_Y) {

			linedata = rle_data + line_offsets[i];
			line_start = reinterpret_cast<uintX *>(static_cast<uint8*>(OFFSET_PIXELS) + _pitch*line);

			LINE_END_ASSIGN();

			do {
				xpos += *linedata++;
			  
				if (xpos == width_) break;

				dlen = *linedata++;
				int type = dlen & 1;
				dlen >>= 1;

				pixptr = line_start+x+XNEG(xpos);
				endrun = pixptr + XNEG(dlen);
				
				if (!type) {
					while (pixptr != endrun)  {
						if (NOT_CLIPPED_X && NOT_DESTINATION_MASKED)  {
							#ifdef XFORM_SHAPES
							if (USE_XFORM_FUNC) {
								*pixptr = CUSTOM_BLEND(BlendPreModulated(xform_pal[*linedata],*pixptr));
							} else 
							#endif
							{
								*pixptr = CUSTOM_BLEND(pal[*linedata]);
							}
						}
						pixptr += XNEG(1);
						linedata++;
					}
				} else {
					#ifdef XFORM_SHAPES
					pix = xform_pal[*linedata];
					if (USE_XFORM_FUNC) {
						while (pixptr != endrun) {
							if (NOT_CLIPPED_X && NOT_DESTINATION_MASKED) *pixptr = CUSTOM_BLEND(BlendPreModulated(xform_pal[*linedata],*pixptr));
							pixptr += XNEG(1);
						}
					} else 
					#endif
					{
						pix = pal[*linedata];
						while (pixptr != endrun) 
						{
							if (NOT_CLIPPED_X && NOT_DESTINATION_MASKED) 
							{
								*pixptr = CUSTOM_BLEND(pix);
							}
							pixptr += XNEG(1);
						}
					}	
					linedata++;
				}

				xpos += dlen;

			} while (xpos < width_);
		}
	}
	// Uncompressed
	else for (int i=0; i<height_; i++)  {
		linedata = rle_data + line_offsets[i];
		xpos = 0;
		line = y+i;

		if (NOT_CLIPPED_Y) {
			line_start = reinterpret_cast<uintX *>(static_cast<uint8*>(OFFSET_PIXELS) + _pitch*line);
			LINE_END_ASSIGN();

			do {
				xpos += *linedata++;
			  
				if (xpos == width_) break;

				dlen = *linedata++;

				pixptr= line_start+x+XNEG(xpos);
				endrun = pixptr + XNEG(dlen);

				while (pixptr != endrun) {
					if (NOT_CLIPPED_X && NOT_DESTINATION_MASKED) {
						#ifdef XFORM_SHAPES
						if (USE_XFORM_FUNC) {
							*pixptr = CUSTOM_BLEND(BlendPreModulated(xform_pal[*linedata],*pixptr));
						}
						else 
						#endif
						{
							*pixptr = CUSTOM_BLEND(pal[*linedata]);
						}
					}
					pixptr += XNEG(1);
					linedata++;
				}

				xpos += dlen;

			} while (xpos < width_);
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
