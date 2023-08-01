/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/***************************************************************************/
/*                                                                         */
/*  ftrender.h                                                             */
/*                                                                         */
/*    FreeType renderer modules public interface (specification).          */
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


#ifndef AGS_LIB_FREETYPE_FTRENDER_H
#define AGS_LIB_FREETYPE_FTRENDER_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftmodule.h"
#include "engines/ags/lib/freetype-2.1.3/ftglyph.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER

/* create a new glyph object */
typedef FT_Error (*FT_Glyph_InitFunc)(FT_Glyph glyph, FT_GlyphSlot slot);

/* destroys a given glyph object */
typedef void (*FT_Glyph_DoneFunc)(FT_Glyph glyph);

typedef void (*FT_Glyph_TransformFunc)(FT_Glyph glyph, FT_Matrix *matrix, FT_Vector *delta);
typedef void (*FT_Glyph_GetBBoxFunc)(FT_Glyph glyph, FT_BBox *abbox);
typedef FT_Error (*FT2_1_3_Glyph_CopyFunc)(FT_Glyph source, FT_Glyph target);
typedef FT_Error (*FT_Glyph_PrepareFunc)(FT_Glyph glyph, FT_GlyphSlot slot);

/* deprecated */
#define FT_Glyph_Init_Func       FT_Glyph_InitFunc
#define FT_Glyph_Done_Func       FT_Glyph_DoneFunc
#define FT_Glyph_Transform_Func  FT_Glyph_TransformFunc
#define FT_Glyph_BBox_Func       FT_Glyph_GetBBoxFunc
#define FT2_1_3_Glyph_Copy_Func       FT2_1_3_Glyph_CopyFunc
#define FT_Glyph_Prepare_Func    FT_Glyph_PrepareFunc

struct  FT_Glyph_Class_ {
	FT_Long                 glyph_size;
	FT_Glyph_Format         glyph_format;
	FT_Glyph_InitFunc       glyph_init;
	FT_Glyph_DoneFunc       glyph_done;
	FT2_1_3_Glyph_CopyFunc       glyph_copy;
	FT_Glyph_TransformFunc  glyph_transform;
	FT_Glyph_GetBBoxFunc    glyph_bbox;
	FT_Glyph_PrepareFunc    glyph_prepare;
};

typedef FT_Error (*FT_Renderer_RenderFunc)(FT_Renderer renderer, FT_GlyphSlot slot, FT_UInt mode, FT_Vector *origin);
typedef FT_Error (*FT_Renderer_TransformFunc)(FT_Renderer renderer, FT_GlyphSlot slot, FT_Matrix *matrix, FT_Vector *delta);
typedef void (*FT_Renderer_GetCBoxFunc)(FT_Renderer renderer, FT_GlyphSlot slot, FT_BBox *cbox);
typedef FT_Error (*FT_Renderer_SetModeFunc)(FT_Renderer renderer, FT_ULong mode_tag, FT_Pointer mode_ptr);

/* deprecated identifiers */
#define FTRenderer_render  FT_Renderer_RenderFunc
#define FTRenderer_transform  FT_Renderer_TransformFunc
#define FTRenderer_getCBox  FT_Renderer_GetCBoxFunc
#define FTRenderer_setMode  FT_Renderer_SetModeFunc

typedef struct FT_Renderer_Class_ {
	FT_Module_Class root;

	FT_Glyph_Format glyph_format;

	FT_Renderer_RenderFunc	  render_glyph;
	FT_Renderer_TransformFunc transform_glyph;
	FT_Renderer_GetCBoxFunc   get_glyph_cbox;
	FT_Renderer_SetModeFunc   set_mode;

	FT_Raster_Funcs *raster_class;

} FT_Renderer_Class;


FT_EXPORT(FT_Renderer)
FT_Get_Renderer(FT_Library library, FT_Glyph_Format format);

FT_EXPORT(FT_Error)
FT_Set_Renderer(FT_Library library, FT_Renderer renderer, FT_UInt num_params, FT_Parameter *parameters);


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTRENDER_H */
