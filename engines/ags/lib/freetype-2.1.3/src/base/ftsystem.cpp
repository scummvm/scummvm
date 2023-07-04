/***************************************************************************/
/*                                                                         */
/*  ftsystem.c                                                             */
/*                                                                         */
/*    ANSI-specific FreeType low-level system interface (body).            */
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

/*************************************************************************/
/*                                                                       */
/* This file contains the default interface used by FreeType to access   */
/* low-level, i.e. memory management, i/o access as well as thread       */
/* synchronisation.  It can be replaced by user-specific routines if     */
/* necessary.                                                            */
/*                                                                       */
/*************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/config/ftconfig.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftsystem.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/fterrors.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/fttypes.h"

#include <stdio.h>
#include <stdlib.h>

namespace AGS3 {
namespace FreeType213 {

/*************************************************************************/
/*                                                                       */
/*                       MEMORY MANAGEMENT INTERFACE                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* It is not necessary to do any error checking for the                  */
/* allocation-related functions.  This will be done by the higher level  */
/* routines like FT2_1_3_Alloc() or FT2_1_3_Realloc().                             */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    ft_alloc                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    The memory allocation function.                                    */
/*                                                                       */
/* <Input>                                                               */
/*    memory :: A pointer to the memory object.                          */
/*                                                                       */
/*    size   :: The requested size in bytes.                             */
/*                                                                       */
/* <Return>                                                              */
/*    The address of newly allocated block.                              */
/*                                                                       */
FT2_1_3_CALLBACK_DEF( void* )
ft_alloc( FT2_1_3_Memory  memory,
		  long       size ) {
	FT2_1_3_UNUSED( memory );

	return malloc( size );
}


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    ft_realloc                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    The memory reallocation function.                                  */
/*                                                                       */
/* <Input>                                                               */
/*    memory   :: A pointer to the memory object.                        */
/*                                                                       */
/*    cur_size :: The current size of the allocated memory block.        */
/*                                                                       */
/*    new_size :: The newly requested size in bytes.                     */
/*                                                                       */
/*    block    :: The current address of the block in memory.            */
/*                                                                       */
/* <Return>                                                              */
/*    The address of the reallocated memory block.                       */
/*                                                                       */
FT2_1_3_CALLBACK_DEF( void* )
ft_realloc( FT2_1_3_Memory  memory,
			long       cur_size,
			long       new_size,
			void*      block ) {
	FT2_1_3_UNUSED( memory );
	FT2_1_3_UNUSED( cur_size );

	return realloc( block, new_size );
}


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    ft_free                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    The memory release function.                                       */
/*                                                                       */
/* <Input>                                                               */
/*    memory  :: A pointer to the memory object.                         */
/*                                                                       */
/*    block   :: The address of block in memory to be freed.             */
/*                                                                       */
FT2_1_3_CALLBACK_DEF( void )
ft_free( FT2_1_3_Memory  memory,
		 void*      block ) {
	FT2_1_3_UNUSED( memory );

	free( block );
}


/*************************************************************************/
/*                                                                       */
/*                     RESOURCE MANAGEMENT INTERFACE                     */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_io

/* We use the macro STREAM_FILE for convenience to extract the       */
/* system-specific stream handle from a given FreeType stream object */
#define STREAM_FILE( stream )  ( (FILE*)stream->descriptor.pointer )


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    ft_ansi_stream_close                                               */
/*                                                                       */
/* <Description>                                                         */
/*    The function to close a stream.                                    */
/*                                                                       */
/* <Input>                                                               */
/*    stream :: A pointer to the stream object.                          */
/*                                                                       */
FT2_1_3_CALLBACK_DEF( void )
ft_ansi_stream_close( FT2_1_3_Stream  stream ) {
	fclose( STREAM_FILE( stream ) );

	stream->descriptor.pointer = NULL;
	stream->size               = 0;
	stream->base               = 0;
}


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    ft_ansi_stream_io                                                  */
/*                                                                       */
/* <Description>                                                         */
/*    The function to open a stream.                                     */
/*                                                                       */
/* <Input>                                                               */
/*    stream :: A pointer to the stream object.                          */
/*                                                                       */
/*    offset :: The position in the data stream to start reading.        */
/*                                                                       */
/*    buffer :: The address of buffer to store the read data.            */
/*                                                                       */
/*    count  :: The number of bytes to read from the stream.             */
/*                                                                       */
/* <Return>                                                              */
/*    The number of bytes actually read.                                 */
/*                                                                       */
FT2_1_3_CALLBACK_DEF( unsigned long )
ft_ansi_stream_io( FT2_1_3_Stream       stream,
				   unsigned long   offset,
				   unsigned char*  buffer,
				   unsigned long   count ) {
	FILE*  file;


	file = STREAM_FILE( stream );

	fseek( file, offset, SEEK_SET );

	return (unsigned long)fread( buffer, 1, count, file );
}


/* documentation is in ftobjs.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Stream_Open( FT2_1_3_Stream    stream,
				const char*  filepathname ) {
	FILE*  file;


	if ( !stream )
		return FT2_1_3_Err_Invalid_Stream_Handle;

	file = fopen( filepathname, "rb" );
	if ( !file ) {
		FT2_1_3_ERROR(( "FT2_1_3_Stream_Open:" ));
		FT2_1_3_ERROR(( " could not open `%s'\n", filepathname ));

		return FT2_1_3_Err_Cannot_Open_Resource;
	}

	fseek( file, 0, SEEK_END );
	stream->size = ftell( file );
	fseek( file, 0, SEEK_SET );

	stream->descriptor.pointer = file;
	stream->pathname.pointer = const_cast<char *>(filepathname);
	stream->pos                = 0;

	stream->read  = ft_ansi_stream_io;
	stream->close = ft_ansi_stream_close;

	FT2_1_3_TRACE1(( "FT2_1_3_Stream_Open:" ));
	FT2_1_3_TRACE1(( " opened `%s' (%d bytes) successfully\n",
				filepathname, stream->size ));

	return FT2_1_3_Err_Ok;
}


#ifdef FT2_1_3_DEBUG_MEMORY

extern FT2_1_3_Int
ft_mem_debug_init( FT2_1_3_Memory  memory );

extern void
ft_mem_debug_done( FT2_1_3_Memory  memory );

#endif


/* documentation is in ftobjs.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Memory )
FT2_1_3_New_Memory( void ) {
	FT2_1_3_Memory  memory;


	memory = (FT2_1_3_Memory)malloc( sizeof ( *memory ) );
	if ( memory ) {
		memory->user    = 0;
		memory->alloc   = ft_alloc;
		memory->realloc = ft_realloc;
		memory->free    = ft_free;
#ifdef FT2_1_3_DEBUG_MEMORY
		ft_mem_debug_init( memory );
#endif
	}

	return memory;
}


/* documentation is in ftobjs.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_Done_Memory( FT2_1_3_Memory  memory ) {
#ifdef FT2_1_3_DEBUG_MEMORY
	ft_mem_debug_done( memory );
#endif
	memory->free( memory, memory );
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
