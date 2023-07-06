/***************************************************************************/
/*                                                                         */
/*  pfrobjs.h                                                              */
/*                                                                         */
/*    FreeType PFR object methods (specification).                         */
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


#ifndef AGS_LIB_FREETYPE_PFROBJS_H
#define AGS_LIB_FREETYPE_PFROBJS_H

#include "pfrtypes.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER

typedef struct PFR_FaceRec_*  PFR_Face;

typedef struct PFR_SizeRec_*  PFR_Size;

typedef struct PFR_SlotRec_*  PFR_Slot;


typedef struct  PFR_FaceRec_ {
	FT_FaceRec      root;
	PFR_HeaderRec   header;
	PFR_LogFontRec  log_font;
	PFR_PhyFontRec  phy_font;

} PFR_FaceRec;


typedef struct  PFR_SizeRec_ {
	FT_SizeRec  root;

} PFR_SizeRec;


typedef struct  PFR_SlotRec_ {
	FT_GlyphSlotRec  root;
	PFR_GlyphRec     glyph;

} PFR_SlotRec;


FT2_1_3_LOCAL( FT_Error )
pfr_face_init( FT2_1_3_Stream      stream,
			   PFR_Face       face,
			   FT_Int         face_index,
			   FT_Int         num_params,
			   FT_Parameter*  params );

FT2_1_3_LOCAL( void )
pfr_face_done( PFR_Face  face );


FT2_1_3_LOCAL( FT_Error )
pfr_face_get_kerning( PFR_Face   face,
					  FT_UInt    glyph1,
					  FT_UInt    glyph2,
					  FT_Vector* kerning );


FT2_1_3_LOCAL( FT_Error )
pfr_slot_init( PFR_Slot  slot );

FT2_1_3_LOCAL( void )
pfr_slot_done( PFR_Slot  slot );


FT2_1_3_LOCAL( FT_Error )
pfr_slot_load( PFR_Slot  slot,
			   PFR_Size  size,
			   FT_UInt   gindex,
			   FT_Int32  load_flags );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PFROBJS_H */


/* END */
