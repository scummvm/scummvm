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
/*  freetype.h                                                             */
/*                                                                         */
/*    FreeType high-level API and common types (specification only).       */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FREETYPE_H
#define AGS_LIB_FREETYPE_FREETYPE_H


/*************************************************************************/
/*                                                                       */
/* The `raster' component duplicates some of the declarations in         */
/* freetype.h for stand-alone use if _FREETYPE_ isn't defined.           */
/*                                                                       */


/*************************************************************************/
/*                                                                       */
/* The FREETYPE_MAJOR and FREETYPE_MINOR macros are used to version the  */
/* new FreeType design, which is able to host several kinds of font      */
/* drivers.  It starts at 2.0.                                           */
/*                                                                       */
#define FREETYPE213_MAJOR 2
#define FREETYPE213_MINOR 1
#define FREETYPE213_PATCH 3


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/config/ftconfig.h"
#include "engines/ags/lib/freetype-2.1.3/fterrors.h"
#include "engines/ags/lib/freetype-2.1.3/fttypes.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER

typedef struct FT_Glyph_Metrics_ {
	FT_Pos width;  /* glyph width  */
	FT_Pos height; /* glyph height */

	FT_Pos horiBearingX; /* left side bearing in horizontal layouts */
	FT_Pos horiBearingY; /* top side bearing in horizontal layouts  */
	FT_Pos horiAdvance;  /* advance width for horizontal layout     */

	FT_Pos vertBearingX; /* left side bearing in vertical layouts */
	FT_Pos vertBearingY; /* top side bearing in vertical layouts  */
	FT_Pos vertAdvance;  /* advance height for vertical layout    */
} FT_Glyph_Metrics;

typedef struct FT_Bitmap_Size_ {
	FT_Short height;
	FT_Short width;
} FT_Bitmap_Size;


typedef struct FT_LibraryRec_ *FT_Library;
typedef struct FT_ModuleRec_ *FT_Module;
typedef struct FT_DriverRec_ *FT_Driver;
typedef struct FT_RendererRec_ *FT_Renderer;
typedef struct FT_FaceRec_ *FT_Face;
typedef struct FT_SizeRec_ *FT_Size;
typedef struct FT_GlyphSlotRec_ *FT_GlyphSlot;
typedef struct FT_CharMapRec_ *FT_CharMap;


#ifndef FT_ENC_TAG
#define FT_ENC_TAG(value, a, b, c, d) \
	value = (((FT_UInt32)(a) << 24) |      \
			 ((FT_UInt32)(b) << 16) |      \
			 ((FT_UInt32)(c) << 8) |       \
			 (FT_UInt32)(d))
#endif


typedef enum FT_Encoding_ {
	FT_ENC_TAG(FT_ENCODING_NONE, 0, 0, 0, 0),

	FT_ENC_TAG(FT_ENCODING_MS_SYMBOL, 's', 'y', 'm', 'b'),
	FT_ENC_TAG(FT_ENCODING_UNICODE, 'u', 'n', 'i', 'c'),

	FT_ENC_TAG(FT_ENCODING_MS_SJIS, 's', 'j', 'i', 's'),
	FT_ENC_TAG(FT_ENCODING_MS_GB2312, 'g', 'b', ' ', ' '),
	FT_ENC_TAG(FT_ENCODING_MS_BIG5, 'b', 'i', 'g', '5'),
	FT_ENC_TAG(FT_ENCODING_MS_WANSUNG, 'w', 'a', 'n', 's'),
	FT_ENC_TAG(FT_ENCODING_MS_JOHAB, 'j', 'o', 'h', 'a'),

	FT_ENC_TAG(FT_ENCODING_ADOBE_STANDARD, 'A', 'D', 'O', 'B'),
	FT_ENC_TAG(FT_ENCODING_ADOBE_EXPERT, 'A', 'D', 'B', 'E'),
	FT_ENC_TAG(FT_ENCODING_ADOBE_CUSTOM, 'A', 'D', 'B', 'C'),
	FT_ENC_TAG(FT_ENCODING_ADOBE_LATIN_1, 'l', 'a', 't', '1'),

	FT_ENC_TAG(FT_ENCODING_OLD_LATIN_2, 'l', 'a', 't', '2'),

	FT_ENC_TAG(FT_ENCODING_APPLE_ROMAN, 'a', 'r', 'm', 'n')
} FT_Encoding;

