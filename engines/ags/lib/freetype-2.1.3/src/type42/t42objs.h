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


#ifndef AGS_LIB_FREETYPE_T42OBJS_H
#define AGS_LIB_FREETYPE_T42OBJS_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/t1tables.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/t1types.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/t42types.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/ftdriver.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psnames.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/pshints.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/* Type42 size */
typedef struct  T42_SizeRec_ {
	FT_SizeRec  root;
	FT_Size     ttsize;

} T42_SizeRec, *T42_Size;


/* Type42 slot */
typedef struct  T42_GlyphSlotRec_ {
	FT_GlyphSlotRec  root;
	FT_GlyphSlot     ttslot;

} T42_GlyphSlotRec, *T42_GlyphSlot;


/* Type 42 driver */
typedef struct  T42_DriverRec_ {
	FT_DriverRec     root;
	FT_Driver_Class  ttclazz;
	void*            extension_component;

} T42_DriverRec, *T42_Driver;


/* */


FT2_1_3_LOCAL( FT_Error )
T42_Face_Init( FT_Stream      stream,
			   T42_Face       face,
			   FT_Int         face_index,
			   FT_Int         num_params,
			   FT_Parameter*  params );


FT2_1_3_LOCAL( void )
T42_Face_Done( T42_Face  face );


FT2_1_3_LOCAL( FT_Error )
T42_Size_Init( T42_Size  size );


FT2_1_3_LOCAL( FT_Error )
T42_Size_SetChars( T42_Size    size,
				   FT_F26Dot6  char_width,
				   FT_F26Dot6  char_height,
				   FT_UInt     horz_resolution,
				   FT_UInt     vert_resolution );

FT2_1_3_LOCAL( FT_Error )
T42_Size_SetPixels( T42_Size  size,
					FT_UInt   pixel_width,
					FT_UInt   pixel_height );

FT2_1_3_LOCAL( void )
T42_Size_Done( T42_Size  size );


FT2_1_3_LOCAL( FT_Error )
T42_GlyphSlot_Init( T42_GlyphSlot  slot );


FT2_1_3_LOCAL( FT_Error )
T42_GlyphSlot_Load( FT_GlyphSlot  glyph,
					FT_Size       size,
					FT_Int        glyph_index,
					FT_Int32      load_flags );

FT2_1_3_LOCAL( void )
T42_GlyphSlot_Done( T42_GlyphSlot slot );


FT2_1_3_LOCAL( FT_Error )
T42_Driver_Init( T42_Driver  driver );

FT2_1_3_LOCAL( void )
T42_Driver_Done( T42_Driver  driver );

/* */

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T42OBJS_H */


/* END */
