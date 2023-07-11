/***************************************************************************/
/*                                                                         */
/*  ahhint.h                                                               */
/*                                                                         */
/*    Glyph hinter (declaration).                                          */
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


#ifndef AGS_LIB_FREETYPE_AHHINT_H
#define AGS_LIB_FREETYPE_AHHINT_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "ahglobal.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


#define AH_HINT_DEFAULT        0
#define AH_HINT_NO_ALIGNMENT   1
#define AH_HINT_NO_HORZ_EDGES  0x200000L  /* temporary hack */
#define AH_HINT_NO_VERT_EDGES  0x400000L  /* temporary hack */


/* create a new empty hinter object */
FT2_1_3_LOCAL( FT_Error )
ah_hinter_new( FT_Library  library,
			   AH_Hinter*  ahinter );

/* Load a hinted glyph in the hinter */
FT2_1_3_LOCAL( FT_Error )
ah_hinter_load_glyph( AH_Hinter     hinter,
					  FT_GlyphSlot  slot,
					  FT_Size       size,
					  FT_UInt       glyph_index,
					  FT_Int32      load_flags );

/* finalize a hinter object */
FT2_1_3_LOCAL( void )
ah_hinter_done( AH_Hinter  hinter );

FT2_1_3_LOCAL( void )
ah_hinter_done_face_globals( AH_Face_Globals  globals );

FT2_1_3_LOCAL( void )
ah_hinter_get_global_hints( AH_Hinter  hinter,
							FT_Face    face,
							void**     global_hints,
							long*      global_len );

FT2_1_3_LOCAL( void )
ah_hinter_done_global_hints( AH_Hinter  hinter,
							 void*      global_hints );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_AHHINT_H */


/* END */
