/***************************************************************************/
/*                                                                         */
/*  ftgzip.c                                                               */
/*                                                                         */
/*    FreeType support for .gz compressed fileds                           */
/*                                                                         */
/*  this optional component relies on zlib. It should mainly be used to    */
/*  parse compressed PCF fonts, as found with many X11 server              */
/*  distributions.                                                         */
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
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftmemory.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"
#include <string.h>

#ifdef FT2_1_3_CONFIG_OPTION_USE_ZLIB

#ifdef FT2_1_3_CONFIG_OPTION_SYSTEM_ZLIB

#include "common/compression/zlib.h"

#else /* !SYSTEM_ZLIB */

/* in this case, we include our own modified sources of the ZLib   */
/* within the "ftgzip" component. The modifications were necessary */
/* to #include all files without conflicts, as well as preventing  */
/* the definition of "extern" functions that may cause linking     */
/* conflicts when a program is linked with both FreeType and the   */
/* original ZLib                                                   */

#define NO_DUMMY_DECL
#define BUILDFIXED /* save code size */
#define MY_ZCALLOC

#include "zlib.h"

#undef SLOW
#define SLOW 1 /* we can't use asm-optimized sources here !! */

#include "adler32.cpp"
#include "infblock.cpp"
#include "infcodes.cpp"
#include "inflate.cpp"
#include "inftrees.cpp"
#include "infutil.cpp"
#include "zutil.cpp"

#endif /* !SYSTEM_ZLIB */

#if (defined(__APPLE__) && defined(__MACH__))
int z_verbose = 0;

void z_error(/* should be const */char* message) {
	// do nothing
}
#endif

