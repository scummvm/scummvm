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
#include FT2_1_3_LIST_H
#include FT2_1_3_OUTLINE_H
#include FT2_1_3_INTERNAL_OBJECTS_H
#include FT2_1_3_INTERNAL_DEBUG_H
#include FT2_1_3_INTERNAL_STREAM_H
#include FT2_1_3_TRUETYPE_TABLES_H
#include FT2_1_3_OUTLINE_H


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
                      FT2_1_3_Byte*    base,
                      FT2_1_3_ULong    size,
                      FT2_1_3_Stream   stream ) {
	FT2_1_3_UNUSED( library );

	FT2_1_3_Stream_OpenMemory( stream, base, size );
}


FT2_1_3_BASE_DEF( FT2_1_3_Error )
FT2_1_3_Seek_Stream( FT2_1_3_Stream  stream,
                FT2_1_3_ULong   pos ) {
	return FT2_1_3_Stream_Seek( stream, pos );
}


FT2_1_3_BASE_DEF( FT2_1_3_Error )
FT2_1_3_Skip_Stream( FT2_1_3_Stream  stream,
                FT2_1_3_Long    distance ) {
	return FT2_1_3_Stream_Skip( stream, distance );
}


FT2_1_3_BASE_DEF( FT2_1_3_Error )
FT2_1_3_Read_Stream( FT2_1_3_Stream  stream,
                FT2_1_3_Byte*   buffer,
                FT2_1_3_ULong   count ) {
	return FT2_1_3_Stream_Read( stream, buffer, count );
}


FT2_1_3_BASE_DEF( FT2_1_3_Error )
FT2_1_3_Read_Stream_At( FT2_1_3_Stream  stream,
                   FT2_1_3_ULong   pos,
                   FT2_1_3_Byte*   buffer,
                   FT2_1_3_ULong   count ) {
	return FT2_1_3_Stream_ReadAt( stream, pos, buffer, count );
}


FT2_1_3_BASE_DEF( FT2_1_3_Error )
FT2_1_3_Extract_Frame( FT2_1_3_Stream  stream,
                  FT2_1_3_ULong   count,
                  FT2_1_3_Byte**  pbytes ) {
	return FT2_1_3_Stream_ExtractFrame( stream, count, pbytes );
}


FT2_1_3_BASE_DEF( void )
FT2_1_3_Release_Frame( FT2_1_3_Stream  stream,
                  FT2_1_3_Byte**  pbytes ) {
	FT2_1_3_Stream_ReleaseFrame( stream, pbytes );
}

FT2_1_3_BASE_DEF( FT2_1_3_Error )
FT2_1_3_Access_Frame( FT2_1_3_Stream  stream,
                 FT2_1_3_ULong   count ) {
	return FT2_1_3_Stream_EnterFrame( stream, count );
}


FT2_1_3_BASE_DEF( void )
FT2_1_3_Forget_Frame( FT2_1_3_Stream  stream ) {
	FT2_1_3_Stream_ExitFrame( stream );
}


/* END */
