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


#ifndef __PFRCMAP_H__
#define __PFRCMAP_H__

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_INTERNAL_OBJECTS_H
#include "pfrtypes.h"


FT2_1_3_BEGIN_HEADER

typedef struct  PFR_CMapRec_ {
	FT2_1_3_CMapRec  cmap;
	FT2_1_3_UInt     num_chars;
	PFR_Char    chars;

} PFR_CMapRec, *PFR_CMap;


FT2_1_3_CALLBACK_TABLE const FT2_1_3_CMap_ClassRec  pfr_cmap_class_rec;

FT2_1_3_END_HEADER


#endif /* __PFRCMAP_H__ */


/* END */
