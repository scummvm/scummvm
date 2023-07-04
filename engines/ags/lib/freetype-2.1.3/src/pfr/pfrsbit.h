/***************************************************************************/
/*                                                                         */
/*  pfrsbit.h                                                              */
/*                                                                         */
/*    FreeType PFR bitmap loader (specification).                          */
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


#ifndef AGS_LIB_FREETYPE_PFRSBIT_H
#define AGS_LIB_FREETYPE_PFRSBIT_H

#include "pfrobjs.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER

FT2_1_3_LOCAL( FT2_1_3_Error )
pfr_slot_load_bitmap( PFR_Slot  glyph,
					  PFR_Size  size,
					  FT2_1_3_UInt   glyph_index );

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PFRSBIT_H */


/* END */
