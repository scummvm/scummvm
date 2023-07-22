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


#ifndef AGS_LIB_FREETYPE_CFFDRIVER_H
#define AGS_LIB_FREETYPE_CFFDRIVER_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftdriver.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


FT_CALLBACK_TABLE
const FT_Driver_ClassRec  cff_driver_class;


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_CFFDRIVER_H */


/* END */
