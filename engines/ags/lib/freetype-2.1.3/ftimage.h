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

/***************************************************************************/
/*                                                                         */
/*  ftimage.h                                                              */
/*    FreeType glyph image formats and default raster interface            */
/*    (specification).                                                     */
/*                                                                         */
/***************************************************************************/

/*************************************************************************/
/*                                                                       */
/* Note: A `raster' is simply a scan-line converter, used to render      */
/*       FT_Outlines into FT_Bitmaps.                                    */
/*                                                                       */
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTIMAGE_H
#define AGS_LIB_FREETYPE_FTIMAGE_H


/* _STANDALONE_ is from ftgrays.c */
#ifndef _STANDALONE_
#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#endif

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


/**** BASIC TYPES ****/

typedef signed long FT_Pos;

typedef struct FT_Vector_ {
	FT_Pos x;
	FT_Pos y;
} FT_Vector;

typedef struct FT_BBox_ {
	FT_Pos xMin, yMin;
	FT_Pos xMax, yMax;
} FT_BBox;

typedef enum FT_Pixel_Mode_ {
	FT_PIXEL_MODE_NONE = 0,
	FT_PIXEL_MODE_MONO,
	FT_PIXEL_MODE_GRAY,
	FT_PIXEL_MODE_GRAY2,
	FT_PIXEL_MODE_GRAY4,
	FT_PIXEL_MODE_LCD,
	FT_PIXEL_MODE_LCD_V,

	FT_PIXEL_MODE_MAX /* do not remove */
} FT_Pixel_Mode;


/*    A list of deprecated constants. */
#define ft_pixel_mode_none   FT_PIXEL_MODE_NONE
#define ft_pixel_mode_mono   FT_PIXEL_MODE_MONO
#define ft_pixel_mode_grays  FT_PIXEL_MODE_GRAY
#define ft_pixel_mode_pal2   FT_PIXEL_MODE_GRAY2
#define ft_pixel_mode_pal4   FT_PIXEL_MODE_GRAY4


#if 0
/* THIS TYPE IS DEPRECATED  */
typedef enum  FT_Palette_Mode_ {
	ft_palette_mode_rgb = 0,
	ft_palette_mode_rgba,

	ft_palettte_mode_max   /* do not remove */

} FT_Palette_Mode;
#endif


typedef struct  FT_Bitmap_ {
	int             rows;
	int             width;
	int             pitch;
	unsigned char   *buffer;
	short           num_grays;
	char            pixel_mode;
	char            palette_mode;
	void            *palette;
} FT_Bitmap;


/**** OUTLINE PROCESSING ****/

typedef struct  FT_Outline_ {
	short       n_contours;      /* number of contours in glyph        */
	short       n_points;        /* number of points in the glyph      */

	FT_Vector   *points;         /* the outline's points               */
	char        *tags;           /* the points flags                   */
	short       *contours;       /* the contour end points             */

	int         flags;           /* outline masks                      */
} FT_Outline;

typedef enum  FT_Outline_Flags_ {
	FT_OUTLINE_NONE            = 0,
	FT_OUTLINE_OWNER           = 1,
	FT_OUTLINE_EVEN_ODD_FILL   = 2,
	FT_OUTLINE_REVERSE_FILL    = 4,
	FT_OUTLINE_IGNORE_DROPOUTS = 8,
	FT_OUTLINE_HIGH_PRECISION  = 256,
	FT_OUTLINE_SINGLE_PASS     = 512
} FT_Outline_Flags;


/*   These constants are deprecated. */
#define ft_outline_none             FT_OUTLINE_NONE
#define ft_outline_owner            FT_OUTLINE_OWNER
#define ft_outline_even_odd_fill    FT_OUTLINE_EVEN_ODD_FILL
#define ft_outline_reverse_fill     FT_OUTLINE_REVERSE_FILL
#define ft_outline_ignore_dropouts  FT_OUTLINE_IGNORE_DROPOUTS
#define ft_outline_high_precision   FT_OUTLINE_HIGH_PRECISION
#define ft_outline_single_pass      FT_OUTLINE_SINGLE_PASS


#define FT_CURVE_TAG(flag) (flag & 3)

#define FT_CURVE_TAG_ON           1
#define FT_CURVE_TAG_CONIC        0
#define FT_CURVE_TAG_CUBIC        2

#define FT_CURVE_TAG_TOUCH_X      8  /* reserved for the TrueType hinter */
#define FT_CURVE_TAG_TOUCH_Y     16  /* reserved for the TrueType hinter */

#define FT_CURVE_TAG_TOUCH_BOTH	(FT_CURVE_TAG_TOUCH_X | FT_CURVE_TAG_TOUCH_Y)

#define  FT_Curve_Tag_On       FT_CURVE_TAG_ON
#define  FT_Curve_Tag_Conic    FT_CURVE_TAG_CONIC
#define  FT_Curve_Tag_Cubic    FT_CURVE_TAG_CUBIC
#define  FT_Curve_Tag_Touch_X  FT_CURVE_TAG_TOUCH_X
#define  FT_Curve_Tag_Touch_Y  FT_CURVE_TAG_TOUCH_Y


typedef int (*FT_Outline_MoveToFunc)(FT_Vector *to, void *user);

#define FT_Outline_MoveTo_Func FT_Outline_MoveToFunc

typedef int (*FT_Outline_LineToFunc)(FT_Vector *to, void *user);

#define FT_Outline_LineTo_Func FT_Outline_LineToFunc

typedef int (*FT_Outline_ConicToFunc)(FT_Vector *control, FT_Vector *to, void *user);

