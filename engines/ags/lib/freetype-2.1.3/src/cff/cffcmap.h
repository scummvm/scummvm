/***************************************************************************/
/*                                                                         */
/*  cffcmap.h                                                              */
/*                                                                         */
/*    CFF character mapping table (cmap) support (specification).          */
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


#ifndef AGS_LIB_FREETYPE_CFFCMAP_H
#define AGS_LIB_FREETYPE_CFFCMAP_H

#include "cffobjs.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****          TYPE1 STANDARD (AND EXPERT) ENCODING CMAPS           *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

/* standard (and expert) encoding cmaps */
typedef struct CFF_CMapStdRec_*  CFF_CMapStd;

typedef struct  CFF_CMapStdRec_ {
	FT2_1_3_CMapRec  cmap;
	FT_UInt     count;
	FT_UShort*  gids;   /* up to 256 elements */

} CFF_CMapStdRec;


FT2_1_3_CALLBACK_TABLE const FT2_1_3_CMap_ClassRec
cff_cmap_encoding_class_rec;


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****               CFF SYNTHETIC UNICODE ENCODING CMAP             *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

/* unicode (synthetic) cmaps */
typedef struct CFF_CMapUnicodeRec_*  CFF_CMapUnicode;

typedef struct  CFF_CMapUniPairRec_ {
	FT_UInt32  unicode;
	FT_UInt    gindex;

} CFF_CMapUniPairRec, *CFF_CMapUniPair;


typedef struct  CFF_CMapUnicodeRec_ {
	FT2_1_3_CMapRec       cmap;
	FT_UInt          num_pairs;
	CFF_CMapUniPair  pairs;

} CFF_CMapUnicodeRec;


FT2_1_3_CALLBACK_TABLE const FT2_1_3_CMap_ClassRec
cff_cmap_unicode_class_rec;


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_CFFCMAP_H */


/* END */
