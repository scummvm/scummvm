/***************************************************************************/
/*                                                                         */
/*  ftstream.h                                                             */
/*                                                                         */
/*    Stream handling (specification).                                     */
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


#ifndef __FTSTREAM_H__
#define __FTSTREAM_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_SYSTEM_H
#include FT2_1_3_INTERNAL_OBJECTS_H


FT2_1_3_BEGIN_HEADER


/* format of an 8-bit frame_op value:           */
/*                                              */
/* bit  76543210                                */
/*      xxxxxxes                                */
/*                                              */
/* s is set to 1 if the value is signed.        */
/* e is set to 1 if the value is little-endian. */
/* xxx is a command.                            */

#define FT2_1_3_FRAME_OP_SHIFT         2
#define FT2_1_3_FRAME_OP_SIGNED        1
#define FT2_1_3_FRAME_OP_LITTLE        2
#define FT2_1_3_FRAME_OP_COMMAND( x )  ( x >> FT2_1_3_FRAME_OP_SHIFT )

#define FT2_1_3_MAKE_FRAME_OP( command, little, sign ) \
          ( ( command << FT2_1_3_FRAME_OP_SHIFT ) | ( little << 1 ) | sign )

#define FT2_1_3_FRAME_OP_END    0
#define FT2_1_3_FRAME_OP_START  1  /* start a new frame     */
#define FT2_1_3_FRAME_OP_BYTE   2  /* read 1-byte value     */
#define FT2_1_3_FRAME_OP_SHORT  3  /* read 2-byte value     */
#define FT2_1_3_FRAME_OP_LONG   4  /* read 4-byte value     */
#define FT2_1_3_FRAME_OP_OFF3   5  /* read 3-byte value     */
#define FT2_1_3_FRAME_OP_BYTES  6  /* read a bytes sequence */


typedef enum  FT2_1_3_Frame_Op_ {
	ft_frame_end       = 0,
	ft_frame_start     = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_START, 0, 0 ),

	ft_frame_byte      = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_BYTE,  0, 0 ),
	ft_frame_schar     = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_BYTE,  0, 1 ),

	ft_frame_ushort_be = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_SHORT, 0, 0 ),
	ft_frame_short_be  = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_SHORT, 0, 1 ),
	ft_frame_ushort_le = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_SHORT, 1, 0 ),
	ft_frame_short_le  = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_SHORT, 1, 1 ),

	ft_frame_ulong_be  = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_LONG, 0, 0 ),
	ft_frame_long_be   = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_LONG, 0, 1 ),
	ft_frame_ulong_le  = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_LONG, 1, 0 ),
	ft_frame_long_le   = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_LONG, 1, 1 ),

	ft_frame_uoff3_be  = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_OFF3, 0, 0 ),
	ft_frame_off3_be   = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_OFF3, 0, 1 ),
	ft_frame_uoff3_le  = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_OFF3, 1, 0 ),
	ft_frame_off3_le   = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_OFF3, 1, 1 ),

	ft_frame_bytes     = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_BYTES, 0, 0 ),
	ft_frame_skip      = FT2_1_3_MAKE_FRAME_OP( FT2_1_3_FRAME_OP_BYTES, 0, 1 )

} FT2_1_3_Frame_Op;


typedef struct  FT2_1_3_Frame_Field_ {
	FT2_1_3_Byte    value;
	FT2_1_3_Byte    size;
	FT2_1_3_UShort  offset;

} FT2_1_3_Frame_Field;


/* Construct an FT2_1_3_Frame_Field out of a structure type and a field name. */
/* The structure type must be set in the FT2_1_3_STRUCTURE macro before       */
/* calling the FT2_1_3_FRAME_START() macro.                                   */
/*                                                                       */
#define FT2_1_3_FIELD_SIZE( f ) \
          (FT2_1_3_Byte)sizeof ( ((FT2_1_3_STRUCTURE*)0)->f )

