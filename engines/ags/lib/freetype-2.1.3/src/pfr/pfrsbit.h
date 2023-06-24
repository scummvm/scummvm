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


#ifndef __PFRSBIT_H__
#define __PFRSBIT_H__

#include "pfrobjs.h"

FT2_1_3_BEGIN_HEADER

FT2_1_3_LOCAL( FT2_1_3_Error )
pfr_slot_load_bitmap( PFR_Slot  glyph,
                      PFR_Size  size,
                      FT2_1_3_UInt   glyph_index );

FT2_1_3_END_HEADER

#endif /* __PFR_SBIT_H__ */


/* END */
