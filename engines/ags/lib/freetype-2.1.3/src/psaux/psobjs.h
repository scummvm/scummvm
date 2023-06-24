/***************************************************************************/
/*                                                                         */
/*  psobjs.h                                                               */
/*                                                                         */
/*    Auxiliary functions for PostScript fonts (specification).            */
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


#ifndef __PSOBJS_H__
#define __PSOBJS_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_INTERNAL_POSTSCRIPT_AUX_H


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                             T1_TABLE                          *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/


FT2_1_3_CALLBACK_TABLE
const PS_Table_FuncsRec    ps_table_funcs;

FT2_1_3_CALLBACK_TABLE
const PS_Parser_FuncsRec   ps_parser_funcs;

FT2_1_3_CALLBACK_TABLE
const T1_Builder_FuncsRec  t1_builder_funcs;


FT2_1_3_LOCAL( FT2_1_3_Error )
ps_table_new( PS_Table   table,
              FT2_1_3_Int     count,
              FT2_1_3_Memory  memory );

FT2_1_3_LOCAL( FT2_1_3_Error )
ps_table_add( PS_Table  table,
              FT2_1_3_Int    idx,
              void*     object,
              FT2_1_3_Int    length );

FT2_1_3_LOCAL( void )
ps_table_done( PS_Table  table );


FT2_1_3_LOCAL( void )
ps_table_release( PS_Table  table );


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                            T1 PARSER                          *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/


FT2_1_3_LOCAL( void )
ps_parser_skip_spaces( PS_Parser  parser );

FT2_1_3_LOCAL( void )
ps_parser_skip_alpha( PS_Parser  parser );

FT2_1_3_LOCAL( void )
ps_parser_to_token( PS_Parser  parser,
                    T1_Token   token );

FT2_1_3_LOCAL( void )
ps_parser_to_token_array( PS_Parser  parser,
                          T1_Token   tokens,
                          FT2_1_3_UInt    max_tokens,
                          FT2_1_3_Int*    pnum_tokens );

FT2_1_3_LOCAL( FT2_1_3_Error )
ps_parser_load_field( PS_Parser       parser,
                      const T1_Field  field,
                      void**          objects,
                      FT2_1_3_UInt         max_objects,
                      FT2_1_3_ULong*       pflags );

FT2_1_3_LOCAL( FT2_1_3_Error )
ps_parser_load_field_table( PS_Parser       parser,
                            const T1_Field  field,
                            void**          objects,
                            FT2_1_3_UInt         max_objects,
                            FT2_1_3_ULong*       pflags );

FT2_1_3_LOCAL( FT2_1_3_Long )
ps_parser_to_int( PS_Parser  parser );


FT2_1_3_LOCAL( FT2_1_3_Fixed )
ps_parser_to_fixed( PS_Parser  parser,
                    FT2_1_3_Int     power_ten );


FT2_1_3_LOCAL( FT2_1_3_Int )
ps_parser_to_coord_array( PS_Parser  parser,
                          FT2_1_3_Int     max_coords,
                          FT2_1_3_Short*  coords );

FT2_1_3_LOCAL( FT2_1_3_Int )
ps_parser_to_fixed_array( PS_Parser  parser,
                          FT2_1_3_Int     max_values,
                          FT2_1_3_Fixed*  values,
                          FT2_1_3_Int     power_ten );


FT2_1_3_LOCAL( void )
ps_parser_init( PS_Parser  parser,
                FT2_1_3_Byte*   base,
                FT2_1_3_Byte*   limit,
                FT2_1_3_Memory  memory );

FT2_1_3_LOCAL( void )
ps_parser_done( PS_Parser  parser );


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                            T1 BUILDER                         *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

FT2_1_3_LOCAL( void )
t1_builder_init( T1_Builder    builder,
                 FT2_1_3_Face       face,
                 FT2_1_3_Size       size,
                 FT2_1_3_GlyphSlot  glyph,
                 FT2_1_3_Bool       hinting );

FT2_1_3_LOCAL( void )
t1_builder_done( T1_Builder  builder );

FT2_1_3_LOCAL( FT2_1_3_Error )
t1_builder_check_points( T1_Builder  builder,
                         FT2_1_3_Int      count );

FT2_1_3_LOCAL( void )
t1_builder_add_point( T1_Builder  builder,
                      FT2_1_3_Pos      x,
                      FT2_1_3_Pos      y,
                      FT2_1_3_Byte     flag );

FT2_1_3_LOCAL( FT2_1_3_Error )
t1_builder_add_point1( T1_Builder  builder,
                       FT2_1_3_Pos      x,
                       FT2_1_3_Pos      y );

FT2_1_3_LOCAL( FT2_1_3_Error )
t1_builder_add_contour( T1_Builder  builder );


FT2_1_3_LOCAL( FT2_1_3_Error )
t1_builder_start_point( T1_Builder  builder,
                        FT2_1_3_Pos      x,
                        FT2_1_3_Pos      y );


FT2_1_3_LOCAL( void )
t1_builder_close_contour( T1_Builder  builder );


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                            OTHER                              *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

FT2_1_3_LOCAL( void )
t1_decrypt( FT2_1_3_Byte*   buffer,
            FT2_1_3_Offset  length,
            FT2_1_3_UShort  seed );


FT2_1_3_END_HEADER

#endif /* __PSOBJS_H__ */


/* END */