#define FT2_1_3_FIELD_SIZE_DELTA( f ) \
          (FT2_1_3_Byte)sizeof ( ((FT2_1_3_STRUCTURE*)0)->f[0] )

#define FT2_1_3_FIELD_OFFSET( f ) \
          (FT2_1_3_UShort)( offsetof( FT2_1_3_STRUCTURE, f ) )

#define FT2_1_3_FRAME_FIELD( frame_op, field ) \
          {                               \
            frame_op,                     \
            FT2_1_3_FIELD_SIZE( field ),       \
            FT2_1_3_FIELD_OFFSET( field )      \
          }

#define FT2_1_3_MAKE_EMPTY_FIELD( frame_op )  { frame_op, 0, 0 }

#define FT2_1_3_FRAME_START( size )   { ft_frame_start, 0, size }
#define FT2_1_3_FRAME_END             { ft_frame_end, 0, 0 }

#define FT2_1_3_FRAME_LONG( f )       FT2_1_3_FRAME_FIELD( ft_frame_long_be, f )
#define FT2_1_3_FRAME_ULONG( f )      FT2_1_3_FRAME_FIELD( ft_frame_ulong_be, f )
#define FT2_1_3_FRAME_SHORT( f )      FT2_1_3_FRAME_FIELD( ft_frame_short_be, f )
#define FT2_1_3_FRAME_USHORT( f )     FT2_1_3_FRAME_FIELD( ft_frame_ushort_be, f )
#define FT2_1_3_FRAME_OFF3( f )       FT2_1_3_FRAME_FIELD( ft_frame_off3_be, f )
#define FT2_1_3_FRAME_UOFF3( f )      FT2_1_3_FRAME_FIELD( ft_frame_uoff3_be, f )
#define FT2_1_3_FRAME_BYTE( f )       FT2_1_3_FRAME_FIELD( ft_frame_byte, f )
#define FT2_1_3_FRAME_CHAR( f )       FT2_1_3_FRAME_FIELD( ft_frame_schar, f )

#define FT2_1_3_FRAME_LONG_LE( f )    FT2_1_3_FRAME_FIELD( ft_frame_long_le, f )
#define FT2_1_3_FRAME_ULONG_LE( f )   FT2_1_3_FRAME_FIELD( ft_frame_ulong_le, f )
#define FT2_1_3_FRAME_SHORT_LE( f )   FT2_1_3_FRAME_FIELD( ft_frame_short_le, f )
#define FT2_1_3_FRAME_USHORT_LE( f )  FT2_1_3_FRAME_FIELD( ft_frame_ushort_le, f )
#define FT2_1_3_FRAME_OFF3_LE( f )    FT2_1_3_FRAME_FIELD( ft_frame_off3_le, f )
#define FT2_1_3_FRAME_UOFF3_LE( f )   FT2_1_3_FRAME_FIELD( ft_frame_uoff3_le, f )

#define FT2_1_3_FRAME_SKIP_LONG       { ft_frame_long_be, 0, 0 }
#define FT2_1_3_FRAME_SKIP_SHORT      { ft_frame_short_be, 0, 0 }
#define FT2_1_3_FRAME_SKIP_BYTE       { ft_frame_byte, 0, 0 }

#define FT2_1_3_FRAME_BYTES( field, count ) \
          {                            \
            ft_frame_bytes,            \
            count,                     \
            FT2_1_3_FIELD_OFFSET( field )   \
          }

#define FT2_1_3_FRAME_SKIP_BYTES( count )  { ft_frame_skip, count, 0 }


/*************************************************************************/
/*                                                                       */
/* Integer extraction macros -- the `buffer' parameter must ALWAYS be of */
/* type `char*' or equivalent (1-byte elements).                         */
/*                                                                       */

#define FT2_1_3_BYTE_( p, i )  ( ((const FT2_1_3_Byte*)(p))[(i)] )
#define FT2_1_3_INT8_( p, i )  ( ((const FT2_1_3_Char*)(p))[(i)] )

