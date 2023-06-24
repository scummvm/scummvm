/***************************************************************************/
/*                                                                         */
/*  cffdrivr.c                                                             */
/*                                                                         */
/*    OpenType font driver implementation (body).                          */
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
#include FT2_1_3_FREETYPE_H
#include FT2_1_3_INTERNAL_DEBUG_H
#include FT2_1_3_INTERNAL_STREAM_H
#include FT2_1_3_INTERNAL_SFNT_H
#include FT2_1_3_TRUETYPE_IDS_H
#include FT2_1_3_INTERNAL_POSTSCRIPT_NAMES_H

#include "cffdrivr.h"
#include "cffgload.h"
#include "cffload.h"

#include "cfferrs.h"


/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_cffdriver


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/****                                                                 ****/
/****                                                                 ****/
/****                          F A C E S                              ****/
/****                                                                 ****/
/****                                                                 ****/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


#undef  PAIR_TAG
#define PAIR_TAG( left, right )  ( ( (FT2_1_3_ULong)left << 16 ) | \
                                     (FT2_1_3_ULong)right        )


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    Get_Kerning                                                        */
/*                                                                       */
/* <Description>                                                         */
/*    A driver method used to return the kerning vector between two      */
/*    glyphs of the same face.                                           */
/*                                                                       */
/* <Input>                                                               */
/*    face        :: A handle to the source face object.                 */
/*                                                                       */
/*    left_glyph  :: The index of the left glyph in the kern pair.       */
/*                                                                       */
/*    right_glyph :: The index of the right glyph in the kern pair.      */
/*                                                                       */
/* <Output>                                                              */
/*    kerning     :: The kerning vector.  This is in font units for      */
/*                   scalable formats, and in pixels for fixed-sizes     */
/*                   formats.                                            */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    Only horizontal layouts (left-to-right & right-to-left) are        */
/*    supported by this function.  Other layouts, or more sophisticated  */
/*    kernings, are out of scope of this method (the basic driver        */
/*    interface is meant to be simple).                                  */
/*                                                                       */
/*    They can be implemented by format-specific interfaces.             */
/*                                                                       */
static FT2_1_3_Error
Get_Kerning( TT_Face     face,
             FT2_1_3_UInt     left_glyph,
             FT2_1_3_UInt     right_glyph,
             FT2_1_3_Vector*  kerning ) {
	TT_Kern0_Pair  pair;


	if ( !face )
		return CFF_Err_Invalid_Face_Handle;

	kerning->x = 0;
	kerning->y = 0;

	if ( face->kern_pairs ) {
		/* there are some kerning pairs in this font file! */
		FT2_1_3_ULong  search_tag = PAIR_TAG( left_glyph, right_glyph );
		FT2_1_3_Long   left, right;


		left  = 0;
		right = face->num_kern_pairs - 1;

		while ( left <= right ) {
			FT2_1_3_Long   middle = left + ( ( right - left ) >> 1 );
			FT2_1_3_ULong  cur_pair;


			pair     = face->kern_pairs + middle;
			cur_pair = PAIR_TAG( pair->left, pair->right );

			if ( cur_pair == search_tag )
				goto Found;

			if ( cur_pair < search_tag )
				left = middle + 1;
			else
				right = middle - 1;
		}
	}

Exit:
	return CFF_Err_Ok;

Found:
	kerning->x = pair->value;
	goto Exit;
}


