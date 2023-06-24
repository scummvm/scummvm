/***************************************************************************/
/*                                                                         */
/*  fttypes.h                                                              */
/*                                                                         */
/*    FreeType simple types definitions (specification only).              */
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


#ifndef __FTTYPES_H__
#define __FTTYPES_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_CONFIG_CONFIG_H
#include FT2_1_3_SYSTEM_H
#include FT2_1_3_IMAGE_H

#include <stddef.h>


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    basic_types                                                        */
/*                                                                       */
/* <Title>                                                               */
/*    Basic Data Types                                                   */
/*                                                                       */
/* <Abstract>                                                            */
/*    The basic data types defined by the library.                       */
/*                                                                       */
/* <Description>                                                         */
/*    This section contains the basic data types defined by FreeType 2,  */
/*    ranging from simple scalar types to bitmap descriptors.  More      */
/*    font-specific structures are defined in a different section.       */
/*                                                                       */
/* <Order>                                                               */
/*    FT2_1_3_Byte                                                            */
/*    FT2_1_3_Char                                                            */
/*    FT2_1_3_Int                                                             */
/*    FT2_1_3_UInt                                                            */
/*    FT2_1_3_Short                                                           */
/*    FT2_1_3_UShort                                                          */
/*    FT2_1_3_Long                                                            */
/*    FT2_1_3_ULong                                                           */
/*    FT2_1_3_Bool                                                            */
/*    FT2_1_3_Offset                                                          */
/*    FT2_1_3_PtrDist                                                         */
/*    FT2_1_3_String                                                          */
/*    FT2_1_3_Error                                                           */
/*    FT2_1_3_Fixed                                                           */
/*    FT2_1_3_Pointer                                                         */
/*    FT2_1_3_Pos                                                             */
/*    FT2_1_3_Vector                                                          */
/*    FT2_1_3_BBox                                                            */
/*    FT2_1_3_Matrix                                                          */
/*    FT2_1_3_FWord                                                           */
/*    FT2_1_3_UFWord                                                          */
/*    FT2_1_3_F2Dot14                                                         */
/*    FT2_1_3_UnitVector                                                      */
/*    FT2_1_3_F26Dot6                                                         */
/*                                                                       */
/*                                                                       */
/*    FT2_1_3_Generic                                                         */
/*    FT2_1_3_Generic_Finalizer                                               */
/*                                                                       */
/*    FT2_1_3_Bitmap                                                          */
/*    FT2_1_3_Pixel_Mode                                                      */
/*    FT2_1_3_Palette_Mode                                                    */
/*    FT2_1_3_Glyph_Format                                                    */
/*    FT2_1_3_IMAGE_TAG                                                       */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_Bool                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef of unsigned char, used for simple booleans.              */
/*                                                                       */
typedef unsigned char  FT2_1_3_Bool;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_FWord                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    A signed 16-bit integer used to store a distance in original font  */
/*    units.                                                             */
/*                                                                       */
typedef signed short  FT2_1_3_FWord;   /* distance in FUnits */


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_UFWord                                                          */
/*                                                                       */
/* <Description>                                                         */
/*    An unsigned 16-bit integer used to store a distance in original    */
/*    font units.                                                        */
/*                                                                       */
typedef unsigned short  FT2_1_3_UFWord;  /* unsigned distance */


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_Char                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A simple typedef for the _signed_ char type.                       */
/*                                                                       */
typedef signed char  FT2_1_3_Char;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_Byte                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A simple typedef for the _unsigned_ char type.                     */
/*                                                                       */
typedef unsigned char  FT2_1_3_Byte;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_String                                                          */
/*                                                                       */
/* <Description>                                                         */
/*    A simple typedef for the char type, usually used for strings.      */
/*                                                                       */
typedef char  FT2_1_3_String;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_Short                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for signed short.                                        */
/*                                                                       */
typedef signed short  FT2_1_3_Short;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_UShort                                                          */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for unsigned short.                                      */
/*                                                                       */
typedef unsigned short  FT2_1_3_UShort;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_Int                                                             */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for the int type.                                        */
/*                                                                       */
typedef int  FT2_1_3_Int;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_UInt                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for the unsigned int type.                               */
/*                                                                       */
typedef unsigned int  FT2_1_3_UInt;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_Long                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for signed long.                                         */
/*                                                                       */
typedef signed long  FT2_1_3_Long;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_ULong                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for unsigned long.                                       */
/*                                                                       */
typedef unsigned long  FT2_1_3_ULong;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_F2Dot14                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    A signed 2.14 fixed float type used for unit vectors.              */
/*                                                                       */
typedef signed short  FT2_1_3_F2Dot14;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_F26Dot6                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    A signed 26.6 fixed float type used for vectorial pixel            */
/*    coordinates.                                                       */
/*                                                                       */
typedef signed long  FT2_1_3_F26Dot6;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_Fixed                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    This type is used to store 16.16 fixed float values, like scales   */
/*    or matrix coefficients.                                            */
/*                                                                       */
typedef signed long  FT2_1_3_Fixed;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_Error                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    The FreeType error code type.  A value of 0 is always interpreted  */
/*    as a successful operation.                                         */
/*                                                                       */
typedef int  FT2_1_3_Error;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_Pointer                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    A simple typedef for a typeless pointer.                           */
/*                                                                       */
typedef void*  FT2_1_3_Pointer;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_Offset                                                          */
/*                                                                       */
/* <Description>                                                         */
/*    This is equivalent to the ANSI C `size_t' type, i.e. the largest   */
/*    _unsigned_ integer type used to express a file size or position,   */
/*    or a memory block size.                                            */
/*                                                                       */
typedef size_t  FT2_1_3_Offset;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_PtrDist                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    This is equivalent to the ANSI C `ptrdiff_t' type, i.e. the        */
/*    largest _signed_ integer type used to express the distance         */
/*    between two pointers.                                              */
/*                                                                       */
typedef size_t  FT2_1_3_PtrDist;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_UnitVector                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    A simple structure used to store a 2D vector unit vector.  Uses    */
/*    FT2_1_3_F2Dot14 types.                                                  */
/*                                                                       */
/* <Fields>                                                              */
/*    x :: Horizontal coordinate.                                        */
/*                                                                       */
/*    y :: Vertical coordinate.                                          */
/*                                                                       */
typedef struct  FT2_1_3_UnitVector_ {
	FT2_1_3_F2Dot14  x;
	FT2_1_3_F2Dot14  y;

} FT2_1_3_UnitVector;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_Matrix                                                          */
/*                                                                       */
/* <Description>                                                         */
/*    A simple structure used to store a 2x2 matrix.  Coefficients are   */
/*    in 16.16 fixed float format.  The computation performed is:        */
/*                                                                       */
/*       {                                                               */
/*          x' = x*xx + y*xy                                             */
/*          y' = x*yx + y*yy                                             */
/*       }                                                               */
/*                                                                       */
/* <Fields>                                                              */
/*    xx :: Matrix coefficient.                                          */
/*                                                                       */
/*    xy :: Matrix coefficient.                                          */
/*                                                                       */
/*    yx :: Matrix coefficient.                                          */
/*                                                                       */
/*    yy :: Matrix coefficient.                                          */
/*                                                                       */
typedef struct  FT2_1_3_Matrix_ {
	FT2_1_3_Fixed  xx, xy;
	FT2_1_3_Fixed  yx, yy;

} FT2_1_3_Matrix;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_Data	                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    Read-only binary data represented as a pointer and a length.       */
/*                                                                       */
/* <Fields>                                                              */
/*    pointer :: The data.                                               */
/*                                                                       */
/*    length  :: The length of the data in bytes.                        */
/*                                                                       */
typedef struct  FT2_1_3_Data_ {
	const FT2_1_3_Byte*  pointer;
	FT2_1_3_Int          length;

} FT2_1_3_Data;


/*************************************************************************/
/*                                                                       */
/* <FuncType>                                                            */
/*    FT2_1_3_Generic_Finalizer                                               */
/*                                                                       */
/* <Description>                                                         */
/*    Describes a function used to destroy the `client' data of any      */
/*    FreeType object.  See the description of the FT2_1_3_Generic type for   */
/*    details of usage.                                                  */
/*                                                                       */
/* <Input>                                                               */
/*    The address of the FreeType object which is under finalization.    */
/*    Its client data is accessed through its `generic' field.           */
/*                                                                       */
typedef void  (*FT2_1_3_Generic_Finalizer)(void*  object);


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_Generic                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    Client applications often need to associate their own data to a    */
/*    variety of FreeType core objects.  For example, a text layout API  */
/*    might want to associate a glyph cache to a given size object.      */
/*                                                                       */
/*    Most FreeType object contains a `generic' field, of type           */
/*    FT2_1_3_Generic, which usage is left to client applications and font    */
/*    servers.                                                           */
/*                                                                       */
/*    It can be used to store a pointer to client-specific data, as well */
/*    as the address of a `finalizer' function, which will be called by  */
/*    FreeType when the object is destroyed (for example, the previous   */
/*    client example would put the address of the glyph cache destructor */
/*    in the `finalizer' field).                                         */
/*                                                                       */
/* <Fields>                                                              */
/*    data      :: A typeless pointer to any client-specified data. This */
/*                 field is completely ignored by the FreeType library.  */
/*                                                                       */
/*    finalizer :: A pointer to a `generic finalizer' function, which    */
/*                 will be called when the object is destroyed.  If this */
/*                 field is set to NULL, no code will be called.         */
/*                                                                       */
typedef struct  FT2_1_3_Generic_ {
	void*                 data;
	FT2_1_3_Generic_Finalizer  finalizer;

} FT2_1_3_Generic;


