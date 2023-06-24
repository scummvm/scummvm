/***************************************************************************/
/*                                                                         */
/*  psauxmod.c                                                             */
/*                                                                         */
/*    FreeType auxiliary PostScript module implementation (body).          */
/*                                                                         */
/*  Copyright 2000-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "psauxmod.h"
#include "psobjs.h"
#include "t1decode.h"
#include "t1cmap.h"


FT2_1_3_CALLBACK_TABLE_DEF
const PS_Table_FuncsRec  ps_table_funcs = {
	ps_table_new,
	ps_table_done,
	ps_table_add,
	ps_table_release
};


FT2_1_3_CALLBACK_TABLE_DEF
const PS_Parser_FuncsRec  ps_parser_funcs = {
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
	ps_parser_load_field_table
};


FT2_1_3_CALLBACK_TABLE_DEF
const T1_Builder_FuncsRec  t1_builder_funcs = {
	t1_builder_init,
	t1_builder_done,
	t1_builder_check_points,
	t1_builder_add_point,
	t1_builder_add_point1,
	t1_builder_add_contour,
	t1_builder_start_point,
	t1_builder_close_contour
};


FT2_1_3_CALLBACK_TABLE_DEF
const T1_Decoder_FuncsRec  t1_decoder_funcs = {
	t1_decoder_init,
	t1_decoder_done,
	t1_decoder_parse_charstrings
};


FT2_1_3_CALLBACK_TABLE_DEF
const T1_CMap_ClassesRec  t1_cmap_classes = {
	&t1_cmap_standard_class_rec,
	&t1_cmap_expert_class_rec,
	&t1_cmap_custom_class_rec,
	&t1_cmap_unicode_class_rec
};


static
const PSAux_Interface  psaux_interface = {
	&ps_table_funcs,
	&ps_parser_funcs,
	&t1_builder_funcs,
	&t1_decoder_funcs,

	t1_decrypt,

	(const T1_CMap_ClassesRec*) &t1_cmap_classes,
};


FT2_1_3_CALLBACK_TABLE_DEF
const FT2_1_3_Module_Class  psaux_module_class = {
	0,
	sizeof( FT2_1_3_ModuleRec ),
	"psaux",
	0x10000L,
	0x20000L,

	&psaux_interface,  /* module-specific interface */

	(FT2_1_3_Module_Constructor)0,
	(FT2_1_3_Module_Destructor) 0,
	(FT2_1_3_Module_Requester)  0
};


/* END */
