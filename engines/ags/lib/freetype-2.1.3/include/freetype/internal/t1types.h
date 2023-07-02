/***************************************************************************/
/*                                                                         */
/*  t1types.h                                                              */
/*                                                                         */
/*    Basic Type1/Type2 type definitions and interface (specification      */
/*    only).                                                               */
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


#ifndef AGS_LIB_FREETYPE_T1TYPES_H
#define AGS_LIB_FREETYPE_T1TYPES_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/t1tables.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psnames.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/pshints.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/***                                                                   ***/
/***                                                                   ***/
/***              REQUIRED TYPE1/TYPE2 TABLES DEFINITIONS              ***/
/***                                                                   ***/
/***                                                                   ***/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    T1_EncodingRec                                                     */
/*                                                                       */
/* <Description>                                                         */
/*    A structure modeling a custom encoding.                            */
/*                                                                       */
/* <Fields>                                                              */
/*    num_chars  :: The number of character codes in the encoding.       */
/*                  Usually 256.                                         */
/*                                                                       */
/*    code_first :: The lowest valid character code in the encoding.     */
/*                                                                       */
/*    code_last  :: The highest valid character code in the encoding.    */
/*                                                                       */
/*    char_index :: An array of corresponding glyph indices.             */
/*                                                                       */
/*    char_name  :: An array of corresponding glyph names.               */
/*                                                                       */
typedef struct  T1_EncodingRecRec_ {
	FT2_1_3_Int       num_chars;
	FT2_1_3_Int       code_first;
	FT2_1_3_Int       code_last;

	FT2_1_3_UShort*   char_index;
	FT2_1_3_String**  char_name;

} T1_EncodingRec, *T1_Encoding;


typedef enum  T1_EncodingType_ {
	T1_ENCODING_TYPE_NONE = 0,
	T1_ENCODING_TYPE_ARRAY,
	T1_ENCODING_TYPE_STANDARD,
	T1_ENCODING_TYPE_ISOLATIN1,
	T1_ENCODING_TYPE_EXPERT

} T1_EncodingType;


typedef struct  T1_FontRec_ {
	PS_FontInfoRec   font_info;         /* font info dictionary */
	PS_PrivateRec    private_dict;      /* private dictionary   */
	FT2_1_3_String*       font_name;         /* top-level dictionary */

	T1_EncodingType  encoding_type;
	T1_EncodingRec   encoding;

	FT2_1_3_Byte*         subrs_block;
	FT2_1_3_Byte*         charstrings_block;
	FT2_1_3_Byte*         glyph_names_block;

	FT2_1_3_Int           num_subrs;
	FT2_1_3_Byte**        subrs;
	FT2_1_3_Int*          subrs_len;

	FT2_1_3_Int           num_glyphs;
	FT2_1_3_String**      glyph_names;       /* array of glyph names       */
	FT2_1_3_Byte**        charstrings;       /* array of glyph charstrings */
	FT2_1_3_Int*          charstrings_len;

	FT2_1_3_Byte          paint_type;
	FT2_1_3_Byte          font_type;
	FT2_1_3_Matrix        font_matrix;
	FT2_1_3_Vector        font_offset;
	FT2_1_3_BBox          font_bbox;
	FT2_1_3_Long          font_id;

	FT2_1_3_Int           stroke_width;

} T1_FontRec, *T1_Font;


typedef struct  CID_SubrsRec_ {
	FT2_1_3_UInt    num_subrs;
	FT2_1_3_Byte**  code;

} CID_SubrsRec, *CID_Subrs;


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/***                                                                   ***/
/***                                                                   ***/
/***                ORIGINAL T1_FACE CLASS DEFINITION                  ***/
/***                                                                   ***/
/***                                                                   ***/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* This structure/class is defined here because it is common to the      */
/* following formats: TTF, OpenType-TT, and OpenType-CFF.                */
/*                                                                       */
/* Note, however, that the classes TT_Size, TT_GlyphSlot, and TT_CharMap */
/* are not shared between font drivers, and are thus defined normally in */
/* `ttobjs.h'.                                                           */
/*                                                                       */
/*************************************************************************/

typedef struct T1_FaceRec_*   T1_Face;
typedef struct CID_FaceRec_*  CID_Face;


typedef struct  T1_FaceRec_ {
	FT2_1_3_FaceRec     root;
	T1_FontRec     type1;
	const void*    psnames;
	const void*    psaux;
	const void*    afm_data;
	FT2_1_3_CharMapRec  charmaprecs[2];
	FT2_1_3_CharMap     charmaps[2];
	PS_Unicodes    unicode_map;

	/* support for Multiple Masters fonts */
	PS_Blend       blend;

	/* since FT 2.1 - interface to PostScript hinter */
	const void*    pshinter;

} T1_FaceRec;


typedef struct  CID_FaceRec_ {
	FT2_1_3_FaceRec       root;
	void*            psnames;
	void*            psaux;
	CID_FaceInfoRec  cid;
	void*            afm_data;
	CID_Subrs        subrs;

	/* since FT 2.1 - interface to PostScript hinter */
	void*            pshinter;

} CID_FaceRec;


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T1TYPES_H */


/* END */
