/***************************************************************************/
/*                                                                         */
/*  pfrcmap.h                                                              */
/*                                                                         */
/*    FreeType PFR cmap handling (specification).                          */
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


#ifndef AGS_LIB_FREETYPE_PFRCMAP_H
#define AGS_LIB_FREETYPE_PFRCMAP_H

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"
#include "pfrtypes.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER

typedef struct  PFR_CMapRec_ {
	FT2_1_3_CMapRec  cmap;
	FT_UInt     num_chars;
	PFR_Char    chars;

} PFR_CMapRec, *PFR_CMap;


FT2_1_3_CALLBACK_TABLE const FT2_1_3_CMap_ClassRec  pfr_cmap_class_rec;

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PFRCMAP_H */


/* END */