#define FT2_1_3_INT16( x )   ( (FT2_1_3_Int16)(x)  )
#define FT2_1_3_UINT16( x )  ( (FT2_1_3_UInt16)(x) )
#define FT2_1_3_INT32( x )   ( (FT2_1_3_Int32)(x)  )
#define FT2_1_3_UINT32( x )  ( (FT2_1_3_UInt32)(x) )

#define FT2_1_3_BYTE_I16( p, i, s )  ( FT2_1_3_INT16(  FT2_1_3_BYTE_( p, i ) ) << (s) )
#define FT2_1_3_BYTE_U16( p, i, s )  ( FT2_1_3_UINT16( FT2_1_3_BYTE_( p, i ) ) << (s) )
#define FT2_1_3_BYTE_I32( p, i, s )  ( FT2_1_3_INT32(  FT2_1_3_BYTE_( p, i ) ) << (s) )
#define FT2_1_3_BYTE_U32( p, i, s )  ( FT2_1_3_UINT32( FT2_1_3_BYTE_( p, i ) ) << (s) )

#define FT2_1_3_INT8_I16( p, i, s )  ( FT2_1_3_INT16(  FT2_1_3_INT8_( p, i ) ) << (s) )
#define FT2_1_3_INT8_U16( p, i, s )  ( FT2_1_3_UINT16( FT2_1_3_INT8_( p, i ) ) << (s) )
#define FT2_1_3_INT8_I32( p, i, s )  ( FT2_1_3_INT32(  FT2_1_3_INT8_( p, i ) ) << (s) )
#define FT2_1_3_INT8_U32( p, i, s )  ( FT2_1_3_UINT32( FT2_1_3_INT8_( p, i ) ) << (s) )


#define FT2_1_3_PEEK_SHORT( p )  FT2_1_3_INT16( FT2_1_3_INT8_I16( p, 0, 8) | \
                                      FT2_1_3_BYTE_I16( p, 1, 0) )

#define FT2_1_3_PEEK_USHORT( p )  FT2_1_3_UINT16( FT2_1_3_BYTE_U16( p, 0, 8 ) | \
                                        FT2_1_3_BYTE_U16( p, 1, 0 ) )

#define FT2_1_3_PEEK_LONG( p )  FT2_1_3_INT32( FT2_1_3_INT8_I32( p, 0, 24 ) | \
                                     FT2_1_3_BYTE_I32( p, 1, 16 ) | \
                                     FT2_1_3_BYTE_I32( p, 2,  8 ) | \
                                     FT2_1_3_BYTE_I32( p, 3,  0 ) )

#define FT2_1_3_PEEK_ULONG( p )  FT2_1_3_UINT32( FT2_1_3_BYTE_U32( p, 0, 24 ) | \
                                       FT2_1_3_BYTE_U32( p, 1, 16 ) | \
                                       FT2_1_3_BYTE_U32( p, 2,  8 ) | \
                                       FT2_1_3_BYTE_U32( p, 3,  0 ) )

#define FT2_1_3_PEEK_OFF3( p )  FT2_1_3_INT32( FT2_1_3_INT8_I32( p, 0, 16 ) | \
                                     FT2_1_3_BYTE_I32( p, 1,  8 ) | \
                                     FT2_1_3_BYTE_I32( p, 2,  0 ) )

#define FT2_1_3_PEEK_UOFF3( p )  FT2_1_3_UINT32( FT2_1_3_BYTE_U32( p, 0, 16 ) | \
                                       FT2_1_3_BYTE_U32( p, 1,  8 ) | \
                                       FT2_1_3_BYTE_U32( p, 2,  0 ) )

#define FT2_1_3_PEEK_SHORT_LE( p )  FT2_1_3_INT16( FT2_1_3_INT8_I16( p, 1, 8 ) | \
                                         FT2_1_3_BYTE_I16( p, 0, 0 ) )

