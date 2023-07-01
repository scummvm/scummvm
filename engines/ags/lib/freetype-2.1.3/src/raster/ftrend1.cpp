/***************************************************************************/
/*                                                                         */
/*  ftrend1.c                                                              */
/*                                                                         */
/*    The FreeType glyph rasterizer interface (body).                      */
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


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftoutln.h"
#include "ftrend1.h"
#include "ftraster.h"

#include "rasterrs.h"


/* initialize renderer -- init its raster */
static FT2_1_3_Error
ft_raster1_init( FT2_1_3_Renderer  render ) {
	FT2_1_3_Library  library = FT2_1_3_MODULE_LIBRARY( render );


	render->clazz->raster_class->raster_reset( render->raster,
	        library->raster_pool,
	        library->raster_pool_size );

	return Raster_Err_Ok;
}


/* set render-specific mode */
static FT2_1_3_Error
ft_raster1_set_mode( FT2_1_3_Renderer  render,
                     FT2_1_3_ULong     mode_tag,
                     FT2_1_3_Pointer   data ) {
	/* we simply pass it to the raster */
	return render->clazz->raster_class->raster_set_mode( render->raster,
	        mode_tag,
	        data );
}


/* transform a given glyph image */
static FT2_1_3_Error
ft_raster1_transform( FT2_1_3_Renderer   render,
                      FT2_1_3_GlyphSlot  slot,
                      FT2_1_3_Matrix*    matrix,
                      FT2_1_3_Vector*    delta ) {
	FT2_1_3_Error error = Raster_Err_Ok;


	if ( slot->format != render->glyph_format ) {
		error = Raster_Err_Invalid_Argument;
		goto Exit;
	}

	if ( matrix )
		FT2_1_3_Outline_Transform( &slot->outline, matrix );

	if ( delta )
		FT2_1_3_Outline_Translate( &slot->outline, delta->x, delta->y );

Exit:
	return error;
}


/* return the glyph's control box */
static void
ft_raster1_get_cbox( FT2_1_3_Renderer   render,
                     FT2_1_3_GlyphSlot  slot,
                     FT2_1_3_BBox*      cbox ) {
	FT2_1_3_MEM_ZERO( cbox, sizeof ( *cbox ) );

	if ( slot->format == render->glyph_format )
		FT2_1_3_Outline_Get_CBox( &slot->outline, cbox );
}


