/***************************************************************************/
/*                                                                         */
/*  ftsystem.h                                                             */
/*                                                                         */
/*    FreeType low-level system interface definition (specification).      */
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


#ifndef __FTSYSTEM_H__
#define __FTSYSTEM_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    system_interface                                                   */
/*                                                                       */
/* <Title>                                                               */
/*    System Interface                                                   */
/*                                                                       */
/* <Abstract>                                                            */
/*    How FreeType manages memory and i/o.                               */
/*                                                                       */
/* <Description>                                                         */
/*    This section contains various definitions related to memory        */
/*    management and i/o access.  You need to understand this            */
/*    information if you want to use a custom memory manager or you own  */
/*    input i/o streams.                                                 */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/*                  M E M O R Y   M A N A G E M E N T                    */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* @type:                                                                */
/*    FT2_1_3_Memory                                                          */
/*                                                                       */
/* @description:                                                         */
/*    A handle to a given memory manager object, defined with a          */
/*    @FT2_1_3_MemoryRec structure.                                           */
/*                                                                       */
typedef struct FT2_1_3_MemoryRec_*  FT2_1_3_Memory;


/*************************************************************************/
/*                                                                       */
/* @functype:                                                            */
/*    FT2_1_3_Alloc_Func                                                      */
/*                                                                       */
/* @description:                                                         */
/*    A function used to allocate `size' bytes from `memory'.            */
/*                                                                       */
/* @input:                                                               */
/*    memory :: A handle to the source memory manager.                   */
/*                                                                       */
/*    size   :: The size in bytes to allocate.                           */
/*                                                                       */
/* @return:                                                              */
/*    Address of new memory block.  0 in case of failure.                */
/*                                                                       */
typedef void*
(*FT2_1_3_Alloc_Func)( FT2_1_3_Memory  memory,
                  long       size );


/*************************************************************************/
/*                                                                       */
/* @functype:                                                            */
/*    FT2_1_3_Free_Func                                                       */
/*                                                                       */
/* @description:                                                         */
/*    A function used to release a given block of memory.                */
/*                                                                       */
/* @input:                                                               */
/*    memory :: A handle to the source memory manager.                   */
/*                                                                       */
/*    block  :: The address of the target memory block.                  */
/*                                                                       */
typedef void
(*FT2_1_3_Free_Func)( FT2_1_3_Memory  memory,
                 void*      block );


/*************************************************************************/
/*                                                                       */
/* @functype:                                                            */
/*    FT2_1_3_Realloc_Func                                                    */
/*                                                                       */
/* @description:                                                         */
/*    a function used to re-allocate a given block of memory.            */
/*                                                                       */
/* @input:                                                               */
/*    memory   :: A handle to the source memory manager.                 */
/*                                                                       */
/*    cur_size :: The block's current size in bytes.                     */
/*                                                                       */
/*    new_size :: The block's requested new size.                        */
/*                                                                       */
/*    block    :: The block's current address.                           */
/*                                                                       */
/* @return:                                                              */
/*    New block address.  0 in case of memory shortage.                  */
/*                                                                       */
/* @note:                                                                */
/*   In case of error, the old block must still be available.            */
/*                                                                       */
typedef void*
(*FT2_1_3_Realloc_Func)( FT2_1_3_Memory  memory,
                    long       cur_size,
                    long       new_size,
                    void*      block );


/*************************************************************************/
/*                                                                       */
/* @struct:                                                              */
/*    FT2_1_3_MemoryRec                                                       */
/*                                                                       */
/* @description:                                                         */
/*    A structure used to describe a given memory manager to FreeType 2. */
/*                                                                       */
/* @fields:                                                              */
/*    user    :: A generic typeless pointer for user data.               */
/*                                                                       */
/*    alloc   :: A pointer type to an allocation function.               */
/*                                                                       */
/*    free    :: A pointer type to an memory freeing function.           */
/*                                                                       */
/*    realloc :: A pointer type to a reallocation function.              */
/*                                                                       */
struct  FT2_1_3_MemoryRec_ {
	void*            user;
	FT2_1_3_Alloc_Func    alloc;
	FT2_1_3_Free_Func     free;
	FT2_1_3_Realloc_Func  realloc;
};