#define FT2_1_3_PEEK_USHORT_LE( p )  FT2_1_3_UINT16( FT2_1_3_BYTE_U16( p, 1, 8 ) |  \
                                           FT2_1_3_BYTE_U16( p, 0, 0 ) )

#define FT2_1_3_PEEK_LONG_LE( p )  FT2_1_3_INT32( FT2_1_3_INT8_I32( p, 3, 24 ) | \
                                        FT2_1_3_BYTE_I32( p, 2, 16 ) | \
                                        FT2_1_3_BYTE_I32( p, 1,  8 ) | \
                                        FT2_1_3_BYTE_I32( p, 0,  0 ) )

#define FT2_1_3_PEEK_ULONG_LE( p )  FT2_1_3_UINT32( FT2_1_3_BYTE_U32( p, 3, 24 ) | \
                                          FT2_1_3_BYTE_U32( p, 2, 16 ) | \
                                          FT2_1_3_BYTE_U32( p, 1,  8 ) | \
                                          FT2_1_3_BYTE_U32( p, 0,  0 ) )

#define FT2_1_3_PEEK_OFF3_LE( p )  FT2_1_3_INT32( FT2_1_3_INT8_I32( p, 2, 16 ) | \
                                        FT2_1_3_BYTE_I32( p, 1,  8 ) | \
                                        FT2_1_3_BYTE_I32( p, 0,  0 ) )

#define FT2_1_3_PEEK_UOFF3_LE( p )  FT2_1_3_UINT32( FT2_1_3_BYTE_U32( p, 2, 16 ) | \
                                          FT2_1_3_BYTE_U32( p, 1,  8 ) | \
                                          FT2_1_3_BYTE_U32( p, 0,  0 ) )


#define FT2_1_3_NEXT_CHAR( buffer )       \
          ( (signed char)*buffer++ )

#define FT2_1_3_NEXT_BYTE( buffer )         \
          ( (unsigned char)*buffer++ )

#define FT2_1_3_NEXT_SHORT( buffer )                                   \
          ( (short)( buffer += 2, FT2_1_3_PEEK_SHORT( buffer - 2 ) ) )

#define FT2_1_3_NEXT_USHORT( buffer )                                            \
          ( (unsigned short)( buffer += 2, FT2_1_3_PEEK_USHORT( buffer - 2 ) ) )

#define FT2_1_3_NEXT_OFF3( buffer )                                  \
          ( (long)( buffer += 3, FT2_1_3_PEEK_OFF3( buffer - 3 ) ) )

#define FT2_1_3_NEXT_UOFF3( buffer )                                           \
          ( (unsigned long)( buffer += 3, FT2_1_3_PEEK_UOFF3( buffer - 3 ) ) )

#define FT2_1_3_NEXT_LONG( buffer )                                  \
          ( (long)( buffer += 4, FT2_1_3_PEEK_LONG( buffer - 4 ) ) )

#define FT2_1_3_NEXT_ULONG( buffer )                                           \
          ( (unsigned long)( buffer += 4, FT2_1_3_PEEK_ULONG( buffer - 4 ) ) )


#define FT2_1_3_NEXT_SHORT_LE( buffer )                                   \
          ( (short)( buffer += 2, FT2_1_3_PEEK_SHORT_LE( buffer - 2 ) ) )

#define FT2_1_3_NEXT_USHORT_LE( buffer )                                            \
          ( (unsigned short)( buffer += 2, FT2_1_3_PEEK_USHORT_LE( buffer - 2 ) ) )

#define FT2_1_3_NEXT_OFF3_LE( buffer )                                  \
          ( (long)( buffer += 3, FT2_1_3_PEEK_OFF3_LE( buffer - 3 ) ) )

#define FT2_1_3_NEXT_UOFF3_LE( buffer )                                           \
          ( (unsigned long)( buffer += 3, FT2_1_3_PEEK_UOFF3_LE( buffer - 3 ) ) )

