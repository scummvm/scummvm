/***************************************************************************/
/*                                                                         */
/*  ttcmap0.h                                                              */
/*                                                                         */
/*    TrueType new character mapping table (cmap) support (specification). */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_TTCMAP0_H
#define AGS_LIB_FREETYPE_TTCMAP0_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/tttypes.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"


namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER

typedef struct  TT_CMapRec_ {
	FT_CMapRec  cmap;
	FT_Byte*    data;           /* pointer to in-memory cmap table */

} TT_CMapRec, *TT_CMap;

typedef const struct TT_CMap_ClassRec_*  TT_CMap_Class;


typedef FT_Error
(*TT_CMap_ValidateFunc)( FT_Byte*      data,
						 FT_Validator  valid );

typedef struct  TT_CMap_ClassRec_ {
	FT_CMap_ClassRec      clazz;
	FT_UInt               format;
	TT_CMap_ValidateFunc  validate;

} TT_CMap_ClassRec;


typedef struct  TT_ValidatorRec_ {
	FT_ValidatorRec  validator;
	FT_UInt          num_glyphs;

} TT_ValidatorRec, *TT_Validator;


#define TT_VALIDATOR( x )          ((TT_Validator)( x ))
#define TT_VALID_GLYPH_COUNT( x )  TT_VALIDATOR( x )->num_glyphs


FT2_1_3_LOCAL( FT_Error )
tt_face_build_cmaps( TT_Face  face );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_TTCMAP0_H */


/* END */