/* convert a slot's glyph image into a bitmap */
static FT2_1_3_Error
ft_raster1_render( FT2_1_3_Renderer     render,
                   FT2_1_3_GlyphSlot    slot,
                   FT2_1_3_Render_Mode  mode,
                   FT2_1_3_Vector*      origin ) {
	FT2_1_3_Error     error;
	FT2_1_3_Outline*  outline;
	FT2_1_3_BBox      cbox;
	FT2_1_3_UInt      width, height, pitch;
	FT2_1_3_Bitmap*   bitmap;
	FT2_1_3_Memory    memory;

	FT2_1_3_Raster_Params  params;


	/* check glyph image format */
	if ( slot->format != render->glyph_format ) {
		error = Raster_Err_Invalid_Argument;
		goto Exit;
	}

	/* check rendering mode */
	if ( mode != FT2_1_3_RENDER_MODE_MONO ) {
		/* raster1 is only capable of producing monochrome bitmaps */
		if ( render->clazz == &ft_raster1_renderer_class )
			return Raster_Err_Cannot_Render_Glyph;
	} else {
		/* raster5 is only capable of producing 5-gray-levels bitmaps */
		if ( render->clazz == &ft_raster5_renderer_class )
			return Raster_Err_Cannot_Render_Glyph;
	}

	outline = &slot->outline;

	/* translate the outline to the new origin if needed */
	if ( origin )
		FT2_1_3_Outline_Translate( outline, origin->x, origin->y );

	/* compute the control box, and grid fit it */
	FT2_1_3_Outline_Get_CBox( outline, &cbox );

	cbox.xMin &= -64;
	cbox.yMin &= -64;
	cbox.xMax  = ( cbox.xMax + 63 ) & -64;
	cbox.yMax  = ( cbox.yMax + 63 ) & -64;

	width  = (FT2_1_3_UInt)( ( cbox.xMax - cbox.xMin ) >> 6 );
	height = (FT2_1_3_UInt)( ( cbox.yMax - cbox.yMin ) >> 6 );
	bitmap = &slot->bitmap;
	memory = render->root.memory;

	/* release old bitmap buffer */
	if ( slot->flags & FT2_1_3_GLYPH_OWN_BITMAP ) {
		FT2_1_3_FREE( bitmap->buffer );
		slot->flags &= ~FT2_1_3_GLYPH_OWN_BITMAP;
	}

	/* allocate new one, depends on pixel format */
	if ( !( mode & FT2_1_3_RENDER_MODE_MONO ) ) {
		/* we pad to 32 bits, only for backwards compatibility with FT 1.x */
		pitch = ( width + 3 ) & -4;
		bitmap->pixel_mode = FT2_1_3_PIXEL_MODE_GRAY;
		bitmap->num_grays  = 256;
	} else {
		pitch = ( ( width + 15 ) >> 4 ) << 1;
		bitmap->pixel_mode = FT2_1_3_PIXEL_MODE_MONO;
	}

	bitmap->width = width;
	bitmap->rows  = height;
	bitmap->pitch = pitch;

	if ( FT2_1_3_ALLOC( bitmap->buffer, (FT2_1_3_ULong)pitch * height ) )
		goto Exit;

	slot->flags |= FT2_1_3_GLYPH_OWN_BITMAP;

	/* translate outline to render it into the bitmap */
	FT2_1_3_Outline_Translate( outline, -cbox.xMin, -cbox.yMin );

	/* set up parameters */
	params.target = bitmap;
	params.source = outline;
	params.flags  = 0;

	if ( bitmap->pixel_mode == FT2_1_3_PIXEL_MODE_GRAY )
		params.flags |= FT2_1_3_RASTER_FLAG_AA;

	/* render outline into the bitmap */
	error = render->raster_render( render->raster, &params );

	FT2_1_3_Outline_Translate( outline, cbox.xMin, cbox.yMin );

	if ( error )
		goto Exit;

	slot->format      = FT2_1_3_GLYPH_FORMAT_BITMAP;
	slot->bitmap_left = (FT2_1_3_Int)( cbox.xMin >> 6 );
	slot->bitmap_top  = (FT2_1_3_Int)( cbox.yMax >> 6 );

Exit:
	return error;
}


FT2_1_3_CALLBACK_TABLE_DEF
const FT2_1_3_Renderer_Class  ft_raster1_renderer_class = {
	{
		ft_module_renderer,
		sizeof( FT2_1_3_RendererRec ),

		"raster1",
		0x10000L,
		0x20000L,

		0,    /* module specific interface */

		(FT2_1_3_Module_Constructor)ft_raster1_init,
		(FT2_1_3_Module_Destructor) 0,
		(FT2_1_3_Module_Requester)  0
	},

	FT2_1_3_GLYPH_FORMAT_OUTLINE,

	(FT2_1_3_Renderer_RenderFunc)   ft_raster1_render,
	(FT2_1_3_Renderer_TransformFunc)ft_raster1_transform,
	(FT2_1_3_Renderer_GetCBoxFunc)  ft_raster1_get_cbox,
	(FT2_1_3_Renderer_SetModeFunc)  ft_raster1_set_mode,

	(FT2_1_3_Raster_Funcs*)    &ft_standard_raster
};


/* This renderer is _NOT_ part of the default modules; you will need */
/* to register it by hand in your application.  It should only be    */
/* used for backwards-compatibility with FT 1.x anyway.              */
/*                                                                   */
FT2_1_3_CALLBACK_TABLE_DEF
const FT2_1_3_Renderer_Class  ft_raster5_renderer_class = {
	{
		ft_module_renderer,
		sizeof( FT2_1_3_RendererRec ),

		"raster5",
		0x10000L,
		0x20000L,

		0,    /* module specific interface */

		(FT2_1_3_Module_Constructor)ft_raster1_init,
		(FT2_1_3_Module_Destructor) 0,
		(FT2_1_3_Module_Requester)  0
	},

	FT2_1_3_GLYPH_FORMAT_OUTLINE,

	(FT2_1_3_Renderer_RenderFunc)   ft_raster1_render,
	(FT2_1_3_Renderer_TransformFunc)ft_raster1_transform,
	(FT2_1_3_Renderer_GetCBoxFunc)  ft_raster1_get_cbox,
	(FT2_1_3_Renderer_SetModeFunc)  ft_raster1_set_mode,

	(FT2_1_3_Raster_Funcs*)    &ft_standard_raster
};


/* END */
