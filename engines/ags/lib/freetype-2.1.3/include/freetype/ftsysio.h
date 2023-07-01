#ifndef __FT2_1_3_SYSTEM_IO_H__
#define __FT2_1_3_SYSTEM_IO_H__

/************************************************************************/
/************************************************************************/
/*****                                                              *****/
/*****    NOTE: THE CONTENT OF THIS FILE IS NOT CURRENTLY USED      *****/
/*****          IN NORMAL BUILDS.  CONSIDER IT EXPERIMENTAL.        *****/
/*****                                                              *****/
/************************************************************************/
/************************************************************************/


/********************************************************************
 *
 *  designing custom streams is a bit different now
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobject.h"

FT2_1_3_BEGIN_HEADER

/*@*******************************************************************
 *
 * @type: FT2_1_3_Stream
 *
 * @description:
 *   handle to an input stream object. These are also @FT2_1_3_Object handles
 */
typedef struct FT2_1_3_StreamRec_*    FT2_1_3_Stream;


/*@*******************************************************************
 *
 * @type: FT2_1_3_Stream_Class
 *
 * @description:
 *   opaque handle to a @FT2_1_3_Stream_ClassRec class structure describing
 *   the methods of input streams
 */
typedef const struct FT2_1_3_Stream_ClassRec_*   FT2_1_3_Stream_Class;


/*@*******************************************************************
 *
 * @functype: FT2_1_3_Stream_ReadFunc
 *
 * @description:
 *   a method used to read bytes from an input stream into memory
 *
 * @input:
 *   stream  :: target stream handle
 *   buffer  :: target buffer address
 *   size    :: number of bytes to read
 *
 * @return:
 *   number of bytes effectively read. Must be <= 'size'.
 */
typedef FT2_1_3_ULong  (*FT2_1_3_Stream_ReadFunc)( FT2_1_3_Stream   stream,
        FT2_1_3_Byte*    buffer,
        FT2_1_3_ULong    size );


/*@*******************************************************************
 *
 * @functype: FT2_1_3_Stream_SeekFunc
 *
 * @description:
 *   a method used to seek to a new position within a stream
 *
 * @input:
 *   stream  :: target stream handle
 *   pos     :: new read position, from start of stream
 *
 * @return:
 *   error code. 0 means success
 */
typedef FT2_1_3_Error  (*FT2_1_3_Stream_SeekFunc)( FT2_1_3_Stream   stream,
        FT2_1_3_ULong    pos );


/*@*******************************************************************
 *
 * @struct: FT2_1_3_Stream_ClassRec
 *
 * @description:
 *   a structure used to describe an input stream class
 *
 * @input:
 *   clazz       :: root @FT2_1_3_ClassRec fields
 *   stream_read :: stream byte read method
 *   stream_seek :: stream seek method
 */
typedef struct FT2_1_3_Stream_ClassRec_ {
	FT2_1_3_ClassRec          clazz;
	FT2_1_3_Stream_ReadFunc   stream_read;
	FT2_1_3_Stream_SeekFunc   stream_seek;

} FT2_1_3_Stream_ClassRec;

/* */

#define  FT2_1_3_STREAM_CLASS(x)        ((FT2_1_3_Stream_Class)(x))
#define  FT2_1_3_STREAM_CLASS__READ(x)  FT2_1_3_STREAM_CLASS(x)->stream_read
#define  FT2_1_3_STREAM_CLASS__SEEK(x)  FT2_1_3_STREAM_CLASS(x)->stream_seek;

/*@*******************************************************************
 *
 * @struct: FT2_1_3_StreamRec
 *
 * @description:
 *   the input stream object structure. See @FT2_1_3_Stream_ClassRec for
 *   its class descriptor
 *
 * @fields:
 *   object      :: root @FT2_1_3_ObjectRec fields
 *   size        :: size of stream in bytes (0 if unknown)
 *   pos         :: current position within stream
 *   base        :: for memory-based streams, the address of the stream's
 *                  first data byte in memory. NULL otherwise
 *
 *   cursor      :: the current cursor position within an input stream
 *                  frame. Only valid within a FT2_1_3_FRAME_ENTER .. FT2_1_3_FRAME_EXIT
 *                  block; NULL otherwise
 *
 *   limit       :: the current frame limit within a FT2_1_3_FRAME_ENTER ..
 *                  FT2_1_3_FRAME_EXIT block. NULL otherwise
 */
typedef struct FT2_1_3_StreamRec_ {
	FT2_1_3_ObjectRec        object;
	FT2_1_3_ULong            size;
	FT2_1_3_ULong            pos;
	const FT2_1_3_Byte*      base;
	const FT2_1_3_Byte*      cursor;
	const FT2_1_3_Byte*      limit;

} FT2_1_3_StreamRec;

/* some useful macros */
#define  FT2_1_3_STREAM(x)    ((FT2_1_3_Stream)(x))
#define  FT2_1_3_STREAM_P(x)  ((FT2_1_3_Stream*)(x))

#define  FT2_1_3_STREAM__READ(x)  FT2_1_3_STREAM_CLASS__READ(FT2_1_3_OBJECT__CLASS(x))
#define  FT2_1_3_STREAM__SEEK(x)  FT2_1_3_STREAM_CLASS__SEEK(FT2_1_3_OBJECT__CLASS(x))

#define  FT2_1_3_STREAM_IS_BASED(x)  ( FT2_1_3_STREAM(x)->base != NULL )

/* */

/* create new memory-based stream */
FT2_1_3_BASE( FT2_1_3_Error )   ft_stream_new_memory( const FT2_1_3_Byte*  stream_base,
        FT2_1_3_ULong        stream_size,
        FT2_1_3_Memory       memory,
        FT2_1_3_Stream      *astream );

FT2_1_3_BASE( FT2_1_3_Error )   ft_stream_new_iso( const char*  pathanme,
        FT2_1_3_Memory    memory,
        FT2_1_3_Stream   *astream );


/* handle to default stream class implementation for a given build */
/* this is used by "FT2_1_3_New_Face"                                   */
/*                                                                 */
FT2_1_3_APIVAR( FT2_1_3_Type )   ft_stream_default_type;

FT2_1_3_END_HEADER

#endif /* __FT2_1_3_SYSTEM_STREAM_H__ */