/*    These constants are deprecated; use the corresponding @FT_Encoding */
/*    values instead.                                                    */
#define  ft_encoding_none            FT_ENCODING_NONE
#define  ft_encoding_unicode         FT_ENCODING_UNICODE
#define  ft_encoding_symbol          FT_ENCODING_MS_SYMBOL
#define  ft_encoding_latin_1         FT_ENCODING_ADOBE_LATIN_1
#define  ft_encoding_latin_2         FT_ENCODING_OLD_LATIN_2
#define  ft_encoding_sjis            FT_ENCODING_MS_SJIS
#define  ft_encoding_gb2312          FT_ENCODING_MS_GB2312
#define  ft_encoding_big5            FT_ENCODING_MS_BIG5
#define  ft_encoding_wansung         FT_ENCODING_MS_WANSUNG
#define  ft_encoding_johab           FT_ENCODING_MS_JOHAB

#define  ft_encoding_adobe_standard  FT_ENCODING_ADOBE_STANDARD
#define  ft_encoding_adobe_expert    FT_ENCODING_ADOBE_EXPERT
#define  ft_encoding_adobe_custom    FT_ENCODING_ADOBE_CUSTOM
#define  ft_encoding_apple_roman     FT_ENCODING_APPLE_ROMAN


typedef struct FT_CharMapRec_ {
	FT_Face face;
	FT_Encoding encoding;
	FT_UShort platform_id;
	FT_UShort encoding_id;
} FT_CharMapRec;

typedef struct FT_Face_InternalRec_ *FT_Face_Internal;

typedef struct FT_FaceRec_ {
	FT_Long num_faces;
	FT_Long face_index;

	FT_Long face_flags;
	FT_Long style_flags;

	FT_Long num_glyphs;

	FT_String *family_name;
	FT_String *style_name;

	FT_Int num_fixed_sizes;
	FT_Bitmap_Size *available_sizes;

	FT_Int num_charmaps;
	FT_CharMap *charmaps;

	FT_Generic generic;

	/*# the following are only relevant to scalable outlines */
	FT_BBox bbox;

	FT_UShort units_per_EM;
	FT_Short ascender;
	FT_Short descender;
	FT_Short height;

	FT_Short max_advance_width;
	FT_Short max_advance_height;

	FT_Short underline_position;
	FT_Short underline_thickness;

	FT_GlyphSlot glyph;
	FT_Size size;
	FT_CharMap charmap;

	/*@private begin */

	FT_Driver driver;
	FT_Memory memory;
	FT_Stream stream;

	FT_ListRec sizes_list;

	FT_Generic autohint;
	void *extensions;

	FT_Face_Internal internal;

	/*@private end */

} FT_FaceRec;


#define FT_FACE_FLAG_SCALABLE          ( 1L <<  0 )
#define FT_FACE_FLAG_FIXED_SIZES       ( 1L <<  1 )
#define FT_FACE_FLAG_FIXED_WIDTH       ( 1L <<  2 )
#define FT_FACE_FLAG_SFNT              ( 1L <<  3 )
#define FT_FACE_FLAG_HORIZONTAL        ( 1L <<  4 )
#define FT_FACE_FLAG_VERTICAL          ( 1L <<  5 )
#define FT_FACE_FLAG_KERNING           ( 1L <<  6 )
#define FT_FACE_FLAG_FAST_GLYPHS       ( 1L <<  7 )
#define FT_FACE_FLAG_MULTIPLE_MASTERS  ( 1L <<  8 )
#define FT_FACE_FLAG_GLYPH_NAMES       ( 1L <<  9 )
#define FT_FACE_FLAG_EXTERNAL_STREAM   ( 1L << 10 )

