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


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftlist.h"
#include "engines/ags/lib/freetype-2.1.3/ftoutln.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/tttables.h"
#include "engines/ags/lib/freetype-2.1.3/ftoutln.h"


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
FT2_1_3_New_Memory_Stream( FT_Library  library,
                      FT_Byte*    base,
                      FT_ULong    size,
                      FT_Stream   stream ) {
	FT2_1_3_UNUSED( library );

	FT_Stream_OpenMemory( stream, base, size );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Seek_Stream( FT_Stream  stream,
                FT_ULong   pos ) {
	return FT_Stream_Seek( stream, pos );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Skip_Stream( FT_Stream  stream,
                FT_Long    distance ) {
	return FT_Stream_Skip( stream, distance );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Read_Stream( FT_Stream  stream,
                FT_Byte*   buffer,
                FT_ULong   count ) {
	return FT_Stream_Read( stream, buffer, count );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Read_Stream_At( FT_Stream  stream,
                   FT_ULong   pos,
                   FT_Byte*   buffer,
                   FT_ULong   count ) {
	return FT_Stream_ReadAt( stream, pos, buffer, count );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Extract_Frame( FT_Stream  stream,
                  FT_ULong   count,
                  FT_Byte**  pbytes ) {
	return FT_Stream_ExtractFrame( stream, count, pbytes );
}


FT2_1_3_BASE_DEF( void )
FT2_1_3_Release_Frame( FT_Stream  stream,
                  FT_Byte**  pbytes ) {
	FT_Stream_ReleaseFrame( stream, pbytes );
}

FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_Access_Frame( FT_Stream  stream,
                 FT_ULong   count ) {
	return FT_Stream_EnterFrame( stream, count );
}


FT2_1_3_BASE_DEF( void )
FT2_1_3_Forget_Frame( FT_Stream  stream ) {
	FT_Stream_ExitFrame( stream );
}


/* END */
