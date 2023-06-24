/***************************************************************************/
/*                                                                         */
/*  t42parse.h                                                             */
/*                                                                         */
/*    Type 42 font parser (specification).                                 */
/*                                                                         */
/*  Copyright 2002 by Roberto Alameda.                                     */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __T42PARSE_H__
#define __T42PARSE_H__


#include "t42objs.h"
#include FT2_1_3_INTERNAL_POSTSCRIPT_AUX_H


FT2_1_3_BEGIN_HEADER

typedef struct  T42_ParserRec_ {
	PS_ParserRec  root;
	FT2_1_3_Stream     stream;

	FT2_1_3_Byte*      base_dict;
	FT2_1_3_Int        base_len;

	FT2_1_3_Byte       in_memory;

} T42_ParserRec, *T42_Parser;


typedef struct  T42_Loader_ {
	T42_ParserRec  parser;          /* parser used to read the stream */

	FT2_1_3_Int         num_chars;       /* number of characters in encoding */
	PS_TableRec    encoding_table;  /* PS_Table used to store the       */
	/* encoding character names         */

	FT2_1_3_Int         num_glyphs;
	PS_TableRec    glyph_names;
	PS_TableRec    charstrings;

} T42_LoaderRec, *T42_Loader;


FT2_1_3_LOCAL( FT2_1_3_Error )
t42_parser_init( T42_Parser     parser,
                 FT2_1_3_Stream      stream,
                 FT2_1_3_Memory      memory,
                 PSAux_Service  psaux );

FT2_1_3_LOCAL( void )
t42_parser_done( T42_Parser  parser );


FT2_1_3_LOCAL( FT2_1_3_Error )
t42_parse_dict( T42_Face    face,
                T42_Loader  loader,
                FT2_1_3_Byte*    base,
                FT2_1_3_Long     size );


FT2_1_3_LOCAL( void )
t42_loader_init( T42_Loader  loader,
                 T42_Face    face );

FT2_1_3_LOCAL( void )
t42_loader_done( T42_Loader  loader );


/* */

FT2_1_3_END_HEADER


#endif /* __T42PARSE_H__ */


/* END */
