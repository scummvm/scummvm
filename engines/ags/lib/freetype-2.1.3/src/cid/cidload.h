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


#ifndef __CIDLOAD_H__
#define __CIDLOAD_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_INTERNAL_STREAM_H
#include "cidparse.h"


FT2_1_3_BEGIN_HEADER


typedef struct  CID_Loader_ {
	CID_Parser  parser;          /* parser used to read the stream */
	FT2_1_3_Int      num_chars;       /* number of characters in encoding */

} CID_Loader;


FT2_1_3_LOCAL( FT2_1_3_Long )
cid_get_offset( FT2_1_3_Byte**  start,
                FT2_1_3_Byte    offsize );

FT2_1_3_LOCAL( void )
cid_decrypt( FT2_1_3_Byte*   buffer,
             FT2_1_3_Offset  length,
             FT2_1_3_UShort  seed );

FT2_1_3_LOCAL( FT2_1_3_Error )
cid_face_open( CID_Face  face );


FT2_1_3_END_HEADER

#endif /* __CIDLOAD_H__ */


/* END */
