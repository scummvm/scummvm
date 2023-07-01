/***************************************************************************/
/*                                                                         */
/*  ftsizes.h                                                              */
/*                                                                         */
/*    FreeType size objects management (specification).                    */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
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
/* Typical application would normally not need to use these functions.   */
/* However, they have been placed in a public API for the rare cases     */
/* where they are needed.                                                */
/*                                                                       */
/*************************************************************************/


#ifndef __FTSIZES_H__
#define __FTSIZES_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    sizes_management                                                   */
/*                                                                       */
/* <Title>                                                               */
/*    Size management                                                    */
/*                                                                       */
/* <Abstract>                                                            */
/*    Managing multiple sizes per face                                   */
/*                                                                       */
/* <Description>                                                         */
/*    When creating a new face object (e.g. with @FT2_1_3_New_Face), an       */
/*    @FT2_1_3_Size object is automatically created and used to store all     */
/*    pixel-size dependent information, available in the "face->size"    */
/*    field.                                                             */
/*                                                                       */
/*    It is however possible to create more sizes for a given face,      */
/*    mostly in order to manage several character pixel sizes of the     */
/*    same font family and style.  See @FT2_1_3_New_Size and @FT2_1_3_Done_Size.   */
/*                                                                       */
/*    Note that @FT2_1_3_Set_Pixel_Sizes and @FT2_1_3_Set_Char_Size only           */
/*    modify the contents of the current "active" size; you thus need    */
/*    to use @FT2_1_3_Activate_Size to change it.                             */
/*                                                                       */
/*    99% of applications won't need the functions provided here,        */
/*    especially if they use the caching sub-system, so be cautious      */
/*    when using these.                                                  */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_New_Size                                                        */
/*                                                                       */
/* <Description>                                                         */
/*    Creates a new size object from a given face object.                */
/*                                                                       */
/* <Input>                                                               */
/*    face :: A handle to a parent face object.                          */
/*                                                                       */
/* <Output>                                                              */
/*    asize :: A handle to a new size object.                            */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    You need to call @FT2_1_3_Activate_Size in order to select the new size */
/*    for upcoming calls to @FT2_1_3_Set_Pixel_Sizes, @FT2_1_3_Set_Char_Size,      */
/*    @FT2_1_3_Load_Glyph, @FT2_1_3_Load_Char, etc.                                */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_New_Size( FT2_1_3_Face   face,
             FT2_1_3_Size*  size );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Done_Size                                                       */
/*                                                                       */
/* <Description>                                                         */
/*    Discards a given size object.                                      */
/*                                                                       */
/* <Input>                                                               */
/*    size :: A handle to a target size object.                          */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Done_Size( FT2_1_3_Size  size );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Activate_Size                                                   */
/*                                                                       */
/* <Description>                                                         */
/*    Even though it is possible to create several size objects for a    */
/*    given face (see @FT2_1_3_New_Size for details), functions like          */
/*    @FT2_1_3_Load_Glyph or @FT2_1_3_Load_Char only use the last-created one to   */
/*    determine the "current character pixel size".                      */
/*                                                                       */
/*    This function can be used to "activate" a previously created size  */
/*    object.                                                            */
/*                                                                       */
/* <Input>                                                               */
/*    size :: A handle to a target size object.                          */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    If "face" is the size's parent face object, this function changes  */
/*    the value of "face->size" to the input size handle.                */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Activate_Size( FT2_1_3_Size  size );

/* */


FT2_1_3_END_HEADER

#endif /* __FTSIZES_H__ */


/* END */