#define FT_Outline_ConicTo_Func FT_Outline_ConicToFunc

typedef int (*FT_Outline_CubicToFunc)(FT_Vector *control1, FT_Vector *control2, FT_Vector *to, void *user);

#define FT_Outline_CubicTo_Func FT_Outline_CubicToFunc

typedef struct  FT_Outline_Funcs_ {
	FT_Outline_MoveToFunc   move_to;
	FT_Outline_LineToFunc   line_to;
	FT_Outline_ConicToFunc  conic_to;
	FT_Outline_CubicToFunc  cubic_to;

	int                     shift;
	FT_Pos                  delta;
} FT_Outline_Funcs;


#ifndef FT_IMAGE_TAG
#define FT_IMAGE_TAG(value, _x1, _x2, _x3, _x4) \
	value = (((unsigned long)_x1 << 24) |       \
			 ((unsigned long)_x2 << 16) |       \
			 ((unsigned long)_x3 << 8) |        \
			 (unsigned long)_x4)
#endif /* FT_IMAGE_TAG */

typedef enum  FT_Glyph_Format_ {
	FT_IMAGE_TAG(FT_GLYPH_FORMAT_NONE, 0, 0, 0, 0),

	FT_IMAGE_TAG(FT_GLYPH_FORMAT_COMPOSITE, 'c', 'o', 'm', 'p'),
	FT_IMAGE_TAG(FT_GLYPH_FORMAT_BITMAP,    'b', 'i', 't', 's'),
	FT_IMAGE_TAG(FT_GLYPH_FORMAT_OUTLINE,   'o', 'u', 't', 'l'),
	FT_IMAGE_TAG(FT_GLYPH_FORMAT_PLOTTER,   'p', 'l', 'o', 't')
} FT_Glyph_Format;


#define ft_glyph_format_none       FT_GLYPH_FORMAT_NONE
#define ft_glyph_format_composite  FT_GLYPH_FORMAT_COMPOSITE
#define ft_glyph_format_bitmap     FT_GLYPH_FORMAT_BITMAP
#define ft_glyph_format_outline    FT_GLYPH_FORMAT_OUTLINE
#define ft_glyph_format_plotter    FT_GLYPH_FORMAT_PLOTTER


/**** RASTER DEFINITIONS ****/

/*************************************************************************/
/*                                                                       */
/* A raster is a scan converter, in charge of rendering an outline into  */
/* a a bitmap.  This section contains the public API for rasters.        */
/*                                                                       */
/* Note that in FreeType 2, all rasters are now encapsulated within      */
/* specific modules called `renderers'.  See `freetype/ftrender.h' for   */
/* more details on renderers.                                            */
/*                                                                       */
/*************************************************************************/

typedef struct FT_RasterRec_ *FT_Raster;

typedef struct  FT_Span_ {
	short           x;
	unsigned short  len;
	unsigned char   coverage;
} FT_Span;

typedef void (*FT_SpanFunc)(int y, int count, FT_Span *spans, void *user);

#define FT_Raster_Span_Func FT_SpanFunc

typedef int (*FT_Raster_BitTest_Func)(int y, int x, void *user);

typedef void (*FT_Raster_BitSet_Func)(int y, int x, void *user);

typedef  enum {
	FT_RASTER_FLAG_DEFAULT = 0,
	FT_RASTER_FLAG_AA      = 1,
	FT_RASTER_FLAG_DIRECT  = 2,
	FT_RASTER_FLAG_CLIP    = 4
} FT_Raster_Flag;

#define ft_raster_flag_default  FT_RASTER_FLAG_DEFAULT
#define ft_raster_flag_aa       FT_RASTER_FLAG_AA
#define ft_raster_flag_direct   FT_RASTER_FLAG_DIRECT
#define ft_raster_flag_clip     FT_RASTER_FLAG_CLIP

typedef struct FT_Raster_Params_ {
	FT_Bitmap 				*target;
	void 					*source;
	int 					flags;
	FT_SpanFunc 			gray_spans;
	FT_SpanFunc 			black_spans;
	FT_Raster_BitTest_Func 	bit_test; /* doesn't work! */
	FT_Raster_BitSet_Func 	bit_set;  /* doesn't work! */
	void 					*user;
	FT_BBox 				clip_box;
} FT_Raster_Params;

typedef int (*FT_Raster_NewFunc)(void *memory, FT_Raster *raster);

#define FT_Raster_New_Func FT_Raster_NewFunc

typedef void (*FT_Raster_DoneFunc)(FT_Raster raster);

#define FT_Raster_Done_Func FT_Raster_DoneFunc

typedef void (*FT_Raster_ResetFunc)(FT_Raster raster, unsigned char *pool_base, unsigned long pool_size);

#define FT_Raster_Reset_Func FT_Raster_ResetFunc

typedef int (*FT_Raster_SetModeFunc)(FT_Raster raster, unsigned long mode, void *args);

#define FT_Raster_Set_Mode_Func FT_Raster_SetModeFunc

typedef int (*FT_Raster_RenderFunc)(FT_Raster raster, FT_Raster_Params *params);

#define FT_Raster_Render_Func FT_Raster_RenderFunc

typedef struct  FT_Raster_Funcs_ {
	FT_Glyph_Format         glyph_format;
	FT_Raster_NewFunc       raster_new;
	FT_Raster_ResetFunc     raster_reset;
	FT_Raster_SetModeFunc   raster_set_mode;
	FT_Raster_RenderFunc    raster_render;
	FT_Raster_DoneFunc      raster_done;
} FT_Raster_Funcs;


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTIMAGE_H */
