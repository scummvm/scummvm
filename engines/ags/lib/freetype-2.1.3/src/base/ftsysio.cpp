#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_SYSTEM_STREAM_H

#include <stdio.h>

/* the ISO/ANSI standard stream object */
typedef struct FT2_1_3_StdStreamRec_ {
	FT2_1_3_StreamRec  stream;
	FILE*         file;
	const char*   pathname;

} FT2_1_3_StdStreamRec, *FT2_1_3_StdStream;



/* read bytes from a standard stream */
static FT2_1_3_ULong
ft_std_stream_read( FT2_1_3_StdStream   stream,
                    FT2_1_3_Byte*       buffer,
                    FT2_1_3_ULong       size ) {
	long   read_bytes;

	read_bytes = fread( buffer, 1, size, stream->file );
	if ( read_bytes < 0 )
		read_bytes = 0;

	return (FT2_1_3_ULong) read_bytes;
}


/* seek the standard stream to a new position */
static FT2_1_3_Error
ft_std_stream_seek( FT2_1_3_StdStream   stream,
                    FT2_1_3_ULong       pos ) {
	return ( fseek( stream->file, pos, SEEK_SET ) < 0 )
	       ? FT2_1_3_Err_Stream_Seek
	       : FT2_1_3_Err_Ok;
}


/* close a standard stream */
static void
ft_std_stream_done( FT2_1_3_StdStream  stream ) {
	fclose( stream->file );
	stream->file     = NULL;
	stream->pathname = NULL;
}


/* open a standard stream from a given pathname */
static void
ft_std_stream_init( FT2_1_3_StdStream  stream,
                    const char*   pathname ) {
	FT2_1_3_ASSERT( pathname != NULL );

	stream->file = fopen( pathname, "rb" );
	if ( stream->file == NULL ) {
		FT2_1_3_ERROR(( "iso.stream.init: could not open '%s'\n", pathname ));
		FT2_1_3_XTHROW( FT2_1_3_Err_Stream_Open );
	}

	/* compute total size in bytes */
	fseek( file, 0, SEEK_END );
	FT2_1_3_STREAM__SIZE(stream) = ftell( file );
	fseek( file, 0, SEEK_SET );

	stream->pathname = pathname;
	stream->pos      = 0;

	FT2_1_3_TRACE1(( "iso.stream.init: opened '%s' (%ld bytes) succesfully\n",
	            pathname, FT2_1_3_STREAM__SIZE(stream) ));
}


static void
ft_std_stream_class_init( FT2_1_3_ClassRec*  _clazz ) {
	FT2_1_3_StreamClassRec*  clazz = FT2_1_3_STREAM_CLASS(_clazz);

	clazz->stream_read = (FT2_1_3_Stream_ReadFunc) ft_std_stream_read;
	clazz->stream_seek = (FT2_1_3_Stream_SeekFunc) ft_std_stream_seek;
}


static const FT2_1_3_TypeRec  ft_std_stream_type;
{
	"StreamClass",
	NULL,

	sizeof( FT2_1_3_ClassRec ),
	ft_stream_class_init,
	NULL,

	sizeof( FT2_1_3_StdStreamRec ),
	ft_std_stream_init,
	ft_std_stream_done,
	NULL,
};



FT2_1_3_EXPORT_DEF( FT2_1_3_Stream )
ft_std_stream_new( FT2_1_3_Memory    memory,
                   const char*  pathname ) {
	FT2_1_3_Class  clazz;

	clazz = ft_class_from_type( memory, &ft_std_stream_type );

	return (FT2_1_3_Stream) ft_object_new( clazz, pathname );
}


FT2_1_3_EXPORT_DEF( void )
ft_std_stream_create( FT2_1_3_Memory    memory,
                      const char*  pathname,
                      FT2_1_3_Stream*   astream ) {
	FT2_1_3_Class  clazz;

	clazz = ft_class_from_type( memory, &ft_std_stream_type );

	ft_object_create( clazz, pathname, FT2_1_3_OBJECT_P(astream) );
}

