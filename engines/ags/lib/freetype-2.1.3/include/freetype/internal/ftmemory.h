/***************************************************************************/
/*                                                                         */
/*  ftmemory.h                                                             */
/*                                                                         */
/*    The FreeType memory management macros (specification).               */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg                       */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTMEMORY_H__
#define __FTMEMORY_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_CONFIG_CONFIG_H
#include FT2_1_3_TYPES_H


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Macro>                                                               */
/*    FT2_1_3_SET_ERROR                                                       */
/*                                                                       */
/* <Description>                                                         */
/*    This macro is used to set an implicit `error' variable to a given  */
/*    expression's value (usually a function call), and convert it to a  */
/*    boolean which is set whenever the value is != 0.                   */
/*                                                                       */
#undef  FT2_1_3_SET_ERROR
#define FT2_1_3_SET_ERROR( expression ) \
          ( ( error = (expression) ) != 0 )


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/****                                                                 ****/
/****                                                                 ****/
/****                           M E M O R Y                           ****/
/****                                                                 ****/
/****                                                                 ****/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

#ifdef FT2_1_3_DEBUG_MEMORY

FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Alloc_Debug( FT2_1_3_Memory    memory,
                FT2_1_3_Long      size,
                void*       *P,
                const char*  file_name,
                FT2_1_3_Long      line_no );

FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Realloc_Debug( FT2_1_3_Memory    memory,
                  FT2_1_3_Long      current,
                  FT2_1_3_Long      size,
                  void*       *P,
                  const char*  file_name,
                  FT2_1_3_Long      line_no );

FT2_1_3_BASE( void )
FT2_1_3_Free_Debug( FT2_1_3_Memory    memory,
               FT2_1_3_Pointer   block,
               const char*  file_name,
               FT2_1_3_Long      line_no );

#endif


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Alloc                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    Allocates a new block of memory.  The returned area is always      */
/*    zero-filled; this is a strong convention in many FreeType parts.   */
/*                                                                       */
/* <Input>                                                               */
/*    memory :: A handle to a given `memory object' which handles        */
/*              allocation.                                              */
/*                                                                       */
/*    size   :: The size in bytes of the block to allocate.              */
/*                                                                       */
/* <Output>                                                              */
/*    P      :: A pointer to the fresh new block.  It should be set to   */
/*              NULL if `size' is 0, or in case of error.                */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Alloc( FT2_1_3_Memory  memory,
          FT2_1_3_Long    size,
          void*     *P );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Realloc                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    Reallocates a block of memory pointed to by `*P' to `Size' bytes   */
/*    from the heap, possibly changing `*P'.                             */
/*                                                                       */
/* <Input>                                                               */
/*    memory  :: A handle to a given `memory object' which handles       */
/*               reallocation.                                           */
/*                                                                       */
/*    current :: The current block size in bytes.                        */
/*                                                                       */
/*    size    :: The new block size in bytes.                            */
/*                                                                       */
/* <InOut>                                                               */
/*    P       :: A pointer to the fresh new block.  It should be set to  */
/*               NULL if `size' is 0, or in case of error.               */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    All callers of FT2_1_3_Realloc() _must_ provide the current block size  */
/*    as well as the new one.                                            */
/*                                                                       */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_Realloc( FT2_1_3_Memory  memory,
            FT2_1_3_Long    current,
            FT2_1_3_Long    size,
            void**     P );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Free                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    Releases a given block of memory allocated through FT2_1_3_Alloc().     */
/*                                                                       */
/* <Input>                                                               */
/*    memory :: A handle to a given `memory object' which handles        */
/*              memory deallocation                                      */
/*                                                                       */
/*    P      :: This is the _address_ of a _pointer_ which points to the */
/*              allocated block.  It is always set to NULL on exit.      */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    If P or *P are NULL, this function should return successfully.     */
/*    This is a strong convention within all of FreeType and its         */
/*    drivers.                                                           */
/*                                                                       */
FT2_1_3_BASE( void )
FT2_1_3_Free( FT2_1_3_Memory  memory,
         void**     P );


#define FT2_1_3_MEM_SET( dest, byte, count )     ft_memset( dest, byte, count )

#define FT2_1_3_MEM_COPY( dest, source, count )  ft_memcpy( dest, source, count )

#define FT2_1_3_MEM_MOVE( dest, source, count )  ft_memmove( dest, source, count )


#define FT2_1_3_MEM_ZERO( dest, count )  FT2_1_3_MEM_SET( dest, 0, count )

#define FT2_1_3_ZERO( p )                FT2_1_3_MEM_ZERO( p, sizeof ( *(p) ) )


/*************************************************************************/
/*                                                                       */
/* We first define FT2_1_3_MEM_ALLOC, FT2_1_3_MEM_REALLOC, and FT2_1_3_MEM_FREE.  All   */
/* macros use an _implicit_ `memory' parameter to access the current     */
/* memory allocator.                                                     */
/*                                                                       */

#ifdef FT2_1_3_DEBUG_MEMORY