#define FT_HAS_HORIZONTAL(face) \
	(face->face_flags & FT_FACE_FLAG_HORIZONTAL)

#define FT_HAS_VERTICAL(face) \
	(face->face_flags & FT_FACE_FLAG_VERTICAL)

#define FT_HAS_KERNING(face) \
	(face->face_flags & FT_FACE_FLAG_KERNING)

#define FT_IS_SCALABLE(face) \
	(face->face_flags & FT_FACE_FLAG_SCALABLE)

#define FT_IS_SFNT(face) \
	(face->face_flags & FT_FACE_FLAG_SFNT)

#define FT_IS_FIXED_WIDTH(face) \
	(face->face_flags & FT_FACE_FLAG_FIXED_WIDTH)

#define FT_HAS_FIXED_SIZES(face) \
	(face->face_flags & FT_FACE_FLAG_FIXED_SIZES)

#define FT_HAS_FAST_GLYPHS(face) \
	(face->face_flags & FT_FACE_FLAG_FAST_GLYPHS)

#define FT_HAS_GLYPH_NAMES(face) \
	(face->face_flags & FT_FACE_FLAG_GLYPH_NAMES)

#define FT_HAS_MULTIPLE_MASTERS(face) \
	(face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS)

#define FT_STYLE_FLAG_ITALIC (1 << 0)
#define FT_STYLE_FLAG_BOLD   (1 << 1)


typedef struct FT_Size_InternalRec_ *FT_Size_Internal;

typedef struct FT_Size_Metrics_ {
	FT_UShort x_ppem; /* horizontal pixels per EM               */
	FT_UShort y_ppem; /* vertical pixels per EM                 */

	FT_Fixed x_scale; /* two scales used to convert font units  */
	FT_Fixed y_scale; /* to 26.6 frac. pixel coordinates..      */

	FT_Pos ascender;    /* ascender in 26.6 frac. pixels          */
	FT_Pos descender;   /* descender in 26.6 frac. pixels         */
	FT_Pos height;      /* text height in 26.6 frac. pixels       */
	FT_Pos max_advance; /* max horizontal advance, in 26.6 pixels */
} FT_Size_Metrics;

typedef struct FT_SizeRec_ {
	FT_Face face;            /* parent face object              */
	FT_Generic generic;      /* generic pointer for client uses */
	FT_Size_Metrics metrics; /* size metrics                    */
	FT_Size_Internal internal;
} FT_SizeRec;

typedef struct FT_SubGlyphRec_ *FT_SubGlyph;
typedef struct FT_Slot_InternalRec_ *FT_Slot_Internal;

typedef struct FT_GlyphSlotRec_ {
	FT_Library library;
	FT_Face face;
	FT_GlyphSlot next;
	FT_UInt flags;
	FT_Generic generic;

	FT_Glyph_Metrics metrics;
	FT_Fixed linearHoriAdvance;
	FT_Fixed linearVertAdvance;
	FT_Vector advance;

	FT_Glyph_Format format;

	FT_Bitmap bitmap;
	FT_Int bitmap_left;
	FT_Int bitmap_top;

	FT_Outline outline;

	FT_UInt num_subglyphs;
	FT_SubGlyph subglyphs;

	void *control_data;
	long control_len;

	void *other;

	FT_Slot_Internal internal;

} FT_GlyphSlotRec;


/*************************************************************************/
/*                         F U N C T I O N S                             */
/*************************************************************************/

FT_EXPORT(FT_Error)
FT_Init_FreeType(FT_Library *alibrary);

FT_EXPORT(void)
FT_Library_Version(FT_Library library, FT_Int *amajor, FT_Int *aminor, FT_Int *apatch);

