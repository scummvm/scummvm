/***************************************************************************/
/*                                                                         */
/*  ftdriver.h                                                             */
/*                                                                         */
/*    FreeType font driver interface (specification).                      */
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


#ifndef AGS_LIB_FREETYPE_FTDRIVER_H
#define AGS_LIB_FREETYPE_FTDRIVER_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftmodule.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


typedef FT2_1_3_Error
(*FT2_1_3_Face_InitFunc)( FT2_1_3_Stream      stream,
					 FT2_1_3_Face        face,
					 FT2_1_3_Int         typeface_index,
					 FT2_1_3_Int         num_params,
					 FT2_1_3_Parameter*  parameters );

typedef void
(*FT2_1_3_Face_DoneFunc)( FT2_1_3_Face  face );


typedef FT2_1_3_Error
(*FT2_1_3_Size_InitFunc)( FT2_1_3_Size  size );

typedef void
(*FT2_1_3_Size_DoneFunc)( FT2_1_3_Size  size );


typedef FT2_1_3_Error
(*FT2_1_3_Slot_InitFunc)( FT2_1_3_GlyphSlot  slot );

typedef void
(*FT2_1_3_Slot_DoneFunc)( FT2_1_3_GlyphSlot  slot );


typedef FT2_1_3_Error
(*FT2_1_3_Size_ResetPointsFunc)( FT2_1_3_Size     size,
							FT2_1_3_F26Dot6  char_width,
							FT2_1_3_F26Dot6  char_height,
							FT2_1_3_UInt     horz_resolution,
							FT2_1_3_UInt     vert_resolution );

typedef FT2_1_3_Error
(*FT2_1_3_Size_ResetPixelsFunc)( FT2_1_3_Size  size,
							FT2_1_3_UInt  pixel_width,
							FT2_1_3_UInt  pixel_height );

typedef FT2_1_3_Error
(*FT2_1_3_Slot_LoadFunc)( FT2_1_3_GlyphSlot  slot,
					 FT2_1_3_Size       size,
					 FT2_1_3_UInt       glyph_index,
					 FT2_1_3_Int32      load_flags );


typedef FT2_1_3_UInt
(*FT2_1_3_CharMap_CharIndexFunc)( FT2_1_3_CharMap  charmap,
							 FT2_1_3_Long     charcode );

typedef FT2_1_3_Long
(*FT2_1_3_CharMap_CharNextFunc)( FT2_1_3_CharMap  charmap,
							FT2_1_3_Long     charcode );

typedef FT2_1_3_Error
(*FT2_1_3_Face_GetKerningFunc)( FT2_1_3_Face     face,
						   FT2_1_3_UInt     leFT2_1_3_glyph,
						   FT2_1_3_UInt     right_glyph,
						   FT2_1_3_Vector*  kerning );


typedef FT2_1_3_Error
(*FT2_1_3_Face_AttachFunc)( FT2_1_3_Face    face,
					   FT2_1_3_Stream  stream );


typedef FT2_1_3_Error
(*FT2_1_3_Face_GetAdvancesFunc)( FT2_1_3_Face     face,
							FT2_1_3_UInt     first,
							FT2_1_3_UInt     count,
							FT2_1_3_Bool     vertical,
							FT2_1_3_UShort*  advances );


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_Driver_ClassRec                                                 */
/*                                                                       */
/* <Description>                                                         */
/*    The font driver class.  This structure mostly contains pointers to */
/*    driver methods.                                                    */
/*                                                                       */
/* <Fields>                                                              */
/*    root             :: The parent module.                             */
/*                                                                       */
/*    face_object_size :: The size of a face object in bytes.            */
/*                                                                       */
/*    size_object_size :: The size of a size object in bytes.            */
/*                                                                       */
/*    slot_object_size :: The size of a glyph object in bytes.           */
/*                                                                       */
/*    init_face        :: The format-specific face constructor.          */
/*                                                                       */
/*    done_face        :: The format-specific face destructor.           */
/*                                                                       */
/*    init_size        :: The format-specific size constructor.          */
/*                                                                       */
/*    done_size        :: The format-specific size destructor.           */
/*                                                                       */
/*    init_slot        :: The format-specific slot constructor.          */
/*                                                                       */
/*    done_slot        :: The format-specific slot destructor.           */
/*                                                                       */
/*    set_char_sizes   :: A handle to a function used to set the new     */
/*                        character size in points + resolution.  Can be */
/*                        set to 0 to indicate default behaviour.        */
/*                                                                       */
/*    set_pixel_sizes  :: A handle to a function used to set the new     */
/*                        character size in pixels.  Can be set to 0 to  */
/*                        indicate default behaviour.                    */
/*                                                                       */
/*    load_glyph       :: A function handle to load a given glyph image  */
/*                        in a slot.  This field is mandatory!           */
/*                                                                       */
/*    get_char_index   :: A function handle to return the glyph index of */
/*                        a given character for a given charmap.  This   */
/*                        field is mandatory!                            */
/*                                                                       */
/*    get_kerning      :: A function handle to return the unscaled       */
/*                        kerning for a given pair of glyphs.  Can be    */
/*                        set to 0 if the format doesn't support         */
/*                        kerning.                                       */
/*                                                                       */
/*    attach_file      :: This function handle is used to read           */
/*                        additional data for a face from another        */
/*                        file/stream.  For example, this can be used to */
/*                        add data from AFM or PFM files on a Type 1     */
/*                        face, or a CIDMap on a CID-keyed face.         */
/*                                                                       */
/*    get_advances     :: A function handle used to return the advances  */
/*                        of 'count' glyphs, starting at `index'.  the   */
/*                        `vertical' flags must be set when vertical     */
/*                        advances are queried.  The advances buffer is  */
/*                        caller-allocated.                              */
/*                                                                       */
/* <Note>                                                                */
/*    Most function pointers, with the exception of `load_glyph' and     */
/*    `get_char_index' can be set to 0 to indicate a default behaviour.  */
/*                                                                       */
typedef struct  FT2_1_3_Driver_ClassRec_ {
	FT2_1_3_Module_Class           root;

	FT2_1_3_Int                    face_object_size;
	FT2_1_3_Int                    size_object_size;
	FT2_1_3_Int                    slot_object_size;

	FT2_1_3_Face_InitFunc          init_face;
	FT2_1_3_Face_DoneFunc          done_face;

	FT2_1_3_Size_InitFunc          init_size;
	FT2_1_3_Size_DoneFunc          done_size;

	FT2_1_3_Slot_InitFunc          init_slot;
	FT2_1_3_Slot_DoneFunc          done_slot;

	FT2_1_3_Size_ResetPointsFunc   set_char_sizes;
	FT2_1_3_Size_ResetPixelsFunc   set_pixel_sizes;

	FT2_1_3_Slot_LoadFunc          load_glyph;

	FT2_1_3_Face_GetKerningFunc    get_kerning;
	FT2_1_3_Face_AttachFunc        attach_file;
	FT2_1_3_Face_GetAdvancesFunc   get_advances;

} FT2_1_3_Driver_ClassRec, *FT2_1_3_Driver_Class;


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTDRIVER_H */


/* END */
