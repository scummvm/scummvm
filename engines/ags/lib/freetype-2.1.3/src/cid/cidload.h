/***************************************************************************/
/*                                                                         */
/*  cidload.h                                                              */
/*                                                                         */
/*    CID-keyed Type1 font loader (specification).                         */
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


#ifndef AGS_LIB_FREETYPE_CIDLOAD_H
#define AGS_LIB_FREETYPE_CIDLOAD_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftstream.h"
#include "cidparse.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


typedef struct  CID_Loader_ {
	CID_Parser  parser;          /* parser used to read the stream */
	FT_Int      num_chars;       /* number of characters in encoding */

} CID_Loader;


FT2_1_3_LOCAL( FT_Long )
cid_get_offset( FT_Byte**  start,
				FT_Byte    offsize );

FT2_1_3_LOCAL( void )
cid_decrypt( FT_Byte*   buffer,
			 FT_Offset  length,
			 FT_UShort  seed );

FT2_1_3_LOCAL( FT_Error )
cid_face_open( CID_Face  face );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_CIDLOAD_H */

/* END */
