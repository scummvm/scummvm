/***************************************************************************/
/*                                                                         */
/*  cffparse.h                                                             */
/*                                                                         */
/*    CFF token stream parser (specification)                              */
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


#ifndef AGS_LIB_FREETYPE_CFF_PARSE_H
#define AGS_LIB_FREETYPE_CFF_PARSE_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/cff/cfftypes.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


#define CFF_MAX_STACK_DEPTH  96

#define CFF_CODE_TOPDICT  0x1000
#define CFF_CODE_PRIVATE  0x2000


typedef struct  CFF_ParserRec_ {
	FT_Byte*   start;
	FT_Byte*   limit;
	FT_Byte*   cursor;

	FT_Byte*   stack[CFF_MAX_STACK_DEPTH + 1];
	FT_Byte**  top;

	FT_UInt    object_code;
	void*      object;

} CFF_ParserRec, *CFF_Parser;


FT2_1_3_LOCAL( void )
cff_parser_init( CFF_Parser  parser,
				 FT_UInt     code,
				 void*       object );

FT2_1_3_LOCAL( FT_Error )
cff_parser_run( CFF_Parser  parser,
				FT_Byte*    start,
				FT_Byte*    limit );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_CFF_PARSE_H */


/* END */