#define FT2_1_3_MEM_ALLOC( _pointer_, _size_ )                            \
          FT2_1_3_Alloc_Debug( memory, _size_,                            \
                          (void**)&(_pointer_), __FILE__, __LINE__ )

#define FT2_1_3_MEM_REALLOC( _pointer_, _current_, _size_ )                 \
          FT2_1_3_Realloc_Debug( memory, _current_, _size_,                 \
                            (void**)&(_pointer_), __FILE__, __LINE__ )

#define FT2_1_3_MEM_FREE( _pointer_ )                                            \
          FT2_1_3_Free_Debug( memory, (void**)&(_pointer_), __FILE__, __LINE__ )


#else  /* !FT2_1_3_DEBUG_MEMORY */


#define FT2_1_3_MEM_ALLOC( _pointer_, _size_ )                  \
          FT2_1_3_Alloc( memory, _size_, (void**)&(_pointer_) )

#define FT2_1_3_MEM_FREE( _pointer_ )                  \
          FT2_1_3_Free( memory, (void**)&(_pointer_) )

#define FT2_1_3_MEM_REALLOC( _pointer_, _current_, _size_ )                  \
          FT2_1_3_Realloc( memory, _current_, _size_, (void**)&(_pointer_) )


#endif /* !FT2_1_3_DEBUG_MEMORY */


/*************************************************************************/
/*                                                                       */
/* The following functions macros expect that their pointer argument is  */
/* _typed_ in order to automatically compute array element sizes.        */
/*                                                                       */

#define FT2_1_3_MEM_NEW( _pointer_ )                               \
          FT2_1_3_MEM_ALLOC( _pointer_, sizeof ( *(_pointer_) ) )

#define FT2_1_3_MEM_NEW_ARRAY( _pointer_, _count_ )                           \
          FT2_1_3_MEM_ALLOC( _pointer_, (_count_) * sizeof ( *(_pointer_) ) )

#define FT2_1_3_MEM_RENEW_ARRAY( _pointer_, _old_, _new_ )                    \
          FT2_1_3_MEM_REALLOC( _pointer_, (_old_) * sizeof ( *(_pointer_) ),  \
                                     (_new_) * sizeof ( *(_pointer_) ) )


/*************************************************************************/
/*                                                                       */
/* the following macros are obsolete but kept for compatibility reasons  */
/*                                                                       */

#define FT2_1_3_MEM_ALLOC_ARRAY( _pointer_, _count_, _type_ )           \
          FT2_1_3_MEM_ALLOC( _pointer_, (_count_) * sizeof ( _type_ ) )

#define FT2_1_3_MEM_REALLOC_ARRAY( _pointer_, _old_, _new_, _type_ )    \
          FT2_1_3_MEM_REALLOC( _pointer_, (_old_) * sizeof ( _type ),   \
                                     (_new_) * sizeof ( _type_ ) )


/*************************************************************************/
/*                                                                       */
/* The following macros are variants of their FT2_1_3_MEM_XXXX equivalents;   */
/* they are used to set an _implicit_ `error' variable and return TRUE   */
/* if an error occured (i.e. if 'error != 0').                           */
/*                                                                       */

#define FT2_1_3_ALLOC( _pointer_, _size_ )                       \
          FT2_1_3_SET_ERROR( FT2_1_3_MEM_ALLOC( _pointer_, _size_ ) )

#define FT2_1_3_REALLOC( _pointer_, _current_, _size_ )                       \
          FT2_1_3_SET_ERROR( FT2_1_3_MEM_REALLOC( _pointer_, _current_, _size_ ) )

#define FT2_1_3_FREE( _pointer_ )       \
          FT2_1_3_MEM_FREE( _pointer_ )

#define FT2_1_3_NEW( _pointer_ )  \
          FT2_1_3_SET_ERROR( FT2_1_3_MEM_NEW( _pointer_ ) )

#define FT2_1_3_NEW_ARRAY( _pointer_, _count_ )  \
          FT2_1_3_SET_ERROR( FT2_1_3_MEM_NEW_ARRAY( _pointer_, _count_ ) )

#define FT2_1_3_RENEW_ARRAY( _pointer_, _old_, _new_ )   \
          FT2_1_3_SET_ERROR( FT2_1_3_MEM_RENEW_ARRAY( _pointer_, _old_, _new_ ) )

#define FT2_1_3_ALLOC_ARRAY( _pointer_, _count_, _type_ )                    \
          FT2_1_3_SET_ERROR( FT2_1_3_MEM_ALLOC( _pointer_,                        \
                                      (_count_) * sizeof ( _type_ ) ) )

#define FT2_1_3_REALLOC_ARRAY( _pointer_, _old_, _new_, _type_ )             \
          FT2_1_3_SET_ERROR( FT2_1_3_MEM_REALLOC( _pointer_,                      \
                                        (_old_) * sizeof ( _type_ ),    \
                                        (_new_) * sizeof ( _type_ ) ) )

/* */


FT2_1_3_END_HEADER

#endif /* __FTMEMORY_H__ */


/* END */