/*************************************************************************/
/*                                                                       */
/* <Macro>                                                               */
/*    FT2_1_3_MAKE_TAG                                                        */
/*                                                                       */
/* <Description>                                                         */
/*    This macro converts four letter tags which are used to label       */
/*    TrueType tables into an unsigned long to be used within FreeType.  */
/*                                                                       */
/* <Note>                                                                */
/*    The produced values *must* be 32bit integers.  Don't redefine this */
/*    macro.                                                             */
/*                                                                       */
#define FT2_1_3_MAKE_TAG( _x1, _x2, _x3, _x4 ) \
          ( ( (FT2_1_3_ULong)_x1 << 24 ) |     \
            ( (FT2_1_3_ULong)_x2 << 16 ) |     \
            ( (FT2_1_3_ULong)_x3 <<  8 ) |     \
              (FT2_1_3_ULong)_x4         )


/*************************************************************************/
/*************************************************************************/
/*                                                                       */
/*                    L I S T   M A N A G E M E N T                      */
/*                                                                       */
/*************************************************************************/
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    list_processing                                                    */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_ListNode                                                        */
/*                                                                       */
/* <Description>                                                         */
/*     Many elements and objects in FreeType are listed through a        */
/*     FT2_1_3_List record (see FT2_1_3_ListRec).  As its name suggests, a         */
/*     FT2_1_3_ListNode is a handle to a single list element.                 */
/*                                                                       */
typedef struct FT2_1_3_ListNodeRec_*  FT2_1_3_ListNode;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FT2_1_3_List                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A handle to a list record (see FT2_1_3_ListRec).                        */
/*                                                                       */
typedef struct FT2_1_3_ListRec_*  FT2_1_3_List;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_ListNodeRec                                                     */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to hold a single list element.                    */
/*                                                                       */
/* <Fields>                                                              */
/*    prev :: The previous element in the list.  NULL if first.          */
/*                                                                       */
/*    next :: The next element in the list.  NULL if last.               */
/*                                                                       */
/*    data :: A typeless pointer to the listed object.                   */
/*                                                                       */
typedef struct  FT2_1_3_ListNodeRec_ {
	FT2_1_3_ListNode  prev;
	FT2_1_3_ListNode  next;
	void*        data;

} FT2_1_3_ListNodeRec;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_ListRec                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to hold a simple doubly-linked list.  These are   */
/*    used in many parts of FreeType.                                    */
/*                                                                       */
/* <Fields>                                                              */
/*    head :: The head (first element) of doubly-linked list.            */
/*                                                                       */
/*    tail :: The tail (last element) of doubly-linked list.             */
/*                                                                       */
typedef struct  FT2_1_3_ListRec_ {
	FT2_1_3_ListNode  head;
	FT2_1_3_ListNode  tail;

} FT2_1_3_ListRec;


/* */

#define FT2_1_3_IS_EMPTY( list )  ( (list).head == 0 )

/* return base error code (without module-specific prefix) */
#define FT2_1_3_ERROR_BASE( x )    ( (x) & 0xFF )

/* return module error code */
#define FT2_1_3_ERROR_MODULE( x )  ( (x) & 0xFF00U )

#define FT2_1_3_BOOL( x )  ( (FT2_1_3_Bool)( x ) )

FT2_1_3_END_HEADER

#endif /* __FTTYPES_H__ */


/* END */
