/***************************************************************************/
/*                                                                         */
/*  cffdrivr.h                                                             */
/*                                                                         */
/*    High-level OpenType driver interface (specification).                */
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


#ifndef __CFFDRIVER_H__
#define __CFFDRIVER_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_INTERNAL_DRIVER_H


FT2_1_3_BEGIN_HEADER


FT2_1_3_CALLBACK_TABLE
const FT2_1_3_Driver_ClassRec  cff_driver_class;


FT2_1_3_END_HEADER

#endif /* __CFFDRIVER_H__ */


/* END */