#define FT2_1_3_NEXT_LONG_LE( buffer )                                  \
          ( (long)( buffer += 4, FT2_1_3_PEEK_LONG_LE( buffer - 4 ) ) )

#define FT2_1_3_NEXT_ULONG_LE( buffer )                                           \
          ( (unsigned long)( buffer += 4, FT2_1_3_PEEK_ULONG_LE( buffer - 4 ) ) )


/*************************************************************************/
/*                                                                       */
/* Each GET_xxxx() macro uses an implicit `stream' variable.             */
/*                                                                       */
#define FT2_1_3_GET_MACRO( func, type )        ( (type)func( stream ) )

#define FT2_1_3_GET_CHAR()       FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetChar, FT2_1_3_Char )
#define FT2_1_3_GET_BYTE()       FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetChar, FT2_1_3_Byte )
#define FT2_1_3_GET_SHORT()      FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetShort, FT2_1_3_Short )
#define FT2_1_3_GET_USHORT()     FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetShort, FT2_1_3_UShort )
#define FT2_1_3_GET_OFF3()       FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetOffset, FT2_1_3_Long )
#define FT2_1_3_GET_UOFF3()      FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetOffset, FT2_1_3_ULong )
#define FT2_1_3_GET_LONG()       FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetLong, FT2_1_3_Long )
#define FT2_1_3_GET_ULONG()      FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetLong, FT2_1_3_ULong )
#define FT2_1_3_GET_TAG4()       FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetLong, FT2_1_3_ULong )

#define FT2_1_3_GET_SHORT_LE()   FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetShortLE, FT2_1_3_Short )
#define FT2_1_3_GET_USHORT_LE()  FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetShortLE, FT2_1_3_UShort )
#define FT2_1_3_GET_LONG_LE()    FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetLongLE, FT2_1_3_Long )
#define FT2_1_3_GET_ULONG_LE()   FT2_1_3_GET_MACRO( FT2_1_3_Stream_GetLongLE, FT2_1_3_ULong )

#define FT2_1_3_READ_MACRO( func, type, var )        \
          ( var = (type)func( stream, &error ), \
            error != FT2_1_3_Err_Ok )

#define FT2_1_3_READ_BYTE( var )       FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadChar, FT2_1_3_Byte, var )
#define FT2_1_3_READ_CHAR( var )       FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadChar, FT2_1_3_Char, var )
#define FT2_1_3_READ_SHORT( var )      FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadShort, FT2_1_3_Short, var )
#define FT2_1_3_READ_USHORT( var )     FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadShort, FT2_1_3_UShort, var )
#define FT2_1_3_READ_OFF3( var )       FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadOffset, FT2_1_3_Long, var )
#define FT2_1_3_READ_UOFF3( var )      FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadOffset, FT2_1_3_ULong, var )
#define FT2_1_3_READ_LONG( var )       FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadLong, FT2_1_3_Long, var )
#define FT2_1_3_READ_ULONG( var )      FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadLong, FT2_1_3_ULong, var )

#define FT2_1_3_READ_SHORT_LE( var )   FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadShortLE, FT2_1_3_Short, var )
#define FT2_1_3_READ_USHORT_LE( var )  FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadShortLE, FT2_1_3_UShort, var )
#define FT2_1_3_READ_LONG_LE( var )    FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadLongLE, FT2_1_3_Long, var )
#define FT2_1_3_READ_ULONG_LE( var )   FT2_1_3_READ_MACRO( FT2_1_3_Stream_ReadLongLE, FT2_1_3_ULong, var )


#ifndef FT2_1_3_CONFIG_OPTION_NO_DEFAULT_SYSTEM

/* initialize a stream for reading a regular system stream */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Stream_Open( FT2_1_3_Stream    stream,
                const char*  filepathname );

#endif /* FT2_1_3_CONFIG_OPTION_NO_DEFAULT_SYSTEM */