FT_EXPORT(FT_Error)
FT_Done_FreeType(FT_Library library);


typedef enum {
	FT_OPEN_MEMORY = 1,
	FT_OPEN_STREAM = 2,
	FT_OPEN_PATHNAME = 4,
	FT_OPEN_DRIVER = 8,
	FT_OPEN_PARAMS = 16
} FT_Open_Flags;

#define  FT_OPEN_MEMORY    FT_OPEN_MEMORY
#define  FT_OPEN_STREAM    FT_OPEN_STREAM
#define  FT_OPEN_PATHNAME  FT_OPEN_PATHNAME
#define  FT_OPEN_DRIVER    FT_OPEN_DRIVER
#define  FT_OPEN_PARAMS    FT_OPEN_PARAMS

typedef struct FT_Parameter_ {
	FT_ULong tag;
	FT_Pointer data;
} FT_Parameter;

typedef struct FT_Open_Args_ {
	FT_Open_Flags flags;
	const FT_Byte *memory_base;
	FT_Long memory_size;
	FT_String *pathname;
	FT_Stream stream;
	FT_Module driver;
	FT_Int num_params;
	FT_Parameter *params;
} FT_Open_Args;

FT_EXPORT(FT_Error)
FT_New_Face(FT_Library library, const char *filepathname, FT_Long face_index, FT_Face *aface);

FT_EXPORT(FT_Error)
FT_New_Memory_Face(FT_Library library, const FT_Byte *file_base, FT_Long file_size, FT_Long face_index, FT_Face *aface);

FT_EXPORT(FT_Error)
FT_Open_Face(FT_Library library, const FT_Open_Args *args, FT_Long face_index, FT_Face *aface);

FT_EXPORT(FT_Error)
FT_Attach_File(FT_Face face, const char *filepathname);

FT_EXPORT(FT_Error)
FT_Attach_Stream(FT_Face face, FT_Open_Args *parameters);

FT_EXPORT(FT_Error)
FT_Done_Face(FT_Face face);

FT_EXPORT(FT_Error)
FT_Set_Char_Size(FT_Face face, FT_F26Dot6 char_width, FT_F26Dot6 char_height, FT_UInt horz_resolution, FT_UInt vert_resolution);

FT_EXPORT(FT_Error)
FT_Set_Pixel_Sizes(FT_Face face, FT_UInt pixel_width, FT_UInt pixel_height);

FT_EXPORT(FT_Error)
FT_Load_Glyph(FT_Face face, FT_UInt glyph_index, FT_Int32 load_flags);

FT_EXPORT(FT_Error)
FT_Load_Char(FT_Face face, FT_ULong char_code, FT_Int32 load_flags);


#define FT_LOAD_NO_SCALE                     0x1
#define FT_LOAD_NO_HINTING                   0x2
#define FT_LOAD_RENDER                       0x4
#define FT_LOAD_NO_BITMAP                    0x8
#define FT_LOAD_VERTICAL_LAYOUT              0x10
#define FT_LOAD_FORCE_AUTOHINT               0x20
#define FT_LOAD_CROP_BITMAP                  0x40
#define FT_LOAD_PEDANTIC                     0x80
#define FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH  0x200
#define FT_LOAD_NO_RECURSE                   0x400
#define FT_LOAD_IGNORE_TRANSFORM             0x800
#define FT_LOAD_MONOCHROME                   0x1000
#define FT_LOAD_LINEAR_DESIGN                0x2000

/* temporary hack! */
#define FT_LOAD_SBITS_ONLY                   0x4000
#define FT_LOAD_NO_AUTOHINT                  0x8000U

#define FT_LOAD_TARGET_(x)			((FT_Int32)((x) & 15) << 16)
#define FT_LOAD_TARGET_MODE(x)		((FT_Render_Mode)(((x) >> 16) & 15))

