/***************************************************************************/
/*                                                                         */
/*  t1afm.h                                                                */
/*                                                                         */
/*    AFM support for Type 1 fonts (specification).                        */
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


#ifndef AGS_LIB_FREETYPE_T1AFM_H
#define AGS_LIB_FREETYPE_T1AFM_H

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "t1objs.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


typedef struct  T1_Kern_Pair_ {
	FT_UInt    glyph1;
	FT_UInt    glyph2;
	FT_Vector  kerning;

} T1_Kern_Pair;


typedef struct  T1_AFM_ {
	FT_Int         num_pairs;
	T1_Kern_Pair*  kern_pairs;

} T1_AFM;


FT2_1_3_LOCAL( FT_Error )
T1_Read_AFM( FT_Face    face,
			 FT2_1_3_Stream  stream );

FT2_1_3_LOCAL( void )
T1_Done_AFM( FT2_1_3_Memory  memory,
			 T1_AFM*    afm );

FT2_1_3_LOCAL( void )
T1_Get_Kerning( T1_AFM*     afm,
				FT_UInt     glyph1,
				FT_UInt     glyph2,
				FT_Vector*  kerning );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T1AFM_H */


/* END */
