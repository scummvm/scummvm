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


#ifndef __AHGLOBAL_H__
#define __AHGLOBAL_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "ahtypes.h"
#include FT2_1_3_INTERNAL_OBJECTS_H


FT2_1_3_BEGIN_HEADER


#define AH_IS_TOP_BLUE( b )  ( (b) == AH_BLUE_CAPITAL_TOP || \
                               (b) == AH_BLUE_SMALL_TOP   )


/* compute global metrics automatically */
FT2_1_3_LOCAL( FT2_1_3_Error )
ah_hinter_compute_globals( AH_Hinter  hinter );


FT2_1_3_END_HEADER

#endif /* __AHGLOBAL_H__ */


/* END */
