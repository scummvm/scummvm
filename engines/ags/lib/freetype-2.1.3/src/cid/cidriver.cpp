/***************************************************************************/
/*                                                                         */
/*  cidriver.c                                                             */
/*                                                                         */
/*    CID driver interface (body).                                         */
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


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "cidriver.h"
#include "cidgload.h"
#include FT2_1_3_INTERNAL_DEBUG_H
#include FT2_1_3_INTERNAL_STREAM_H
#include FT2_1_3_INTERNAL_POSTSCRIPT_NAMES_H

#include "ciderrs.h"


/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_ciddriver


static const char*
cid_get_postscript_name( CID_Face  face ) {
	const char*  result = face->cid.cid_font_name;


	if ( result && result[0] == '/' )
		result++;

	return result;
}


static FT2_1_3_Module_Interface
cid_get_interface( FT2_1_3_Driver         driver,
                   const FT2_1_3_String*  cid_interface ) {
	FT2_1_3_UNUSED( driver );
	FT2_1_3_UNUSED( cid_interface );

	if ( ft_strcmp( (const char*)cid_interface, "postscript_name" ) == 0 )
		return (FT2_1_3_Module_Interface)cid_get_postscript_name;

	return 0;
}



FT2_1_3_CALLBACK_TABLE_DEF
const FT2_1_3_Driver_ClassRec  t1cid_driver_class = {
	/* first of all, the FT2_1_3_Module_Class fields */
	{
		ft_module_font_driver       |
		ft_module_driver_scalable   |
		ft_module_driver_has_hinter,

		sizeof( FT2_1_3_DriverRec ),
		"t1cid",   /* module name           */
		0x10000L,  /* version 1.0 of driver */
		0x20000L,  /* requires FreeType 2.0 */

		0,

		(FT2_1_3_Module_Constructor)cid_driver_init,
		(FT2_1_3_Module_Destructor) cid_driver_done,
		(FT2_1_3_Module_Requester)  cid_get_interface
	},

	/* then the other font drivers fields */
	sizeof( CID_FaceRec ),
	sizeof( CID_SizeRec ),
	sizeof( CID_GlyphSlotRec ),

	(FT2_1_3_Face_InitFunc)       cid_face_init,
	(FT2_1_3_Face_DoneFunc)       cid_face_done,

	(FT2_1_3_Size_InitFunc)       cid_size_init,
	(FT2_1_3_Size_DoneFunc)       cid_size_done,
	(FT2_1_3_Slot_InitFunc)       cid_slot_init,
	(FT2_1_3_Slot_DoneFunc)       cid_slot_done,

	(FT2_1_3_Size_ResetPointsFunc)cid_size_reset,
	(FT2_1_3_Size_ResetPixelsFunc)cid_size_reset,

	(FT2_1_3_Slot_LoadFunc)       cid_slot_load_glyph,

	(FT2_1_3_Face_GetKerningFunc) 0,
	(FT2_1_3_Face_AttachFunc)     0,

	(FT2_1_3_Face_GetAdvancesFunc)0,
};


/* END */
