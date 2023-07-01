/***************************************************************************/
/*                                                                         */
/*  t42objs.h                                                              */
/*                                                                         */
/*    Type 42 objects manager (specification).                             */
/*                                                                         */
/*  Copyright 2002 by Roberto Alameda.                                     */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __T42OBJS_H__
#define __T42OBJS_H__

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/t1tables.h"
#include FT2_1_3_INTERNAL_TYPE1_TYPES_H
#include FT2_1_3_INTERNAL_TYPE42_TYPES_H
#include FT2_1_3_INTERNAL_OBJECTS_H
#include FT2_1_3_INTERNAL_DRIVER_H
#include FT2_1_3_INTERNAL_POSTSCRIPT_NAMES_H
#include FT2_1_3_INTERNAL_POSTSCRIPT_HINTS_H


FT2_1_3_BEGIN_HEADER


/* Type42 size */
typedef struct  T42_SizeRec_ {
	FT2_1_3_SizeRec  root;
	FT2_1_3_Size     ttsize;

} T42_SizeRec, *T42_Size;


/* Type42 slot */
typedef struct  T42_GlyphSlotRec_ {
	FT2_1_3_GlyphSlotRec  root;
	FT2_1_3_GlyphSlot     ttslot;

} T42_GlyphSlotRec, *T42_GlyphSlot;


/* Type 42 driver */
typedef struct  T42_DriverRec_ {
	FT2_1_3_DriverRec     root;
	FT2_1_3_Driver_Class  ttclazz;
	void*            extension_component;

} T42_DriverRec, *T42_Driver;


/* */


FT2_1_3_LOCAL( FT2_1_3_Error )
T42_Face_Init( FT2_1_3_Stream      stream,
               T42_Face       face,
               FT2_1_3_Int         face_index,
               FT2_1_3_Int         num_params,
               FT2_1_3_Parameter*  params );


FT2_1_3_LOCAL( void )
T42_Face_Done( T42_Face  face );


FT2_1_3_LOCAL( FT2_1_3_Error )
T42_Size_Init( T42_Size  size );


FT2_1_3_LOCAL( FT2_1_3_Error )
T42_Size_SetChars( T42_Size    size,
                   FT2_1_3_F26Dot6  char_width,
                   FT2_1_3_F26Dot6  char_height,
                   FT2_1_3_UInt     horz_resolution,
                   FT2_1_3_UInt     vert_resolution );

FT2_1_3_LOCAL( FT2_1_3_Error )
T42_Size_SetPixels( T42_Size  size,
                    FT2_1_3_UInt   pixel_width,
                    FT2_1_3_UInt   pixel_height );

FT2_1_3_LOCAL( void )
T42_Size_Done( T42_Size  size );


FT2_1_3_LOCAL( FT2_1_3_Error )
T42_GlyphSlot_Init( T42_GlyphSlot  slot );


FT2_1_3_LOCAL( FT2_1_3_Error )
T42_GlyphSlot_Load( FT2_1_3_GlyphSlot  glyph,
                    FT2_1_3_Size       size,
                    FT2_1_3_Int        glyph_index,
                    FT2_1_3_Int32      load_flags );

FT2_1_3_LOCAL( void )
T42_GlyphSlot_Done( T42_GlyphSlot slot );


FT2_1_3_LOCAL( FT2_1_3_Error )
T42_Driver_Init( T42_Driver  driver );

FT2_1_3_LOCAL( void )
T42_Driver_Done( T42_Driver  driver );

/* */

FT2_1_3_END_HEADER


#endif /* __T42OBJS_H__ */


/* END */
