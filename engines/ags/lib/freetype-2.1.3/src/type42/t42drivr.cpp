/***************************************************************************/
/*                                                                         */
/*  t42drivr.c                                                             */
/*                                                                         */
/*    High-level Type 42 driver interface (body).                          */
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


/*************************************************************************/
/*                                                                       */
/* This driver implements Type42 fonts as described in the               */
/* Technical Note #5012 from Adobe, with these limitations:              */
/*                                                                       */
/* 1) CID Fonts are not currently supported.                             */
/* 2) Incremental fonts making use of the GlyphDirectory keyword         */
/*    will be loaded, but the rendering will be using the TrueType       */
/*    tables.                                                            */
/* 3) The sfnts array is expected to be ASCII, not binary.               */
/* 4) As for Type1 fonts, CDevProc is not supported.                     */
/* 5) The Metrics dictionary is not supported.                           */
/* 6) AFM metrics are not supported.                                     */
/*                                                                       */
/* In other words, this driver supports Type42 fonts derived from        */
/* TrueType fonts in a non-CID manner, as done by usual conversion       */
/* programs.                                                             */
/*                                                                       */
/*************************************************************************/


#include "t42drivr.h"
#include "t42objs.h"
#include "t42error.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"


#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_t42

namespace AGS3 {
namespace FreeType213 {

static FT2_1_3_Error
t42_get_glyph_name( T42_Face    face,
					FT2_1_3_UInt     glyph_index,
					FT2_1_3_Pointer  buffer,
					FT2_1_3_UInt     buffer_max ) {
	FT2_1_3_String*  gname;


	gname = face->type1.glyph_names[glyph_index];

	if ( buffer_max > 0 ) {
		FT2_1_3_UInt  len = (FT2_1_3_UInt)( ft_strlen( gname ) );


		if ( len >= buffer_max )
			len = buffer_max - 1;

		FT2_1_3_MEM_COPY( buffer, gname, len );
		((FT2_1_3_Byte*)buffer)[len] = 0;
	}

	return T42_Err_Ok;
}


static const char*
t42_get_ps_name( T42_Face  face ) {
	return (const char*)face->type1.font_name;
}


static FT2_1_3_UInt
t42_get_name_index( T42_Face    face,
					FT2_1_3_String*  glyph_name ) {
	FT2_1_3_Int      i;
	FT2_1_3_String*  gname;


	for ( i = 0; i < face->type1.num_glyphs; i++ ) {
		gname = face->type1.glyph_names[i];

		if ( !ft_strcmp( glyph_name, gname ) )
			return ft_atoi( (const char *)face->type1.charstrings[i] );
	}

	return 0;
}


static FT2_1_3_Module_Interface
T42_Get_Interface( FT2_1_3_Driver         driver,
				   const FT2_1_3_String*  t42_interface ) {
	FT2_1_3_UNUSED( driver );

	/* Any additional interface are defined here */
	if (ft_strcmp( (const char*)t42_interface, "glyph_name" ) == 0 )
		return (FT2_1_3_Module_Interface)t42_get_glyph_name;

	if ( ft_strcmp( (const char*)t42_interface, "name_index" ) == 0 )
		return (FT2_1_3_Module_Interface)t42_get_name_index;

	if ( ft_strcmp( (const char*)t42_interface, "postscript_name" ) == 0 )
		return (FT2_1_3_Module_Interface)t42_get_ps_name;

	return 0;
}


const FT2_1_3_Driver_ClassRec  t42_driver_class = {
	{
		ft_module_font_driver      |
		ft_module_driver_scalable  |
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
		ft_module_driver_has_hinter,
#else
		0,
#endif

		sizeof ( T42_DriverRec ),

		"type42",
		0x10000L,
		0x20000L,

		0,    /* format interface */

		(FT2_1_3_Module_Constructor)T42_Driver_Init,
		(FT2_1_3_Module_Destructor) T42_Driver_Done,
		(FT2_1_3_Module_Requester)  T42_Get_Interface,
	},

	sizeof ( T42_FaceRec ),
	sizeof ( T42_SizeRec ),
	sizeof ( T42_GlyphSlotRec ),

	(FT2_1_3_Face_InitFunc)        T42_Face_Init,
	(FT2_1_3_Face_DoneFunc)        T42_Face_Done,
	(FT2_1_3_Size_InitFunc)        T42_Size_Init,
	(FT2_1_3_Size_DoneFunc)        T42_Size_Done,
	(FT2_1_3_Slot_InitFunc)        T42_GlyphSlot_Init,
	(FT2_1_3_Slot_DoneFunc)        T42_GlyphSlot_Done,

	(FT2_1_3_Size_ResetPointsFunc) T42_Size_SetChars,
	(FT2_1_3_Size_ResetPixelsFunc) T42_Size_SetPixels,
	(FT2_1_3_Slot_LoadFunc)        T42_GlyphSlot_Load,

	(FT2_1_3_Face_GetKerningFunc)  0,
	(FT2_1_3_Face_AttachFunc)      0,

	(FT2_1_3_Face_GetAdvancesFunc) 0
};

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