/*************************************************************************/
/*                                                                       */
/*                       I / O   M A N A G E M E N T                     */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* @type:                                                                */
/*    FT2_1_3_Stream                                                          */
/*                                                                       */
/* @description:                                                         */
/*    A handle to an input stream.                                       */
/*                                                                       */
typedef struct FT2_1_3_StreamRec_*  FT2_1_3_Stream;


/*************************************************************************/
/*                                                                       */
/* @struct:                                                              */
/*    FT2_1_3_StreamDesc                                                      */
/*                                                                       */
/* @description:                                                         */
/*    A union type used to store either a long or a pointer.  This is    */
/*    used to store a file descriptor or a FILE* in an input stream.     */
/*                                                                       */
typedef union  FT2_1_3_StreamDesc_ {
	long   value;
	void*  pointer;

} FT2_1_3_StreamDesc;


/*************************************************************************/
/*                                                                       */
/* @functype:                                                            */
/*    FT2_1_3_Stream_IoFunc                                                   */
/*                                                                       */
/* @description:                                                         */
/*    A function used to seek and read data from a given input stream.   */
/*                                                                       */
/* @input:                                                               */
/*    stream :: A handle to the source stream.                           */
/*                                                                       */
/*    offset :: The offset of read in stream (always from start).        */
/*                                                                       */
/*    buffer :: The address of the read buffer.                          */
/*                                                                       */
/*    count  :: The number of bytes to read from the stream.             */
/*                                                                       */
/* @return:                                                              */
/*    The number of bytes effectively read by the stream.                */
/*                                                                       */
/* @note:                                                                */
/*    This function might be called to perform a seek or skip operation  */
/*    with a `count' of 0.                                               */
/*                                                                       */
typedef unsigned long
(*FT2_1_3_Stream_IoFunc)( FT2_1_3_Stream       stream,
                     unsigned long   offset,
                     unsigned char*  buffer,
                     unsigned long   count );


/*************************************************************************/
/*                                                                       */
/* @functype:                                                            */
/*    FT2_1_3_Stream_CloseFunc                                                */
/*                                                                       */
/* @description:                                                         */
/*    A function used to close a given input stream.                     */
/*                                                                       */
/* @input:                                                               */
/*   stream :: A handle to the target stream.                            */
/*                                                                       */
typedef void
(*FT2_1_3_Stream_CloseFunc)( FT2_1_3_Stream  stream );


/*************************************************************************/
/*                                                                       */
/* @struct:                                                              */
/*    FT2_1_3_StreamRec                                                       */
/*                                                                       */
/* @description:                                                         */
/*   A structure used to describe an input stream.                       */
/*                                                                       */
/* @input:                                                               */
/*   base       :: For memory-based streams, this is the address of the  */
/*                 first stream byte in memory.  This field should       */
/*                 always be set to NULL for disk-based streams.         */
/*                                                                       */
/*   size       :: The stream size in bytes.                             */
/*                                                                       */
/*   pos        :: The current position within the stream.               */
/*                                                                       */
/*   descriptor :: This field is a union that can hold an integer or a   */
/*                 pointer.  It is used by stream implementations to     */
/*                 store file descriptors or FILE* pointers.             */
/*                                                                       */
/*   pathname   :: This field is completely ignored by FreeType.         */
/*                 However, it is often useful during debugging to use   */
/*                 it to store the stream's filename (where available).  */
/*                                                                       */
/*   read       :: The stream's input function.                          */
/*                                                                       */
/*   close      :: The stream;s close function.                          */
/*                                                                       */
/*   memory     :: The memory manager to use to preload frames.  This is */
/*                 set internally by FreeType and shouldn't be touched   */
/*                 by stream implementations.                            */
/*                                                                       */
/*   cursor     :: This field is set and used internally by FreeType     */
/*                 when parsing frames.                                  */
/*                                                                       */
/*   limit      :: This field is set and used internally by FreeType     */
/*                 when parsing frames.                                  */
/*                                                                       */
typedef struct  FT2_1_3_StreamRec_ {
	unsigned char*       base;
	unsigned long        size;
	unsigned long        pos;

	FT2_1_3_StreamDesc        descriptor;
	FT2_1_3_StreamDesc        pathname;
	FT2_1_3_Stream_IoFunc     read;
	FT2_1_3_Stream_CloseFunc  close;

	FT2_1_3_Memory            memory;
	unsigned char*       cursor;
	unsigned char*       limit;

} FT2_1_3_StreamRec;


/* */


FT2_1_3_END_HEADER

#endif /* __FTSYSTEM_H__ */


/* END */
