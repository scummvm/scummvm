/***************************************************************************/
/*                                                                         */
/*  ttpload.c                                                              */
/*                                                                         */
/*    TrueType glyph data/program tables loader (body).                    */
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
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/tttags.h"

#include "ttpload.h"

#include "tterrors.h"


/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_ttpload


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    tt_face_load_loca                                                  */
/*                                                                       */
/* <Description>                                                         */
/*    Loads the locations table.                                         */
/*                                                                       */
/* <InOut>                                                               */
/*    face   :: A handle to the target face object.                      */
/*                                                                       */
/* <Input>                                                               */
/*    stream :: The input stream.                                        */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
tt_face_load_loca( TT_Face    face,
                   FT2_1_3_Stream  stream ) {
	FT2_1_3_Error   error;
	FT2_1_3_Memory  memory = stream->memory;
	FT2_1_3_Short   LongOffsets;
	FT2_1_3_ULong   table_len;


	FT2_1_3_TRACE2(( "Locations " ));
	LongOffsets = face->header.Index_To_Loc_Format;

	error = face->goto_table( face, TTAG_loca, stream, &table_len );
	if ( error ) {
		error = TT_Err_Locations_Missing;
		goto Exit;
	}

	if ( LongOffsets != 0 ) {
		face->num_locations = (FT2_1_3_UShort)( table_len >> 2 );

		FT2_1_3_TRACE2(( "(32bit offsets): %12d ", face->num_locations ));

		if ( FT2_1_3_NEW_ARRAY( face->glyph_locations, face->num_locations ) )
			goto Exit;

		if ( FT2_1_3_FRAME_ENTER( face->num_locations * 4L ) )
			goto Exit;

		{
			FT2_1_3_Long*  loc   = face->glyph_locations;
			FT2_1_3_Long*  limit = loc + face->num_locations;


			for ( ; loc < limit; loc++ )
				*loc = FT2_1_3_GET_LONG();
		}

		FT2_1_3_FRAME_EXIT();
	} else {
		face->num_locations = (FT2_1_3_UShort)( table_len >> 1 );

		FT2_1_3_TRACE2(( "(16bit offsets): %12d ", face->num_locations ));

		if ( FT2_1_3_NEW_ARRAY( face->glyph_locations, face->num_locations ) )
			goto Exit;

		if ( FT2_1_3_FRAME_ENTER( face->num_locations * 2L ) )
			goto Exit;
		{
			FT2_1_3_Long*  loc   = face->glyph_locations;
			FT2_1_3_Long*  limit = loc + face->num_locations;


			for ( ; loc < limit; loc++ )
				*loc = (FT2_1_3_Long)( (FT2_1_3_ULong)FT2_1_3_GET_USHORT() * 2 );
		}
		FT2_1_3_FRAME_EXIT();
	}

	FT2_1_3_TRACE2(( "loaded\n" ));

Exit:
	return error;
}


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    tt_face_load_cvt                                                   */
/*                                                                       */
/* <Description>                                                         */
/*    Loads the control value table into a face object.                  */
/*                                                                       */
/* <InOut>                                                               */
/*    face   :: A handle to the target face object.                      */
/*                                                                       */
/* <Input>                                                               */
/*    stream :: A handle to the input stream.                            */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
tt_face_load_cvt( TT_Face    face,
                  FT2_1_3_Stream  stream ) {
	FT2_1_3_Error   error;
	FT2_1_3_Memory  memory = stream->memory;
	FT2_1_3_ULong   table_len;


	FT2_1_3_TRACE2(( "CVT " ));

	error = face->goto_table( face, TTAG_cvt, stream, &table_len );
	if ( error ) {
		FT2_1_3_TRACE2(( "is missing!\n" ));

		face->cvt_size = 0;
		face->cvt      = NULL;
		error          = TT_Err_Ok;

		goto Exit;
	}

	face->cvt_size = table_len / 2;

	if ( FT2_1_3_NEW_ARRAY( face->cvt, face->cvt_size ) )
		goto Exit;

	if ( FT2_1_3_FRAME_ENTER( face->cvt_size * 2L ) )
		goto Exit;

	{
		FT2_1_3_Short*  cur   = face->cvt;
		FT2_1_3_Short*  limit = cur + face->cvt_size;


		for ( ; cur <  limit; cur++ )
			*cur = FT2_1_3_GET_SHORT();
	}

	FT2_1_3_FRAME_EXIT();
	FT2_1_3_TRACE2(( "loaded\n" ));

Exit:
	return error;
}


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    tt_face_load_fpgm                                                  */
/*                                                                       */
/* <Description>                                                         */
/*    Loads the font program and the cvt program.                        */
/*                                                                       */
/* <InOut>                                                               */
/*    face   :: A handle to the target face object.                      */
/*                                                                       */
/* <Input>                                                               */
/*    stream :: A handle to the input stream.                            */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
tt_face_load_fpgm( TT_Face    face,
                   FT2_1_3_Stream  stream ) {
	FT2_1_3_Error   error;
	FT2_1_3_ULong   table_len;


	FT2_1_3_TRACE2(( "Font program " ));

	/* The font program is optional */
	error = face->goto_table( face, TTAG_fpgm, stream, &table_len );
	if ( error ) {
		face->font_program      = NULL;
		face->font_program_size = 0;

		FT2_1_3_TRACE2(( "is missing!\n" ));
	} else {
		face->font_program_size = table_len;
		if ( FT2_1_3_FRAME_EXTRACT( table_len, face->font_program ) )
			goto Exit;

		FT2_1_3_TRACE2(( "loaded, %12d bytes\n", face->font_program_size ));
	}

	FT2_1_3_TRACE2(( "Prep program " ));

	error = face->goto_table( face, TTAG_prep, stream, &table_len );
	if ( error ) {
		face->cvt_program      = NULL;
		face->cvt_program_size = 0;
		error                  = TT_Err_Ok;

		FT2_1_3_TRACE2(( "is missing!\n" ));
	} else {
		face->cvt_program_size = table_len;
		if ( FT2_1_3_FRAME_EXTRACT( table_len, face->cvt_program ) )
			goto Exit;

		FT2_1_3_TRACE2(( "loaded, %12d bytes\n", face->cvt_program_size ));
	}

Exit:
	return error;
}


/* END */
