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
/*  psauxmod.c                                                             */
/*    FreeType auxiliary PostScript module implementation (body).          */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/modules/psaux/psauxmod.h"
#include "engines/ags/lib/freetype-2.1.3/modules/psaux/psobjs.h"
#include "engines/ags/lib/freetype-2.1.3/modules/psaux/t1decode.h"
#include "engines/ags/lib/freetype-2.1.3/modules/psaux/t1cmap.h"

namespace AGS3 {
namespace FreeType213 {


FT_CALLBACK_TABLE_DEF
const PS_Table_FuncsRec ps_table_funcs = {
	ps_table_new,
	ps_table_done,
	ps_table_add,
	ps_table_release};

FT_CALLBACK_TABLE_DEF
const PS_Parser_FuncsRec ps_parser_funcs = {
	ps_parser_init,
	ps_parser_done,
	ps_parser_skip_spaces,
	ps_parser_skip_alpha,
	ps_parser_to_int,
	ps_parser_to_fixed,
	ps_parser_to_coord_array,
	ps_parser_to_fixed_array,
	ps_parser_to_token,
	ps_parser_to_token_array,
	ps_parser_load_field,
	ps_parser_load_field_table};

FT_CALLBACK_TABLE_DEF
const T1_Builder_FuncsRec t1_builder_funcs = {
	t1_builder_init,
	t1_builder_done,
	t1_builder_check_points,
	t1_builder_add_point,
	t1_builder_add_point1,
	t1_builder_add_contour,
	t1_builder_start_point,
	t1_builder_close_contour};

FT_CALLBACK_TABLE_DEF
const T1_Decoder_FuncsRec t1_decoder_funcs = {
	t1_decoder_init,
	t1_decoder_done,
	t1_decoder_parse_charstrings};

FT_CALLBACK_TABLE_DEF
const T1_CMap_ClassesRec t1_cmap_classes = {
	&t1_cmap_standard_class_rec,
	&t1_cmap_expert_class_rec,
	&t1_cmap_custom_class_rec,
	&t1_cmap_unicode_class_rec};

static const PSAux_Interface psaux_interface = {
	&ps_table_funcs,
	&ps_parser_funcs,
	&t1_builder_funcs,
	&t1_decoder_funcs,

	t1_decrypt,

	(const T1_CMap_ClassesRec *)&t1_cmap_classes,
};

FT_CALLBACK_TABLE_DEF
const FT_Module_Class psaux_module_class = {
	0,
	sizeof(FT_ModuleRec),
	"psaux",
	0x10000L,
	0x20000L,

	&psaux_interface, /* module-specific interface */

	(FT_Module_Constructor) 0,
	(FT_Module_Destructor)  0,
	(FT_Module_Requester)   0
};

} // End of namespace FreeType213
} // End of namespace AGS3
