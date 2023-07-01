/***************************************************************************/
/*                                                                         */
/*  t1load.h                                                               */
/*                                                                         */
/*    Type 1 font loader (specification).                                  */
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


#ifndef __T1LOAD_H__
#define __T1LOAD_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psaux.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftmm.h"

#include "t1parse.h"


FT2_1_3_BEGIN_HEADER


typedef struct  T1_Loader_ {
	T1_ParserRec  parser;          /* parser used to read the stream */

	FT2_1_3_Int        num_chars;       /* number of characters in encoding */
	PS_TableRec   encoding_table;  /* PS_Table used to store the       */
	/* encoding character names         */

	FT2_1_3_Int        num_glyphs;
	PS_TableRec   glyph_names;
	PS_TableRec   charstrings;
	PS_TableRec   swap_table;      /* For moving .notdef glyph to index 0. */

	FT2_1_3_Int        num_subrs;
	PS_TableRec   subrs;
	FT2_1_3_Bool       fontdata;

} T1_LoaderRec, *T1_Loader;


FT2_1_3_LOCAL( FT2_1_3_Error )
T1_Open_Face( T1_Face  face );

#ifndef T1_CONFIG_OPTION_NO_MM_SUPPORT

FT2_1_3_LOCAL( FT2_1_3_Error )
T1_Get_Multi_Master( T1_Face           face,
                     FT2_1_3_Multi_Master*  master );

FT2_1_3_LOCAL( FT2_1_3_Error )
T1_Set_MM_Blend( T1_Face    face,
                 FT2_1_3_UInt    num_coords,
                 FT2_1_3_Fixed*  coords );

FT2_1_3_LOCAL( FT2_1_3_Error )
T1_Set_MM_Design( T1_Face   face,
                  FT2_1_3_UInt   num_coords,
                  FT2_1_3_Long*  coords );

FT2_1_3_LOCAL( void )
T1_Done_Blend( T1_Face  face );

#endif /* !T1_CONFIG_OPTION_NO_MM_SUPPORT */


FT2_1_3_END_HEADER

#endif /* __T1LOAD_H__ */


/* END */