#undef PAIR_TAG


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    Load_Glyph                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    A driver method used to load a glyph within a given glyph slot.    */
/*                                                                       */
/* <Input>                                                               */
/*    slot        :: A handle to the target slot object where the glyph  */
/*                   will be loaded.                                     */
/*                                                                       */
/*    size        :: A handle to the source face size at which the glyph */
/*                   must be scaled, loaded, etc.                        */
/*                                                                       */
/*    glyph_index :: The index of the glyph in the font file.            */
/*                                                                       */
/*    load_flags  :: A flag indicating what to load for this glyph.  The */
/*                   FTLOAD_??? constants can be used to control the     */
/*                   glyph loading process (e.g., whether the outline    */
/*                   should be scaled, whether to load bitmaps or not,   */
/*                   whether to hint the outline, etc).                  */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
static FT2_1_3_Error
Load_Glyph( CFF_GlyphSlot  slot,
            CFF_Size       size,
            FT2_1_3_UShort      glyph_index,
            FT2_1_3_Int32       load_flags ) {
	FT2_1_3_Error  error;


	if ( !slot )
		return CFF_Err_Invalid_Slot_Handle;

	/* check whether we want a scaled outline or bitmap */
	if ( !size )
		load_flags |= FT2_1_3_LOAD_NO_SCALE | FT2_1_3_LOAD_NO_HINTING;

	if ( load_flags & FT2_1_3_LOAD_NO_SCALE )
		size = NULL;

	/* reset the size object if necessary */
	if ( size ) {
		/* these two object must have the same parent */
		if ( size->face != slot->root.face )
			return CFF_Err_Invalid_Face_Handle;
	}

	/* now load the glyph outline if necessary */
	error = cff_slot_load( slot, size, glyph_index, load_flags );

	/* force drop-out mode to 2 - irrelevant now */
	/* slot->outline.dropout_mode = 2; */

	return error;
}


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/****                                                                 ****/
/****                                                                 ****/
/****             C H A R A C T E R   M A P P I N G S                 ****/
/****                                                                 ****/
/****                                                                 ****/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

static FT2_1_3_Error
cff_get_glyph_name( CFF_Face    face,
                    FT2_1_3_UInt     glyph_index,
                    FT2_1_3_Pointer  buffer,
                    FT2_1_3_UInt     buffer_max ) {
	CFF_Font         font   = (CFF_Font)face->extra.data;
	FT2_1_3_Memory        memory = FT2_1_3_FACE_MEMORY( face );
	FT2_1_3_String*       gname;
	FT2_1_3_UShort        sid;
	PSNames_Service  psnames;
	FT2_1_3_Error         error;


	psnames = (PSNames_Service)FT2_1_3_Get_Module_Interface(
	              face->root.driver->root.library, "psnames" );

	if ( !psnames ) {
		FT2_1_3_ERROR(( "cff_get_glyph_name:" ));
		FT2_1_3_ERROR(( " cannot open CFF & CEF fonts\n" ));
		FT2_1_3_ERROR(( "                   " ));
		FT2_1_3_ERROR(( " without the `PSNames' module\n" ));
		error = CFF_Err_Unknown_File_Format;
		goto Exit;
	}

	/* first, locate the sid in the charset table */
	sid = font->charset.sids[glyph_index];

	/* now, lookup the name itself */
	gname = cff_index_get_sid_string( &font->string_index, sid, psnames );

	if ( buffer_max > 0 ) {
		FT2_1_3_UInt  len = (FT2_1_3_UInt)ft_strlen( gname );


		if ( len >= buffer_max )
			len = buffer_max - 1;

		FT2_1_3_MEM_COPY( buffer, gname, len );
		((FT2_1_3_Byte*)buffer)[len] = 0;
	}

	FT2_1_3_FREE ( gname );
	error = CFF_Err_Ok;

Exit:
	return error;
}