#define FT_LOAD_TARGET_NORMAL		FT_LOAD_TARGET_(FT_RENDER_MODE_NORMAL)
#define FT_LOAD_TARGET_LIGHT		FT_LOAD_TARGET_(FT_RENDER_MODE_LIGHT)
#define FT_LOAD_TARGET_MONO			FT_LOAD_TARGET_(FT_RENDER_MODE_MONO)
#define FT_LOAD_TARGET_LCD			FT_LOAD_TARGET_(FT_RENDER_MODE_LCD)
#define FT_LOAD_TARGET_LCD_V		FT_LOAD_TARGET_(FT_RENDER_MODE_LCD_V)

#define FT_LOAD_DEFAULT			0x0

FT_EXPORT(void)
FT_Set_Transform(FT_Face face, FT_Matrix *matrix, FT_Vector *delta);

typedef enum FT_Render_Mode_ {
	FT_RENDER_MODE_NORMAL = 0,
	FT_RENDER_MODE_LIGHT,
	FT_RENDER_MODE_MONO,
	FT_RENDER_MODE_LCD,
	FT_RENDER_MODE_LCD_V,
	FT_RENDER_MODE_MAX
} FT_Render_Mode;

/*    These constats are deprecated. */
#define ft_render_mode_normal  FT_RENDER_MODE_NORMAL
#define ft_render_mode_mono    FT_RENDER_MODE_MONO

FT_EXPORT(FT_Error)
FT_Render_Glyph(FT_GlyphSlot slot, FT_Render_Mode render_mode);

typedef enum FT_Kerning_Mode_ {
	FT_KERNING_DEFAULT = 0,
	FT_KERNING_UNFITTED,
	FT_KERNING_UNSCALED
} FT_Kerning_Mode;

/*    These constats are deprecated. */
#define ft_kerning_default	FT_KERNING_DEFAULT
#define ft_kerning_unfitted FT_KERNING_UNFITTED
#define ft_kerning_unscaled FT_KERNING_UNSCALED

FT_EXPORT(FT_Error)
FT_Get_Kerning(FT_Face face, FT_UInt left_glyph, FT_UInt right_glyph, FT_UInt kern_mode, FT_Vector *akerning);

FT_EXPORT(FT_Error)
FT_Get_Glyph_Name(FT_Face face, FT_UInt glyph_index, FT_Pointer buffer, FT_UInt buffer_max);

FT_EXPORT(const char *)
FT_Get_Postscript_Name(FT_Face face);

FT_EXPORT(FT_Error)
FT_Select_Charmap(FT_Face face, FT_Encoding encoding);

FT_EXPORT(FT_Error)
FT_Set_Charmap(FT_Face face, FT_CharMap charmap);

FT_EXPORT(FT_UInt)
FT_Get_Char_Index(FT_Face face, FT_ULong charcode);

FT_EXPORT(FT_ULong)
FT_Get_First_Char(FT_Face face, FT_UInt *agindex);

FT_EXPORT(FT_ULong)
FT_Get_Next_Char(FT_Face face, FT_ULong char_code, FT_UInt *agindex);

FT_EXPORT(FT_UInt)
FT_Get_Name_Index(FT_Face face, FT_String *glyph_name);


/* COMPUTATIONS */

FT_EXPORT(FT_Long)
FT_MulDiv(FT_Long a, FT_Long b, FT_Long c);

FT_EXPORT(FT_Long)
FT_MulFix(FT_Long a, FT_Long b);

FT_EXPORT(FT_Long)
FT_DivFix(FT_Long a, FT_Long b);

FT_EXPORT(FT_Fixed)
FT_RoundFix(FT_Fixed a);

FT_EXPORT(FT_Fixed)
FT_CeilFix(FT_Fixed a);

FT_EXPORT(FT_Fixed)
FT_FloorFix(FT_Fixed a);

FT_EXPORT(void)
FT_Vector_Transform(FT_Vector *vec, FT_Matrix *matrix);


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FREETYPE_H */