/* initialize a stream for reading in-memory data */
FT2_1_3_BASE( void )
FT2_1_3_Stream_OpenMemory( FT2_1_3_Stream       stream,
                      const FT2_1_3_Byte*  base,
                      FT2_1_3_ULong        size );

/* close a stream (does not destroy the stream structure) */
FT2_1_3_BASE( void )
FT2_1_3_Stream_Close( FT2_1_3_Stream  stream );


/* seek within a stream. position is relative to start of stream */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Stream_Seek( FT2_1_3_Stream  stream,
                FT2_1_3_ULong   pos );

/* skip bytes in a stream */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Stream_Skip( FT2_1_3_Stream  stream,
                FT2_1_3_Long    distance );

/* return current stream position */
FT2_1_3_BASE( FT2_1_3_Long )
FT2_1_3_Stream_Pos( FT2_1_3_Stream  stream );

/* read bytes from a stream into a user-allocated buffer, returns an */
/* error if not all bytes could be read.                             */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Stream_Read( FT2_1_3_Stream  stream,
                FT2_1_3_Byte*   buffer,
                FT2_1_3_ULong   count );

/* read bytes from a stream at a given position */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Stream_ReadAt( FT2_1_3_Stream  stream,
                  FT2_1_3_ULong   pos,
                  FT2_1_3_Byte*   buffer,
                  FT2_1_3_ULong   count );

/* Enter a frame of `count' consecutive bytes in a stream.  Returns an */
/* error if the frame could not be read/accessed.  The caller can use  */
/* the FT2_1_3_Stream_Get_XXX functions to retrieve frame data without      */
/* error checks.                                                       */
/*                                                                     */
/* You must _always_ call FT2_1_3_Stream_ExitFrame() once you have entered  */
/* a stream frame!                                                     */
/*                                                                     */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Stream_EnterFrame( FT2_1_3_Stream  stream,
                      FT2_1_3_ULong   count );

/* exit a stream frame */
FT2_1_3_BASE( void )
FT2_1_3_Stream_ExitFrame( FT2_1_3_Stream  stream );

/* Extract a stream frame.  If the stream is disk-based, a heap block */
/* is allocated and the frame bytes are read into it.  If the stream  */
/* is memory-based, this function simply set a pointer to the data.   */
/*                                                                    */
/* Useful to optimize access to memory-based streams transparently.   */
/*                                                                    */
/* All extracted frames must be `freed` with a call to the function   */
/* FT2_1_3_Stream_ReleaseFrame().                                          */
/*                                                                    */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Stream_ExtractFrame( FT2_1_3_Stream  stream,
                        FT2_1_3_ULong   count,
                        FT2_1_3_Byte**  pbytes );

/* release an extract frame (see FT2_1_3_Stream_ExtractFrame) */
FT2_1_3_BASE( void )
FT2_1_3_Stream_ReleaseFrame( FT2_1_3_Stream  stream,
                        FT2_1_3_Byte**  pbytes );

/* read a byte from an entered frame */
FT2_1_3_BASE( FT2_1_3_Char )
FT2_1_3_Stream_GetChar( FT2_1_3_Stream  stream );

/* read a 16-bit big-endian integer from an entered frame */
FT2_1_3_BASE( FT2_1_3_Short )
FT2_1_3_Stream_GetShort( FT2_1_3_Stream  stream );

/* read a 24-bit big-endian integer from an entered frame */
FT2_1_3_BASE( FT2_1_3_Long )
FT2_1_3_Stream_GetOffset( FT2_1_3_Stream  stream );

/* read a 32-bit big-endian integer from an entered frame */
FT2_1_3_BASE( FT2_1_3_Long )
FT2_1_3_Stream_GetLong( FT2_1_3_Stream  stream );

/* read a 16-bit little-endian integer from an entered frame */
FT2_1_3_BASE( FT2_1_3_Short )
FT2_1_3_Stream_GetShortLE( FT2_1_3_Stream  stream );

