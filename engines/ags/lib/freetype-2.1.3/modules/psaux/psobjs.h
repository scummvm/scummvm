/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/***************************************************************************/
/*                                                                         */
/*  psobjs.h                                                               */
/*    Auxiliary functions for PostScript fonts (specification).            */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_PSOBJS_H
#define AGS_LIB_FREETYPE_PSOBJS_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/psaux.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/**** T1_TABLE ****/

FT2_1_3_CALLBACK_TABLE
const PS_Table_FuncsRec ps_table_funcs;

FT2_1_3_CALLBACK_TABLE
const PS_Parser_FuncsRec ps_parser_funcs;

FT2_1_3_CALLBACK_TABLE
const T1_Builder_FuncsRec t1_builder_funcs;

FT2_1_3_LOCAL(FT_Error)
ps_table_new(PS_Table table, FT_Int count, FT_Memory memory);

FT2_1_3_LOCAL(FT_Error)
ps_table_add(PS_Table table, FT_Int idx, void *object, FT_Int length);

FT2_1_3_LOCAL(void)
ps_table_done(PS_Table table);

FT2_1_3_LOCAL(void)
ps_table_release(PS_Table table);


/**** T1 PARSER ****/

FT2_1_3_LOCAL(void)
ps_parser_skip_spaces(PS_Parser parser);

FT2_1_3_LOCAL(void)
ps_parser_skip_alpha(PS_Parser parser);

FT2_1_3_LOCAL(void)
ps_parser_to_token(PS_Parser parser, T1_Token token);

FT2_1_3_LOCAL(void)
ps_parser_to_token_array(PS_Parser parser, T1_Token tokens, FT_UInt max_tokens, FT_Int *pnum_tokens);

FT2_1_3_LOCAL(FT_Error)
ps_parser_load_field(PS_Parser parser, const T1_Field field, void **objects, FT_UInt max_objects, FT_ULong *pflags);

FT2_1_3_LOCAL(FT_Error)
ps_parser_load_field_table(PS_Parser parser, const T1_Field field, void **objects, FT_UInt max_objects, FT_ULong *pflags);

FT2_1_3_LOCAL(FT_Long)
ps_parser_to_int(PS_Parser parser);

FT2_1_3_LOCAL(FT_Fixed)
ps_parser_to_fixed(PS_Parser parser, FT_Int power_ten);

FT2_1_3_LOCAL(FT_Int)
ps_parser_to_coord_array(PS_Parser parser, FT_Int max_coords, FT_Short *coords);

FT2_1_3_LOCAL(FT_Int)
ps_parser_to_fixed_array(PS_Parser parser, FT_Int max_values, FT_Fixed *values, FT_Int power_ten);

FT2_1_3_LOCAL(void)
ps_parser_init(PS_Parser parser, FT_Byte *base, FT_Byte *limit, FT_Memory memory);

FT2_1_3_LOCAL(void)
ps_parser_done(PS_Parser parser);


/**** T1 BUILDER ****/

FT2_1_3_LOCAL(void)
t1_builder_init(T1_Builder builder, FT_Face face, FT_Size size, FT_GlyphSlot glyph, FT_Bool hinting);

FT2_1_3_LOCAL(void)
t1_builder_done(T1_Builder builder);

FT2_1_3_LOCAL(FT_Error)
t1_builder_check_points(T1_Builder builder, FT_Int count);

FT2_1_3_LOCAL(void)
t1_builder_add_point(T1_Builder builder, FT_Pos x, FT_Pos y, FT_Byte flag);

FT2_1_3_LOCAL(FT_Error)
t1_builder_add_point1(T1_Builder builder, FT_Pos x, FT_Pos y);

FT2_1_3_LOCAL(FT_Error)
t1_builder_add_contour(T1_Builder builder);

FT2_1_3_LOCAL(FT_Error)
t1_builder_start_point(T1_Builder builder, FT_Pos x, FT_Pos y);

FT2_1_3_LOCAL(void)
t1_builder_close_contour(T1_Builder builder);


/**** OTHER ****/

FT2_1_3_LOCAL(void)
t1_decrypt(FT_Byte *buffer, FT_Offset length, FT_UShort seed);


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PSOBJS_H */
