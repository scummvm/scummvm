/***************************************************************************/
/*                                                                         */
/*  cffobjs.h                                                              */
/*                                                                         */
/*    OpenType objects manager (specification).                            */
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


#ifndef AGS_LIB_FREETYPE_CFFOBJS_H
#define AGS_LIB_FREETYPE_CFFOBJS_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/cfftypes.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/tttypes.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psnames.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    CFF_Driver                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    A handle to an OpenType driver object.                             */
/*                                                                       */
typedef struct CFF_DriverRec_*  CFF_Driver;

typedef TT_Face  CFF_Face;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    CFF_Size                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    A handle to an OpenType size object.                               */
/*                                                                       */
typedef FT_Size  CFF_Size;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    CFF_GlyphSlot                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    A handle to an OpenType glyph slot object.                         */
/*                                                                       */
typedef struct  CFF_GlyphSlotRec_ {
	FT_GlyphSlotRec  root;

	FT_Bool          hint;
	FT_Bool          scaled;

	FT_Fixed         x_scale;
	FT_Fixed         y_scale;

} CFF_GlyphSlotRec, *CFF_GlyphSlot;



/*************************************************************************/
/*                                                                       */
/* Subglyph transformation record.                                       */
/*                                                                       */
typedef struct  CFF_Transform_ {
	FT_Fixed    xx, xy;     /* transformation matrix coefficients */
	FT_Fixed    yx, yy;
	FT_F26Dot6  ox, oy;     /* offsets        */

} CFF_Transform;


/* this is only used in the case of a pure CFF font with no charmap */
typedef struct  CFF_CharMapRec_ {
	TT_CharMapRec  root;
	PS_Unicodes    unicodes;

} CFF_CharMapRec, *CFF_CharMap;


/***********************************************************************/
/*                                                                     */
/* TrueType driver class.                                              */
/*                                                                     */
typedef struct  CFF_DriverRec_ {
	FT_DriverRec  root;
	void*         extension_component;

} CFF_DriverRec;


FT2_1_3_LOCAL( FT_Error )
cff_size_init( CFF_Size  size );

FT2_1_3_LOCAL( void )
cff_size_done( CFF_Size  size );

FT2_1_3_LOCAL( FT_Error )
cff_size_reset( CFF_Size  size );

FT2_1_3_LOCAL( void )
cff_slot_done( CFF_GlyphSlot  slot );

FT2_1_3_LOCAL( FT_Error )
cff_slot_init( CFF_GlyphSlot   slot );


/*************************************************************************/
/*                                                                       */
/* Face functions                                                        */
/*                                                                       */
FT2_1_3_LOCAL( FT_Error )
cff_face_init( FT_Stream      stream,
			   CFF_Face       face,
			   FT_Int         face_index,
			   FT_Int         num_params,
			   FT_Parameter*  params );

FT2_1_3_LOCAL( void )
cff_face_done( CFF_Face  face );


/*************************************************************************/
/*                                                                       */
/* Driver functions                                                      */
/*                                                                       */
FT2_1_3_LOCAL( FT_Error )
cff_driver_init( CFF_Driver  driver );

FT2_1_3_LOCAL( void )
cff_driver_done( CFF_Driver  driver );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_CFFOBJS_H */


/* END */