/* read a 32-bit little-endian integer from an entered frame */
FT2_1_3_BASE( FT2_1_3_Long )
FT2_1_3_Stream_GetLongLE( FT2_1_3_Stream  stream );


/* read a byte from a stream */
FT2_1_3_BASE( FT2_1_3_Char )
FT2_1_3_Stream_ReadChar( FT2_1_3_Stream  stream,
                    FT2_1_3_Error*  error );

/* read a 16-bit big-endian integer from a stream */
FT2_1_3_BASE( FT2_1_3_Short )
FT2_1_3_Stream_ReadShort( FT2_1_3_Stream  stream,
                     FT2_1_3_Error*  error );

/* read a 24-bit big-endian integer from a stream */
FT2_1_3_BASE( FT2_1_3_Long )
FT2_1_3_Stream_ReadOffset( FT2_1_3_Stream  stream,
                      FT2_1_3_Error*  error );

/* read a 32-bit big-endian integer from a stream */
FT2_1_3_BASE( FT2_1_3_Long )
FT2_1_3_Stream_ReadLong( FT2_1_3_Stream  stream,
                    FT2_1_3_Error*  error );

/* read a 16-bit little-endian integer from a stream */
FT2_1_3_BASE( FT2_1_3_Short )
FT2_1_3_Stream_ReadShortLE( FT2_1_3_Stream  stream,
                       FT2_1_3_Error*  error );

/* read a 32-bit little-endian integer from a stream */
FT2_1_3_BASE( FT2_1_3_Long )
FT2_1_3_Stream_ReadLongLE( FT2_1_3_Stream  stream,
                      FT2_1_3_Error*  error );

/* Read a structure from a stream.  The structure must be described */
/* by an array of FT2_1_3_Frame_Field records.                           */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Stream_ReadFields( FT2_1_3_Stream              stream,
                      const FT2_1_3_Frame_Field*  fields,
                      void*                  structure );


#define FT2_1_3_STREAM_POS()           \
          FT2_1_3_Stream_Pos( stream )

#define FT2_1_3_STREAM_SEEK( position )                           \
          FT2_1_3_SET_ERROR( FT2_1_3_Stream_Seek( stream, position ) )

#define FT2_1_3_STREAM_SKIP( distance )                           \
          FT2_1_3_SET_ERROR( FT2_1_3_Stream_Skip( stream, distance ) )

#define FT2_1_3_STREAM_READ( buffer, count )                   \
          FT2_1_3_SET_ERROR( FT2_1_3_Stream_Read( stream,           \
                                        (FT2_1_3_Byte*)buffer, \
                                        count ) )

#define FT2_1_3_STREAM_READ_AT( position, buffer, count )         \
          FT2_1_3_SET_ERROR( FT2_1_3_Stream_ReadAt( stream,            \
                                           position,         \
                                           (FT2_1_3_Byte*)buffer, \
                                           count ) )

#define FT2_1_3_STREAM_READ_FIELDS( fields, object )                          \
          FT2_1_3_SET_ERROR( FT2_1_3_Stream_ReadFields( stream, fields, object ) )


#define FT2_1_3_FRAME_ENTER( size )                                 \
          FT2_1_3_SET_ERROR( FT2_1_3_Stream_EnterFrame( stream, size ) )

#define FT2_1_3_FRAME_EXIT()                 \
          FT2_1_3_Stream_ExitFrame( stream )

#define FT2_1_3_FRAME_EXTRACT( size, bytes )                                 \
          FT2_1_3_SET_ERROR( FT2_1_3_Stream_ExtractFrame( stream, size,           \
                                                (FT2_1_3_Byte**)&(bytes) ) )

#define FT2_1_3_FRAME_RELEASE( bytes )                               \
          FT2_1_3_Stream_ReleaseFrame( stream, (FT2_1_3_Byte**)&(bytes) )


FT2_1_3_END_HEADER

#endif /* __FTSTREAM_H__ */


/* END */
