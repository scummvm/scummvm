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


#ifndef __CFF_PARSE_H__
#define __CFF_PARSE_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_INTERNAL_CFF_TYPES_H
#include FT2_1_3_INTERNAL_OBJECTS_H


FT2_1_3_BEGIN_HEADER


#define CFF_MAX_STACK_DEPTH  96

#define CFF_CODE_TOPDICT  0x1000
#define CFF_CODE_PRIVATE  0x2000


typedef struct  CFF_ParserRec_ {
	FT2_1_3_Byte*   start;
	FT2_1_3_Byte*   limit;
	FT2_1_3_Byte*   cursor;

	FT2_1_3_Byte*   stack[CFF_MAX_STACK_DEPTH + 1];
	FT2_1_3_Byte**  top;

	FT2_1_3_UInt    object_code;
	void*      object;

} CFF_ParserRec, *CFF_Parser;


FT2_1_3_LOCAL( void )
cff_parser_init( CFF_Parser  parser,
                 FT2_1_3_UInt     code,
                 void*       object );

FT2_1_3_LOCAL( FT2_1_3_Error )
cff_parser_run( CFF_Parser  parser,
                FT2_1_3_Byte*    start,
                FT2_1_3_Byte*    limit );


FT2_1_3_END_HEADER


#endif /* __CFF_PARSE_H__ */


/* END */
