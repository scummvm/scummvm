/***************************************************************************/
/*                                                                         */
/*  t42types.h                                                             */
/*                                                                         */
/*    Type 42 font data types (specification only).                        */
/*                                                                         */
/*  Copyright 2002 by Roberto Alameda.                                     */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_T42TYPES_H
#define AGS_LIB_FREETYPE_T42TYPES_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/t1tables.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/t1types.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psnames.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/pshints.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


typedef struct  T42_FaceRec_ {
	FT_FaceRec     root;
	T1_FontRec     type1;
	const void*    psnames;
	const void*    psaux;
	const void*    afm_data;
	FT_Byte*       ttf_data;
	FT_ULong       ttf_size;
	FT_Face        ttf_face;
	FT_CharMapRec  charmaprecs[2];
	FT_CharMap     charmaps[2];
	PS_Unicodes    unicode_map;

} T42_FaceRec, *T42_Face;


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T42TYPES_H */


/* END */
