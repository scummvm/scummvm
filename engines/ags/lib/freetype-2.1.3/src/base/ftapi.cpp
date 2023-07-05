/***************************************************************************/
/*                                                                         */
/*  ftapi.c                                                                */
/*                                                                         */
/*    The FreeType compatibility functions (body).                         */
/*                                                                         */
/*  Copyright 2002 by                                                      */
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
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftlist.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftoutln.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/tttables.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftoutln.h"


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/****                                                                 ****/
/****                                                                 ****/
/****                 C O M P A T I B I L I T Y                       ****/
/****                                                                 ****/
/****                                                                 ****/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

/* backwards compatibility API */

FT2_1_3_BASE_DEF( void )
FT2_1_3_New_Memory_Stream( FT2_1_3_Library  library,
                      FT_Byte*    base,
                      FT_ULong    size,
                      FT2_1_3_Stream   stream ) {
	FT2_1_3_UNUSED( library );

	FT2_1_3_Stream_OpenMemory( stream, base, size );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Seek_Stream( FT2_1_3_Stream  stream,
                FT_ULong   pos ) {
	return FT2_1_3_Stream_Seek( stream, pos );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Skip_Stream( FT2_1_3_Stream  stream,
                FT_Long    distance ) {
	return FT2_1_3_Stream_Skip( stream, distance );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Read_Stream( FT2_1_3_Stream  stream,
                FT_Byte*   buffer,
                FT_ULong   count ) {
	return FT2_1_3_Stream_Read( stream, buffer, count );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Read_Stream_At( FT2_1_3_Stream  stream,
                   FT_ULong   pos,
                   FT_Byte*   buffer,
                   FT_ULong   count ) {
	return FT2_1_3_Stream_ReadAt( stream, pos, buffer, count );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Extract_Frame( FT2_1_3_Stream  stream,
                  FT_ULong   count,
                  FT_Byte**  pbytes ) {
	return FT2_1_3_Stream_ExtractFrame( stream, count, pbytes );
}


FT2_1_3_BASE_DEF( void )
FT2_1_3_Release_Frame( FT2_1_3_Stream  stream,
                  FT_Byte**  pbytes ) {
	FT2_1_3_Stream_ReleaseFrame( stream, pbytes );
}

FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Access_Frame( FT2_1_3_Stream  stream,
                 FT_ULong   count ) {
	return FT2_1_3_Stream_EnterFrame( stream, count );
}


FT2_1_3_BASE_DEF( void )
FT2_1_3_Forget_Frame( FT2_1_3_Stream  stream ) {
	FT2_1_3_Stream_ExitFrame( stream );
}


/* END */
