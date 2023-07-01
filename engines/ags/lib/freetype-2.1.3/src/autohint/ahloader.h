/***************************************************************************/
/*                                                                         */
/*  ahloader.h                                                             */
/*                                                                         */
/*    Glyph loader for the auto-hinting module (declaration only).         */
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


/*************************************************************************/
/*                                                                       */
/* This defines the AH_GlyphLoader type; it is simply a typedef to       */
/* FT2_1_3_GlyphLoader.                                                       */
/*                                                                       */
/*************************************************************************/


#ifndef __AHLOADER_H__
#define __AHLOADER_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"


FT2_1_3_BEGIN_HEADER

#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftgloadr.h"

#define AH_Load    FT2_1_3_GlyphLoad
#define AH_Loader  FT2_1_3_GlyphLoader

#define ah_loader_new              FT2_1_3_GlyphLoader_New
#define ah_loader_done             FT2_1_3_GlyphLoader_Done
#define ah_loader_reset            FT2_1_3_GlyphLoader_Reset
#define ah_loader_rewind           FT2_1_3_GlyphLoader_Rewind
#define ah_loader_create_extra     FT2_1_3_GlyphLoader_CreateExtra
#define ah_loader_check_points     FT2_1_3_GlyphLoader_CheckPoints
#define ah_loader_check_subglyphs  FT2_1_3_GlyphLoader_CheckSubGlyphs
#define ah_loader_prepare          FT2_1_3_GlyphLoader_Prepare
#define ah_loader_add              FT2_1_3_GlyphLoader_Add
#define ah_loader_copy_points      FT2_1_3_GlyphLoader_CopyPoints


FT2_1_3_END_HEADER

#endif /* __AHLOADER_H__ */


/* END */