namespace AGS3 {
namespace FreeType213 {

/***************************************************************************/
/***************************************************************************/
/*****                                                                 *****/
/*****            Z L I B   M E M O R Y   M A N A G E M E N T          *****/
/*****                                                                 *****/
/***************************************************************************/
/***************************************************************************/

/* it's better to use FreeType memory routines instead of raw 'malloc/free' */


static voidpf
ft_gzip_alloc( FT_Memory  memory,
               uInt       items,
               uInt       size ) {
	FT_ULong    sz = (FT_ULong)size * items;
	FT_Pointer  p;

	FT2_1_3_MEM_ALLOC( p, sz );

	return (voidpf) p;
}


static void
ft_gzip_free( FT_Memory  memory,
              voidpf     address ) {
	FT2_1_3_MEM_FREE( address );
}


#ifndef FT2_1_3_CONFIG_OPTION_SYSTEM_ZLIB

local voidpf
zcalloc (opaque, items, size)
voidpf opaque;
unsigned items;
unsigned size;
{
	return ft_gzip_alloc( opaque, items, size );
}

local void
zcfree( voidpf  opaque,
        voidpf  ptr ) {
	ft_gzip_free( opaque, ptr );
}

#endif /* !SYSTEM_ZLIB */


/***************************************************************************/
/***************************************************************************/
/*****                                                                 *****/
/*****               Z L I B   F I L E   D E S C R I P T O R           *****/
/*****                                                                 *****/
/***************************************************************************/
/***************************************************************************/

#define  FT2_1_3_GZIP_BUFFER_SIZE          4096

typedef struct FT2_1_3_GZipFileRec_ {
	FT_Stream    source;         /* parent/source stream        */
	FT_Stream    stream;         /* embedding stream            */
	FT_Memory    memory;         /* memory allocator            */
	z_stream     zstream;        /* zlib input stream           */

	FT_ULong     start;          /* starting position, after .gz header */
	FT_Byte      input[ FT2_1_3_GZIP_BUFFER_SIZE ];  /* input read buffer */

	FT_Byte      buffer[ FT2_1_3_GZIP_BUFFER_SIZE ];  /* output buffer      */
	FT_ULong     pos;                            /* position in output */
	FT_Byte*     cursor;
	FT_Byte*     limit;

} FT2_1_3_GZipFileRec, *FT2_1_3_GZipFile;


/* gzip flag byte */
#define FT2_1_3_GZIP_ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define FT2_1_3_GZIP_HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define FT2_1_3_GZIP_EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define FT2_1_3_GZIP_ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define FT2_1_3_GZIP_COMMENT      0x10 /* bit 4 set: file comment present */
#define FT2_1_3_GZIP_RESERVED     0xE0 /* bits 5..7: reserved */


/* check and skip .gz header - we don't support "transparent" compression */
static FT_Error
ft_gzip_check_header( FT_Stream  stream ) {
	FT_Error  error;
	FT_Byte   head[4];

	if ( FT2_1_3_STREAM_SEEK( 0 )       ||
	        FT2_1_3_STREAM_READ( head, 4 ) )
		goto Exit;

	/* head[0] && head[1] are the magic numbers     */
	/* head[2] is the method, and head[3] the flags */
	if ( head[0] != 0x1f              ||
	        head[1] != 0x8b              ||
	        head[2] != Z_DEFLATED        ||
	        (head[3] & FT2_1_3_GZIP_RESERVED)  ) {
		error = FT2_1_3_Err_Invalid_File_Format;
		goto Exit;
	}

	/* skip time, xflags and os code */
	(void)FT2_1_3_STREAM_SKIP( 6 );

	/* skip the extra field */
	if ( head[3] & FT2_1_3_GZIP_EXTRA_FIELD ) {
		FT_UInt  len;

		if ( FT2_1_3_READ_USHORT_LE( len ) ||
		        FT2_1_3_STREAM_SKIP( len )    )
			goto Exit;
	}

	/* skip original file name */
	if ( head[3] & FT2_1_3_GZIP_ORIG_NAME )
		for (;;) {
			FT_UInt  c;

			if ( FT2_1_3_READ_BYTE( c) )
				goto Exit;

			if ( c == 0 )
				break;
		}

	/* skip .gz comment */
	if ( head[3] & FT2_1_3_GZIP_COMMENT )
		for (;;) {
			FT_UInt  c;

			if ( FT2_1_3_READ_BYTE( c) )
				goto Exit;

			if ( c == 0 )
				break;
		}

	/* skip CRC */
	if ( head[3] & FT2_1_3_GZIP_HEAD_CRC )
		if ( FT2_1_3_STREAM_SKIP( 2 ) )
			goto Exit;

Exit:
	return error;
}



static FT_Error
ft_gzip_file_init( FT2_1_3_GZipFile   zip,
                   FT_Stream     stream,
                   FT_Stream     source ) {
	z_stream*  zstream = &zip->zstream;
	FT_Error   error   = 0;

	zip->stream = stream;
	zip->source = source;
	zip->memory = stream->memory;

	zip->limit  = zip->buffer + FT2_1_3_GZIP_BUFFER_SIZE;
	zip->cursor = zip->limit;
	zip->pos    = 0;

	/* check and skip .gz header */
	{
		stream = source;

		error = ft_gzip_check_header( stream );
		if (error)
			goto Exit;

		zip->start = FT2_1_3_STREAM_POS();
	}

	/* initialize zlib - there is no zlib header in the compressed stream */
	zstream->zalloc = (alloc_func) ft_gzip_alloc;
	zstream->zfree  = (free_func)  ft_gzip_free;
	zstream->opaque = stream->memory;

	zstream->avail_in = 0;
	zstream->next_in  = zip->buffer;

	if ( inflateInit2( zstream, -MAX_WBITS ) != Z_OK ||
	        zstream->next_in == NULL                     ) {
		error = FT2_1_3_Err_Invalid_File_Format;
		goto Exit;
	}

Exit:
	return error;
}



static void
ft_gzip_file_done( FT2_1_3_GZipFile  zip ) {
	z_stream*  zstream = &zip->zstream;

	/* clear the rest */
	zstream->zalloc    = NULL;
	zstream->zfree     = NULL;
	zstream->opaque    = NULL;
	zstream->next_in   = NULL;
	zstream->next_out  = NULL;
	zstream->avail_in  = 0;
	zstream->avail_out = 0;

	zip->memory = NULL;
	zip->source = NULL;
	zip->stream = NULL;
}


static FT_Error
ft_gzip_file_reset( FT2_1_3_GZipFile  zip ) {
	FT_Stream  stream = zip->source;
	FT_Error   error;

	if ( !FT2_1_3_STREAM_SEEK( zip->start ) ) {
		z_stream*  zstream = &zip->zstream;

		inflateReset( zstream );

		zstream->avail_in  = 0;
		zstream->next_in   = zip->input;
		zstream->avail_out = 0;
		zstream->next_out  = zip->buffer;

		zip->limit  = zip->buffer + FT2_1_3_GZIP_BUFFER_SIZE;
		zip->cursor = zip->limit;
		zip->pos    = 0;
	}
	return  error;
}


static FT_Error
ft_gzip_file_fill_input( FT2_1_3_GZipFile  zip ) {
	z_stream*  zstream = &zip->zstream;
	FT_Stream  stream  = zip->source;
	FT_ULong   size;

	if ( stream->read ) {
		size = stream->read( stream, stream->pos, zip->input, FT2_1_3_GZIP_BUFFER_SIZE );
		if ( size == 0 )
			return FT2_1_3_Err_Invalid_Stream_Operation;
	} else {
		size = stream->size - stream->pos;
		if ( size > FT2_1_3_GZIP_BUFFER_SIZE )
			size = FT2_1_3_GZIP_BUFFER_SIZE;

		if ( size == 0 )
			return FT2_1_3_Err_Invalid_Stream_Operation;

		FT2_1_3_MEM_COPY( zip->input, stream->base + stream->pos, size );
	}
	stream->pos += size;

	zstream->next_in  = zip->input;
	zstream->avail_in = size;

	return 0;
}



static FT_Error
ft_gzip_file_fill_output( FT2_1_3_GZipFile  zip ) {
	z_stream*  zstream = &zip->zstream;
	FT_Error   error   = 0;

	zip->cursor        = zip->buffer;
	zstream->next_out  = zip->cursor;
	zstream->avail_out = FT2_1_3_GZIP_BUFFER_SIZE;

	while ( zstream->avail_out > 0 ) {
		int  err;

		if ( zstream->avail_in == 0 ) {
			error = ft_gzip_file_fill_input( zip );
			if ( error )
				break;
		}

		err = inflate( zstream, Z_NO_FLUSH );

		if ( err == Z_STREAM_END ) {
			zip->limit = zstream->next_out;
			break;
		} else if ( err != Z_OK ) {
			error = FT2_1_3_Err_Invalid_Stream_Operation;
			break;
		}
	}
	return error;
}


/* fill output buffer, 'count' must be <= FT2_1_3_GZIP_BUFFER_SIZE */
static FT_Error
ft_gzip_file_skip_output( FT2_1_3_GZipFile  zip,
                          FT_ULong     count ) {
	FT_Error   error   = 0;
	FT_ULong   delta;

	for (;;) {
		delta = (FT_ULong)( zip->limit - zip->cursor );
		if ( delta >= count )
			delta = count;

		zip->cursor += delta;
		zip->pos    += delta;

		count -= delta;
		if ( count == 0 )
			break;

		error = ft_gzip_file_fill_output( zip );
		if ( error )
			break;
	}

	return error;
}


static FT_ULong
ft_gzip_file_io( FT2_1_3_GZipFile   zip,
                 FT_ULong      pos,
                 FT_Byte*      buffer,
                 FT_ULong      count ) {
	FT_ULong   result = 0;
	FT_Error   error;

	/* reset inflate stream if we're seeking backwards        */
	/* yes, that's not too efficient, but it saves memory :-) */
	if ( pos < zip->pos ) {
		error = ft_gzip_file_reset( zip );
		if ( error ) goto Exit;
	}

	/* skip unwanted bytes */
	if ( pos > zip->pos ) {
		error = ft_gzip_file_skip_output( zip, (FT_ULong)( pos - zip->pos ) );
		if (error)
			goto Exit;
	}

	if ( count == 0 )
		goto Exit;

	/* now read the data */
	for (;;) {
		FT_ULong   delta;

		delta = (FT_ULong)( zip->limit - zip->cursor );
		if ( delta >= count )
			delta = count;

		FT2_1_3_MEM_COPY( buffer, zip->cursor, delta );
		buffer      += delta;
		result      += delta;
		zip->cursor += delta;
		zip->pos    += delta;

		count -= delta;
		if ( count == 0 )
			break;

		error = ft_gzip_file_fill_output( zip );
		if (error)
			break;
	}

Exit:
	return result;
}


/***************************************************************************/
/***************************************************************************/
/*****                                                                 *****/
/*****               G Z   E M B E D D I N G   S T R E A M             *****/
/*****                                                                 *****/
/***************************************************************************/
/***************************************************************************/

static void
ft_gzip_stream_close( FT_Stream  stream ) {
	FT2_1_3_GZipFile  zip    = (FT2_1_3_GZipFile) stream->descriptor.pointer;
	FT_Memory    memory = stream->memory;

	if ( zip ) {
		/* finalize gzip file descriptor */
		ft_gzip_file_done( zip );

		FT2_1_3_FREE( zip );

		stream->descriptor.pointer = NULL;
	}
}


static FT_ULong
ft_gzip_stream_io( FT_Stream   stream,
                   FT_ULong    pos,
                   FT_Byte*    buffer,
                   FT_ULong    count ) {
	FT2_1_3_GZipFile  zip = (FT2_1_3_GZipFile) stream->descriptor.pointer;

	return ft_gzip_file_io( zip, pos, buffer, count );
}


FT2_1_3_EXPORT_DEF( FT_Error )
FT_Stream_OpenGzip( FT_Stream    stream,
                    FT_Stream    source ) {
	FT_Error     error;
	FT_Memory    memory = source->memory;
	FT2_1_3_GZipFile  zip;

	FT2_1_3_ZERO( stream );
	stream->memory = memory;

	if ( !FT2_1_3_NEW( zip ) ) {
		error = ft_gzip_file_init( zip, stream, source );
		if ( error ) {
			FT2_1_3_FREE( zip );
			goto Exit;
		}

		stream->descriptor.pointer = zip;
	}

	stream->size = 0x7FFFFFFF;  /* don't know the real size !! */
	stream->pos   = 0;
	stream->base  = 0;
	stream->read  = ft_gzip_stream_io;
	stream->close = ft_gzip_stream_close;

Exit:
	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3

#else  /* !FT2_1_3_CONFIG_OPTION_USE_ZLIB */

FT2_1_3_EXPORT_DEF( AGS3::FreeType213::FT_Error )
FT_Stream_OpenGzip( AGS3::FreeType213::FT_Stream    stream,
                    AGS3::FreeType213::FT_Stream    source ) {
	FT2_1_3_UNUSED( stream );
	FT2_1_3_UNUSED( source );

	return FT2_1_3_Err_Unimplemented_Feature;
}

#endif /* !FT2_1_3_CONFIG_OPTION_USE_ZLIB */
