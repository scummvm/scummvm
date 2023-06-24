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


#ifndef __FTRENDER_H__
#define __FTRENDER_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_MODULE_H
#include FT2_1_3_GLYPH_H


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    module_management                                                  */
/*                                                                       */
/*************************************************************************/


/* create a new glyph object */
typedef FT2_1_3_Error
(*FT2_1_3_Glyph_InitFunc)( FT2_1_3_Glyph      glyph,
                      FT2_1_3_GlyphSlot  slot );

/* destroys a given glyph object */
typedef void
(*FT2_1_3_Glyph_DoneFunc)( FT2_1_3_Glyph  glyph );

typedef void
(*FT2_1_3_Glyph_TransformFunc)( FT2_1_3_Glyph    glyph,
                           FT2_1_3_Matrix*  matrix,
                           FT2_1_3_Vector*  delta );

typedef void
(*FT2_1_3_Glyph_GetBBoxFunc)( FT2_1_3_Glyph  glyph,
                         FT2_1_3_BBox*  abbox );

typedef FT2_1_3_Error
(*FT2_1_3_Glyph_CopyFunc)( FT2_1_3_Glyph   source,
                      FT2_1_3_Glyph   target );

typedef FT2_1_3_Error
(*FT2_1_3_Glyph_PrepareFunc)( FT2_1_3_Glyph      glyph,
                         FT2_1_3_GlyphSlot  slot );

/* deprecated */
#define FT2_1_3_Glyph_Init_Func       FT2_1_3_Glyph_InitFunc
#define FT2_1_3_Glyph_Done_Func       FT2_1_3_Glyph_DoneFunc
#define FT2_1_3_Glyph_Transform_Func  FT2_1_3_Glyph_TransformFunc
#define FT2_1_3_Glyph_BBox_Func       FT2_1_3_Glyph_GetBBoxFunc
#define FT2_1_3_Glyph_Copy_Func       FT2_1_3_Glyph_CopyFunc
#define FT2_1_3_Glyph_Prepare_Func    FT2_1_3_Glyph_PrepareFunc


struct  FT2_1_3_Glyph_Class_ {
	FT2_1_3_Long                 glyph_size;
	FT2_1_3_Glyph_Format         glyph_format;
	FT2_1_3_Glyph_InitFunc       glyph_init;
	FT2_1_3_Glyph_DoneFunc       glyph_done;
	FT2_1_3_Glyph_CopyFunc       glyph_copy;
	FT2_1_3_Glyph_TransformFunc  glyph_transform;
	FT2_1_3_Glyph_GetBBoxFunc    glyph_bbox;
	FT2_1_3_Glyph_PrepareFunc    glyph_prepare;
};


typedef FT2_1_3_Error
(*FT2_1_3_Renderer_RenderFunc)( FT2_1_3_Renderer   renderer,
                           FT2_1_3_GlyphSlot  slot,
                           FT2_1_3_UInt       mode,
                           FT2_1_3_Vector*    origin );

typedef FT2_1_3_Error
(*FT2_1_3_Renderer_TransformFunc)( FT2_1_3_Renderer   renderer,
                              FT2_1_3_GlyphSlot  slot,
                              FT2_1_3_Matrix*    matrix,
                              FT2_1_3_Vector*    delta );


typedef void
(*FT2_1_3_Renderer_GetCBoxFunc)( FT2_1_3_Renderer   renderer,
                            FT2_1_3_GlyphSlot  slot,
                            FT2_1_3_BBox*      cbox );


typedef FT2_1_3_Error
(*FT2_1_3_Renderer_SetModeFunc)( FT2_1_3_Renderer  renderer,
                            FT2_1_3_ULong     mode_tag,
                            FT2_1_3_Pointer   mode_ptr );

/* deprecated identifiers */
#define FTRenderer_render  FT2_1_3_Renderer_RenderFunc
#define FTRenderer_transform  FT2_1_3_Renderer_TransformFunc
#define FTRenderer_getCBox  FT2_1_3_Renderer_GetCBoxFunc
#define FTRenderer_setMode  FT2_1_3_Renderer_SetModeFunc


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_Renderer_Class                                                  */
/*                                                                       */
/* <Description>                                                         */
/*    The renderer module class descriptor.                              */
/*                                                                       */
/* <Fields>                                                              */
/*    root         :: The root FT2_1_3_Module_Class fields.                   */
/*                                                                       */
/*    glyph_format :: The glyph image format this renderer handles.      */
/*                                                                       */
/*    render_glyph :: A method used to render the image that is in a     */
/*                    given glyph slot into a bitmap.                    */
/*                                                                       */
/*    set_mode     :: A method used to pass additional parameters.       */
/*                                                                       */
/*    raster_class :: For `FT2_1_3_GLYPH_FORMAT_OUTLINE' renderers only, this */
/*                    is a pointer to its raster's class.                */
/*                                                                       */
/*    raster       :: For `FT2_1_3_GLYPH_FORMAT_OUTLINE' renderers only. this */
/*                    is a pointer to the corresponding raster object,   */
/*                    if any.                                            */
/*                                                                       */
typedef struct  FT2_1_3_Renderer_Class_ {
	FT2_1_3_Module_Class       root;

	FT2_1_3_Glyph_Format       glyph_format;

	FT2_1_3_Renderer_RenderFunc     render_glyph;
	FT2_1_3_Renderer_TransformFunc  transform_glyph;
	FT2_1_3_Renderer_GetCBoxFunc    get_glyph_cbox;
	FT2_1_3_Renderer_SetModeFunc    set_mode;

	FT2_1_3_Raster_Funcs*           raster_class;

} FT2_1_3_Renderer_Class;


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Get_Renderer                                                    */
/*                                                                       */
/* <Description>                                                         */
/*    Retrieves the current renderer for a given glyph format.           */
/*                                                                       */
/* <Input>                                                               */
/*    library :: A handle to the library object.                         */
/*                                                                       */
/*    format  :: The glyph format.                                       */
/*                                                                       */
/* <Return>                                                              */
/*    A renderer handle.  0 if none found.                               */
/*                                                                       */
/* <Note>                                                                */
/*    An error will be returned if a module already exists by that name, */
/*    or if the module requires a version of FreeType that is too great. */
/*                                                                       */
/*    To add a new renderer, simply use FT2_1_3_Add_Module().  To retrieve a  */
/*    renderer by its name, use FT2_1_3_Get_Module().                         */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Renderer )
FT2_1_3_Get_Renderer( FT2_1_3_Library       library,
                 FT2_1_3_Glyph_Format  format );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Set_Renderer                                                    */
/*                                                                       */
/* <Description>                                                         */
/*    Sets the current renderer to use, and set additional mode.         */
/*                                                                       */
/* <InOut>                                                               */
/*    library    :: A handle to the library object.                      */
/*                                                                       */
/* <Input>                                                               */
/*    renderer   :: A handle to the renderer object.                     */
/*                                                                       */
/*    num_params :: The number of additional parameters.                 */
/*                                                                       */
/*    parameters :: Additional parameters.                               */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    In case of success, the renderer will be used to convert glyph     */
/*    images in the renderer's known format into bitmaps.                */
/*                                                                       */
/*    This doesn't change the current renderer for other formats.        */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Set_Renderer( FT2_1_3_Library     library,
                 FT2_1_3_Renderer    renderer,
                 FT2_1_3_UInt        num_params,
                 FT2_1_3_Parameter*  parameters );


/* */


FT2_1_3_END_HEADER

#endif /* __FTRENDER_H__ */


/* END */
