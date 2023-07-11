/***************************************************************************/
/*                                                                         */
/*  ahglobal.h                                                             */
/*                                                                         */
/*    Routines used to compute global metrics automatically                */
/*    (specification).                                                     */
/*                                                                         */
/*  Copyright 2000-2001, 2002 Catharon Productions Inc.                    */
/*  Author: David Turner                                                   */
/*                                                                         */
/*  This file is part of the Catharon Typography Project and shall only    */
/*  be used, modified, and distributed under the terms of the Catharon     */
/*  Open Source License that should come with this file under the name     */
/*  `CatharonLicense.txt'.  By continuing to use, modify, or distribute    */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*  Note that this license is compatible with the FreeType license.        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_AHGLOBAL_H
#define AGS_LIB_FREETYPE_AHGLOBAL_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "ahtypes.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


#define AH_IS_TOP_BLUE( b )  ( (b) == AH_BLUE_CAPITAL_TOP || \
							   (b) == AH_BLUE_SMALL_TOP   )


/* compute global metrics automatically */
FT2_1_3_LOCAL( FT_Error )
ah_hinter_compute_globals( AH_Hinter  hinter );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_AHGLOBAL_H */


/* END */