/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    cff_get_name_index                                                 */
/*                                                                       */
/* <Description>                                                         */
/*    Uses the psnames module and the CFF font's charset to to return a  */
/*    a given glyph name's glyph index.                                  */
/*                                                                       */
/* <Input>                                                               */
/*    face       :: A handle to the source face object.                  */
/*                                                                       */
/*    glyph_name :: The glyph name.                                      */
/*                                                                       */
/* <Return>                                                              */
/*    Glyph index.  0 means `undefined character code'.                  */
/*                                                                       */
static FT2_1_3_UInt
cff_get_name_index( CFF_Face    face,
                    FT2_1_3_String*  glyph_name ) {
	CFF_Font         cff;
	CFF_Charset      charset;
	PSNames_Service  psnames;
	FT2_1_3_Memory        memory = FT2_1_3_FACE_MEMORY( face );
	FT2_1_3_String*       name;
	FT2_1_3_UShort        sid;
	FT2_1_3_UInt          i;
	FT2_1_3_Int           result;


	cff     = (CFF_FontRec *)face->extra.data;
	charset = &cff->charset;

	psnames = (PSNames_Service)FT2_1_3_Get_Module_Interface(
	              face->root.driver->root.library, "psnames" );

	for ( i = 0; i < cff->num_glyphs; i++ ) {
		sid = charset->sids[i];

		if ( sid > 390 )
			name = cff_index_get_name( &cff->string_index, sid - 391 );
		else
			name = (FT2_1_3_String *)psnames->adobe_std_strings( sid );

		result = ft_strcmp( glyph_name, name );

		if ( sid > 390 )
			FT2_1_3_FREE( name );

		if ( !result )
			return i;
	}

	return 0;
}


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/****                                                                 ****/
/****                                                                 ****/
/****                D R I V E R  I N T E R F A C E                   ****/
/****                                                                 ****/
/****                                                                 ****/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

static FT2_1_3_Module_Interface
cff_get_interface( CFF_Driver   driver,
                   const char*  module_interface ) {
	FT2_1_3_Module  sfnt;


#ifndef FT2_1_3_CONFIG_OPTION_NO_GLYPH_NAMES

	if ( ft_strcmp( (const char*)module_interface, "glyph_name" ) == 0 )
		return (FT2_1_3_Module_Interface)cff_get_glyph_name;

	if ( ft_strcmp( (const char*)module_interface, "name_index" ) == 0 )
		return (FT2_1_3_Module_Interface)cff_get_name_index;

#endif

	/* we simply pass our request to the `sfnt' module */
	sfnt = FT2_1_3_Get_Module( driver->root.root.library, "sfnt" );

	return sfnt ? sfnt->clazz->get_interface( sfnt, module_interface ) : 0;
}


/* The FT2_1_3_DriverInterface structure is defined in ftdriver.h. */

FT2_1_3_CALLBACK_TABLE_DEF
const FT2_1_3_Driver_ClassRec  cff_driver_class = {
	/* begin with the FT2_1_3_Module_Class fields */
	{
		ft_module_font_driver       |
		ft_module_driver_scalable   |
		ft_module_driver_has_hinter,

		sizeof( CFF_DriverRec ),
		"cff",
		0x10000L,
		0x20000L,

		0,   /* module-specific interface */

		(FT2_1_3_Module_Constructor)cff_driver_init,
		(FT2_1_3_Module_Destructor) cff_driver_done,
		(FT2_1_3_Module_Requester)  cff_get_interface,
	},

	/* now the specific driver fields */
	sizeof( TT_FaceRec ),
	sizeof( FT2_1_3_SizeRec ),
	sizeof( CFF_GlyphSlotRec ),

	(FT2_1_3_Face_InitFunc)       cff_face_init,
	(FT2_1_3_Face_DoneFunc)       cff_face_done,
	(FT2_1_3_Size_InitFunc)       cff_size_init,
	(FT2_1_3_Size_DoneFunc)       cff_size_done,
	(FT2_1_3_Slot_InitFunc)       cff_slot_init,
	(FT2_1_3_Slot_DoneFunc)       cff_slot_done,

	(FT2_1_3_Size_ResetPointsFunc)cff_size_reset,
	(FT2_1_3_Size_ResetPixelsFunc)cff_size_reset,

	(FT2_1_3_Slot_LoadFunc)       Load_Glyph,

	(FT2_1_3_Face_GetKerningFunc) Get_Kerning,
	(FT2_1_3_Face_AttachFunc)     0,
	(FT2_1_3_Face_GetAdvancesFunc)0,
};


/* END */
