/***************************************************************************/
/*                                                                         */
/*  cidobjs.h                                                              */
/*                                                                         */
/*    CID objects manager (specification).                                 */
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


#ifndef __CIDOBJS_H__
#define __CIDOBJS_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_INTERNAL_OBJECTS_H
#include FT2_1_3_CONFIG_CONFIG_H
#include FT2_1_3_INTERNAL_TYPE1_TYPES_H


FT2_1_3_BEGIN_HEADER


/* The following structures must be defined by the hinter */
typedef struct CID_Size_Hints_   CID_Size_Hints;
typedef struct CID_Glyph_Hints_  CID_Glyph_Hints;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    CID_Driver                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    A handle to a Type 1 driver object.                                */
/*                                                                       */
typedef struct CID_DriverRec_*  CID_Driver;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    CID_Size                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    A handle to a Type 1 size object.                                  */
/*                                                                       */
typedef struct CID_SizeRec_*  CID_Size;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    CID_GlyphSlot                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    A handle to a Type 1 glyph slot object.                            */
/*                                                                       */
typedef struct CID_GlyphSlotRec_*  CID_GlyphSlot;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    CID_CharMap                                                        */
/*                                                                       */
/* <Description>                                                         */
/*    A handle to a Type 1 character mapping object.                     */
/*                                                                       */
/* <Note>                                                                */
/*    The Type 1 format doesn't use a charmap but an encoding table.     */
/*    The driver is responsible for making up charmap objects            */
/*    corresponding to these tables.                                     */
/*                                                                       */
typedef struct CID_CharMapRec_*  CID_CharMap;


/*************************************************************************/
/*                                                                       */
/* HERE BEGINS THE TYPE 1 SPECIFIC STUFF                                 */
/*                                                                       */
/*************************************************************************/


typedef struct  CID_SizeRec_ {
	FT2_1_3_SizeRec  root;
	FT2_1_3_Bool     valid;

} CID_SizeRec;


typedef struct  CID_GlyphSlotRec_ {
	FT2_1_3_GlyphSlotRec  root;

	FT2_1_3_Bool          hint;
	FT2_1_3_Bool          scaled;

	FT2_1_3_Fixed         x_scale;
	FT2_1_3_Fixed         y_scale;

} CID_GlyphSlotRec;


FT2_1_3_LOCAL( void )
cid_slot_done( CID_GlyphSlot  slot );

FT2_1_3_LOCAL( FT2_1_3_Error )
cid_slot_init( CID_GlyphSlot   slot );


FT2_1_3_LOCAL( void )
cid_size_done( CID_Size  size );


FT2_1_3_LOCAL( FT2_1_3_Error )
cid_size_init( CID_Size  size );


FT2_1_3_LOCAL( FT2_1_3_Error )
cid_size_reset( CID_Size  size );


FT2_1_3_LOCAL( FT2_1_3_Error )
cid_face_init( FT2_1_3_Stream      stream,
               CID_Face       face,
               FT2_1_3_Int         face_index,
               FT2_1_3_Int         num_params,
               FT2_1_3_Parameter*  params );


FT2_1_3_LOCAL( void )
cid_face_done( CID_Face  face );


FT2_1_3_LOCAL( FT2_1_3_Error )
cid_driver_init( CID_Driver  driver );


FT2_1_3_LOCAL( void )
cid_driver_done( CID_Driver  driver );


FT2_1_3_END_HEADER

#endif /* __CIDOBJS_H__ */


/* END */
