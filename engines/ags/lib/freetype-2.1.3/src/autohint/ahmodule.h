/***************************************************************************/
/*                                                                         */
/*  ahmodule.h                                                             */
/*                                                                         */
/*    Auto-hinting module (declaration).                                   */
/*                                                                         */
/*  Copyright 2000-2001 Catharon Productions Inc.                          */
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


#ifndef __AHMODULE_H__
#define __AHMODULE_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftmodule.h"


FT2_1_3_BEGIN_HEADER


FT2_1_3_CALLBACK_TABLE
const FT2_1_3_Module_Class  autohint_module_class;


FT2_1_3_END_HEADER

#endif /* __AHMODULE_H__ */


/* END */
